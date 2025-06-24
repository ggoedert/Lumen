//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMeshFilter.h"

using namespace Lumen;

/// MeshFilter::Impl class
class MeshFilter::Impl
{
    CLASS_NO_DEFAULT_CTOR(MeshFilter::Impl);
    CLASS_NO_COPY_MOVE(MeshFilter::Impl);
    CLASS_PTR_UNIQUEMAKER(MeshFilter::Impl);
    friend class MeshFilter;

public:
    /// constructs a mesh filter
    explicit Impl(int a) {}

private:
    /// run component
    void Run() {}
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TRAITS(MeshFilter);

/// constructs a mesh filter with name and background color
MeshFilter::MeshFilter(const GameObjectWeakPtr &gameObject/*, Math::Vector backgroundColor*/) :
    Behavior(Type(), Name(), gameObject), mImpl(MeshFilter::Impl::MakeUniquePtr(0))
{}

/// creates a smart pointer version of the mesh filter component
ComponentPtr MeshFilter::MakePtr(const GameObjectWeakPtr &gameObject, const std::any &params)
{
    if (params.type() == typeid(Params))
    {
        const auto &createParams = std::any_cast<const Params &>(params);
        return ComponentPtr(new MeshFilter(gameObject/*, createParams.backgroundColor*/));
    }
    DebugLog::Error("Create component, unknown parameter type: " + std::string(params.type().name()));
    return ComponentPtr();
}

/// run component
void MeshFilter::Run() { mImpl->Run(); }
