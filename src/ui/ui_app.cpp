#include "ui_app.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <windows.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <d3d11.h>
#include <dxgi.h>
#include <tchar.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>
#include <fstream>
#include <algorithm>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace NirUI {

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {}
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

UIApp::UIApp() {
    m_nircmdManager = std::make_unique<NirCmdManager>();
    LoadRecentValues();
    LoadHistory();
    m_appGroupsManager.SetDataPath(m_nircmdManager->GetAppDataPath());
    m_appGroupsManager.Load();
}

UIApp::~UIApp() {
    SaveRecentValues();
    SaveFavorites();
    SaveHistory();
    m_appGroupsManager.Save();
    m_svgIcons.Cleanup();
    CleanupD3D();
}

bool UIApp::InitWindow() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(1));
    HICON hIconSm = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, 
                                       GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
                                       LR_DEFAULTCOLOR);
    
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = hIcon;
    wc.hIconSm = hIconSm ? hIconSm : hIcon;
    wc.lpszClassName = L"NirUI";
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowW(wc.lpszClassName, L"NirUI - NirCmd Wrapper",
                          WS_OVERLAPPEDWINDOW, 100, 100, m_windowWidth, m_windowHeight,
                          nullptr, nullptr, wc.hInstance, nullptr);

    if (!m_hwnd) return false;
    SetWindowLongPtr((HWND)m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    UpdateTitleBarColor();
    return true;
}

bool UIApp::InitD3D() {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (HWND)m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
        &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext) != S_OK)
        return false;

    ID3D11Texture2D* pBackBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();

    return true;
}

void UIApp::CleanupD3D() {
    if (m_mainRenderTargetView) { m_mainRenderTargetView->Release(); m_mainRenderTargetView = nullptr; }
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_pd3dDeviceContext) { m_pd3dDeviceContext->Release(); m_pd3dDeviceContext = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
}

void UIApp::SetupFonts() {
    ImGuiIO& io = ImGui::GetIO();
    
    m_defaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
    
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.GlyphMinAdvanceX = 16.0f;
    iconConfig.GlyphOffset = ImVec2(0, 2);
    
    static const ImWchar iconRanges[] = { 0xE700, 0xF000, 0 };
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segmdl2.ttf", 16.0f, &iconConfig, iconRanges);
    
    m_iconFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segmdl2.ttf", 18.0f, nullptr, iconRanges);
}

void UIApp::UpdateTitleBarColor() {
    if (!m_hwnd) return;
    
    BOOL useDarkMode = m_darkTheme ? TRUE : FALSE;
    DwmSetWindowAttribute((HWND)m_hwnd, 20, &useDarkMode, sizeof(useDarkMode)); // DWMWA_USE_IMMERSIVE_DARK_MODE
    
    COLORREF captionColor = m_darkTheme ? 0x00352F2D : 0x00FFFFFF; // Dark gray or white
    DwmSetWindowAttribute((HWND)m_hwnd, 35, &captionColor, sizeof(captionColor)); // DWMWA_CAPTION_COLOR
}

void UIApp::ApplyDarkTheme() {
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.55f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.45f, 0.80f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.50f, 0.85f, 0.50f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.50f, 0.85f, 0.70f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.50f, 0.85f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.50f, 0.85f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.50f, 0.85f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.50f, 0.85f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.50f, 0.85f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.35f, 0.60f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.50f, 0.85f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.50f, 0.85f, 0.50f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.20f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
}

void UIApp::ApplyLightTheme() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
}

int UIApp::Run() {
    if (!InitWindow()) return 1;
    if (!InitD3D()) return 1;

    ShowWindow((HWND)m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow((HWND)m_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupFonts();
    
    if (m_darkTheme) ApplyDarkTheme();
    else ApplyLightTheme();

    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);
    
    m_svgIcons.Initialize(m_pd3dDevice);
    LoadFavorites();

    if (!m_nircmdManager->IsAvailable()) {
        m_showDownloadDialog = true;
    }

    MSG msg = {};
    while (m_running && msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        RECT rect;
        GetClientRect((HWND)m_hwnd, &rect);
        int newWidth = rect.right - rect.left;
        int newHeight = rect.bottom - rect.top;
        
        if (newWidth > 0 && newHeight > 0 && (newWidth != m_windowWidth || newHeight != m_windowHeight)) {
            m_windowWidth = newWidth;
            m_windowHeight = newHeight;
            
            if (m_mainRenderTargetView) {
                m_mainRenderTargetView->Release();
                m_mainRenderTargetView = nullptr;
            }
            m_pSwapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* pBackBuffer;
            m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
            pBackBuffer->Release();
        }

        Render();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow((HWND)m_hwnd);
    UnregisterClassW(L"NirUI", GetModuleHandle(nullptr));

    return 0;
}

void UIApp::Render() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    
    if (!m_dockLayoutInitialized) {
        m_dockLayoutInitialized = true;
        
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
        
        ImGuiID dock_left, dock_right;
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.22f, &dock_left, &dock_right);
        
        ImGuiID dock_right_top, dock_right_bottom;
        ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Up, 0.7f, &dock_right_top, &dock_right_bottom);
        
        ImGui::DockBuilderDockWindow("Commands", dock_left);
        ImGui::DockBuilderDockWindow("Command Builder", dock_right_top);
        ImGui::DockBuilderDockWindow("Output", dock_right_bottom);
        
        ImGui::DockBuilderFinish(dockspace_id);
    }
    
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    DrawMenuBar();
    DrawSidebar();
    DrawMainPanel();
    DrawOutputPanel();
    
    if (m_showSettings) DrawSettingsPanel();
    if (m_showAbout) DrawAboutPanel();
    if (m_showHistory) DrawHistoryPanel();
    if (m_showDownloadDialog) DrawDownloadDialog();
    if (m_showAppGroups) DrawAppGroupsPanel();
    if (m_showAppGroupEditor) DrawAppGroupEditor();
    if (m_showWindowManager) DrawWindowManagerPanel();

    DrawStatusBar();

    ImGui::End();

    ImGui::Render();
    const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
    m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    m_pSwapChain->Present(1, 0);
}

void UIApp::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Download NirCmd", nullptr, false, !m_nircmdManager->IsAvailable())) {
                m_showDownloadDialog = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                m_running = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("History", "Ctrl+H", m_showHistory)) {
                m_showHistory = !m_showHistory;
            }
            if (ImGui::MenuItem("App Groups", "Ctrl+G", m_showAppGroups)) {
                m_showAppGroups = !m_showAppGroups;
            }
            if (ImGui::MenuItem("Window Manager", "Ctrl+W", m_showWindowManager)) {
                m_showWindowManager = !m_showWindowManager;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Dark Theme", nullptr, m_darkTheme)) {
                m_darkTheme = true;
                ApplyDarkTheme();
                UpdateTitleBarColor();
            }
            if (ImGui::MenuItem("Light Theme", nullptr, !m_darkTheme)) {
                m_darkTheme = false;
                ApplyLightTheme();
                UpdateTitleBarColor();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About NirUI")) {
                m_showAbout = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("NirCmd Documentation")) {
                ShellExecuteA(nullptr, "open", "https://www.nirsoft.net/utils/nircmd.html", nullptr, nullptr, SW_SHOW);
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
}

std::string UIApp::GetCategoryIconName(const std::string& categoryName) {
    if (categoryName == "Volume Control") return "volume";
    if (categoryName == "Monitor Control") return "monitor";
    if (categoryName == "System Control") return "settings";
    if (categoryName == "Window Management") return "window";
    if (categoryName == "Process Management") return "process";
    if (categoryName == "Clipboard") return "clipboard";
    if (categoryName == "CD-ROM") return "audio";
    if (categoryName == "Display Settings") return "display";
    if (categoryName == "File Operations") return "folder";
    if (categoryName == "Registry") return "registry";
    if (categoryName == "Shortcuts") return "shortcut";
    if (categoryName == "Network") return "network";
    if (categoryName == "Services") return "system";
    if (categoryName == "Text-to-Speech") return "dialog";
    if (categoryName == "Screenshots") return "monitor";
    if (categoryName == "Input Simulation") return "keyboard";
    if (categoryName == "Dialogs & Messages") return "dialog";
    if (categoryName == "Miscellaneous") return "settings";
    return "settings";
}

void UIApp::DrawIcon(const std::string& iconName, float size) {
    auto icon = m_svgIcons.GetIcon(iconName);
    if (icon) {
        ImGui::Image(reinterpret_cast<ImTextureID>(icon), ImVec2(size, size));
    } else {
        ImGui::Dummy(ImVec2(size, size));
    }
}

