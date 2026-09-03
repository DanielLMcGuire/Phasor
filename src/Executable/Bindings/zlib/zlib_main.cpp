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

#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>
#include <zlib.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <vector>
#include <stdexcept>

namespace {

constexpr size_t kChunkSize = 262144;

[[noreturn]] void throwf(const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    throw std::runtime_error(buf);
}

int64_t plainFileSize(const char *path)
{
    FILE *f = nullptr;

#ifdef _WIN32
    if (fopen_s(&f, path, "rb") != 0 || !f) [[unlikely]] {
        return -1;
    }
#else
    f = fopen(path, "rb");
    if (!f) [[unlikely]] {
        return -1;
    }
#endif

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    return (int64_t)size;
}

} // namespace

PhasorValue phasor_zlib_compress_file(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 2) [[unlikely]] {
        throw std::runtime_error("ZLIB_CompressFile requires at least 2 arguments: src_path (string), dst_path (string), [level (int, 0-9, default 6)]");
    }

    if (!phasor_is_string(argv[0]) || !phasor_is_string(argv[1])) [[unlikely]] {
        throw std::runtime_error("ZLIB_CompressFile: arguments 1 and 2 (src_path, dst_path) must be strings");
    }

    int level = Z_DEFAULT_COMPRESSION;

    if (argc >= 3) {
        if (!phasor_is_int(argv[2])) [[unlikely]] {
            throw std::runtime_error("ZLIB_CompressFile: argument 3 (level) must be an integer");
        }

        level = (int)phasor_to_int(argv[2]);

        if (level < 0 || level > 9) [[unlikely]] {
            throw std::runtime_error("ZLIB_CompressFile: level must be between 0 and 9");
        }
    }

    const char *srcPath = phasor_to_string(argv[0]);
    const char *dstPath = phasor_to_string(argv[1]);

    FILE *in = nullptr;

#ifdef _WIN32
    if (fopen_s(&in, srcPath, "rb") != 0 || !in) [[unlikely]] {
        throwf("ZLIB_CompressFile: failed to open source file '%s'", srcPath);
    }
#else
    in = fopen(srcPath, "rb");
    if (!in) [[unlikely]] {
        throwf("ZLIB_CompressFile: failed to open source file '%s'", srcPath);
    }
#endif

    char mode[8];
    snprintf(mode, sizeof(mode), "wb%d", level);

    gzFile out = gzopen(dstPath, mode);
    if (!out) [[unlikely]] {
        fclose(in);
        throwf("ZLIB_CompressFile: failed to open destination file '%s' for writing", dstPath);
    }

    std::vector<char> buf(kChunkSize);
    int64_t totalIn = 0;
    size_t nread;

    while ((nread = fread(buf.data(), 1, buf.size(), in)) > 0) {
        totalIn += (int64_t)nread;

        int written = gzwrite(out, buf.data(), (unsigned)nread);
        if (written == 0) {
            int errnum = Z_OK;
            const char *msg = gzerror(out, &errnum);

            char errbuf[256];
            snprintf(
                errbuf,
                sizeof(errbuf),
                "ZLIB_CompressFile: compression write failed: %s",
                msg ? msg : "unknown error"
            );

            fclose(in);
            gzclose(out);
            throw std::runtime_error(errbuf);
        }
    }

    bool readFailed = ferror(in) != 0;
    fclose(in);

    int closeResult = gzclose(out);

    if (readFailed) [[unlikely]] {
        throwf("ZLIB_CompressFile: error reading source file '%s'", srcPath);
    }

    if (closeResult != Z_OK) [[unlikely]] {
        throwf(
            "ZLIB_CompressFile: error finalizing compressed file '%s' (zlib error %d)",
            dstPath,
            closeResult
        );
    }

    int64_t totalOut = plainFileSize(dstPath);

    static const char *keys[] = { "bytes_in", "bytes_out", "ratio" };
    PhasorValue values[3];

    values[0] = phasor_make_int(totalIn);
    values[1] = phasor_make_int(totalOut);
    values[2] = phasor_make_float(
        totalIn > 0
            ? (double)totalOut / (double)totalIn
            : 0.0
    );

    return phasor_make_struct("ZlibCompressResult", keys, values, 3);
}

