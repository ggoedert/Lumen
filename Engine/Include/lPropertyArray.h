//==============================================================================================================================================================================
/// \file
/// \brief     PropertyArray template class for encapsulating a array with getter and setter methods
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lPropertyBase.h"

/// Lumen namespace
namespace Lumen
{
    /// PropertyArray template class
    template<typename T, PropertyDetail::Mode mode, typename Getter, typename Setter>
    class PropertyArray : public PropertyBase<T, mode, Getter, Setter, true>
    {
        using BaseType = PropertyBase<T, mode, Getter, Setter, true>;

        /// Proxy aux template class, used to provide array-like access to the property
        class Proxy
        {
            CLASS_NO_DEFAULT_CTOR(Proxy);

        public:
            /// constructs an proxy for a specific index in the array
            Proxy(std::size_t index, PropertyArray *parent) : mIndex(index), mParent(parent) {}

            /// conversion to T
            operator T() const requires PropertyDetail::HasRead<mode>
            {
                return mParent->Get(mIndex);
            }

            /// assignment (lvalue)
            Proxy &operator=(const T &val) requires PropertyDetail::HasWrite<mode>
            {
                mParent->Set(mIndex, val);
                return *this;
            }

            /// assignment (rvalue)
            Proxy &operator=(T &&val) requires PropertyDetail::HasWrite<mode>
            {
                mParent->Set(mIndex, std::move(val));
                return *this;
            }

            /// assignment from another Proxy (lvalue)
            Proxy &operator=(const Proxy &other) requires PropertyDetail::HasWrite<mode>
            {
                mParent->Set(mIndex, static_cast<T>(other));
                return *this;
            }

            /// assignment from another Proxy (rvalue)
            Proxy &operator=(Proxy &&other) requires PropertyDetail::HasWrite<mode>
            {
                mParent->Set(mIndex, std::move(static_cast<T>(other)));
                return *this;
            }

            /// operator==
            friend bool operator==(const Proxy &a, const T &b) requires PropertyDetail::HasRead<mode> { return a.mParent->Get(a.mIndex) == b; }

            /// operator!=
            friend bool operator!=(const Proxy &a, const T &b) requires PropertyDetail::HasRead<mode> { return a.mParent->Get(a.mIndex) != b; }

        private:
            /// parent array property
            PropertyArray *mParent;

            /// index in the array
            std::size_t mIndex;
        };

    public:
        /// constructs a read write array property
        PropertyArray(Getter get, Setter set, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsReadWrite<mode> 
            : BaseType(std::move(get), std::move(set), hash, name, size) {}

        /// constructs a read only array property
        PropertyArray(Getter get, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsReadOnly<mode> 
            : BaseType(std::move(get), hash, name, size) {}

        /// constructs a write only array property
        PropertyArray(Setter set, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsWriteOnly<mode> 
            : BaseType(std::move(set), hash, name, size) {}

        /// conversion to std::vector<T>
        operator std::vector<T>() const requires PropertyDetail::HasRead<mode>
        {
            const std::size_t count = this->Size();
            std::vector<T> vec;
            vec.reserve(count);

            for (std::size_t i = 0; i < count; ++i)
            {
                vec.emplace_back(this->Get(i));
            }

            return vec;
        }

        /// array-like assignment
        PropertyArray &operator=(std::initializer_list<T> list) requires PropertyDetail::HasWrite<mode>
        {
            std::size_t i = 0;
            for (const auto &val : list)
            {
                if (i >= this->Size()) break;  // Prevent out-of-bounds
                this->Set(i, val);
                ++i;
            }
            return *this;
        }

        /// array-like access operator
        Proxy operator[](std::size_t index)
        {
            return Proxy(index, this);
        }
    };

    /// array property macro to define a array property with getter and setter methods
    #define PROPERTYARRAY(TYPE, NAME, SIZE, GETTER, SETTER)                                   \
    auto& NAME() {                                                                            \
        using PropType = Lumen::PropertyArray<TYPE, Lumen::PropertyDetail::Mode::ReadWrite,   \
            std::function<TYPE(std::size_t)>, std::function<void(std::size_t, const TYPE&)>>; \
        static PropType arrayProperty(                                                        \
            [this](std::size_t i) -> TYPE { return this->GETTER(i); },                        \
            [this](std::size_t i, const TYPE& v) { this->SETTER(i, v); },                     \
            Lumen::PodType(#TYPE), #NAME, SIZE);                                              \
        return arrayProperty;                                                                 \
    }

    /// read only array property macro to define a array property with getter method
    #define PROPERTYARRAY_RO(TYPE, NAME, SIZE, GETTER)                                        \
    auto& NAME() {                                                                            \
        using PropType = Lumen::PropertyArray<TYPE, Lumen::PropertyDetail::Mode::Read,        \
            std::function<TYPE(std::size_t)>, std::function<void(std::size_t, const TYPE&)>>; \
        static PropType arrayProperty(                                                        \
            [this](std::size_t i) -> TYPE { return this->GETTER(i); },                        \
            Lumen::PodType(#TYPE), #NAME, SIZE);                                              \
        return arrayProperty;                                                                 \
    }

    /// write only array property macro to define a array property with setter method
    #define PROPERTYARRAY_WO(TYPE, NAME, SIZE, SETTER)                                        \
    auto& NAME() {                                                                            \
        using PropType = Lumen::PropertyArray<TYPE, Lumen::PropertyDetail::Mode::Write,       \
            std::function<TYPE(std::size_t)>, std::function<void(std::size_t, const TYPE&)>>; \
        static PropType arrayProperty(                                                        \
            [this](std::size_t i, const TYPE& v) { this->SETTER(i, v); },                     \
            Lumen::PodType(#TYPE), #NAME, SIZE);                                              \
        return arrayProperty;                                                                 \
    }
}
