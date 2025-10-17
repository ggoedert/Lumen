//==============================================================================================================================================================================
/// \file
/// \brief     sphere script interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "Test1.h"

#include "lBehavior.h"

/// SphereScript class
using namespace Lumen;
class SphereScript : public Behavior
{
    CLASS_NO_DEFAULT_CTOR(SphereScript);
    CLASS_NO_COPY_MOVE(SphereScript);
    COMPONENT_TYPEINFO;

public:
    /// sphere script creation parameters
    struct Params : Object
    {
        OBJECT_TYPEINFO;
        explicit Params() : Object(Type()) {}
    };

    /// Start is called before the first frame update
    void Start() override;

    /// Update is called once per frame
    void Update() override;

private:
    /// constructs a sphere script
    explicit SphereScript(const GameObjectWeakPtr &gameObject);

    /// creates a smart pointer version of the sphere script component
    static ComponentPtr MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, const Object &params);
};
