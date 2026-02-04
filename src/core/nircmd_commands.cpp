#include "nircmd_commands.h"
#include <algorithm>

namespace NirUI {

std::vector<Category> NirCmdCommands::s_categories;
bool NirCmdCommands::s_initialized = false;

const std::vector<Category>& NirCmdCommands::GetCategories() {
    if (!s_initialized) {
        InitializeCommands();
        s_initialized = true;
    }
    return s_categories;
}

const Command* NirCmdCommands::FindCommand(const std::string& name) {
    for (const auto& category : GetCategories()) {
        for (const auto& cmd : category.commands) {
            if (cmd.name == name) {
                return &cmd;
            }
        }
    }
    return nullptr;
}

std::vector<const Command*> NirCmdCommands::SearchCommands(const std::string& query) {
    std::vector<const Command*> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& category : GetCategories()) {
        for (const auto& cmd : category.commands) {
            std::string lowerName = cmd.name;
            std::string lowerDesc = cmd.description;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
            
            if (lowerName.find(lowerQuery) != std::string::npos ||
                lowerDesc.find(lowerQuery) != std::string::npos) {
                results.push_back(&cmd);
            }
        }
    }
    return results;
}

void NirCmdCommands::InitializeCommands() {
    s_categories.clear();
    
    InitVolumeCommands();
    InitMonitorCommands();
    InitSystemCommands();
    InitWindowCommands();
    InitProcessCommands();
    InitClipboardCommands();
    InitCDROMCommands();
    InitDisplayCommands();
    InitFileCommands();
    InitRegistryCommands();
    InitShortcutCommands();
    InitNetworkCommands();
    InitServiceCommands();
    InitSpeechCommands();
    InitScreenshotCommands();
    InitInputCommands();
    InitDialogCommands();
    InitMiscCommands();
    InitAppGroupCommands();
}