void UIApp::DrawSidebar() {
    ImGui::SetNextWindowSize(ImVec2(280, 0), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Commands", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##Search", "Search commands...", m_searchBuffer, sizeof(m_searchBuffer));
        
        std::string searchQuery = m_searchBuffer;
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        const auto& categories = NirCmdCommands::GetCategories();
        
        for (size_t i = 0; i < categories.size(); ++i) {
            const auto& cat = categories[i];
            
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
            if (m_selectedCategory == static_cast<int>(i)) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            
            DrawIcon(GetCategoryIconName(cat.name), 14.0f);
            ImGui::SameLine();
            bool open = ImGui::TreeNodeEx(("##cat" + std::to_string(i)).c_str(), flags);
            ImGui::SameLine();
            ImGui::Text("%s", cat.name.c_str());
            
            if (ImGui::IsItemClicked()) {
                m_selectedCategory = static_cast<int>(i);
                m_selectedCommand = -1;
                m_searchBuffer[0] = '\0';
            }
            
            if (open) {
                for (size_t j = 0; j < cat.commands.size(); ++j) {
                    const auto& cmd = cat.commands[j];
                    
                    if (!searchQuery.empty()) {
                        std::string lowerName = cmd.name;
                        std::string lowerQuery = searchQuery;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
                        if (lowerName.find(lowerQuery) == std::string::npos) {
                            continue;
                        }
                    }
                    
                    bool selected = (m_selectedCategory == static_cast<int>(i) && 
                                    m_selectedCommand == static_cast<int>(j));
                    
                    if (ImGui::Selectable(("  " + cmd.name).c_str(), selected)) {
                        m_selectedCategory = static_cast<int>(i);
                        m_selectedCommand = static_cast<int>(j);
                        m_parameterValues.clear();
                        m_customCommandBuffer[0] = '\0';
                    }
                    
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", cmd.description.c_str());
                        ImGui::EndTooltip();
                    }
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

void UIApp::DrawWindowTargetSelector(const std::string& paramName, std::string& targetType, std::string& targetValue) {
    static const char* windowTargetTypes[] = { "title", "ititle", "class", "process", "handle", "folder", "active", "foreground", "alltop" };
    
    ImGui::Text("Target Type:");
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo("##targettype", targetType.empty() ? "title" : targetType.c_str())) {
        for (const char* type : windowTargetTypes) {
            if (ImGui::Selectable(type, targetType == type)) {
                targetType = type;
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    ImGui::Text("Value:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    
    char buffer[256] = {};
    strncpy_s(buffer, targetValue.c_str(), sizeof(buffer) - 1);
    
    std::string inputId = "##targetval_" + paramName;
    if (ImGui::InputText(inputId.c_str(), buffer, sizeof(buffer))) {
        targetValue = buffer;
    }
    
    std::string recentKey = "window_target_" + targetType;
    DrawRecentValuesPopup(recentKey, targetValue);
}

void UIApp::DrawRecentValuesPopup(const std::string& paramKey, std::string& currentValue) {
    auto it = m_recentValues.find(paramKey);
    if (it == m_recentValues.end() || it->second.empty()) return;
    
    ImGui::SameLine();
    std::string popupId = "recent_" + paramKey;
    
    if (ImGui::Button(("...##" + paramKey).c_str())) {
        ImGui::OpenPopup(popupId.c_str());
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Recent values");
        ImGui::EndTooltip();
    }
    
    if (ImGui::BeginPopup(popupId.c_str())) {
        ImGui::Text("Recent Values:");
        ImGui::Separator();
        
        std::vector<std::string> toRemove;
        
        for (const auto& val : it->second) {
            ImGui::PushID(val.c_str());
            
            if (ImGui::Selectable(val.c_str(), false, 0, ImVec2(200, 0))) {
                currentValue = val;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            if (ImGui::SmallButton("X")) {
                toRemove.push_back(val);
            }
            ImGui::PopStyleColor();
            
            ImGui::PopID();
        }
        
        for (const auto& val : toRemove) {
            RemoveRecentValue(paramKey, val);
        }
        
        ImGui::EndPopup();
    }
}

void UIApp::DrawMainPanel() {
    if (ImGui::Begin("Command Builder", nullptr, ImGuiWindowFlags_NoCollapse)) {
        const auto& categories = NirCmdCommands::GetCategories();
        
        if (m_selectedCategory >= 0 && m_selectedCategory < static_cast<int>(categories.size())) {
            const auto& cat = categories[m_selectedCategory];
            
            if (m_selectedCommand >= 0 && m_selectedCommand < static_cast<int>(cat.commands.size())) {
                const auto& cmd = cat.commands[m_selectedCommand];
                
                DrawIcon(GetCategoryIconName(cat.name), 18.0f);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", cmd.name.c_str());
                ImGui::TextWrapped("%s", cmd.description.c_str());
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                if (!cmd.parameters.empty()) {
                    ImGui::Text("Parameters:");
                    ImGui::Spacing();
                    
                    for (const auto& param : cmd.parameters) {
                        ImGui::PushID(param.name.c_str());
                        
                        auto& value = m_parameterValues[param.name];
                        if (value.empty() && !param.defaultValue.empty()) {
                            value = param.defaultValue;
                        }
                        
                        // Skip recursive parameter unless folder type is selected
                        if (param.name == "recursive") {
                            std::string typeVal = m_parameterValues["find_type"];
                            if (typeVal.empty()) typeVal = m_parameterValues["target_type"];
                            if (typeVal != "folder") {
                                ImGui::PopID();
                                continue;
                            }
                        }
                        
                        ImGui::Text("%s%s:", param.name.c_str(), param.required ? "*" : "");
                        ImGui::SameLine();
                        ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", param.description.c_str());
                            ImGui::EndTooltip();
                        }
                        
                        bool isWindowFindType = (param.name == "find_type" || param.name == "parent_find_type" || param.name == "child_find_type");
                        bool isWindowFindValue = (param.name == "find_value" || param.name == "parent_find_value" || param.name == "child_find_value");
                        bool isGroupParam = (param.name == "group" && (cmd.name.substr(0, 6) == "group "));
                        
                        ImGui::SetNextItemWidth(isWindowFindValue ? -120 : -40);
                        
                        // Group parameter - show combobox with existing groups
                        if (isGroupParam) {
                            const auto& groups = m_appGroupsManager.GetGroups();
                            std::string preview = value.empty() ? (groups.empty() ? "<no groups>" : groups[0].name) : value;
                            if (ImGui::BeginCombo("##groupcombo", preview.c_str())) {
                                for (const auto& group : groups) {
                                    if (ImGui::Selectable(group.name.c_str(), value == group.name)) {
                                        value = group.name;
                                    }
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else if (param.type == ParamType::Choice && !param.choices.empty()) {
                            if (ImGui::BeginCombo("##combo", value.empty() ? param.choices[0].c_str() : value.c_str())) {
                                for (const auto& choice : param.choices) {
                                    if (ImGui::Selectable(choice.c_str(), value == choice)) {
                                        value = choice;
                                    }
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else if (param.type == ParamType::Boolean) {
                            bool boolValue = (value == "1" || value == "true");
                            if (ImGui::Checkbox("##bool", &boolValue)) {
                                value = boolValue ? "1" : "0";
                            }
                        }
                        else if (param.type == ParamType::Integer) {
                            int intValue = value.empty() ? 0 : std::atoi(value.c_str());
                            if (ImGui::InputInt("##int", &intValue)) {
                                value = std::to_string(intValue);
                            }
                        }
                        else if (param.type == ParamType::FilePath || param.type == ParamType::FolderPath) {
                            char buffer[512] = {};
                            strncpy_s(buffer, value.c_str(), sizeof(buffer) - 1);
                            if (ImGui::InputText("##path", buffer, sizeof(buffer))) {
                                value = buffer;
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("...")) {}
                            
                            std::string recentKey = "path_" + param.name;
                            DrawRecentValuesPopup(recentKey, value);
                        }
                        else {
                            char buffer[512] = {};
                            strncpy_s(buffer, value.c_str(), sizeof(buffer) - 1);
                            if (ImGui::InputText("##text", buffer, sizeof(buffer))) {
                                value = buffer;
                            }
                            
                            if (isWindowFindValue) {
                                std::string findTypeParam = param.name;
                                size_t pos = findTypeParam.find("_value");
                                if (pos != std::string::npos) {
                                    findTypeParam.replace(pos, 6, "_type");
                                }
                                std::string findType = m_parameterValues[findTypeParam];
                                if (findType.empty()) findType = "title";
                                
                                std::string recentKey = "window_" + findType;
                                DrawRecentValuesPopup(recentKey, value);
                                
                                ImGui::SameLine();
                                std::string pickerId = "##picker_" + param.name;
                                if (ImGui::Button(("Pick" + pickerId).c_str())) {
                                    RefreshWindowList();
                                    ImGui::OpenPopup(("WindowPicker" + param.name).c_str());
                                }
                                
                                if (ImGui::BeginPopup(("WindowPicker" + param.name).c_str())) {
                                    ImGui::Text("Select a window:");
                                    ImGui::SameLine(280);
                                    if (ImGui::SmallButton("X##closeWP")) {
                                        ImGui::CloseCurrentPopup();
                                    }
                                    ImGui::Separator();
                                    
                                    static char windowPickerSearch[128] = {};
                                    ImGui::SetNextItemWidth(290);
                                    ImGui::InputTextWithHint("##wpSearch", "Search...", windowPickerSearch, sizeof(windowPickerSearch));
                                    
                                    std::string searchLower = windowPickerSearch;
                                    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                                    
                                    ImGui::BeginChild("WindowPickerList", ImVec2(300, 300), true);
                                    if (m_windowList.empty()) {
                                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No windows found.");
                                    }
                                    for (const auto& win : m_windowList) {
                                        std::string displayText = win.title.empty() ? win.processName : win.title;
                                        std::string searchText = displayText + win.processName;
                                        std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
                                        
                                        if (!searchLower.empty() && searchText.find(searchLower) == std::string::npos) {
                                            continue;
                                        }
                                        
                                        if (displayText.length() > 40) {
                                            displayText = displayText.substr(0, 37) + "...";
                                        }
                                        
                                        ImGui::PushID(static_cast<int>(win.hwnd));
                                        if (ImGui::Selectable(displayText.c_str())) {
                                            if (findType == "process") {
                                                value = win.processName;
                                            } else if (findType == "class") {
                                                value = win.className;
                                            } else {
                                                value = win.title;
                                            }
                                            windowPickerSearch[0] = '\0';
                                            ImGui::CloseCurrentPopup();
                                        }
                                        ImGui::SameLine();
                                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", win.processName.c_str());
                                        ImGui::PopID();
                                    }
                                    ImGui::EndChild();
                                    ImGui::EndPopup();
                                }
                            }
                        }
                        
                        ImGui::PopID();
                        ImGui::Spacing();
                    }
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                std::string cmdLine = cmd.name;
                for (const auto& param : cmd.parameters) {
                    // Skip recursive if not folder type
                    if (param.name == "recursive") {
                        std::string typeVal = m_parameterValues["find_type"];
                        if (typeVal.empty()) typeVal = m_parameterValues["target_type"];
                        if (typeVal != "folder") continue;
                    }
                    
                    auto it = m_parameterValues.find(param.name);
                    if (it != m_parameterValues.end() && !it->second.empty()) {
                        cmdLine += " ";
                        if (it->second.find(' ') != std::string::npos) {
                            cmdLine += "\"" + it->second + "\"";
                        } else {
                            cmdLine += it->second;
                        }
                    }
                }
                
                ImGui::Text("Command Preview:");
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##preview", &cmdLine[0], cmdLine.size() + 1, ImGuiInputTextFlags_ReadOnly);
                ImGui::PopStyleColor();
                
                ImGui::Spacing();
                
                bool canExecute = m_nircmdManager->IsAvailable();
                if (!canExecute) ImGui::BeginDisabled();
                
                if (ImGui::Button("Execute", ImVec2(120, 35))) {
                    strncpy_s(m_customCommandBuffer, cmdLine.c_str(), sizeof(m_customCommandBuffer) - 1);
                    
                    for (const auto& param : cmd.parameters) {
                        auto it = m_parameterValues.find(param.name);
                        if (it != m_parameterValues.end() && !it->second.empty()) {
                            bool isWindowFindValue = (param.name == "find_value" || param.name.find("_value") != std::string::npos);
                            if (isWindowFindValue) {
                                std::string findTypeParam = param.name;
                                size_t pos = findTypeParam.find("_value");
                                if (pos != std::string::npos) {
                                    findTypeParam.replace(pos, 6, "_type");
                                }
                                std::string findType = m_parameterValues[findTypeParam];
                                if (findType.empty()) findType = "title";
                                AddRecentValue("window_" + findType, it->second);
                            }
                            else if (param.type == ParamType::FilePath || param.type == ParamType::FolderPath) {
                                AddRecentValue("path_" + param.name, it->second);
                            }
                        }
                    }
                    
                    ExecuteCurrentCommand();
                }
                
                if (!canExecute) {
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "NirCmd not found");
                }
                
                ImGui::SameLine();
                if (ImGui::Button("Copy", ImVec2(100, 35))) {
                    CopyToClipboard("nircmd " + cmdLine);
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("Example:");
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", cmd.example.c_str());
            }
            else {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a command from the sidebar");
            }
        }
        else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a category to get started");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Text("Custom Command:");
        ImGui::SetNextItemWidth(-120);
        ImGui::InputText("##custom", m_customCommandBuffer, sizeof(m_customCommandBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Run", ImVec2(100, 0))) {
            ExecuteCurrentCommand();
        }
    }
    ImGui::End();
}

void UIApp::DrawOutputPanel() {
    if (ImGui::Begin("Output", nullptr, ImGuiWindowFlags_NoCollapse)) {
        if (ImGui::Button("Clear")) {
            m_lastOutput.clear();
            m_lastError.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy")) {
            CopyToClipboard(m_lastOutput + m_lastError);
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::BeginChild("OutputScroll", ImVec2(0, 0), true);
        
        if (!m_lastOutput.empty()) {
            ImGui::TextWrapped("%s", m_lastOutput.c_str());
        }
        
        if (!m_lastError.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_lastError.c_str());
        }
        
        if (m_lastOutput.empty() && m_lastError.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Command output will appear here...");
        }
        
        ImGui::EndChild();
    }
    ImGui::End();
}

void UIApp::DrawHistoryPanel() {
    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Command History", &m_showHistory)) {
        if (ImGui::Button("Clear History")) {
            m_history.clear();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        for (int i = static_cast<int>(m_history.size()) - 1; i >= 0; --i) {
            const auto& entry = m_history[i];
            
            ImGui::PushID(i);
            
            ImVec4 color = entry.success ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f);
            ImGui::TextColored(color, entry.success ? "[OK]" : "[ERR]");
            ImGui::SameLine();
            ImGui::Text("%s", entry.timestamp.c_str());
            ImGui::Text("  %s", entry.command.c_str());
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  (%.1f ms)", entry.executionTime);
            
            if (ImGui::Button("Run")) {
                strncpy_s(m_customCommandBuffer, entry.command.c_str(), sizeof(m_customCommandBuffer) - 1);
                ExecuteCurrentCommand();
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy")) {
                CopyToClipboard("nircmd " + entry.command);
            }
            
            ImGui::Separator();
            ImGui::PopID();
        }
    }
    ImGui::End();
}

void UIApp::DrawSettingsPanel() {
    if (ImGui::Begin("Settings", &m_showSettings)) {
        ImGui::Text("Theme:");
        if (ImGui::RadioButton("Dark", m_darkTheme)) {
            m_darkTheme = true;
            ApplyDarkTheme();
            UpdateTitleBarColor();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Light", !m_darkTheme)) {
            m_darkTheme = false;
            ApplyLightTheme();
            UpdateTitleBarColor();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Text("NirCmd Location:");
        ImGui::Text("  %s", m_nircmdManager->GetNirCmdPath().c_str());
        
        if (!m_nircmdManager->IsAvailable()) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "NirCmd not found!");
            if (ImGui::Button("Download NirCmd")) {
                m_showDownloadDialog = true;
            }
        }
    }
    ImGui::End();
}

void UIApp::DrawAboutPanel() {
    ImGui::SetNextWindowSize(ImVec2(450, 320), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("About NirUI", &m_showAbout, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("NirUI v1.0.0");
        ImGui::Spacing();
        ImGui::TextWrapped("A modern graphical and command-line wrapper for NirCmd.");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Text("NirCmd Information:");
        ImGui::BulletText("Version: 2.87");
        ImGui::BulletText("Author: Nir Sofer (NirSoft)");
        ImGui::BulletText("Website: nirsoft.net");
        
        ImGui::Spacing();
        
        if (ImGui::Button("Visit NirSoft Website")) {
            ShellExecuteA(nullptr, "open", "https://www.nirsoft.net/utils/nircmd.html", nullptr, nullptr, SW_SHOW);
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::TextWrapped("NirCmd is freeware. NirUI is a third-party wrapper and is not affiliated with NirSoft.");
    }
    ImGui::End();
}

void UIApp::DrawDownloadDialog() {
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Download NirCmd", &m_showDownloadDialog, 
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        
        if (!m_downloadInProgress) {
            ImGui::TextWrapped("NirCmd is required to run commands. Would you like to download it from the official NirSoft website?");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Text("Download URL:");
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), 
                             m_nircmdManager->IsSystem64Bit() 
                             ? "nirsoft.net/utils/nircmd-x64.zip" 
                             : "nirsoft.net/utils/nircmd.zip");
            
            ImGui::Spacing();
            
            if (ImGui::Button("Download", ImVec2(120, 35))) {
                m_downloadInProgress = true;
                m_downloadProgress = 0;
                m_downloadStatus = "Starting download...";
                
                std::thread([this]() {
                    bool success = m_nircmdManager->DownloadNirCmd([this](int progress, const std::string& status) {
                        m_downloadProgress = progress;
                        m_downloadStatus = status;
                    });
                    
                    m_downloadInProgress = false;
                    if (success) {
                        m_showDownloadDialog = false;
                    }
                }).detach();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 35))) {
                m_showDownloadDialog = false;
            }
        }
        else {
            ImGui::Text("%s", m_downloadStatus.c_str());
            ImGui::Spacing();
            ImGui::ProgressBar(m_downloadProgress / 100.0f, ImVec2(-1, 0));
        }
    }
    ImGui::End();
}

void UIApp::DrawStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - 25));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 25));
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    
    ImGui::Begin("##StatusBar", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);
    
    if (m_nircmdManager->IsAvailable()) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "NirCmd Ready");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "NirCmd Not Found");
    }
    
    ImGui::SameLine(viewport->WorkSize.x - 200);
    ImGui::Text("Commands: %d", static_cast<int>(m_history.size()));
    
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

static std::string ExtractQuotedOrWord(const std::string& str, size_t& pos) {
    while (pos < str.length() && str[pos] == ' ') pos++;
    if (pos >= str.length()) return "";
    
    if (str[pos] == '"') {
        size_t end = str.find('"', pos + 1);
        if (end != std::string::npos) {
            std::string result = str.substr(pos + 1, end - pos - 1);
            pos = end + 1;
            return result;
        }
    }
    size_t end = str.find(' ', pos);
    if (end == std::string::npos) end = str.length();
    std::string result = str.substr(pos, end - pos);
    pos = end;
    return result;
}

void UIApp::ExecuteCurrentCommand() {
    std::string command = m_customCommandBuffer;
    if (command.empty()) return;
    
    if (command.substr(0, 11) == "win freeze " && command.length() > 11) {
        std::string rest = command.substr(11);
        size_t spacePos = rest.find(' ');
        if (spacePos != std::string::npos) {
            std::string findType = rest.substr(0, spacePos);
            std::string findValue = rest.substr(spacePos + 1);
            bool recursive = false;
            size_t recPos = findValue.find(" recursive=");
            if (recPos != std::string::npos) {
                std::string recVal = findValue.substr(recPos + 11);
                recursive = (recVal == "true" || recVal == "1");
                findValue = findValue.substr(0, recPos);
            }
            if (!findValue.empty() && findValue.front() == '"' && findValue.back() == '"') {
                findValue = findValue.substr(1, findValue.length() - 2);
            }
            FreezeWindow(findType, findValue, findType == "process" ? findValue : "", "", "", recursive);
            
            ExecutionResult result;
            result.success = true;
            result.output = "Frozen: " + findValue;
            result.executionTimeMs = 0;
            AddToHistory(command, result);
            return;
        }
    }
    else if (command.substr(0, 13) == "win unfreeze " && command.length() > 13) {
        std::string rest = command.substr(13);
        size_t spacePos = rest.find(' ');
        if (spacePos != std::string::npos) {
            std::string findType = rest.substr(0, spacePos);
            std::string findValue = rest.substr(spacePos + 1);
            bool recursive = false;
            size_t recPos = findValue.find(" recursive=");
            if (recPos != std::string::npos) {
                std::string recVal = findValue.substr(recPos + 11);
                recursive = (recVal == "true" || recVal == "1");
                findValue = findValue.substr(0, recPos);
            }
            if (!findValue.empty() && findValue.front() == '"' && findValue.back() == '"') {
                findValue = findValue.substr(1, findValue.length() - 2);
            }
            auto it = std::find_if(m_frozenWindows.begin(), m_frozenWindows.end(),
                [&](const FrozenWindow& fw) { return fw.targetValue == findValue; });
            if (it != m_frozenWindows.end()) {
                UnfreezeWindow(*it);
                m_frozenWindows.erase(it);
            } else {
                FrozenWindow tempFw;
                tempFw.targetType = findType;
                tempFw.targetValue = findValue;
                tempFw.processName = (findType == "process") ? findValue : "";
                UnfreezeWindow(tempFw);
            }
            
            ExecutionResult result;
            result.success = true;
            result.output = "Unfrozen: " + findValue;
            result.executionTimeMs = 0;
            AddToHistory(command, result);
            return;
        }
    }
    else if (command.substr(0, 6) == "group ") {
        ExecutionResult result;
        result.success = true;
        result.executionTimeMs = 0;
        
        std::string rest = command.substr(6);
        size_t pos = 0;
        std::string subCmd = ExtractQuotedOrWord(rest, pos);
        
        if (subCmd == "list") {
            std::string output = "App Groups:\n";
            for (const auto& group : m_appGroupsManager.GetGroups()) {
                output += "  " + group.name + " (" + std::to_string(group.apps.size()) + " apps)\n";
                for (const auto& app : group.apps) {
                    output += "    - " + app.name + " [" + app.targetType + ": " + app.targetValue + "]";
                    if (app.targetType == "folder" && app.recursive) output += " (recursive)";
                    output += "\n";
                }
            }
            result.output = output;
        }
        else if (subCmd == "create") {
            std::string name = ExtractQuotedOrWord(rest, pos);
            if (!name.empty()) {
                if (m_appGroupsManager.CreateGroup(name)) {
                    m_appGroupsManager.Save();
                    result.output = "Created group: " + name;
                } else {
                    result.output = "Group already exists: " + name;
                    result.success = false;
                }
            }
        }
        else if (subCmd == "delete") {
            std::string name = ExtractQuotedOrWord(rest, pos);
            if (!name.empty()) {
                if (m_appGroupsManager.DeleteGroup(name)) {
                    m_appGroupsManager.Save();
                    result.output = "Deleted group: " + name;
                } else {
                    result.output = "Group not found: " + name;
                    result.success = false;
                }
            }
        }
        else if (subCmd == "add") {
            std::string groupName = ExtractQuotedOrWord(rest, pos);
            std::string appName = ExtractQuotedOrWord(rest, pos);
            std::string targetType = ExtractQuotedOrWord(rest, pos);
            std::string targetValue = ExtractQuotedOrWord(rest, pos);
            std::string recursiveStr = ExtractQuotedOrWord(rest, pos);
            bool recursive = (recursiveStr.empty() || recursiveStr == "true" || recursiveStr == "1");
            
            if (!groupName.empty() && !appName.empty() && !targetType.empty() && !targetValue.empty()) {
                if (m_appGroupsManager.AddApp(groupName, appName, targetType, targetValue, recursive)) {
                    m_appGroupsManager.Save();
                    result.output = "Added " + appName + " to " + groupName;
                } else {
                    result.output = "Group not found: " + groupName;
                    result.success = false;
                }
            }
        }
        else if (subCmd == "remove") {
            std::string groupName = ExtractQuotedOrWord(rest, pos);
            std::string appName = ExtractQuotedOrWord(rest, pos);
            if (!groupName.empty() && !appName.empty()) {
                if (m_appGroupsManager.RemoveApp(groupName, appName)) {
                    m_appGroupsManager.Save();
                    result.output = "Removed " + appName + " from " + groupName;
                } else {
                    result.output = "Group or app not found";
                    result.success = false;
                }
            }
        }
        else if (subCmd == "run") {
            std::string groupName = ExtractQuotedOrWord(rest, pos);
            std::string action = ExtractQuotedOrWord(rest, pos);
            if (!groupName.empty() && !action.empty()) {
                ExecuteOnAppGroup(groupName, action);
                result.output = "Executed " + action + " on group: " + groupName;
            }
        }
        
        m_lastOutput = result.output;
        m_lastError = result.success ? "" : result.output;
        AddToHistory(command, result);
        return;
    }
    
    auto result = m_nircmdManager->Execute(command);
    
    m_lastOutput = result.output;
    m_lastError = result.error;
    
    if (result.success && m_lastOutput.empty()) {
        m_lastOutput = "Command executed successfully.";
    }
    
    AddToHistory(command, result);
}

void UIApp::AddToHistory(const std::string& cmd, const ExecutionResult& result) {
    HistoryEntry entry;
    entry.command = cmd;
    entry.output = result.output;
    entry.success = result.success;
    entry.executionTime = result.executionTimeMs;
    entry.timestamp = GetCurrentTimestamp();
    
    m_history.push_back(entry);
    
    if (m_history.size() > 100) {
        m_history.erase(m_history.begin());
    }
}

void UIApp::AddRecentValue(const std::string& paramKey, const std::string& value) {
    if (value.empty()) return;
    
    auto& recent = m_recentValues[paramKey];
    
    auto it = std::find(recent.begin(), recent.end(), value);
    if (it != recent.end()) {
        recent.erase(it);
    }
    
    recent.insert(recent.begin(), value);
    
    if (recent.size() > MAX_RECENT_VALUES) {
        recent.resize(MAX_RECENT_VALUES);
    }
}

void UIApp::RemoveRecentValue(const std::string& paramKey, const std::string& value) {
    auto it = m_recentValues.find(paramKey);
    if (it != m_recentValues.end()) {
        auto& vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
    }
}

void UIApp::SaveRecentValues() {
    std::filesystem::path savePath = m_nircmdManager->GetAppDataPath() / "recent_values.txt";
    std::ofstream file(savePath);
    if (!file) return;
    
    for (const auto& [key, values] : m_recentValues) {
        for (const auto& val : values) {
            file << key << "|" << val << "\n";
        }
    }
}

void UIApp::LoadRecentValues() {
    std::filesystem::path savePath = m_nircmdManager->GetAppDataPath() / "recent_values.txt";
    std::ifstream file(savePath);
    if (!file) return;
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('|');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            m_recentValues[key].push_back(value);
        }
    }
}

