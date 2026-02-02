#include "cli/cli_parser.h"
#include "core/nircmd_manager.h"
#include "core/nircmd_commands.h"
#include "core/app_groups.h"

#ifndef NIRUI_CLI_MODE
#include "ui/ui_app.h"
#endif

#include <iostream>
#include <windows.h>

using namespace NirUI;

void ExecuteOnGroup(NirCmdManager& manager, AppGroupsManager& groups, 
                    const std::string& groupName, const std::string& action) {
    AppGroup* group = groups.FindGroup(groupName);
    if (!group) {
        std::cerr << "Group not found: " << groupName << std::endl;
        return;
    }
    
    bool isFreeze = (action == "freeze");
    bool isUnfreeze = (action == "unfreeze");
    
    for (const auto& app : group->apps) {
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
    }
    
    std::cout << "Applied '" << action << "' to " << group->apps.size() << " apps in '" << groupName << "'" << std::endl;
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
            std::cerr << "Usage: --add-app GROUP NAME TYPE VALUE\n";
            return 1;
        }
        if (appGroups.AddApp(options.appGroupName, options.appName, 
                            options.appTargetType, options.appTargetValue)) {
            appGroups.Save();
            std::cout << "Added '" << options.appName << "' to group '" << options.appGroupName << "'\n";
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
