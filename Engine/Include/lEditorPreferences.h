//==============================================================================================================================================================================
/// \file
/// \brief     Editor Preferences interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_PTR_DEF(EditorPreferences);

    /// EditorPreferences class
    class EditorPreferences
    {
        CLASS_NO_COPY_MOVE(EditorPreferences);

    public:
        /// destructor
        ~EditorPreferences();

        /// creates a smart pointer version of the editor preferences
        static EditorPreferencesPtr MakePtr();

        /// run editor preferences
        void Run(const char *title, Lumen::EnginePtr engine);

        /// return editor preferences window visibility
        bool Visible();

        /// set editor preferences window visibility
        void Show(bool visible);

        /// get theme
        int GetTheme();

        /// set theme
        void SetTheme(int themeIndex, Lumen::EnginePtr engine);

    protected:
        /// constructs editor preferences
        explicit EditorPreferences();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