void UIApp::DrawAppGroupsPanel() {
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("App Groups", &m_showAppGroups)) {
        DrawIcon("app_group", 18.0f);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Application Groups");
        ImGui::TextWrapped("Create groups of applications and apply window commands to all apps in a group at once.");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("New Group", ImVec2(120, 0))) {
            m_editingAppGroup = -1;
            m_newGroupName[0] = '\0';
            m_showAppGroupEditor = true;
        }
        
        ImGui::Spacing();
        
        auto& groups = m_appGroupsManager.GetGroups();
        
        if (groups.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No app groups defined. Create one to get started!");
        }
        
        for (size_t i = 0; i < groups.size(); ++i) {
            auto& group = groups[i];
            
            ImGui::PushID(static_cast<int>(i));
            
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
            bool open = ImGui::TreeNodeEx(("##group" + std::to_string(i)).c_str(), flags);
            
            ImGui::SameLine();
            DrawIcon("folder", 14.0f);
            ImGui::SameLine();
            ImGui::Text("%s (%d apps)", group.name.c_str(), static_cast<int>(group.apps.size()));
            
            ImGui::SameLine(ImGui::GetWindowWidth() - 200);
            
            if (ImGui::SmallButton("Run")) {
                ImGui::OpenPopup("GroupActions");
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Edit")) {
                m_editingAppGroup = static_cast<int>(i);
                strncpy_s(m_newGroupName, group.name.c_str(), sizeof(m_newGroupName) - 1);
                m_showAppGroupEditor = true;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            if (ImGui::SmallButton("X")) {
                m_appGroupsManager.DeleteGroup(group.name);
                ImGui::PopStyleColor();
                ImGui::PopID();
                if (open) ImGui::TreePop();
                break;
            }
            ImGui::PopStyleColor();
            
            if (ImGui::BeginPopup("GroupActions")) {
                ImGui::Text("Apply to all apps in '%s':", group.name.c_str());
                ImGui::Separator();
                
                if (ImGui::MenuItem("Minimize All")) {
                    ExecuteOnAppGroup(group.name, "min");
                }
                if (ImGui::MenuItem("Maximize All")) {
                    ExecuteOnAppGroup(group.name, "max");
                }
                if (ImGui::MenuItem("Restore All")) {
                    ExecuteOnAppGroup(group.name, "normal");
                }
                if (ImGui::MenuItem("Close All")) {
                    ExecuteOnAppGroup(group.name, "close");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Freeze All (Hide + Suspend)")) {
                    ExecuteOnAppGroup(group.name, "freeze");
                }
                if (ImGui::MenuItem("Unfreeze All (Resume + Show)")) {
                    ExecuteOnAppGroup(group.name, "unfreeze");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Hide All")) {
                    ExecuteOnAppGroup(group.name, "hide");
                }
                if (ImGui::MenuItem("Show All")) {
                    ExecuteOnAppGroup(group.name, "show");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Bring to Front")) {
                    ExecuteOnAppGroup(group.name, "activate");
                }
                
                ImGui::EndPopup();
            }
            
            if (open) {
                for (size_t j = 0; j < group.apps.size(); ++j) {
                    const auto& app = group.apps[j];
                    ImGui::BulletText("%s [%s: %s]", app.name.c_str(), app.targetType.c_str(), app.targetValue.c_str());
                }
                ImGui::TreePop();
            }
            
            ImGui::PopID();
        }
    }
    ImGui::End();
}

