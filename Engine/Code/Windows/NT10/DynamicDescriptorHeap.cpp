//==============================================================================================================================================================================
/// \file
/// \brief     Dynamic descriptor heap
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "DynamicDescriptorHeap.h"

#include <lDefs.h>

Lumen::WindowsNT10::DynamicDescriptorHeap::DynamicDescriptorHeap(ID3D12Device *device, int initialSize)
{
    mResourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(device, initialSize);
    m_top = 0;
}

Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType Lumen::WindowsNT10::DynamicDescriptorHeap::Allocate()
{
    // get the current top
    Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType index = m_top;

    // increment top with new request
    m_top++;

    // make sure we have enough room
    L_ASSERT_MSG(
        m_top <= mResourceDescriptors->Count(),
        "DescriptorPile has %zu of %zu descriptors; failed request for one more",
        static_cast<size_t>(index),
        static_cast<size_t>(mResourceDescriptors->Count())
    );
    return index;
}
