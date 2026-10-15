#!/usr/bin/env bash
# ==============================================================================
# InstallZone Installation Script
# Targets: Linux / Kubuntu 26.04.1 LTS with KDE Plasma
# ==============================================================================
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$SCRIPT_DIR"

echo "=================================================================="
echo "                   Installing InstallZone                         "
echo "  Modular JPL-Compliant Scriptable Installer System for Linux     "
echo "=================================================================="

# Check if cmake is installed
if ! command -v cmake >/dev/null 2>&1; then
    echo "Error: cmake is required to build InstallZone."
    exit 1
fi

# Build InstallZone
echo "[1/4] Configuring build system with CMake..."
cmake -S src -B build -DCMAKE_BUILD_TYPE=Release

echo "[2/4] Compiling InstallZone Studio and Base Runner..."
cmake --build build -j"$(nproc)"

# Determine installation destination
if [ "$(id -u)" -eq 0 ]; then
    INSTALL_PREFIX="/usr/local"
    echo "[3/4] Installing system-wide to $INSTALL_PREFIX (root mode)..."
else
    INSTALL_PREFIX="$HOME/.local"
    echo "[3/4] Installing to user directory $INSTALL_PREFIX (user mode)..."
fi

BIN_DIR="$INSTALL_PREFIX/bin"
APPS_DIR="$INSTALL_PREFIX/share/applications"
ICONS_DIR="$INSTALL_PREFIX/share/icons/hicolor/scalable/apps"
TEMPLATES_DIR="$INSTALL_PREFIX/share/installzone/templates"
ASSETS_DIR="$INSTALL_PREFIX/share/installzone/assets"

mkdir -p "$BIN_DIR" "$APPS_DIR" "$ICONS_DIR" "$TEMPLATES_DIR" "$ASSETS_DIR"

# Install binaries
install -m 0755 build/installzone-studio "$BIN_DIR/installzone-studio"
install -m 0755 build/installzone-runner "$BIN_DIR/installzone-runner"
install -m 0755 build/installzone-runner "$TEMPLATES_DIR/installzone-runner"

# Install desktop shortcut & icon
install -m 0644 assets/desktop/installzone-studio.desktop "$APPS_DIR/installzone-studio.desktop"
install -m 0644 assets/icons/installzone-studio.svg "$ICONS_DIR/installzone-studio.svg"
install -m 0644 assets/banner.jpg "$ASSETS_DIR/banner.jpg"
install -m 0644 assets/banner.png "$ASSETS_DIR/banner.png"

echo "[4/4] Updating desktop database and icon caches..."
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$APPS_DIR" 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "$INSTALL_PREFIX/share/icons/hicolor" 2>/dev/null || true
fi

echo ""
echo "=================================================================="
echo "            InstallZone Successfully Installed!                   "
echo "=================================================================="
echo "Executables installed to: $BIN_DIR"
echo "  - Studio (Creator):     $BIN_DIR/installzone-studio"
echo "  - Runner (Installer):   $BIN_DIR/installzone-runner"
echo "Desktop launcher installed to: $APPS_DIR/installzone-studio.desktop"
echo ""
echo "You can launch InstallZone Studio from your KDE Plasma Application"
echo "Launcher or by running: installzone-studio"
echo "=================================================================="
