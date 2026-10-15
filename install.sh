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

# Determine installation destination (target /usr/bin and /usr/share)
if [ "$(id -u)" -ne 0 ]; then
    if sudo -n true 2>/dev/null; then
        SUDO_CMD="sudo"
    else
        echo "[3/5] Requesting sudo elevation to install into /usr/bin and /usr/share..."
        if [ -n "$DISPLAY$WAYLAND_DISPLAY" ] && (command -v kdialog >/dev/null 2>&1 || command -v zenity >/dev/null 2>&1); then
            PASS=""
            if command -v kdialog >/dev/null 2>&1; then
                PASS=$(kdialog --password "InstallZone requires administrator privileges to install:")
            elif command -v zenity >/dev/null 2>&1; then
                PASS=$(zenity --password --title="InstallZone Authentication")
            fi
            if [ -n "$PASS" ]; then
                echo "$PASS" | sudo -S -v 2>/dev/null
            fi
        elif command -v sudo >/dev/null 2>&1; then
            sudo -v
        fi
        SUDO_CMD="sudo"
    fi
else
    SUDO_CMD=""
fi

BIN_DIR="/usr/bin"
APPS_DIR="/usr/share/applications"
ICONS_DIR="/usr/share/icons/hicolor/scalable/apps"
PIXMAPS_DIR="/usr/share/pixmaps"
TEMPLATES_DIR="/usr/share/installzone/templates"
ASSETS_DIR="/usr/share/installzone/assets"

echo "[4/5] Installing binaries to $BIN_DIR and assets to /usr/share/installzone..."
$SUDO_CMD mkdir -p "$BIN_DIR" "$APPS_DIR" "$ICONS_DIR" "$PIXMAPS_DIR" "$TEMPLATES_DIR" "$ASSETS_DIR"

# Install binaries
$SUDO_CMD install -m 0755 build/installzone-studio "$BIN_DIR/installzone-studio"
$SUDO_CMD install -m 0755 build/installzone-runner "$BIN_DIR/installzone-runner"
$SUDO_CMD install -m 0755 build/installzone-runner "$TEMPLATES_DIR/installzone-runner"

# Install desktop shortcut & icons
$SUDO_CMD install -m 0644 assets/desktop/installzone-studio.desktop "$APPS_DIR/installzone-studio.desktop"
$SUDO_CMD install -m 0644 assets/icons/installzone-studio.svg "$ICONS_DIR/installzone-studio.svg"
$SUDO_CMD install -m 0644 assets/icons/installzone-studio.svg "$PIXMAPS_DIR/installzone-studio.svg"
$SUDO_CMD install -m 0644 assets/banner.jpg "$ASSETS_DIR/banner.jpg"
$SUDO_CMD install -m 0644 assets/banner.png "$ASSETS_DIR/banner.png"
$SUDO_CMD install -m 0644 assets/banner_studio.png "$ASSETS_DIR/banner_studio.png"
$SUDO_CMD install -m 0644 assets/chiptune.wav "$ASSETS_DIR/chiptune.wav"

echo "[5/5] Updating desktop database and icon caches..."
if command -v update-desktop-database >/dev/null 2>&1; then
    $SUDO_CMD update-desktop-database "$APPS_DIR" 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    $SUDO_CMD gtk-update-icon-cache -f -t /usr/share/icons/hicolor 2>/dev/null || true
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
