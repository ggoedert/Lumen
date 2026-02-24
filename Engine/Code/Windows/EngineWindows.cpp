//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetManager.h"
#include "lConcurrentBatchQueue.h"
#include "lEngineWindows.h"

#include "EngineWindows.h"

#ifdef EDITOR
// ImGui editor support
#include "lImGuiLib.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

using namespace Lumen::Windows;

/// windows procedure prototype
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/// EngineWindows::Impl class
class EngineWindows::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class EngineWindows;

public:
    /// constructs an engine
    explicit Impl(EngineWindows &owner) : mOwner(owner) {}

    /// destroys engine
    ~Impl() = default;

    /// initialization and management
    bool Initialize(const Object &config);

    /// pop all batches of items
    bool PopAssetChangeBatchQueue(std::list<std::vector<AssetManager::AssetChange>> &batchQueue) { return mAssetChangeBatches.PopBatchQueue(batchQueue); }

    /// file change callback, static version
    static void CALLBACK StaticFileChangeCallback(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped);

    /// file change callback
    void FileChangeCallback();

private:
    /// owner
    EngineWindows &mOwner;

    /// asset change batches
    ConcurrentBatchQueue<AssetManager::AssetChange> mAssetChangeBatches;

#ifdef EDITOR
    static BYTE sBuffer[65536];
    static OVERLAPPED sOverlapped;
    static HANDLE sDirHandle;
#endif
};

#ifdef EDITOR
BYTE EngineWindows::Impl::sBuffer[65536];
OVERLAPPED EngineWindows::Impl::sOverlapped;
HANDLE EngineWindows::Impl::sDirHandle;
#endif

/// simple windows events
Lumen::HashType EngineWindows::DisplayChanged = Lumen::EncodeType("EngineWindows::DisplayChanged");
Lumen::HashType EngineWindows::WindowMoved = Lumen::EncodeType("EngineWindows::WindowMoved");
Lumen::HashType EngineWindows::Suspending = Lumen::EncodeType("EngineWindows::Suspending");
Lumen::HashType EngineWindows::Resuming = Lumen::EncodeType("EngineWindows::Resuming");
Lumen::HashType EngineWindows::Activated = Lumen::EncodeType("EngineWindows::Activated");
Lumen::HashType EngineWindows::Deactivated = Lumen::EncodeType("EngineWindows::Deactivated");

#ifdef EDITOR
/// file change callback, static version
void CALLBACK EngineWindows::Impl::StaticFileChangeCallback(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
{
    if (errorCode == ERROR_SUCCESS)
    {
        reinterpret_cast<EngineWindows::Impl *>(overlapped->hEvent)->FileChangeCallback();
    }
    else
    {
        std::string message;
        LPSTR buffer = nullptr;
        DWORD size = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&buffer,
            0, nullptr);
        if (size != 0 && buffer != nullptr)
        {
            message = buffer;
            LocalFree(buffer);
        }
        else
        {
            message = "Unknown error";
        }
        Lumen::DebugLog::Error("FileChange callback error: {}", message);  // OutputDebugStringA should be thread safe
    }

    // re-arm the watcher
    ReadDirectoryChangesW(
        sDirHandle,
        sBuffer,
        sizeof(sBuffer),
        TRUE, // recursive
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL,
        overlapped,
        StaticFileChangeCallback
    );
}

