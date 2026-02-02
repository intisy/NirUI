#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace NirUI {

// Parameter types for NirCmd commands
enum class ParamType {
    None,
    String,
    Integer,
    FilePath,
    FolderPath,
    Choice,
    Boolean,
    KeyCombo,
    Color,
    Rectangle
};

// Parameter definition
struct Parameter {
    std::string name;
    std::string description;
    ParamType type;
    bool required;
    std::string defaultValue;
    std::vector<std::string> choices; // For Choice type
    
    Parameter(const std::string& n, const std::string& desc, ParamType t, 
              bool req = true, const std::string& def = "", 
              const std::vector<std::string>& ch = {})
        : name(n), description(desc), type(t), required(req), 
          defaultValue(def), choices(ch) {}
};

// Command definition
struct Command {
    std::string name;
    std::string description;
    std::string example;
    std::vector<Parameter> parameters;
    std::string category;
    
    Command() = default;
    Command(const std::string& n, const std::string& desc, const std::string& ex,
            const std::vector<Parameter>& params, const std::string& cat)
        : name(n), description(desc), example(ex), parameters(params), category(cat) {}
};

// Category definition
struct Category {
    std::string name;
    std::string icon; // Unicode icon
    std::string description;
    std::vector<Command> commands;
};

// All NirCmd commands organized by category
class NirCmdCommands {
public:
    static const std::vector<Category>& GetCategories();
    static const Command* FindCommand(const std::string& name);
    static std::vector<const Command*> SearchCommands(const std::string& query);
    
private:
    static void InitializeCommands();
    static std::vector<Category> s_categories;
    static bool s_initialized;
    
    // Category initialization functions
    static void InitVolumeCommands();
    static void InitMonitorCommands();
    static void InitSystemCommands();
    static void InitWindowCommands();
    static void InitProcessCommands();
    static void InitClipboardCommands();
    static void InitCDROMCommands();
    static void InitDisplayCommands();
    static void InitFileCommands();
    static void InitRegistryCommands();
    static void InitShortcutCommands();
    static void InitNetworkCommands();
    static void InitServiceCommands();
    static void InitSpeechCommands();
    static void InitScreenshotCommands();
    static void InitInputCommands();
    static void InitDialogCommands();
    static void InitMiscCommands();
    static void InitNirUICustomCommands();
};

} // namespace NirUI
