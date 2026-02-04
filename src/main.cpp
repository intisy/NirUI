#include "cli/cli_parser.h"
#include "core/nircmd_manager.h"
#include "core/nircmd_commands.h"
#include "core/app_groups.h"

#ifndef NIRUI_CLI_MODE
#include "ui/ui_app.h"
#endif

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <algorithm>
#include <set>

using namespace NirUI;

static bool PathStartsWithFolder(const std::string& path, const std::string& folder, bool recursive) {
    std::string normPath = path;
    std::string normFolder = folder;
    for (char& c : normPath) if (c == '/') c = '\\';
    for (char& c : normFolder) if (c == '/') c = '\\';
    while (!normFolder.empty() && normFolder.back() == '\\') normFolder.pop_back();
    
    if (normPath.length() <= normFolder.length()) return false;
    
    std::string pathLower = normPath;
    std::string folderLower = normFolder;
    std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), ::tolower);
    std::transform(folderLower.begin(), folderLower.end(), folderLower.begin(), ::tolower);
    
    if (pathLower.substr(0, folderLower.length()) != folderLower) return false;
    if (normPath[folderLower.length()] != '\\') return false;
    
    if (!recursive) {
        std::string remainder = normPath.substr(folderLower.length() + 1);
        if (remainder.find('\\') != std::string::npos) return false;
    }
    return true;
}

struct ProcessInfo {
    DWORD pid;
    std::string name;
    std::string path;
};

static std::string WideToNarrow(const wchar_t* wide) {
    if (!wide) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &result[0], len, nullptr, nullptr);
    return result;
}

