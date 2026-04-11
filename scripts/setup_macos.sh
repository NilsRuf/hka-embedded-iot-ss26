#!/bin/env bash

# This setup script installs all required dependencies.
# It can be used under Linux (Debian-based systems) and WSL under Windows.
# Make sure you run this script with your Pyhton virtualenv activated!

set -e

# Install build tools (CMake, Ninja, etc.)
echo "Installing build tools..."
brew install cmake ninja gperf python3 python-tk ccache qemu dtc libmagic wget openocd minicom

# Get the tools for flashing. This is mostly proprietary stuff but it is considerably easier than
# tools like OpenOCD - especially with Nordic's toolchain.
echo "Installing Segger JLink for device flashing..."
wget --post-data="accept_license_agreement=accepted"  \
    "https://www.segger.com/downloads/jlink/JLink_MacOSX_V934b_universal.pkg" -O jlink.pkg && \
    sudo installer -pkg ./jlink.pkg -target / &&  \
    rm jlink.pkg

echo "Installing nrfutil..."
wget "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables/universal-apple-darwin/nrfutil"
    -O nrfutil && \
    chmod +x nrfutil && \
    sudo mv nrfutil "/usr/local/bin"
nrfutil self-upgrade
# The completion is just for autocompletion, device is for interacting with the devkits themselves.
nrfutil install completion device

# Call west update
echo "Downloading Zephyr and installing dependencies..."
west update
west zephyr-export
west packages pip --install
# We need to install the NRF-specific Python dependencies separately...
pip install -r ../nrf/scripts/requirements.txt

# Install the west SDK (aka compilers and stuff)
# We only need the ARM toolchain for the NRF chip.
# Installing all toolchains for all platforms would require gigabytes of download and disk space...
echo "Installing Zephyr SDK"
west sdk install -t arm-zephyr-eabi

# And that's it. We have everything we need now!
echo "All set, your're good to go!"

