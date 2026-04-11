#!/bin/env bash

# This setup script installs all required dependencies.
# It can be used under Linux (Debian-based systems) and WSL under Windows.
# Make sure you run this script with your Pyhton virtualenv activated!

set -e

# Install build tools (CMake, Ninja, etc.)
echo "Installing build tools..."
sudo apt install --no-install-recommends -y git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget python3-dev python3-venv python3-tk \
  xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 minicom

# Get the tools for flashing. This is mostly proprietary stuff but it is considerably easier than
# tools like OpenOCD - especially with Nordic's toolchain.
echo "Installing Segger JLink for device flashing..."
wget --post-data="accept_license_agreement=accepted"  \
    "https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb" -O jlink.deb && \
    sudo dpkg -i jlink.deb &&  \
    rm jlink.deb

# Next, we need to set some udev rules so that the Nordic devkits get recognized properly.
echo "Installing udev rules for Nordic chips..."
wget "https://github.com/NordicSemiconductor/nrf-udev/releases/download/v1.0.1/nrf-udev_1.0.1-all.deb"  \
    -O udev.deb && \
    sudo dpkg -i udev.deb && \
    rm udev.deb

echo "Installing nrfutil..."
wget "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables/x86_64-unknown-linux-gnu/nrfutil" \
    -O nrfutil && \
    chmod +x nrfutil && \
    mv nrfutil "$HOME/.local/bin"
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

