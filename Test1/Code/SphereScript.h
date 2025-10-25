//==============================================================================================================================================================================
/// \file
/// \brief     sphere script interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "Test1.h"

#include "lBehavior.h"

/// SphereScript class
class SphereScript : public Lumen::Behavior
{
    CLASS_NO_DEFAULT_CTOR(SphereScript);
    CLASS_NO_COPY_MOVE(SphereScript);
    COMPONENT_TYPEINFO;

public:
    /// Start is called before the first frame update
    void Start() override;

    /// Update is called once per frame
    void Update() override;

private:
    /// constructs a sphere script
    explicit SphereScript(const Lumen::GameObjectWeakPtr &gameObject);

    /// creates a smart pointer version of the sphere script component
    static Lumen::ComponentPtr MakePtr(const Lumen::EngineWeakPtr &engine, const Lumen::GameObjectWeakPtr &gameObject, const Object &params);
};