void UIApp::DrawAppGroupEditor() {
    ImGui::SetNextWindowSize(ImVec2(450, 400), ImGuiCond_FirstUseEver);
    
    std::string title = (m_editingAppGroup >= 0) ? "Edit App Group" : "New App Group";
    
    if (ImGui::Begin(title.c_str(), &m_showAppGroupEditor, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Group Name:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##groupname", m_newGroupName, sizeof(m_newGroupName));
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        auto& groups = m_appGroupsManager.GetGroups();
        AppGroup* editGroup = nullptr;
        if (m_editingAppGroup >= 0 && m_editingAppGroup < static_cast<int>(groups.size())) {
            editGroup = &groups[m_editingAppGroup];
        }
        
        ImGui::Text("Applications in Group:");
        
        if (editGroup) {
            ImGui::BeginChild("AppList", ImVec2(0, 150), true);
            for (size_t i = 0; i < editGroup->apps.size(); ++i) {
                auto& app = editGroup->apps[i];
                ImGui::PushID(static_cast<int>(i));
                
                ImGui::Text("%s", app.name.c_str());
                ImGui::SameLine();
                if (app.targetType == "folder" && app.recursive) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s: %s] (recursive)", 
                                      app.targetType.c_str(), app.targetValue.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s: %s]", 
                                      app.targetType.c_str(), app.targetValue.c_str());
                }
                ImGui::SameLine(ImGui::GetWindowWidth() - 40);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
                if (ImGui::SmallButton("X")) {
                    editGroup->apps.erase(editGroup->apps.begin() + i);
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                    break;
                }
                ImGui::PopStyleColor();
                
                ImGui::PopID();
            }
            ImGui::EndChild();
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Save the group first, then add apps.");
        }
        
        ImGui::Spacing();
        
        static bool newAppRecursive = true;
        
        if (editGroup) {
            ImGui::Text("Add Application:");
            
            ImGui::SetNextItemWidth(150);
            ImGui::InputTextWithHint("##appname", "Display name", m_newAppName, sizeof(m_newAppName));
            
            ImGui::SameLine();
            static const char* targetTypes[] = { "process", "class", "title", "ititle", "folder" };
            ImGui::SetNextItemWidth(100);
            ImGui::Combo("##targettype", &m_newAppTargetType, targetTypes, IM_ARRAYSIZE(targetTypes));
            
            bool isFolder = (m_newAppTargetType == 4);
            
            if (isFolder) {
                ImGui::SetNextItemWidth(-1);
                ImGui::InputTextWithHint("##appvalue", "e.g. C:\\Games", m_newAppValue, sizeof(m_newAppValue));
                
                if (ImGui::Button("Browse##folder")) {
                    BROWSEINFOA bi = {};
                    bi.lpszTitle = "Select Folder";
                    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
                    if (pidl) {
                        char path[MAX_PATH] = {};
                        if (SHGetPathFromIDListA(pidl, path)) {
                            strncpy_s(m_newAppValue, path, sizeof(m_newAppValue) - 1);
                            if (strlen(m_newAppName) == 0) {
                                std::string folderName = path;
                                size_t lastSlash = folderName.find_last_of("\\/");
                                if (lastSlash != std::string::npos) {
                                    folderName = folderName.substr(lastSlash + 1);
                                }
                                strncpy_s(m_newAppName, folderName.c_str(), sizeof(m_newAppName) - 1);
                            }
                        }
                        CoTaskMemFree(pidl);
                    }
                }
                ImGui::SameLine();
                ImGui::Checkbox("Recursive", &newAppRecursive);
                ImGui::SameLine();
            } else {
                ImGui::SetNextItemWidth(-120);
                ImGui::InputTextWithHint("##appvalue", "e.g. code.exe or Code", m_newAppValue, sizeof(m_newAppValue));
                ImGui::SameLine();
            }
            
            if (ImGui::Button("Pick##appgroup")) {
                RefreshWindowList();
                ImGui::OpenPopup("AppGroupWindowPicker");
            }
            
            if (ImGui::BeginPopup("AppGroupWindowPicker")) {
                ImGui::Text("Select a window:");
                ImGui::SameLine(330);
                if (ImGui::SmallButton("X##closeAGP")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Separator();
                
                static char appGroupPickerSearch[128] = {};
                ImGui::SetNextItemWidth(340);
                ImGui::InputTextWithHint("##agpSearch", "Search...", appGroupPickerSearch, sizeof(appGroupPickerSearch));
                
                std::string searchLower = appGroupPickerSearch;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                
                ImGui::BeginChild("AppGroupWindowList", ImVec2(350, 300), true);
                if (m_windowList.empty()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No windows found.");
                }
                for (const auto& win : m_windowList) {
                    std::string displayText = win.title.empty() ? win.processName : win.title;
                    std::string searchText = displayText + win.processName;
                    std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
                    
                    if (!searchLower.empty() && searchText.find(searchLower) == std::string::npos) {
                        continue;
                    }
                    
                    if (displayText.length() > 45) {
                        displayText = displayText.substr(0, 42) + "...";
                    }
                    
                    ImGui::PushID(static_cast<int>(win.hwnd));
                    if (ImGui::Selectable(displayText.c_str())) {
                        std::string autoName = win.title.empty() ? win.processName : win.title;
                        if (autoName.length() > 30) {
                            autoName = autoName.substr(0, 27) + "...";
                        }
                        strncpy_s(m_newAppName, autoName.c_str(), sizeof(m_newAppName) - 1);
                        
                        if (m_newAppTargetType == 0) {
                            strncpy_s(m_newAppValue, win.processName.c_str(), sizeof(m_newAppValue) - 1);
                        } else if (m_newAppTargetType == 1) {
                            strncpy_s(m_newAppValue, win.className.c_str(), sizeof(m_newAppValue) - 1);
                        } else {
                            strncpy_s(m_newAppValue, win.title.c_str(), sizeof(m_newAppValue) - 1);
                        }
                        appGroupPickerSearch[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", win.processName.c_str());
                    ImGui::PopID();
                }
                ImGui::EndChild();
                ImGui::EndPopup();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                if (strlen(m_newAppName) > 0 && strlen(m_newAppValue) > 0) {
                    m_appGroupsManager.AddApp(editGroup->name, m_newAppName, 
                                             targetTypes[m_newAppTargetType], m_newAppValue,
                                             isFolder && newAppRecursive);
                    m_newAppName[0] = '\0';
                    m_newAppValue[0] = '\0';
                }
            }
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
                "Target: process=exe, class=window class, title=title, folder=all exes in folder");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button(editGroup ? "Save Changes" : "Create Group", ImVec2(120, 30))) {
            if (strlen(m_newGroupName) > 0) {
                if (editGroup) {
                    editGroup->name = m_newGroupName;
                } else {
                    m_appGroupsManager.CreateGroup(m_newGroupName);
                    m_editingAppGroup = static_cast<int>(groups.size()) - 1;
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(80, 30))) {
            m_showAppGroupEditor = false;
        }
        
        ImGui::Spacing();
        
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Common Examples:");
        ImGui::BulletText("VS Code: process = Code.exe");
        ImGui::BulletText("Chrome: process = chrome.exe");
        ImGui::BulletText("IntelliJ: class = SunAwtFrame");
        ImGui::BulletText("CLion: process = clion64.exe");
    }
    ImGui::End();
}

void UIApp::DrawWindowManagerPanel() {
    ImGui::SetNextWindowSize(ImVec2(600, 550), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Window Manager", &m_showWindowManager)) {
        if (m_windowList.empty() || m_windowListNeedsRefresh) {
            RefreshWindowList();
            m_windowListNeedsRefresh = false;
        }
        
        DrawIcon("search", 16.0f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-100);
        ImGui::InputTextWithHint("##windowsearch", "Search windows...", m_windowSearchBuffer, sizeof(m_windowSearchBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Refresh", ImVec2(90, 0))) {
            m_windowListNeedsRefresh = true;
        }
        
        std::string searchQuery = m_windowSearchBuffer;
        std::transform(searchQuery.begin(), searchQuery.end(), searchQuery.begin(), ::tolower);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::BeginChild("WindowListArea", ImVec2(0, -30), false);
        
        if (!m_frozenWindows.empty()) {
            DrawIcon("freeze", 16.0f);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Frozen (%d)", static_cast<int>(m_frozenWindows.size()));
            ImGui::Indent();
            
            for (size_t i = 0; i < m_frozenWindows.size(); ++i) {
                auto& fw = m_frozenWindows[i];
                
                std::string displayName = fw.windowTitle.empty() ? fw.targetValue : fw.windowTitle;
                std::string searchText = displayName + fw.processName;
                std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
                if (!searchQuery.empty() && searchText.find(searchQuery) == std::string::npos) {
                    continue;
                }
                
                ImGui::PushID(("frozen_" + std::to_string(i)).c_str());
                
                bool isFav = m_favoriteProcesses.count(fw.processName) > 0;
                if (isFav) {
                    DrawIcon("star_filled", 14.0f);
                } else {
                    DrawIcon("star", 14.0f);
                }
                if (ImGui::IsItemClicked()) {
                    ToggleFavorite(fw.processName);
                }
                
                ImGui::SameLine();
                if (displayName.length() > 35) {
                    displayName = displayName.substr(0, 32) + "...";
                }
                ImGui::Text("%s", displayName.c_str());
                if (!fw.windowTitle.empty()) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", fw.processName.c_str());
                }
                
                ImGui::SameLine(ImGui::GetWindowWidth() - 80);
                if (ImGui::SmallButton("Unfreeze")) {
                    UnfreezeWindow(fw);
                    m_frozenWindows.erase(m_frozenWindows.begin() + i);
                    ImGui::PopID();
                    break;
                }
                
                ImGui::PopID();
            }
            ImGui::Unindent();
            ImGui::Spacing();
        }
        
        std::vector<WindowInfo*> favoriteWindows;
        for (auto& win : m_windowList) {
            if (m_favoriteProcesses.count(win.processName) > 0) {
                favoriteWindows.push_back(&win);
            }
        }
        
        if (!favoriteWindows.empty()) {
            DrawIcon("star_filled", 16.0f);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Favorites (%d)", static_cast<int>(favoriteWindows.size()));
            ImGui::Indent();
            
            for (auto* win : favoriteWindows) {
                std::string searchText = win->title + win->processName;
                std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
                if (!searchQuery.empty() && searchText.find(searchQuery) == std::string::npos) {
                    continue;
                }
                
                bool isFrozen = false;
                for (const auto& fw : m_frozenWindows) {
                    if (fw.processName == win->processName) {
                        isFrozen = true;
                        break;
                    }
                }
                if (isFrozen) continue;
                
                ImGui::PushID(("fav_" + std::to_string(win->hwnd)).c_str());
                
                DrawIcon("star_filled", 14.0f);
                if (ImGui::IsItemClicked()) {
                    ToggleFavorite(win->processName);
                }
                
                ImGui::SameLine();
                std::string displayTitle = win->title.empty() ? win->processName : win->title;
                if (displayTitle.length() > 35) {
                    displayTitle = displayTitle.substr(0, 32) + "...";
                }
                ImGui::Text("%s", displayTitle.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", win->processName.c_str());
                
                ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                if (ImGui::SmallButton("Freeze")) {
                    std::stringstream ss;
                    ss << "0x" << std::hex << win->hwnd;
                    FreezeWindow("handle", ss.str(), win->processName);
                }
                
                ImGui::PopID();
            }
            ImGui::Unindent();
            ImGui::Spacing();
        }
        
        auto& groups = m_appGroupsManager.GetGroups();
        if (!groups.empty()) {
            DrawIcon("app_group", 16.0f);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "App Groups (%d)", static_cast<int>(groups.size()));
            
            for (const auto& group : groups) {
                ImGui::PushID(("group_" + group.name).c_str());
                
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
                
                ImGui::Indent();
                DrawIcon("folder", 14.0f);
                ImGui::SameLine();
                bool open = ImGui::TreeNodeEx(("##grp_" + group.name).c_str(), flags);
                ImGui::SameLine();
                ImGui::Text("%s (%d apps)", group.name.c_str(), static_cast<int>(group.apps.size()));
                
                ImGui::SameLine(ImGui::GetWindowWidth() - 160);
                if (ImGui::SmallButton("Freeze All")) {
                    ExecuteOnAppGroup(group.name, "freeze");
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Unfreeze")) {
                    ExecuteOnAppGroup(group.name, "unfreeze");
                }
                
                if (open) {
                    for (size_t j = 0; j < group.apps.size(); ++j) {
                        const auto& app = group.apps[j];
                        
                        std::string searchText = app.name + app.targetValue;
                        std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
                        if (!searchQuery.empty() && searchText.find(searchQuery) == std::string::npos) {
                            continue;
                        }
                        
                        ImGui::PushID(("app_" + std::to_string(j)).c_str());
                        
                        auto frozenIt = std::find_if(m_frozenWindows.begin(), m_frozenWindows.end(),
                            [&](const FrozenWindow& fw) { 
                                return fw.targetValue == app.targetValue || fw.processName == app.targetValue; 
                            });
                        bool isAppFrozen = (frozenIt != m_frozenWindows.end());
                        
                        bool isFav = m_favoriteProcesses.count(app.targetValue) > 0;
                        if (isFav) {
                            DrawIcon("star_filled", 14.0f);
                        } else {
                            DrawIcon("star", 14.0f);
                        }
                        if (ImGui::IsItemClicked()) {
                            ToggleFavorite(app.targetValue);
                        }
                        
                        ImGui::SameLine();
                        DrawIcon("window", 14.0f);
                        ImGui::SameLine();
                        ImGui::Text("%s", app.name.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s]", app.targetValue.c_str());
                        
                        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
                        if (isAppFrozen) {
                            if (ImGui::SmallButton("Unfreeze")) {
                                for (auto it = m_frozenWindows.begin(); it != m_frozenWindows.end(); ) {
                                    if (it->targetValue == app.targetValue || it->processName == app.targetValue) {
                                        UnfreezeWindow(*it);
                                        it = m_frozenWindows.erase(it);
                                    } else {
                                        ++it;
                                    }
                                }
                            }
                        } else {
                            if (ImGui::SmallButton("Freeze")) {
                                FreezeWindow(app.targetType, app.targetValue, 
                                            app.targetType == "process" ? app.targetValue : "",
                                            "", "", app.recursive);
                            }
                        }
                        
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                ImGui::Unindent();
                
                ImGui::PopID();
            }
            ImGui::Spacing();
        }
        
        DrawIcon("window", 16.0f);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "All Windows (%d)", static_cast<int>(m_windowList.size()));
        ImGui::Indent();
        
        int displayedWindows = 0;
        for (auto& win : m_windowList) {
            bool isFrozen = false;
            for (const auto& fw : m_frozenWindows) {
                if (fw.processName == win.processName) {
                    isFrozen = true;
                    break;
                }
            }
            if (isFrozen) continue;
            
            bool isFav = m_favoriteProcesses.count(win.processName) > 0;
            if (isFav) continue;
            
            std::string searchText = win.title + win.processName;
            std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
            if (!searchQuery.empty() && searchText.find(searchQuery) == std::string::npos) {
                continue;
            }
            
            if (displayedWindows >= 100) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "... and more (use search to filter)");
                break;
            }
            
            ImGui::PushID(("win_" + std::to_string(win.hwnd)).c_str());
            
            DrawIcon("star", 14.0f);
            if (ImGui::IsItemClicked()) {
                ToggleFavorite(win.processName);
            }
            
            ImGui::SameLine();
            std::string displayName = win.title.empty() ? win.processName : win.title;
            if (displayName.length() > 40) {
                displayName = displayName.substr(0, 37) + "...";
            }
            ImGui::Text("%s", displayName.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", win.processName.c_str());
            
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::SmallButton("Freeze")) {
                std::stringstream ss;
                ss << "0x" << std::hex << win.hwnd;
                FreezeWindow("handle", ss.str(), win.processName);
            }
            
            ImGui::PopID();
            displayedWindows++;
        }
        
        ImGui::Unindent();
        ImGui::EndChild();
        
        if (!m_frozenWindows.empty()) {
            if (ImGui::Button("Unfreeze All", ImVec2(100, 0))) {
                for (const auto& fw : m_frozenWindows) {
                    UnfreezeWindow(fw);
                }
                m_frozenWindows.clear();
            }
        }
    }
    ImGui::End();
}

