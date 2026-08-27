#include "ISA.hpp"
#include <unordered_map>
#include <string>
#include <PhasorString.hpp>
#include <stdexcept>

namespace Phasor
{

Phasor::string opCodeToString(OpCode op);
std::string opCodeToStlString(OpCode op);

OpCode stringToOpCode(const Phasor::string &str);
OpCode stringToOpCode(const std::string &str);

} // namespace Phasor