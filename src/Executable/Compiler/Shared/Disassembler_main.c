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

typedef struct _disassembler_args
{
    const char *inputFile;
    const char *outputFile;
    bool outputFileOwned;
    bool showHelp;
    bool silent;
} DisassemblerArgs;

static void printDisassemblerHelp(const char *programName)
{
    const char *name = programName ? programName : "phasordisasm";

    const char *slash = strrchr(name, '/');
    const char *backslash = strrchr(name, '\\');
    const char *base = (slash != nullptr && (backslash == nullptr || slash > backslash))
        ? slash + 1
        : (backslash != nullptr ? backslash + 1 : name);

    const char *dot = strrchr(base, '.');
    int stemLength = (dot != nullptr && dot > base) ? (int)(dot - base) : (int)strlen(base);

    const char *version = getVersion();
    fprintf(stdout,
            "Phasor Disassembler v%s\n"
            "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
            "Usage:\n"
            "  %.*s [options] <input.phsb>\n"
            "Options:\n"
            "  -o, --output <file>   Output file\n"
            "  -h, --help            Show this help message\n"
            "  -s, --silent          Do not print anything except errors (no stdout)\n",
            version, stemLength, base);
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

static void parseDisassemblerArgs(int argc, char *argv[], DisassemblerArgs *args)
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

static bool decompileBinary(DisassemblerArgs *args)
{
    if (args->outputFile == nullptr)
    {
        args->outputFile = withExtension(args->inputFile, ".phir");
        args->outputFileOwned = true;
    }

    size_t bcSize = 0;
    unsigned char *bcBuffer = readBinaryFile(args->inputFile, &bcSize);
    if (bcBuffer == nullptr)
    {
        return false;
    }

    size_t requiredSize = 0;
    bool ok = disassembleToIR(bcBuffer, bcSize, nullptr, 0, &requiredSize);
    if (!ok)
    {
        free(bcBuffer);
        return false;
    }

    unsigned char *irBuffer = (unsigned char *)malloc(requiredSize == 0 ? 1 : requiredSize);
    if (irBuffer == nullptr)
    {
        fprintf(stderr, "Error: out of memory\n");
        free(bcBuffer);
        return false;
    }

    size_t actualSize = 0;
    ok = disassembleToIR(bcBuffer, bcSize, irBuffer, requiredSize, &actualSize);
    free(bcBuffer);

    if (!ok)
    {
        free(irBuffer);
        return false;
    }

    bool written = writeBinaryFile(args->outputFile, irBuffer, actualSize);
    free(irBuffer);
    return written;
}

int main(int argc, char *argv[])
{
    DisassemblerArgs args;
    parseDisassemblerArgs(argc, argv, &args);

    if (args.showHelp)
    {
        printDisassemblerHelp(argv[0]);
        return 0;
    }

    if (args.inputFile == nullptr)
    {
        args.inputFile = "";
    }

    int status = 0;
    if (decompileBinary(&args))
    {
        if (!args.silent)
        {
            fprintf(stdout, "Success! Output to %s\n", args.outputFile);
        }
        status = 0;
    }
    else
    {
        fprintf(stderr, "Failed to disassemble program!\n");
        status = 1;
    }

    if (args.outputFileOwned)
    {
        free((void *)args.outputFile);
    }

    return status;
}
