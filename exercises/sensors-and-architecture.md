# More Sensors and Some Architecture

Time to use some actual RTOS features like multithreading and message passing interfaces!

## Exercise 1: Threading!
Zephyr has got [Threads](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html).
The documentation is quite exhaustive but there is not much you need to know right now.

Basically, all you need is this:
```c
#define MY_STACK_SIZE 500
#define MY_PRIORITY 5

static void my_entry_point(void *, void *, void *);

K_THREAD_DEFINE(my_tid, MY_STACK_SIZE,
                my_entry_point, NULL, NULL, NULL,
                MY_PRIORITY, 0, 0);

// Somewhere later in the code...
static void my_entry_point(void *v1, void *v2, void *v3)
{
    for(;;) {
        // Do thread stuff...
    }
}
```

The thread will be created at startup.
Zephyr schedules by priority.
A lower priority value means a higher system priority.
Zephyr does not guarantee progress for all threads by default, so be careful here.
Threads are scheduled whenever the current thread waits or a higher priority thread becomes
available.
Threads can wait on locks, message queues, etc., or just yield using `k_yield()`.

- Create a sensor measurement thread (in another source file so it does not get messy).
- Configure an initial measurement interval via Kconfig.
- Also make the thread priority and stack size configurable.
- For now, do not perform any measurements just yet. Just keep printing logs from inside that
  thread.

## Exercise 2: Some real sensors
Plug the _BOSCH BMP280_ sensor with the **VDD** pin connected to **P0.02** into the header.

- Uncomment the devicetree overlay entry for the sensor.
- Add the following `prj.conf` settings:
    - `CONFIG_SENSOR=y`
    - `CONFIG_BOSCH_BME280=y`
    - `CONFIG_I2C=y`
- Add three GPIOs:
    - A VDD pin on **P0.02** which has to be high.
    - A CDB pin on pin **P1.13** which is also high.
    - An SDO pin on pin **P1.12** which has to be low (I2C address)
- Periodically read the sensor values using the [Sensor API](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/index.html)
- Log everything of course.

**Hint:** You do not have to configure those static GPIO pins in your C code. There is a convenient
devicetree feature called _GPIO hogs_:
```
&gpio0 {
    status = "okay";

    bosch_vdd_pin {
        gpio-hog;
        output-high;
        gpios = <2 GPIO_ACTIVE_HIGH>;
    };
};
```

## Exercise 3 (ADVANCED): Message bus
Read about [Zephyr's message bus (ZBUS)](https://docs.zephyrproject.org/latest/services/zbus/index.html).
Then implement a measurement channel that provides you with the latest measurements.

> We will also do this in class ;)

1. Add a trigger channel, so you can request a sample on button press and when a timer expires
2. Add a measurement channel where you can send the updated values.

## Relevant functions and APIs:
- Define channel `ZBUS_CHAN_DEFINE(name, type, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));`
- Declare channel `ZBUS_CHAN_DECLARE(name);`
- Define MSG subscriber: `ZBUS_MSG_SUBSCRIBER_DEFINE(name)`
- Attach subscriber to channel: `ZBUS_CHAN_ADD_OBS(channel, subscriber, prio)`
- Publish message: `int zbus_chan_pub(&chan, &msg, K_NO_WAIT);`
- Read message from channel: `int zbus_sub_wait_msg(&sub, &chan_ptr, &msg, K_FOREVER);`
