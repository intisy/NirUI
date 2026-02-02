#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace NirUI {

struct CliOptions {
    bool showHelp = false;
    bool showVersion = false;
    bool launchGui = false;
    bool listCategories = false;
    bool listCommands = false;
    bool downloadNirCmd = false;
    bool verbose = false;
    std::string category;
    std::string searchQuery;
    std::string command;
    std::vector<std::string> commandArgs;
    
    bool listAppGroups = false;
    bool createAppGroup = false;
    bool deleteAppGroup = false;
    bool addToAppGroup = false;
    bool removeFromAppGroup = false;
    bool runOnAppGroup = false;
    std::string appGroupName;
    std::string appName;
    std::string appTargetType;
    std::string appTargetValue;
    std::string appGroupAction;
};

class CliParser {
public:
    CliParser();
    
    CliOptions Parse(int argc, char* argv[]);
    
    void PrintHelp() const;
    void PrintVersion() const;
    void PrintCategories() const;
    void PrintCommands(const std::string& category = "") const;
    void PrintCommandDetails(const std::string& commandName) const;
    void PrintSearchResults(const std::string& query) const;
    void PrintAppGroupsHelp() const;
    
private:
    std::string m_programName;
};

} // namespace NirUI
