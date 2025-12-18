//==============================================================================================================================================================================
/// \file
/// \brief     Editor interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Editor);
    CLASS_WEAK_PTR_DEF(Application);

    /// Editor class
    class Editor : public std::enable_shared_from_this<Editor>
    {
        friend class Application;
        CLASS_NO_COPY_MOVE(Editor);

    public:
        /// creates a smart pointer version of the editor
        static EditorPtr MakePtr();

        /// set application
        void SetApplication(const ApplicationWeakPtr &application);

        /// initialize editor
        virtual void Initialize();

        /// shutdown editor
        virtual void Shutdown();

        /// run editor
        void Run();

    //protected:
        /// constructs editor
        explicit Editor();

        /// virtual destructor
        virtual ~Editor();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
