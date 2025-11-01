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
Lumen::ComponentPtr SphereScript::MakePtr(const Lumen::EngineWeakPtr &engine, const Lumen::GameObjectWeakPtr &gameObject)
{
    return Lumen::ComponentPtr(new SphereScript(gameObject));
}

/// serialize
void SphereScript::Serialize(Lumen::Serialized::Type &out, bool packed) const
{
    // set to empty object
    out = Lumen::Serialized::Type::object();
}

/// deserialize
void SphereScript::Deserialize(const Lumen::Serialized::Type &in, bool packed)
{
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
