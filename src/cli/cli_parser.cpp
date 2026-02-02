#include "cli_parser.h"
#include "core/nircmd_commands.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace NirUI {

CliParser::CliParser() : m_programName("NirUI") {
}

CliOptions CliParser::Parse(int argc, char* argv[]) {
    CliOptions options;
    
    if (argc > 0) {
        m_programName = argv[0];
    }
    
    if (argc <= 1) {
        options.launchGui = true;
        return options;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help" || arg == "/?") {
            options.showHelp = true;
        }
        else if (arg == "-v" || arg == "--version") {
            options.showVersion = true;
        }
        else if (arg == "-g" || arg == "--gui") {
            options.launchGui = true;
        }
        else if (arg == "-l" || arg == "--list-categories") {
            options.listCategories = true;
        }
        else if (arg == "-c" || arg == "--category") {
            if (i + 1 < argc) {
                options.category = argv[++i];
                options.listCommands = true;
            }
        }
        else if (arg == "-s" || arg == "--search") {
            if (i + 1 < argc) {
                options.searchQuery = argv[++i];
            }
        }
        else if (arg == "-d" || arg == "--download") {
            options.downloadNirCmd = true;
        }
        else if (arg == "--verbose") {
            options.verbose = true;
        }
        else if (arg == "--commands") {
            options.listCommands = true;
        }
        else if (arg == "--info") {
            if (i + 1 < argc) {
                options.command = argv[++i];
                options.showHelp = true;
            }
        }
        else if (arg == "--groups" || arg == "--list-groups") {
            options.listAppGroups = true;
        }
        else if (arg == "--create-group") {
            options.createAppGroup = true;
            if (i + 1 < argc) {
                options.appGroupName = argv[++i];
            }
        }
        else if (arg == "--delete-group") {
            options.deleteAppGroup = true;
            if (i + 1 < argc) {
                options.appGroupName = argv[++i];
            }
        }
        else if (arg == "--add-app") {
            options.addToAppGroup = true;
            if (i + 4 < argc) {
                options.appGroupName = argv[++i];
                options.appName = argv[++i];
                options.appTargetType = argv[++i];
                options.appTargetValue = argv[++i];
            }
        }
        else if (arg == "--remove-app") {
            options.removeFromAppGroup = true;
            if (i + 2 < argc) {
                options.appGroupName = argv[++i];
                options.appName = argv[++i];
            }
        }
        else if (arg == "--run-group") {
            options.runOnAppGroup = true;
            if (i + 2 < argc) {
                options.appGroupName = argv[++i];
                options.appGroupAction = argv[++i];
            }
        }
        else if (arg[0] != '-') {
            if (options.command.empty()) {
                options.command = arg;
            } else {
                options.commandArgs.push_back(arg);
            }
        }
    }
    
    return options;
}

