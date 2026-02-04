# NirUI

A modern graphical and command-line wrapper for [NirCmd](https://www.nirsoft.net/utils/nircmd.html) built with C++20 and Dear ImGui.

[![Build](https://github.com/yourusername/NirUI/actions/workflows/build.yml/badge.svg)](https://github.com/yourusername/NirUI/actions/workflows/build.yml)
![Windows](https://img.shields.io/badge/platform-Windows-blue)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Download

- **[Latest Release](https://github.com/yourusername/NirUI/releases/latest)** - Installer or portable ZIP
- **[All Releases](https://github.com/yourusername/NirUI/releases)**

## Features

### Graphical Interface
- **Command Builder** - Browse and execute 100+ NirCmd commands with an intuitive UI
- **Categorized Commands** - Commands organized by function (Volume, Monitor, System, Window Management, etc.)
- **Window Manager** - Freeze, unfreeze, hide, and manage windows with one click
- **App Groups** - Create groups of applications and apply actions to all at once
- **Command History** - Track and re-run previous commands
- **Dark/Light Themes** - Modern UI with custom title bar colors (Windows 11)
- **Auto-Download** - Automatically downloads NirCmd if not present

### Command Line Interface
```bash
# Execute any NirCmd command
NirUI_cli mutesysvolume 2

# Manage app groups
NirUI_cli --create-group "Development"
NirUI_cli --add-app "Development" "VS Code" process "Code.exe"
NirUI_cli --run-group "Development" freeze

# List available commands
NirUI_cli --list
NirUI_cli --list-category "Window Management"
```

### Window Management
- **Freeze/Unfreeze** - Hide and suspend windows, restore them later
- **Favorites** - Quick access to frequently managed windows
- **Bulk Actions** - Apply commands to multiple windows via App Groups
- **Window Picker** - Select windows from a live list
- **Folder Targeting** - Target all processes running from a specific folder (with optional subfolder recursion)

## Screenshots

*Coming soon*

## Building

### Requirements
- Windows 10/11
- CMake 3.16+
- C++20 compatible compiler (MSVC, Clang)

### Build Steps
```bash
git clone https://github.com/yourusername/NirUI.git
cd NirUI
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Dependencies are automatically fetched via CMake FetchContent:
- [Dear ImGui](https://github.com/ocornut/imgui) (docking branch)
- [nanosvg](https://github.com/memononen/nanosvg)

## Usage

### GUI Mode
Simply run `NirUI.exe` to launch the graphical interface.

### CLI Mode
```bash
# Direct command execution
NirUI_cli <nircmd_command> [arguments]

# Examples
NirUI_cli setsysvolume 32768          # Set volume to 50%
NirUI_cli monitor off                  # Turn off monitor
NirUI_cli win min class "Notepad"      # Minimize Notepad

# App Group Management
NirUI_cli --groups                     # List all groups
NirUI_cli --create-group NAME          # Create a group
NirUI_cli --delete-group NAME          # Delete a group
NirUI_cli --add-app GROUP NAME TYPE VALUE
NirUI_cli --add-app GROUP NAME folder "C:\Games" --recursive  # Target all exes in folder
NirUI_cli --remove-app GROUP NAME
NirUI_cli --run-group GROUP ACTION     # Actions: min, max, close, hide, show, freeze, unfreeze

# Target Types for App Groups
# process - Match by executable name (e.g., Code.exe)
# class   - Match by window class (e.g., Chrome_WidgetWin_1)
# title   - Match by exact window title
# ititle  - Match by partial title (case-insensitive)
# folder  - Match all executables in a folder (use --recursive for subfolders)
```

## Custom Commands

NirUI extends NirCmd with compound commands in the Window Management category:

| Command | Description |
|---------|-------------|
| `win freeze` | Hide window and suspend its process |
| `win unfreeze` | Resume process and show window |

All window commands (`win min`, `win max`, `win hide`, `win show`, `win close`, `win freeze`, `win unfreeze`) support the `folder` target type to match all executables in a directory.

## Configuration

User data is stored in `%LOCALAPPDATA%\NirUI\`:
- `app_groups.txt` - App group definitions
- `recent_values.txt` - Recently used parameter values
- `favorites.txt` - Favorite processes
- `history.txt` - Command history

## License

MIT License - See [LICENSE](LICENSE) for details.

NirCmd is freeware by NirSoft. NirUI is an independent third-party wrapper.

## Acknowledgments

- [NirSoft](https://www.nirsoft.net/) for NirCmd
- [Omar Cornut](https://github.com/ocornut) for Dear ImGui
- [Mikko Mononen](https://github.com/memononen) for nanosvg
