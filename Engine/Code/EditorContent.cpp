//==============================================================================================================================================================================
/// \file
/// \brief     Editor Content
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditorContent.h"
#include "lImGuiLib.h"
#include "lNodeForest.h"

#include <filesystem>

using namespace Lumen;

/// EditorContent::Impl class
class EditorContent::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// asset tree
    using AssetTree = NodeForest<std::pair<bool, std::string>>;

    /// asset struct
    struct Asset
    {
        const char *mIcon;
        std::string mName;
    };

    /// constructs editor
    explicit Impl() : mWindowOpen(true)
    {
        mVisibleKey = mRootKey = mAssetTree.insert(AssetTree::NoKey, std::pair<bool, std::string>{ true, "Assets" })->first;
    }

    /// destructor
    ~Impl() {}

    /// editor content dialog name
    static const char *Name()
    {
        return sName;
    }

    /// draw editor content
    void Draw()
    {
        DrawTree();
        DrawAssetBrowser();
    }

    /// draw tree
    void DrawTree()
    {
        // draw tree
        if (ImGui::BeginChild("##tree", ImVec2(300, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
        {
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            ImGui::PopItemFlag();

            if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
            {
                DrawTree(mRootKey);
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }

    /// draw tree node
    void DrawTree(AssetTree::KeyType key)
    {
        auto nodeIt = mAssetTree.find(key);
        if (nodeIt != mAssetTree.end() && nodeIt->second.mData.first)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(key));
            ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
            tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
            tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;  // Left arrow support
            tree_flags |= ImGuiTreeNodeFlags_SpanFullWidth;         // Span full width for easier mouse reach
            tree_flags |= ImGuiTreeNodeFlags_DrawLinesToNodes;      // Always draw hierarchy outlines
            if (key == mVisibleKey)
                tree_flags |= ImGuiTreeNodeFlags_Selected;
            /* commented out, we dont draw leaf nodes in the tree, we only draw folders, so this is not needed
            if (!nodeIt->second.mData.first)
                tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
            */
            if (nodeIt->second.mParentKey == AssetTree::NoKey)
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            // we should not use the name as ID because they are not unique, so we use the UID as ID and display the name only in the label
            bool node_open = ImGui::TreeNodeEx("", tree_flags, "%s", nodeIt->second.mData.second.c_str());
            if (ImGui::IsItemFocused())
                mVisibleKey = key;
            if (node_open)
            {
                for (AssetTree::KeyType& childKey : nodeIt->second.mChildKeys)
                    DrawTree(childKey);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    /// draw asset browser
    void DrawAssetBrowser()
    {
        // gather assets to display
        std::vector<Asset> assets;
        auto visibleNodeIt = mAssetTree.find(mVisibleKey);
        if (visibleNodeIt != mAssetTree.end())
        {
            for (AssetTree::KeyType& childKey : visibleNodeIt->second.mChildKeys)
            {
                auto childIt = mAssetTree.find(childKey);
                if (childIt != mAssetTree.end())
                {
                    // @@REVIEW@@ we should not rely on the name to determine the type, we should store the type in the data, but for now this is just a test
                    AssetTree::Node &child = childIt->second;
                    if (child.mData.first)
                    {
                        assets.push_back({ MATERIAL_ICONS_FOLDER, child.mData.second });
                    }
                    else if (std::string(child.mData.second).ends_with("0"))
                    {
                        assets.push_back({ MATERIAL_ICONS_ASSET, child.mData.second });
                    }
                    else if (std::string(child.mData.second).ends_with("1"))
                    {
                        assets.push_back({ MATERIAL_ICONS_SCRIPT, child.mData.second });
                    }
                    else
                    {
                        assets.push_back({ MATERIAL_ICONS_FILE, child.mData.second });
                    }
                }
            }
        }

        // sort assets alphabetically by name
        std::sort(assets.begin(), assets.end(), [](const Asset &a, const Asset &b) { return a.mName < b.mName; });

        // draw assets in a grid
        static int selected = -1;
        static bool drag_selecting = false;
        static ImVec2 drag_start;

        ImGui::BeginChild("AssetPanel", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);

        float padding = 16.0f;
        float icon_size = ImGuiLib::gMaterialIconsFontSize * 2.f;
        float cell_size = icon_size + padding + 40.f;

        float panel_width = ImGui::GetContentRegionAvail().x;
        int max_columns = (int)(panel_width / cell_size);
        if (max_columns < 1) max_columns = 1;

        ImVec2 start_pos = ImGui::GetCursorScreenPos();
        ImVec2 cursor = start_pos;
        int col = 0;

        for (int i = 0; i < assets.size(); i++)
        {
            Asset &asset = assets[i];
            ImGui::PushID(i);

            ImVec2 cell_min = cursor;
            ImVec2 cell_max = ImVec2(cursor.x + cell_size, cursor.y + cell_size);

            // draw invisible button for input
            ImGui::SetCursorScreenPos(cell_min);
            ImGui::InvisibleButton("asset", ImVec2(cell_size, cell_size));

            bool hovered = ImGui::IsItemHovered();
            bool clicked = ImGui::IsItemClicked();
            bool double_clicked = hovered && ImGui::IsMouseDoubleClicked(0);

            if (clicked) selected = i;
            if (double_clicked) DebugLog::Info("Open asset: {}", asset.mName);

            // drag-drop source
            if (ImGui::BeginDragDropSource())
            {
                ImGui::Text("%s", asset.mName.c_str());
                ImGui::SetDragDropPayload("ASSET", &i, sizeof(int));
                ImGui::EndDragDropSource();
            }

            // hover / selection highlight
            if (hovered || selected == i)
            {
                ImGui::GetWindowDrawList()->AddRectFilled(
                    cell_min,
                    cell_max,
                    IM_COL32(255, 255, 255, 50),
                    4.f
                );
            }

            // drag-selection
            if (drag_selecting)
            {
                ImVec2 drag_min = ImGui::GetMousePos();
                ImVec2 drag_max = drag_start;
                ImRect selection_rect(drag_min, drag_max);
                if (selection_rect.Contains(cell_min))
                    selected = i;
            }

            // draw icon centered
            //if (asset.mColor == false)
            //    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.00f, 0.00f, 1.00f));
            ImGui::PushFont(NULL, icon_size);
            ImVec2 icon_pos = ImVec2(cell_min.x + (cell_size - ImGui::CalcTextSize(asset.mIcon).x) / 2.f, cell_min.y);
            ImGui::SetCursorScreenPos(icon_pos);
            ImGui::Text("%s", asset.mIcon);
            ImGui::PopFont();
            //if (asset.mColor == false)
            //    ImGui::PopStyleColor();

            // draw name (with optional rename)
            ImGui::SetCursorScreenPos(ImVec2(cell_min.x, cell_min.y + icon_size + 20.f));
            float icon_width = 90.0f;
            std::string &label = asset.mName;

            // calculate sizing for layout
            float fullTextWidth = ImGui::CalcTextSize(label.c_str()).x;
            float displayWidth = (fullTextWidth > icon_width) ? icon_width : fullTextWidth;

            // this covers the area from the top of the icon to the bottom of the text
            ImVec2 text_render_pos = ImGui::GetCursorScreenPos();
            ImRect total_cell_bb(icon_pos, ImVec2(text_render_pos.x + icon_width, text_render_pos.y + ImGui::GetTextLineHeight()));

            // center the cursor so the text/input box is aligned under the icon
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (icon_width - displayWidth) * 0.5f);

            /*if (asset.mRenaming) @@REVIEW@@ renaming is currently broken, need to review this
            {
                // rename mode with input box
                char buf[256];
                strcpy_s(buf, label.c_str());

                ImGui::PushItemWidth(icon_width);
                if (ImGui::InputText("##rename", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    asset.mName = buf;
                    asset.mRenaming = false;
                }

                // give focus to the box automatically when renaming starts
                if (ImGui::IsItemVisible() && !ImGui::IsAnyItemActive())
                    ImGui::SetKeyboardFocusHere(-1);

                ImGui::PopItemWidth();
            }
            else
            {*/
                // display mode truncated with ...
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImRect total_bb(pos, ImVec2(pos.x + icon_width, pos.y + ImGui::GetTextLineHeight()));

                // render the ellipsis text 
                ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(),
                    total_bb.Min,
                    total_bb.Max,
                    total_bb.Max.x,
                    label.c_str(),
                    NULL,
                    NULL);

                // register the item size so the layout cursor advances properly
                ImGui::ItemSize(total_bb.GetSize(), 0.0f);

                // register the item ID and bounding box for interaction (hover/click)
                if (ImGui::ItemAdd(total_bb, ImGui::GetID(label.c_str())))
                {
                    if (ImGui::IsItemHovered())
                    {
                        // right-click to trigger renaming @@REVIEW@@ renaming broken for now, need to review this
                        /*if (ImGui::IsMouseClicked(1))
                        {
                            asset.mRenaming = true;
                        }*/

                        // show tooltip only if the text was actually truncated
                        if (fullTextWidth > icon_width)
                        {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted(label.c_str());
                            ImGui::EndTooltip();
                        }
                    }
                }
            //}

            // move cursor for next asset
            col++;
            if (col >= max_columns)
            {
                col = 0;
                cursor.x = start_pos.x;
                cursor.y += cell_size + padding;
            }
            else
            {
                cursor.x += cell_size + padding;
            }

            ImGui::PopID();
        }

        // drag-selection logic (mouse held down in empty space)
        if (ImGui::IsMouseClicked(0))
        {
            drag_selecting = true;
            drag_start = ImGui::GetMousePos();
        }
        if (!ImGui::IsMouseDown(0)) drag_selecting = false;

        ImGui::EndChild();
    }

    /// process asset changes
    void ProcessAssetChanges(std::vector<FileSystem::AssetChange> &&assetBatch)
    {
        for (auto item : assetBatch)
        {
            std::filesystem::path oldFilePath = item.mOldName;
            std::filesystem::path filePath = item.mName;
            bool isDirectory = item.mFlags.Has(FileSystem::Flag::Directory);
            AssetTree::KeyType key;
            auto eraseIt = mAssetTree.end();
            switch (item.mChange)
            {
            case FileSystem::Change::Added:
                key = mRootKey;
                for (const auto &part : filePath.parent_path())
                {
                    auto partIt = mAssetTree.find_if([&](const auto &pair) { return pair.second.mParentKey == key && pair.second.mData.second == part.string(); });
                    if (partIt != mAssetTree.end())
                    {
                        key = partIt->first;
                    }
                    else
                    {
                        key = mAssetTree.insert(key, std::pair<bool, std::string>{ true, part.string() })->first;
                    }
                }
                mAssetTree.insert(key, std::pair<bool, std::string>{ isDirectory, filePath.filename().string() });

                DebugLog::Info("Added: {}", item.mName);
                break;

            case FileSystem::Change::Modified:
                DebugLog::Info("Modified: {}", item.mName);
                break;

            case FileSystem::Change::Renamed:
                key = mRootKey;
                for (const auto &part : oldFilePath.parent_path())
                {
                    auto partIt = mAssetTree.find_if([&](const auto &pair) { return pair.second.mParentKey == key && pair.second.mData.second == part.string(); });
                    if (partIt != mAssetTree.end())
                    {
                        key = partIt->first;
                    }
                    else
                    {
                        key = mAssetTree.insert(key, std::pair<bool, std::string>{ true, part.string() })->first;
                    }
                }
                eraseIt = mAssetTree.find_if([&](const auto &pair) {
                    return pair.second.mParentKey == key && pair.second.mData.second == oldFilePath.filename().string(); });
                if (eraseIt != mAssetTree.end())
                {
                    mAssetTree.erase(eraseIt->first);
                }

                key = mRootKey;
                for (const auto &part : filePath.parent_path())
                {
                    auto partIt = mAssetTree.find_if([&](const auto &pair) { return pair.second.mParentKey == key && pair.second.mData.second == part.string(); });
                    if (partIt != mAssetTree.end())
                    {
                        key = partIt->first;
                    }
                    else
                    {
                        key = mAssetTree.insert(key, std::pair<bool, std::string>{ true, part.string() })->first;
                    }
                }
                mAssetTree.insert(key, std::pair<bool, std::string>{ isDirectory, filePath.filename().string() });

                DebugLog::Info("Renamed: {} -> {}", item.mOldName, item.mName);
                break;

            case FileSystem::Change::Removed:
                key = mRootKey;
                for (const auto &part : filePath.parent_path())
                {
                    auto partIt = mAssetTree.find_if([&](const auto &pair) { return pair.second.mParentKey == key && pair.second.mData.second == part.string(); });
                    if (partIt != mAssetTree.end())
                    {
                        key = partIt->first;
                    }
                    else
                    {
                        key = mAssetTree.insert(key, std::pair<bool, std::string>{ true, part.string() })->first;
                    }
                }
                eraseIt = mAssetTree.find_if([&](const auto &pair) { return pair.second.mParentKey == key && pair.second.mData.second == filePath.filename().string(); });
                if (eraseIt != mAssetTree.end())
                {
                    mAssetTree.erase(eraseIt->first);
                }

                DebugLog::Info("Removed: {}", item.mName);
                break;
            }
        }
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

            Draw();

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

    /// asset node tree
    AssetTree mAssetTree;

    /// root node key
    AssetTree::KeyType mRootKey = AssetTree::NoKey;

    /// currently visible node key
    AssetTree::KeyType mVisibleKey = AssetTree::NoKey;

    /// window open bool
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

/// process asset changes
void EditorContent::ProcessAssetChanges(std::vector<FileSystem::AssetChange> &&assetBatch)
{
    mImpl->ProcessAssetChanges(std::move(assetBatch));
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
