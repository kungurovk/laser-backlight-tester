#pragma once

#include <QtGlobal>
#include <type_traits>

namespace detail {
template <typename Unsigned>
constexpr Unsigned byteSwap(Unsigned value)
{
    if constexpr (sizeof(Unsigned) == 1) {
        return value;
    } else if constexpr (sizeof(Unsigned) == 2) {
        return static_cast<Unsigned>((value << 8) | (value >> 8));
    } else if constexpr (sizeof(Unsigned) == 4) {
        return ((value & static_cast<Unsigned>(0x000000FFu)) << 24)
             | ((value & static_cast<Unsigned>(0x0000FF00u)) << 8)
             | ((value & static_cast<Unsigned>(0x00FF0000u)) >> 8)
             | ((value & static_cast<Unsigned>(0xFF000000u)) >> 24);
    } else if constexpr (sizeof(Unsigned) == 8) {
        return ((value & static_cast<Unsigned>(0x00000000000000FFull)) << 56)
             | ((value & static_cast<Unsigned>(0x000000000000FF00ull)) << 40)
             | ((value & static_cast<Unsigned>(0x0000000000FF0000ull)) << 24)
             | ((value & static_cast<Unsigned>(0x00000000FF000000ull)) << 8)
             | ((value & static_cast<Unsigned>(0x000000FF00000000ull)) >> 8)
             | ((value & static_cast<Unsigned>(0x0000FF0000000000ull)) >> 24)
             | ((value & static_cast<Unsigned>(0x00FF000000000000ull)) >> 40)
             | ((value & static_cast<Unsigned>(0xFF00000000000000ull)) >> 56);
    } else {
        static_assert(sizeof(Unsigned) == 1 || sizeof(Unsigned) == 2 || sizeof(Unsigned) == 4 || sizeof(Unsigned) == 8,
                      "Unsupported integer width for endian conversion");
    }
}

template <typename T>
using Decayed = std::decay_t<T>;

template <typename T, bool = std::is_enum_v<T>>
struct RawIntegralImpl
{
    using type = T;
};

template <typename T>
struct RawIntegralImpl<T, true>
{
    using type = std::underlying_type_t<T>;
};

template <typename T>
using RawIntegral = typename RawIntegralImpl<T>::type;

template <typename T>
using UnsignedRaw = std::make_unsigned_t<RawIntegral<T>>;
} // namespace detail

template <typename T>
constexpr T toLittleEndian(T value)
{
    using DecayedT = detail::Decayed<T>;
    static_assert(std::is_integral_v<DecayedT> || std::is_enum_v<DecayedT>,
                  "toLittleEndian expects an integral or enum type");

    using Unsigned = detail::UnsignedRaw<DecayedT>;
    Unsigned u = static_cast<Unsigned>(static_cast<detail::RawIntegral<DecayedT>>(value));

// #if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
//     return static_cast<T>(u);
// #else
    return static_cast<T>(detail::byteSwap(u));
// #endif
}

template <typename T>
constexpr T toBigEndian(T value)
{
    using DecayedT = detail::Decayed<T>;
    static_assert(std::is_integral_v<DecayedT> || std::is_enum_v<DecayedT>,
                  "toBigEndian expects an integral or enum type");

    using Unsigned = detail::UnsignedRaw<DecayedT>;
    Unsigned u = static_cast<Unsigned>(static_cast<detail::RawIntegral<DecayedT>>(value));

// #if Q_BYTE_ORDER == Q_BIG_ENDIAN
//     return static_cast<T>(u);
// #else
    return static_cast<T>(detail::byteSwap(u));
// #endif
}


