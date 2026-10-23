#!/usr/bin/env bash
# ==============================================================================
# R-Type Project - Automated Setup and Build Script
# Automatically detects OS, installs missing dependencies, bootstraps vcpkg,
# and compiles the project.
# ==============================================================================

set -e

# ANSI Color Codes
BOLD="\033[1m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RED="\033[0;31m"
NC="\033[0m"

CLEAN_BUILD=false
AUTO_YES=false
CROSS_WINDOWS=false
BUILD_DIR=""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

log_info()    { echo -e "${BLUE}${BOLD}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}${BOLD}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}${BOLD}[WARNING]${NC} $1"; }
log_error()   { echo -e "${RED}${BOLD}[ERROR]${NC} $1"; }

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -w|--windows)
            CROSS_WINDOWS=true
            shift
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -y|--yes)
            AUTO_YES=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -w, --windows          Cross-compile Windows binaries (.exe) via MinGW (on Linux)"
            echo "  -b, --build-dir <DIR>  Custom build directory (default: build or build-windows)"
            echo "  -c, --clean            Remove build directory before compiling"
            echo "  -y, --yes              Automatically install missing dependencies without prompting"
            echo "  -h, --help             Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            echo "Run '$0 --help' for usage."
            exit 1
            ;;
    esac
done

# Default build directory based on target
if [ -z "$BUILD_DIR" ]; then
    if [ "$CROSS_WINDOWS" = true ]; then
        BUILD_DIR="build-windows"
    else
        BUILD_DIR="build"
    fi
fi

# ------------------------------------------------------------------------------
# 1. OS & Package Manager Detection
# ------------------------------------------------------------------------------
OS_TYPE="$(uname -s)"
DISTRO="unknown"
PKG_MANAGER="unknown"

case "$OS_TYPE" in
    Linux*)
        if [ -f /etc/os-release ]; then
            . /etc/os-release
            DISTRO="$ID"
            case "$ID" in
                ubuntu|debian|linuxmint|pop|elementary|kali)
                    PKG_MANAGER="apt" ;;
                fedora|rhel|centos|rocky|almalinux)
                    PKG_MANAGER="dnf" ;;
                arch|manjaro|endeavouros|garuda)
                    PKG_MANAGER="pacman" ;;
                opensuse*|sles)
                    PKG_MANAGER="zypper" ;;
                *)
                    if [[ "$ID_LIKE" == *"debian"* ]] || [[ "$ID_LIKE" == *"ubuntu"* ]]; then
                        PKG_MANAGER="apt"
                    elif [[ "$ID_LIKE" == *"fedora"* ]] || [[ "$ID_LIKE" == *"rhel"* ]]; then
                        PKG_MANAGER="dnf"
                    elif [[ "$ID_LIKE" == *"arch"* ]]; then
                        PKG_MANAGER="pacman"
                    fi
                    ;;
            esac
        fi
        ;;
    Darwin*)
        DISTRO="macOS"
        if command -v brew >/dev/null 2>&1; then
            PKG_MANAGER="brew"
        fi
        ;;
    MINGW*|MSYS*|CYGWIN*)
        DISTRO="Windows"
        PKG_MANAGER="windows"
        ;;
esac

log_info "Detected OS: $OS_TYPE ($DISTRO)"

# ------------------------------------------------------------------------------
# 2. Dependency Checking & Installation
# ------------------------------------------------------------------------------
missing_pkgs=()

check_cmd() {
    command -v "$1" >/dev/null 2>&1
}

needs_deps=false
if ! check_cmd git || ! check_cmd cmake || ! check_cmd curl || ! check_cmd zip || ! check_cmd unzip || ! check_cmd tar; then
    needs_deps=true
fi

if [ "$CROSS_WINDOWS" = true ]; then
    if ! check_cmd x86_64-w64-mingw32-g++; then
        needs_deps=true
    fi
else
    if ! check_cmd g++ && ! check_cmd clang++; then
        needs_deps=true
    fi

    # Check graphical & audio dependencies on Linux native
    if [ "$OS_TYPE" = "Linux" ]; then
        if ! check_cmd pkg-config && ! check_cmd pkgconf; then
            needs_deps=true
        elif ! pkg-config --exists x11 xi xrandr xcursor libudev fontconfig gl 2>/dev/null; then
            needs_deps=true
        fi
    fi
fi

