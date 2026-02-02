#pragma once

#include <string>
#include <functional>

namespace NirUI {

class HttpDownloader {
public:
    HttpDownloader();
    ~HttpDownloader();
    
    // Download file from URL to local path
    // Progress callback receives 0-100 percentage
    bool Download(const std::string& url, const std::string& outputPath,
                  std::function<void(int progress)> progressCallback = nullptr);
    
    // Get last error message
    std::string GetLastError() const { return m_lastError; }
    
private:
    std::string m_lastError;
};

} // namespace NirUI