static bool PathStartsWith(const std::string& path, const std::string& prefix, bool recursive) {
    std::string normPath = path;
    std::string normPrefix = prefix;
    for (char& c : normPath) if (c == '/') c = '\\';
    for (char& c : normPrefix) if (c == '/') c = '\\';
    while (!normPrefix.empty() && normPrefix.back() == '\\') normPrefix.pop_back();
    
    if (normPath.length() <= normPrefix.length()) return false;
    
    std::string pathLower = normPath;
    std::string prefixLower = normPrefix;
    for (char& c : pathLower) c = static_cast<char>(tolower(c));
    for (char& c : prefixLower) c = static_cast<char>(tolower(c));
    
    if (pathLower.substr(0, prefixLower.length()) != prefixLower) return false;
    if (normPath[prefixLower.length()] != '\\') return false;
    
    if (!recursive) {
        std::string remainder = normPath.substr(prefixLower.length() + 1);
        if (remainder.find('\\') != std::string::npos) return false;
    }
    
    return true;
}

void UIApp::FreezeWindow(const std::string& targetType, const std::string& targetValue, const std::string& processName, const std::string& className, const std::string& windowTitle, bool recursive) {
    RefreshWindowList();
    
    std::vector<WindowInfo> matchingWindows;
    std::set<DWORD> uniquePIDs;
    std::string capturedProcess = processName;
    
    for (const auto& win : m_windowList) {
        bool match = false;
        if (targetType == "process" && win.processName == targetValue) match = true;
        else if (targetType == "class" && win.className == targetValue) match = true;
        else if ((targetType == "title" || targetType == "ititle") && win.title.find(targetValue) != std::string::npos) match = true;
        else if (targetType == "handle") {
            std::stringstream ss;
            ss << "0x" << std::hex << win.hwnd;
            if (ss.str() == targetValue) match = true;
        }
        else if (targetType == "folder" && !win.processPath.empty()) {
            match = PathStartsWith(win.processPath, targetValue, recursive);
        }
        
        if (match) {
            matchingWindows.push_back(win);
            if (win.processId != 0) uniquePIDs.insert(win.processId);
            if (capturedProcess.empty()) capturedProcess = win.processName;
        }
    }
    
    for (const auto& win : matchingWindows) {
        std::stringstream ss;
        ss << std::hex << win.hwnd;
        std::string hexHandle = "0x" + ss.str();
        
        std::string hideCmd = "win hide handle " + hexHandle;
        m_nircmdManager->Execute(hideCmd);
        
        FrozenWindow fw;
        fw.targetType = targetType;
        fw.targetValue = targetValue;
        fw.processName = win.processName;
        fw.className = win.className;
        fw.windowTitle = win.title;
        fw.hwnd = win.hwnd;
        fw.processId = win.processId;
        fw.isFrozen = true;
        m_frozenWindows.push_back(fw);
    }
    
    if (matchingWindows.empty()) {
        std::string hideCmd = "win hide " + targetType + " \"" + targetValue + "\"";
        m_nircmdManager->Execute(hideCmd);
        
        FrozenWindow fw;
        fw.targetType = targetType;
        fw.targetValue = targetValue;
        fw.processName = capturedProcess.empty() ? (targetType == "process" ? targetValue : "") : capturedProcess;
        fw.className = className;
        fw.windowTitle = windowTitle;
        fw.hwnd = 0;
        fw.processId = 0;
        fw.isFrozen = true;
        m_frozenWindows.push_back(fw);
    }
    
    int suspendedCount = 0;
    if (!uniquePIDs.empty()) {
        for (DWORD pid : uniquePIDs) {
            std::string suspendCmd = "suspendprocess /" + std::to_string(pid);
            m_nircmdManager->Execute(suspendCmd);
            suspendedCount++;
        }
    } else if (targetType == "process" || !capturedProcess.empty()) {
        std::string proc = capturedProcess.empty() ? targetValue : capturedProcess;
        std::string suspendCmd = "suspendprocess " + proc;
        m_nircmdManager->Execute(suspendCmd);
        suspendedCount = 1;
    }
    
    int windowCount = matchingWindows.empty() ? 1 : static_cast<int>(matchingWindows.size());
    m_lastOutput = "Frozen: " + targetValue + " (" + std::to_string(windowCount) + " window" + 
                   (windowCount > 1 ? "s" : "") + ", " + std::to_string(suspendedCount) + " process" +
                   (suspendedCount > 1 ? "es" : "") + ")";
    m_lastError.clear();
}

