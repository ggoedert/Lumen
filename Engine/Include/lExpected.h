//==============================================================================================================================================================================
/// \file
/// \brief     Expected template class for handling values or string errors
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <variant>
#include <string>

/// Lumen namespace
namespace Lumen
{
    /// struct to hold an error message
    struct ErrorMessage { std::string message; };

    /// Expected template class for handling values or string errors
    template<typename T>
    class Expected
    {
    public:
        /// constructor
        Expected(const T &value) : mData(value) {}

        /// constructor with rvalue reference
        Expected(T &&value) : mData(std::move(value)) {}

        /// error constructor
        Expected(const ErrorMessage &error) : mData(error) {}

        /// error constructor with rvalue reference
        Expected(ErrorMessage &&error) : mData(std::move(error)) {}

        /// static helper to construct an error
        static Expected Unexpected(const std::string &error) { return Expected(ErrorMessage { error }); }

        /// static helper to construct an error with rvalue reference
        static Expected Unexpected(std::string &&error) { return Expected(ErrorMessage { std::move(error) }); }

        /// accessors
        bool HasValue() const noexcept { return std::holds_alternative<T>(mData); }

        /// value accessor
        T &Value() { return std::get<T>(mData); }

        /// const value accessor
        const T &Value() const { return std::get<T>(mData); }

        /// value or default accessor with rvalue reference
        T ValueOr(T &&defaultValue) const { return HasValue() ? std::get<T>(mData) : std::move(defaultValue); }

        /// value or default accessor (non-const lvalue)
        T &ValueOr(T &defaultValue) const { return HasValue() ? std::get<T>(mData) : defaultValue; }

        /// value or default accessor (const lvalue)
        const T &ValueOr(const T &defaultValue) const { return HasValue() ? std::get<T>(mData) : defaultValue; }

        /// error accessor
        std::string &Error() { return std::get<ErrorMessage>(mData).message; }

        /// const error accessor
        const std::string &Error() const { return std::get<ErrorMessage>(mData).message; }

        /// operator bool for convenience
        explicit operator bool() const noexcept { return HasValue(); }

    private:

        /// data storage
        std::variant<T, ErrorMessage> mData;
    };
}
