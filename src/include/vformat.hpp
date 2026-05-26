#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <climits>
#include <algorithm>
#include <vector>
#include <Value.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

namespace vformat {

namespace detail {

enum class Length { None, hh, h, l, ll, L, z, t, j };

struct Spec {
    bool minus = false;
    bool plus = false;
    bool space = false;
    bool hash = false;
    bool zero = false;
    bool quote = false;

    int width = 0;
    int prec = -1;

    Length length = Length::None;
    char conv = '\0';
};

static const char *parse(const char *f, Spec &s, va_list &ap)
{
    for (bool more = true; more;) {
        switch (*f) {
            case '-':
                s.minus = true;
                ++f;
                break;
            case '+':
                s.plus = true;
                ++f;
                break;
            case ' ':
                s.space = true;
                ++f;
                break;
            case '#':
                s.hash = true;
                ++f;
                break;
            case '0': 
                s.zero = true;
                ++f;
                break;
            case '\'':
                s.quote = true;
                ++f;
                break;
            default: 
                more = false; 
                break;
        }
    }

    if (*f == '*') {
        s.width = va_arg(ap, int);
        if (s.width < 0) { s.minus = true; s.width = -s.width; }
        ++f;
    } else {
        while (*f >= '0' && *f <= '9') { s.width = s.width * 10 + (*f++ - '0'); }
    }

    if (*f == '.') {
        ++f;
        s.prec = 0;
        if (*f == '*') {
            s.prec = va_arg(ap, int);
            if (s.prec < 0) s.prec = -1;
            ++f;
        } else {
            while (*f >= '0' && *f <= '9') { s.prec = s.prec * 10 + (*f++ - '0'); }
        }
    }

    switch (*f) {
        case 'h':
            ++f;
            if (*f == 'h') { s.length = Length::hh; ++f; }
            else s.length = Length::h;
            break;
        case 'l':
            ++f;
            if (*f == 'l') { s.length = Length::ll; ++f; }
            else s.length = Length::l;
            break;
        case 'L': 
            s.length = Length::L;
            ++f; 
            break;
        case 'z':
        case 'Z': 
            s.length = Length::z; 
            ++f; 
            break;
        case 't': 
            s.length = Length::t;
            ++f;
            break;
        case 'j':
            s.length = Length::j;
            ++f; 
            break;
        case 'q':
            s.length = Length::ll;
            ++f;
            break;
        default:
            break;
    }

    s.conv = *f;
    if (*f) ++f;
    return f;
}

static std::string build_fmt(const Spec &s, const char *len_override, char conv)
{
    std::string f = "%";
    if (s.minus) f += '-';
    if (s.plus) f += '+';
    if (s.space) f += ' ';
    if (s.hash) f += '#';
    if (s.zero) f += '0';
    if (s.quote) f += '\'';
    if (s.width > 0) f += std::to_string(s.width);
    if (s.prec  >= 0) { f += '.'; f += std::to_string(s.prec); }
    f += len_override;
    f += conv;
    return f;
}

template<typename Fill>
static std::string snprintf_into(int hint, Fill fill)
{
    int sz = std::max(hint, 64);
    std::vector<char> buf(sz);
    int needed = fill(buf.data(), sz);
    if (needed < 0) return {};
    if (needed >= sz) {
        buf.resize(needed + 1);
        fill(buf.data(), needed + 1);
    }
    return std::string(buf.data());
}

static int hint(const Spec &s) {
    return std::max({ s.width, s.prec, 0 }) + 80;
}

static std::string fmt_signed(const Spec &s, va_list &ap)
{
    long long val;
    switch (s.length) {
        case Length::hh:
            val = (signed char)va_arg(ap, int);
            break;
        case Length::h:
            val = (short)va_arg(ap, int);
            break;
        case Length::l:
            val = va_arg(ap, long);
            break;
        case Length::ll:
            val = va_arg(ap, long long);
            break;
        case Length::z:
            val = (long long)va_arg(ap, std::ptrdiff_t);
            break;
        case Length::t:
            val = va_arg(ap, std::ptrdiff_t);
            break;
        case Length::j:
            val = (long long)va_arg(ap, std::intmax_t);
            break;
        default:
            val = va_arg(ap, int);
            break;
    }
    std::string f = build_fmt(s, "ll", s.conv);
    return snprintf_into(hint(s), [&](char *b, int n){
        return std::snprintf(b, n, f.c_str(), val);
    });
}

static std::string fmt_unsigned(const Spec &s, va_list &ap)
{
    unsigned long long val;
    switch (s.length) {
        case Length::hh: 
            val = (unsigned char)va_arg(ap, unsigned int);
            break;
        case Length::h:
            val = (unsigned short)va_arg(ap, unsigned int);
            break;
        case Length::l:
            val = va_arg(ap, unsigned long);
            break;
        case Length::ll: 
            val = va_arg(ap, unsigned long long);
            break;
        case Length::z:
            val = (unsigned long long) va_arg(ap, std::size_t);
            break;
        case Length::t:
            val = (unsigned long long)(std::ptrdiff_t)va_arg(ap, std::ptrdiff_t);
            break;
        case Length::j:
            val = (unsigned long long) va_arg(ap, std::uintmax_t);
            break;
        default:
            val = va_arg(ap, unsigned int);
            break;
    }
    std::string f = build_fmt(s, "ll", s.conv);
    return snprintf_into(hint(s), [&](char *b, int n){
        return std::snprintf(b, n, f.c_str(), val);
    });
}

static std::string fmt_float(const Spec &s, va_list &ap)
{
    if (s.length == Length::L) {
        long double val = va_arg(ap, long double);
        std::string f = build_fmt(s, "L", s.conv);
        return snprintf_into(hint(s), [&](char *b, int n){
            return std::snprintf(b, n, f.c_str(), val);
        });
    } else {
        double val = va_arg(ap, double);
        std::string f = build_fmt(s, "", s.conv);
        return snprintf_into(hint(s), [&](char *b, int n){
            return std::snprintf(b, n, f.c_str(), val);
        });
    }
}

static std::string fmt_char(const Spec &s, va_list &ap)
{
    char c = (char)va_arg(ap, int);
    int pad = s.width - 1;
    std::string out;
    if (!s.minus && pad > 0) out.append(pad, ' ');
    out += c;
    if ( s.minus && pad > 0) out.append(pad, ' ');
    return out;
}

static std::string fmt_string(const Spec &s, va_list &ap)
{
    const char *str = va_arg(ap, const char *);
    if (!str) str = "(null)";

    std::size_t len = (s.prec >= 0)
                    ? ::strnlen(str, (std::size_t)s.prec)
                    : ::strlen(str);

    int pad = s.width - (int)len;
    std::string out;
    out.reserve(len + std::max(pad, 0));
    if (!s.minus && pad > 0) out.append(pad, ' ');
    out.append(str, len);
    if ( s.minus && pad > 0) out.append(pad, ' ');
    return out;
}

static std::string fmt_pointer(const Spec &s, va_list &ap)
{
    void *val = va_arg(ap, void *);
    std::string f = "%";
    if (s.minus) f += '-';
    if (s.width > 0) f += std::to_string(s.width);
    f += 'p';
    return snprintf_into(hint(s), [&](char *b, int n){
        return std::snprintf(b, n, f.c_str(), val);
    });
}

static std::string do_vformat(const char *fmt, va_list ap_in)
{
    va_list ap;
    va_copy(ap, ap_in);

    std::string result;
    result.reserve(128);

    const char *f = fmt;
    while (*f) {
        if (*f != '%') {
            result += *f++;
            continue;
        }

        ++f;
        
        if (*f == '%') {
            result += '%';
            ++f;
            continue;
        }

        Spec s;
        f = parse(f, s, ap);

        switch (s.conv) {
            case 'd':
            case 'i':
                result += fmt_signed(s, ap);
                break;

            case 'u':
            case 'o':
            case 'x':
            case 'X':
                result += fmt_unsigned(s, ap);
                break;

            case 'f': case 'F':
            case 'e': case 'E':
            case 'g': case 'G':
            case 'a': case 'A':
                result += fmt_float(s, ap);
                break;

            case 'c':
                result += fmt_char(s, ap);
                break;

            case 's':
                result += fmt_string(s, ap);
                break;

            case 'p':
                result += fmt_pointer(s, ap);
                break;

            case 'm':
                result += ::strerror(errno);
                break;

            case 'n': {
                (void)va_arg(ap, int *);
                break;
            }

            default:
                result += '%';
                if (s.conv) result += s.conv;
                break;
        }
    }

    va_end(ap);
    return result;
}

} // namespace detail

inline std::string vformat(const char *fmt, va_list ap)
{
    return detail::do_vformat(fmt, ap);
}

inline std::string format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string result = detail::do_vformat(fmt, ap);
    va_end(ap);
    return result;
}

