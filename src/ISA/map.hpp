#include "ISA.hpp"
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace Phasor {
    
std::string opCodeToString(OpCode op);

OpCode stringToOpCode(const std::string &str);

} // namespace Phasor