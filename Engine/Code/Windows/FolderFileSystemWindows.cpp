//==============================================================================================================================================================================
/// \file
/// \brief     FolderFileSystemWindows
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFolderFileSystemWindows.h"

/// \cond
#include <Windows.h>
/// \endcond

using namespace Lumen;

#ifdef EDITOR
/// quick function to convert string to wstring
std::wstring ToWString(const std::string &str)
{
    if (str.empty()) return {};

    int sizeNeeded = MultiByteToWideChar(
        CP_UTF8,
        0,
        str.data(),
        (int)str.size(),
        nullptr,
        0
    );

    std::wstring result(sizeNeeded, 0);

    MultiByteToWideChar(
        CP_UTF8,
        0,
        str.data(),
        (int)str.size(),
        result.data(),
        sizeNeeded
    );

    return result;
}
#endif

/// FolderFileSystemWindows::Impl class
class FolderFileSystemWindows::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs a folder file system implementation
    explicit Impl(const std::filesystem::path &path);

    /// destroys folder file system
    ~Impl();

    /// initialize file system
    void Initialize();

    /// get path
    const std::filesystem::path &Path() const { return mPath; }

private:
    /// path
    const std::filesystem::path mPath;

#ifdef EDITOR
    /// file change callback, static version
    static void CALLBACK StaticFileChangeCallback(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped);

    /// file change callback
    void FileChangeCallback();

    /// arm the watcher
    void ArmWatcher();

    DWORD mLastAction;
    std::string mLastFilename;
    std::vector<FileSystem::FileChange> mFileBatch;

    DWORD mIoThreadId;
    HANDLE mDirHandle;
    BYTE mBuffer[65536];
    OVERLAPPED mOverlapped;
#endif
};

/// constructs a folder file system implementation
FolderFileSystemWindows::Impl::Impl(const std::filesystem::path &path) : mPath(path)
{
#ifdef EDITOR
    mLastAction = -1;
    mIoThreadId = 0;
    mDirHandle = INVALID_HANDLE_VALUE;
#endif
}

/// destroys folder file system
FolderFileSystemWindows::Impl::~Impl()
{
#ifdef EDITOR
    // stop ReadDirectoryChangesW
    if (mIoThreadId && (mDirHandle != INVALID_HANDLE_VALUE))
    {
        // APC completion routines are queued to the thread that issued the I/O
        // this destructor must be called from that same thread or SleepEx will never wake
        L_ASSERT(GetCurrentThreadId() == mIoThreadId);

        if (CancelIoEx(mDirHandle, nullptr))
        {
            // wait until the cancelled I/O finishes, the completion routine resets mIoThreadId when it sees ERROR_OPERATION_ABORTED
            while (mIoThreadId)
            {
                SleepEx(INFINITE, TRUE);
            }
        }
        else
        {
            // ERROR_NOT_FOUND means no I/O was pending, nothing to cancel
            L_ASSERT(GetLastError() == ERROR_NOT_FOUND);
        }

        // close the handle
        CloseHandle(mDirHandle);
        mDirHandle = INVALID_HANDLE_VALUE;
    }
#endif
}

