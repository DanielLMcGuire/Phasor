#define _WIN32_WINNT 0x0601

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static void print_time(const wchar_t *name, LONGLONG ticks)
{
    double seconds = (double)ticks / 10000000.0;
    int minutes = (int)(seconds / 60.0);
    seconds -= minutes * 60.0;

    wprintf(L"%-7s %dm%0.3fs\n", name, minutes, seconds);
}

static wchar_t *get_child_command_line(void)
{
    wchar_t *raw = GetCommandLineW();
    wchar_t *p = raw;

    while (*p == L' ' || *p == L'\t')
        ++p;

    if (*p == L'"') {
        ++p;

        while (*p) {
            if (*p == L'"') {
                ++p;
                break;
            }
            ++p;
        }
    }
    else {
        while (*p && *p != L' ' && *p != L'\t')
            ++p;
    }

    while (*p == L' ' || *p == L'\t')
        ++p;

    if (!*p)
        return NULL;

    return _wcsdup(p);
}

int wmain(void)
{
    wchar_t *command_line = get_child_command_line();

    if (!command_line) {
        fwprintf(stderr, L"usage: time <command> [args...]\n");
        return 1;
    }

    HANDLE job = CreateJobObjectW(NULL, NULL);
    if (!job) {
        fwprintf(stderr, L"CreateJobObject failed: %lu\n", GetLastError());
        free(command_line);
        return 1;
    }

    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(si);

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    if (!CreateProcessW(
            NULL,
            command_line,
            NULL,
            NULL,
            TRUE,
            CREATE_SUSPENDED,
            NULL,
            NULL,
            &si,
            &pi)) {

        fwprintf(stderr, L"CreateProcess failed: %lu\n", GetLastError());

        CloseHandle(job);
        free(command_line);
        return 1;
    }

    if (!AssignProcessToJobObject(job, pi.hProcess)) {
        fwprintf(stderr,
                 L"AssignProcessToJobObject failed: %lu\n",
                 GetLastError());

        TerminateProcess(pi.hProcess, 1);

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(job);
        free(command_line);

        return 1;
    }

    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    QueryPerformanceCounter(&end);

    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting = {0};

    if (!QueryInformationJobObject(
            job,
            JobObjectBasicAccountingInformation,
            &accounting,
            sizeof(accounting),
            NULL)) {

        fwprintf(stderr,
                 L"QueryInformationJobObject failed: %lu\n",
                 GetLastError());

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(job);
        free(command_line);

        return 1;
    }

    double real_seconds =
        (double)(end.QuadPart - start.QuadPart) /
        (double)frequency.QuadPart;

    int real_minutes = (int)(real_seconds / 60.0);
    real_seconds -= real_minutes * 60.0;

    wprintf(L"real    %dm%0.3fs\n",
            real_minutes,
            real_seconds);

    print_time(L"user", accounting.TotalUserTime.QuadPart);
    print_time(L"sys", accounting.TotalKernelTime.QuadPart);

    DWORD exit_code = 1;

    if (!GetExitCodeProcess(pi.hProcess, &exit_code))
        exit_code = 1;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(job);
    free(command_line);

    return (int)exit_code;
}
