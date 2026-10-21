#!/bin/bash
set -e

FILTER="${1:-BrainCloudS2S}"
UPROJECT="${WORKSPACE}/PluginBuild/HostProject/HostProject.uproject"

if [ ! -f "$UPROJECT" ]; then
    echo "ERROR: HostProject not found at $UPROJECT"
    echo "Run: node bccm test unreal_s2s"
    exit 1
fi

echo ""
echo "Running Unreal automation tests: $FILTER"
echo "UE:      $UE_INSTALL_PATH"
echo "Project: $UPROJECT"
echo ""

"$UE_INSTALL_PATH/Engine/Binaries/Mac/UnrealEditor-Cmd" \
    "$UPROJECT" \
    -ExecCmds="Automation RunTests $FILTER;quit" \
    -unattended \
    -NullRHI \
    -nosplash \
    -nocontroller \
    -nosound \
    -log="${WORKSPACE}/artifacts/tests.log" \
    -TestExit="Automation Test Queue Empty"
