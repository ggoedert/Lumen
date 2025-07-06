//==============================================================================================================================================================================
/// \file
/// \brief     ArrayProperty template class for encapsulating a array with getter and setter methods
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lProperty.h"

/// Lumen namespace
namespace Lumen
{
    /// ArrayProperty template class
    template<typename T, PropertyDetail::Mode StaticMode = PropertyDetail::Mode::ReadWrite, typename Getter = std::monostate, typename Setter = std::monostate>
    class ArrayProperty : public IProperty
    {
        CLASS_NO_DEFAULT_CTOR(ArrayProperty);
        CLASS_NO_COPY_MOVE(ArrayProperty);

        /// Proxy aux template class, used to provide array-like access to the property
        class Proxy
        {
            CLASS_NO_DEFAULT_CTOR(Proxy);

        public:
            /// constructs an proxy for a specific index in the array
            Proxy(std::size_t index, ArrayProperty *parent) : mIndex(index), mParent(parent) {}

            /// conversion to T
            operator T() const requires PropertyDetail::HasRead<StaticMode>
            {
                return mParent->mGetter(mIndex);
            }

            /// assignment (lvalue)
            void operator=(const T &val) requires PropertyDetail::IsWriteOnly<StaticMode>
            {
                mParent->mSetter(mIndex, val);
            }

            /// assignment (rvalue)
            void operator=(T &&val) requires PropertyDetail::IsWriteOnly<StaticMode>
            {
                mParent->mSetter(mIndex, std::move(val));
            }

            /// assignment (lvalue) with return
            T operator=(const T &val) requires PropertyDetail::IsReadWrite<StaticMode>
            {
                mParent->mSetter(mIndex, val);
                return mParent->mGetter(mIndex);
            }

            /// assignment (rvalue) with return
            T operator=(T &&val) requires PropertyDetail::IsReadWrite<StaticMode>
            {
                mParent->mSetter(mIndex, std::move(val));
                return mParent->mGetter(mIndex);
            }

            /// assignment from Proxy (rvalue) with return
            T operator=(Proxy &&proxy) requires PropertyDetail::IsReadWrite<StaticMode>
            {
                mParent->mSetter(mIndex, static_cast<T>(proxy));
                return mParent->mGetter(mIndex);
            }

            /// operator==
            friend bool operator==(const Proxy &a, const T &b)  requires PropertyDetail::HasRead<StaticMode> { return a.mParent->mGetter(mIndex) == b; }

            /// operator!=
            friend bool operator!=(const Proxy &a, const T &b)  requires PropertyDetail::HasRead<StaticMode> { return a.mParent->mGetter(mIndex) != b; }

        private:
            /// parent array property
            ArrayProperty *mParent;

            /// index in the array
            std::size_t mIndex;
        };

    public:
        /// constructs an array property
        ArrayProperty(Getter get, Setter set, HashType hash, std::string_view name, std::size_t size) :
            IProperty(StaticMode, hash, name, size), mGetter(std::move(get)), mSetter(std::move(set)) {}
        ArrayProperty(Getter get, HashType hash, std::string_view name, std::size_t size) : IProperty(StaticMode, hash, name, size), mGetter(std::move(get)) {}
        ArrayProperty(Setter set, HashType hash, std::string_view name, std::size_t size) : IProperty(StaticMode, hash, name, size), mSetter(std::move(set)) {}

        /// generic get, only enabled if mode has read
        T Get(const std::size_t index) const
        {
            if constexpr (StaticMode == PropertyDetail::Mode::Read || StaticMode == PropertyDetail::Mode::ReadWrite)
            {
                return mGetter(index);
            }
            else
            {
                L_ASSERT_MSG(false, "Get() called on non-readable array property");
            }
        }

        /// generic set, only enabled if mode has write
        void Set(const std::size_t index, const T &value)
        {
            if constexpr (StaticMode == PropertyDetail::Mode::Write || StaticMode == PropertyDetail::Mode::ReadWrite)
            {
                mSetter(index, value);
            }
            else
            {
                L_ASSERT_MSG(false, "Set() called on non-writable array property");
            }
        }

        /// conversion to std::vector<T>
        operator std::vector<T>() const
        {
            const std::size_t count = this->Size();
            std::vector<T> vec;
            vec.reserve(count);

            for (std::size_t i = 0; i < count; ++i)
            {
                vec.emplace_back(mGetter(i));
            }

            return vec;
        }

        /// array-like assigment
        template<typename U = T, Mode mode = StaticMode>
        ArrayProperty &operator=(std::initializer_list<U> list) requires PropertyDetail::HasWrite<StaticMode>
        {
            std::size_t i = 0;
            for (const auto &val : list)
            {
                if (i >= this->Size()) break;  // Prevent out-of-bounds
                mSetter(i, val);
                ++i;
            }
            return *this;
        }

        /// array-like access operator
        Proxy operator[](std::size_t index)
        {
            return Proxy(index, this);
        }

    private:
        /// getter method to retrieve the value
        [[no_unique_address]] Getter mGetter;

        /// setter method to set the value
        [[no_unique_address]] Setter mSetter;
    };


    /// array property macro to define a array property with getter and setter methods
    #define ARRAYPROPERTY(TYPE, NAME, SIZE, GETTER, SETTER)                                   \
    auto& NAME() {                                                                            \
        using PropType = Lumen::ArrayProperty<TYPE, Lumen::PropertyDetail::Mode::ReadWrite,   \
            std::function<TYPE(std::size_t)>, std::function<void(std::size_t, const TYPE&)>>; \
        static PropType arrayProperty(                                                        \
            [this](std::size_t i) { return this->GETTER(i); },                                \
            [this](std::size_t i, const TYPE& v) { this->SETTER(i, v); },                     \
            Lumen::PodType(#TYPE), #NAME, SIZE);                                              \
        return arrayProperty;                                                                 \
    }

    /// read only array property macro to define a array property with getter method
    #define ARRAYPROPERTY_RO(TYPE, NAME, SIZE, GETTER)                                        \
    auto& NAME() {                                                                            \
        using PropType = Lumen::ArrayProperty<TYPE, Lumen::PropertyDetail::Mode::Read,        \
            std::function<TYPE(std::size_t)>, std::function<void(std::size_t, const TYPE&)>>; \
        static PropType arrayProperty(                                                        \
            [this](std::size_t i) { return this->GETTER(i); },                                \
            Lumen::PodType(#TYPE), #NAME, SIZE);                                              \
        return arrayProperty;                                                                 \
    }

    /// write only array property macro to define a array property with setter method
    #define ARRAYPROPERTY_WO(TYPE, NAME, SIZE, SETTER)                                        \
    auto& NAME() {                                                                            \
        using PropType = Lumen::ArrayProperty<TYPE, Lumen::PropertyDetail::Mode::Write,       \
            std::function<TYPE(std::size_t)>, std::function<void(std::size_t, const TYPE&)>>; \
        static PropType arrayProperty(                                                        \
            [this](std::size_t i, const TYPE& v) { this->SETTER(i, v); },                     \
            Lumen::PodType(#TYPE), #NAME, SIZE);                                              \
        return arrayProperty;                                                                 \
    }
}
