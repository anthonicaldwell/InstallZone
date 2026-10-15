#!/usr/bin/env bash
# ==============================================================================
# InstallZone Reinstallation Script
# ==============================================================================
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$SCRIPT_DIR"

echo "=================================================================="
echo "                 Reinstalling InstallZone                         "
echo "=================================================================="

# Pre-authenticate sudo once so uninstall.sh and install.sh run seamlessly
if [ "$(id -u)" -ne 0 ]; then
    if [ ! -t 0 ] && [ -n "$DISPLAY$WAYLAND_DISPLAY" ]; then
        PASS=""
        if command -v kdialog >/dev/null 2>&1; then
            PASS=$(kdialog --password "InstallZone requires administrator privileges to reinstall:")
        elif command -v zenity >/dev/null 2>&1; then
            PASS=$(zenity --password --title="InstallZone Authentication")
        fi
        if [ -n "$PASS" ]; then
            echo "$PASS" | sudo -S -v 2>/dev/null
        fi
    elif command -v sudo >/dev/null 2>&1; then
        sudo -v
    fi
fi

./uninstall.sh
./install.sh

echo "Reinstallation complete!"
