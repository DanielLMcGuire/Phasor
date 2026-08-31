// Copyright 2025-2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define _CRT_SECURE_NO_WARNINGS

#include <PhasorRT.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

typedef struct _rt_args
{
    const char *inputFile;
    bool verbose;
    int scriptArgc;
    char **scriptArgv;
} RuntimeArgs;

static void printRuntimeHelp(const char *programName)
{
    const char *name = programName ? programName : "phasorrt";
    const char *version = getVersion();
    fprintf(stdout,
            "Phasor Runtime v%s\n"
            "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
            "Usage: %s [options] <file.phsb> [...script args]\n\n"
            "Options:\n"
            "  -v, --verbose       Enable verbose output\n"
            "  -h, --help          Show this help message\n",
            version, name);
}

static unsigned char *readBinaryFile(const char *path, size_t *outSize)
{
    FILE *file = fopen(path, "rb");
    if (file == nullptr)
    {
        fprintf(stderr, "Error: could not open '%s': %s\n", path, strerror(errno));
        return nullptr;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        fprintf(stderr, "Error: failed to seek '%s'\n", path);
        return nullptr;
    }

    long fileSize = ftell(file);
    if (fileSize < 0)
    {
        fclose(file);
        fprintf(stderr, "Error: failed to measure '%s'\n", path);
        return nullptr;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        fprintf(stderr, "Error: failed to rewind '%s'\n", path);
        return nullptr;
    }

    unsigned char *buffer = (unsigned char *)malloc(fileSize == 0 ? 1 : (size_t)fileSize);
    if (buffer == nullptr)
    {
        fclose(file);
        fprintf(stderr, "Error: out of memory\n");
        return nullptr;
    }

    size_t readCount = 0;
    if (fileSize > 0)
    {
        readCount = fread(buffer, 1, (size_t)fileSize, file);
    }
    fclose(file);

    if (readCount != (size_t)fileSize)
    {
        free(buffer);
        fprintf(stderr, "Error: failed to read '%s'\n", path);
        return nullptr;
    }

    if (outSize != nullptr)
    {
        *outSize = (size_t)fileSize;
    }

    return buffer;
}

static RuntimeArgs parseRuntimeArgs(int argc, char *argv[])
{
    RuntimeArgs args = {};
    args.inputFile = nullptr;
    args.verbose = false;
    args.scriptArgc = 0;
    args.scriptArgv = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0)
        {
            args.verbose = true;
        }
        else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            printRuntimeHelp(argv[0]);
            exit(0);
        }
        else
        {
            args.inputFile = arg;
            args.scriptArgv = &argv[i];
            args.scriptArgc = argc - i;
            break;
        }
    }

    return args;
}

int main(int argc, char *argv[])
{
    RuntimeArgs args = parseRuntimeArgs(argc, argv);
    if (args.inputFile == nullptr)
    {
        printRuntimeHelp(argv[0]);
        return 1;
    }

    const char *moduleName = strrchr(args.inputFile, '/');
    const char *moduleNameWin = strrchr(args.inputFile, '\\');
    moduleName = (moduleName != nullptr && (moduleNameWin == nullptr || moduleName > moduleNameWin))
        ? moduleName + 1
        : (moduleNameWin != nullptr ? moduleNameWin + 1 : args.inputFile);

    const char *dot = strrchr(moduleName, '.');
    size_t baseNameLength = dot != nullptr ? (size_t)(dot - moduleName) : strlen(moduleName);
    char *moduleNameCopy = (char *)malloc(baseNameLength + 1);
    if (moduleNameCopy == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        return 1;
    }
    memcpy(moduleNameCopy, moduleName, baseNameLength);
    moduleNameCopy[baseNameLength] = '\0';

    if (args.verbose)
    {
        fprintf(stderr, "DEBUG: Loading bytecode from: %s\n", args.inputFile);
    }

    size_t bytecodeSize = 0;
    unsigned char *bytecode = readBinaryFile(args.inputFile, &bytecodeSize);
    if (bytecode == nullptr)
    {
        free(moduleNameCopy);
        return 1;
    }

    if (args.verbose)
    {
        fprintf(stderr, "DEBUG: Bytecode loaded successfully (%zu bytes)\n", bytecodeSize);
    }

    void *state = createState();
    if (state == nullptr)
    {
        fprintf(stderr, "Error: failed to create VM state\n");
        free(bytecode);
        free(moduleNameCopy);
        return 1;
    }

    initStdLib(state);

#if defined(_WIN32)
    const char *ffiPaths[] = { "phasornative", "plugins" };
#elif defined(__APPLE__)
    const char *ffiPaths[] = { "phasornative", "/Library/Application Support/org.Phasor.Phasor/plugins" };
#else
    const char *ffiPaths[] = { "phasornative", "/usr/lib/phasor/plugins/" };
#endif
    initFFI(state, ffiPaths, sizeof(ffiPaths) / sizeof(ffiPaths[0]));

    if (args.verbose)
    {
        fprintf(stderr, "DEBUG: About to run bytecode\n");
    }

    int status = exec(state, bytecode, bytecodeSize, moduleNameCopy, args.scriptArgc,
                       (const char **)args.scriptArgv);

    if (args.verbose)
    {
        fprintf(stderr, "DEBUG: Bytecode execution complete with return %d\n", status);
    }

    if (isErrorStatus(state))
    {
        fprintf(stderr, "Error: '%s' exited with an unhandled VM error\n", args.inputFile);
        if (status == 0)
        {
            status = -1;
        }
    }

    freeState(state);
    free(bytecode);
    free(moduleNameCopy);

    return status;
}