void UIApp::UnfreezeWindow(const FrozenWindow& fw) {
    if (fw.processId != 0) {
        std::string resumeCmd = "resumeprocess /" + std::to_string(fw.processId);
        m_nircmdManager->Execute(resumeCmd);
    } else if (!fw.processName.empty()) {
        std::string resumeCmd = "resumeprocess " + fw.processName;
        m_nircmdManager->Execute(resumeCmd);
    }
    
    if (fw.hwnd != 0) {
        std::stringstream ss;
        ss << std::hex << fw.hwnd;
        std::string hexHandle = "0x" + ss.str();
        
        std::string showCmd = "win show handle " + hexHandle;
        m_nircmdManager->Execute(showCmd);
        
        std::string normalCmd = "win normal handle " + hexHandle;
        m_nircmdManager->Execute(normalCmd);
        
        std::string activateCmd = "win activate handle " + hexHandle;
        m_nircmdManager->Execute(activateCmd);
    } else {
        std::string showCmd = "win show " + fw.targetType + " \"" + fw.targetValue + "\"";
        m_nircmdManager->Execute(showCmd);
        
        std::string normalCmd = "win normal " + fw.targetType + " \"" + fw.targetValue + "\"";
        m_nircmdManager->Execute(normalCmd);
    }
    
    m_lastOutput = "Unfrozen: " + fw.targetValue;
    m_lastError.clear();
}

