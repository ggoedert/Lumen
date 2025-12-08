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
    mFreeIndices.reserve(initialSize);
    for (int n = initialSize; n > 0; n--)
    {
        mFreeIndices.push_back(n - 1);
    }
}

Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType Lumen::WindowsNT10::DynamicDescriptorHeap::GetIndex(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) const
{
    IndexType cpuIndex = (cpuHandle.ptr - mResourceDescriptors->GetFirstCpuHandle().ptr) / mResourceDescriptors->Increment();
    IndexType gpuIndex = (gpuHandle.ptr - mResourceDescriptors->GetFirstGpuHandle().ptr) / mResourceDescriptors->Increment();
    L_ASSERT_MSG(cpuIndex == gpuIndex, "CPU and GPU descriptor handles do not match indices");
    return static_cast<IndexType>(cpuIndex);
}

Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType Lumen::WindowsNT10::DynamicDescriptorHeap::Allocate()
{
    // make sure we have enough room
    L_ASSERT_MSG(
        mFreeIndices.size() > 0,
        "DescriptorPile has %zu of %zu descriptors; failed request for one more",
        static_cast<size_t>(mFreeIndices.size()),
        static_cast<size_t>(mResourceDescriptors->Count())
    );

    // get a free index
    Lumen::WindowsNT10::DynamicDescriptorHeap::IndexType index = mFreeIndices.back();
    mFreeIndices.pop_back();
    return index;
}
