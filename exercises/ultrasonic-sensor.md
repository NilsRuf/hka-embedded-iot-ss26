# The Ultrasonic Sensor

We have a [Grove Ultrasonic Ranger](https://media.digikey.com/pdf/Data%20Sheets/Seeed%20Technology/Grove_Ultrasonic_Ranger_101020010_Web.pdf)
which is a very simple ultrasonic sensor for measuring the distance to a target.

## How to connect the sensor
The sensor has four pins:
    - `GND` (BLACK): Ground
    - `VDD` (RED): Supply voltage
    - `SIG` (YELLOW): The actual signal pin
    - `NC` (WHITE): NC stands for Not Connected, so we do not need that pin

As for the connections of the pins:
    - The **black GND** cable needs to be plugged into the pin next to your micro USB cable where it
      says _External Connector_. Make sure to plug it into the **MINUS (-)** side.
    - **Red VDD** cable goes into `VDD` on the long side of the board.
    - In the middle of the board, there are six pins as two groups of three standing out, labeled
    `P1.14`, `P1.15`, `RESET`, `5V`, `P1.13`, and `DETECT`.
    - **YELLOW SIG** cable goes into `P1.13`
    - Leave the white cable hanging around.

> **Important:** Do **not** put the red cable into the `5V` connector! The NRF cannot handle that
> voltage.

It basic working principle is as follows:
    - We need to configure `SIG` as output and assert the pin for at least `10us`.
    - Then we deassert `SIG` and change its direction to input.
    - The sensor sends out a few echo pulses and determines the round-trip-time of these pulses.
    - The sensor will then drive `SIG` high for the duration of a round-trip-time.

## Exercise 1: Physics!
Physics is important in embedded development (well, a little at least).
Figure out a formula to calculate the distance from that sensor signal.

> **Hint:** The speed of sound in air is around `343m/s`.

## Exercise 2: Interfacing with the sensor
With the math out of the way, we can actually measure some distances!
    - Define the `SIG` pin inside our [devicetree overlay](../boards/nrf52840dk_nrf52840.overlay)
      like this:
```c
/ {
    custom_gpios {
        compatible = "gpio-keys";

        ultrasonic_signal: ultrasonic_signal {
            gpios = <&gpio1 13 GPIO_ACTIVE_HIGH>;
        };
    };
};
```
    - Send a trigger of at least `10us` on the pin.
    - Reconfigure the pin as input and register interrupts on it.
    - Measure the pulse response using `k_cycle_get32` from the [Kernel Timing API](https://docs.zephyrproject.org/latest/kernel/services/timing/clocks.html)
    - Convert that measurement into a distance
    - Log that to the console
    - Trigger a measurement when the button is pressed.

## Exercise 3 (ADVANCED): Implement your own sensor driver
Take a look at a driver, e.g. the one for a _BOSCH BME280_ under `zephyr/drivers/sensor/bosch/bme280/bme280.c`.
Try to implement the ultrasonic sensor as a device driver yourself.

This is kind of advanced, though. Feel free to contact me for advice.

Basically, what you need to do according to the [Zephyr docs](https://docs.zephyrproject.org/latest/kernel/drivers/index.html):
1. Define a `grove,ultrasonic.yaml` file under an app-local `dts/bindings/sensor` folder.
2. Specify the trigger pin in the DTS file (take a look at the Zephyr repo how they do it).
3. Add a custom `ultrasonic.c` file that implements the [Sensor API](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/index.html) (only fetch and channel retrieval).
4. Define the required functions for `sensor_sample_fetch`, and `sensor_channel_get`.
5. Use the Zephyr macros to define a device driver, most importantly:
```c
// This goes somewhere near the top of your file.
#define DT_DRV_COMPAT grove_ultrasonic

// ...

// This API struct will be used by Zephyr to dispatch the API calls to your sensor device.
// You need only implement those two functions.
static DEVICE_API(sensor, grove_ranger_driver_api) = {
    .sample_fetch = sample_fetch,
    .channel_get = channel_get,
};

#define GROVE_ULTRASONIC_DEFINE(inst)                                   \
    // Define your device data here                                     \
    ...                                                                 \
    SENSOR_DEVICE_DT_INST_DEFINE(inst, ultrasonic_init, NULL,           \
            &data##inst, &config##inst, POST_KERNEL,                    \
            CONFIG_SENSOR_INIT_PRIORITY, &ultrasonic_api)               \

DT_INST_FOREACH_STATUS_OKAY(GROVE_ULTRASONIC_DEFINE);
```

Also, [this](https://academy.nordicsemi.com/courses/nrf-connect-sdk-intermediate/lessons/lesson-7-device-driver-dev/topic/exercise-1-13/) is a really good guide on how to implement your custom Zephyr sensor driver using the sensor device API.
