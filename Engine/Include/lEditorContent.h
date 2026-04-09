//==============================================================================================================================================================================
/// \file
/// \brief     Editor Content interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lDefs.h"
#include "lEditor.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(EditorContent);

    /// EditorContent class
    class EditorContent
    {
        CLASS_NO_COPY_MOVE(EditorContent);

    public:
        /// destructor
        ~EditorContent();

        /// creates a smart pointer version of the editor content
        static EditorContentPtr MakePtr();

        /// editor content dialog name
        static const char *Name();

        /// process asset changes
        void ProcessAssetChanges(std::vector<FileSystem::AssetChange> &&assetBatch);

        /// run editor content
        void Run();

        /// return editor content window visibility
        bool Visible();

        /// set editor content window visibility
        void Show(bool visible);

    protected:
        /// constructs editor content
        explicit EditorContent();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
