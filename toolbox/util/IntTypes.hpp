// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TOOLBOX_UTIL_INTTYPES_HPP
#define TOOLBOX_UTIL_INTTYPES_HPP

#include <toolbox/util/Concepts.hpp>
#include <toolbox/util/TypeTraits.hpp>

#include <boost/functional/hash.hpp>

namespace toolbox {
inline namespace util {

template <typename ValueT>
struct IntPolicy {
    using ValueType = ValueT;
    static constexpr ValueT min() noexcept { return std::numeric_limits<ValueT>::min(); }
    static constexpr ValueT max() noexcept { return std::numeric_limits<ValueT>::max(); }
};
using Int16Policy = IntPolicy<std::int16_t>;
using Int32Policy = IntPolicy<std::int32_t>;
using Int64Policy = IntPolicy<std::int64_t>;

template <typename PolicyT>
struct TOOLBOX_PACKED IntWrapper {
    using ValueType = typename PolicyT::ValueType;

    template <typename RhsT>
        requires std::convertible_to<RhsT, ValueType>
    constexpr explicit IntWrapper(RhsT rhs) noexcept
    : value_{static_cast<ValueType>(rhs)}
    {
    }

    constexpr IntWrapper() noexcept = default;
    ~IntWrapper() = default;

    // Copy.
    constexpr IntWrapper(const IntWrapper&) noexcept = default;
    constexpr IntWrapper& operator=(const IntWrapper&) noexcept = default;

    // Move.
    constexpr IntWrapper(IntWrapper&&) noexcept = default;
    constexpr IntWrapper& operator=(IntWrapper&&) noexcept = default;

    constexpr ValueType count() const noexcept { return value_; }

    static constexpr IntWrapper min() noexcept { return PolicyT::min(); }
    static constexpr IntWrapper max() noexcept { return PolicyT::max(); }
    static constexpr IntWrapper zero() noexcept { return IntWrapper{0}; }

    // Assignment.

    /// Addition assignment.
    constexpr IntWrapper& operator+=(IntWrapper rhs) noexcept
    {
        value_ += rhs.value_;
        return *this;
    }

    /// Subtraction assignment.
    constexpr IntWrapper& operator-=(IntWrapper rhs) noexcept
    {
        value_ -= rhs.value_;
        return *this;
    }

    /// Multiplication assignment.
    constexpr IntWrapper& operator*=(IntWrapper rhs) noexcept
    {
        value_ *= rhs.value_;
        return *this;
    }

    /// Division assignment.
    constexpr IntWrapper& operator/=(IntWrapper rhs) noexcept
    {
        value_ /= rhs.value_;
        return *this;
    }

    /// Modulo assignment.
    constexpr IntWrapper& operator%=(IntWrapper rhs) noexcept
    {
        value_ %= rhs.value_;
        return *this;
    }

    /// Bitwise AND assignment.
    constexpr IntWrapper& operator&=(IntWrapper rhs) noexcept
    {
        value_ &= rhs.value_;
        return *this;
    }

    /// Bitwise OR assignment.
    constexpr IntWrapper& operator|=(IntWrapper rhs) noexcept
    {
        value_ |= rhs.value_;
        return *this;
    }

    /// Bitwise XOR assignment.
    constexpr IntWrapper& operator^=(IntWrapper rhs) noexcept
    {
        value_ ^= rhs.value_;
        return *this;
    }

    /// Bitwise left shift assignment.
    constexpr IntWrapper& operator<<=(IntWrapper rhs) noexcept
    {
        value_ <<= rhs.value_;
        return *this;
    }

    /// Bitwise right shift assignment.
    constexpr IntWrapper& operator>>=(IntWrapper rhs) noexcept
    {
        value_ >>= rhs.value_;
        return *this;
    }

    // Increment/Decrement.

    /// Pre-increment.
    constexpr IntWrapper& operator++() noexcept
    {
        ++value_;
        return *this;
    }

    /// Pre-decrement.
    constexpr IntWrapper& operator--() noexcept
    {
        --value_;
        return *this;
    }

    /// Post-increment.
    constexpr IntWrapper operator++(int) noexcept { return IntWrapper{value_++}; }

    /// Post-decrement.
    constexpr IntWrapper operator--(int) noexcept { return IntWrapper{value_--}; }

    // Arithmetic.

    /// Unary plus.
    constexpr IntWrapper operator+() const noexcept { return IntWrapper{+value_}; }

    /// Unary minus.
    constexpr IntWrapper operator-() const noexcept { return IntWrapper{-value_}; }

    /// Addition.
    friend constexpr IntWrapper operator+(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ + rhs.value_};
    }

    /// Subtraction.
    friend constexpr IntWrapper operator-(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ - rhs.value_};
    }

