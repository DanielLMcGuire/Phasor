// Copyright 2026 Daniel McGuire
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

typedef struct
{
    const char *inputFile;
    const char *outputFile;
    const char **includePaths;
    int includePathCount;
    const char **defines;
    int defineCount;
    bool verbose;
    bool irMode;
    int scriptArgc;
    char **scriptArgv;
} CompilerArgs;

static void printCompilerHelp(const char *programName)
{
    const char *name = programName ? programName : "phasorc";
    const char *version = getVersion();
    fprintf(stdout,
            "Phasor Compiler v%s\n"
	        "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
            "Usage: %s [options] <file.phs>\n\n"
            "Options:\n"
            "  -o, --output FILE   Specify output file\n"
            "  -i, --ir            Compile to IR format (.phir) instead of bytecode\n"
            "  -I, --include PATHS Comma-separated list of include directories (e.g. -I=..., --include=...)\n"
            "  -D, --define DEFS  Comma-separated list of NAME or NAME=VALUE definitions\n"
            "  -v, --verbose       Enable verbose output\n"
            "  -h, --help          Show this help message\n",
            version, name);
}

static void appendStringEntry(const char ***items, int *count, const char *value)
{
    const char **newItems = (const char **)realloc((void *)*items, sizeof(char *) * (*count + 1));
    if (newItems == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }

    *items = newItems;
    (*items)[*count] = value;
    (*count) += 1;
}

static void parseCommaSeparatedValues(const char *values, const char ***items, int *count)
{
    if (values == nullptr || *values == '\0')
    {
        return;
    }

    size_t valueLength = strlen(values);
    char *copy = (char *)malloc(valueLength + 1);
    if (copy == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }

    memcpy(copy, values, valueLength + 1);
    char *cursor = copy;

    while (*cursor != '\0')
    {
        char *itemStart = cursor;

        while (*cursor != '\0' && *cursor != ',')
        {
            ++cursor;
        }

        if (*cursor == ',')
        {
            *cursor = '\0';
            ++cursor;
        }

        while (*itemStart == ' ' || *itemStart == '\t' || *itemStart == '\r' || *itemStart == '\n')
        {
            ++itemStart;
        }

        char *itemEnd = itemStart + strlen(itemStart);
        while (itemEnd > itemStart && (itemEnd[-1] == ' ' || itemEnd[-1] == '\t' || itemEnd[-1] == '\r' || itemEnd[-1] == '\n'))
        {
            *--itemEnd = '\0';
        }

        if (*itemStart != '\0')
        {
            size_t itemLength = strlen(itemStart);
            char *item = (char *)malloc(itemLength + 1);
            if (item == nullptr)
            {
                free(copy);
                fprintf(stderr, "Error: out of memory\n");
                exit(1);
            }
            memcpy(item, itemStart, itemLength + 1);
            appendStringEntry(items, count, item);
        }
    }

    free(copy);
}

static void freeStringArray(const char **items, int count)
{
    if (items == nullptr)
    {
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        if (items[i] != nullptr)
        {
            free((void *)items[i]);
        }
    }

    free((void *)items);
}

static char *duplicateString(const char *value)
{
    if (value == nullptr)
    {
        return nullptr;
    }

    size_t length = strlen(value);
    char *copy = (char *)malloc(length + 1);
    if (copy == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }

    memcpy(copy, value, length + 1);
    return copy;
}

static char *readTextFile(const char *path, size_t *outSize)
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

    char *buffer = (char *)malloc((size_t)fileSize + 1);
    if (buffer == nullptr)
    {
        fclose(file);
        fprintf(stderr, "Error: out of memory\n");
        return nullptr;
    }

    size_t readCount = fread(buffer, 1, (size_t)fileSize, file);
    fclose(file);
    if (readCount != (size_t)fileSize)
    {
        free(buffer);
        fprintf(stderr, "Error: failed to read '%s'\n", path);
        return nullptr;
    }

    buffer[fileSize] = '\0';
    if (outSize != nullptr)
    {
        *outSize = (size_t)fileSize;
    }

    return buffer;
}