if [ "$needs_deps" = true ]; then
    log_warning "Missing build tools or system libraries."

    case "$PKG_MANAGER" in
        apt)
            missing_pkgs=("build-essential" "cmake" "ninja-build" "pkg-config" "curl" "zip" "unzip" "tar" "git")
            if [ "$CROSS_WINDOWS" = true ]; then
                missing_pkgs+=("mingw-w64")
            else
                missing_pkgs+=("libx11-dev" "libxi-dev" "libxrandr-dev" "libxcursor-dev" "libudev-dev" "libgl1-mesa-dev" "libfontconfig1-dev")
            fi
            ;;
        dnf)
            missing_pkgs=("gcc-c++" "make" "cmake" "ninja-build" "pkgconf-pkg-config" "curl" "zip" "unzip" "tar" "git")
            if [ "$CROSS_WINDOWS" = true ]; then
                missing_pkgs+=("mingw64-gcc-c++")
            else
                missing_pkgs+=("libX11-devel" "libXi-devel" "libXrandr-devel" "libXcursor-devel" "systemd-devel" "mesa-libGL-devel" "fontconfig-devel")
            fi
            ;;
        pacman)
            missing_pkgs=("base-devel" "cmake" "ninja" "pkgconf" "curl" "zip" "unzip" "tar" "git")
            if [ "$CROSS_WINDOWS" = true ]; then
                missing_pkgs+=("mingw-w64-gcc")
            else
                missing_pkgs+=("libx11" "libxi" "libxrandr" "libxcursor" "systemd" "mesa" "fontconfig")
            fi
            ;;
        zypper)
            missing_pkgs=("gcc-c++" "make" "cmake" "ninja" "pkg-config" "curl" "zip" "unzip" "tar" "git")
            if [ "$CROSS_WINDOWS" = true ]; then
                missing_pkgs+=("mingw64-cross-gcc-c++")
            else
                missing_pkgs+=("libX11-devel" "libXi-devel" "libXrandr-devel" "libXcursor-devel" "libudev-devel" "Mesa-libGL-devel" "fontconfig-devel")
            fi
            ;;
        brew)
            missing_pkgs=("cmake" "ninja" "pkg-config" "git" "curl")
            ;;
        windows)
            log_warning "On Windows, please use .\\build.ps1 in PowerShell with Visual Studio C++."
            ;;
        *)
            log_error "Unsupported package manager. Please install dependencies manually."
            exit 1
            ;;
    esac

    if [ ${#missing_pkgs[@]} -gt 0 ]; then
        if [ "$AUTO_YES" = false ]; then
            echo ""
            read -p "Install missing dependencies with $PKG_MANAGER? [Y/n] " response
            case "$response" in
                [yY][eE][sS]|[yY]|"") ;;
                *) log_warning "Skipping installation; compilation might fail.";;
            esac
        fi

        log_info "Installing dependencies..."
        SUDO_CMD=""
        if [ "$EUID" -ne 0 ] && command -v sudo >/dev/null 2>&1; then
            SUDO_CMD="sudo"
        fi

        case "$PKG_MANAGER" in
            apt)
                $SUDO_CMD apt-get update && $SUDO_CMD apt-get install -y "${missing_pkgs[@]}" ;;
            dnf)
                $SUDO_CMD dnf install -y "${missing_pkgs[@]}" ;;
            pacman)
                $SUDO_CMD pacman -Sy --noconfirm "${missing_pkgs[@]}" ;;
            zypper)
                $SUDO_CMD zypper --non-interactive install "${missing_pkgs[@]}" ;;
            brew)
                brew install "${missing_pkgs[@]}" ;;
        esac
        log_success "Dependencies installed."
    fi
else
    log_success "All required dependencies are present."
fi

# ------------------------------------------------------------------------------
# 3. Vcpkg Setup
# ------------------------------------------------------------------------------
if [ ! -f "vcpkg/.vcpkg-root" ] || [ ! -f "vcpkg/bootstrap-vcpkg.sh" ]; then
    log_info "Initializing vcpkg submodule..."
    git submodule update --init --recursive
fi

if [ ! -f "vcpkg/vcpkg" ]; then
    log_info "Bootstrapping vcpkg..."
    ./vcpkg/bootstrap-vcpkg.sh -disableMetrics
    log_success "vcpkg ready."
fi

# ------------------------------------------------------------------------------
# 4. Build
# ------------------------------------------------------------------------------
if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    log_info "Cleaning build directory ($BUILD_DIR)..."
    rm -rf "$BUILD_DIR"
fi

CMAKE_GEN=""
if command -v ninja >/dev/null 2>&1; then
    CMAKE_GEN="-G Ninja"
fi

TOOLCHAIN_FILE="$SCRIPT_DIR/vcpkg/scripts/buildsystems/vcpkg.cmake"

if [ "$CROSS_WINDOWS" = true ]; then
    log_info "Configuring and compiling R-Type for Windows (MinGW cross-compilation)..."
    cmake -S . -B "$BUILD_DIR" \
        $CMAKE_GEN \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DVCPKG_TARGET_TRIPLET="x64-mingw-dynamic" \
        -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$SCRIPT_DIR/mingw-toolchain.cmake"
else
    log_info "Configuring and compiling R-Type (Native)..."
    cmake -S . -B "$BUILD_DIR" \
        $CMAKE_GEN \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"
fi

cmake --build "$BUILD_DIR" --parallel

echo ""
log_success "Build complete!"
echo "Artifacts located in: $BUILD_DIR/"

if [ "$CROSS_WINDOWS" = true ]; then
    echo "  - Server: $BUILD_DIR/r-type_server.exe"
    echo "  - Client: $BUILD_DIR/r-type_client.exe"
    echo ""
    echo "Run (on Windows):"
    echo "  $BUILD_DIR\\r-type_server.exe 4242"
    echo "  $BUILD_DIR\\r-type_client.exe 127.0.0.1 4242"
else
    echo "  - Server: $BUILD_DIR/r-type_server"
    echo "  - Client: $BUILD_DIR/r-type_client"
    echo ""
    echo "Run:"
    echo "  ./$BUILD_DIR/r-type_server 4242"
    echo "  ./$BUILD_DIR/r-type_client 127.0.0.1 4242"
fi
