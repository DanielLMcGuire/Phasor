/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                             //
//   PPPPPPP  H     H      AA      SSSSSSS  OOOOOOO  RRRRRRR    L            AA      NN    N  GGGGGGG  U     U      AA      GGGGGGG  EEEEEEE   //
//   P     P  H     H     A  A     S        O     O  R     R    L           A  A     N N   N  G        U     U     A  A     G        E         //
//   PPPPPPP  HHHHHHH    AAAAAA    SSSSSSS  O     O  RRRRRRR    L          AAAAAA    N  N  N  G  GGGG  U     U    AAAAAA    G  GGGG  EEEEEEE   //
//   P        H     H   A      A         S  O     O  R    R     L         A      A   N   N N  G     G  U     U   A      A   G     G  E         //
//   P        H     H  A        A  SSSSSSS  OOOOOOO  R     R    LLLLLLL  A        A  N    NN  GGGGGGG  UUUUUUU  A        A  GGGGGGG  EEEEEEE   //
//                                                                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <algorithm>
#include <cassert>
#include "phsint.hpp"
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <format>
#include <compare>
#include <type_traits>

inline constexpr std::size_t kSSOCapacity = 23;

static_assert(kSSOCapacity >= 1,   "SSO capacity must be at least 1");
static_assert(kSSOCapacity <= 255, "SSO capacity must fit in a u8");

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/*
 * @brief Phasor SSO String
 */
class PhsString {
public:
    static constexpr std::size_t SSO_CAPACITY = kSSOCapacity;
    static constexpr std::size_t npos         = std::string::npos;

    using value_type             = char;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = char&;
    using const_reference        = const char&;
    using pointer                = char*;
    using const_pointer          = const char*;
    using iterator               = char*;
    using const_iterator         = const char*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
 
    PhsString() noexcept : m_store{SmallBuf{}} {}
 
    PhsString(const char* s) : PhsString(s, s ? std::strlen(s) : 0) {}
 
    PhsString(const char* s, std::size_t n) {
        if (n <= SSO_CAPACITY) m_store = SmallBuf{s, n};
        else                   m_store = std::string{s, n};
    }
 
    PhsString(std::string_view sv)
        : PhsString(sv.data(), sv.size()) {}
 
    PhsString(const std::string& s)
        : PhsString(s.data(), s.size()) {}
 
    PhsString(std::size_t n, char c) {
        if (n <= SSO_CAPACITY) {
            SmallBuf buf{};
            std::memset(buf.data, c, n);
            buf.data[n] = '\0';
            buf.len     = static_cast<u8>(n);
            m_store     = buf;
        } else {
            m_store = std::string(n, c);
        }
    }

    PhsString(const PhsString&)                = default;
    PhsString(PhsString&&) noexcept            = default;
    PhsString& operator=(const PhsString&)     = default;
    PhsString& operator=(PhsString&&) noexcept = default;
    ~PhsString()                               = default;
 
    PhsString& operator=(const char* s)      { return *this = PhsString{s}; }
    PhsString& operator=(std::string_view sv){ return *this = PhsString{sv}; }
 
    [[nodiscard]] std::size_t size()   const noexcept { return is_small() ? sm().len : lg().size(); }
    [[nodiscard]] std::size_t length() const noexcept { return size(); }
    [[nodiscard]] bool        empty()  const noexcept { return size() == 0; }
 
    [[nodiscard]] std::size_t capacity() const noexcept {
        return is_small() ? SSO_CAPACITY : lg().capacity();
    }
    [[nodiscard]] std::size_t max_size() const noexcept {
        return std::string{}.max_size();
    }

    [[nodiscard]] bool is_small()          const noexcept { return std::holds_alternative<SmallBuf>(m_store); }
    [[nodiscard]] bool is_heap_allocated() const noexcept { return !is_small(); }
 
    [[nodiscard]] const char* data()  const noexcept { return is_small() ? sm().data : lg().data(); }
    [[nodiscard]] char*       data()        noexcept { return is_small() ? sm().data : lg().data(); }
    [[nodiscard]] const char* c_str() const noexcept { return data(); }
    [[nodiscard]] std::string_view view() const noexcept { return {data(), size()}; }
 
