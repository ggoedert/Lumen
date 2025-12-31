//==============================================================================================================================================================================
/// \file
/// \brief     Editor Scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_PTR_DEF(EditorScene);

    /// EditorScene class
    class EditorScene
    {
        CLASS_NO_COPY_MOVE(EditorScene);

    public:
        /// destructor
        ~EditorScene();

        /// creates a smart pointer version of the editor scene
        static EditorScenePtr MakePtr();

        /// run editor scene
        void Run(const char *title, Lumen::EnginePtr engine);

        /// return editor scene window visibility
        bool Visible();

        /// set editor scene window visibility
        void Show(bool visible);

    protected:
        /// constructs editor scene
        explicit EditorScene();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
