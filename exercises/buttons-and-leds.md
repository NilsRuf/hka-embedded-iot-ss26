# Buttons and LEDs

Well well... Now it's time to get properly started by writing your first pieces of code in this
course.
As with every new tech we learn, there is some _Hello world sample_.
In the world of embedded this corresponds to the _Blinky Sample_ that just lets an LED blink
periodically.
Since this is boring, it is already provided for you.
You can take a look at the example in [src/main.c](../src/main.c).
In there we are making use the [GPIOs](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
for toggling a _Light Emitting Diode (LED)_.

## Exercise 1: Make it interactive
The devkit has four LEDs and four buttons.
The LEDs are labeled `led0`, `led1`, and so on while the buttons are aliased as `sw0` to `sw3` while
their `DT_NODELABEL` references are `button0` to `button3`.

- Use [Zephyr's GPIO API](https://docs.zephyrproject.org/latest/doxygen/html/group__gpio__interface.html)
to turn the LEDs on when there corresponding button is pressed.
- Turn them off again when the button is released.
- Make sure to log everything that is happening in your system.
- Also, count and log the number of times each LED has been turned on.

## Exercise 2: Events
Right now, everything happens inside a main loop.
Additionally, everything blocks everything else, and we cannot react to events immediately.
Not much of an issue right now however, the frequent polling on the buttons keeps our CPU quite
occupied and prevents it from what it is doing best: sleeping.
Thus, it consumes a lot of energy.

Therefore:
- Use the GPIO interface again and activate **interrupts on both edges** for the buttons. Register callbacks accordingly.
- Now use the interrupts to toggle the LEDs.
- Change your software, so that one short press activates the LED, and the next press deactivates it
  again.
    - Do you notice anything? If yes, how can you fix it?

## Exercise 3: Adding some timers
Next up, we want our LEDs to blink with different intervals.
- On a button press, let the corresponding LED blink with its interval.
    The [Timer API](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html)
    might come in handy.
- Needless to say: the next button press should deactivate it.
- Make the timing intervals configurable using `Kconfig`.


## Tired of LEDs and Buttons?
I can understand. However, you have already learned quite a few nice concepts of embedded firmware:
    - GPIOs for input and output
    - Interrupts for catching events and reducing CPU load
    - Timers for doing stuff periodically

This is actually enough to interface with some simple peripherals!
