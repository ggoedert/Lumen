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

    /// Property template class
    template<typename T, PropertyDetail::Mode StaticMode = PropertyDetail::Mode::ReadWrite, typename Getter = std::monostate, typename Setter = std::monostate>
    class Property : public IProperty
    {
        CLASS_NO_DEFAULT_CTOR(Property);
        CLASS_NO_COPY_MOVE(Property);

    public:
        /// constructs a property
        Property(Getter get, Setter set, HashType hash, std::string_view name) requires PropertyDetail::IsReadWrite<StaticMode> :
            IProperty(StaticMode, hash, name, 1), mGetter(std::move(get)), mSetter(std::move(set)) {}
        Property(Getter get, HashType hash, std::string_view name) requires PropertyDetail::IsReadOnly<StaticMode> :
            IProperty(StaticMode, hash, name, 1), mGetter(std::move(get)) {}
        Property(Setter set, HashType hash, std::string_view name) requires PropertyDetail::IsWriteOnly<StaticMode> :
            IProperty(StaticMode, hash, name, 1), mSetter(std::move(set)) {}

        /// generic get, only enabled if mode has read
        T Get() const
        {
            if constexpr (StaticMode == Mode::Read || StaticMode == Mode::ReadWrite)
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
            if constexpr (StaticMode == Mode::Write || StaticMode == Mode::ReadWrite)
            {
                mSetter(value);
            }
            else
            {
                L_ASSERT_MSG(false, "Set() called on non-writable property");
            }
        }

        /// conversion to T
        operator T() const requires PropertyDetail::HasRead<StaticMode>
        {
            return mGetter();
        }

        /// assignment (lvalue)
        void operator=(const T &val) requires PropertyDetail::IsWriteOnly<StaticMode>
        {
            mSetter(val);
        }

        /// assignment (rvalue)
        void operator=(T &&val) requires PropertyDetail::IsWriteOnly<StaticMode>
        {
            mSetter(std::move(val));
        }

        /// assignment (lvalue) with return
        T operator=(const T &val) requires PropertyDetail::IsReadWrite<StaticMode>
        {
            mSetter(val);
            return mGetter();
        }

        /// assignment (rvalue) with return
        T operator=(T &&val) requires PropertyDetail::IsReadWrite<StaticMode>
        {
            mSetter(std::move(val));
            return mGetter();
        }

        /// operator==
        friend bool operator==(const Property &a, const T &b)  requires PropertyDetail::HasRead<StaticMode> { return a.mGetter() == b; }

        /// operator!=
        friend bool operator!=(const Property &a, const T &b)  requires PropertyDetail::HasRead<StaticMode> { return a.mGetter() != b; }

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