    /// Multiplication.
    friend constexpr IntWrapper operator*(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ * rhs.value_};
    }

    /// Division.
    friend constexpr IntWrapper operator/(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ / rhs.value_};
    }

    /// Modulo.
    friend constexpr IntWrapper operator%(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ % rhs.value_};
    }

    // Bitwise.

    /// Bitwise NOT.
    constexpr IntWrapper operator~() const noexcept { return IntWrapper{~value_}; }

    /// Bitwise AND.
    friend constexpr IntWrapper operator&(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ & rhs.value_};
    }

    /// Bitwise OR.
    friend constexpr IntWrapper operator|(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ | rhs.value_};
    }

    /// Bitwise XOR.
    friend constexpr IntWrapper operator^(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ ^ rhs.value_};
    }

    /// Bitwise left shift.
    friend constexpr IntWrapper operator<<(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ << rhs.value_};
    }

    /// Bitwise right shift.
    friend constexpr IntWrapper operator>>(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return IntWrapper{lhs.value_ >> rhs.value_};
    }

    // Comparison.
    friend constexpr auto operator<=>(const IntWrapper&, const IntWrapper&) noexcept = default;
    friend constexpr bool operator==(const IntWrapper&, const IntWrapper&) noexcept = default;

    // Stream.

    /// Insertion.
    template <typename StreamT>
        requires Streamable<StreamT>
    friend constexpr StreamT& operator<<(StreamT& os, IntWrapper rhs)
    {
        os << rhs.value_;
        return os;
    }

  private:
    ValueType value_;
};

static_assert(IntWrapper<Int32Policy>{1} == IntWrapper<Int32Policy>{1});
static_assert(IntWrapper<Int32Policy>{1} != IntWrapper<Int32Policy>{2});
static_assert(std::is_standard_layout_v<IntWrapper<Int32Policy>>
              && std::is_trivial_v<IntWrapper<Int32Policy>>);
static_assert(sizeof(IntWrapper<Int16Policy>) == 2, "must be specific size");
static_assert(sizeof(IntWrapper<Int32Policy>) == 4, "must be specific size");
static_assert(sizeof(IntWrapper<Int64Policy>) == 8, "must be specific size");

template <typename ValueT>
concept IsIntWrapper = is_instantiation_of<ValueT, IntWrapper>::value;

template <typename PolicyT>
std::size_t hash_value(IntWrapper<PolicyT> wrapper)
{
    boost::hash<typename PolicyT::ValueType> hasher;
    return hasher(wrapper.count());
}

template <typename ValueT>
    requires IsIntWrapper<ValueT>
struct TypeTraits<ValueT> {
    static constexpr auto from_string(std::string_view sv) noexcept
    {
        using UnderlyingTraits = TypeTraits<typename ValueT::ValueType>;
        return ValueT{UnderlyingTraits::from_string(sv)};
    }
};

struct Id16Policy : Int16Policy {};
struct Id32Policy : Int32Policy {};
struct Id64Policy : Int64Policy {};

/// 16 bit identifier.
using Id16 = IntWrapper<Id16Policy>;

constexpr Id16 operator""_id16(unsigned long long val) noexcept
{
    return Id16{val};
}

/// 32 bit identifier.
using Id32 = IntWrapper<Id32Policy>;

constexpr Id32 operator""_id32(unsigned long long val) noexcept
{
    return Id32{val};
}

/// 64 bit identifier.
using Id64 = IntWrapper<Id64Policy>;

constexpr Id64 operator""_id64(unsigned long long val) noexcept
{
    return Id64{val};
}

} // namespace util
} // namespace toolbox

namespace std {
template <typename PolicyT>
struct hash<toolbox::IntWrapper<PolicyT>> {
    inline std::size_t operator()(toolbox::IntWrapper<PolicyT> wrapper) const
    {
        return wrapper.count();
    }
};
} // namespace std

#endif // TOOLBOX_UTIL_INTTYPES_HPP
