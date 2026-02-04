#pragma once

#include <string>
#include <functional>

namespace NirUI {

class HttpDownloader {
public:
    HttpDownloader();
    ~HttpDownloader();
    
    bool Download(const std::string& url, const std::string& outputPath,
                  std::function<void(int progress)> progressCallback = nullptr);
    std::string GetLastError() const { return m_lastError; }
    
private:
    std::string m_lastError;
};

} // namespace NirUI
