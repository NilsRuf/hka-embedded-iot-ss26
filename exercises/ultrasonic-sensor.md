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
    ultrasonic_signal: ultrasonic_signal {
        gpios = <&gpio1 13 GPIO_ACTIVE_HIGH>;
    };
};
```
    - Send a trigger of at least `10us` on the pin.
    - Reconfigure the pin as input and register interrupts on it.
    - Measure the pulse response using `k_cycle_get64` from the [Kernel Timing API](https://docs.zephyrproject.org/latest/kernel/services/timing/clocks.html)
    - Convert that measurement into a distance
    - Log that to the console
    - Trigger a measurement when the button is pressed.

## Exercise 3 (ADVANCED): Implement your own sensor driver
Take a look at a driver, e.g. the one for a _BOSCH BME280_ under `zephyr/drivers/sensor/bosch/bme280/bme280.c`.
Try to implement the ultrasonic sensor as a device driver yourself.

This is kind of advanced, though. Feel free to contact me for advice.

Basically, what you need to do according to the [Zephyr docs](https://docs.zephyrproject.org/latest/kernel/drivers/index.html):
1. Define a `grove,ultrasonic.yaml` file under an app-local `dts/bindings` folder.
2. Add `grove Grove` as an entry to `dts/vendors.txt`.
3. Specify the trigger pin in the DTS file (take a look at the Zephyr repo how they do it).
4. Add a custom `ultrasonic.c` file that implements the [Sensor API](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/index.html) (only fetch and channel retrieval).
5. Use the Zephyr macros to define a device driver.

