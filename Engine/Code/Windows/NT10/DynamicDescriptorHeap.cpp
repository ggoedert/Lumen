//==============================================================================================================================================================================
/// \file
/// \brief     Dynamic descriptor heap
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "DynamicDescriptorHeap.h"

#include "lDefs.h"

Lumen::WindowsNT10::DynamicDescriptorHeap::DynamicDescriptorHeap(ID3D12Device *device, int initialSize)
{
    mResourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(device, initialSize);
    mTop = 0;
}

Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType Lumen::WindowsNT10::DynamicDescriptorHeap::Allocate()
{
    // get the current top
    Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType index = mTop;

    // increment top with new request
    mTop++;

    // make sure we have enough room
    L_ASSERT_MSG(
        mTop <= mResourceDescriptors->Count(),
        "DescriptorPile has %zu of %zu descriptors; failed request for one more",
        static_cast<size_t>(index),
        static_cast<size_t>(mResourceDescriptors->Count())
    );
    return index;
}

void Lumen::WindowsNT10::DynamicDescriptorHeap::Free(IndexType index)
{
    DebugLog::Warning("Trying to free descriptor heap index {}, please implement DynamicDescriptorHeap::Free()", index);
}