static std::vector<ProcessInfo> GetRunningProcesses() {
    std::vector<ProcessInfo> processes;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return processes;
    
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    
    if (Process32FirstW(snapshot, &pe)) {
        do {
            ProcessInfo info;
            info.pid = pe.th32ProcessID;
            info.name = WideToNarrow(pe.szExeFile);
            
            HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
            if (hProc) {
                char path[MAX_PATH] = {};
                DWORD size = MAX_PATH;
                if (QueryFullProcessImageNameA(hProc, 0, path, &size)) {
                    info.path = path;
                }
                CloseHandle(hProc);
            }
            processes.push_back(info);
        } while (Process32NextW(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return processes;
}

void ExecuteOnGroup(NirCmdManager& manager, AppGroupsManager& groups, 
                    const std::string& groupName, const std::string& action) {
    AppGroup* group = groups.FindGroup(groupName);
    if (!group) {
        std::cerr << "Group not found: " << groupName << std::endl;
        return;
    }
    
    bool isFreeze = (action == "freeze");
    bool isUnfreeze = (action == "unfreeze");
    
    auto runningProcesses = GetRunningProcesses();
    int totalAffected = 0;
    
    for (const auto& app : group->apps) {
        if (app.targetType == "folder") {
            std::set<DWORD> matchedPIDs;
            for (const auto& proc : runningProcesses) {
                if (!proc.path.empty() && PathStartsWithFolder(proc.path, app.targetValue, app.recursive)) {
                    matchedPIDs.insert(proc.pid);
                }
            }
            
            for (DWORD pid : matchedPIDs) {
                if (isFreeze) {
                    manager.Execute("suspendprocess /" + std::to_string(pid));
                    std::cout << "  Frozen PID " << pid << std::endl;
                } else if (isUnfreeze) {
                    manager.Execute("resumeprocess /" + std::to_string(pid));
                    std::cout << "  Unfrozen PID " << pid << std::endl;
                }
            }
            
            if (!isFreeze && !isUnfreeze) {
                for (const auto& proc : runningProcesses) {
                    if (!proc.path.empty() && PathStartsWithFolder(proc.path, app.targetValue, app.recursive)) {
                        std::string cmd = "win " + action + " process \"" + proc.name + "\"";
                        manager.Execute(cmd);
                        std::cout << "  " << action << ": " << proc.name << std::endl;
                    }
                }
            }
            totalAffected += static_cast<int>(matchedPIDs.size());
        } else {
            if (isFreeze) {
                std::string hideCmd = "win hide " + app.targetType + " \"" + app.targetValue + "\"";
                manager.Execute(hideCmd);
                
                if (app.targetType == "process") {
                    std::string suspendCmd = "suspendprocess " + app.targetValue;
                    manager.Execute(suspendCmd);
                }
                std::cout << "  Frozen: " << app.name << std::endl;
            }
            else if (isUnfreeze) {
                if (app.targetType == "process") {
                    std::string resumeCmd = "resumeprocess " + app.targetValue;
                    manager.Execute(resumeCmd);
                }
                
                std::string showCmd = "win show " + app.targetType + " \"" + app.targetValue + "\"";
                manager.Execute(showCmd);
                std::cout << "  Unfrozen: " << app.name << std::endl;
            }
            else {
                std::string cmd = "win " + action + " " + app.targetType + " \"" + app.targetValue + "\"";
                manager.Execute(cmd);
                std::cout << "  " << action << ": " << app.name << std::endl;
            }
            totalAffected++;
        }
    }
    
    std::cout << "Applied '" << action << "' to " << totalAffected << " items in '" << groupName << "'" << std::endl;
}

void AttachOrAllocConsole() {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }
    
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    std::vector<std::string> argStrings;
    std::vector<char*> argv;
    
    for (int i = 0; i < argc; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        std::string str(size - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, &str[0], size, nullptr, nullptr);
        argStrings.push_back(str);
    }
    
    for (auto& s : argStrings) {
        argv.push_back(&s[0]);
    }
    
    LocalFree(argvW);
    
    CliParser parser;
    CliOptions options = parser.Parse(argc, argv.data());
    
    if (options.showHelp && options.command.empty()) {
        AttachOrAllocConsole();
        parser.PrintHelp();
        return 0;
    }
    
    if (options.showVersion) {
        AttachOrAllocConsole();
        parser.PrintVersion();
        return 0;
    }
    
    if (options.listCategories) {
        AttachOrAllocConsole();
        parser.PrintCategories();
        return 0;
    }
    
    if (options.listCommands) {
        AttachOrAllocConsole();
        parser.PrintCommands(options.category);
        return 0;
    }
    
    if (!options.searchQuery.empty()) {
        AttachOrAllocConsole();
        parser.PrintSearchResults(options.searchQuery);
        return 0;
    }
    
    if (options.showHelp && !options.command.empty()) {
        AttachOrAllocConsole();
        parser.PrintCommandDetails(options.command);
        return 0;
    }
    
    if (options.downloadNirCmd) {
        AttachOrAllocConsole();
        std::cout << "Downloading NirCmd..." << std::endl;
        
        NirCmdManager manager;
        bool success = manager.DownloadNirCmd([](int progress, const std::string& status) {
            std::cout << "\r" << status;
            if (progress >= 0) {
                std::cout << " [" << progress << "%]";
            }
            std::cout << "          " << std::flush;
        });
        
        std::cout << std::endl;
        
        if (success) {
            std::cout << "NirCmd downloaded successfully to: " << manager.GetNirCmdPath() << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to download NirCmd." << std::endl;
            return 1;
        }
    }
    
    NirCmdManager nircmdMgr;
    AppGroupsManager appGroups;
    appGroups.SetDataPath(nircmdMgr.GetAppDataPath());
    appGroups.Load();
    
    if (options.listAppGroups) {
        AttachOrAllocConsole();
        std::cout << "\nApp Groups:\n";
        std::cout << "===========\n\n";
        
        if (appGroups.GetGroups().empty()) {
            std::cout << "No app groups defined.\n";
            std::cout << "Use --create-group NAME to create one.\n";
        } else {
            for (const auto& group : appGroups.GetGroups()) {
                std::cout << group.name << " (" << group.apps.size() << " apps):\n";
                for (const auto& app : group.apps) {
                    std::cout << "  - " << app.name << " [" << app.targetType << ": " << app.targetValue << "]\n";
                }
                std::cout << "\n";
            }
        }
        return 0;
    }
    
    if (options.createAppGroup) {
        AttachOrAllocConsole();
        if (options.appGroupName.empty()) {
            std::cerr << "Error: Group name required.\n";
            return 1;
        }
        if (appGroups.CreateGroup(options.appGroupName)) {
            appGroups.Save();
            std::cout << "Created group: " << options.appGroupName << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Group already exists or creation failed.\n";
            return 1;
        }
    }
    
    if (options.deleteAppGroup) {
        AttachOrAllocConsole();
        if (options.appGroupName.empty()) {
            std::cerr << "Error: Group name required.\n";
            return 1;
        }
        if (appGroups.DeleteGroup(options.appGroupName)) {
            appGroups.Save();
            std::cout << "Deleted group: " << options.appGroupName << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Group not found.\n";
            return 1;
        }
    }
    
    if (options.addToAppGroup) {
        AttachOrAllocConsole();
        if (options.appGroupName.empty() || options.appName.empty() || 
            options.appTargetType.empty() || options.appTargetValue.empty()) {
            std::cerr << "Error: Missing parameters.\n";
            std::cerr << "Usage: --add-app GROUP NAME TYPE VALUE [--recursive]\n";
            return 1;
        }
        if (appGroups.AddApp(options.appGroupName, options.appName, 
                            options.appTargetType, options.appTargetValue,
                            options.appRecursive)) {
            appGroups.Save();
            std::string recursiveStr = (options.appTargetType == "folder" && options.appRecursive) ? " (recursive)" : "";
            std::cout << "Added '" << options.appName << "' to group '" << options.appGroupName << "'" << recursiveStr << "\n";
            return 0;
        } else {
            std::cerr << "Error: Group not found.\n";
            return 1;
        }
    }
    
    if (options.removeFromAppGroup) {
        AttachOrAllocConsole();
        if (options.appGroupName.empty() || options.appName.empty()) {
            std::cerr << "Error: Missing parameters.\n";
            std::cerr << "Usage: --remove-app GROUP NAME\n";
            return 1;
        }
        if (appGroups.RemoveApp(options.appGroupName, options.appName)) {
            appGroups.Save();
            std::cout << "Removed '" << options.appName << "' from group '" << options.appGroupName << "'\n";
            return 0;
        } else {
            std::cerr << "Error: Group or app not found.\n";
            return 1;
        }
    }
    
    if (options.runOnAppGroup) {
        AttachOrAllocConsole();
        if (options.appGroupName.empty() || options.appGroupAction.empty()) {
            std::cerr << "Error: Missing parameters.\n";
            std::cerr << "Usage: --run-group GROUP ACTION\n";
            return 1;
        }
        if (!nircmdMgr.IsAvailable()) {
            std::cerr << "Error: NirCmd not found. Use --download first.\n";
            return 1;
        }
        ExecuteOnGroup(nircmdMgr, appGroups, options.appGroupName, options.appGroupAction);
        return 0;
    }
    
    if (!options.command.empty()) {
        AttachOrAllocConsole();
        
        NirCmdManager manager;
        
        if (!manager.IsAvailable()) {
            std::cerr << "Error: NirCmd not found. Use --download to download it." << std::endl;
            return 1;
        }
        
        std::string cmdLine = options.command;
        for (const auto& arg : options.commandArgs) {
            cmdLine += " ";
            if (arg.find(' ') != std::string::npos && arg[0] != '"') {
                cmdLine += "\"" + arg + "\"";
            } else {
                cmdLine += arg;
            }
        }
        
        if (cmdLine.substr(0, 10) == "win freeze" || cmdLine.substr(0, 12) == "win unfreeze") {
            bool isFreeze = cmdLine.substr(0, 10) == "win freeze";
            std::string rest = cmdLine.substr(isFreeze ? 11 : 13);
            
            size_t spacePos = rest.find(' ');
            if (spacePos != std::string::npos) {
                std::string findType = rest.substr(0, spacePos);
                std::string findValue = rest.substr(spacePos + 1);
                if (!findValue.empty() && findValue.front() == '"' && findValue.back() == '"') {
                    findValue = findValue.substr(1, findValue.length() - 2);
                }
                
                bool recursive = false;
                size_t recPos = findValue.find(" --recursive");
                if (recPos != std::string::npos) {
                    recursive = true;
                    findValue = findValue.substr(0, recPos);
                }
                
                if (findType == "folder") {
                    auto runningProcesses = GetRunningProcesses();
                    int affected = 0;
                    for (const auto& proc : runningProcesses) {
                        if (!proc.path.empty() && PathStartsWithFolder(proc.path, findValue, recursive)) {
                            if (isFreeze) {
                                manager.Execute("win hide handle /" + std::to_string(proc.pid));
                                manager.Execute("suspendprocess /" + std::to_string(proc.pid));
                            } else {
                                manager.Execute("resumeprocess /" + std::to_string(proc.pid));
                                manager.Execute("win show handle /" + std::to_string(proc.pid));
                            }
                            std::cout << (isFreeze ? "Frozen: " : "Unfrozen: ") << proc.name << " (PID " << proc.pid << ")" << std::endl;
                            affected++;
                        }
                    }
                    std::cout << "Total affected: " << affected << " process(es)" << std::endl;
                    return 0;
                }
                else if (findType == "process") {
                    auto runningProcesses = GetRunningProcesses();
                    int affected = 0;
                    for (const auto& proc : runningProcesses) {
                        if (_stricmp(proc.name.c_str(), findValue.c_str()) == 0) {
                            if (isFreeze) {
                                manager.Execute("win hide handle /" + std::to_string(proc.pid));
                                manager.Execute("suspendprocess /" + std::to_string(proc.pid));
                            } else {
                                manager.Execute("resumeprocess /" + std::to_string(proc.pid));
                                manager.Execute("win show handle /" + std::to_string(proc.pid));
                            }
                            std::cout << (isFreeze ? "Frozen: " : "Unfrozen: ") << proc.name << " (PID " << proc.pid << ")" << std::endl;
                            affected++;
                        }
                    }
                    std::cout << "Total affected: " << affected << " process(es)" << std::endl;
                    return 0;
                }
                else {
                    if (isFreeze) {
                        manager.Execute("win hide " + findType + " \"" + findValue + "\"");
                        if (findType == "handle") {
                            manager.Execute("suspendprocess /" + findValue);
                        }
                    } else {
                        if (findType == "handle") {
                            manager.Execute("resumeprocess /" + findValue);
                        }
                        manager.Execute("win show " + findType + " \"" + findValue + "\"");
                    }
                    std::cout << (isFreeze ? "Frozen" : "Unfrozen") << ": " << findValue << std::endl;
                    return 0;
                }
            }
        }
        
        if (options.verbose) {
            std::cout << "Executing: nircmd " << cmdLine << std::endl;
        }
        
        auto result = manager.Execute(cmdLine);
        
        if (!result.output.empty()) {
            std::cout << result.output;
        }
        
        if (!result.error.empty()) {
            std::cerr << result.error;
        }
        
        if (options.verbose) {
            std::cout << "\nExecution time: " << result.executionTimeMs << " ms" << std::endl;
            std::cout << "Exit code: " << result.exitCode << std::endl;
        }
        
        return result.exitCode;
    }
    
#ifndef NIRUI_CLI_MODE
    UIApp app;
    return app.Run();
#else
    AttachOrAllocConsole();
    parser.PrintHelp();
    return 0;
#endif
}

#ifdef NIRUI_CLI_MODE
int main(int argc, char* argv[]) {
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOW);
}
#endif
