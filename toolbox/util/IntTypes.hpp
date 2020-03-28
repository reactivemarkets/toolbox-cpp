// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2020 Reactive Markets Limited
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

#include <toolbox/util/TypeTraits.hpp>

#include <boost/functional/hash.hpp>

#include <cstdint>
#include <iosfwd>
#include <limits>
#include <type_traits>

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

struct IntBase {
};

template <typename PolicyT>
struct TOOLBOX_PACKED IntWrapper : IntBase {
    using ValueType = typename PolicyT::ValueType;

    template <typename RhsT,
              typename = typename std::enable_if_t<std::is_convertible_v<RhsT, ValueType>>>
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
    IntWrapper& operator+=(IntWrapper rhs) noexcept
    {
        value_ += rhs.value_;
        return *this;
    }

    /// Subtraction assignment.
    IntWrapper& operator-=(IntWrapper rhs) noexcept
    {
        value_ -= rhs.value_;
        return *this;
    }

    /// Multiplication assignment.
    IntWrapper& operator*=(IntWrapper rhs) noexcept
    {
        value_ *= rhs.value_;
        return *this;
    }

    /// Division assignment.
    IntWrapper& operator/=(IntWrapper rhs) noexcept
    {
        value_ /= rhs.value_;
        return *this;
    }

    /// Modulo assignment.
    IntWrapper& operator%=(IntWrapper rhs) noexcept
    {
        value_ %= rhs.value_;
        return *this;
    }

    /// Bitwise AND assignment.
    IntWrapper& operator&=(IntWrapper rhs) noexcept
    {
        value_ &= rhs.value_;
        return *this;
    }

    /// Bitwise OR assignment.
    IntWrapper& operator|=(IntWrapper rhs) noexcept
    {
        value_ |= rhs.value_;
        return *this;
    }

    /// Bitwise XOR assignment.
    IntWrapper& operator^=(IntWrapper rhs) noexcept
    {
        value_ ^= rhs.value_;
        return *this;
    }

    /// Bitwise left shift assignment.
    IntWrapper& operator<<=(IntWrapper rhs) noexcept
    {
        value_ <<= rhs.value_;
        return *this;
    }

    /// Bitwise right shift assignment.
    IntWrapper& operator>>=(IntWrapper rhs) noexcept
    {
        value_ >>= rhs.value_;
        return *this;
    }

    // Increment/Decrement.

    /// Pre-increment.
    IntWrapper& operator++() noexcept
    {
        ++value_;
        return *this;
    }

    /// Pre-decrement.
    IntWrapper& operator--() noexcept
    {
        --value_;
        return *this;
    }

    /// Post-increment.
    IntWrapper operator++(int) noexcept { return IntWrapper{value_++}; }

    /// Post-decrement.
    IntWrapper operator--(int) noexcept { return IntWrapper{value_--}; }

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

    /// Equal to.
    friend constexpr bool operator==(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return lhs.value_ == rhs.value_;
    }

    /// Not equal to.
    friend constexpr bool operator!=(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return lhs.value_ != rhs.value_;
    }

    /// Less than.
    friend constexpr bool operator<(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return lhs.value_ < rhs.value_;
    }

    /// Greater than.
    friend constexpr bool operator>(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return lhs.value_ > rhs.value_;
    }

    /// Less than or equal to.
    friend constexpr bool operator<=(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return lhs.value_ <= rhs.value_;
    }

    /// Greater than or equal to.
    friend constexpr bool operator>=(IntWrapper lhs, IntWrapper rhs) noexcept
    {
        return lhs.value_ >= rhs.value_;
    }

    // Stream.

    /// Insertion.
    friend std::ostream& operator<<(std::ostream& os, IntWrapper rhs) { return os << rhs.value_; }

  private:
    ValueType value_;
};
static_assert(std::is_pod_v<IntWrapper<Int32Policy>>);
static_assert(sizeof(IntWrapper<Int16Policy>) == 2, "must be specific size");
static_assert(sizeof(IntWrapper<Int32Policy>) == 4, "must be specific size");
static_assert(sizeof(IntWrapper<Int64Policy>) == 8, "must be specific size");

template <typename ValueT>
constexpr bool is_int_wrapper = std::is_base_of_v<IntBase, ValueT>;

template <typename PolicyT>
std::size_t hash_value(IntWrapper<PolicyT> wrapper)
{
    boost::hash<typename PolicyT::ValueType> hasher;
    return hasher(wrapper.count());
}

template <typename ValueT>
struct TypeTraits<ValueT, std::enable_if_t<is_int_wrapper<ValueT>>> {
    static constexpr auto from_string(std::string_view sv) noexcept
    {
        using UnderlyingTraits = TypeTraits<typename ValueT::ValueType>;
        return ValueT{UnderlyingTraits::from_string(sv)};
    }
};

struct Id16Policy : Int16Policy {
};
struct Id32Policy : Int32Policy {
};
struct Id64Policy : Int64Policy {
};

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
