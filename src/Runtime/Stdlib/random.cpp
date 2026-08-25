#include "StdLib.hpp"
#include "core/random.hpp"
#include <cstdint>
#include <cstddef>
#include <climits>
#include <stdexcept>
#include <cstring>
#include <cmath>

#if defined(_WIN32)
    #define WIN32_NO_STATUS
    #include <windows.h>
    #undef WIN32_NO_STATUS
    #include <ntstatus.h>
    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__)
    #include <Security/SecRandom.h>
#elif (defined(__linux__) || defined(__gnu_linux__)) && \
      !(defined(__ANDROID_API__) && __ANDROID_API__ < 28)
    #include <sys/random.h>
    #include <unistd.h>
    #include <cerrno>
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <cerrno>
#endif

namespace Phasor
{

namespace CryptoRand {

    inline void get_bytes(uint8_t* buffer, size_t size)
    {
        if (size == 0)
        { 
            return;
        }
        if (buffer == nullptr)
        {
            throw std::invalid_argument("Crypto::get_bytes: null buffer with nonzero size.");
        }

    #if defined(_WIN32)
        size_t offset = 0;
        while (offset < size)
        {
            size_t chunk = size - offset;
            if (chunk > ULONG_MAX)
            { 
                chunk = ULONG_MAX;
            }

            NTSTATUS status = BCryptGenRandom(
                nullptr,
                buffer + offset,
                static_cast<ULONG>(chunk),
                BCRYPT_USE_SYSTEM_PREFERRED_RNG
            );
            if (status != STATUS_SUCCESS)
            {
                PHS_ERROR("Windows BCryptGenRandom failed.");
            }
            offset += chunk;
        }
    #elif defined(__APPLE__)
        int status = SecRandomCopyBytes(kSecRandomDefault, size, buffer);
        if (status != 0)
        {
            PHS_ERROR("Apple SecRandomCopyBytes failed.");
        }
    #elif (defined(__linux__) || defined(__gnu_linux__)) && \
          !(defined(__ANDROID_API__) && __ANDROID_API__ < 28)
        size_t bytes_read = 0;
        while (bytes_read < size)
        {
            ssize_t result = getrandom(buffer + bytes_read, size - bytes_read, 0);
            if (result < 0)
            {
                if (errno == EINTR) continue;
                PHS_ERROR("Linux getrandom failed.");
            }
            bytes_read += static_cast<size_t>(result);
        }
    #else
        int fd = ::open("/dev/urandom", O_RDONLY);
        if (fd < 0)
        {
            PHS_ERROR("Failed to open /dev/urandom.");
        }

        size_t bytes_read = 0;
        while (bytes_read < size)
        {
            ssize_t result = ::read(fd, buffer + bytes_read, size - bytes_read);
            if (result < 0)
            {
                if (errno == EINTR) continue;
                ::close(fd);
                PHS_ERROR("Failed to read from /dev/urandom.");
            }
            if (result == 0)
            {
                ::close(fd);
                PHS_ERROR("Unexpected EOF reading from /dev/urandom.");
            }
            bytes_read += static_cast<size_t>(result);
        }
        ::close(fd);
    #endif
    }

} // namespace CryptoRand

void StdLib::registerRandomFunctions(VM *vm)
{
	vm->registerNativeFunction("rand_seed", StdLib::rand_seed);
	vm->registerNativeFunction("rand_next_range", StdLib::rand_next_range);
	vm->registerNativeFunction("rand_next_float", StdLib::rand_next_float);
	vm->registerNativeFunction("rand_crypto_int", StdLib::rand_get_crypto_int);
	vm->registerNativeFunction("rand_crypto_float", StdLib::rand_get_crypto_float);
}

Value StdLib::rand_seed(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "rand_seed");

	if (!args[0].isInt())
		PHS_ERROR("rand_seed() expects an integer as its first argument");
	if (!args[1].isInt())
		PHS_ERROR("rand_seed() expects an integer as its second argument");

	i64 s1 = args[0].asInt();
	i64 s2 = args[1].asInt();

	if (s1 <= 0 || s2 <= 0)
	{
		PHS_ERROR("rand_seed(): Both values must be positive integers");
	}

	PHASORstd_rand_seed(static_cast<u64>(s1), static_cast<u64>(s2));
	return phsnull;
}

i64 StdLib::rand_next_range(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "rand_next_range");

	if (!args[0].isInt())
		PHS_ERROR("rand_next_range() expects an integer as its first argument (min)");
	if (!args[1].isInt())
		PHS_ERROR("rand_next_range() expects an integer as its second argument (max)");

	i64 min = args[0].asInt();
	i64 max = args[1].asInt();

	if (min > max)
	{
		PHS_ERROR("rand_next_range(): min value cannot be greater than max value");
	}

	return PHASORstd_rand_next_range(static_cast<u64>(min), static_cast<u64>(max));
}

f64 StdLib::rand_next_float(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 0, "rand_next_float");
	return PHASORstd_rand_next_double();
}

Value StdLib::rand_get_crypto_int(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 0, "rand_crypto_int");
	i64 val = 0;
	try {
		CryptoRand::get_bytes(reinterpret_cast<uint8_t*>(&val), sizeof(val));
	} catch (...)
    {
		return phsnull;
	}
	return val;
}

Value StdLib::rand_get_crypto_float(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 0, "rand_crypto_float");
	uint64_t  val = 0;
	try {
        CryptoRand::get_bytes(reinterpret_cast<uint8_t*>(&val), sizeof(val));
    } catch (...)
    {
        return phsnull;
    }
	return static_cast<f64>(val & 0x1FFFFFFFFFFFFF) / 9007199254740992.0;
}

} // namespace Phasor
