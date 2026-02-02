#include "nircmd_manager.h"
#include "utils/http_downloader.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>

namespace NirUI {

NirCmdManager::NirCmdManager() {
    wchar_t* appDataPathW = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appDataPathW))) {
        m_appDataPath = std::filesystem::path(appDataPathW) / "NirUI";
        CoTaskMemFree(appDataPathW);
    } else {
        m_appDataPath = std::filesystem::current_path();
    }
    
    std::filesystem::create_directories(m_appDataPath);
    FindNirCmd();
}

NirCmdManager::~NirCmdManager() {
}

void NirCmdManager::FindNirCmd() {
    std::vector<std::filesystem::path> searchPaths = {
        std::filesystem::current_path() / (IsSystem64Bit() ? "nircmd.exe" : "nircmd.exe"),
        std::filesystem::current_path() / "nircmd.exe",
        m_appDataPath / "nircmd.exe",
        std::filesystem::path("C:\\Windows\\nircmd.exe"),
        std::filesystem::path("C:\\Windows\\System32\\nircmd.exe"),
    };
    
    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            m_nircmdPath = path;
            return;
        }
    }
    
    m_nircmdPath.clear();
}

bool NirCmdManager::IsAvailable() const {
    return !m_nircmdPath.empty() && std::filesystem::exists(m_nircmdPath);
}

std::string NirCmdManager::GetNirCmdPath() const {
    return m_nircmdPath.string();
}

std::filesystem::path NirCmdManager::GetAppDataPath() const {
    return m_appDataPath;
}

bool NirCmdManager::IsSystem64Bit() const {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

bool NirCmdManager::DownloadNirCmd(std::function<void(int progress, const std::string& status)> progressCallback) {
    if (progressCallback) progressCallback(0, "Starting download...");
    
    std::string url = IsSystem64Bit() 
        ? "https://www.nirsoft.net/utils/nircmd-x64.zip"
        : "https://www.nirsoft.net/utils/nircmd.zip";
    
    std::filesystem::path zipPath = m_appDataPath / "nircmd.zip";
    
    if (progressCallback) progressCallback(10, "Downloading NirCmd...");
    
    HttpDownloader downloader;
    if (!downloader.Download(url, zipPath.string(), [&](int progress) {
        if (progressCallback) {
            progressCallback(10 + (progress * 70 / 100), "Downloading: " + std::to_string(progress) + "%");
        }
    })) {
        if (progressCallback) progressCallback(-1, "Download failed!");
        return false;
    }
    
    if (progressCallback) progressCallback(80, "Extracting...");
    
    if (!ExtractNirCmd(zipPath)) {
        if (progressCallback) progressCallback(-1, "Extraction failed!");
        return false;
    }
    
    std::filesystem::remove(zipPath);
    
    if (progressCallback) progressCallback(100, "Done!");
    
    FindNirCmd();
    
    return IsAvailable();
}

bool NirCmdManager::ExtractNirCmd(const std::filesystem::path& zipPath) {
    std::wstring command = L"powershell -Command \"Expand-Archive -Path '";
    command += zipPath.wstring();
    command += L"' -DestinationPath '";
    command += m_appDataPath.wstring();
    command += L"' -Force\"";
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    std::vector<wchar_t> cmdBuffer(command.begin(), command.end());
    cmdBuffer.push_back(L'\0');
    
    if (!CreateProcessW(nullptr, cmdBuffer.data(), nullptr, nullptr, FALSE,
                       CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return exitCode == 0;
}

ExecutionResult NirCmdManager::Execute(const std::string& command, bool waitForCompletion) {
    ExecutionResult result;
    result.success = false;
    result.exitCode = -1;
    result.executionTimeMs = 0;
    
    if (!IsAvailable()) {
        result.error = "NirCmd not available. Please download it first.";
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string fullCommand = "\"" + m_nircmdPath.string() + "\" " + command;
    
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;
    
    HANDLE hStdOutRead, hStdOutWrite;
    HANDLE hStdErrRead, hStdErrWrite;
    
    CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
    CreatePipe(&hStdErrRead, &hStdErrWrite, &sa, 0);
    
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0);
    
    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdErrWrite;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    
    std::vector<char> cmdBuffer(fullCommand.begin(), fullCommand.end());
    cmdBuffer.push_back('\0');
    
    if (!CreateProcessA(nullptr, cmdBuffer.data(), nullptr, nullptr, TRUE,
                       CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        DWORD err = ::GetLastError();
        result.error = "Failed to create process. Error code: " + std::to_string(err);
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        CloseHandle(hStdErrRead);
        CloseHandle(hStdErrWrite);
        return result;
    }
    
    CloseHandle(hStdOutWrite);
    CloseHandle(hStdErrWrite);
    
    if (waitForCompletion) {
        char buffer[4096];
        DWORD bytesRead;
        
        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result.output += buffer;
        }
        
        while (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result.error += buffer;
        }
        
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        result.exitCode = static_cast<int>(exitCode);
        result.success = (exitCode == 0);
    } else {
        result.success = true;
        result.output = "Command started in background";
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    CloseHandle(hStdOutRead);
    CloseHandle(hStdErrRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return result;
}

ExecutionResult NirCmdManager::ExecuteWithCallback(const std::string& command, ExecutionCallback callback) {
    ExecutionResult result;
    result.success = false;
    result.exitCode = -1;
    result.executionTimeMs = 0;
    
    if (!IsAvailable()) {
        result.error = "NirCmd not available. Please download it first.";
        if (callback) callback(result.error);
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string fullCommand = "\"" + m_nircmdPath.string() + "\" " + command;
    
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;
    
    HANDLE hStdOutRead, hStdOutWrite;
    CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
    
    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    
    std::vector<char> cmdBuffer(fullCommand.begin(), fullCommand.end());
    cmdBuffer.push_back('\0');
    
    if (!CreateProcessA(nullptr, cmdBuffer.data(), nullptr, nullptr, TRUE,
                       CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        result.error = "Failed to create process";
        if (callback) callback(result.error);
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        return result;
    }
    
    CloseHandle(hStdOutWrite);
    
    char buffer[1024];
    DWORD bytesRead;
    
    while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result.output += buffer;
        if (callback) callback(std::string(buffer, bytesRead));
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    result.exitCode = static_cast<int>(exitCode);
    result.success = (exitCode == 0);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    CloseHandle(hStdOutRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return result;
}

std::string NirCmdManager::BuildCommandLine(const std::string& commandName, const std::vector<std::string>& params) {
    std::ostringstream oss;
    oss << commandName;
    
    for (const auto& param : params) {
        oss << " ";
        if (param.find(' ') != std::string::npos && param[0] != '"') {
            oss << "\"" << param << "\"";
        } else {
            oss << param;
        }
    }
    
    return oss.str();
}

std::string NirCmdManager::GetNirCmdVersion() const {
    return "2.87";
}

} // namespace NirUI
