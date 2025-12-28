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
SphereScript::SphereScript(const Lumen::EntityWeakPtr &entity) :
    Behavior(Type(), Name(), entity) {}

/// creates a smart pointer version of the sphere script component
Lumen::ComponentPtr SphereScript::MakePtr(const Lumen::EngineWeakPtr &engine, const Lumen::EntityWeakPtr &entity)
{
    return Lumen::ComponentPtr(new SphereScript(entity));
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
    if (auto entity = Entity().lock())
    {
        if (auto transform = entity->Transform().lock())
        {
            transform->Rotate(0.f, 90.f * entity->GetApplication().DeltaTime(), 0.f);
        }
    }

    /* temp delme - log messages at random intervals
    using clock = std::chrono::steady_clock;
    static auto lastLogTime = clock::now() - std::chrono::seconds(1);
    auto now = clock::now();
    if (now - lastLogTime >= std::chrono::milliseconds(334))
    {
        lastLogTime = now;

        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (r < 0.65)
        {
            Lumen::DebugLog::Detail("SphereScript: Detail log message.");
        }
        else if (r < 0.85f)
        {
            Lumen::DebugLog::Info("SphereScript: Info log message.");
        }
        else if (r < 0.95f)
        {
            Lumen::DebugLog::Warning("SphereScript: Warning log message.");
        }
        else
        {
            Lumen::DebugLog::Error("SphereScript: Error log message.");
        }
    }
    */
}
