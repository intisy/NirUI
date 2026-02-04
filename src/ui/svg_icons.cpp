#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg.h"
#include "nanosvgrast.h"
#include "svg_icons.h"
#include <cstring>

namespace NirUI {

namespace SvgData {

const char* VOLUME = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"/>
  <path d="M15.54 8.46a5 5 0 0 1 0 7.07"/>
  <path d="M19.07 4.93a10 10 0 0 1 0 14.14"/>
</svg>)";

const char* MONITOR = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="2" y="3" width="20" height="14" rx="2" ry="2"/>
  <line x1="8" y1="21" x2="16" y2="21"/>
  <line x1="12" y1="17" x2="12" y2="21"/>
</svg>)";

const char* SETTINGS = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <circle cx="12" cy="12" r="3"/>
  <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/>
</svg>)";

const char* WINDOW = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="3" y="3" width="18" height="18" rx="2" ry="2"/>
  <line x1="3" y1="9" x2="21" y2="9"/>
  <line x1="9" y1="21" x2="9" y2="9"/>
</svg>)";

const char* FOLDER = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/>
</svg>)";

const char* CLIPBOARD = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M16 4h2a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V6a2 2 0 0 1 2-2h2"/>
  <rect x="8" y="2" width="8" height="4" rx="1" ry="1"/>
</svg>)";

const char* KEYBOARD = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="2" y="4" width="20" height="16" rx="2" ry="2"/>
  <line x1="6" y1="8" x2="6" y2="8"/>
  <line x1="10" y1="8" x2="10" y2="8"/>
  <line x1="14" y1="8" x2="14" y2="8"/>
  <line x1="18" y1="8" x2="18" y2="8"/>
  <line x1="6" y1="12" x2="6" y2="12"/>
  <line x1="10" y1="12" x2="10" y2="12"/>
  <line x1="14" y1="12" x2="14" y2="12"/>
  <line x1="18" y1="12" x2="18" y2="12"/>
  <line x1="6" y1="16" x2="18" y2="16"/>
</svg>)";

const char* PROCESS = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="4" y="4" width="16" height="16" rx="2" ry="2"/>
  <rect x="9" y="9" width="6" height="6"/>
  <line x1="9" y1="2" x2="9" y2="4"/>
  <line x1="15" y1="2" x2="15" y2="4"/>
  <line x1="9" y1="20" x2="9" y2="22"/>
  <line x1="15" y1="20" x2="15" y2="22"/>
  <line x1="20" y1="9" x2="22" y2="9"/>
  <line x1="20" y1="14" x2="22" y2="14"/>
  <line x1="2" y1="9" x2="4" y2="9"/>
  <line x1="2" y1="14" x2="4" y2="14"/>
</svg>)";

const char* POWER = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M18.36 6.64a9 9 0 1 1-12.73 0"/>
  <line x1="12" y1="2" x2="12" y2="12"/>
</svg>)";

const char* NETWORK = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <circle cx="12" cy="12" r="10"/>
  <line x1="2" y1="12" x2="22" y2="12"/>
  <path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/>
</svg>)";

const char* MOUSE = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="6" y="2" width="12" height="20" rx="6" ry="6"/>
  <line x1="12" y1="6" x2="12" y2="10"/>
</svg>)";

const char* DIALOG = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M21 15a2 2 0 0 1-2 2H7l-4 4V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2z"/>
</svg>)";

const char* SHORTCUT = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M10 13a5 5 0 0 0 7.54.54l3-3a5 5 0 0 0-7.07-7.07l-1.72 1.71"/>
  <path d="M14 11a5 5 0 0 0-7.54-.54l-3 3a5 5 0 0 0 7.07 7.07l1.71-1.71"/>
</svg>)";

const char* SYSTEM = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="2" y="2" width="20" height="8" rx="2" ry="2"/>
  <rect x="2" y="14" width="20" height="8" rx="2" ry="2"/>
  <line x1="6" y1="6" x2="6" y2="6"/>
  <line x1="6" y1="18" x2="6" y2="18"/>
</svg>)";

