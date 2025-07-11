//==============================================================================================================================================================================
/// \file
/// \brief     PropertyBase generic template class and interface for encapsulating a value with getter and setter methods
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
        template<Mode mode> concept IsReadOnly = (mode == Mode::Read);

        /// compile-time write only mode check
        template<Mode mode> concept IsWriteOnly = (mode == Mode::Write);

        /// compile-time read/write mode check
        template<Mode mode> concept IsReadWrite = (mode == Mode::ReadWrite);

        /// compile-time has read mode check
        template<Mode mode> concept HasRead = (mode == Mode::Read || mode == Mode::ReadWrite);

        /// compile-time has write mode check
        template<Mode mode> concept HasWrite = (mode == Mode::Write || mode == Mode::ReadWrite);
    }

    /// IProperty interface class
    class IProperty
    {
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
    class PropertyGetter
    {
    public:
        /// default constructor
        explicit PropertyGetter() = default;
    };

    /// Specialized Property base class with getter only if HasRead == true
    template<typename T, PropertyDetail::Mode mode, typename Getter>
    class PropertyGetter<T, mode, Getter, true>
    {
    public:
        /// default constructor
        explicit PropertyGetter() = default;

        /// generic get, only enabled if mode has read
        template<typename... CallArgs>
        T Get(CallArgs&&... args) const
        {
            return mGetter(std::forward<CallArgs>(args)...);
        }

    protected:
        /// getter method to retrieve the value
        Getter mGetter;
    };

    /// Property empty base with no setter
    template<typename T, PropertyDetail::Mode mode, typename Setter, bool HasWrite = PropertyDetail::HasWrite<mode>>
    class PropertySetter
    {
    public:
        /// default constructor
        explicit PropertySetter() = default;
    };

    /// Specialized Property base class with setter only if HasWrite == true
    template<typename T, PropertyDetail::Mode mode, typename Setter>
    class PropertySetter<T, mode, Setter, true>
    {
    public:
        /// default constructor
        explicit PropertySetter() = default;

        /// generic set, only enabled if mode has write
        template<typename... CallArgs>
        void Set(CallArgs&&... args)
        {
            mSetter(std::forward<CallArgs>(args)...);
        }

    protected:
        /// setter method to set the value
        Setter mSetter;
    };

    /// Core Property template
    template<typename T, PropertyDetail::Mode mode, typename Getter, typename Setter, bool IsArray = false>
    class PropertyBase : public PropertyGetter<T, mode, Getter>, public PropertySetter<T, mode, Setter>, public IProperty
    {
        CLASS_NO_DEFAULT_CTOR(PropertyBase);

    public:
        /// constructs a read write property
        PropertyBase(Getter get, Setter set, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsReadWrite<mode> :
            IProperty(mode, hash, name, size)
        {
            if constexpr (PropertyDetail::HasRead<mode>)
                this->mGetter = std::move(get);
            if constexpr (PropertyDetail::HasWrite<mode>)
                this->mSetter = std::move(set);
        }

        /// constructs a read only property
        PropertyBase(Getter get, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsReadOnly<mode> :
        IProperty(mode, hash, name, size)
        {
            if constexpr (PropertyDetail::HasRead<mode>)
                this->mGetter = std::move(get);
        }

        /// constructs a write only property
        PropertyBase(Setter set, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsWriteOnly<mode> :
        IProperty(mode, hash, name, size)
        {
            if constexpr (PropertyDetail::HasWrite<mode>)
                this->mSetter = std::move(set);
        }
    };
}
