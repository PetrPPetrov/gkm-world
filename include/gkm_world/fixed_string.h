// MIT License
//
// Copyright(c) 2021 Janitha Prasad Meedeniya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Project home: https://github.com/m3janitha/fixed_size_string
// License: MIT
// Modified by Petr Petrovich Petrov

#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <algorithm>
#include <ostream>

#pragma pack(push, 1)

template<class CharType, class SizeType, std::size_t MaxCapacity, class Traits = std::char_traits<CharType>>
class FixedCapacityBasicString {
public:
    constexpr FixedCapacityBasicString() noexcept = default;
    constexpr FixedCapacityBasicString(const CharType* str) :
        active_length(static_cast<SizeType>(std::min<std::size_t>(Traits::length(str), MAX_CAPACITY))) {
        std::copy(str, str + active_length, buffer);
    }
    constexpr FixedCapacityBasicString(const CharType* str, std::size_t length) :
        active_length(static_cast<SizeType>(std::min(length, MAX_CAPACITY))) {
        std::copy(str, str + active_length, buffer);
    }
    constexpr std::basic_string_view<CharType, Traits> strView() const noexcept {
        return std::basic_string_view<CharType, Traits>(buffer, active_length);
    }
    constexpr std::basic_string<CharType, Traits> str() const noexcept {
        return std::basic_string<CharType, Traits>(buffer, active_length);
    }
    constexpr std::size_t length() const noexcept {
        return active_length;
    }
    constexpr std::size_t maxSize() const noexcept {
        return MAX_CAPACITY;
    }
    std::size_t unusedSpace() const {
        return MAX_CAPACITY - active_length;
    }
    constexpr auto empty() const noexcept {
        return active_length == 0;
    }
    constexpr void clear() noexcept {
        active_length = 0;
    }
    void validate() {
        active_length = static_cast<SizeType>(std::min(active_length, MAX_CAPACITY));
    }
    constexpr void reset(const CharType* str) {
        active_length = static_cast<SizeType>(std::min(Traits::length(str), MAX_CAPACITY));
        resetImpl(str, active_length);
    }
    constexpr void reset(const CharType* str, std::size_t length) {
        active_length = static_cast<SizeType>(std::min<std::size_t>(length, MAX_CAPACITY));
        resetImpl(str, active_length);
    }
    constexpr void append(const CharType* str) {
        auto to_copy = static_cast<SizeType>(std::min(Traits::length(str), (MAX_CAPACITY - active_length)));
        appendImpl(str, to_copy);
    }
    constexpr void append(const CharType* str, std::size_t length) {
        auto to_copy = static_cast<SizeType>(std::min(length, (MAX_LENGTH - active_length)));
        appendImpl(str, to_copy);
    }
    constexpr void removePrefix(std::size_t length) {
        std::copy(buffer + length, buffer + active_length, buffer);
        active_length -= length;
    }
    constexpr void removeSuffix(std::size_t length) noexcept {
        active_length = active_length - length;
    }
    // Implemented as a member to avoid implicit conversion
    constexpr bool operator==(const FixedCapacityBasicString& rhs) const {
        return (maxSize() == rhs.maxSize())
            && (length() == rhs.length())
            && std::equal(buffer, buffer + length(), rhs.buffer);
    }
    constexpr bool operator!=(const FixedCapacityBasicString& rhs) const {
        return !(*this == rhs);
    }
    FixedCapacityBasicString& operator=(const std::basic_string_view<CharType, Traits>& str) noexcept {
        reset(str.c_str(), str.length());
        return *this;
    }
    FixedCapacityBasicString& operator=(const std::basic_string<CharType, Traits>& str) noexcept {
        reset(str.c_str(), str.length());
        return *this;
    }
    constexpr void swap(FixedCapacityBasicString& rhs) noexcept {
        std::swap(active_length, rhs.active_length);
        std::swap(buffer, rhs.buffer);
    }

private:
    constexpr void resetImpl(const CharType* str, std::size_t length) {
        std::copy(str, str + length, buffer);
    }
    constexpr void appendImpl(const CharType* str, std::size_t to_copy) {
        std::copy(str, str + to_copy, buffer + active_length);
        active_length += to_copy;
    }

    constexpr static std::uint16_t MAX_CAPACITY = MaxCapacity;
    SizeType active_length = 0;
    CharType buffer[MAX_CAPACITY] = { 0 };
};

template<class CharT, class SizeType, std::size_t MaxCapacity, class Traits = std::char_traits<CharT>>
inline constexpr void swap(const FixedCapacityBasicString<CharT, SizeType, MaxCapacity>& lhs, const FixedCapacityBasicString<CharT, SizeType, MaxCapacity>& rhs) noexcept {
    rhs.swap(lhs);
}

template<class CharT, class SizeType, std::size_t MaxCapacity, class Traits = std::char_traits<CharT>>
inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const FixedCapacityBasicString<CharT, SizeType, MaxCapacity>& str) {
    return os << str.strView();
}

typedef FixedCapacityBasicString<char, std::uint8_t, 255> String255;
typedef FixedCapacityBasicString<char, std::uint16_t, 512> String512;

#pragma pack(pop)
