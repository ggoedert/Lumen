//==============================================================================================================================================================================
/// \file
/// \brief     Dynamic descriptor heap
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

#include "DescriptorHeap.h"

/// Lumen WindowsNT10 namespace
namespace Lumen::WindowsNT10
{
    /// DynamicDescriptorHeap class
    class DynamicDescriptorHeap
    {
        CLASS_NO_DEFAULT_CTOR(DynamicDescriptorHeap);
        CLASS_NO_COPY_MOVE(DynamicDescriptorHeap);

    public:
        /// index id type
        using IndexType = size_t;

        /// invalid index id
        static constexpr IndexType InvalidTextureID = static_cast<IndexType>(SIZE_MAX);

        DynamicDescriptorHeap(ID3D12Device *device, int initialSize);

        inline ID3D12DescriptorHeap *Heap() const noexcept { return mResourceDescriptors->Heap(); }

        inline D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(IndexType index) const { return mResourceDescriptors->GetGpuHandle(static_cast<size_t>(index)); }

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(IndexType index) const { return mResourceDescriptors->GetCpuHandle(static_cast<size_t>(index)); }

        IndexType Allocate();

    private:
        std::unique_ptr<DirectX::DescriptorHeap> mResourceDescriptors;
        IndexType m_top;
    };
}
