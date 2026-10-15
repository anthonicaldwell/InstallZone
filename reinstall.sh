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

./uninstall.sh
./install.sh

echo "Reinstallation complete!"
