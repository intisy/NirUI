#include "http_downloader.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <fstream>
#include <string>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

namespace NirUI {

static std::string GetWinError() {
    DWORD err = ::GetLastError();
    return std::to_string(err);
}

HttpDownloader::HttpDownloader() {
}

HttpDownloader::~HttpDownloader() {
}

bool HttpDownloader::Download(const std::string& url, const std::string& outputPath,
                              std::function<void(int progress)> progressCallback) {
    HINTERNET hInternet = InternetOpenA("NirUI/1.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet) {
        m_lastError = "Failed to initialize WinINet. Error: " + GetWinError();
        return false;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0,
                                       INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        m_lastError = "Failed to open URL. Error: " + GetWinError();
        InternetCloseHandle(hInternet);
        return false;
    }
    
    DWORD contentLength = 0;
    DWORD sizeOfDword = sizeof(DWORD);
    DWORD index = 0;
    HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &sizeOfDword, &index);
    
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        m_lastError = "Failed to create output file: " + outputPath;
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }
    
    char buffer[8192];
    DWORD bytesRead;
    DWORD totalBytesRead = 0;
    
    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        outFile.write(buffer, bytesRead);
        totalBytesRead += bytesRead;
        if (progressCallback && contentLength > 0) {
            progressCallback(static_cast<int>((totalBytesRead * 100) / contentLength));
        }
    }
    
    outFile.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    if (progressCallback) {
        progressCallback(100);
    }
    
    return true;
}

} // namespace NirUI