/// initialize file system
void FolderFileSystemWindows::Impl::Initialize()
{
#ifdef EDITOR
    mDirHandle = CreateFileA(
        mPath.string().c_str(),
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

    if (mDirHandle != INVALID_HANDLE_VALUE)
    {
        mOverlapped = {};
        mOverlapped.hEvent = this; // store "this" so callback can find the object

        // start watching
        ArmWatcher();
    }
    else
    {
        Lumen::DebugLog::Error("Could not open directory to monitor: {}", mPath.string());
    }
#endif
}

#ifdef EDITOR
/// file change callback, static version
void CALLBACK FolderFileSystemWindows::Impl::StaticFileChangeCallback(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
{
    auto self = reinterpret_cast<FolderFileSystemWindows::Impl *>(overlapped->hEvent);

    if (errorCode == ERROR_SUCCESS)
    {
        self->FileChangeCallback();
    }
    else if (errorCode == ERROR_OPERATION_ABORTED)
    {
        // operation canceled
        self->mIoThreadId = 0;
        return;
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
    self->ArmWatcher();
}

/// file change callback
void FolderFileSystemWindows::Impl::FileChangeCallback()
{
    FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)mBuffer;
    while (true)
    {
        std::wstring wfilename(info->FileName, info->FileNameLength / sizeof(WCHAR));
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), (int)wfilename.size(), nullptr, 0, nullptr, nullptr);
        std::string filename(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), (int)wfilename.size(), filename.data(), size_needed, nullptr, nullptr);

        if ((mLastFilename != filename) || (info->Action != mLastAction) || (info->Action == FILE_ACTION_MODIFIED))
        {
            WIN32_FILE_ATTRIBUTE_DATA attr;
            FileSystem::Flags flags = FileSystem::Flag::None;
            std::filesystem::file_time_type lastWrite = {};
            std::wstring wFullPath = ToWString(mPath.string()) + wfilename;
            std::string name, oldName;
            if (GetFileAttributesEx(wFullPath.c_str(), GetFileExInfoStandard, &attr))
            {
                // determine if the file is a directory or a file
                flags = (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FileSystem::Flag::Directory : FileSystem::Flag::File;

                // convert attr.ftLastWriteTime to std::filesystem::file_time_type, file_time_type ticks are 100ns since 1601-01-01, same as windows FILETIME
                ULARGE_INTEGER uliLastWrite { attr.ftLastWriteTime.dwLowDateTime, attr.ftLastWriteTime.dwHighDateTime };
                lastWrite = std::filesystem::file_time_type { std::filesystem::file_time_type::duration{ static_cast<int64_t>(uliLastWrite.QuadPart) } };
            }

            name = FileSystem::NormalizeFilePath(mPath.string() + filename).string();
            switch (info->Action)
            {
            case FILE_ACTION_ADDED:
                mFileBatch.push_back({ FileSystem::Change::Added, flags, lastWrite, name, "" });
                break;
            case FILE_ACTION_MODIFIED:
                mFileBatch.push_back({ FileSystem::Change::Modified, flags, lastWrite, name, "" });
                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                oldName = FileSystem::NormalizeFilePath(mPath.string() + mLastFilename).string();
                mFileBatch.push_back({ FileSystem::Change::Renamed, flags, lastWrite, name, oldName });
                break;
            case FILE_ACTION_REMOVED:
                mFileBatch.push_back({ FileSystem::Change::Removed, flags, lastWrite, name, "" });
                break;
            }
            mLastFilename = filename;
            mLastAction = info->Action;
        }

        if (info->NextEntryOffset == 0)
        {
            break;
        }
        info = (FILE_NOTIFY_INFORMATION *)(((BYTE *)info) + info->NextEntryOffset);
    }
    if (!mFileBatch.empty())
    {
        FileSystem::PushFileChangeBatch(std::move(mFileBatch));
        mFileBatch.clear();
    }
}

/// arm the watcher
void FolderFileSystemWindows::Impl::ArmWatcher()
{
    mIoThreadId = GetCurrentThreadId();
    ReadDirectoryChangesW(
        mDirHandle,
        mBuffer,
        sizeof(mBuffer),
        TRUE, // recursive
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL,
        &mOverlapped,
        StaticFileChangeCallback
    );
}
#endif

//==============================================================================================================================================================================

/// constructs a folder file system
FolderFileSystemWindows::FolderFileSystemWindows(const std::filesystem::path &path) :
    FolderFileSystem(path), mImpl(FolderFileSystemWindows::Impl::MakeUniquePtr(FileSystem::NormalizeDirPath(path))) {}

/// creates a smart pointer version of the folder file system
IFileSystemPtr FolderFileSystem::MakePtr(const std::filesystem::path &path)
{
    return IFileSystemPtr(new FolderFileSystemWindows(path));
}

/// initialize file system
void FolderFileSystemWindows::Initialize()
{
    FolderFileSystem::Initialize();
    mImpl->Initialize();
}

/// get path
const std::filesystem::path &FolderFileSystemWindows::Path() const
{
    return mImpl->Path();
}
