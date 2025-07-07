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
    /// ArrayProperty empty base with no getter
    template<typename T, PropertyDetail::Mode mode, typename Getter, bool HasRead = PropertyDetail::HasRead<mode>>
    class ArrayPropertyGetter
    {
        CLASS_NO_COPY_MOVE(ArrayPropertyGetter);

    public:
        /// default constructor
        explicit ArrayPropertyGetter() = default;
    };

    /// Specialized ArrayProperty base class with getter only if HasRead == true
    template<typename T, PropertyDetail::Mode mode, typename Getter>
    class ArrayPropertyGetter<T, mode, Getter, true>
    {
        CLASS_NO_COPY_MOVE(ArrayPropertyGetter);

    public:
        /// default constructor
        explicit ArrayPropertyGetter() = default;

        /// generic get, only enabled if mode has read
        T Get(std::size_t index) const
        {
            return mGetter(index);
        }

    protected:
        /// getter method to retrieve the value
        Getter mGetter;
    };

    /// ArrayProperty empty base with no setter
    template<typename T, PropertyDetail::Mode mode, typename Setter, bool HasWrite = PropertyDetail::HasWrite<mode>>
    class ArrayPropertySetter
    {
        CLASS_NO_COPY_MOVE(ArrayPropertySetter);

    public:
        /// default constructor
        explicit ArrayPropertySetter() = default;
    };

    /// Specialized ArrayProperty base class with setter only if HasWrite == true
    template<typename T, PropertyDetail::Mode mode, typename Setter>
    class ArrayPropertySetter<T, mode, Setter, true>
    {
        CLASS_NO_COPY_MOVE(ArrayPropertySetter);

    public:
        /// default constructor
        explicit ArrayPropertySetter() = default;

        /// generic set, only enabled if mode has write
        void Set(std::size_t index, const T &value)
        {
            mSetter(index, value);
        }

    protected:
        /// setter method to set the value
        Setter mSetter;
    };

    /// ArrayProperty template class
    template<typename T, PropertyDetail::Mode mode = PropertyDetail::Mode::ReadWrite, typename Getter = std::monostate, typename Setter = std::monostate>
    class ArrayProperty : public ArrayPropertyGetter<T, mode, Getter>, public ArrayPropertySetter<T, mode, Setter>, public IProperty
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
            operator T() const requires PropertyDetail::HasRead<mode>
            {
                return mParent->mGetter(mIndex);
            }

            /// assignment (lvalue)
            void operator=(const T &val) requires PropertyDetail::IsWriteOnly<mode>
            {
                mParent->mSetter(mIndex, val);
            }

            /// assignment (rvalue)
            void operator=(T &&val) requires PropertyDetail::IsWriteOnly<mode>
            {
                mParent->mSetter(mIndex, std::move(val));
            }

            /// assignment (lvalue) with return
            T operator=(const T &val) requires PropertyDetail::IsReadWrite<mode>
            {
                mParent->mSetter(mIndex, val);
                return mParent->mGetter(mIndex);
            }

            /// assignment (rvalue) with return
            T operator=(T &&val) requires PropertyDetail::IsReadWrite<mode>
            {
                mParent->mSetter(mIndex, std::move(val));
                return mParent->mGetter(mIndex);
            }

            /// assignment from Proxy (rvalue) with return
            T operator=(Proxy &&proxy) requires PropertyDetail::IsReadWrite<mode>
            {
                mParent->mSetter(mIndex, static_cast<T>(proxy));
                return mParent->mGetter(mIndex);
            }

            /// operator==
            friend bool operator==(const Proxy &a, const T &b)  requires PropertyDetail::HasRead<mode> { return a.mParent->mGetter(mIndex) == b; }

            /// operator!=
            friend bool operator!=(const Proxy &a, const T &b)  requires PropertyDetail::HasRead<mode> { return a.mParent->mGetter(mIndex) != b; }

        private:
            /// parent array property
            ArrayProperty *mParent;

            /// index in the array
            std::size_t mIndex;
        };

    public:
        /// constructs a read write array property
        ArrayProperty(Getter get, Setter set, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsReadWrite<mode> :
            IProperty(mode, hash, name, size)
        {
            if constexpr (PropertyDetail::HasRead<mode>)
                ArrayPropertyGetter<T, mode, Getter, true>::mGetter = std::move(get);
            if constexpr (PropertyDetail::HasWrite<mode>)
                this->mSetter = std::move(set);
        }

        /// constructs a read only array property
        ArrayProperty(Getter get, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsReadOnly<mode> :
            IProperty(mode, hash, name, size)
        {
            if constexpr (PropertyDetail::HasRead<mode>)
                ArrayPropertyGetter<T, mode, Getter, true>::mGetter = std::move(get);
        }

        /// constructs a write only array property
        ArrayProperty(Setter set, HashType hash, std::string_view name, std::size_t size) requires PropertyDetail::IsWriteOnly<mode> :
            IProperty(mode, hash, name, size)
        {
            if constexpr (PropertyDetail::HasWrite<mode>)
                this->mSetter = std::move(set);
        }

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
        template<typename U = T, Mode mode = mode>
        ArrayProperty &operator=(std::initializer_list<U> list) requires PropertyDetail::HasWrite<mode>
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
