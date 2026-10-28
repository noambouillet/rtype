<#
.SYNOPSIS
    Automated Setup and Build Script for Windows (PowerShell)
.DESCRIPTION
    Checks tools, automatically downloads and installs missing dependencies
    (Git, CMake, Visual Studio C++ Build Tools via winget or direct download),
    initializes vcpkg, and builds R-Type in Release mode.
.EXAMPLE
    .\build.ps1
    .\build.ps1 -Clean
    .\build.ps1 -Yes
    .\build.ps1 -BuildDir "custom-build"
#>

[CmdletBinding()]
param (
    [Alias("b")]
    [string]$BuildDir = "build",

    [Alias("c")]
    [switch]$Clean,

    [Alias("y")]
    [switch]$Yes,

    [Alias("h")]
    [switch]$Help
)

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    exit 0
}

$ErrorActionPreference = "Stop"

$RepoRoot = $PSScriptRoot
Set-Location $RepoRoot

function Write-Info { param ([string]$Msg) Write-Host "[INFO] $Msg" -ForegroundColor Cyan }
function Write-Success { param ([string]$Msg) Write-Host "[SUCCESS] $Msg" -ForegroundColor Green }
function Write-Warn { param ([string]$Msg) Write-Host "[WARNING] $Msg" -ForegroundColor Yellow }
function Write-Err { param ([string]$Msg) Write-Host "[ERROR] $Msg" -ForegroundColor Red }

