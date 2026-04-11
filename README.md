# Embedded Firmware for the IoT - Summer Semester 2026

This repository contains the code and exercises for the HKA course _Embedded Software for the IoT_
held in the summer semester 2026.

The samples (and final submissions) will be built on the
[Nordic NRF52 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52-DK) - a development
kit by [Nordic Semiconductors](https://www.nordicsemi.com/About-us) featuring an
[NRF52840](https://www.nordicsemi.com/Products/nRF52840) multiprotocol BLE chip.

All software will be built with the [Zephyr RTOS](https://github.com/zephyrproject-rtos/zephyr), a
modern, and quite popular RTOS.


# Getting started

## Set up the `west` tool
In order to get your hands dirty, you must first setup a few things.
**Before** cloning this repo - yes, I am aware of the irony - you should first create a folder where
this repo and all Zephyr-related dependencies go (they will live side-by-side).
Then, you install Zephyr's meta-build-slash-package-manager
[West](https://docs.zephyrproject.org/latest/develop/west/index.html), and let `west` clone this
repo for you.
This will also set up your workspace the way Zephyr needs it...

> **Note:** You will need to install Python and `virtualenv` (if you don't have these already).
> Also, make sure to always activate your virtual environment when working with Zephyr.

In short:
```bash
mkdir <your-folder>
cd <your-folder>

python -m venv .env
. ./.env/bin/activate
pip install west

# This creates your application folder called 'app'.
west init -m "https://gitlab.com/hka-embedded-iot/ss26.git"
```

## Install everything
After setting up the west workspace, you now need a few build tools, as well as the code for Zephyr
and all its dependencies (Nordic stacks, chip support/hardware abstraction layers, etc.).
This repo contains two scripts: [scripts/setup\_linux.sh](./scripts/setup_linux.sh), and
[scripts/setup\_macos.sh](./scripts/setup_macos.sh).
Pick the correct one and execute it like `bash <correct_script>`.

> **The windows users:** If you are on Windows, you should use WSL for this.
> Once WSL is set up you can just use the Linux script.
> In order to interface with USB devices on WSL, you need to set up
> [USBIPD](https://learn.microsoft.com/en-us/windows/wsl/connect-usb).

Now, you are ready!

In [the hardware overview](./doc/hardware.md) you can get familiar with the hardware you will use.


# The Exercises

In this course, you will gradually develop the firmware for a sensor device with BLE
connectivity.
There are multiple exercise modules teaching you the basics of embedded software development using C
and Zephyr RTOS.

We will do some of them together, and some of them are for you to figure out.
Take a look in the [exercises](./exercises/) folder.
This contains an [index](./exercises/README.md) pointing you to all the tasks.


# Development Workflow
The [doc](./doc/) folder contains some useful commands for building, flashing, and debugging
applications which are located [here](./doc/development-workflow.md).
This document also describes how Zephyr builds and contains some useful knowledge.


# Final Project Submission

After the course, you can prove your freshly learned skills to work on a final project.
Details will be discussed in class.