struct EnumWindowsData {
    std::vector<WindowInfo>* windows;
};

static BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
    if (!hwnd || !lParam) return TRUE;
    
    auto* data = reinterpret_cast<EnumWindowsData*>(lParam);
    if (!data->windows) return TRUE;
    
    if (!IsWindowVisible(hwnd)) return TRUE;
    
    char title[256] = {};
    GetWindowTextA(hwnd, title, sizeof(title) - 1);
    
    char className[256] = {};
    GetClassNameA(hwnd, className, sizeof(className) - 1);
    
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == 0) return TRUE;
    
    std::string procName;
    std::string procPath;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        char processPath[MAX_PATH] = {};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameA(hProcess, 0, processPath, &size)) {
            procPath = processPath;
            procName = procPath;
            size_t lastSlash = procName.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                procName = procName.substr(lastSlash + 1);
            }
        }
        CloseHandle(hProcess);
    }
    
    if (strlen(title) == 0 && procName.empty()) return TRUE;
    if (procName == "NirUI.exe") return TRUE;
    
    WindowInfo info;
    info.title = title;
    info.processName = procName;
    info.processPath = procPath;
    info.className = className;
    info.hwnd = reinterpret_cast<unsigned long long>(hwnd);
    info.processId = processId;
    info.isFrozen = false;
    info.isFavorite = false;
    
    data->windows->push_back(info);
    
    return TRUE;
}

