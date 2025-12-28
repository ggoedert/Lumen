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
    explicit Impl() : mWindowOpen(true), mAutoScroll(true), mCurrentLogLevel(2) {}

    /// destructor
    ~Impl() {}

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

            // combo box to switch log level
            const char *logLevelArray[] = { "Detail", "Info", "Warning", "Error" };

            // calc the width required by the combo box
            float maxWidth = 0;
            for (int i = 0; i < IM_ARRAYSIZE(logLevelArray); i++)
            {
                float width = ImGui::CalcTextSize(logLevelArray[i]).x;
                if (width > maxWidth) maxWidth = width;
            }
            float totalWidth = maxWidth + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetFrameHeight();

            // draw the combo box
            ImGui::SetNextItemWidth(totalWidth);
            int logLevel = mCurrentLogLevel - 1;
            if (ImGui::Combo("##LogLevel", &logLevel, logLevelArray, IM_ARRAYSIZE(logLevelArray))) {}
            logLevel++;
            ImGui::SameLine();

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
            static char inputFilter[256] = "";
            ImGui::InputTextWithHint("##filter", "Filter...", inputFilter, sizeof(inputFilter));

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

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));

                std::vector<AppLogMessage> *appLog = &mAppLog;
                bool filterChanged = SetFilter(logLevel, inputFilter);
                if (!mCurrentFilter.empty() || mCurrentLogLevel > 0)
                {
                    appLog = &mFilteredAppLog;
                }

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(appLog->size()));
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        PrintLog((*appLog)[line_no]);
                    }
                }
                clipper.End();

                ImGui::PopStyleVar();

                if (filterChanged || (mAutoScroll && wasAtBottom))
                    ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();

            ImGui::End();
        }
    }

    /// add message to editor log
    void AddMessage(DebugLog::LogLevel level, std::string_view message)
    {
        if (level == DebugLog::LogLevel::None)
        {
            return;
        }

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
        if (appLogMessage.level != DebugLog::LogLevel::Detail)
        {
            mStatus = appLogMessage;
        }
    }

    /// print status
    void PrintStatus()
    {
        PrintLog(mStatus);
    }

    /// return editor log visibility
    bool Visible()
    {
        return mWindowOpen;
    }
    /// set editor log visibility
    void Show(bool visible)
    {
        mWindowOpen = visible;
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

    /// print log
    void PrintLog(AppLogMessage &appLogMessage)
    {
        if (appLogMessage.level != DebugLog::LogLevel::None)
        {
            switch (appLogMessage.level)
            {
            case DebugLog::LogLevel::Error:
                ImGui::TextColored({ 1.f, 0.333f, 0.333f, 1.f }, "[Error]");
                break;
            case DebugLog::LogLevel::Warning:
                ImGui::TextColored({ 1.f, 1.f, 0.333f, 1.f }, "[Warning]");
                break;
            case DebugLog::LogLevel::Info:
                ImGui::TextColored({ 0.667f, 0.667f, 0.667f, 1.0f }, "[Info]");
                break;
            case DebugLog::LogLevel::Detail:
                ImGui::TextColored({ 0.5f, 0.5f, 0.5f, 1.0f }, "[Detail]");
                break;
            default:
                ImGui::TextUnformatted("[Log]");
                break;
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(appLogMessage.message.c_str());
        }
    }

    /// add message to filtered log if it passes the filter
    void AddMessageFiltered(const AppLogMessage &message)
    {
        bool logLevel = (static_cast<int>(message.level) >= mCurrentLogLevel);
        bool filter = mCurrentFilter.empty() || (message.messageLower.find(mFilterLower) != std::string::npos);
        if (logLevel && filter)
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

    /// set filter
    bool SetFilter(int currentLogLevel, const char *inputFilter)
    {
        if ((mCurrentLogLevel != currentLogLevel) || (mCurrentFilter != inputFilter))
        {
            mCurrentLogLevel = currentLogLevel;
            mCurrentFilter = inputFilter;
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

    /// current log level
    int mCurrentLogLevel;

    /// current filter
    std::string mCurrentFilter;

    /// lowercase version of the current filter
    std::string mFilterLower;

    /// app log messages
    std::vector<AppLogMessage> mAppLog;

    /// filtered app log messages
    std::vector<AppLogMessage> mFilteredAppLog;

    /// last status message
    AppLogMessage mStatus = { DebugLog::LogLevel::None, std::string(), std::string() };
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

/// run editor log
void EditorLog::Run(const char *title)
{
    mImpl->Run(title);
}

/// add message to editor log
void EditorLog::AddMessage(DebugLog::LogLevel level, std::string_view message)
{
    mImpl->AddMessage(level, message);
}

/// print status
void EditorLog::PrintStatus()
{
    mImpl->PrintStatus();
}

/// return editor log visibility
bool EditorLog::Visible()
{
    return mImpl->Visible();
}

/// set editor log visibility
void EditorLog::Show(bool visible)
{
    mImpl->Show(visible);
}
#endif