    char&       operator[](std::size_t i)       noexcept { return data()[i]; }
    const char& operator[](std::size_t i) const noexcept { return data()[i]; }
 
    char& at(std::size_t i) {
        if (i >= size()) throw std::out_of_range{"PhsString::at: index out of range"};
        return data()[i];
    }
    const char& at(std::size_t i) const {
        if (i >= size()) throw std::out_of_range{"PhsString::at: index out of range"};
        return data()[i];
    }
 
    char&       front()       noexcept { assert(!empty()); return data()[0]; }
    const char& front() const noexcept { assert(!empty()); return data()[0]; }
    char&       back()        noexcept { assert(!empty()); return data()[size() - 1]; }
    const char& back()  const noexcept { assert(!empty()); return data()[size() - 1]; }

 
    iterator       begin()        noexcept { return data(); }
    iterator       end()          noexcept { return data() + size(); }
    const_iterator begin()  const noexcept { return data(); }
    const_iterator end()    const noexcept { return data() + size(); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend()   const noexcept { return end(); }
 
    reverse_iterator       rbegin()        noexcept { return reverse_iterator{end()}; }
    reverse_iterator       rend()          noexcept { return reverse_iterator{begin()}; }
    const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator{end()}; }
    const_reverse_iterator rend()    const noexcept { return const_reverse_iterator{begin()}; }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend()   const noexcept { return rend(); }

 
    void clear() noexcept {
        if (is_small()) { sm().data[0] = '\0'; sm().len = 0; }
        else            { lg().clear(); }
    }
 
    PhsString& append(const char* s, std::size_t n) {
        const std::size_t old_sz = size();
        const std::size_t new_sz = old_sz + n;
 
        if (is_small()) {
            if (new_sz <= SSO_CAPACITY) {
                std::memcpy(sm().data + old_sz, s, n);
                sm().data[new_sz] = '\0';
                sm().len          = static_cast<u8>(new_sz);
                return *this;
            }
            std::string promoted;
            promoted.reserve(new_sz);
            promoted.append(sm().data, old_sz);
            promoted.append(s, n);
            m_store = std::move(promoted);
        } else {
            lg().append(s, n);
        }
        return *this;
    }
 
    PhsString& append(std::string_view sv) { return append(sv.data(), sv.size()); }
    PhsString& append(const char* s)       { return append(s, std::strlen(s)); }
    PhsString& append(std::size_t n, char c) {
        const std::size_t old_sz = size();
        const std::size_t new_sz = old_sz + n;
        if (is_small()) {
            if (new_sz <= SSO_CAPACITY) {
                std::memset(sm().data + old_sz, c, n);
                sm().data[new_sz] = '\0';
                sm().len          = static_cast<u8>(new_sz);
                return *this;
            }
            std::string promoted;
            promoted.reserve(new_sz);
            promoted.append(sm().data, old_sz);
            promoted.append(n, c);
            m_store = std::move(promoted);
        } else {
            lg().append(n, c);
        }
        return *this;
    }

    PhsString& operator+=(std::string_view sv) { return append(sv.data(), sv.size()); }
    PhsString& operator+=(const char* s)       { return append(s); }
    PhsString& operator+=(char c)              { return append(&c, 1); }
 
    void push_back(char c) { append(&c, 1); }
    void pop_back() noexcept {
        assert(!empty());
        if (is_small()) sm().data[--sm().len] = '\0';
        else            lg().pop_back();
    }
 
    void resize(std::size_t n, char c = '\0') {
        const std::size_t old_sz = size();
        if      (n == old_sz) { return; }
        else if (n  < old_sz) {
            if (is_small()) { sm().data[n] = '\0'; sm().len = static_cast<u8>(n); }
            else            { lg().resize(n); }
        } else {
            append(n - old_sz, c);
        }
    }

    void reserve(std::size_t n) {
        if (is_small()) {
            if (n > SSO_CAPACITY) {
                std::string promoted;
                promoted.reserve(n);
                promoted.append(sm().data, sm().len);
                m_store = std::move(promoted);
            }
        } else {
            lg().reserve(n);
        }
    }

