# Final Project

The goal of the final project is to create a sensor device that sends out measurement values via BLE and can be connected via a client like your phone.
It is built based on what we have covered in the practice sessions. This code can be reused.

You can use the contents of branch `template` as a start.

**Mandatory requirements** are - well - mandatory. It is a lot about basic code quality and hygiene
as well as some core features. Implement those first.

Then, choose **at least two** optional feature to implement.

If you have questions concerning the implementation, write an email to `runi0001@h-ka.de`.

Submit your code before **Sunday, 31.05.2026 23:59** as a zipped archive.
Your archive **must also include the `.git` folder** of your repository.

# Mandatory requirements (50P)

## Code quality (5P)
- The code has to compile without warnings from **your** code (Zephyr warnings not originating from your code don't matter).
- Every return value needs to be checked for errors or casted to void with a comment explaining why this is okay.
- Every function in a header file must contain descriptions for each parameter and the return value.
- General coding best practices (split your code into functions, sensible variable naming, a few comments, etc.) should be applied.

## Architecture (10P)
- Focus on a clean design with your code structured in modules (different C files) and different threads (where it makes sense).
- Design a proper way of communication between the threads (e.g. message bus or something else)

## Logging (2P)
- All system events (measurements, config changes, sending data, etc. must be logged to the console).
- All errors have to be logged at least once (either with a warning or error log - depending on the severity).

## Measurements (10P)
- The device has two sensor sources: our ultrasonic sensor and the Bosch BMP280 environmental temperature and
  pressure sensor.
- The environment sensor has an initial measurement interval of 10 seconds. This value must be configurable via a Kconfig option.
- The environment sensor has a _fast mode_ which triggers a measurement every 5 seconds. It is activated by pressing `button0` and deactivated by pressing `button0` again.
- If fast mode is active, let LED0 blink with a rate of 2Hz (once per second). In normal mode
  the blink interval of LED0 should be 1Hz.
- The ultrasonic sensor has a default measurement interval of 2s. Make this configurable via Kconfig.
- Let LED1 blink for 500ms after each measurement (either environmental or ultrasonic sensor).

## Bluetooth functionality (10P)
- Make your device connectable.
- Show the connection status via LED2: off -> disconnected, on -> connected
- Define a service comprising **three read-only characteristics** for distance (ultrasonic),
  temperature, and pressure, as well as **three read-write characteristics** for setting the
  ultrasonic measurement interval, as well as the standard and fast modes for the environment
  sensor.
- Each value is encoded in 16 bits. Use a signed value for temperature and an unsigned value for
  pressure, and distance, respectively.
    - Formats are as follows:
        - Distance in millimeters (`uint16_t`)
        - Temperature in celsius divided by 100 as `int16_t`, i.e. 23.58°C -> 2358
        - Pressure in deci-Pascal, i.e. 100.58kPa -> 10058
        - All writeable intervals are in milliseconds.

## System reset (3P)
- Pressing `button3` resets the system to its default configuration (measurement timers.)
    - Make sure the updated state is also visible via BLE!

## Documentation (10P)
- Create a markdown file under `docs` explaining your architecture (e.g. which modules exist, what they do, how they interact).
- Add a figure showing the different modules/threads and their interaction.
- Keep it short - just the essentials ;)
- Document, which optional features you implemented and explain how to use those features if you
  created your own.

# Optional Features (20P)
Choose at **least two** optional features.
You can also come up with features on your own.
Make sure to document these.
If you are not sure whether your idea counts as a feature, just ask (it usually does...).

## BLE Temperature Alarms (10P + 1P)
- Add a NOTIFY characteristic to your service which sends an alarm to the connected client if
  the ambient temperature has exceeded or fallen below a certain threshold.
- Configure the thresholds via Kconfig option.
- Use only one characteristic to represent all states (normal, freezing, or hot).
- Only drop back to normal if the temperature measurement is in range for a full minute.
- Turn LED3 on if the device is outside its specs.
- **Bonus Point:** You get an extra point if you make the limits configurable via your service.

## Presence Detection (kind of) (10P + 1P)
- Imagine the ultrasonic sensor is directed at a door.
- Detect if the door has been opened.
- For as long as the door is open, turn LED1 on (LED1 is removed of its previous functionality).
  Make sure to docuent this!
- If the door is open, switch to fast measurement mode.
- Bonus point: Add a NOTIFY characteristic to BLE indicating when the door is open.

## Pairing (10P)
- Make your BLE device pairable. Take a look at `struct bt_conn_auth_cb` in `zephyr/bluetooth/conn.h` for that.
- Use `button1` to accept a connection.

## Persistence (10P)
- Use the [Zephyr's settings](https://docs.zephyrproject.org/latest/services/settings/index.html) subsystem to persist all measurement intervals and other system configuration settings to persistent storage (flash).
- Make sure to load the settings at boot and distribute them to the right places.
- **Hint:** Take inspiration from `zephyr/samples/subsys/settings` (this even works on our
  board)

## Shell Control (10P)
- Make all your options (update intervals, etc.) configurable via the serial terminal. You can use [Zephyr's shell subsystem](https://docs.zephyrproject.org/latest/services/shell/index.html) for this or simply read the commands on your own.
- Define the commands you need (and document them + display help texts) - Zephyr's shell
  subsystem already supports all that, so you are highly encouraged to use this ;)
- Sanity checks: Always check that the fast update interval is faster than the slow interval.
- **Validate your input!**

## Get creative (5-15P)
Come up with your own ideas and implement + document them.
If you are not sure how many points you get for this, just ask.