function Refresh-EnvPath {
    $machinePath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
    $userPath = [System.Environment]::GetEnvironmentVariable("Path", "User")
    $env:Path = "$machinePath;$userPath"
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "            R-Type Windows Setup & Build System           " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# ------------------------------------------------------------------------------
# 1. Dependency Checking & Automated Installation
# ------------------------------------------------------------------------------
Write-Info "Verifying development tools..."

$MissingTools = @()

$HasGit = [bool](Get-Command git -ErrorAction SilentlyContinue)
$HasCMake = [bool](Get-Command cmake -ErrorAction SilentlyContinue)

# Check for MSVC / Visual Studio C++ compiler
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$HasMsvc = $false
if (Test-Path $VsWhere) {
    $Installation = & $VsWhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($Installation) {
        $HasMsvc = $true
    }
}
if (-not $HasMsvc -and (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $HasMsvc = $true
}

if (-not $HasGit)   { $MissingTools += "Git" }
if (-not $HasCMake) { $MissingTools += "CMake (>= 3.21)" }
if (-not $HasMsvc)  { $MissingTools += "Visual Studio C++ Build Tools (MSVC)" }

if ($MissingTools.Count -gt 0) {
    Write-Warn "Missing required dependencies:"
    foreach ($Tool in $MissingTools) {
        Write-Host "  - $Tool" -ForegroundColor Red
    }

    $ShouldInstall = $Yes
    if (-not $ShouldInstall) {
        Write-Host ""
        $Prompt = Read-Host "Would you like to automatically download and install missing tools? [Y/n]"
        if ($Prompt -match "^[yY]" -or $Prompt -eq "") {
            $ShouldInstall = $true
        }
    }

    if ($ShouldInstall) {
        $HasWinget = [bool](Get-Command winget -ErrorAction SilentlyContinue)

        if ($HasWinget) {
            Write-Info "Using Windows Package Manager (winget) to install dependencies..."

            if (-not $HasGit) {
                Write-Info "Downloading and installing Git..."
                winget install --id Git.Git -e --silent --accept-source-agreements --accept-package-agreements
            }

            if (-not $HasCMake) {
                Write-Info "Downloading and installing CMake..."
                winget install --id Kitware.CMake -e --silent --accept-source-agreements --accept-package-agreements
            }

            if (-not $HasMsvc) {
                Write-Info "Downloading and installing Visual Studio 2022 C++ Build Tools (this may take a few minutes)..."
                winget install --id Microsoft.VisualStudio.2022.BuildTools -e `
                    --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" `
                    --accept-source-agreements --accept-package-agreements
            }
        } else {
            Write-Info "winget not detected, falling back to direct installer downloads..."
            $TempDir = "$env:TEMP\rtype_installer"
            New-Item -ItemType Directory -Force -Path $TempDir | Out-Null

            if (-not $HasGit) {
                Write-Info "Downloading Git installer..."
                $GitUrl = "https://github.com/git-for-windows/git/releases/download/v2.44.0.windows.1/Git-2.44.0-64-bit.exe"
                $GitInstaller = "$TempDir\Git-Installer.exe"
                Invoke-WebRequest -Uri $GitUrl -OutFile $GitInstaller
                Write-Info "Installing Git..."
                Start-Process -FilePath $GitInstaller -ArgumentList "/VERYSILENT", "/NORESTART" -Wait
            }

            if (-not $HasCMake) {
                Write-Info "Downloading CMake installer..."
                $CmakeUrl = "https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-windows-x86_64.msi"
                $CmakeInstaller = "$TempDir\CMake-Installer.msi"
                Invoke-WebRequest -Uri $CmakeUrl -OutFile $CmakeInstaller
                Write-Info "Installing CMake..."
                Start-Process -FilePath "msiexec.exe" -ArgumentList "/i", "`"$CmakeInstaller`"", "/quiet", "/norestart", "ADD_CMAKE_TO_PATH=System" -Wait
            }

            if (-not $HasMsvc) {
                Write-Info "Downloading Visual Studio Build Tools bootstrapper..."
                $VsUrl = "https://aka.ms/vs/17/release/vs_buildtools.exe"
                $VsInstaller = "$TempDir\vs_buildtools.exe"
                Invoke-WebRequest -Uri $VsUrl -OutFile $VsInstaller
                Write-Info "Installing Visual Studio C++ Build Tools..."
                Start-Process -FilePath $VsInstaller -ArgumentList "--passive", "--wait", "--add", "Microsoft.VisualStudio.Workload.VCTools", "--includeRecommended" -Wait
            }
        }

        # Refresh PATH environment variable in the current PowerShell process
        Refresh-EnvPath
        Write-Success "Dependencies installed and PATH refreshed."
    } else {
        Write-Err "Missing dependencies. Cannot proceed with compilation."
        exit 1
    }
} else {
    Write-Success "All required development tools are present."
}

# ------------------------------------------------------------------------------
# 2. Vcpkg Submodule & Bootstrap
# ------------------------------------------------------------------------------
if (-not (Test-Path "$RepoRoot\vcpkg\.vcpkg-root") -or -not (Test-Path "$RepoRoot\vcpkg\bootstrap-vcpkg.bat")) {
    Write-Info "Initializing vcpkg submodule..."
    git submodule update --init --recursive
}

if (-not (Test-Path "$RepoRoot\vcpkg\vcpkg.exe")) {
    Write-Info "Bootstrapping vcpkg..."
    & "$RepoRoot\vcpkg\bootstrap-vcpkg.bat" -disableMetrics
}

# ------------------------------------------------------------------------------
# 3. Clean & Build
# ------------------------------------------------------------------------------
$FullBuildDir = Join-Path $RepoRoot $BuildDir
if ($Clean -and (Test-Path $FullBuildDir)) {
    Write-Info "Cleaning build directory ($FullBuildDir)..."
    Remove-Item -Recurse -Force $FullBuildDir
}

Write-Info "Configuring and compiling R-Type with CMake..."
$ToolchainFile = "$RepoRoot/vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake -S "$RepoRoot" -B "$FullBuildDir" -DCMAKE_TOOLCHAIN_FILE="$ToolchainFile"
cmake --build "$FullBuildDir" --config Release --parallel

Write-Host ""
Write-Success "=========================================================="
Write-Success "                Build completed successfully!             "
Write-Success "=========================================================="
Write-Host ""
Write-Host "Executables located in: $BuildDir\Release\"
Write-Host "  - Server: .\\$BuildDir\Release\r-type_server.exe" -ForegroundColor Green
Write-Host "  - Client: .\\$BuildDir\Release\r-type_client.exe" -ForegroundColor Green
Write-Host ""
Write-Host "Run:" -ForegroundColor Yellow
Write-Host "  .\\$BuildDir\Release\r-type_server.exe 4242"
Write-Host "  .\\$BuildDir\Release\r-type_client.exe 127.0.0.1 4242"
