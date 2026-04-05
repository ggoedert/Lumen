//==============================================================================================================================================================================
/// \file
/// \brief     Flags template class
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    /// Flags template class
    template<typename EnumT>
    class Flags
    {
        static_assert(std::is_enum_v<EnumT>, "Flags<EnumT> requires an enum type");

        /// unsigned underlying type from the enum
        using Underlying = std::make_unsigned_t<std::underlying_type_t<EnumT>>;

    public:

        /// default constructor
        constexpr Flags() = default;

        /// construct from enum value
        constexpr Flags(EnumT _enum) noexcept : mValue(static_cast<Underlying>(_enum)) {}

        /// construct from underlying value
        constexpr explicit Flags(Underlying value) noexcept : mValue(value) {}

        /// equality operator
        [[nodiscard]] constexpr bool operator==(const Flags &other) const noexcept { return mValue == other.mValue; }

        /// inequality operator
        [[nodiscard]] constexpr bool operator!=(const Flags &other) const noexcept { return mValue != other.mValue; }

        /// bitwise OR from enum
        [[nodiscard]] constexpr Flags operator|(EnumT _enum) const noexcept { return Flags(mValue | static_cast<Underlying>(_enum)); }

        /// bitwise OR from other
        [[nodiscard]] constexpr Flags operator|(const Flags &other) const noexcept { return Flags(mValue | other.mValue); }

        /// bitwise OR assignment from enum
        constexpr Flags &operator|=(EnumT _enum) noexcept { mValue |= static_cast<Underlying>(_enum); return *this; }

        /// bitwise OR assignment from other
        constexpr Flags &operator|=(const Flags &other) noexcept { mValue |= other.mValue; return *this; }

        /// bitwise AND from enum
        [[nodiscard]] constexpr Flags operator&(EnumT _enum) const noexcept { return Flags(mValue & static_cast<Underlying>(_enum)); }

        /// bitwise AND from other
        [[nodiscard]] constexpr Flags operator&(const Flags &other) const noexcept { return Flags(mValue & other.mValue); }

        /// bitwise AND assignment from enum
        constexpr Flags &operator&=(EnumT _enum) noexcept { mValue &= static_cast<Underlying>(_enum); return *this; }

        /// bitwise AND assignment from other
        constexpr Flags &operator&=(const Flags &other) noexcept { mValue &= other.mValue; return *this; }

        /// bitwise XOR from enum
        [[nodiscard]] constexpr Flags operator^(EnumT _enum) const noexcept { return Flags(mValue ^ static_cast<Underlying>(_enum)); }

        /// bitwise XOR from other
        [[nodiscard]] constexpr Flags operator^(const Flags &other) const noexcept { return Flags(mValue ^ other.mValue); }

        /// bitwise XOR assignment from enum
        constexpr Flags &operator^=(EnumT _enum) noexcept { mValue ^= static_cast<Underlying>(_enum); return *this; }

        /// bitwise XOR assignment from other
        constexpr Flags &operator^=(const Flags &other) noexcept { mValue ^= other.mValue; return *this; }

        /// bitwise NOT operator
        [[nodiscard]] constexpr Flags operator~() const noexcept { return Flags(~mValue); }

        /// check if bits in the enum are set
        [[nodiscard]] constexpr bool Has(EnumT _enum) const noexcept { return (mValue & static_cast<Underlying>(_enum)) == static_cast<Underlying>(_enum); }

        /// check if bits in other are set
        [[nodiscard]] constexpr bool Has(const Flags &other) const noexcept { return (mValue & other.mValue) == other.mValue; }

        /// check if any bits are set
        [[nodiscard]] constexpr bool Any() const noexcept { return mValue != 0; }

        /// check if no bits are set
        [[nodiscard]] constexpr bool None() const noexcept { return mValue == 0; }

        /// get raw value
        [[nodiscard]] constexpr Underlying Value() const noexcept { return mValue; }

        /// boolean context
        constexpr explicit operator bool() const noexcept { return Any(); }

        /// friend OR function
        [[nodiscard]] friend constexpr Flags operator|(EnumT lhs, EnumT rhs) noexcept { return Flags(lhs) | rhs; }

        /// friend AND function
        [[nodiscard]] friend constexpr Flags operator&(EnumT lhs, EnumT rhs) noexcept { return Flags(lhs) & rhs; }

        /// friend XOR function
        [[nodiscard]] friend constexpr Flags operator^(EnumT lhs, EnumT rhs) noexcept { return Flags(lhs) ^ rhs; }

        /// friend NOT function
        [[nodiscard]] friend constexpr Flags operator~(EnumT _enum) noexcept { return ~Flags(_enum); }

    private:
        /// underlying value
        Underlying mValue = 0;
    };
}