const char* REGISTRY = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"/>
  <path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"/>
  <line x1="8" y1="7" x2="16" y2="7"/>
  <line x1="8" y1="11" x2="16" y2="11"/>
</svg>)";

const char* DISPLAY = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="2" y="3" width="20" height="14" rx="2" ry="2"/>
  <line x1="8" y1="21" x2="16" y2="21"/>
  <line x1="12" y1="17" x2="12" y2="21"/>
</svg>)";

const char* AUDIO = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <path d="M9 18V5l12-2v13"/>
  <circle cx="6" cy="18" r="3"/>
  <circle cx="18" cy="16" r="3"/>
</svg>)";

const char* TIME = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <circle cx="12" cy="12" r="10"/>
  <polyline points="12 6 12 12 16 14"/>
</svg>)";

const char* STAR = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/>
</svg>)";

const char* STAR_FILLED = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor" stroke-width="2">
  <polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/>
</svg>)";

const char* FREEZE = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <line x1="12" y1="2" x2="12" y2="22"/>
  <line x1="2" y1="12" x2="22" y2="12"/>
  <line x1="4.93" y1="4.93" x2="19.07" y2="19.07"/>
  <line x1="19.07" y1="4.93" x2="4.93" y2="19.07"/>
</svg>)";

const char* PLAY = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <polygon points="5 3 19 12 5 21 5 3"/>
</svg>)";

const char* SEARCH = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <circle cx="11" cy="11" r="8"/>
  <line x1="21" y1="21" x2="16.65" y2="16.65"/>
</svg>)";

const char* CLOSE = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <line x1="18" y1="6" x2="6" y2="18"/>
  <line x1="6" y1="6" x2="18" y2="18"/>
</svg>)";

const char* MINIMIZE = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <line x1="5" y1="12" x2="19" y2="12"/>
</svg>)";

const char* MAXIMIZE = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="3" y="3" width="18" height="18" rx="2" ry="2"/>
</svg>)";

const char* APP_GROUP = R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
  <rect x="3" y="3" width="7" height="7"/>
  <rect x="14" y="3" width="7" height="7"/>
  <rect x="14" y="14" width="7" height="7"/>
  <rect x="3" y="14" width="7" height="7"/>
</svg>)";

} // namespace SvgData

SvgIconManager::SvgIconManager() {}

SvgIconManager::~SvgIconManager() {
    Cleanup();
}

void SvgIconManager::Initialize(ID3D11Device* device) {
    m_device = device;
    LoadBuiltinIcons();
}

void SvgIconManager::Cleanup() {
    for (auto& pair : m_icons) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    m_icons.clear();
}

ID3D11ShaderResourceView* SvgIconManager::LoadSvgFromMemory(const char* svgData, int size) {
    if (!m_device || !svgData || size <= 0) return nullptr;
    
    // nsvgParse modifies the string, so we need a mutable copy
    char* svgCopy = new char[size + 1];
    memcpy(svgCopy, svgData, size);
    svgCopy[size] = '\0';
    
    NSVGimage* image = nsvgParse(svgCopy, "px", 96.0f);
    delete[] svgCopy;
    
    if (!image) return nullptr;
    
    if (image->width <= 0 || image->height <= 0) {
        nsvgDelete(image);
        return nullptr;
    }
    
    int iconSize = 16;
    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) {
        nsvgDelete(image);
        return nullptr;
    }
    
    unsigned char* pixels = new unsigned char[iconSize * iconSize * 4];
    memset(pixels, 0, iconSize * iconSize * 4);
    
    float maxDim = image->width > image->height ? image->width : image->height;
    float scale = (float)iconSize / maxDim;
    nsvgRasterize(rast, image, 0, 0, scale, pixels, iconSize, iconSize, iconSize * 4);
    
    for (int i = 0; i < iconSize * iconSize; ++i) {
        unsigned char r = pixels[i * 4 + 0];
        unsigned char g = pixels[i * 4 + 1];
        unsigned char b = pixels[i * 4 + 2];
        unsigned char a = pixels[i * 4 + 3];
        
        if (a > 0) {
            pixels[i * 4 + 0] = 200;
            pixels[i * 4 + 1] = 200;
            pixels[i * 4 + 2] = 200;
        }
    }
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = iconSize;
    desc.Height = iconSize;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels;
    initData.SysMemPitch = iconSize * 4;
    
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&desc, &initData, &texture);
    
    delete[] pixels;
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
    
    if (FAILED(hr)) return nullptr;
    
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    
    hr = m_device->CreateShaderResourceView(texture, &srvDesc, &srv);
    texture->Release();
    
    if (FAILED(hr)) return nullptr;
    
    return srv;
}

