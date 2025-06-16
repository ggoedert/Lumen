//==============================================================================================================================================================================
/// \file
/// \brief     Property template class for encapsulating a value with getter and setter methods
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <utility>

/// Lumen namespace
namespace Lumen
{
    /// Property template class
    template<typename T, typename Getter, typename Setter>
    class Property
    {
        CLASS_NO_DEFAULT_CTOR(Property);
        CLASS_NO_COPY_MOVE(Property);

    public:
        /// constructs a property
        Property(Getter get, Setter set) : mGetter(std::move(get)), mSetter(std::move(set)) {}

        /// assignment (lvalue)
        Property &operator=(const T &val)
        {
            mSetter(val);
            return *this;
        }

        /// assignment (rvalue)
        Property &operator=(T &&val)
        {
            mSetter(std::move(val));
            return *this;
        }

        /// implicit conversion to T
        operator T() const
        {
            return mGetter();
        }

        /// implicit conversion to const T&
        template<typename U = Getter,
            typename R = decltype(std::declval<U>()()),
            std::enable_if_t<std::is_same_v<R, const T &>, int> = 0>
        operator const T &() const
        {
            return mGetter();
        }

        /// operator+= (lvalue)
        Property &operator+=(const T &val)
        {
            mSetter(mGetter() + val);
            return *this;
        }

        /// operator+= (rvalue)
        Property &operator+=(T &&val)
        {
            mSetter(mGetter() + std::move(val));
            return *this;
        }

        /// operator-= (lvalue)
        Property &operator-=(const T &val)
        {
            mSetter(mGetter() - val);
            return *this;
        }

        /// operator-= (rvalue)
        Property &operator-=(T &&val)
        {
            mSetter(mGetter() - std::move(val));
            return *this;
        }

        /// operator*= (lvalue)
        Property &operator*=(const T &val)
        {
            mSetter(mGetter() * val);
            return *this;
        }

        /// operator*= (rvalue)
        Property &operator*=(T &&val)
        {
            mSetter(mGetter() * std::move(val));
            return *this;
        }

        /// operator/= (lvalue)
        Property &operator/=(const T &val)
        {
            mSetter(mGetter() / val);
            return *this;
        }

        /// operator/= (rvalue)
        Property &operator/=(T &&val)
        {
            mSetter(mGetter() / std::move(val));
            return *this;
        }

        /// pre increment
        Property &operator++()
        {
            mSetter(mGetter() + T(1));
            return *this;
        }

        /// pre decrement 
        Property &operator--()
        {
            mSetter(mGetter() - T(1));
            return *this;
        }

        /// post increment
        T operator++(int)
        {
            T tmp = mGetter();
            mSetter(tmp + T(1));
            return tmp;
        }

        /// post decrement 
        T operator--(int)
        {
            T tmp = mGetter();
            mSetter(tmp - T(1));
            return tmp;
        }

        /// operator==
        friend bool operator==(const Property &a, const T &b) { return a.mGetter() == b; }

        /// operator!=
        friend bool operator!=(const Property &a, const T &b) { return a.mGetter() != b; }

    private:
        /// getter method to retrieve the value
        Getter mGetter;

        /// setter method to set the value
        Setter mSetter;
    };

    /// helper function to create a property with getter and setter methods
    template<typename T, typename Getter, typename Setter>
    auto MakeProperty(Getter &&get, Setter &&set)
    {
        return Property<T, std::decay_t<Getter>, std::decay_t<Setter>>(
            std::forward<Getter>(get),
            std::forward<Setter>(set)
        );
    }
    /// property macro to define a property with getter and setter methods
    #define PROPERTY(TYPE, NAME)                                               \
    auto NAME()                                                                \
    {                                                                          \
        return Lumen::MakeProperty<TYPE>(                                      \
            [this]() -> const TYPE & { return Get####NAME(); },                \
            [this](const TYPE &v) -> const TYPE & { return Set####NAME(v); }); \
    }
}
