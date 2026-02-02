#pragma once

#include <string>
#include <unordered_map>
#include <d3d11.h>

namespace NirUI {

class SvgIconManager {
public:
    SvgIconManager();
    ~SvgIconManager();
    
    void Initialize(ID3D11Device* device);
    void Cleanup();
    
    ID3D11ShaderResourceView* GetIcon(const std::string& name);
    void LoadBuiltinIcons();
    
    static const char* GetSvgData(const std::string& name);
    
private:
    ID3D11ShaderResourceView* LoadSvgFromMemory(const char* svgData, int size);
    
    ID3D11Device* m_device = nullptr;
    std::unordered_map<std::string, ID3D11ShaderResourceView*> m_icons;
};

namespace SvgData {
    extern const char* VOLUME;
    extern const char* MONITOR;
    extern const char* SETTINGS;
    extern const char* WINDOW;
    extern const char* FOLDER;
    extern const char* CLIPBOARD;
    extern const char* KEYBOARD;
    extern const char* PROCESS;
    extern const char* POWER;
    extern const char* NETWORK;
    extern const char* MOUSE;
    extern const char* DIALOG;
    extern const char* SHORTCUT;
    extern const char* SYSTEM;
    extern const char* REGISTRY;
    extern const char* DISPLAY;
    extern const char* AUDIO;
    extern const char* TIME;
    extern const char* STAR;
    extern const char* STAR_FILLED;
    extern const char* FREEZE;
    extern const char* PLAY;
    extern const char* SEARCH;
    extern const char* CLOSE;
    extern const char* MINIMIZE;
    extern const char* MAXIMIZE;
    extern const char* APP_GROUP;
}

} // namespace NirUI
