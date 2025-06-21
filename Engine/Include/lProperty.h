//==============================================================================================================================================================================
/// \file
/// \brief     Property template class for encapsulating a value with getter and setter methods
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <utility>
#include <stdexcept>
#include <typeinfo>

/// Lumen namespace
namespace Lumen
{
    /// detection idiom helper
    template<typename, template<typename...> class, typename...>
    struct is_detected : std::false_type {};

    /// detection idiom helper
    template<template<typename...> class Op, typename... Args>
    struct is_detected<std::void_t<Op<Args...>>, Op, Args...> : std::true_type {};

    /// detection idiom helper
    template<template<typename...> class Op, typename... Args>
    inline constexpr bool is_detected_v = is_detected<void, Op, Args...>::value;

    /// check if T supports operator[]
    template<typename T, typename Index>
    using detect_subscript_t = decltype(std::declval<T>()[std::declval<Index>()]);

    /// check if T supports operator[]
    template<typename T, typename Index>
    constexpr bool is_subscriptable_v = is_detected_v<detect_subscript_t, T, Index>;

    /// IProperty base class
    class IProperty
    {
        CLASS_NO_COPY_MOVE(IProperty);

    public:
        /// access modes for properties
        enum class Mode { Read, Write, ReadWrite };

        /// constructs a IProperty
        IProperty(Mode mode, std::string name) : mMode(mode), mName(std::move(name)) {}

        /// default destructor
        virtual ~IProperty() = default;

        /// checks if the property is of a specific type
        virtual bool IsTypeId(const std::type_info &typeId) const = 0;

        /// compile-time read mode check
        static constexpr bool HasRead(Mode mode) { return (mode == Mode::Read) || (mode == Mode::ReadWrite); }

        /// compile-time write mode check
        static constexpr bool HasWrite(Mode mode) { return (mode == Mode::Write) || (mode == Mode::ReadWrite); }

        /// compile-time read/write mode check
        static constexpr bool HasReadWrite(Mode mode) { return (mode == Mode::ReadWrite); }

        /// mode check
        bool HasMode(Mode flag)
        {
            switch (flag)
            {
            case Mode::Read:
                return HasRead(mMode);
                break;
            case Mode::Write:
                return HasWrite(mMode);
                break;
            case Mode::ReadWrite:
                return HasReadWrite(mMode);
                break;
            }
            return false;
        }
        /// return property name
        std::string_view Name() const { return mName; }

        /// generic get
        virtual std::any Get() const = 0;

        /// generic set
        virtual bool Set(const std::any &) = 0;

    protected:
        /// property access mode
        Mode mMode;

        /// property name
        std::string mName;
    };

    /// Property template class
    template<typename T, typename Getter, typename Setter, IProperty::Mode StaticMode>
    class Property : public IProperty
    {
        CLASS_NO_DEFAULT_CTOR(Property);

    public:
        /// constructs a property
        Property(Getter get, Setter set, std::string name) : IProperty(StaticMode, std::move(name)), mGetter(std::move(get)), mSetter(std::move(set)) {}

        /// returns the type id of the property
        bool IsTypeId(const std::type_info &typeId) const override { return typeId == typeid(T); }

        /// generic get, only enabled if mode has read
        std::any Get() const override
        {
            if constexpr (StaticMode == Mode::Read || StaticMode == Mode::ReadWrite)
            {
                return mGetter();
            }
            else
            {
                assert(false && "Get() called on non-readable property");
                throw std::logic_error("Get() called on non-readable property");
            }
        }

        /// generic set, only enabled if mode has write
        bool Set(const std::any &value) override
        {
            if constexpr (StaticMode == Mode::Write || StaticMode == Mode::ReadWrite)
            {
                try
                {
                    mSetter(std::any_cast<const T &>(value));
                    return true;
                }
                catch (const std::bad_any_cast &)
                {
                    return false;
                }
            }
            else
            {
                assert(false && "Set() called on non-writable property");
                throw std::logic_error("Set() called on non-writable property");
            }
        }

        /// assignment (lvalue)
        template<typename Dummy = void, std::enable_if_t<HasWrite(StaticMode), int> = 0>
        Property &operator=(const T &val)
        {
            mSetter(val);
            return *this;
        }

        /// assignment (rvalue)
        template<typename Dummy = void, std::enable_if_t<HasWrite(StaticMode), int> = 0>
        Property &operator=(T &&val)
        {
            mSetter(std::move(val));
            return *this;
        }

        /// operator-> for Read or ReadWrite, const version
        template<IProperty::Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<HasRead(mode) && std::is_reference_v<R>, int> = 0>
        const std::remove_reference_t<R> *operator->() const
        {
            return &mGetter();
        }

        /// operator-> for Read or ReadWrite, non-const version
        template<IProperty::Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<HasRead(mode) && std::is_reference_v<R>, int> = 0>
        std::remove_reference_t<R> *operator->()
        {
            return &mGetter();
        }

        /// operator* for Read or ReadWrite, getter returns a reference, const version
        template<IProperty::Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<HasRead(mode) && std::is_reference_v<R>, int> = 0>
        const std::remove_reference_t<R> &operator*() const
        {
            return mGetter();
        }

