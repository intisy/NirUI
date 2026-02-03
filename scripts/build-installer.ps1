# Build script for creating NirUI installer
# Requires: CMake, MSVC, Inno Setup 6

param(
    [string]$BuildType = "Release",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir

Push-Location $RootDir

try {
    # Check for Inno Setup
    $InnoPath = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
    if (-not (Test-Path $InnoPath)) {
        Write-Error "Inno Setup 6 not found. Install from https://jrsoftware.org/isinfo.php"
        exit 1
    }

    # Build the project
    if (-not $SkipBuild) {
        Write-Host "Configuring CMake..." -ForegroundColor Cyan
        cmake -B build -DCMAKE_BUILD_TYPE=$BuildType
        
        Write-Host "Building..." -ForegroundColor Cyan
        cmake --build build --config $BuildType
    }

    # Verify executables exist
    $ExePath = "build\$BuildType\NirUI.exe"
    $CliPath = "build\$BuildType\NirUI_cli.exe"
    
    if (-not (Test-Path $ExePath)) {
        Write-Error "NirUI.exe not found at $ExePath"
        exit 1
    }
    if (-not (Test-Path $CliPath)) {
        Write-Error "NirUI_cli.exe not found at $CliPath"
        exit 1
    }

    # Create dist directory
    New-Item -ItemType Directory -Force -Path dist | Out-Null

    # Create placeholder icon if missing
    if (-not (Test-Path "assets\icon.ico")) {
        Write-Warning "assets\icon.ico not found - using placeholder"
        New-Item -ItemType Directory -Force -Path assets | Out-Null
        # Minimal valid ICO
        $bytes = [byte[]](0,0,1,0,1,0,1,1,0,0,1,0,24,0,30,0,0,0,22,0,0,0,40,0,0,0,1,0,0,0,2,0,0,0,1,0,24,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
        [IO.File]::WriteAllBytes("assets\icon.ico", $bytes)
    }

    # Build installer
    Write-Host "Building installer..." -ForegroundColor Cyan
    & $InnoPath installer\NirUI.iss

    # Create portable ZIP
    Write-Host "Creating portable ZIP..." -ForegroundColor Cyan
    $Version = "1.0.0"
    Compress-Archive -Path "build\$BuildType\NirUI.exe", "build\$BuildType\NirUI_cli.exe" -DestinationPath "dist\NirUI-v$Version-portable.zip" -Force

    Write-Host "`nBuild complete!" -ForegroundColor Green
    Write-Host "Output files:"
    Get-ChildItem dist | ForEach-Object { Write-Host "  - $($_.Name)" }
}
finally {
    Pop-Location
}