void CliParser::PrintHelp() const {
    std::cout << "\n";
    std::cout << "  _   _ _      _   _ ___ \n";
    std::cout << " | \\ | (_)_ __| | | |_ _|\n";
    std::cout << " |  \\| | | '__| | | || | \n";
    std::cout << " | |\\  | | |  | |_| || | \n";
    std::cout << " |_| \\_|_|_|   \\___/|___|\n";
    std::cout << "\n";
    std::cout << "NirUI - A graphical and command-line wrapper for NirCmd\n";
    std::cout << "\n";
    std::cout << "USAGE:\n";
    std::cout << "  " << m_programName << " [OPTIONS] [COMMAND] [ARGS...]\n";
    std::cout << "\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -v, --version           Show version information\n";
    std::cout << "  -g, --gui               Launch graphical interface\n";
    std::cout << "  -l, --list-categories   List all command categories\n";
    std::cout << "  -c, --category NAME     List commands in a category\n";
    std::cout << "  --commands              List all available commands\n";
    std::cout << "  -s, --search QUERY      Search for commands\n";
    std::cout << "  -d, --download          Download NirCmd from NirSoft\n";
    std::cout << "  --info COMMAND          Show detailed info about a command\n";
    std::cout << "  --verbose               Show verbose output\n";
    std::cout << "\n";
    std::cout << "APP GROUP OPTIONS:\n";
    std::cout << "  --groups                List all app groups\n";
    std::cout << "  --create-group NAME     Create a new app group\n";
    std::cout << "  --delete-group NAME     Delete an app group\n";
    std::cout << "  --add-app GROUP NAME TYPE VALUE\n";
    std::cout << "                          Add app to group (TYPE: process|class|title|ititle)\n";
    std::cout << "  --remove-app GROUP NAME Remove app from group\n";
    std::cout << "  --run-group GROUP ACTION\n";
    std::cout << "                          Run action on group (min|max|close|hide|show|freeze|unfreeze)\n";
    std::cout << "\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  " << m_programName << "                           Launch GUI\n";
    std::cout << "  " << m_programName << " -l                        List categories\n";
    std::cout << "  " << m_programName << " -c \"Volume Control\"       List volume commands\n";
    std::cout << "  " << m_programName << " -s mute                   Search for mute commands\n";
    std::cout << "  " << m_programName << " mutesysvolume 2           Toggle system mute\n";
    std::cout << "  " << m_programName << " monitor off               Turn off monitor\n";
    std::cout << "  " << m_programName << " --info setsysvolume       Show command details\n";
    std::cout << "\n";
    std::cout << "APP GROUP EXAMPLES:\n";
    std::cout << "  " << m_programName << " --create-group Coding\n";
    std::cout << "  " << m_programName << " --add-app Coding \"VS Code\" process Code.exe\n";
    std::cout << "  " << m_programName << " --add-app Coding CLion process clion64.exe\n";
    std::cout << "  " << m_programName << " --run-group Coding min    Minimize all coding apps\n";
    std::cout << "  " << m_programName << " --run-group Coding freeze Hide and suspend all\n";
    std::cout << "\n";
    std::cout << "For more information about NirCmd, visit:\n";
    std::cout << "  https://www.nirsoft.net/utils/nircmd.html\n";
    std::cout << "\n";
}

void CliParser::PrintVersion() const {
    std::cout << "NirUI version 1.0.0\n";
    std::cout << "NirCmd wrapper with GUI and CLI interface\n";
    std::cout << "\n";
    std::cout << "NirCmd version: 2.87\n";
    std::cout << "Website: https://www.nirsoft.net/utils/nircmd.html\n";
}

void CliParser::PrintCategories() const {
    const auto& categories = NirCmdCommands::GetCategories();
    
    std::cout << "\n";
    std::cout << "Available Command Categories:\n";
    std::cout << "============================\n\n";
    
    for (const auto& cat : categories) {
        std::cout << "  " << cat.name;
        std::cout << " (" << cat.commands.size() << " commands)\n";
        std::cout << "     " << cat.description << "\n\n";
    }
    
    std::cout << "Use '" << m_programName << " -c \"Category Name\"' to list commands in a category.\n\n";
}

void CliParser::PrintCommands(const std::string& category) const {
    const auto& categories = NirCmdCommands::GetCategories();
    
    bool foundCategory = false;
    
    for (const auto& cat : categories) {
        if (category.empty() || cat.name == category) {
            foundCategory = true;
            
            std::cout << "\n";
            std::cout << cat.name << "\n";
            std::cout << std::string(cat.name.length(), '=') << "\n\n";
            
            for (const auto& cmd : cat.commands) {
                std::cout << "  " << std::left << std::setw(28) << cmd.name;
                std::cout << cmd.description << "\n";
            }
            std::cout << "\n";
        }
    }
    
    if (!foundCategory) {
        std::cout << "Category not found: " << category << "\n";
        std::cout << "Use '" << m_programName << " -l' to list all categories.\n";
    }
}

