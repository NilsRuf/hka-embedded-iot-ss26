# Development Workflow

To get your samples up and running quickly, you might find this document helpful.

## Building, flashing, debugging
Building and flashing a Zephyr application is quite easy.
Just make sure, your device is plugged in.

> **Note:** If you are on Windows, make sure your device is properly attached via USBIPD.

```bash

# Complete rebuild or initial build of the software - make sure you are in the app directory!
west build -p -b nrf52840dk/nrf52840

# Incremental build
west build

# Flashing (also calls build on code changes)
west flash

# Sometimes needed (definitely the first time you flash a new NRF chip)
west flash --recover
```

Debugging is easy as well:
```bash
west debug
```

There are also some nice VS Code plugins btw.

Additionally, it is often easier to debug using logs - especially when concurrency or external
hardware comes into play.
Therefore, we have `minicom`:

```bash
minicom -b 115200 -con -D /dev/ttyACM0
```

The device name `/dev/ttyACM0` may differ on your system.

This opens a serial connection to the chip which can be used to transfer logs or even shell
commands.

## Troubleshooting
- Try a complete rebuild!
- Check the board connectivity
- Sometimes it is necessary to execute `west flash --recover` for flashing
- Ask me, the internet or an LLM of your choice

## CMake, Kconfig, Devicetree, etc.
Here is a quick overview of the build process.
If you are interested you can read (a lot) more:
    - [Build system - CMake](https://docs.zephyrproject.org/latest/build/cmake/index.html)
    - [Devicetree](https://docs.zephyrproject.org/latest/build/dts/index.html)
    - [Configuration system - Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html)

For the impatient, the TL;DR version:

Zephyr uses a combination of three tools - namely [CMake](https://cmake.org/documentation/),
[Kconfig](https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html), and
[Devicetree](https://www.devicetree.org/) for abstracting and decoupling software and hardware layers.
This makes it easy to develop for different targets or run firmware in a simulation environment.

##### Devicetree
Devicetree files (`.dts`, `.dtsi`, and `.overlay` files) describe the **target hardware** and all
its peripherals in a hierarchical tree structure which is turned into a series of macro definitions
at build time.
In your code you can reference the nodes and their properties and use them to toggle GPIOs, or write
device drivers.
Take a look at [our devicetree overlay](../boards/nrf52840dk_nrf52840.overlay) for details.

The overlay files are used to modify a board definition to your needs.
This is useful when you want to connect peripherals to a development kit.
You will see examples in the course.

##### Kconfig
Next, there is Kconfig. What devicetree is for the hardware, Kconfig is for **software**.
It is really just a fancy way of defining compile time constants that may even have dependencies.
They are always located in files named `Kconfig`.
The `Kconfig` files only define available configuration options and assign default values.
For project-specific configuration the [prj.conf](../prj.conf) file is used.

##### CMake
CMake is the glue that merges all the devicetree and Kconfig configurations with your source code to
produce the final executable.
You do not need to know much of it except that you have to include your source files in our toplevel
[CMakeLists.txt](../CMakeLists.txt) file.
