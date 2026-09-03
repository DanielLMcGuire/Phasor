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

#include <PhasorRT.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

typedef struct _assembler_args
{
    const char *inputFile;
    const char *outputFile;
    bool outputFileOwned;
    bool showHelp;
    bool silent;
} AssemblerArgs;

static void printAssemblerHelp(const char *programName)
{
    const char *name = programName ? programName : "phasorasm";

    const char *slash = strrchr(name, '/');
    const char *backslash = strrchr(name, '\\');
    const char *base = (slash != nullptr && (backslash == nullptr || slash > backslash))
        ? slash + 1
        : (backslash != nullptr ? backslash + 1 : name);

    const char *dot = strrchr(base, '.');
    int stemLength = (dot != nullptr && dot > base) ? (int)(dot - base) : (int)strlen(base);

    const char *version = getVersion();
    fprintf(stdout,
            "Phasor Assembler v%s\n"
            "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
            "Usage:\n"
            "  %.*s [options] <input.phir>\n\n"
            "Options:\n"
            "  -o, --output <file>   Output file\n"
            "  -h, --help            Show this help message\n"
            "  -s, --silent          Do not print anything except errors (no stdout)\n",
            version, stemLength, base);
}

static void printFileOpenError(const char *path, bool writing)
{
    char errorBuffer[256];
#if defined(_WIN32)
    errno_t err = strerror_s(errorBuffer, sizeof(errorBuffer), errno);
    if (err != 0)
    {
        snprintf(errorBuffer, sizeof(errorBuffer), "unknown error");
    }
#else
    const char *errorMessage = strerror(errno);
    if (errorMessage == nullptr)
    {
        errorMessage = "unknown error";
    }
    snprintf(errorBuffer, sizeof(errorBuffer), "%s", errorMessage);
#endif

    fprintf(stderr, "Error: could not open '%s'%s: %s\n", path, writing ? " for writing" : "", errorBuffer);
}

static unsigned char *readBinaryFile(const char *path, size_t *outSize)
{
    FILE *file = nullptr;
#if defined(_WIN32)
    if (fopen_s(&file, path, "rb") != 0)
#else
    file = fopen(path, "rb");
    if (file == nullptr)
#endif
    {
        printFileOpenError(path, false);
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

static bool writeBinaryFile(const char *path, const unsigned char *buffer, size_t size)
{
    FILE *file = nullptr;
#if defined(_WIN32)
    if (fopen_s(&file, path, "wb") != 0)
#else
    file = fopen(path, "wb");
    if (file == nullptr)
#endif
    {
        printFileOpenError(path, true);
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

static char *withExtension(const char *inputFile, const char *suffix)
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

static void parseAssemblerArgs(int argc, char *argv[], AssemblerArgs *args)
{
    args->inputFile = nullptr;
    args->outputFile = nullptr;
    args->outputFileOwned = false;
    args->showHelp = false;
    args->silent = false;

    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            args->showHelp = true;
            return;
        }
        else if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0)
        {
            if (i + 1 < argc)
            {
                args->outputFile = argv[++i];
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg);
                args->showHelp = true;
                return;
            }
        }
        else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--silent") == 0)
        {
            args->silent = true;
        }
        else if (arg[0] == '-')
        {
            fprintf(stderr, "Error: Unknown option: %s\n", arg);
            args->showHelp = true;
            return;
        }
        else
        {
            if (args->inputFile == nullptr)
            {
                args->inputFile = arg;
            }
            else
            {
                fprintf(stderr, "Error: Multiple input files specified\n");
                args->showHelp = true;
                return;
            }
        }
    }
}

static bool assembleBinary(AssemblerArgs *args)
{
    if (args->outputFile == nullptr)
    {
        args->outputFile = withExtension(args->inputFile, ".phsb");
        args->outputFileOwned = true;
    }

    size_t irSize = 0;
    unsigned char *irBuffer = readBinaryFile(args->inputFile, &irSize);
    if (irBuffer == nullptr)
    {
        return false;
    }

    size_t requiredSize = 0;
    bool ok = assembleIR(irBuffer, irSize, nullptr, 0, &requiredSize);
    if (!ok)
    {
        free(irBuffer);
        return false;
    }

    unsigned char *bytecode = (unsigned char *)malloc(requiredSize == 0 ? 1 : requiredSize);
    if (bytecode == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        free(irBuffer);
        return false;
    }

    size_t actualSize = 0;
    ok = assembleIR(irBuffer, irSize, bytecode, requiredSize, &actualSize);
    free(irBuffer);

    if (!ok)
    {
        free(bytecode);
        return false;
    }

    bool written = writeBinaryFile(args->outputFile, bytecode, actualSize);
    free(bytecode);
    return written;
}

int main(int argc, char *argv[])
{
    AssemblerArgs args;
    parseAssemblerArgs(argc, argv, &args);

    if (args.showHelp)
    {
        printAssemblerHelp(argv[0]);
        return 0;
    }

    if (args.inputFile == nullptr)
    {
        args.inputFile = "";
    }

    int status = 0;
    if (assembleBinary(&args))
    {
        if (!args.silent)
        {
            fprintf(stdout, "Success! Output to %s\n", args.outputFile);
        }
        status = 0;
    }
    else
    {
        fprintf(stdout, "Failed to assemble program!\n");
        status = 1;
    }

    if (args.outputFileOwned)
    {
        free((void *)args.outputFile);
    }

    return status;
}
