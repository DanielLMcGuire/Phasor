#include "IO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

static char **parse_argv(const char *cmd, int *argc)
{
	if (!cmd || !*cmd)
		return NULL;
	size_t len = strlen(cmd);
	char  *copy = malloc(len + 1);
	if (!copy)
		return NULL;

#ifdef _WIN32
	strcpy_s(copy, len + 1, cmd);
#else
	strcpy(copy, cmd);
#endif

	*argc = 0;
#ifdef _WIN32
	char *context = NULL;
	char *token = strtok_s(copy, " ", &context);
#else
	char *token = strtok(copy, " ");
#endif
	while (token)
	{
		(*argc)++;
#ifdef _WIN32
		token = strtok_s(NULL, " ", &context);
#else
		token = strtok(NULL, " ");
#endif
	}

#ifdef _WIN32
	strcpy_s(copy, len + 1, cmd);
#else
	strcpy(copy, cmd);
#endif

	char **argv = malloc((*argc + 1) * sizeof(char *));
	if (!argv)
	{
		free(copy);
		return NULL;
	}

	int i = 0;
#ifdef _WIN32
	token = strtok_s(copy, " ", &context);
#else
	token = strtok(copy, " ");
#endif
	while (token)
	{
		argv[i] = malloc(strlen(token) + 1);
		if (!argv[i])
		{
			for (int j = 0; j < i; j++)
				free(argv[j]);
			free(argv);
			free(copy);
			return NULL;
		}
#ifdef _WIN32
		strcpy_s(argv[i], strlen(token) + 1, token);
#else
		strcpy(argv[i], token);
#endif
		i++;
#ifdef _WIN32
		token = strtok_s(NULL, " ", &context);
#else
		token = strtok(NULL, " ");
#endif
	}
	argv[i] = NULL;
	free(copy);
	return argv;
}

static void free_argv(char **argv)
{
	if (!argv)
		return;
	for (int i = 0; argv[i]; i++)
		free(argv[i]);
	free(argv);
}

void asm_print_stdout(const char *s, int64_t len)
{
	fwrite(s, 1, (size_t)len, stdout);
	fflush(stdout);
}

void asm_print_stderr(const char *s, int64_t len)
{
	fwrite(s, 1, (size_t)len, stderr);
	fflush(stderr);
}

int64_t asm_system(const char *cmd)
{
	return (int64_t)system(cmd);
}

char *asm_system_out(const char *cmd)
{
	int    argc;
	char **argv = parse_argv(cmd, &argc);
	if (!argv)
		return NULL;
	char  *output = NULL;
	size_t output_size = 0;
	size_t output_capacity = 1024;
	output = malloc(output_capacity);
	if (!output)
	{
		free_argv(argv);
		return NULL;
	}
#ifdef _WIN32
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	HANDLE              hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		free(output);
		free_argv(argv);
		return NULL;
	}
	STARTUPINFO si = {sizeof(STARTUPINFO)};
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWrite; // capture stdout
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	PROCESS_INFORMATION pi;
	if (!CreateProcess(NULL, (char *)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(hRead);
		CloseHandle(hWrite);
		free(output);
		free_argv(argv);
		return NULL;
	}
	CloseHandle(hWrite);
	DWORD bytesRead;
	char  buffer[1024];
	while (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
	{
		if (output_size + bytesRead >= output_capacity)
		{
			output_capacity *= 2;
			char *new_output = realloc(output, output_capacity);
			if (!new_output)
			{
				CloseHandle(hRead);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				free(output);
				free_argv(argv);
				return NULL;
			}
			output = new_output;
		}
		memcpy(output + output_size, buffer, bytesRead);
		output_size += bytesRead;
	}
	output[output_size] = '\0';
	CloseHandle(hRead);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		free(output);
		free_argv(argv);
		return NULL;
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		free(output);
		free_argv(argv);
		return NULL;
	}
	if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		execvp(argv[0], argv);
		_exit(1);
	}
	else
	{
		close(pipefd[1]);
		ssize_t bytesRead;
		char    buffer[1024];
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		{
			if (output_size + bytesRead >= output_capacity)
			{
				output_capacity *= 2;
				char *new_output = realloc(output, output_capacity);
				if (!new_output)
				{
					close(pipefd[0]);
					free(output);
					free_argv(argv);
					return NULL;
				}
				output = new_output;
			}
			memcpy(output + output_size, buffer, bytesRead);
			output_size += bytesRead;
		}
		output[output_size] = '\0';
		close(pipefd[0]);
		int status;
		waitpid(pid, &status, 0);
	}
#endif
	free_argv(argv);
	return output;
}

char *asm_system_err(const char *cmd)
{
	int    argc;
	char **argv = parse_argv(cmd, &argc);
	if (!argv)
		return NULL;
	char  *output = NULL;
	size_t output_size = 0;
	size_t output_capacity = 1024;
	output = malloc(output_capacity);
	if (!output)
	{
		free_argv(argv);
		return NULL;
	}
#ifdef _WIN32
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	HANDLE              hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		free(output);
		free_argv(argv);
		return NULL;
	}
	STARTUPINFO si = {sizeof(STARTUPINFO)};
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = hWrite;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	PROCESS_INFORMATION pi;
	if (!CreateProcess(NULL, (char *)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(hRead);
		CloseHandle(hWrite);
		free(output);
		free_argv(argv);
		return NULL;
	}
	CloseHandle(hWrite);
	DWORD bytesRead;
	char  buffer[1024];
	while (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
	{
		if (output_size + bytesRead >= output_capacity)
		{
			output_capacity *= 2;
			char *new_output = realloc(output, output_capacity);
			if (!new_output)
			{
				CloseHandle(hRead);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				free(output);
				free_argv(argv);
				return NULL;
			}
			output = new_output;
		}
		memcpy(output + output_size, buffer, bytesRead);
		output_size += bytesRead;
	}
	output[output_size] = '\0';
	CloseHandle(hRead);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		free(output);
		free_argv(argv);
		return NULL;
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		free(output);
		free_argv(argv);
		return NULL;
	}
	if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);
		execvp(argv[0], argv);
		_exit(1);
	}
	else
	{
		close(pipefd[1]);
		ssize_t bytesRead;
		char    buffer[1024];
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		{
			if (output_size + bytesRead >= output_capacity)
			{
				output_capacity *= 2;
				char *new_output = realloc(output, output_capacity);
				if (!new_output)
				{
					close(pipefd[0]);
					free(output);
					free_argv(argv);
					return NULL;
				}
				output = new_output;
			}
			memcpy(output + output_size, buffer, bytesRead);
			output_size += bytesRead;
		}
		output[output_size] = '\0';
		close(pipefd[0]);
		int status;
		waitpid(pid, &status, 0);
	}
#endif
	free_argv(argv);
	return output;
}