void UIApp::RefreshWindowList() {
    m_windowList.clear();
    
    EnumWindowsData data;
    data.windows = &m_windowList;
    
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&data));
    
    for (auto& win : m_windowList) {
        win.isFavorite = m_favoriteProcesses.count(win.processName) > 0;
    }
}

void UIApp::ToggleFavorite(const std::string& processName) {
    if (m_favoriteProcesses.count(processName) > 0) {
        m_favoriteProcesses.erase(processName);
    } else {
        m_favoriteProcesses.insert(processName);
    }
    
    for (auto& win : m_windowList) {
        win.isFavorite = m_favoriteProcesses.count(win.processName) > 0;
    }
}

void UIApp::SaveFavorites() {
    std::filesystem::path savePath = m_nircmdManager->GetAppDataPath() / "favorites.txt";
    std::ofstream file(savePath);
    if (!file) return;
    
    for (const auto& fav : m_favoriteProcesses) {
        file << fav << "\n";
    }
}

void UIApp::LoadFavorites() {
    std::filesystem::path savePath = m_nircmdManager->GetAppDataPath() / "favorites.txt";
    std::ifstream file(savePath);
    if (!file) return;
    
    m_favoriteProcesses.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            m_favoriteProcesses.insert(line);
        }
    }
}

void UIApp::SaveHistory() {
    std::filesystem::path savePath = m_nircmdManager->GetAppDataPath() / "history.txt";
    std::ofstream file(savePath);
    if (!file) return;
    
    int count = 0;
    for (const auto& entry : m_history) {
        if (count >= 100) break;
        file << (entry.success ? "1" : "0") << "|"
             << entry.executionTime << "|"
             << entry.timestamp << "|"
             << entry.command << "\n";
        count++;
    }
}

void UIApp::LoadHistory() {
    std::filesystem::path savePath = m_nircmdManager->GetAppDataPath() / "history.txt";
    std::ifstream file(savePath);
    if (!file) return;
    
    m_history.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        size_t pos1 = line.find('|');
        size_t pos2 = line.find('|', pos1 + 1);
        size_t pos3 = line.find('|', pos2 + 1);
        
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
            continue;
        }
        
        HistoryEntry entry;
        entry.success = (line.substr(0, pos1) == "1");
        entry.executionTime = std::stod(line.substr(pos1 + 1, pos2 - pos1 - 1));
        entry.timestamp = line.substr(pos2 + 1, pos3 - pos2 - 1);
        entry.command = line.substr(pos3 + 1);
        entry.output = "";
        
        m_history.push_back(entry);
    }
}

void UIApp::ExecuteOnAppGroup(const std::string& groupName, const std::string& action) {
    AppGroup* group = m_appGroupsManager.FindGroup(groupName);
    if (!group) return;
    
    bool isFreeze = (action == "freeze");
    bool isUnfreeze = (action == "unfreeze");
    
    for (const auto& app : group->apps) {
        if (isFreeze) {
            FreezeWindow(app.targetType, app.targetValue, 
                        app.targetType == "process" ? app.targetValue : "",
                        "", "", app.recursive);
        }
        else if (isUnfreeze) {
            auto it = std::find_if(m_frozenWindows.begin(), m_frozenWindows.end(),
                [&](const FrozenWindow& fw) { return fw.targetValue == app.targetValue; });
            
            if (it != m_frozenWindows.end()) {
                UnfreezeWindow(*it);
                m_frozenWindows.erase(it);
            } else {
                FrozenWindow tempFw;
                tempFw.targetType = app.targetType;
                tempFw.targetValue = app.targetValue;
                tempFw.processName = (app.targetType == "process") ? app.targetValue : "";
                UnfreezeWindow(tempFw);
            }
        }
        else {
            std::string cmd = "win " + action + " " + app.targetType + " \"" + app.targetValue + "\"";
            auto result = m_nircmdManager->Execute(cmd);
            AddToHistory(cmd, result);
        }
    }
    
    m_lastOutput = "Executed '" + action + "' on " + std::to_string(group->apps.size()) + " apps in group '" + groupName + "'";
    m_lastError.clear();
}

void UIApp::CopyToClipboard(const std::string& text) {
    if (OpenClipboard((HWND)m_hwnd)) {
        EmptyClipboard();
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hGlobal) {
            char* pGlobal = static_cast<char*>(GlobalLock(hGlobal));
            memcpy(pGlobal, text.c_str(), text.size() + 1);
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_TEXT, hGlobal);
        }
        CloseClipboard();
    }
}

std::string UIApp::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%H:%M:%S");
    return oss.str();
}

} // namespace NirUI
