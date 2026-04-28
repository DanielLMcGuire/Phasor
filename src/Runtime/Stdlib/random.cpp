#include "StdLib.hpp"
#include "core/random.hpp"


namespace Phasor
{

void StdLib::registerRandomFunctions(VM *vm)
{
	vm->registerNativeFunction("rand_seed", StdLib::rand_seed);
    vm->registerNativeFunction("rand_next_range", StdLib::rand_next_range);
    vm->registerNativeFunction("rand_next_float", StdLib::rand_next_float);
}

Value StdLib::rand_seed(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 2, "rand_seed");
    int64_t s1 = args[0].asInt();
    int64_t s2 = args[1].asInt()0;

    if (s1 <= 0 || s2 <= 0) {
        throw std::runtime_error("rand_seed(): Both values must be positive integers");
    }

    PHASORstd_rand_seed(static_cast<uint64_t>(s1), static_cast<uint64_t>(s2));
    return Value();
}

int64_t StdLib::rand_next_range(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 2, "rand_next_range");
    int64_t min = args[0].asInt();
    int64_t max = args[1].asInt();

    if (min > max) {
        throw std::runtime_error("rand_get(): min value cannot be greater than max value");
    }

    return PHASORstd_rand_next_range(static_cast<uint64_t>(min), static_cast<uint64_t>(max));
}

double StdLib::rand_next_float(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 0, "rand_next_float");
    return PHASORstd_rand_next_double();
}

} // namespace Phasor