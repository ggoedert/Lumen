//==============================================================================================================================================================================
/// \file
/// \brief     Editor View interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_PTR_DEF(EditorView);

    /// EditorView class
    class EditorView
    {
        CLASS_NO_COPY_MOVE(EditorView);

    public:
        /// destructor
        ~EditorView();

        /// creates a smart pointer version of the editor view
        static EditorViewPtr MakePtr();

        /// run editor view
        void Run(const char *title, Lumen::EnginePtr engine);

        /// return editor view visibility
        bool Visible();

        /// set editor view visibility
        void Show(bool visible);

    protected:
        /// constructs editor view
        explicit EditorView();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
