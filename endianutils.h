#pragma once

#include <QtGlobal>
#include <type_traits>

template <typename T>
constexpr T toLittleEndian(T value)
{
    // static_assert(std::is_integral_v<T>, "toLittleEndian expects an integral type");

// #if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
//     return value;
// #else
    using Unsigned = std::make_unsigned_t<T>;
    Unsigned u = static_cast<Unsigned>(value);

    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        u = static_cast<Unsigned>((u << 8) | (u >> 8));
    } else if constexpr (sizeof(T) == 4) {
        u = ((u & static_cast<Unsigned>(0x000000FFu)) << 24)
            | ((u & static_cast<Unsigned>(0x0000FF00u)) << 8)
            | ((u & static_cast<Unsigned>(0x00FF0000u)) >> 8)
            | ((u & static_cast<Unsigned>(0xFF000000u)) >> 24);
    } else if constexpr (sizeof(T) == 8) {
        u = ((u & static_cast<Unsigned>(0x00000000000000FFull)) << 56)
            | ((u & static_cast<Unsigned>(0x000000000000FF00ull)) << 40)
            | ((u & static_cast<Unsigned>(0x0000000000FF0000ull)) << 24)
            | ((u & static_cast<Unsigned>(0x00000000FF000000ull)) << 8)
            | ((u & static_cast<Unsigned>(0x000000FF00000000ull)) >> 8)
            | ((u & static_cast<Unsigned>(0x0000FF0000000000ull)) >> 24)
            | ((u & static_cast<Unsigned>(0x00FF000000000000ull)) >> 40)
            | ((u & static_cast<Unsigned>(0xFF00000000000000ull)) >> 56);
    } else {
        static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                      "Unsupported integer width for toLittleEndian");
    }

    return static_cast<T>(u);
// #endif
}


