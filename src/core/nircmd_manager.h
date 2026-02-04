#pragma once

#include <string>
#include <vector>
#include <functional>
#include <filesystem>

namespace NirUI {

struct ExecutionResult {
    int exitCode;
    std::string output;
    std::string error;
    bool success;
    double executionTimeMs;
};

using ExecutionCallback = std::function<void(const std::string& output)>;

class NirCmdManager {
public:
    NirCmdManager();
    ~NirCmdManager();
    
    bool IsAvailable() const;
    std::string GetNirCmdPath() const;
    bool DownloadNirCmd(std::function<void(int progress, const std::string& status)> progressCallback = nullptr);
    ExecutionResult Execute(const std::string& command, bool waitForCompletion = true);
    ExecutionResult ExecuteWithCallback(const std::string& command, ExecutionCallback callback);
    static std::string BuildCommandLine(const std::string& commandName, const std::vector<std::string>& params);
    std::filesystem::path GetAppDataPath() const;
    std::string GetNirCmdVersion() const;
    bool IsSystem64Bit() const;
    
private:
    std::filesystem::path m_nircmdPath;
    std::filesystem::path m_appDataPath;
    
    void FindNirCmd();
    bool ExtractNirCmd(const std::filesystem::path& zipPath);
};

} // namespace NirUI