static bool writeBinaryFile(const char *path, const unsigned char *buffer, size_t size)
{
    FILE *file = fopen(path, "wb");
    if (file == nullptr)
    {
        fprintf(stderr, "Error: could not open '%s' for writing: %s\n", path, strerror(errno));
        return false;
    }

    size_t written = 0;
    if (size > 0)
    {
        written = fwrite(buffer, 1, size, file);
    }
    fclose(file);

    if (written != size)
    {
        fprintf(stderr, "Error: failed to write '%s'\n", path);
        return false;
    }

    return true;
}

static char *defaultOutputPath(const char *inputFile, bool irMode)
{
    const char *dot = strrchr(inputFile, '.');
    const char *slash = strrchr(inputFile, '/');
    const char *backslash = strrchr(inputFile, '\\');

    const char *base = inputFile;

    if (slash != nullptr && (backslash == nullptr || slash > backslash))
    {
        base = slash;
    }
    else if (backslash != nullptr)
    {
        base = backslash;
    }

    const char *nameStart = base == inputFile ? inputFile : base + 1;

    size_t stemLength = strlen(nameStart);
    if (dot != nullptr && dot > nameStart)
    {
        stemLength = (size_t)(dot - nameStart);
    }

    const char *suffix = irMode ? ".phir" : ".phsb";

    size_t directoryLength = base == inputFile ? 0 : (size_t)(base - inputFile + 1);
    size_t resultSize = directoryLength + stemLength + strlen(suffix) + 1;

    char *result = (char *)malloc(resultSize);
    if (result == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }

    size_t offset = 0;

    if (directoryLength > 0)
    {
        memcpy(result + offset, inputFile, directoryLength);
        offset += directoryLength;
    }

    memcpy(result + offset, nameStart, stemLength);
    offset += stemLength;

    memcpy(result + offset, suffix, strlen(suffix));
    offset += strlen(suffix);

    result[offset] = '\0';

    return result;
}

