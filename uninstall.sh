#!/usr/bin/env bash
# ==============================================================================
# InstallZone Uninstallation Script
# ==============================================================================
set -e

echo "=================================================================="
echo "                  Uninstalling InstallZone                        "
echo "=================================================================="

# Determine target prefix
if [ "$(id -u)" -eq 0 ]; then
    INSTALL_PREFIX="/usr/local"
else
    INSTALL_PREFIX="$HOME/.local"
fi

BIN_DIR="$INSTALL_PREFIX/bin"
APPS_DIR="$INSTALL_PREFIX/share/applications"
ICONS_DIR="$INSTALL_PREFIX/share/icons/hicolor/scalable/apps"
TEMPLATES_DIR="$INSTALL_PREFIX/share/installzone"

echo "Removing installed files from $INSTALL_PREFIX..."
rm -f "$BIN_DIR/installzone-studio"
rm -f "$BIN_DIR/installzone-runner"
rm -f "$APPS_DIR/installzone-studio.desktop"
rm -f "$ICONS_DIR/installzone-studio.svg"
rm -rf "$TEMPLATES_DIR"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$APPS_DIR" 2>/dev/null || true
fi

echo "InstallZone has been uninstalled successfully."
echo "=================================================================="