    void swap(PhsString& other) noexcept { m_store.swap(other.m_store); }
 
    std::size_t find(std::string_view sv, std::size_t pos = 0)     const noexcept { return view().find(sv, pos); }
    std::size_t find(char c, std::size_t pos = 0)                  const noexcept { return view().find(c, pos); }
    std::size_t rfind(std::string_view sv, std::size_t pos = npos) const noexcept { return view().rfind(sv, pos); }
    std::size_t rfind(char c, std::size_t pos = npos)              const noexcept { return view().rfind(c, pos); }
 
    std::size_t find_first_of    (std::string_view sv, std::size_t pos = 0)   const noexcept { return view().find_first_of(sv, pos); }
    std::size_t find_last_of     (std::string_view sv, std::size_t pos = npos)const noexcept { return view().find_last_of(sv, pos); }
    std::size_t find_first_not_of(std::string_view sv, std::size_t pos = 0)   const noexcept { return view().find_first_not_of(sv, pos); }
    std::size_t find_last_not_of (std::string_view sv, std::size_t pos = npos)const noexcept { return view().find_last_not_of(sv, pos); }
 
    int compare(std::string_view sv) const noexcept { return view().compare(sv); }
 
    [[nodiscard]] bool starts_with(std::string_view sv) const noexcept { return view().starts_with(sv); }
    [[nodiscard]] bool ends_with  (std::string_view sv) const noexcept { return view().ends_with(sv); }
    [[nodiscard]] bool contains   (std::string_view sv) const noexcept { return find(sv) != npos; }
 
    [[nodiscard]] PhsString substr(std::size_t pos = 0, std::size_t len = npos) const {
        const std::size_t sz = size();
        if (pos > sz) throw std::out_of_range{"PhsString::substr: pos out of range"};
        return PhsString{data() + pos, std::min(len, sz - pos)};
    }
 
    [[nodiscard]] std::string str() const {
        return is_small() ? std::string{sm().data, sm().len} : lg();
    }
    operator std::string()          const { return str(); }
    operator std::string_view()     const noexcept { return view(); }

    [[nodiscard]] friend std::strong_ordering operator<=>(const PhsString& lhs, const PhsString& rhs) noexcept {
        return lhs.view() <=> rhs.view();
    }

    template <typename T>
        requires std::is_convertible_v<const T&, std::string_view> && (!std::is_same_v<std::remove_cvref_t<T>, PhsString>)
    [[nodiscard]] friend std::strong_ordering operator<=>(const PhsString& lhs, const T& rhs) noexcept {
        return lhs.view() <=> std::string_view(rhs);
    }

    [[nodiscard]] friend bool operator==(const PhsString& lhs, const PhsString& rhs) noexcept {
        return lhs.view() == rhs.view();
    }

    template <typename T>
        requires std::is_convertible_v<const T&, std::string_view> && (!std::is_same_v<std::remove_cvref_t<T>, PhsString>)
    [[nodiscard]] friend bool operator==(const PhsString& lhs, const T& rhs) noexcept {
        return lhs.view() == std::string_view(rhs);
    }
 
    friend std::ostream& operator<<(std::ostream& os, const PhsString& s) {
        return os.write(s.data(), static_cast<std::streamsize>(s.size()));
    }

    /// @brief Erases characters from the string.
    /// @param pos Position to start erasing from.
    /// @param len Number of characters to erase.
    PhsString& erase(std::size_t pos = 0, std::size_t len = npos) {
        const std::size_t sz = size();
        if (pos > sz) {
            throw std::out_of_range{"PhsString::erase: pos out of range"};
        }
        
        if (len == npos || pos + len > sz) {
            len = sz - pos;
        }

        if (len == 0) return *this;

        if (is_small()) {
            const std::size_t remaining = sz - (pos + len);
            if (remaining > 0) {
                std::memmove(sm().data + pos, sm().data + pos + len, remaining);
            }
            sm().len = static_cast<u8>(sz - len);
            sm().data[sm().len] = '\0';
        } else {
            lg().erase(pos, len);
        }
        return *this;
    }

