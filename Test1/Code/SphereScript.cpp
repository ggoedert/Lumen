//==============================================================================================================================================================================
/// \file
/// \brief     sphere script
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "SphereScript.h"

#include "lTransform.h"
#include "lSceneManager.h"

DEFINE_COMPONENT_TYPEINFO(SphereScript);

/// constructs a sphere script with a background color
SphereScript::SphereScript(const Lumen::GameObjectWeakPtr &gameObject) :
    Behavior(Type(), Name(), gameObject) {}

/// creates a smart pointer version of the sphere script component
Lumen::ComponentPtr SphereScript::MakePtr(const Lumen::EngineWeakPtr &engine, const Lumen::GameObjectWeakPtr &gameObject, const Object &params)
{
    if (params.Type() == SphereScript::Params::Type())
    {
        const auto &createParams = static_cast<const Params &>(params);
        return Lumen::ComponentPtr(new SphereScript(gameObject));
    }
#ifdef TYPEINFO
    Lumen::DebugLog::Error("Create component, unknown parameter type: {}", params.Type().mName);
#else
    Lumen::DebugLog::Error("Create component, unknown parameter hash type: 0x{:08X}", params.Type());
#endif
    return {};
}

/// start is called before the first frame update
void SphereScript::Start()
{
}

/// update is called once per frame
void SphereScript::Update()
{
    if (auto gameObject = GameObject().lock())
    {
        if (auto transform = gameObject->Transform().lock())
        {
            transform->Rotate(0.f, 90.f * gameObject->GetApplication().DeltaTime(), 0.f);
        }
    }
}