/// file change callback
void EngineWindows::Impl::FileChangeCallback()
{
    static DWORD sLastAction = -1;
    static double sLastTimer = -1.f;
    static std::string sLastFilename;
    static std::vector<Lumen::AssetManager::AssetChange> batch;

    /// get elapsed milliseconds since last callback
    double timer = mOwner.GetElapsedTime();
    double elapsedMiliseconds = (sLastTimer >= 0.f) ? (timer - sLastTimer) * 1000.f : FLT_MAX;
    sLastTimer = timer;

    FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)sBuffer;
    while (true)
    {
        std::wstring wfilename(info->FileName, info->FileNameLength / sizeof(WCHAR));
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), (int)wfilename.size(), nullptr, 0, nullptr, nullptr);
        std::string filename(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), (int)wfilename.size(), filename.data(), size_needed, nullptr, nullptr);

        if ((elapsedMiliseconds >= 50.f) || (sLastAction != info->Action) || (sLastFilename != filename))
        {
            switch (info->Action)
            {
            case FILE_ACTION_ADDED:
                batch.push_back({ Lumen::AssetManager::AssetChange::Type::Added, filename, "" });
                break;
            case FILE_ACTION_MODIFIED:
                batch.push_back({ Lumen::AssetManager::AssetChange::Type::Modified, filename, "" });
                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                batch.push_back({ Lumen::AssetManager::AssetChange::Type::Renamed, filename, sLastFilename });
                break;
            case FILE_ACTION_REMOVED:
                batch.push_back({ Lumen::AssetManager::AssetChange::Type::Removed, filename, "" });
                break;
            }
            sLastFilename = filename;
            sLastAction = info->Action;
        }

        if (info->NextEntryOffset == 0)
        {
            break;
        }
        info = (FILE_NOTIFY_INFORMATION *)(((BYTE *)info) + info->NextEntryOffset);
    }
    if (!batch.empty())
    {
        mAssetChangeBatches.PushBatch(std::move(batch));
    }
}
#endif

/// initialization and management
bool EngineWindows::Impl::Initialize(const Object &config)
{
    try
    {
        static std::vector<Lumen::AssetManager::AssetChange> batch;
        for (const auto &entry : std::filesystem::recursive_directory_iterator("Assets"))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().lexically_relative("Assets").generic_string();
                batch.push_back({ Lumen::AssetManager::AssetChange::Type::Added, entry.path().generic_string(), "" });
            }
        }
        if (!batch.empty())
        {
            mAssetChangeBatches.PushBatch(std::move(batch));
        }
    }
    catch (const std::exception &e)
    {
        DebugLog::Error("Initialize engine, unable to process {} directory: {}", "Assets", e.what());
    }

#ifdef EDITOR
    std::string monitorDir = "Assets";
    sDirHandle = CreateFileA(
        monitorDir.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ |
        FILE_SHARE_WRITE |
        FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS |
        FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (sDirHandle != INVALID_HANDLE_VALUE)
    {
        sOverlapped = {};
        sOverlapped.hEvent = this; // store "this" so callback can find the object

        // start watching
        ReadDirectoryChangesW(
            sDirHandle,
            sBuffer,
            sizeof(sBuffer),
            TRUE, // recursive
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL,
            &sOverlapped,
            StaticFileChangeCallback
        );
    }
    else
    {
        Lumen::DebugLog::Error("Could not open directory to monitor: {}", monitorDir);
    }
#endif

    return true;
}

/// start engine
int Lumen::Windows::Start(HINSTANCE hInstance, int nCmdShow, WCHAR *szTitle, WCHAR *szWindowClass, HICON hIcon, const ApplicationPtr application)
{
    // create engine
    auto engine = CreateEngine(application);

    // register class and create window
    {
        // register class
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = hIcon;
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = hIcon;
        if (!RegisterClassExW(&wcex))
            return 1;

#ifdef EDITOR
        // create window
        int width = 1280;
        int height = 720;
        engine->GetFullscreenSize(width, height);
        RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        if (!hwnd)
            return 1;
        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
#else
        // create window
        int width = 1280 / 3;
        int height = 720 / 3;
        if (auto application = engine->GetApplication().lock())
        {
            application->GetWindowSize(width, height);
        }
        RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
#ifdef _DEBUG
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        if (!hwnd)
            return 1;
        ShowWindow(hwnd, nCmdShow);
#else
        AdjustWindowRect(&rc, WS_POPUP, FALSE);

        HWND hwnd = CreateWindowExW(0, szWindowClass, szTitle, WS_POPUP,
            WS_EX_TOPMOST, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        if (!hwnd)
            return 1;
        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
#endif
#endif

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(engine.get()));

        if (!engine->Initialize(Config(hwnd)))
            return 1;

        if (!engine->Open())
            return 1;
    }

    // main message loop
    MSG msg = {};
    bool running = true;
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (running)
            {
                if (!engine->Run())
                {
                    running = false;
                    PostQuitMessage(0);
                }
            }
        }
    }

    engine->Shutdown();

    return static_cast<int>(msg.wParam);
}

/// windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
#ifdef _DEBUG
    static bool s_fullscreen = false;
#else
    static bool s_fullscreen = true;
#endif

    Lumen::Engine *engine = reinterpret_cast<Lumen::Engine *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

#ifdef EDITOR
    if (engine && engine->Initialized())
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureMouseUnlessPopupClose)
        {
            switch (message)
            {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:
                ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
                return TRUE;
            }
        }
        if (io.WantCaptureKeyboard)
        {
            switch (message)
            {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_CHAR:
                ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
                return TRUE;
            }
        }
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        {
            return TRUE;
        }
    }
#endif

    switch (message)
    {
    case WM_PAINT:
        if (s_in_sizemove)
        {
            if (engine)
                engine->Run();
        }
        else
        {
            PAINTSTRUCT ps;
            (void)BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DISPLAYCHANGE:
        if (engine)
            engine->PostEvent(Lumen::Event::MakeUniquePtr(EngineWindows::DisplayChanged));
        break;

    case WM_MOVE:
        if (engine)
            engine->PostEvent(Lumen::Event::MakeUniquePtr(EngineWindows::WindowMoved));
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (engine && !s_in_suspend)
                    engine->PostEvent(Lumen::Event::MakeUniquePtr(EngineWindows::Suspending));
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (engine && s_in_suspend)
                engine->PostEvent(Lumen::Event::MakeUniquePtr(EngineWindows::Resuming));
            s_in_suspend = false;
        }
        else if (!s_in_sizemove)
        {
            if (engine)
                engine->PostEvent(EngineWindows::WindowSizeChanged::MakeUniquePtr(LOWORD(lParam), HIWORD(lParam)));
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (engine)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            engine->PostEvent(EngineWindows::WindowSizeChanged::MakeUniquePtr(rc.right - rc.left, rc.bottom - rc.top));
        }
        break;

    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO *>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

    case WM_ACTIVATEAPP:
        if (engine)
            engine->PostEvent(wParam ? Lumen::Event::MakeUniquePtr(EngineWindows::Activated) : Lumen::Event::MakeUniquePtr(EngineWindows::Deactivated));
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (engine && !s_in_suspend)
                engine->PostEvent(Lumen::Event::MakeUniquePtr(EngineWindows::Suspending));
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (engine && s_in_suspend)
                    engine->PostEvent(Lumen::Event::MakeUniquePtr(EngineWindows::Resuming));
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;

/*DELME!!!???
#ifdef EDITOR
        case WM_CLOSE:
            if (engine)
                engine->GetSettings();
            break;
#endif
*/

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

#if defined(_DEBUG) && !defined(EDITOR)
    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // implements the classic ALT+ENTER fullscreen toggle
            if (s_fullscreen)
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                int width = 1280 / 3;
                int height = 720 / 3;
                if (engine)
                {
                    if (auto application = engine->GetApplication().lock())
                        application->GetWindowSize(width, height);
                }

                ShowWindow(hWnd, SW_SHOWNORMAL);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            else
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }

            s_fullscreen = !s_fullscreen;
        }
        break;
#endif

    case WM_MENUCHAR:
        // a menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

//==============================================================================================================================================================================

/// constructs an engine
EngineWindows::EngineWindows() : mImpl(EngineWindows::Impl::MakeUniquePtr(*this)) {}

/// destroys engine
EngineWindows::~EngineWindows() {}

/// initialization and management
bool EngineWindows::Initialize(const Object &config)
{
    return mImpl->Initialize(config);
}

/// pop all batches of items
bool EngineWindows::PopAssetChangeBatchQueue(std::list<std::vector<AssetManager::AssetChange>> &batchQueue)
{
    return mImpl->PopAssetChangeBatchQueue(batchQueue);
}

/// debug log, windows support
void Lumen::Engine::DebugOutput(const std::string &message)
{
    OutputDebugStringA((message + '\n').c_str());
}