    PhsString& insert(std::size_t pos, std::string_view sv) {
        const std::size_t sz = size();
        if (pos > sz) {
            throw std::out_of_range{"PhsString::insert: pos out of range"};
        }
        
        const std::size_t n = sv.size();
        if (n == 0) return *this;

        const std::size_t new_sz = sz + n;

        if (is_small()) {
            if (new_sz <= SSO_CAPACITY) {
                const std::size_t suffix_len = sz - pos;
                if (suffix_len > 0) {
                    std::memmove(sm().data + pos + n, sm().data + pos, suffix_len);
                }
                std::memcpy(sm().data + pos, sv.data(), n);
                sm().len = static_cast<u8>(new_sz);
                sm().data[new_sz] = '\0';
                return *this;
            } else {
                std::string promoted;
                promoted.reserve(new_sz);
                promoted.append(sm().data, pos);
                promoted.append(sv);
                promoted.append(sm().data + pos, sz - pos);
                m_store = std::move(promoted);
            }
        } else {
            lg().insert(pos, sv);
        }
        return *this;
    }

    PhsString& replace(std::size_t pos, std::size_t len, std::string_view sv) {
        erase(pos, len);
        return insert(pos, sv);
    }

    /// @brief Erases a single character at the specified iterator position.
    iterator erase(iterator p) {
        // Calculate index from pointer arithmetic
        std::size_t pos = static_cast<std::size_t>(p - begin());
        erase(pos, 1);
        return begin() + pos;
    }

    /// @brief Erases a range of characters.
    iterator erase(iterator first, iterator last) {
        std::size_t pos = static_cast<std::size_t>(first - begin());
        std::size_t len = static_cast<std::size_t>(last - first);
        erase(pos, len);
        return begin() + pos;
    }

    /// @brief Assigns a string view to the PhsString.
    PhsString& assign(std::string_view sv) {
        *this = PhsString(sv);
        return *this;
    }

    /// @brief Assigns a specific length of a character array to the PhsString.
    PhsString& assign(const char* s, std::size_t n) {
        *this = PhsString(s, n);
        return *this;
    }

    /// @brief Assigns n copies of character c to the PhsString.
    PhsString& assign(std::size_t n, char c) {
        *this = PhsString(n, c);
        return *this;
    }
 
private:
 
    struct SmallBuf {
        char         data[SSO_CAPACITY + 1];
        u8 len;
 
        constexpr SmallBuf() noexcept : data{}, len{0} {}
 
        SmallBuf(const char* s, std::size_t n) noexcept
            : len{static_cast<u8>(n)} {
            std::memcpy(data, s, n);
            data[n] = '\0';
        }
    };

    std::variant<SmallBuf, std::string> m_store;
 
    SmallBuf&          sm()       noexcept { return std::get<SmallBuf>(m_store); }
    const SmallBuf&    sm() const noexcept { return std::get<SmallBuf>(m_store); }
    std::string&       lg()       noexcept { return std::get<std::string>(m_store); }
    const std::string& lg() const noexcept { return std::get<std::string>(m_store); }
};
 
inline PhsString operator+(PhsString lhs, const PhsString& rhs) { return lhs += rhs.view(); }
inline PhsString operator+(PhsString lhs, const char* rhs)      { return lhs += rhs; }
inline PhsString operator+(PhsString lhs, char rhs)             { return lhs += rhs; }
inline PhsString operator+(const char* lhs, const PhsString& rhs) { return PhsString{lhs} += rhs.view(); }
inline PhsString operator+(char lhs, const PhsString& rhs)        { return PhsString(1, lhs) += rhs.view(); }

inline void swap(PhsString& lhs, PhsString& rhs) noexcept { lhs.swap(rhs); }

} // namespace Phasor

template <>
struct std::hash<Phasor::PhsString> {
    std::size_t operator()(const Phasor::PhsString& s) const noexcept {
        return std::hash<std::string_view>{}(s.view());
    }
};

template <>
struct std::formatter<Phasor::PhsString> : std::formatter<std::string_view>
{
	template <typename FormatContext>
	auto format(const Phasor::PhsString &s, FormatContext &ctx) const
	{
		return std::formatter<std::string_view>::format(s.view(), ctx);
	}
};