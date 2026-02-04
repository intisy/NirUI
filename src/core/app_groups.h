#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace NirUI {

struct AppEntry {
    std::string name;
    std::string targetType;
    std::string targetValue;
    bool recursive = false;
};

struct AppGroup {
    std::string name;
    std::vector<AppEntry> apps;
};

class AppGroupsManager {
public:
    AppGroupsManager();
    
    void SetDataPath(const std::filesystem::path& path);
    void Load();
    void Save();
    
    std::vector<AppGroup>& GetGroups() { return m_groups; }
    const std::vector<AppGroup>& GetGroups() const { return m_groups; }
    
    AppGroup* FindGroup(const std::string& name);
    bool CreateGroup(const std::string& name);
    bool DeleteGroup(const std::string& name);
    bool AddApp(const std::string& groupName, const std::string& appName, 
                const std::string& targetType, const std::string& targetValue,
                bool recursive = false);
    bool RemoveApp(const std::string& groupName, const std::string& appName);
    
private:
    std::vector<AppGroup> m_groups;
    std::filesystem::path m_dataPath;
};

} // namespace NirUI
