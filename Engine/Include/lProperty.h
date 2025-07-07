//==============================================================================================================================================================================
/// \file
/// \brief     Property template class for encapsulating a value with getter and setter methods
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    /// IProperty interface class
    class IProperty
    {
        CLASS_NO_COPY_MOVE(IProperty);
        using Mode = PropertyDetail::Mode;

    public:
        /// constructs a IProperty
        IProperty(Mode mode, HashType hash, std::string_view name, std::size_t size) : mMode(mode), mHash(hash), mName(name), mSize(size) {}

        /// default destructor
        virtual ~IProperty() noexcept = default;

        /// read only mode check
        bool IsReadOnly() { return mMode == Mode::Read; }

        /// write only mode check
        bool IsWriteOnly() { return mMode == Mode::Write; }

        /// read/write mode check
        bool IsReadWrite() { return mMode == Mode::ReadWrite; }

        /// has read mode check
        bool HasRead() { return mMode == Mode::Read || mMode == Mode::ReadWrite; }

        /// has write mode check
        bool HasWrite() { return mMode == Mode::Write || mMode == Mode::ReadWrite; }

        /// return type
        HashType Type() { return mHash; }

        /// return property name
        std::string_view Name() const { return mName; }

        /// return property size
        std::size_t Size() const { return mSize; }

    protected:
        /// property access mode
        Mode mMode;

        /// property hash
        HashType mHash;

        /// property name
        std::string mName;

        /// property size
        std::size_t mSize;
    };

    /// Property empty base with no getter
    template<typename T, PropertyDetail::Mode mode, typename Getter, bool HasRead = PropertyDetail::HasRead<mode>>
    class PropertyGetter {
        CLASS_NO_COPY_MOVE(PropertyGetter);

    public:
        /// default constructor
        explicit PropertyGetter() = default;
    };

    /// Specialized Property base class with getter only if HasRead == true
    template<typename T, PropertyDetail::Mode mode, typename Getter>
    class PropertyGetter<T, mode, Getter, true>
    {
        CLASS_NO_COPY_MOVE(PropertyGetter);

    public:
        /// default constructor
        explicit PropertyGetter() = default;

        /// generic get, only enabled if mode has read
        T Get() const
        {
            return mGetter();
        }

    protected:
        /// getter method to retrieve the value
        Getter mGetter;
    };

    /// Property empty base with no setter
    template<typename T, PropertyDetail::Mode mode, typename Setter, bool HasWrite = PropertyDetail::HasWrite<mode>>
    class PropertySetter {
        CLASS_NO_COPY_MOVE(PropertySetter);

    public:
        /// default constructor
        explicit PropertySetter() = default;
    };

    /// Specialized Property base class with setter only if HasWrite == true
    template<typename T, PropertyDetail::Mode mode, typename Setter>
    class PropertySetter<T, mode, Setter, true>
    {
        CLASS_NO_COPY_MOVE(PropertySetter);

    public:
        /// default constructor
        explicit PropertySetter() = default;

        /// generic set, only enabled if mode has write
        void Set(const T &value)
        {
            mSetter(value);
        }

    protected:
        /// setter method to set the value
        Setter mSetter;
    };

    /// Property template class
    template<typename T, PropertyDetail::Mode mode = PropertyDetail::Mode::ReadWrite, typename Getter = std::monostate, typename Setter = std::monostate>
    class Property : public PropertyGetter<T, mode, Getter>, public PropertySetter<T, mode, Setter>, public IProperty
    {
        CLASS_NO_DEFAULT_CTOR(Property);
        CLASS_NO_COPY_MOVE(Property);

    public:
        /// constructs a read write property
        Property(Getter get, Setter set, HashType hash, std::string_view name) requires PropertyDetail::IsReadWrite<mode> : IProperty(mode, hash, name, 1)
        {
            if constexpr (PropertyDetail::HasRead<mode>)
                this->mGetter = std::move(get);
            if constexpr (PropertyDetail::HasWrite<mode>)
                this->mSetter = std::move(set);
        }

        /// constructs a read only property
        Property(Getter get, HashType hash, std::string_view name) requires PropertyDetail::IsReadOnly<mode> : IProperty(mode, hash, name, 1)
        {
            if constexpr (PropertyDetail::HasRead<mode>)
                this->mGetter = std::move(get);
        }

        /// constructs a write only property
        Property(Setter set, HashType hash, std::string_view name) requires PropertyDetail::IsWriteOnly<mode> : IProperty(mode, hash, name, 1)
        {
            if constexpr (PropertyDetail::HasWrite<mode>)
                this->mSetter = std::move(set);
        }

        /// conversion to T
        operator T() const requires PropertyDetail::HasRead<mode>
        {
            return PropertyGetter<T, mode, Getter>::Get();
        }

        /// assignment (lvalue)
        void operator=(const T &val) requires PropertyDetail::IsWriteOnly<mode>
        {
            PropertySetter<T, mode, Setter>::Set(val);
        }

        /// assignment (rvalue)
        void operator=(T &&val) requires PropertyDetail::IsWriteOnly<mode>
        {
            PropertySetter<T, mode, Setter>::Set(std::move(val));
        }

        /// assignment (lvalue) with return
        T operator=(const T &val) requires PropertyDetail::IsReadWrite<mode>
        {
            PropertySetter<T, mode, Setter>::Set(val);
            return PropertyGetter<T, mode, Getter>::Get();
        }

        /// assignment (rvalue) with return
        T operator=(T &&val) requires PropertyDetail::IsReadWrite<mode>
        {
            PropertySetter<T, mode, Setter>::Set(std::move(val));
            return PropertyGetter<T, mode, Getter>::Get();
        }

        /// operator==
        friend bool operator==(const Property &a, const T &b)  requires PropertyDetail::HasRead<mode> { return a.PropertyGetter<T, mode, Getter>::Get() == b; }

        /// operator!=
        friend bool operator!=(const Property &a, const T &b)  requires PropertyDetail::HasRead<mode> { return a.PropertyGetter<T, mode, Getter>::Get() != b; }
    };

    /// property macro to define a property with getter and setter methods
    #define PROPERTY(TYPE, NAME, GETTER, SETTER)                                                                 \
    auto &NAME()                                                                                                 \
    {                                                                                                            \
        using PropType = Lumen::Property<TYPE, Lumen::PropertyDetail::Mode::ReadWrite,                           \
            std::function<const TYPE()>, std::function<void(const TYPE &)>>;                                     \
        static PropType property(                                                                                \
            std::function<const TYPE()>([this]() -> decltype(auto) { return this->GETTER(); }),                  \
            std::function<void(const TYPE &)>([this](auto &&v) { this->SETTER(std::forward<decltype(v)>(v)); }), \
            Lumen::PodType(#TYPE), #NAME);                                                                       \
        return property;                                                                                         \
    }

    /// read only property macro to define a property with getter method
    #define PROPERTY_RO(TYPE, NAME, GETTER)                                                     \
    auto &NAME()                                                                                \
    {                                                                                           \
        using PropType = Lumen::Property<TYPE, Lumen::PropertyDetail::Mode::Read,               \
            std::function<const TYPE()>, std::function<void(const TYPE &)>>;                    \
        static PropType property(                                                               \
            std::function<const TYPE()>([this]() -> decltype(auto) { return this->GETTER(); }), \
            Lumen::PodType(#TYPE), #NAME);                                                      \
        return property;                                                                        \
    }

    /// write only property macro to define a property with setter method
    #define PROPERTY_WO(TYPE, NAME, SETTER)                                                                      \
    auto &NAME()                                                                                                 \
    {                                                                                                            \
        using PropType = Lumen::Property<TYPE, Lumen::PropertyDetail::Mode::Write,                               \
            std::function<const TYPE()>, std::function<void(const TYPE &)>>;                                     \
        static PropType property(                                                                                \
            std::function<void(const TYPE &)>([this](auto &&v) { this->SETTER(std::forward<decltype(v)>(v)); }), \
            Lumen::PodType(#TYPE), #NAME);                                                                       \
        return property;                                                                                         \
    }
}
