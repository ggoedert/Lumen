//==============================================================================================================================================================================
/// \file
/// \brief     sphere script
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "SphereScript.h"

#include "lTransform.h"

using namespace Lumen;

DEFINE_COMPONENT_TYPEINFO(SphereScript);

/// constructs a sphere script with a background color
SphereScript::SphereScript(const GameObjectWeakPtr &gameObject) :
    Behavior(Type(), Name(), gameObject) {}

/// creates a smart pointer version of the sphere script component
ComponentPtr SphereScript::MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, const Object &params)
{
    if (params.Type() == SphereScript::Params::Type())
    {
        const auto &createParams = static_cast<const Params &>(params);
        return ComponentPtr(new SphereScript(gameObject));
    }
#ifdef TYPEINFO
    DebugLog::Error("Create component, unknown parameter type: {}", params.Type().mName);
#else
    DebugLog::Error("Create component, unknown parameter hash type: 0x{:08X}", params.Type());
#endif
    return {};
}

/// Start is called before the first frame update
void SphereScript::Start()
{
}

/// Update is called once per frame
void SphereScript::Update()
{
    /// WE NEED THE ENGINE TO GIVE US DELTA TIME IN THE UPDATE FUNCTION TO MAKE THIS WORK PROPERLY
    /// PROBLEM: WE CANNOT GET THE ENGINE FROM THE GAME OBJECT, AS IT IS NOT STORED THERE
    /// FIX: STORE A WEAK POINTER TO THE ENGINE IN THE GAME OBJECT WHEN CREATING IT, SO WE CAN GET IT HERE

    const auto transform = GameObject().lock()->Transform().lock();
    if (!transform)
    {
        DebugLog::Error("SphereScript::Update, could not get transform");
        return;
    }

    constexpr float XM_PI = 3.141592654f;
    float ToRad = (XM_PI / 180.0f);

    // Rotate the sphere around its Y axis at 90 degrees per second
    float angle = static_cast<float>(rand()) / RAND_MAX * 360.0f;
    float randrand = (XM_PI / 180.0f) * angle;
    transform->SetRotation(Math::Quaternion::FromYawPitchRoll(0, 90.f * randrand, 0));

    // we need to just rotate the transform, insted of setting it directly, just incremently do it by the amount
    //something like -> transform.Rotate(0, 90 * Time.deltaTime, 0);
}