void SvgIconManager::LoadBuiltinIcons() {
    const std::pair<std::string, const char*> icons[] = {
        {"volume", SvgData::VOLUME},
        {"monitor", SvgData::MONITOR},
        {"settings", SvgData::SETTINGS},
        {"window", SvgData::WINDOW},
        {"folder", SvgData::FOLDER},
        {"clipboard", SvgData::CLIPBOARD},
        {"keyboard", SvgData::KEYBOARD},
        {"process", SvgData::PROCESS},
        {"power", SvgData::POWER},
        {"network", SvgData::NETWORK},
        {"mouse", SvgData::MOUSE},
        {"dialog", SvgData::DIALOG},
        {"shortcut", SvgData::SHORTCUT},
        {"system", SvgData::SYSTEM},
        {"registry", SvgData::REGISTRY},
        {"display", SvgData::DISPLAY},
        {"audio", SvgData::AUDIO},
        {"time", SvgData::TIME},
        {"star", SvgData::STAR},
        {"star_filled", SvgData::STAR_FILLED},
        {"freeze", SvgData::FREEZE},
        {"play", SvgData::PLAY},
        {"search", SvgData::SEARCH},
        {"close", SvgData::CLOSE},
        {"minimize", SvgData::MINIMIZE},
        {"maximize", SvgData::MAXIMIZE},
        {"app_group", SvgData::APP_GROUP},
    };
    
    for (const auto& icon : icons) {
        auto srv = LoadSvgFromMemory(icon.second, static_cast<int>(strlen(icon.second)));
        if (srv) {
            m_icons[icon.first] = srv;
        }
    }
}

ID3D11ShaderResourceView* SvgIconManager::GetIcon(const std::string& name) {
    auto it = m_icons.find(name);
    if (it != m_icons.end()) {
        return it->second;
    }
    return nullptr;
}

const char* SvgIconManager::GetSvgData(const std::string& name) {
    if (name == "volume") return SvgData::VOLUME;
    if (name == "monitor") return SvgData::MONITOR;
    if (name == "settings") return SvgData::SETTINGS;
    if (name == "window") return SvgData::WINDOW;
    if (name == "folder") return SvgData::FOLDER;
    if (name == "clipboard") return SvgData::CLIPBOARD;
    if (name == "keyboard") return SvgData::KEYBOARD;
    if (name == "process") return SvgData::PROCESS;
    if (name == "power") return SvgData::POWER;
    if (name == "network") return SvgData::NETWORK;
    if (name == "mouse") return SvgData::MOUSE;
    if (name == "dialog") return SvgData::DIALOG;
    if (name == "shortcut") return SvgData::SHORTCUT;
    if (name == "system") return SvgData::SYSTEM;
    if (name == "registry") return SvgData::REGISTRY;
    if (name == "display") return SvgData::DISPLAY;
    if (name == "audio") return SvgData::AUDIO;
    if (name == "time") return SvgData::TIME;
    if (name == "star") return SvgData::STAR;
    if (name == "star_filled") return SvgData::STAR_FILLED;
    if (name == "freeze") return SvgData::FREEZE;
    if (name == "play") return SvgData::PLAY;
    if (name == "search") return SvgData::SEARCH;
    if (name == "close") return SvgData::CLOSE;
    if (name == "minimize") return SvgData::MINIMIZE;
    if (name == "maximize") return SvgData::MAXIMIZE;
    if (name == "app_group") return SvgData::APP_GROUP;
    return nullptr;
}

} // namespace NirUI
