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
    /// Property helper utilities
    namespace PropertyDetail
    {
        /// access modes for properties
        enum class Mode { Read, Write, ReadWrite };

        /// compile-time read only mode check
        template<Mode mode> inline constexpr bool IsReadOnly = (mode == Mode::Read);

        /// compile-time write only mode check
        template<Mode mode> inline constexpr bool IsWriteOnly = (mode == Mode::Write);

        /// compile-time read/write mode check
        template<Mode mode> inline constexpr bool IsReadWrite = (mode == Mode::ReadWrite);

        /// compile-time has read mode check
        template<Mode mode> inline constexpr bool HasRead = (mode == Mode::Read || mode == Mode::ReadWrite);

        /// compile-time has write mode check
        template<Mode mode> inline constexpr bool HasWrite = (mode == Mode::Write || mode == Mode::ReadWrite);
    }

    /// IProperty interface class
    class IProperty
    {
        CLASS_NO_COPY_MOVE(IProperty);
        using Mode = PropertyDetail::Mode;

    public:
        /// constructs a IProperty
        IProperty(Mode mode, HashType hash, std::string_view name) : mMode(mode), mHash(hash), mName(name) {}

        /// default destructor
        virtual ~IProperty() = default;

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

    protected:
        /// property access mode
        Mode mMode;

        /// property hash
        HashType mHash;

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
        Property(Getter get, Setter set, HashType hash, std::string_view name) : IProperty(StaticMode, hash, name), mGetter(std::move(get)), mSetter(std::move(set)) {}
        Property(Getter get, HashType hash, std::string_view name) : IProperty(StaticMode, hash, name), mGetter(std::move(get)) {}
        Property(Setter set, HashType hash, std::string_view name) : IProperty(StaticMode, hash, name), mSetter(std::move(set)) {}

        /// generic get, only enabled if mode has read
        T Get() const
        {
            if constexpr (StaticMode == PropertyDetail::Mode::Read || StaticMode == PropertyDetail::Mode::ReadWrite)
            {
                return mGetter();
            }
            else
            {
                L_ASSERT_MSG(false, "Get() called on non-readable property");
            }
        }

        /// generic set, only enabled if mode has write
        void Set(const T &value)
        {
            if constexpr (StaticMode == PropertyDetail::Mode::Write || StaticMode == PropertyDetail::Mode::ReadWrite)
            {
                mSetter(value);
            }
            else
            {
                L_ASSERT_MSG(false, "Set() called on non-writable property");
            }
        }

        /// conversion to T
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::HasRead<mode>, int> = 0>
        operator T() const
        {
            return mGetter();
        }

        /// assignment (lvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::IsWriteOnly<mode>, int> = 0>
        void operator=(const T &val)
        {
            mSetter(val);
        }

        /// assignment (rvalue)
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::IsWriteOnly<mode>, int> = 0>
        void operator=(T &&val)
        {
            mSetter(std::move(val));
        }

        /// assignment (lvalue) with return
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::IsReadWrite<mode>, int> = 0>
        T operator=(const T &val)
        {
            mSetter(val);
            return mGetter();
        }

        /// assignment (rvalue) with return
        template<Mode mode = StaticMode, std::enable_if_t<PropertyDetail::IsReadWrite<mode>, int> = 0>
        T operator=(T &&val)
        {
            mSetter(std::move(val));
            return mGetter();
        }

        /* we actually need an specialization for vectors, but not for now
        /// operator[] for vectors
        template<Mode mode = StaticMode, typename U = T,
            std::enable_if_t<PropertyDetail::HasRead<mode> &&std::is_same_v<U, std::vector<typename U::value_type, typename U::allocator_type>>, int> = 0>
        decltype(auto) operator[](size_t index) const
        {
            return mGetter()[index];
        }
        */

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