CompilerArgs parseCompilerArgs(int argc, char *argv[])
{
    CompilerArgs args = {};
    args.inputFile = nullptr;
    args.outputFile = nullptr;
    args.includePaths = nullptr;
    args.includePathCount = 0;
    args.defines = nullptr;
    args.defineCount = 0;
    args.verbose = false;
    args.irMode = false;
    args.scriptArgc = 0;
    args.scriptArgv = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0)
        {
            if (i + 1 < argc)
            {
                args.outputFile = argv[++i];
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg);
                exit(1);
            }
        }
        else if (strncmp(arg, "-o=", 3) == 0 || strncmp(arg, "--output=", 9) == 0)
        {
            args.outputFile = strchr(arg, '=') + 1;
        }
        else if (strcmp(arg, "-I") == 0 || strcmp(arg, "--include") == 0)
        {
            if (i + 1 < argc)
            {
                parseCommaSeparatedValues(argv[++i], &args.includePaths, &args.includePathCount);
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg);
                exit(1);
            }
        }
        else if (strncmp(arg, "-I=", 3) == 0 || strncmp(arg, "--include=", 10) == 0)
        {
            parseCommaSeparatedValues(strchr(arg, '=') + 1, &args.includePaths, &args.includePathCount);
        }
        else if (strcmp(arg, "-D") == 0 || strcmp(arg, "--define") == 0)
        {
            if (i + 1 < argc)
            {
                parseCommaSeparatedValues(argv[++i], &args.defines, &args.defineCount);
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg);
                exit(1);
            }
        }
        else if (strncmp(arg, "-D=", 3) == 0 || strncmp(arg, "--define=", 10) == 0)
        {
            parseCommaSeparatedValues(strchr(arg, '=') + 1, &args.defines, &args.defineCount);
        }
        else if (strcmp(arg, "-i") == 0 || strcmp(arg, "--ir") == 0)
        {
            args.irMode = true;
        }
        else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0)
        {
            args.verbose = true;
        }
        else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            printCompilerHelp(argv[0]);
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
    CompilerArgs args = parseCompilerArgs(argc, argv);
    if (args.inputFile == nullptr)
    {
        printCompilerHelp(argv[0]);
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

    const char *pathSep = strrchr(args.inputFile, '/');
    const char *pathSepWin = strrchr(args.inputFile, '\\');
    char *modulePath = nullptr;
    if (pathSep != nullptr || pathSepWin != nullptr)
    {
        size_t directoryLength = ((pathSep != nullptr && (pathSepWin == nullptr || pathSep > pathSepWin))
                                     ? (size_t)(pathSep - args.inputFile)
                                     : (pathSepWin != nullptr ? (size_t)(pathSepWin - args.inputFile) : 0));
        char *modulePathCopy = (char *)malloc(directoryLength + 1);
        if (modulePathCopy == nullptr)
        {
            free(moduleNameCopy);
            fprintf(stderr, "Error: out of memory\n");
            return 1;
        }
        memcpy(modulePathCopy, args.inputFile, directoryLength);
        modulePathCopy[directoryLength] = '\0';
        modulePath = modulePathCopy;
    }

    size_t sourceSize = 0;
    char *source = readTextFile(args.inputFile, &sourceSize);
    if (source == nullptr)
    {
        free(moduleNameCopy);
        if (modulePath != nullptr)
        {
            free(modulePath);
        }
        return 1;
    }

    char *outputPath = args.outputFile != nullptr ? duplicateString(args.outputFile) : defaultOutputPath(args.inputFile, args.irMode);
    size_t requiredSize = 0;
    bool ok = false;
    unsigned char *buffer = nullptr;

    const char *modulePathArg = modulePath != nullptr ? modulePath : ".";

    if (args.irMode)
    {
        ok = compilePHSToIR(source, moduleNameCopy, modulePathArg, args.includePaths, args.includePathCount,
                           args.defines, args.defineCount, nullptr, 0, &requiredSize);
    }
    else
    {
        ok = compilePHS(source, moduleNameCopy, modulePathArg, args.includePaths, args.includePathCount,
                        args.defines, args.defineCount, nullptr, 0, &requiredSize);
    }

    if (!ok)
    {
        fprintf(stderr, "Error: failed to compile '%s'\n", args.inputFile);
        free(source);
        free(moduleNameCopy);
        if (modulePath != nullptr)
        {
            free(modulePath);
        }
        free(outputPath);
        return 1;
    }

    buffer = (unsigned char *)malloc(requiredSize == 0 ? 1 : requiredSize);
    if (buffer == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        free(source);
        free(moduleNameCopy);
        if (modulePath != nullptr)
        {
            free(modulePath);
        }
        free(outputPath);
        return 1;
    }

    size_t actualSize = 0;
    if (args.irMode)
    {
        ok = compilePHSToIR(source, moduleNameCopy, modulePathArg, args.includePaths, args.includePathCount,
                           args.defines, args.defineCount, buffer, requiredSize, &actualSize);
    }
    else
    {
        ok = compilePHS(source, moduleNameCopy, modulePathArg, args.includePaths, args.includePathCount,
                        args.defines, args.defineCount, buffer, requiredSize, &actualSize);
    }

    free(source);
    free(moduleNameCopy);
    if (modulePath != nullptr)
    {
        free(modulePath);
    }

    if (!ok)
    {
        fprintf(stderr, "Error: compilation did not produce output for '%s'\n", args.inputFile);
        free(buffer);
        free(outputPath);
        return 1;
    }

    if (!writeBinaryFile(outputPath, buffer, actualSize))
    {
        free(buffer);
        free(outputPath);
        return 1;
    }

    if (args.verbose)
    {
        fprintf(stdout, "Compiled %s -> %s (%zu bytes)\n", args.inputFile, outputPath, actualSize);
    }

    free(buffer);
    free(outputPath);
    freeStringArray(args.includePaths, args.includePathCount);
    freeStringArray(args.defines, args.defineCount);
    return 0;
}
