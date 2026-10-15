#!/usr/bin/env bash
# ==============================================================================
# InstallZone Uninstallation Script
# ==============================================================================
set -e

echo "=================================================================="
echo "                  Uninstalling InstallZone                        "
echo "=================================================================="

if [ "$(id -u)" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        SUDO_CMD="sudo"
    else
        SUDO_CMD=""
    fi
else
    SUDO_CMD=""
fi

echo "Removing installed InstallZone files..."
$SUDO_CMD rm -f /usr/bin/installzone-studio
$SUDO_CMD rm -f /usr/bin/installzone-runner
$SUDO_CMD rm -f /usr/share/applications/installzone-studio.desktop
$SUDO_CMD rm -f /usr/share/icons/hicolor/scalable/apps/installzone-studio.svg
$SUDO_CMD rm -f /usr/share/pixmaps/installzone-studio.svg
$SUDO_CMD rm -rf /usr/share/installzone

# Also clean any legacy paths if present
$SUDO_CMD rm -f /usr/local/bin/installzone-studio /usr/local/bin/installzone-runner
$SUDO_CMD rm -f /usr/local/share/applications/installzone-studio.desktop
$SUDO_CMD rm -f /usr/local/share/icons/hicolor/scalable/apps/installzone-studio.svg
$SUDO_CMD rm -rf /usr/local/share/installzone
rm -f "$HOME/.local/bin/installzone-studio" "$HOME/.local/bin/installzone-runner" 2>/dev/null || true
rm -f "$HOME/.local/share/applications/installzone-studio.desktop" 2>/dev/null || true
rm -f "$HOME/.local/share/icons/hicolor/scalable/apps/installzone-studio.svg" 2>/dev/null || true
rm -rf "$HOME/.local/share/installzone" 2>/dev/null || true

if command -v update-desktop-database >/dev/null 2>&1; then
    $SUDO_CMD update-desktop-database /usr/share/applications 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    $SUDO_CMD gtk-update-icon-cache -f -t /usr/share/icons/hicolor 2>/dev/null || true
fi

echo "InstallZone has been uninstalled successfully."
echo "=================================================================="