void CliParser::PrintCommandDetails(const std::string& commandName) const {
    const Command* cmd = NirCmdCommands::FindCommand(commandName);
    
    if (!cmd) {
        std::cout << "Command not found: " << commandName << "\n";
        std::cout << "Use '" << m_programName << " -s " << commandName << "' to search for similar commands.\n";
        return;
    }
    
    std::cout << "\n";
    std::cout << "Command: " << cmd->name << "\n";
    std::cout << std::string(cmd->name.length() + 9, '=') << "\n\n";
    
    std::cout << "Category:    " << cmd->category << "\n";
    std::cout << "Description: " << cmd->description << "\n";
    std::cout << "\n";
    
    if (!cmd->parameters.empty()) {
        std::cout << "Parameters:\n";
        for (const auto& param : cmd->parameters) {
            std::cout << "  " << std::left << std::setw(20) << param.name;
            std::cout << (param.required ? "[required] " : "[optional] ");
            std::cout << param.description;
            if (!param.defaultValue.empty()) {
                std::cout << " (default: " << param.defaultValue << ")";
            }
            std::cout << "\n";
            
            if (!param.choices.empty()) {
                std::cout << "                      Choices: ";
                for (size_t i = 0; i < param.choices.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << param.choices[i];
                }
                std::cout << "\n";
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "Example:\n";
    std::cout << "  " << cmd->example << "\n";
    std::cout << "\n";
}

void CliParser::PrintSearchResults(const std::string& query) const {
    auto results = NirCmdCommands::SearchCommands(query);
    
    std::cout << "\n";
    std::cout << "Search Results for \"" << query << "\":\n";
    std::cout << std::string(22 + query.length(), '=') << "\n\n";
    
    if (results.empty()) {
        std::cout << "No commands found matching \"" << query << "\"\n";
        return;
    }
    
    std::cout << "Found " << results.size() << " command(s):\n\n";
    
    for (const auto* cmd : results) {
        std::cout << "  " << std::left << std::setw(28) << cmd->name;
        std::cout << "[" << cmd->category << "]\n";
        std::cout << "    " << cmd->description << "\n\n";
    }
    
    std::cout << "Use '" << m_programName << " --info COMMAND' for more details.\n\n";
}

void CliParser::PrintAppGroupsHelp() const {
    std::cout << "\nApp Groups Management:\n";
    std::cout << "======================\n\n";
    std::cout << "Create and manage groups of applications, then apply window\n";
    std::cout << "commands to all apps in a group at once.\n\n";
    std::cout << "Commands:\n";
    std::cout << "  --groups                    List all groups and their apps\n";
    std::cout << "  --create-group NAME         Create a new empty group\n";
    std::cout << "  --delete-group NAME         Delete a group\n";
    std::cout << "  --add-app GROUP NAME TYPE VALUE\n";
    std::cout << "                              Add an app to a group\n";
    std::cout << "  --remove-app GROUP NAME     Remove an app from a group\n";
    std::cout << "  --run-group GROUP ACTION    Run action on all apps in group\n\n";
    std::cout << "Target Types:\n";
    std::cout << "  process  - Match by executable name (e.g., Code.exe)\n";
    std::cout << "  class    - Match by window class (e.g., Chrome_WidgetWin_1)\n";
    std::cout << "  title    - Match by exact window title\n";
    std::cout << "  ititle   - Match by partial title (case-insensitive)\n\n";
    std::cout << "Actions:\n";
    std::cout << "  min      - Minimize all windows\n";
    std::cout << "  max      - Maximize all windows\n";
    std::cout << "  normal   - Restore all windows\n";
    std::cout << "  close    - Close all windows\n";
    std::cout << "  hide     - Hide all windows\n";
    std::cout << "  show     - Show hidden windows\n";
    std::cout << "  freeze   - Hide and suspend all processes\n";
    std::cout << "  unfreeze - Resume and show all processes\n\n";
}

} // namespace NirUI
