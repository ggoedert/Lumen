//==============================================================================================================================================================================
/// \file
/// \brief     StringMap template for std::unordered_map<std::string, T> that can be searched with string_view
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <unordered_map>
#include <string>
#include <string_view>

/// Lumen namespace
namespace Lumen
{
    /// transparent string hash
    struct TransparentStringHash
    {
        using is_transparent = void;

        size_t operator()(const std::string &s) const noexcept
        {
            return std::hash<std::string_view>{}(s);
        }

        size_t operator()(std::string_view sv) const noexcept
        {
            return std::hash<std::string_view>{}(sv);
        }
    };

    /// transparent string equality
    struct TransparentStringEqual
    {
        using is_transparent = void;

        bool operator()(const std::string &lhs, const std::string &rhs) const noexcept
        {
            return lhs == rhs;
        }

        bool operator()(const std::string &lhs, std::string_view rhs) const noexcept
        {
            return lhs == rhs;
        }

        bool operator()(std::string_view lhs, const std::string &rhs) const noexcept
        {
            return lhs == rhs;
        }

        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
        {
            return lhs == rhs;
        }
    };

    /// std::unordered_map<std::string, T> that can be searched with string_view
    template<typename T>
    using StringMap = std::unordered_map<std::string, T, TransparentStringHash, TransparentStringEqual>;
}
