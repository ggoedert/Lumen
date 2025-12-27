#ifdef EDITOR

#include "lEditorLog.h"
#include "lImGuiLib.h"

using namespace Lumen;

/// EditorLog::Impl class
class EditorLog::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs editor
    explicit Impl() : mWindowOpen(true), mAutoScroll(true) {}

    /// destructor
    ~Impl() {}

    /// add message to editor log
    void AddMessage(DebugLog::LogLevel level, std::string_view message)
    {
        // create app log message
        std::string messageLower;
        switch (level)
        {
        case DebugLog::LogLevel::Error:
            messageLower = "[error] ";
            break;
        case DebugLog::LogLevel::Warning:
            messageLower = "[warning] ";
            break;
        case DebugLog::LogLevel::Info:
            messageLower = "[info] ";
            break;
        case DebugLog::LogLevel::Detail:
            messageLower = "[detail] ";
            break;
        default:
            messageLower = "[log] ";
            break;
        }
        std::transform(message.begin(), message.end(), std::back_inserter(messageLower), [](char c) { return std::tolower(c); });

        // append to app log
        AppLogMessage appLogMessage = { level, std::string(message), messageLower };
        mAppLog.push_back(appLogMessage);
        AddMessageFiltered(appLogMessage);
    }

    /// run editor log
    void Run(const char *title)
    {
        if (mWindowOpen)
        {
            if (!ImGui::Begin(title, &mWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                ImGui::End();
                return;
            }

            // options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &mAutoScroll);
                ImGui::EndPopup();
            }

            // main window
            if (ImGui::Button("Options"))
            {
                ImGui::OpenPopup("Options");
            }
            ImGui::SameLine();
            bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            static char filterInput[256] = "";
            ImGui::InputTextWithHint("##filter", "Filter...", filterInput, sizeof(filterInput));

            ImGui::Separator();

            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
            {
                bool wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();

                if (clear)
                {
                    Clear();
                }
                if (copy)
                {
                    ImGui::LogToClipboard();
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                std::vector<AppLogMessage> *appLog = &mAppLog;
                bool filterchanged = SetInputFilter(filterInput);
                if (!mCurrentFilter.empty())
                {
                    appLog = &mFilteredAppLog;
                }

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(appLog->size()));
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        switch ((*appLog)[line_no].level)
                        {
                        case DebugLog::LogLevel::Error:
                            ImGui::TextColored({ 1.f, 0.333f, 0.333f, 1.f }, "[Error] ");
                            break;
                        case DebugLog::LogLevel::Warning:
                            ImGui::TextColored({ 1.f, 1.f, 0.333f, 1.f }, "[Warning] ");
                            break;
                        case DebugLog::LogLevel::Info:
                            ImGui::TextColored({ 0.667f, 0.667f, 0.667f, 1.0f }, "[Info] ");
                            break;
                        case DebugLog::LogLevel::Detail:
                            ImGui::TextColored({ 0.5f, 0.5f, 0.5f, 1.0f }, "[Detail] ");
                            break;
                        default:
                            ImGui::TextUnformatted("[Log] ");
                            break;
                        }
                        ImGui::SameLine();
                        ImGui::TextUnformatted((*appLog)[line_no].message.c_str());
                    }
                }
                clipper.End();

                ImGui::PopStyleVar();

                if (filterchanged || (mAutoScroll && wasAtBottom))
                    ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
            ImGui::End();
        }
    }

private:
    /// app log message type
    struct AppLogMessage
    {
        /// log level
        DebugLog::LogLevel level;

        /// log message
        std::string message;

        /// lowercase version of the message
        std::string messageLower;
    };

    /// add message to filtered log if it passes the filter
    void AddMessageFiltered(const AppLogMessage &message)
    {
        if (mCurrentFilter.empty() || (message.messageLower.find(mFilterLower) != std::string::npos))
        {
            mFilteredAppLog.push_back(message);
        }
    }

    /// clear logs/filter
    void Clear()
    {
        mAppLog.clear();
        mFilteredAppLog.clear();
        mCurrentFilter.clear();
    }

    /// set input filter
    bool SetInputFilter(const char *filterInput)
    {
        if (mCurrentFilter != filterInput)
        {
            mCurrentFilter = filterInput;
            mFilterLower.resize(mCurrentFilter.size());
            std::transform(mCurrentFilter.begin(), mCurrentFilter.end(), mFilterLower.begin(), [](char c) { return std::tolower(c); });
            mFilteredAppLog.clear();
            for (const AppLogMessage &message : mAppLog)
            {
                AddMessageFiltered(message);
            }
            return true;
        }
        return false;
    }

    /// window open
    bool mWindowOpen;

    /// auto scroll, keep scrolling if already at the bottom.
    bool mAutoScroll;

    /// current filter
    std::string mCurrentFilter;

    /// lowercase version of the current filter
    std::string mFilterLower;

    /// app log messages
    std::vector<AppLogMessage> mAppLog;

    /// filtered app log messages
    std::vector<AppLogMessage> mFilteredAppLog;
};

//==============================================================================================================================================================================

/// constructs editor
EditorLog::EditorLog() : mImpl(EditorLog::Impl::MakeUniquePtr()) {}

/// destructor
EditorLog::~EditorLog() {}

/// creates a smart pointer version of the editor log
EditorLogPtr EditorLog::MakePtr()
{
    return EditorLogPtr(new EditorLog());
}

/// add message to editor log
void EditorLog::AddMessage(DebugLog::LogLevel level, std::string_view message)
{
    mImpl->AddMessage(level, message);
}

/// run editor log
void EditorLog::Run(const char *title)
{
    mImpl->Run(title);
}
#endif