        /// operator* for Read or ReadWrite, getter returns a reference, non-const version
        template<IProperty::Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<HasRead(mode) && std::is_reference_v<R>, int> = 0>
        std::remove_reference_t<R> &operator*()
        {
            return mGetter();
        }

        // operator[] non-const version
        decltype(auto) operator[](size_t index)
        {
            static_assert(
                std::is_same_v<T, std::vector<typename T::value_type, typename T::allocator_type>>,
                "operator[] only supported for std::vector<T> for now"
                );
            return mGetter()[index];
        }

        // operator[], const version
        decltype(auto) operator[](size_t index) const
        {
            static_assert(
                std::is_same_v<T, std::vector<typename T::value_type, typename T::allocator_type>>,
                "operator[] only supported for std::vector<T> for now"
                );
            return mGetter()[index];
        }

        /// implicit conversion to const T
        template<typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<std::is_convertible_v<R, T> && (StaticMode == IProperty::Mode::Read || StaticMode == IProperty::Mode::ReadWrite),
            int> = 0>
        operator const T() const
        {
            return mGetter();
        }

        /// operator+= (lvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator+=(const T &val)
        {
            mSetter(mGetter() + val);
            return *this;
        }

        /// operator+= (rvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator+=(T &&val)
        {
            mSetter(mGetter() + std::move(val));
            return *this;
        }

        /// operator-= (lvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator-=(const T &val)
        {
            mSetter(mGetter() - val);
            return *this;
        }

        /// operator-= (rvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator-=(T &&val)
        {
            mSetter(mGetter() - std::move(val));
            return *this;
        }

        /// operator*= (lvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator*=(const T &val)
        {
            mSetter(mGetter() * val);
            return *this;
        }

        /// operator*= (rvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator*=(T &&val)
        {
            mSetter(mGetter() * std::move(val));
            return *this;
        }

        /// operator/= (lvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator/=(const T &val)
        {
            mSetter(mGetter() / val);
            return *this;
        }

        /// operator/= (rvalue)
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator/=(T &&val)
        {
            mSetter(mGetter() / std::move(val));
            return *this;
        }

        /// pre increment
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator++()
        {
            mSetter(mGetter() + T(1));
            return *this;
        }

        /// pre decrement 
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        Property &operator--()
        {
            mSetter(mGetter() - T(1));
            return *this;
        }

        /// post increment
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        T operator++(int)
        {
            T tmp = mGetter();
            mSetter(tmp + T(1));
            return tmp;
        }

        /// post decrement 
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasReadWrite(mode), int> = 0>
        T operator--(int)
        {
            T tmp = mGetter();
            mSetter(tmp - T(1));
            return tmp;
        }

        /// operator==
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasRead(mode), int> = 0>
        friend bool operator==(const Property &a, const T &b) { return a.mGetter() == b; }

        /// operator!=
        template<IProperty::Mode mode = StaticMode, std::enable_if_t<HasRead(mode), int> = 0>
        friend bool operator!=(const Property &a, const T &b) { return a.mGetter() != b; }

    private:
        /// getter method to retrieve the value
        Getter mGetter;

        /// setter method to set the value
        Setter mSetter;
    };

    /// helper function to create a property with getter and setter methods
    template<typename T, IProperty::Mode mode, typename Getter, typename Setter>
    auto MakeProperty(std::string name, Getter &&get, Setter &&set)
    {
        // type of getter function call result
        using GetterReturn = decltype(get());

        // remove reference and cv-qualifiers from getter return type for checking
        using GetterReturnDecay = std::decay_t<GetterReturn>;

        // getter return must be convertible to T or const T&
        static_assert(std::is_convertible_v<GetterReturn, T> || std::is_convertible_v<GetterReturn, const T &>,
            "Getter must return a type convertible to T or const T&");

        // setter must be callable with T&&
        static_assert(std::is_invocable_v<Setter, T &&> || std::is_invocable_v<Setter, const T &>,
            "Setter must be callable with T&& or const T&");

        // enforce getter return type consistent with Mode
        if constexpr (mode == IProperty::Mode::Write)
        {
            static_assert(std::is_void_v<GetterReturn> || std::is_same_v<GetterReturn, void>,
                "Getter must not be used in write-only property");
        }

        // make the property
        return Property<T, std::decay_t<Getter>, std::decay_t<Setter>, mode>(
            std::forward<Getter>(get),
            std::forward<Setter>(set),
            std::move(name));
    }

    /// property macro to define a property with getter and setter methods
    #define PROPERTY(TYPE, NAME, GETTER, SETTER)                                             \
    auto &NAME()                                                                             \
    {                                                                                        \
        static auto property = Lumen::MakeProperty<TYPE, Lumen::IProperty::Mode::ReadWrite>( \
            #NAME,                                                                           \
            [this]() -> decltype(auto) { return this->GETTER(); },                           \
            [this](auto &&v) { this->SETTER(std::forward<decltype(v)>(v)); });               \
        return property;                                                                     \
    }
}
