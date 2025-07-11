//==============================================================================================================================================================================
/// \file
/// \brief     Property template class for encapsulating a value with getter and setter methods
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lPropertyBase.h"

/// Lumen namespace
namespace Lumen
{
    /// Property template class
    template<typename T, PropertyDetail::Mode mode, typename Getter, typename Setter>
    class Property : public PropertyBase<T, mode, Getter, Setter, false>
    {
        using BaseType = PropertyBase<T, mode, Getter, Setter, false>;

    public:
        /// constructs a read write property
        Property(Getter get, Setter set, HashType hash, std::string_view name) requires PropertyDetail::IsReadWrite<mode>
            : BaseType(std::move(get), std::move(set), hash, name, 1) {}

        /// constructs a read only property
        Property(Getter get, HashType hash, std::string_view name) requires PropertyDetail::IsReadOnly<mode>
            : BaseType(std::move(get), hash, name, 1) {}

        /// constructs a write only property
        Property(Setter set, HashType hash, std::string_view name) requires PropertyDetail::IsWriteOnly<mode>
            : BaseType(std::move(set), hash, name, 1) {}

        /// conversion to T
        operator T() const requires PropertyDetail::HasRead<mode>
        {
            return this->Get();
        }

        /// assignment (lvalue)
        Property &operator=(const T &val) requires PropertyDetail::HasWrite<mode>
        {
            this->Set(val);
            return *this;
        }

        /// assignment (rvalue)
        Property &operator=(T &&val) requires PropertyDetail::HasWrite<mode>
        {
            this->Set(std::move(val));
            return *this;
        }

        /// assignment from another Property (lvalue)
        Property &operator=(const Property &other) requires PropertyDetail::HasWrite<mode>
        {
            this->Set(static_cast<T>(other));
            return *this;
        }

        /// assignment from another Property (rvalue)
        Property &operator=(Property &&other) requires PropertyDetail::HasWrite<mode>
        {
            this->Set(std::move(static_cast<T>(other)));
            return *this;
        }

        /// operator==
        friend bool operator==(const Property &a, const T &b) requires PropertyDetail::HasRead<mode> { return a.Get() == b; }

        /// operator!=
        friend bool operator!=(const Property &a, const T &b) requires PropertyDetail::HasRead<mode> { return a.Get() != b; }
    };

    /// property macro to define a property with getter and setter methods
    #define PROPERTY(TYPE, NAME, GETTER, SETTER)                                       \
    auto &NAME()                                                                       \
    {                                                                                  \
        using PropType = Lumen::Property<TYPE, Lumen::PropertyDetail::Mode::ReadWrite, \
            std::function<const TYPE()>, std::function<void(const TYPE &)>>;           \
        static PropType property(                                                      \
            [this]() -> TYPE { return this->GETTER(); },                               \
            [this](const TYPE &v) { this->SETTER(v); },                                \
            Lumen::PodType(#TYPE), #NAME);                                             \
        return property;                                                               \
    }

    /// read only property macro to define a property with getter method
    #define PROPERTY_RO(TYPE, NAME, GETTER)                                       \
    auto &NAME()                                                                  \
    {                                                                             \
        using PropType = Lumen::Property<TYPE, Lumen::PropertyDetail::Mode::Read, \
            std::function<const TYPE()>, std::function<void(const TYPE &)>>;      \
        static PropType property(                                                 \
            [this]() -> TYPE { return this->GETTER(); },                          \
            Lumen::PodType(#TYPE), #NAME);                                        \
        return property;                                                          \
    }

    /// write only property macro to define a property with setter method
    #define PROPERTY_WO(TYPE, NAME, SETTER)                                        \
    auto &NAME()                                                                   \
    {                                                                              \
        using PropType = Lumen::Property<TYPE, Lumen::PropertyDetail::Mode::Write, \
            std::function<const TYPE()>, std::function<void(const TYPE &)>>;       \
        static PropType property(                                                  \
            [this](const TYPE &v) { this->SETTER(v); },                            \
            Lumen::PodType(#TYPE), #NAME);                                         \
        return property;                                                           \
    }
}
