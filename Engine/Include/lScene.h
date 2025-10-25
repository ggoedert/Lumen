//==============================================================================================================================================================================
/// \file
/// \brief     scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lGameObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Scene);

    /// Scene class
    class Scene
    {
        CLASS_NO_DEFAULT_CTOR(Scene);
        CLASS_NO_COPY_MOVE(Scene);
        CLASS_PTR_MAKER(Scene);

    public:
        /// constructor
        explicit Scene(Lumen::Application &application);

        /// virtual destructor
        virtual ~Scene();

        /// load scene
        virtual bool Load(const std::filesystem::path &file);

        /// unload scene
        virtual void Unload();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
