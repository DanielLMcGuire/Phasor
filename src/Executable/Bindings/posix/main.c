#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

// -----------------
// File I/O Wrappers
// -----------------
static PhasorValue phasor_open(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 2 || !phasor_is_string(argv[0]) || !phasor_is_int(argv[1]))
		return phasor_make_int(-1);
	const char *path = phasor_to_string(argv[0]);
	int         flags = (int)phasor_to_int(argv[1]);
	int         fd = open(path, flags, 0666);
	return phasor_make_int(fd);
}

static PhasorValue phasor_close(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 1 || !phasor_is_int(argv[0]))
		return phasor_make_int(-1);
	int fd = (int)phasor_to_int(argv[0]);
	return phasor_make_int(close(fd));
}

static PhasorValue phasor_read(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 2 || !phasor_is_int(argv[0]) || !phasor_is_int(argv[1]))
		return phasor_make_string("");
	int    fd = (int)phasor_to_int(argv[0]);
	size_t count = (size_t)phasor_to_int(argv[1]);
	char  *buf = (char *)malloc(count + 1);
	if (!buf)
		return phasor_make_string("");
	ssize_t r = read(fd, buf, count);
	if (r >= 0)
		buf[r] = '\0';
	else
		buf[0] = '\0';
	PhasorValue val = phasor_make_string(buf);
	free(buf);
	return val;
}

static PhasorValue phasor_write(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 2 || !phasor_is_int(argv[0]) || !phasor_is_string(argv[1]))
		return phasor_make_int(-1);
	int         fd = (int)phasor_to_int(argv[0]);
	const char *s = phasor_to_string(argv[1]);
	ssize_t     r = write(fd, s, strlen(s));
	return phasor_make_int(r);
}

static PhasorValue phasor_unlink(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 1 || !phasor_is_string(argv[0]))
		return phasor_make_int(-1);
	return phasor_make_int(unlink(phasor_to_string(argv[0])));
}

// -----------------
// Directory Wrappers
// -----------------
static PhasorValue phasor_mkdir(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 1 || !phasor_is_string(argv[0]))
		return phasor_make_int(-1);
	const char *path = phasor_to_string(argv[0]);
	return phasor_make_int(mkdir(path, 0777));
}

static PhasorValue phasor_rmdir(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 1 || !phasor_is_string(argv[0]))
		return phasor_make_int(-1);
	return phasor_make_int(rmdir(phasor_to_string(argv[0])));
}

// -----------------
// Process Wrappers
// -----------------
static PhasorValue phasor_fork(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	pid_t pid = fork();
	return phasor_make_int((int64_t)pid);
}

static PhasorValue phasor_execve(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 3 || !phasor_is_string(argv[0]) || !phasor_is_array(argv[1]) || !phasor_is_array(argv[2]))
	{
		return phasor_make_int(-1);
	}
	const char        *path = phasor_to_string(argv[0]);
	const PhasorValue *argarr = argv[1].as.a.elements;
	size_t             argcnt = argv[1].as.a.count;
	const PhasorValue *envarr = argv[2].as.a.elements;
	size_t             envcnt = argv[2].as.a.count;

	char **args = (char **)malloc((argcnt + 1) * sizeof(char *));
	char **envs = (char **)malloc((envcnt + 1) * sizeof(char *));
	for (size_t i = 0; i < argcnt; i++)
		args[i] = (char *)phasor_to_string(argarr[i]);
	args[argcnt] = NULL;
	for (size_t i = 0; i < envcnt; i++)
		envs[i] = (char *)phasor_to_string(envarr[i]);
	envs[envcnt] = NULL;

	int r = execve(path, args, envs);
	free(args);
	free(envs);
	return phasor_make_int(r); // will only return on error
}

static PhasorValue phasor_waitpid(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 2 || !phasor_is_int(argv[0]) || !phasor_is_int(argv[1]))
		return phasor_make_int(-1);
	pid_t pid = (pid_t)phasor_to_int(argv[0]);
	int   options = (int)phasor_to_int(argv[1]);
	int   status = 0;
	pid_t r = waitpid(pid, &status, options);
	return phasor_make_array((PhasorValue[]){phasor_make_int(r), phasor_make_int(status)}, 2);
}

// -----------------
// Signal Wrappers
// -----------------
static PhasorValue phasor_kill(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 2 || !phasor_is_int(argv[0]) || !phasor_is_int(argv[1]))
		return phasor_make_int(-1);
	pid_t pid = (pid_t)phasor_to_int(argv[0]);
	int   sig = (int)phasor_to_int(argv[1]);
	return phasor_make_int(kill(pid, sig));
}

// -----------------
// Time Wrappers
// -----------------
static PhasorValue phasor_sleep(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 1 || !phasor_is_int(argv[0]))
		return phasor_make_int(-1);
	unsigned int sec = (unsigned int)phasor_to_int(argv[0]);
	unsigned int r = sleep(sec);
	return phasor_make_int(r);
}

static PhasorValue phasor_nanosleep(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 2 || !phasor_is_int(argv[0]) || !phasor_is_int(argv[1]))
		return phasor_make_int(-1);
	struct timespec req = {(time_t)phasor_to_int(argv[0]), (long)phasor_to_int(argv[1])};
	struct timespec rem;
	int             r = nanosleep(&req, &rem);
	return phasor_make_int(r);
}

static PhasorValue phasor_clock_gettime(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	if (argc < 1 || !phasor_is_int(argv[0]))
		return phasor_make_int(-1);
	struct timespec ts;
	int             r = clock_gettime((clockid_t)phasor_to_int(argv[0]), &ts);
	PhasorValue     vals[2] = {phasor_make_int(ts.tv_sec), phasor_make_int(ts.tv_nsec)};
	return phasor_make_array(vals, 2);
}

// -----------------
// Plugin Entry
// -----------------
PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm)
{
	api->register_function(vm, "posix_open", phasor_open);
	api->register_function(vm, "posix_close", phasor_close);
	api->register_function(vm, "posix_read", phasor_read);
	api->register_function(vm, "posix_write", phasor_write);
	api->register_function(vm, "posix_unlink", phasor_unlink);

	api->register_function(vm, "posix_mkdir", phasor_mkdir);
	api->register_function(vm, "posix_rmdir", phasor_rmdir);

	api->register_function(vm, "posix_fork", phasor_fork);
	api->register_function(vm, "posix_execve", phasor_execve);
	api->register_function(vm, "posix_waitpid", phasor_waitpid);

	api->register_function(vm, "posix_kill", phasor_kill);

	api->register_function(vm, "posix_sleep", phasor_sleep);
	api->register_function(vm, "posix_nanosleep", phasor_nanosleep);
	api->register_function(vm, "posix_clock_gettime", phasor_clock_gettime);
}
