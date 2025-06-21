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
    /// Property helper utilities
    namespace PropertyDetail
    {
        /// access modes for properties
        enum class Mode { Read, Write, ReadWrite };

        /// compile-time read mode check
        template<Mode mode> inline constexpr bool HasRead = (mode == Mode::Read || mode == Mode::ReadWrite);

        /// compile-time write mode check
        template<Mode mode> inline constexpr bool HasWrite = (mode == Mode::Write || mode == Mode::ReadWrite);

        /// compile-time read/write mode check
        template<Mode mode> inline constexpr bool HasReadWrite = (mode == Mode::ReadWrite);
    }

    /// IProperty base class
    class IProperty
    {
        CLASS_NO_COPY_MOVE(IProperty);
        using Mode = PropertyDetail::Mode;

    public:
        /// constructs a IProperty
        IProperty(Mode mode, std::string name) : mMode(mode), mName(std::move(name)) {}

        /// default destructor
        virtual ~IProperty() = default;

        /// checks if the property is of a specific type
        virtual bool IsTypeId(const std::type_info &typeId) const = 0;

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
    template<typename T, PropertyDetail::Mode StaticMode = PropertyDetail::Mode::ReadWrite, typename Getter = std::monostate, typename Setter = std::monostate>
    class Property : public IProperty
    {
        CLASS_NO_DEFAULT_CTOR(Property);
        using Mode = PropertyDetail::Mode;

    public:
        /// constructs a property
        Property(Getter get, Setter set, std::string name) : IProperty(StaticMode, std::move(name)), mGetter(std::move(get)), mSetter(std::move(set)) {}
        Property(Getter get, std::string name) : IProperty(StaticMode, std::move(name)), mGetter(std::move(get)) {}
        Property(Setter set, std::string name) : IProperty(StaticMode, std::move(name)), mSetter(std::move(set)) {}

        /// returns the type id of the property
        bool IsTypeId(const std::type_info &typeId) const override { return typeId == typeid(T); }

        /// generic get, only enabled if mode has read
        std::any Get() const override
        {
            if constexpr (StaticMode == PropertyDetail::Mode::Read || StaticMode == PropertyDetail::Mode::ReadWrite)
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
            if constexpr (StaticMode == PropertyDetail::Mode::Write || StaticMode == PropertyDetail::Mode::ReadWrite)
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
        template<Mode mode = StaticMode, typename = std::enable_if_t<PropertyDetail::HasWrite<mode>>>
        Property &operator=(const T &val)
        {
            mSetter(val);
            return *this;
        }

        /// assignment (rvalue)
        template<Mode mode = StaticMode, typename = std::enable_if_t<PropertyDetail::HasWrite<mode>>>
        Property &operator=(T &&val)
        {
            mSetter(std::move(val));
            return *this;
        }

        /// operator-> for Read or ReadWrite, const version
        template<Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<PropertyDetail::HasRead<mode> && std::is_reference_v<R>, int> = 0>
        const std::remove_reference_t<R> *operator->() const
        {
            return &mGetter();
        }

        /// operator-> for Read or ReadWrite, non-const version
        template<Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<PropertyDetail::HasRead<mode> && std::is_reference_v<R>, int> = 0>
        std::remove_reference_t<R> *operator->()
        {
            return &mGetter();
        }

        /// operator* for Read or ReadWrite, getter returns a reference, const version
        template<Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<PropertyDetail::HasRead<mode> && std::is_reference_v<R>, int> = 0>
        const std::remove_reference_t<R> &operator*() const
        {
            return mGetter();
        }

        /// operator* for Read or ReadWrite, getter returns a reference, non-const version
        template<Mode mode = StaticMode,
            typename U = Getter, typename R = decltype(std::declval<U>()()),
            std::enable_if_t<PropertyDetail::HasRead<mode> && std::is_reference_v<R>, int> = 0>
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
            std::enable_if_t<std::is_convertible_v<R, T> && (StaticMode == PropertyDetail::Mode::Read || StaticMode == PropertyDetail::Mode::ReadWrite),
            int> = 0>
        operator const T() const
        {
            return mGetter();
        }

        /// operator+= (lvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator+=(const T &val)
        {
            mSetter(mGetter() + val);
            return *this;
        }

        /// operator+= (rvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator+=(T &&val)
        {
            mSetter(mGetter() + std::move(val));
            return *this;
        }

        /// operator-= (lvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator-=(const T &val)
        {
            mSetter(mGetter() - val);
            return *this;
        }

        /// operator-= (rvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator-=(T &&val)
        {
            mSetter(mGetter() - std::move(val));
            return *this;
        }

        /// operator*= (lvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator*=(const T &val)
        {
            mSetter(mGetter() * val);
            return *this;
        }

        /// operator*= (rvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator*=(T &&val)
        {
            mSetter(mGetter() * std::move(val));
            return *this;
        }

        /// operator/= (lvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator/=(const T &val)
        {
            mSetter(mGetter() / val);
            return *this;
        }

        /// operator/= (rvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator/=(T &&val)
        {
            mSetter(mGetter() / std::move(val));
            return *this;
        }

        /// pre increment
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator++()
        {
            mSetter(mGetter() + T(1));
            return *this;
        }

        /// pre decrement 
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        Property &operator--()
        {
            mSetter(mGetter() - T(1));
            return *this;
        }

        /// post increment
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        T operator++(int)
        {
            T tmp = mGetter();
            mSetter(tmp + T(1));
            return tmp;
        }

        /// post decrement 
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasReadWrite<mode>, int> = 0>
        T operator--(int)
        {
            T tmp = mGetter();
            mSetter(tmp - T(1));
            return tmp;
        }

        /// operator==
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasRead<mode>, int> = 0>
        friend bool operator==(const Property &a, const T &b) { return a.mGetter() == b; }

        /// operator!=
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasRead<mode>, int> = 0>
        friend bool operator!=(const Property &a, const T &b) { return a.mGetter() != b; }

    private:
        /// getter method to retrieve the value
        [[no_unique_address]] Getter mGetter;

        /// setter method to set the value
        [[no_unique_address]] Setter mSetter;
    };

    /// helper function to create a property with getter and setter methods
    template<typename T, PropertyDetail::Mode mode, typename Getter, typename Setter>
    auto MakeProperty(std::string name, Getter &&get, Setter &&set)
    {
        if constexpr (mode == PropertyDetail::Mode::ReadWrite)
            return Property<T, mode, std::decay_t<Getter>, std::decay_t<Setter>>(std::forward<Getter>(get), std::forward<Setter>(set), std::move(name));
        else if constexpr (mode == PropertyDetail::Mode::Read)
            return Property<T, mode, std::decay_t<Getter>, std::monostate>(std::forward<Getter>(get), std::move(name));
        else if constexpr (mode == PropertyDetail::Mode::Write)
            return Property<T, mode, std::monostate, std::decay_t<Setter>>(std::forward<Setter>(set), std::move(name));
    }

    /// property macro to define a property with getter and setter methods
    #define PROPERTY(TYPE, NAME, GETTER, SETTER)                                                  \
    auto &NAME()                                                                                  \
    {                                                                                             \
        static auto property = Lumen::MakeProperty<TYPE, Lumen::PropertyDetail::Mode::ReadWrite>( \
            #NAME,                                                                                \
            [this]() -> decltype(auto) { return this->GETTER(); },                                \
            [this](auto &&v) { this->SETTER(std::forward<decltype(v)>(v)); });                    \
        return property;                                                                          \
    }

    /// read only property macro to define a property with getter method
    #define PROPERTY_RO(TYPE, NAME, GETTER)                                                  \
    auto &NAME()                                                                             \
    {                                                                                        \
        static auto property = Lumen::MakeProperty<TYPE, Lumen::PropertyDetail::Mode::Read>( \
            #NAME,                                                                           \
            [this]() -> decltype(auto) { return this->GETTER(); },                           \
            std::monostate {});                                                              \
        return property;                                                                     \
    }

    /// write only property macro to define a property with setter method
    #define PROPERTY_WO(TYPE, NAME, SETTER)                                                    \
    auto &NAME()                                                                               \
    {                                                                                          \
        static auto property = Lumen::MakeProperty<TYPE, Lumen::PropertyDetail::Mode::Write>(  \
            #NAME,                                                                             \
            std::monostate {},                                                                 \
            [this](auto &&v) { this->SETTER(std::forward<decltype(v)>(v)); });                 \
        return property;                                                                       \
    }
}
