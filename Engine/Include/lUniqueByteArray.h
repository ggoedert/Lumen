//==============================================================================================================================================================================
/// \file
/// \brief     UniqueByteArray is a unique byte array wrapper, has to be used with std::move, no copies allowed, example usage is for temporary initialization data moved around
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    /// UniqueByteArray class
    class UniqueByteArray
    {
        CLASS_NO_COPY(UniqueByteArray);

    public:
        /// constructs the unique byte array
        explicit UniqueByteArray(size_t size) : mData(std::make_unique<byte[]>(size)), mSize(size) {}

        /// default constructor
        UniqueByteArray() = default;

        /// move constructor
        UniqueByteArray(UniqueByteArray &&other) noexcept : mData(std::move(other.mData)), mSize(other.mSize) { other.mSize = 0; }

        /// move assignment
        UniqueByteArray &operator=(UniqueByteArray &&other) noexcept
        {
            if (this != &other)
            {
                mData = std::move(other.mData);
                mSize = other.mSize;
                other.mSize = 0;
            }
            return *this;
        }

        /// get the array
        byte *data() const { return mData.get(); }

        /// get size
        size_t size() const { return mSize; }

        /// subscript operator
        byte &operator[](size_t i) { return mData[i]; }

        /// subscript operator const
        const byte &operator[](size_t i) const { return mData[i]; }

        /// whether the array is valid
        bool Valid() const { return (mData != nullptr) && (mSize > 0); }

    private:
        /// array
        std::unique_ptr<byte[]> mData;

        /// size
        size_t mSize = 0;
    };
}
