//==============================================================================================================================================================================
/// \file
/// \brief     Editor Content
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditorContent.h"
#include "lImGuiLib.h"

using namespace Lumen;

/// EditorContent::Impl class
class EditorContent::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs editor
    explicit Impl() : mWindowOpen(true) {}

    /// destructor
    ~Impl() {}

    /// editor content dialog name
    static const char *Name()
    {
        return sName;
    }

    /// run editor content
    void Run()
    {
        if (mWindowOpen)
        {
            if (!ImGui::Begin(sName, &mWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                ImGui::End();
                return;
            }

            ImGui::End();
        }
    }

    /// return editor content window visibility
    bool Visible()
    {
        return mWindowOpen;
    }

    /// set editor content window visibility
    void Show(bool visible)
    {
        mWindowOpen = visible;
    }

private:
    /// editor content dialog name
    inline static const char *sName = "Content";

    /// window open
    bool mWindowOpen;
};

//==============================================================================================================================================================================

/// constructs editor
EditorContent::EditorContent() : mImpl(EditorContent::Impl::MakeUniquePtr()) {}

/// destructor
EditorContent::~EditorContent() {}

/// creates a smart pointer version of the editor content
EditorContentPtr EditorContent::MakePtr()
{
    return EditorContentPtr(new EditorContent());
}

/// editor content dialog name
const char *EditorContent::Name()
{
    return Impl::Name();
}

/// run editor content
void EditorContent::Run()
{
    mImpl->Run();
}

/// return editor content window visibility
bool EditorContent::Visible()
{
    return mImpl->Visible();
}

/// set editor content window visibility
void EditorContent::Show(bool visible)
{
    mImpl->Show(visible);
}
#endif
