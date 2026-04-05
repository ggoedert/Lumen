//==============================================================================================================================================================================
/// \file
/// \brief     Editor interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lFlags.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Editor);
    CLASS_WEAK_PTR_DEF(Application);

    /// Editor class
    class Editor : public std::enable_shared_from_this<Editor>
    {
        CLASS_NO_COPY_MOVE(Editor);

    public:
        /// AssetChange struct
        struct AssetChange
        {
            /// change type
            enum class Change { Added, Modified, Renamed, Removed };

            /// flag enum
            enum class Flag : byte { None = 0x0, File = 0x1, Directory = 0x2 };

            /// flags type
            using Flags = Flags<Flag>;

            /// change
            Change mChange;

            /// flags
            Flags mFlags;

            /// name
            std::string mName;

            /// old name
            std::string mOldName;
        };

        /// virtual destructor
        virtual ~Editor();

        /// creates a smart pointer version of the editor
        static EditorPtr MakePtr(const ApplicationWeakPtr &application);

        /// initialize editor
        virtual void Initialize();

        /// shutdown editor
        virtual void Shutdown();

        /// editor first run
        virtual void FirstRun();

        /// process asset changes
        void ProcessAssetChanges(std::list<std::vector<AssetChange>> &&batchQueue);

        /// run editor
        void Run();

    protected:
        /// constructs editor
        explicit Editor(const ApplicationWeakPtr &application);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
