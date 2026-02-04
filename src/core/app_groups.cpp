#include "app_groups.h"
#include <fstream>
#include <algorithm>

namespace NirUI {

AppGroupsManager::AppGroupsManager() {
}

void AppGroupsManager::SetDataPath(const std::filesystem::path& path) {
    m_dataPath = path;
}

void AppGroupsManager::Load() {
    if (m_dataPath.empty()) return;
    
    std::filesystem::path savePath = m_dataPath / "app_groups.txt";
    std::ifstream file(savePath);
    if (!file) return;
    
    m_groups.clear();
    AppGroup* currentGroup = nullptr;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 7) == "[GROUP]") {
            m_groups.push_back(AppGroup());
            currentGroup = &m_groups.back();
            currentGroup->name = line.substr(7);
        } else if (currentGroup && !line.empty()) {
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);
            size_t pos3 = line.find('|', pos2 + 1);
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                AppEntry entry;
                entry.name = line.substr(0, pos1);
                entry.targetType = line.substr(pos1 + 1, pos2 - pos1 - 1);
                if (pos3 != std::string::npos) {
                    entry.targetValue = line.substr(pos2 + 1, pos3 - pos2 - 1);
                    entry.recursive = (line.substr(pos3 + 1) == "1");
                } else {
                    entry.targetValue = line.substr(pos2 + 1);
                    entry.recursive = false;
                }
                currentGroup->apps.push_back(entry);
            }
        }
    }
}

void AppGroupsManager::Save() {
    if (m_dataPath.empty()) return;
    
    std::filesystem::path savePath = m_dataPath / "app_groups.txt";
    std::ofstream file(savePath);
    if (!file) return;
    
    for (const auto& group : m_groups) {
        file << "[GROUP]" << group.name << "\n";
        for (const auto& app : group.apps) {
            file << app.name << "|" << app.targetType << "|" << app.targetValue << "|" << (app.recursive ? "1" : "0") << "\n";
        }
    }
}

AppGroup* AppGroupsManager::FindGroup(const std::string& name) {
    for (auto& group : m_groups) {
        if (group.name == name) {
            return &group;
        }
    }
    return nullptr;
}

bool AppGroupsManager::CreateGroup(const std::string& name) {
    if (FindGroup(name)) return false;
    
    AppGroup group;
    group.name = name;
    m_groups.push_back(group);
    return true;
}

bool AppGroupsManager::DeleteGroup(const std::string& name) {
    auto it = std::find_if(m_groups.begin(), m_groups.end(),
        [&name](const AppGroup& g) { return g.name == name; });
    
    if (it != m_groups.end()) {
        m_groups.erase(it);
        return true;
    }
    return false;
}

bool AppGroupsManager::AddApp(const std::string& groupName, const std::string& appName,
                              const std::string& targetType, const std::string& targetValue,
                              bool recursive) {
    AppGroup* group = FindGroup(groupName);
    if (!group) return false;
    
    AppEntry entry;
    entry.name = appName;
    entry.targetType = targetType;
    entry.targetValue = targetValue;
    entry.recursive = recursive;
    group->apps.push_back(entry);
    return true;
}

bool AppGroupsManager::RemoveApp(const std::string& groupName, const std::string& appName) {
    AppGroup* group = FindGroup(groupName);
    if (!group) return false;
    
    auto it = std::find_if(group->apps.begin(), group->apps.end(),
        [&appName](const AppEntry& e) { return e.name == appName; });
    
    if (it != group->apps.end()) {
        group->apps.erase(it);
        return true;
    }
    return false;
}

} // namespace NirUI
