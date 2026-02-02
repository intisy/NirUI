#pragma once

#include <string>
#include <vector>
#include <functional>
#include <filesystem>

namespace NirUI {

// Result of a NirCmd command execution
struct ExecutionResult {
    int exitCode;
    std::string output;
    std::string error;
    bool success;
    double executionTimeMs;
};

// Callback for execution progress
using ExecutionCallback = std::function<void(const std::string& output)>;

class NirCmdManager {
public:
    NirCmdManager();
    ~NirCmdManager();
    
    // Check if NirCmd is available
    bool IsAvailable() const;
    
    // Get NirCmd executable path
    std::string GetNirCmdPath() const;
    
    // Download NirCmd from NirSoft website
    bool DownloadNirCmd(std::function<void(int progress, const std::string& status)> progressCallback = nullptr);
    
    // Execute a NirCmd command
    ExecutionResult Execute(const std::string& command, bool waitForCompletion = true);
    
    // Execute with live output callback
    ExecutionResult ExecuteWithCallback(const std::string& command, ExecutionCallback callback);
    
    // Build command line from command name and parameters
    static std::string BuildCommandLine(const std::string& commandName, const std::vector<std::string>& params);
    
    // Get the application data directory
    std::filesystem::path GetAppDataPath() const;
    
    // Get version of NirCmd if available
    std::string GetNirCmdVersion() const;
    
    // Use system architecture (x64 vs x86)
    bool IsSystem64Bit() const;
    
private:
    std::filesystem::path m_nircmdPath;
    std::filesystem::path m_appDataPath;
    
    void FindNirCmd();
    bool ExtractNirCmd(const std::filesystem::path& zipPath);
};

} // namespace NirUI
