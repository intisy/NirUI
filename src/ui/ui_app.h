#pragma once

#include "core/nircmd_commands.h"
#include "core/nircmd_manager.h"
#include "core/app_groups.h"
#include "svg_icons.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ImFont;

namespace NirUI {

struct HistoryEntry {
    std::string command;
    std::string output;
    bool success;
    double executionTime;
    std::string timestamp;
};

struct FrozenWindow {
    std::string targetType;
    std::string targetValue;
    std::string processName;
    std::string className;
    std::string windowTitle;
    unsigned long long hwnd;
    unsigned long processId;
    bool isFrozen;
    bool wasMaximized = false;
    bool wasMinimized = false;
    int savedX = 0;
    int savedY = 0;
    int savedWidth = 0;
    int savedHeight = 0;
};

struct WindowInfo {
    std::string title;
    std::string processName;
    std::string processPath;
    std::string className;
    unsigned long long hwnd;
    unsigned long processId;
    bool isFrozen;
    bool isFavorite;
};

class UIApp {
public:
    UIApp();
    ~UIApp();
    
    int Run();
    bool ShouldMinimizeToTray() const { return m_minimizeToTray; }
    void ShowFromTray();
    void MinimizeToTray();
    void RequestExit();
    
    static constexpr unsigned int WM_TRAYICON = 0x8000; // WM_APP
    
private:
    bool InitWindow();
    bool InitD3D();
    void CleanupD3D();
    void Render();
    void SetupFonts();
    
    void DrawMenuBar();
    void DrawSidebar();
    void DrawMainPanel();
    void DrawOutputPanel();
    void DrawHistoryPanel();
    void DrawSettingsPanel();
    void DrawAboutPanel();
    void DrawDownloadDialog();
    void DrawStatusBar();
    void DrawAppGroupsPanel();
    void DrawAppGroupEditor();
    void DrawWindowManagerPanel();
    void DrawWindowTargetSelector(const std::string& paramName, std::string& targetType, std::string& targetValue);
    void DrawRecentValuesPopup(const std::string& paramKey, std::string& currentValue);
    void RefreshWindowList();
    void FreezeWindow(const std::string& targetType, const std::string& targetValue, const std::string& processName, const std::string& className = "", const std::string& windowTitle = "", bool recursive = false);
    void UnfreezeWindow(const FrozenWindow& fw);
    void ToggleFavorite(const std::string& processName);
    void SaveFavorites();
    void LoadFavorites();
    void SaveHistory();
    void LoadHistory();
    void DrawIcon(const std::string& iconName, float size = 16.0f);
    std::string GetCategoryIconName(const std::string& categoryName);
    
    void ExecuteCurrentCommand();
    void ExecuteOnAppGroup(const std::string& groupName, const std::string& action);
    void AddToHistory(const std::string& cmd, const ExecutionResult& result);
    void AddRecentValue(const std::string& paramKey, const std::string& value);
    void RemoveRecentValue(const std::string& paramKey, const std::string& value);
    void CopyToClipboard(const std::string& text);
    std::string GetCurrentTimestamp();
    void ApplyDarkTheme();
    void ApplyLightTheme();
    void UpdateTitleBarColor();
    void SaveRecentValues();
    void LoadRecentValues();
    void SaveAppGroups();
    void LoadAppGroups();
    void SaveSettings();
    void LoadSettings();
    void CreateTrayIcon();
    void RemoveTrayIcon();
    void UnfreezeAllWindows();
    
    void* m_hwnd = nullptr;
    
    ID3D11Device* m_pd3dDevice = nullptr;
    ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
    IDXGISwapChain* m_pSwapChain = nullptr;
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
    
    ImFont* m_defaultFont = nullptr;
    ImFont* m_iconFont = nullptr;
    
    std::unique_ptr<NirCmdManager> m_nircmdManager;
    
    int m_selectedCategory = 0;
    int m_selectedCommand = -1;
    char m_searchBuffer[256] = {};
    std::vector<const Command*> m_filteredCommands;
    
    std::map<std::string, std::string> m_parameterValues;
    std::map<std::string, std::vector<std::string>> m_recentValues;
    char m_customCommandBuffer[1024] = {};
    std::string m_lastOutput;
    std::string m_lastError;
    
    std::vector<HistoryEntry> m_history;
    
    bool m_showSettings = false;
    bool m_showAbout = false;
    bool m_showHistory = false;
    bool m_showDownloadDialog = false;
    bool m_showAppGroups = false;
    bool m_showAppGroupEditor = false;
    bool m_showWindowManager = false;
    bool m_downloadInProgress = false;
    int m_downloadProgress = 0;
    std::string m_downloadStatus;
    
    AppGroupsManager m_appGroupsManager;
    int m_selectedAppGroup = -1;
    int m_editingAppGroup = -1;
    char m_newGroupName[64] = {};
    char m_newAppName[64] = {};
    char m_newAppValue[256] = {};
    int m_newAppTargetType = 0;
    
    std::vector<FrozenWindow> m_frozenWindows;
    char m_quickWindowTarget[256] = {};
    
    SvgIconManager m_svgIcons;
    std::vector<WindowInfo> m_windowList;
    std::set<std::string> m_favoriteProcesses;
    char m_windowSearchBuffer[256] = {};
    bool m_windowListNeedsRefresh = false;
    bool m_dockLayoutInitialized = false;
    
    bool m_darkTheme = true;
    bool m_running = true;
    int m_windowWidth = 1280;
    int m_windowHeight = 800;
    
    // Tray and exit settings
    bool m_minimizeToTray = true;
    bool m_unfreezeOnExit = true;
    bool m_isMinimizedToTray = false;
    bool m_trayIconCreated = false;
    
    static constexpr int MAX_RECENT_VALUES = 10;
    static constexpr unsigned int TRAY_UID = 1;
};

} // namespace NirUI
