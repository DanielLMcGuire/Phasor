# Phasor ![Phasor Logo (Hand-drawn sinewave)](https://phasor.pages.dev/assets/logo-small.webp) Language

A dynamically typed, compiled programming language with a hybrid stack/register-based bytecode virtual machine, *designed* for performance and flexibility.

## Language Features

- **Dynamic typing** with integers, floats, strings, booleans, and of the least for last: null.
- **Functions** with optional type annotations
- **Structs** with static field access, with the benefit of being mostly untested!
- **Control flow**: if/else, while, for, switch/case, break/continue
- **Comprehensive standard library**: without anything smart to put here for appeal
- **Hybrid VM** supporting both stack-based and primarily register-based execution

## Standard Library Modules

| Module      | Include Statement    | Functions                                                             |
| ----------- | -------------------- | --------------------------------------------------------------------- |
| **I/O**     | `include_stdio()`    | `puts`, `puts_error` `printf`, `gets`, `putf`, `msgbox`, `msgbox_err` |
| **Math**    | `include_stdmath()`  | `math_sqrt`, `math_pow`, `math_sin`, `math_cos`, etc.                 |
| **Strings** | `include_stdstr()`   | `len`, `substr`, `concat`, `to_upper`, `to_lower`                     |
| **Files**   | `include_stdfile()`  | `fread`, `fwrite`, `fexists`, `fcopy`, `fmove`                        |
| **System**  | `include_stdsys()`   | `time`, `sleep`, `sys_os`, `sys_exec`, `clear`                        |
| **Types**   | `include_stdtype()`  | `to_int`, `to_float`, `to_string`, `to_bool`                          |
| **Regex**   | `include_stdregex()` | `regex_match`, `regex_search`, `regex_replace`                        |

---

**Phasor** - Fast, flexible programming/scripting with *near* native VM performance.

This project is licensed under the following agreements, which may be updated at any time, with or without notice, upon a new non-patch release of the language.

- Phasor Language / VM / Toolchain | [Phasor Restrictive License 1.2](https://phasor.pages.dev/document?file=content%2Flegal%2Fphasorprl.txt&name=Phasor%20Restrictive%20License%201.2)
- Phasor Standard Library | [Phasor Open License 1.0](https://phasor.pages.dev/document?file=content%2Flegal%2Fphasorpol.txt&name=Phasor%20Open%20License%201.0)
- Phasor Shell | [MIT License](https://opensource.org/license/mit)
- Phasor coreutils implementation | [GNU General Public License 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)
- Phasor binary releases | [Phasor Binary EULA](https://phasor.pages.dev/document?file=content%2Flegal%2Fphasorbineula.txt&name=Phasor%20Binary%20EULA)

Mentions of 'coreutils', the Free Software Foundation, Inc., 'Java™', Oracle® Corporation, '.NET™', Microsoft® Corporation, Google® LLC, or other third-party companies, products, or trademarks do not imply any affiliation, endorsement, or sponsorship by those third parties, or thier affiliates, unless explicitly stated otherwise.