inline int vfprintf(std::ostream &os, const char *fmt, va_list ap)
{
    std::string s = detail::do_vformat(fmt, ap);
    os.write(s.data(), (std::streamsize)s.size());
    if (!os) return -1;
    return (int)s.size();
}

inline int fprintf(std::ostream &os, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(os, fmt, ap);
    va_end(ap);
    return r;
}

inline int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(std::cout, fmt, ap);
    va_end(ap);
    return r;
}

inline std::string str_format_v(const char *fmt, const std::vector<Phasor::Value> &args)
{
    std::string result;
    result.reserve(128);

    const char *f = fmt;
    size_t argIndex = 0;

    while (*f) {
        if (*f != '%') {
            result += *f++;
            continue;
        }

        ++f;

        if (*f == '%') {
            result += '%';
            ++f;
            continue;
        }

        detail::Spec s;

        // ---- SAFE MANUAL PARSER (NO va_list) ----

        while (true) {
            switch (*f) {
                case '-': s.minus = true; ++f; continue;
                case '+': s.plus = true; ++f; continue;
                case ' ': s.space = true; ++f; continue;
                case '#': s.hash = true; ++f; continue;
                case '0': s.zero = true; ++f; continue;
                case '\'': s.quote = true; ++f; continue;
                default: break;
            }
            break;
        }

        if (*f == '*') {
            if (argIndex < args.size() && args[argIndex].getType() == Phasor::ValueType::Int)
                s.width = (int)args[argIndex++].asInt();
            ++f;
        } else {
            while (*f >= '0' && *f <= '9')
                s.width = s.width * 10 + (*f++ - '0');
        }

        if (*f == '.') {
            ++f;
            s.prec = 0;

            if (*f == '*') {
                if (argIndex < args.size() && args[argIndex].getType() == Phasor::ValueType::Int)
                    s.prec = (int)args[argIndex++].asInt();
                ++f;
            } else {
                while (*f >= '0' && *f <= '9')
                    s.prec = s.prec * 10 + (*f++ - '0');
            }
        }

        switch (*f) {
            case 'h':
                ++f;
                if (*f == 'h') { s.length = detail::Length::hh; ++f; }
                else s.length = detail::Length::h;
                break;
            case 'l':
                ++f;
                if (*f == 'l') { s.length = detail::Length::ll; ++f; }
                else s.length = detail::Length::l;
                break;
            case 'L': s.length = detail::Length::L; ++f; break;
            case 'z':
            case 'Z': s.length = detail::Length::z; ++f; break;
            case 't': s.length = detail::Length::t; ++f; break;
            case 'j': s.length = detail::Length::j; ++f; break;
            default: break;
        }

        s.conv = *f;
        if (*f) ++f;

        if (argIndex >= args.size()) {
            result += '%';
            result += s.conv;
            continue;
        }

        const Phasor::Value &val = args[argIndex++];
        const auto type = val.getType();

        switch (s.conv) {

            case 'd': case 'i': {
                long long v = 0;
                if (type == Phasor::ValueType::Int)
                    v = val.asInt();
                else if (type == Phasor::ValueType::Bool)
                    v = val.asBool() ? 1 : 0;
                else if (type == Phasor::ValueType::Float)
                    v = (long long)val.asFloat();

                std::string fmtStr = detail::build_fmt(s, "ll", s.conv);

                result += detail::snprintf_into(detail::hint(s), [&](char *b, int n) {
                    return std::snprintf(b, n, fmtStr.c_str(), v);
                });
                break;
            }

            case 'u': case 'o': case 'x': case 'X': {
                unsigned long long v = 0;

                if (type == Phasor::ValueType::Int)
                    v = (unsigned long long)val.asInt();
                else if (type == Phasor::ValueType::Bool)
                    v = val.asBool() ? 1ULL : 0ULL;
                else if (type == Phasor::ValueType::Float)
                    v = (unsigned long long)val.asFloat();

                std::string fmtStr = detail::build_fmt(s, "ll", s.conv);

                result += detail::snprintf_into(detail::hint(s), [&](char *b, int n) {
                    return std::snprintf(b, n, fmtStr.c_str(), v);
                });
                break;
            }

            case 'f': case 'F':
            case 'e': case 'E':
            case 'g': case 'G':
            case 'a': case 'A': {
                double v = 0.0;

                if (type == Phasor::ValueType::Float)
                    v = val.asFloat();
                else if (type == Phasor::ValueType::Int)
                    v = (double)val.asInt();
                else if (type == Phasor::ValueType::Bool)
                    v = val.asBool() ? 1.0 : 0.0;

                std::string fmtStr = detail::build_fmt(s, "", s.conv);

                result += detail::snprintf_into(detail::hint(s), [&](char *b, int n) {
                    return std::snprintf(b, n, fmtStr.c_str(), v);
                });
                break;
            }

            case 's': {
                std::string str;

                if (type == Phasor::ValueType::String)
                    str = val.asString();
                else if (type == Phasor::ValueType::Bool)
                    str = val.asBool() ? "true" : "false";
                else if (type == Phasor::ValueType::Null)
                    str = "(null)";
                else
                    str = val.jsonSerialize().str();

                std::size_t len = (s.prec >= 0)
                    ? std::min((std::size_t)s.prec, str.size())
                    : str.size();

                int pad = s.width - (int)len;

                if (!s.minus && pad > 0) result.append(pad, ' ');
                result.append(str.data(), len);
                if (s.minus && pad > 0) result.append(pad, ' ');
                break;
            }

            case 'c': {
                char c = '\0';

                if (type == Phasor::ValueType::Int)
                    c = (char)val.asInt();
                else if (type == Phasor::ValueType::Bool)
                    c = val.asBool() ? '1' : '0';

                int pad = s.width - 1;

                if (!s.minus && pad > 0) result.append(pad, ' ');
                result += c;
                if (s.minus && pad > 0) result.append(pad, ' ');
                break;
            }

            case 'p': {
                const void *ptr = nullptr;

                if (type == Phasor::ValueType::Int)
                    ptr = (const void*)(uintptr_t)val.asInt();
                else
                    ptr = nullptr;

                std::string fmtStr = "%";
                if (s.minus) fmtStr += '-';
                if (s.width > 0) fmtStr += std::to_string(s.width);
                fmtStr += 'p';

                result += detail::snprintf_into(detail::hint(s), [&](char *b, int n) {
                    return std::snprintf(b, n, fmtStr.c_str(), ptr);
                });
                break;
            }

            default:
                result += '%';
                result += s.conv;
                break;
        }
    }

    return result;
}

} // namespace vformat

#ifdef _MSC_VER
#pragma warning(pop)
#endif