void NirCmdCommands::InitVolumeCommands() {
    Category cat;
    cat.name = "Volume Control";
    cat.icon = "\xF0\x9F\x94\x8A"; // Speaker icon
    cat.description = "Control system and application volume";
    
    cat.commands.push_back(Command(
        "setsysvolume",
        "Set the system volume to a specific value (0-65535)",
        "nircmd setsysvolume 32768",
        {
            Parameter("volume", "Volume level (0-65535)", ParamType::Integer, true),
            Parameter("component", "Sound component (master, waveout, synth, cd, microphone, phone, aux, line)", ParamType::Choice, false, "master",
                     {"master", "waveout", "synth", "cd", "microphone", "phone", "aux", "line"}),
            Parameter("device_index", "Sound device index", ParamType::Integer, false, "0")
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "changesysvolume",
        "Change the system volume by a relative amount",
        "nircmd changesysvolume 2000",
        {
            Parameter("change", "Volume change (-65535 to 65535)", ParamType::Integer, true),
            Parameter("component", "Sound component", ParamType::Choice, false, "master",
                     {"master", "waveout", "synth", "cd", "microphone", "phone", "aux", "line"}),
            Parameter("device_index", "Sound device index", ParamType::Integer, false, "0")
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "setsysvolume2",
        "Set the system volume using percentage (0-1000 = 0%-100%)",
        "nircmd setsysvolume2 500 master",
        {
            Parameter("volume", "Volume level (0-1000)", ParamType::Integer, true),
            Parameter("component", "Sound component", ParamType::Choice, false, "master",
                     {"master", "waveout", "synth", "cd", "microphone", "phone", "aux", "line"}),
            Parameter("device_index", "Sound device index", ParamType::Integer, false, "0")
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "changesysvolume2",
        "Change the system volume by percentage",
        "nircmd changesysvolume2 50",
        {
            Parameter("change", "Volume change percentage", ParamType::Integer, true),
            Parameter("component", "Sound component", ParamType::Choice, false, "master",
                     {"master", "waveout", "synth", "cd", "microphone", "phone", "aux", "line"}),
            Parameter("device_index", "Sound device index", ParamType::Integer, false, "0")
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "mutesysvolume",
        "Mute, unmute, or toggle the system volume",
        "nircmd mutesysvolume 2",
        {
            Parameter("mute", "0=unmute, 1=mute, 2=toggle", ParamType::Choice, true, "2",
                     {"0", "1", "2"}),
            Parameter("component", "Sound component", ParamType::Choice, false, "master",
                     {"master", "waveout", "synth", "cd", "microphone", "phone", "aux", "line", "default_record"}),
            Parameter("device_index", "Sound device index", ParamType::Integer, false, "0")
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "setappvolume",
        "Set application volume (Windows 7/8/10/11)",
        "nircmd setappvolume firefox.exe 0.5",
        {
            Parameter("process", "Process name or 'focused'", ParamType::String, true),
            Parameter("volume", "Volume level (0.0-1.0)", ParamType::String, true)
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "changeappvolume",
        "Change application volume relatively",
        "nircmd changeappvolume chrome.exe 0.1",
        {
            Parameter("process", "Process name or 'focused'", ParamType::String, true),
            Parameter("change", "Volume change (-1.0 to 1.0)", ParamType::String, true)
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "muteappvolume",
        "Mute/unmute application volume",
        "nircmd muteappvolume spotify.exe 2",
        {
            Parameter("process", "Process name or 'focused'", ParamType::String, true),
            Parameter("mute", "0=unmute, 1=mute, 2=toggle", ParamType::Choice, true, "2",
                     {"0", "1", "2"})
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "setdefaultsounddevice",
        "Set the default sound device (Windows 7+)",
        "nircmd setdefaultsounddevice \"Speakers\"",
        {
            Parameter("device_name", "Name of sound device", ParamType::String, true),
            Parameter("role", "0=console, 1=multimedia, 2=communications", ParamType::Choice, false, "1",
                     {"0", "1", "2"})
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "setsubunitvolumedb",
        "Set volume of sound device subunits in dB",
        "nircmd setsubunitvolumedb \"Microphone\" -10",
        {
            Parameter("subunit_name", "Name of subunit", ParamType::String, true),
            Parameter("volume_db", "Volume in decibels", ParamType::Integer, true)
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "mutesubunitvolume",
        "Mute/unmute sound device subunits",
        "nircmd mutesubunitvolume \"Line In\" 1",
        {
            Parameter("subunit_name", "Name of subunit", ParamType::String, true),
            Parameter("mute", "0=unmute, 1=mute, 2=toggle", ParamType::Choice, true, "2",
                     {"0", "1", "2"})
        },
        "Volume Control"
    ));
    
    cat.commands.push_back(Command(
        "showsounddevices",
        "Show all sound devices in a message box",
        "nircmd showsounddevices",
        {},
        "Volume Control"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitMonitorCommands() {
    Category cat;
    cat.name = "Monitor Control";
    cat.icon = "\xF0\x9F\x96\xA5"; // Monitor icon
    cat.description = "Control monitor power and screen settings";
    
    cat.commands.push_back(Command(
        "monitor",
        "Turn monitor on, off, or set low power mode",
        "nircmd monitor off",
        {
            Parameter("action", "on, off, low, async_off, async_on, async_low", ParamType::Choice, true, "off",
                     {"on", "off", "low", "async_off", "async_on", "async_low"})
        },
        "Monitor Control"
    ));
    
    cat.commands.push_back(Command(
        "screensaver",
        "Start the default screen saver",
        "nircmd screensaver",
        {},
        "Monitor Control"
    ));
    
    cat.commands.push_back(Command(
        "screensavertimeout",
        "Set screen saver timeout in seconds",
        "nircmd screensavertimeout 300",
        {
            Parameter("seconds", "Timeout in seconds", ParamType::Integer, true)
        },
        "Monitor Control"
    ));
    
    cat.commands.push_back(Command(
        "setbrightness",
        "Set screen brightness (laptops)",
        "nircmd setbrightness 50",
        {
            Parameter("brightness", "Brightness level (0-100)", ParamType::Integer, true)
        },
        "Monitor Control"
    ));
    
    cat.commands.push_back(Command(
        "changebrightness",
        "Change screen brightness relatively",
        "nircmd changebrightness 10",
        {
            Parameter("change", "Brightness change (-100 to 100)", ParamType::Integer, true)
        },
        "Monitor Control"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitSystemCommands() {
    Category cat;
    cat.name = "System Control";
    cat.icon = "\xE2\x9A\x99"; // Gear icon
    cat.description = "System power and state control";
    
    cat.commands.push_back(Command(
        "exitwin",
        "Exit Windows (shutdown, restart, logoff, etc.)",
        "nircmd exitwin poweroff",
        {
            Parameter("action", "poweroff, reboot, logoff, standby, hibernate, lock", ParamType::Choice, true, "poweroff",
                     {"poweroff", "reboot", "logoff", "standby", "hibernate", "lock", "shutdown"}),
            Parameter("force", "Force close applications", ParamType::Boolean, false, "false")
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "standby",
        "Put computer in standby mode",
        "nircmd standby",
        {
            Parameter("force", "Force standby (0 or 1)", ParamType::Boolean, false, "false")
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "hibernate",
        "Put computer in hibernate mode",
        "nircmd hibernate",
        {
            Parameter("force", "Force hibernate (0 or 1)", ParamType::Boolean, false, "false")
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "lockws",
        "Lock the workstation",
        "nircmd lockws",
        {},
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "emptybin",
        "Empty the Recycle Bin",
        "nircmd emptybin",
        {
            Parameter("drive", "Drive letter (optional, all drives if empty)", ParamType::String, false)
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "sysrefresh",
        "Refresh system settings after Registry changes",
        "nircmd sysrefresh environment",
        {
            Parameter("what", "environment, policy, intl, all", ParamType::Choice, true, "all",
                     {"environment", "policy", "intl", "all"})
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "shellrefresh",
        "Refresh shell icons and associations",
        "nircmd shellrefresh",
        {},
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "restartexplorer",
        "Restart Windows Explorer gracefully",
        "nircmd restartexplorer",
        {},
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "elevatecmd",
        "Run a NirCmd command with admin rights",
        "nircmd elevatecmd exec calc.exe",
        {
            Parameter("command", "NirCmd command to elevate", ParamType::String, true)
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "elevate",
        "Run an external program with admin rights",
        "nircmd elevate notepad.exe",
        {
            Parameter("program", "Program to run elevated", ParamType::FilePath, true),
            Parameter("parameters", "Command line parameters", ParamType::String, false)
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "runassystem",
        "Run program as SYSTEM user (Windows 7+)",
        "nircmd runassystem regedit.exe",
        {
            Parameter("program", "Program to run", ParamType::FilePath, true),
            Parameter("parameters", "Command line parameters", ParamType::String, false)
        },
        "System Control"
    ));
    
    cat.commands.push_back(Command(
        "runas",
        "Run program with specified credentials",
        "nircmd runas /user:admin /password:pass cmd.exe",
        {
            Parameter("user", "Username", ParamType::String, true),
            Parameter("password", "Password", ParamType::String, true),
            Parameter("program", "Program to run", ParamType::FilePath, true),
            Parameter("parameters", "Command line parameters", ParamType::String, false)
        },
        "System Control"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitWindowCommands() {
    Category cat;
    cat.name = "Window Management";
    cat.icon = "\xF0\x9F\xAA\x9F"; // Window icon
    cat.description = "Control and manipulate windows";
    
    cat.commands.push_back(Command(
        "win close",
        "Close a window",
        "nircmd win close class \"Notepad\"",
        {
            Parameter("find_type", "class, title, ititle, process, handle, folder, active, alltop", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop", "alltopnodesktop", "foreground", "desktop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win hide",
        "Hide a window",
        "nircmd win hide class \"IEFrame\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win show",
        "Show a hidden window",
        "nircmd win show class \"IEFrame\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win min",
        "Minimize a window",
        "nircmd win min title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win max",
        "Maximize a window",
        "nircmd win max title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win normal",
        "Restore a window to normal state",
        "nircmd win normal title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win activate",
        "Activate and bring window to foreground",
        "nircmd win activate title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win focus",
        "Set keyboard focus to a window",
        "nircmd win focus title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win center",
        "Center a window on screen",
        "nircmd win center title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "alltop", "alltopnodesktop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win move",
        "Move a window to specified position",
        "nircmd win move title \"Calculator\" 100 100",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("x", "X position", ParamType::Integer, true),
            Parameter("y", "Y position", ParamType::Integer, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win setsize",
        "Set window size",
        "nircmd win setsize title \"Calculator\" 100 100 400 300",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("x", "X position", ParamType::Integer, true),
            Parameter("y", "Y position", ParamType::Integer, true),
            Parameter("width", "Window width", ParamType::Integer, true),
            Parameter("height", "Window height", ParamType::Integer, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win trans",
        "Set window transparency",
        "nircmd win trans title \"Calculator\" 200",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("transparency", "Transparency (0=invisible, 255=opaque)", ParamType::Integer, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win settopmost",
        "Set window as topmost (always on top)",
        "nircmd win settopmost title \"Calculator\" 1",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("topmost", "1=topmost, 0=normal", ParamType::Choice, true, "1",
                     {"0", "1"})
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win flash",
        "Flash a window in the taskbar",
        "nircmd win flash title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win settext",
        "Change window title text",
        "nircmd win settext title \"Calculator\" \"New Title\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("new_text", "New window title", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win redraw",
        "Force window to redraw",
        "nircmd win redraw title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active", "alltop"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win +style",
        "Add a window style",
        "nircmd win +style title \"Calculator\" 0x00C00000",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("style", "Window style hex value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win -style",
        "Remove a window style",
        "nircmd win -style title \"Calculator\" 0x00C00000",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("style", "Window style hex value to remove", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win +exstyle",
        "Add an extended window style",
        "nircmd win +exstyle title \"my computer\" 0x00400000",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("exstyle", "Extended style hex value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win -exstyle",
        "Remove an extended window style",
        "nircmd win -exstyle title \"Calculator\" 0x00400000",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("exstyle", "Extended style hex value to remove", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win enable",
        "Enable a disabled window",
        "nircmd win enable title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win disable",
        "Disable a window",
        "nircmd win disable title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win togglehide",
        "Toggle window visibility",
        "nircmd win togglehide title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win togglemin",
        "Toggle window minimize state",
        "nircmd win togglemin title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win togglemax",
        "Toggle window maximize state",
        "nircmd win togglemax title \"Calculator\"",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win child",
        "Operate on child windows",
        "nircmd win child class \"Shell_TrayWnd\" hide class \"button\"",
        {
            Parameter("parent_find_type", "How to find parent window", ParamType::Choice, true, "class",
                     {"class", "title", "ititle", "process", "handle"}),
            Parameter("parent_find_value", "Parent window identifier", ParamType::String, true),
            Parameter("action", "Action to perform on child", ParamType::String, true),
            Parameter("child_find_type", "How to find child window", ParamType::Choice, true, "class",
                     {"class", "title", "all"}),
            Parameter("child_find_value", "Child window identifier", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win sendmsg",
        "Send a Windows message to a window",
        "nircmd win sendmsg title \"Calculator\" 0x0010 0 0",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("msg", "Message ID (hex)", ParamType::String, true),
            Parameter("wparam", "WPARAM value", ParamType::String, true),
            Parameter("lparam", "LPARAM value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win postmsg",
        "Post a Windows message to a window",
        "nircmd win postmsg title \"Calculator\" 0x0010 0 0",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier value", ParamType::String, true),
            Parameter("msg", "Message ID (hex)", ParamType::String, true),
            Parameter("wparam", "WPARAM value", ParamType::String, true),
            Parameter("lparam", "LPARAM value", ParamType::String, true)
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win freeze",
        "Hide window and suspend its process (NirUI compound command)",
        "win freeze process notepad.exe",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "process",
                     {"process", "class", "title", "ititle", "handle", "folder"}),
            Parameter("find_value", "Window identifier or folder path", ParamType::String, true),
            Parameter("recursive", "Include subfolders (only for folder type)", ParamType::Boolean, false, "true")
        },
        "Window Management"
    ));
    
    cat.commands.push_back(Command(
        "win unfreeze",
        "Resume process and show its window (NirUI compound command)",
        "win unfreeze process notepad.exe",
        {
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "process",
                     {"process", "class", "title", "ititle", "handle", "folder"}),
            Parameter("find_value", "Window identifier or folder path", ParamType::String, true),
            Parameter("recursive", "Include subfolders (only for folder type)", ParamType::Boolean, false, "true")
        },
        "Window Management"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitAppGroupCommands() {
    Category cat;
    cat.name = "App Groups";
    cat.icon = "\xF0\x9F\x93\x82";
    cat.description = "Manage application groups for batch window operations";
    
    cat.commands.push_back(Command(
        "group list",
        "List all app groups and their contents",
        "group list",
        {},
        "App Groups"
    ));
    
    cat.commands.push_back(Command(
        "group create",
        "Create a new app group",
        "group create \"Coding\"",
        {
            Parameter("name", "Name of the group to create", ParamType::String, true)
        },
        "App Groups"
    ));
    
    cat.commands.push_back(Command(
        "group delete",
        "Delete an app group",
        "group delete \"Coding\"",
        {
            Parameter("name", "Name of the group to delete", ParamType::String, true)
        },
        "App Groups"
    ));
    
    cat.commands.push_back(Command(
        "group add",
        "Add an application to a group",
        "group add \"Coding\" \"VS Code\" process Code.exe",
        {
            Parameter("group", "Name of the group", ParamType::String, true),
            Parameter("app_name", "Display name for the app", ParamType::String, true),
            Parameter("target_type", "How to identify the app", ParamType::Choice, true, "process",
                     {"process", "class", "title", "ititle", "folder"}),
            Parameter("target_value", "Identifier value or folder path", ParamType::String, true),
            Parameter("recursive", "Include subfolders (only for folder type)", ParamType::Boolean, false, "true")
        },
        "App Groups"
    ));
    
    cat.commands.push_back(Command(
        "group remove",
        "Remove an application from a group",
        "group remove \"Coding\" \"VS Code\"",
        {
            Parameter("group", "Name of the group", ParamType::String, true),
            Parameter("app_name", "Display name of the app to remove", ParamType::String, true)
        },
        "App Groups"
    ));
    
    cat.commands.push_back(Command(
        "group run",
        "Run a window action on all apps in a group",
        "group run \"Coding\" freeze",
        {
            Parameter("group", "Name of the group", ParamType::String, true),
            Parameter("action", "Action to perform on all apps", ParamType::Choice, true, "freeze",
                     {"min", "max", "normal", "close", "hide", "show", "freeze", "unfreeze"})
        },
        "App Groups"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitProcessCommands() {
    Category cat;
    cat.name = "Process Management";
    cat.icon = "\xF0\x9F\x94\x84"; // Cycle icon
    cat.description = "Manage running processes";
    
    cat.commands.push_back(Command(
        "killprocess",
        "Terminate a process by name",
        "nircmd killprocess notepad.exe",
        {
            Parameter("process_name", "Name of the process to kill", ParamType::String, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "closeprocess",
        "Close a process gracefully by name",
        "nircmd closeprocess notepad.exe",
        {
            Parameter("process_name", "Name of the process to close", ParamType::String, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "suspendprocess",
        "Suspend a running process",
        "nircmd suspendprocess notepad.exe",
        {
            Parameter("process_name", "Name of the process to suspend", ParamType::String, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "resumeprocess",
        "Resume a suspended process",
        "nircmd resumeprocess notepad.exe",
        {
            Parameter("process_name", "Name of the process to resume", ParamType::String, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "setprocesspriority",
        "Set process priority",
        "nircmd setprocesspriority notepad.exe high",
        {
            Parameter("process_name", "Name of the process", ParamType::String, true),
            Parameter("priority", "Priority level", ParamType::Choice, true, "normal",
                     {"idle", "belownormal", "normal", "abovenormal", "high", "realtime"})
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "setprocessaffinity",
        "Set process CPU affinity",
        "nircmd setprocessaffinity notepad.exe 1",
        {
            Parameter("process_name", "Name of the process", ParamType::String, true),
            Parameter("affinity_mask", "CPU affinity bitmask", ParamType::String, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "waitprocess",
        "Wait for a process to close, then run command",
        "nircmd waitprocess notepad.exe speak text \"Notepad closed\"",
        {
            Parameter("process_name", "Name of the process to wait for", ParamType::String, true),
            Parameter("command", "Command to run after process closes (optional)", ParamType::String, false)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "exec",
        "Execute a program",
        "nircmd exec show notepad.exe",
        {
            Parameter("show_state", "show, hide, min, max", ParamType::Choice, true, "show",
                     {"show", "hide", "min", "max"}),
            Parameter("program", "Program to execute", ParamType::FilePath, true),
            Parameter("parameters", "Command line parameters", ParamType::String, false)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "exec2",
        "Execute a program with working directory",
        "nircmd exec2 show \"C:\\Windows\" notepad.exe",
        {
            Parameter("show_state", "show, hide, min, max", ParamType::Choice, true, "show",
                     {"show", "hide", "min", "max"}),
            Parameter("working_dir", "Working directory", ParamType::FolderPath, true),
            Parameter("program", "Program to execute", ParamType::FilePath, true),
            Parameter("parameters", "Command line parameters", ParamType::String, false)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "execmd",
        "Execute a shell command",
        "nircmd execmd copy file1.txt file2.txt",
        {
            Parameter("command", "Shell command to execute", ParamType::String, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "memdump",
        "Dump process memory to file",
        "nircmd memdump notepad.exe c:\\temp\\dump.bin",
        {
            Parameter("process_name", "Name of the process", ParamType::String, true),
            Parameter("output_file", "Output file path", ParamType::FilePath, true)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "runinteractive",
        "Run program interactively from a service",
        "nircmd runinteractive notepad.exe",
        {
            Parameter("program", "Program to run", ParamType::FilePath, true),
            Parameter("parameters", "Command line parameters", ParamType::String, false)
        },
        "Process Management"
    ));
    
    cat.commands.push_back(Command(
        "runinteractivecmd",
        "Run NirCmd command interactively from a service",
        "nircmd runinteractivecmd savescreenshot c:\\temp\\shot.png",
        {
            Parameter("command", "NirCmd command to run", ParamType::String, true)
        },
        "Process Management"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitClipboardCommands() {
    Category cat;
    cat.name = "Clipboard";
    cat.icon = "\xF0\x9F\x93\x8B"; // Clipboard icon
    cat.description = "Clipboard operations";
    
    cat.commands.push_back(Command(
        "clipboard set",
        "Set clipboard text",
        "nircmd clipboard set \"Hello World\"",
        {
            Parameter("text", "Text to copy to clipboard", ParamType::String, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard clear",
        "Clear the clipboard",
        "nircmd clipboard clear",
        {},
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard readfile",
        "Copy file contents to clipboard",
        "nircmd clipboard readfile c:\\temp\\text.txt",
        {
            Parameter("filepath", "Path to text file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard writefile",
        "Write clipboard contents to file",
        "nircmd clipboard writefile c:\\temp\\output.txt",
        {
            Parameter("filepath", "Path to output file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard writeufile",
        "Write clipboard to file (Unicode)",
        "nircmd clipboard writeufile c:\\temp\\output.txt",
        {
            Parameter("filepath", "Path to output file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard addfile",
        "Append clipboard contents to file",
        "nircmd clipboard addfile c:\\temp\\output.txt",
        {
            Parameter("filepath", "Path to output file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard addufile",
        "Append clipboard to file (Unicode)",
        "nircmd clipboard addufile c:\\temp\\output.txt",
        {
            Parameter("filepath", "Path to output file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard copyimage",
        "Copy image file to clipboard",
        "nircmd clipboard copyimage c:\\temp\\image.png",
        {
            Parameter("filepath", "Path to image file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard saveimage",
        "Save clipboard image to file",
        "nircmd clipboard saveimage c:\\temp\\image.png",
        {
            Parameter("filepath", "Path to output image file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard loadclp",
        "Load clipboard from .clp file",
        "nircmd clipboard loadclp c:\\temp\\clip.clp",
        {
            Parameter("filepath", "Path to .clp file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    cat.commands.push_back(Command(
        "clipboard saveclp",
        "Save clipboard to .clp file",
        "nircmd clipboard saveclp c:\\temp\\clip.clp",
        {
            Parameter("filepath", "Path to output .clp file", ParamType::FilePath, true)
        },
        "Clipboard"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitCDROMCommands() {
    Category cat;
    cat.name = "CD-ROM";
    cat.icon = "\xF0\x9F\x92\xBF"; // CD icon
    cat.description = "CD-ROM drive control";
    
    cat.commands.push_back(Command(
        "cdrom open",
        "Open CD-ROM drive tray",
        "nircmd cdrom open d:",
        {
            Parameter("drive", "Drive letter", ParamType::String, true)
        },
        "CD-ROM"
    ));
    
    cat.commands.push_back(Command(
        "cdrom close",
        "Close CD-ROM drive tray",
        "nircmd cdrom close d:",
        {
            Parameter("drive", "Drive letter", ParamType::String, true)
        },
        "CD-ROM"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitDisplayCommands() {
    Category cat;
    cat.name = "Display Settings";
    cat.icon = "\xF0\x9F\x96\xBC"; // Display icon
    cat.description = "Change display resolution and settings";
    
    cat.commands.push_back(Command(
        "setdisplay",
        "Set display resolution and color depth",
        "nircmd setdisplay 1920 1080 32",
        {
            Parameter("width", "Screen width in pixels", ParamType::Integer, true),
            Parameter("height", "Screen height in pixels", ParamType::Integer, true),
            Parameter("color_bits", "Color depth (16, 24, 32)", ParamType::Choice, true, "32",
                     {"16", "24", "32"}),
            Parameter("refresh_rate", "Refresh rate in Hz (optional)", ParamType::Integer, false),
            Parameter("monitor", "Monitor index (optional)", ParamType::Integer, false)
        },
        "Display Settings"
    ));
    
    cat.commands.push_back(Command(
        "setprimarydisplay",
        "Set primary display monitor",
        "nircmd setprimarydisplay 2",
        {
            Parameter("monitor", "Monitor index", ParamType::Integer, true)
        },
        "Display Settings"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitFileCommands() {
    Category cat;
    cat.name = "File Operations";
    cat.icon = "\xF0\x9F\x93\x81"; // Folder icon
    cat.description = "File time and attribute operations";
    
    cat.commands.push_back(Command(
        "setfiletime",
        "Set file creation and modification time",
        "nircmd setfiletime c:\\file.txt \"01-01-2020 12:00:00\" \"01-01-2020 12:00:00\"",
        {
            Parameter("filepath", "Path to file", ParamType::FilePath, true),
            Parameter("created", "Creation time (dd-mm-yyyy hh:mm:ss or 'now')", ParamType::String, true),
            Parameter("modified", "Modification time (dd-mm-yyyy hh:mm:ss or 'now')", ParamType::String, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "setfilefoldertime",
        "Set folder creation and modification time",
        "nircmd setfilefoldertime c:\\folder \"01-01-2020 12:00:00\" \"01-01-2020 12:00:00\"",
        {
            Parameter("folderpath", "Path to folder", ParamType::FolderPath, true),
            Parameter("created", "Creation time (dd-mm-yyyy hh:mm:ss or 'now')", ParamType::String, true),
            Parameter("modified", "Modification time (dd-mm-yyyy hh:mm:ss or 'now')", ParamType::String, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "clonefiletime",
        "Clone file time from another file",
        "nircmd clonefiletime c:\\source.txt c:\\dest.txt",
        {
            Parameter("source", "Source file", ParamType::FilePath, true),
            Parameter("destination", "Destination file(s)", ParamType::FilePath, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "shellcopy",
        "Copy files using shell",
        "nircmd shellcopy c:\\source\\*.txt c:\\dest",
        {
            Parameter("source", "Source path with wildcards", ParamType::String, true),
            Parameter("destination", "Destination folder", ParamType::FolderPath, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "moverecyclebin",
        "Move file to Recycle Bin",
        "nircmd moverecyclebin c:\\temp\\file.txt",
        {
            Parameter("filepath", "Path to file", ParamType::FilePath, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "filldelete",
        "Securely delete a file (fill with zeros first)",
        "nircmd filldelete c:\\temp\\secret.txt",
        {
            Parameter("filepath", "Path to file", ParamType::FilePath, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "convertimage",
        "Convert image format",
        "nircmd convertimage c:\\image.bmp c:\\image.png",
        {
            Parameter("source", "Source image file", ParamType::FilePath, true),
            Parameter("destination", "Destination image file", ParamType::FilePath, true)
        },
        "File Operations"
    ));
    
    cat.commands.push_back(Command(
        "convertimages",
        "Convert multiple images",
        "nircmd convertimages c:\\images\\*.bmp c:\\output png",
        {
            Parameter("source", "Source path with wildcards", ParamType::String, true),
            Parameter("dest_folder", "Destination folder", ParamType::FolderPath, true),
            Parameter("format", "Output format (png, jpg, bmp, gif)", ParamType::Choice, true, "png",
                     {"png", "jpg", "bmp", "gif", "tiff"})
        },
        "File Operations"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitRegistryCommands() {
    Category cat;
    cat.name = "Registry";
    cat.icon = "\xF0\x9F\x97\x82"; // File cabinet icon
    cat.description = "Windows Registry operations";
    
    cat.commands.push_back(Command(
        "regsetval",
        "Set a Registry value",
        "nircmd regsetval sz \"HKCU\\Software\\Test\" \"MyValue\" \"Hello\"",
        {
            Parameter("type", "Value type (sz, expand_sz, dword, binary, multi_sz)", ParamType::Choice, true, "sz",
                     {"sz", "expand_sz", "dword", "binary", "multi_sz"}),
            Parameter("key", "Registry key path", ParamType::String, true),
            Parameter("value_name", "Value name", ParamType::String, true),
            Parameter("data", "Value data", ParamType::String, true)
        },
        "Registry"
    ));
    
    cat.commands.push_back(Command(
        "regdelval",
        "Delete a Registry value",
        "nircmd regdelval \"HKCU\\Software\\Test\" \"MyValue\"",
        {
            Parameter("key", "Registry key path", ParamType::String, true),
            Parameter("value_name", "Value name", ParamType::String, true)
        },
        "Registry"
    ));
    
    cat.commands.push_back(Command(
        "regdelkey",
        "Delete a Registry key",
        "nircmd regdelkey \"HKCU\\Software\\Test\"",
        {
            Parameter("key", "Registry key path", ParamType::String, true)
        },
        "Registry"
    ));
    
    cat.commands.push_back(Command(
        "regedit",
        "Open Registry Editor at specified key",
        "nircmd regedit \"HKLM\\Software\\Microsoft\"",
        {
            Parameter("key", "Registry key path to open", ParamType::String, true),
            Parameter("value", "Value name to select (optional)", ParamType::String, false)
        },
        "Registry"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitShortcutCommands() {
    Category cat;
    cat.name = "Shortcuts";
    cat.icon = "\xF0\x9F\x94\x97"; // Link icon
    cat.description = "Create shortcuts and URL links";
    
    cat.commands.push_back(Command(
        "shortcut",
        "Create a shortcut file",
        "nircmd shortcut \"C:\\Windows\\notepad.exe\" \"~$folder.desktop$\" \"Notepad\"",
        {
            Parameter("target", "Target file path", ParamType::FilePath, true),
            Parameter("folder", "Folder to create shortcut in", ParamType::FolderPath, true),
            Parameter("name", "Shortcut name (without .lnk)", ParamType::String, true),
            Parameter("arguments", "Command line arguments", ParamType::String, false),
            Parameter("icon_file", "Icon file path", ParamType::FilePath, false),
            Parameter("icon_index", "Icon index", ParamType::Integer, false),
            Parameter("show_cmd", "Show command (1=normal, 3=max, 7=min)", ParamType::Integer, false),
            Parameter("hotkey", "Hotkey", ParamType::KeyCombo, false),
            Parameter("description", "Description", ParamType::String, false)
        },
        "Shortcuts"
    ));
    
    cat.commands.push_back(Command(
        "urlshortcut",
        "Create a URL shortcut",
        "nircmd urlshortcut \"https://www.google.com\" \"~$folder.desktop$\" \"Google\"",
        {
            Parameter("url", "URL address", ParamType::String, true),
            Parameter("folder", "Folder to create shortcut in", ParamType::FolderPath, true),
            Parameter("name", "Shortcut name", ParamType::String, true)
        },
        "Shortcuts"
    ));
    
    cat.commands.push_back(Command(
        "cmdshortcut",
        "Create a shortcut that runs a NirCmd command",
        "nircmd cmdshortcut \"~$folder.desktop$\" \"Mute\" mutesysvolume 2",
        {
            Parameter("folder", "Folder to create shortcut in", ParamType::FolderPath, true),
            Parameter("name", "Shortcut name", ParamType::String, true),
            Parameter("command", "NirCmd command", ParamType::String, true)
        },
        "Shortcuts"
    ));
    
    cat.commands.push_back(Command(
        "cmdshortcutkey",
        "Create a shortcut with hotkey that runs a NirCmd command",
        "nircmd cmdshortcutkey \"~$folder.desktop$\" \"Mute\" \"ctrl+alt+m\" mutesysvolume 2",
        {
            Parameter("folder", "Folder to create shortcut in", ParamType::FolderPath, true),
            Parameter("name", "Shortcut name", ParamType::String, true),
            Parameter("hotkey", "Hotkey combination", ParamType::KeyCombo, true),
            Parameter("command", "NirCmd command", ParamType::String, true)
        },
        "Shortcuts"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitNetworkCommands() {
    Category cat;
    cat.name = "Network";
    cat.icon = "\xF0\x9F\x8C\x90"; // Globe icon
    cat.description = "Network and dial-up connections";
    
    cat.commands.push_back(Command(
        "rasdial",
        "Dial a RAS/VPN connection",
        "nircmd rasdial \"My Connection\"",
        {
            Parameter("connection_name", "Name of the connection", ParamType::String, true),
            Parameter("username", "Username (optional)", ParamType::String, false),
            Parameter("password", "Password (optional)", ParamType::String, false)
        },
        "Network"
    ));
    
    cat.commands.push_back(Command(
        "rashangup",
        "Disconnect a RAS/VPN connection",
        "nircmd rashangup \"My Connection\"",
        {
            Parameter("connection_name", "Name of connection (empty for all)", ParamType::String, false)
        },
        "Network"
    ));
    
    cat.commands.push_back(Command(
        "rasdialdlg",
        "Open dial-up connection dialog",
        "nircmd rasdialdlg \"My Connection\"",
        {
            Parameter("connection_name", "Name of the connection", ParamType::String, true)
        },
        "Network"
    ));
    
    cat.commands.push_back(Command(
        "setdialuplogon",
        "Set auto-dial on Windows logon",
        "nircmd setdialuplogon \"My Connection\" \"username\" \"password\"",
        {
            Parameter("connection_name", "Name of the connection", ParamType::String, true),
            Parameter("username", "Username", ParamType::String, true),
            Parameter("password", "Password", ParamType::String, true)
        },
        "Network"
    ));
    
    cat.commands.push_back(Command(
        "remote",
        "Execute NirCmd on a remote computer",
        "nircmd remote \\\\computer \"exec show notepad.exe\"",
        {
            Parameter("computer", "Remote computer name", ParamType::String, true),
            Parameter("command", "NirCmd command to execute", ParamType::String, true)
        },
        "Network"
    ));
    
    cat.commands.push_back(Command(
        "multiremote",
        "Execute NirCmd on multiple remote computers",
        "nircmd multiremote copy c:\\computers.txt \"exec show notepad.exe\"",
        {
            Parameter("mode", "copy or copyuserpass", ParamType::Choice, true, "copy",
                     {"copy", "copyuserpass"}),
            Parameter("computer_file", "File with computer names", ParamType::FilePath, true),
            Parameter("command", "NirCmd command to execute", ParamType::String, true)
        },
        "Network"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitServiceCommands() {
    Category cat;
    cat.name = "Services";
    cat.icon = "\xF0\x9F\x9B\xA0"; // Tools icon
    cat.description = "Windows service management";
    
    cat.commands.push_back(Command(
        "service start",
        "Start a Windows service",
        "nircmd service start \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service stop",
        "Stop a Windows service",
        "nircmd service stop \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service restart",
        "Restart a Windows service",
        "nircmd service restart \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service pause",
        "Pause a Windows service",
        "nircmd service pause \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service continue",
        "Continue a paused Windows service",
        "nircmd service continue \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service auto",
        "Set service to start automatically",
        "nircmd service auto \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service manual",
        "Set service to manual start",
        "nircmd service manual \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "service disabled",
        "Disable a Windows service",
        "nircmd service disabled \"Apache2.4\"",
        {
            Parameter("service_name", "Service name", ParamType::String, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "regsvr",
        "Register/unregister a DLL",
        "nircmd regsvr c:\\mydll.dll",
        {
            Parameter("action", "register or unregister", ParamType::Choice, true, "register",
                     {"register", "unregister"}),
            Parameter("dll_path", "Path to DLL file", ParamType::FilePath, true)
        },
        "Services"
    ));
    
    cat.commands.push_back(Command(
        "gac",
        "Install/uninstall .NET assembly to GAC",
        "nircmd gac install c:\\MyAssembly.dll",
        {
            Parameter("action", "install or uninstall", ParamType::Choice, true, "install",
                     {"install", "uninstall"}),
            Parameter("assembly_path", "Path to assembly file", ParamType::FilePath, true)
        },
        "Services"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitSpeechCommands() {
    Category cat;
    cat.name = "Text-to-Speech";
    cat.icon = "\xF0\x9F\x97\xA3"; // Speaking head icon
    cat.description = "Text-to-speech functions";
    
    cat.commands.push_back(Command(
        "speak text",
        "Speak the specified text",
        "nircmd speak text \"Hello World\"",
        {
            Parameter("text", "Text to speak", ParamType::String, true),
            Parameter("rate", "Speaking rate (-10 to 10)", ParamType::Integer, false, "0"),
            Parameter("volume", "Volume (0-100)", ParamType::Integer, false, "100")
        },
        "Text-to-Speech"
    ));
    
    cat.commands.push_back(Command(
        "speak file",
        "Speak text from a file",
        "nircmd speak file c:\\text.txt",
        {
            Parameter("filepath", "Path to text file", ParamType::FilePath, true),
            Parameter("rate", "Speaking rate (-10 to 10)", ParamType::Integer, false, "0"),
            Parameter("volume", "Volume (0-100)", ParamType::Integer, false, "100"),
            Parameter("output_file", "Output .wav file (optional)", ParamType::FilePath, false),
            Parameter("audio_format", "Audio format if saving", ParamType::String, false)
        },
        "Text-to-Speech"
    ));
    
    cat.commands.push_back(Command(
        "speak stop",
        "Stop current speech",
        "nircmd speak stop",
        {},
        "Text-to-Speech"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitScreenshotCommands() {
    Category cat;
    cat.name = "Screenshots";
    cat.icon = "\xF0\x9F\x93\xB7"; // Camera icon
    cat.description = "Screen capture operations";
    
    cat.commands.push_back(Command(
        "savescreenshot",
        "Save screenshot to file",
        "nircmd savescreenshot c:\\screenshot.png",
        {
            Parameter("filepath", "Output file path (.png, .jpg, .bmp, .gif)", ParamType::FilePath, true),
            Parameter("x", "X coordinate (optional)", ParamType::Integer, false),
            Parameter("y", "Y coordinate (optional)", ParamType::Integer, false),
            Parameter("width", "Width (optional)", ParamType::Integer, false),
            Parameter("height", "Height (optional)", ParamType::Integer, false)
        },
        "Screenshots"
    ));
    
    cat.commands.push_back(Command(
        "savescreenshotfull",
        "Save full multi-monitor screenshot",
        "nircmd savescreenshotfull c:\\screenshot.png",
        {
            Parameter("filepath", "Output file path", ParamType::FilePath, true)
        },
        "Screenshots"
    ));
    
    cat.commands.push_back(Command(
        "savescreenshotwin",
        "Save screenshot of a specific window",
        "nircmd savescreenshotwin c:\\shot.png title \"Calculator\"",
        {
            Parameter("filepath", "Output file path", ParamType::FilePath, true),
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "active", "foreground"}),
            Parameter("find_value", "Window identifier", ParamType::String, true)
        },
        "Screenshots"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitInputCommands() {
    Category cat;
    cat.name = "Input Simulation";
    cat.icon = "\xE2\x8C\xA8"; // Keyboard icon
    cat.description = "Simulate keyboard and mouse input";
    
    cat.commands.push_back(Command(
        "sendkey",
        "Send a key press",
        "nircmd sendkey ctrl+alt+del press",
        {
            Parameter("key", "Key to send (e.g., F1, enter, ctrl+c)", ParamType::KeyCombo, true),
            Parameter("action", "press, down, or up", ParamType::Choice, true, "press",
                     {"press", "down", "up"})
        },
        "Input Simulation"
    ));
    
    cat.commands.push_back(Command(
        "sendkeypress",
        "Send key combination (easier syntax)",
        "nircmd sendkeypress ctrl+shift+esc",
        {
            Parameter("keys", "Key combination (e.g., ctrl+alt+del)", ParamType::KeyCombo, true)
        },
        "Input Simulation"
    ));
    
    cat.commands.push_back(Command(
        "sendmouse",
        "Simulate mouse action",
        "nircmd sendmouse left click",
        {
            Parameter("button", "left, right, middle, x1, x2", ParamType::Choice, true, "left",
                     {"left", "right", "middle", "x1", "x2", "wheel"}),
            Parameter("action", "click, dblclick, down, up, or wheel amount", ParamType::String, true)
        },
        "Input Simulation"
    ));
    
    cat.commands.push_back(Command(
        "movecursor",
        "Move mouse cursor to position",
        "nircmd movecursor 500 300",
        {
            Parameter("x", "X coordinate", ParamType::Integer, true),
            Parameter("y", "Y coordinate", ParamType::Integer, true)
        },
        "Input Simulation"
    ));
    
    cat.commands.push_back(Command(
        "setcursor",
        "Set cursor type",
        "nircmd setcursor c:\\cursor.cur",
        {
            Parameter("cursor_file", "Path to cursor file", ParamType::FilePath, true)
        },
        "Input Simulation"
    ));
    
    cat.commands.push_back(Command(
        "setcursorwin",
        "Set cursor position in a window",
        "nircmd setcursorwin 100 100 title \"Calculator\"",
        {
            Parameter("x", "X coordinate within window", ParamType::Integer, true),
            Parameter("y", "Y coordinate within window", ParamType::Integer, true),
            Parameter("find_type", "How to find the window", ParamType::Choice, true, "title",
                     {"class", "title", "ititle", "process", "handle", "folder", "active"}),
            Parameter("find_value", "Window identifier", ParamType::String, true)
        },
        "Input Simulation"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitDialogCommands() {
    Category cat;
    cat.name = "Dialogs & Messages";
    cat.icon = "\xF0\x9F\x92\xAC"; // Speech bubble icon
    cat.description = "Display dialogs and interact with message boxes";
    
    cat.commands.push_back(Command(
        "infobox",
        "Display an information message box",
        "nircmd infobox \"Operation completed\" \"Info\"",
        {
            Parameter("message", "Message text", ParamType::String, true),
            Parameter("title", "Window title", ParamType::String, true)
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "qbox",
        "Display a Yes/No question box",
        "nircmd qbox \"Do you want to continue?\" \"Question\"",
        {
            Parameter("message", "Question text", ParamType::String, true),
            Parameter("title", "Window title", ParamType::String, true)
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "qboxcom",
        "Question box that executes command on Yes",
        "nircmd qboxcom \"Reboot now?\" \"Confirm\" exitwin reboot",
        {
            Parameter("message", "Question text", ParamType::String, true),
            Parameter("title", "Window title", ParamType::String, true),
            Parameter("command", "NirCmd command to run on Yes", ParamType::String, true)
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "qboxtop",
        "Question box (always on top)",
        "nircmd qboxtop \"Continue?\" \"Question\"",
        {
            Parameter("message", "Question text", ParamType::String, true),
            Parameter("title", "Window title", ParamType::String, true)
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "qboxcomtop",
        "Question box (always on top) with command",
        "nircmd qboxcomtop \"Shutdown?\" \"Confirm\" exitwin poweroff",
        {
            Parameter("message", "Question text", ParamType::String, true),
            Parameter("title", "Window title", ParamType::String, true),
            Parameter("command", "NirCmd command to run on Yes", ParamType::String, true)
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "trayballoon",
        "Display a tray balloon notification",
        "nircmd trayballoon \"Message text\" \"Title\" \"c:\\icon.ico\" 5000",
        {
            Parameter("message", "Balloon message", ParamType::String, true),
            Parameter("title", "Balloon title", ParamType::String, true),
            Parameter("icon", "Icon file path (or info, warning, error)", ParamType::String, true),
            Parameter("timeout", "Display timeout in milliseconds", ParamType::Integer, true)
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "dlg",
        "Click a button in a message box",
        "nircmd dlg \"\" \"\" click yes",
        {
            Parameter("window_title", "Window title (empty for any)", ParamType::String, false),
            Parameter("window_class", "Window class (empty for any)", ParamType::String, false),
            Parameter("action", "click", ParamType::Choice, true, "click", {"click"}),
            Parameter("button", "Button to click (yes, no, ok, cancel, abort, retry, ignore)", ParamType::Choice, true, "ok",
                     {"yes", "no", "ok", "cancel", "abort", "retry", "ignore"})
        },
        "Dialogs & Messages"
    ));
    
    cat.commands.push_back(Command(
        "dlgany",
        "Click a button in any dialog box",
        "nircmd dlgany \"\" \"\" click no",
        {
            Parameter("window_title", "Window title (empty for any)", ParamType::String, false),
            Parameter("window_class", "Window class (empty for any)", ParamType::String, false),
            Parameter("action", "click", ParamType::Choice, true, "click", {"click"}),
            Parameter("button", "Button to click", ParamType::Choice, true, "ok",
                     {"yes", "no", "ok", "cancel", "abort", "retry", "ignore"})
        },
        "Dialogs & Messages"
    ));
    
    s_categories.push_back(cat);
}

void NirCmdCommands::InitMiscCommands() {
    Category cat;
    cat.name = "Miscellaneous";
    cat.icon = "\xE2\x9C\xA8"; // Sparkles icon
    cat.description = "Other useful commands";
    
    cat.commands.push_back(Command(
        "beep",
        "Play a beep sound",
        "nircmd beep 750 300",
        {
            Parameter("frequency", "Frequency in Hz", ParamType::Integer, true),
            Parameter("duration", "Duration in milliseconds", ParamType::Integer, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "stdbeep",
        "Play standard Windows beep",
        "nircmd stdbeep",
        {},
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "mediaplay",
        "Play a media file for specified duration",
        "nircmd mediaplay 5000 c:\\sound.mp3",
        {
            Parameter("duration", "Play duration in milliseconds", ParamType::Integer, true),
            Parameter("filepath", "Path to media file", ParamType::FilePath, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "wait",
        "Wait for specified milliseconds",
        "nircmd wait 2000",
        {
            Parameter("milliseconds", "Time to wait", ParamType::Integer, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "cmdwait",
        "Wait then execute NirCmd command",
        "nircmd cmdwait 2000 beep 500 200",
        {
            Parameter("milliseconds", "Time to wait", ParamType::Integer, true),
            Parameter("command", "NirCmd command to execute", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "loop",
        "Execute command multiple times",
        "nircmd loop 5 1000 beep 500 200",
        {
            Parameter("count", "Number of iterations", ParamType::Integer, true),
            Parameter("wait_ms", "Wait between iterations", ParamType::Integer, true),
            Parameter("command", "NirCmd command to execute", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "script",
        "Execute NirCmd script file",
        "nircmd script c:\\script.ncl",
        {
            Parameter("filepath", "Path to script file", ParamType::FilePath, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "paramsfile",
        "Execute command with parameters from file",
        "nircmd paramsfile c:\\params.txt \"\" \"\" exec show ~$fparam.1$",
        {
            Parameter("filepath", "Path to parameters file", ParamType::FilePath, true),
            Parameter("prefix", "Prefix for each line", ParamType::String, false),
            Parameter("suffix", "Suffix for each line", ParamType::String, false),
            Parameter("command", "NirCmd command template", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "inisetval",
        "Set value in INI file",
        "nircmd inisetval c:\\config.ini \"Section\" \"Key\" \"Value\"",
        {
            Parameter("filepath", "Path to INI file", ParamType::FilePath, true),
            Parameter("section", "Section name", ParamType::String, true),
            Parameter("key", "Key name", ParamType::String, true),
            Parameter("value", "Value to set", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "inidelval",
        "Delete value from INI file",
        "nircmd inidelval c:\\config.ini \"Section\" \"Key\"",
        {
            Parameter("filepath", "Path to INI file", ParamType::FilePath, true),
            Parameter("section", "Section name", ParamType::String, true),
            Parameter("key", "Key name", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "setconsolemode",
        "Set console display mode",
        "nircmd setconsolemode 1",
        {
            Parameter("mode", "0=windowed, 1=fullscreen", ParamType::Choice, true, "0",
                     {"0", "1"})
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "setconsolecolor",
        "Set console text colors",
        "nircmd setconsolecolor 0x0A",
        {
            Parameter("color", "Color attribute (hex)", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "consolewrite",
        "Write text to console",
        "nircmd consolewrite \"Hello World\"",
        {
            Parameter("text", "Text to write", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "debugwrite",
        "Write text to debug output",
        "nircmd debugwrite \"Debug message\"",
        {
            Parameter("text", "Debug text", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "returnval",
        "Return a specific exit code",
        "nircmd returnval 123",
        {
            Parameter("value", "Exit code to return", ParamType::Integer, true)
        },
        "Miscellaneous"
    ));
    
    cat.commands.push_back(Command(
        "help",
        "Open help for a command",
        "nircmd help speak",
        {
            Parameter("command", "Command name", ParamType::String, true)
        },
        "Miscellaneous"
    ));
    
    s_categories.push_back(cat);
}

} // namespace NirUI