PhasorValue phasor_zlib_decompress_file(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 2) [[unlikely]] {
        throw std::runtime_error("ZLIB_DecompressFile requires 2 arguments: src_path (string), dst_path (string)");
    }

    if (!phasor_is_string(argv[0]) || !phasor_is_string(argv[1])) [[unlikely]] {
        throw std::runtime_error("ZLIB_DecompressFile: both arguments must be strings");
    }

    const char *srcPath = phasor_to_string(argv[0]);
    const char *dstPath = phasor_to_string(argv[1]);

    gzFile in = gzopen(srcPath, "rb");
    if (!in) [[unlikely]] {
        throwf("ZLIB_DecompressFile: failed to open source file '%s'", srcPath);
    }

    FILE *out = nullptr;

#ifdef _WIN32
    if (fopen_s(&out, dstPath, "wb") != 0 || !out) [[unlikely]] {
        gzclose(in);
        throwf("ZLIB_DecompressFile: failed to open destination file '%s' for writing", dstPath);
    }
#else
    out = fopen(dstPath, "wb");
    if (!out) [[unlikely]] {
        gzclose(in);
        throwf("ZLIB_DecompressFile: failed to open destination file '%s' for writing", dstPath);
    }
#endif

    std::vector<char> buf(kChunkSize);
    int64_t totalOut = 0;
    int nread;

    while ((nread = gzread(in, buf.data(), (unsigned)buf.size())) > 0) {
        size_t written = fwrite(buf.data(), 1, (size_t)nread, out);

        if (written != (size_t)nread) [[unlikely]] {
            fclose(out);
            gzclose(in);
            throwf("ZLIB_DecompressFile: failed writing to destination file '%s'", dstPath);
        }

        totalOut += (int64_t)written;
    }

    if (nread < 0) [[unlikely]] {
        int errnum = Z_OK;
        const char *msg = gzerror(in, &errnum);

        char errbuf[256];
        snprintf(
            errbuf,
            sizeof(errbuf),
            "ZLIB_DecompressFile: decompression failed: %s",
            msg ? msg : "unknown error"
        );

        fclose(out);
        gzclose(in);
        throw std::runtime_error(errbuf);
    }

    fclose(out);

    int closeResult = gzclose(in);

    if (closeResult != Z_OK) [[unlikely]] {
        throwf(
            "ZLIB_DecompressFile: error finalizing read of '%s' (zlib error %d)",
            srcPath,
            closeResult
        );
    }

    int64_t totalIn = plainFileSize(srcPath);

    static const char *keys[] = { "bytes_in", "bytes_out" };
    PhasorValue values[2];

    values[0] = phasor_make_int(totalIn);
    values[1] = phasor_make_int(totalOut);

    return phasor_make_struct("ZlibDecompressResult", keys, values, 2);
}

PhasorValue phasor_zlib_is_gzip_file(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) [[unlikely]] {
        throw std::runtime_error("ZLIB_IsGzipFile requires 1 argument: path (string)");
    }

    if (!phasor_is_string(argv[0])) [[unlikely]] {
        throw std::runtime_error("ZLIB_IsGzipFile: argument 1 (path) must be a string");
    }

    const char *path = phasor_to_string(argv[0]);

    FILE *f = nullptr;

#ifdef _WIN32
    if (fopen_s(&f, path, "rb") != 0 || !f) [[unlikely]] {
        throwf("ZLIB_IsGzipFile: failed to open file '%s'", path);
    }
#else
    f = fopen(path, "rb");
    if (!f) [[unlikely]] {
        throwf("ZLIB_IsGzipFile: failed to open file '%s'", path);
    }
#endif

    unsigned char magic[2] = { 0, 0 };
    size_t nread = fread(magic, 1, 2, f);

    fclose(f);

    bool isGzip =
        (nread == 2) &&
        magic[0] == 0x1F &&
        magic[1] == 0x8B;

    return phasor_make_bool(isGzip);
}

PhasorValue phasor_zlib_version(PhasorVM*, int, const PhasorValue*)
{
    return phasor_make_string(zlibVersion());
}

PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm)
{
    // struct ZlibCompressResult { i64 bytes_in, i64 bytes_out, f64 ratio }
    api->register_function(vm, "ZLIB_CompressFile", phasor_zlib_compress_file); // fn ZLIBCompressFile(src_path: string, dst_path: string, level: int = 6) -> ZlibCompressResult;
    api->register_function(vm, "ZLIB_DecompressFile", phasor_zlib_decompress_file); // fn ZLIBDecompressFile(src_path: string, dst_path: string) -> ZlibDecompressResult;
    api->register_function(vm, "ZLIB_IsGzipFile", phasor_zlib_is_gzip_file); // fn ZLIBIsGzipFile(path: string) -> bool;
    api->register_function(vm, "ZLIB_Version", phasor_zlib_version); // fn ZLIBVersion() -> string;
}