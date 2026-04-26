# The Zephyr Cheat Sheet

The following APIs are crucial to writing applications in Zephyr.
There are always links to documentation, APIs and sometimes examples.
You can always visit the [official Zephyr documentation](https://docs.zephyrproject.org/latest/index.html)
for further information.

# Zephyr Source Tree
This is a short explanation of the Zephyr source tree and where to find things.
Our board definition (`nrf52840dk/nrf52840`) is located under `zephyr/boards/nordic/nrf52840dk` by
the way.

- `zephyr`
    - `arch`: Common architecture code for like ARM Cortex M33.
    - `boards`: Board definitions.
    - `cmake`: Build scripts.
    - `doc`: Documentation, contains all the links down below as well.
    - `drivers`: Contains source code for drivers. E.g. BME280 in `drivers/sensors/bosch/bme280`.
    - `dts`: Contains all YAML bindings for devicetrees.
    - `include`: Include files for all the modules.
    - `kernel`: The kernel and all its primitives, schedulers, etc.
    - `lib`: Libraries like POSIX, OpenMP, hash map implementations, etc.
    - `misc`: Nothing, really...
    - `modules`: Some external modules like NRF BLE radio stuff.
    - `samples`: **Very useful** samples to every subsystem and driver you can think of.
    - `scripts`: Lots of build stuff.
    - `share`: Ignore
    - `snippets`: Ignore
    - `soc`: SOC definitions, e.g. for `nordic/nrf52`
    - `submanifests`: Ignore
    - `subsys`: Subsystems like IP or BLE stacks, file systems, logging, etc.
    - `tests`: Zephyr tests.

# Kernel Core APIs

Zephyr has lots of useful kernel APIs you can access by including `zephyr/kernel.h`

### Measuring Time

```c
// Sleep for a specified number of milliseconds.
k_msleep(250);

// Sleep for some time period.
k_sleep(K_SECONDS(2)); // Also, K_MSEC, K_USEC, and K_NSEC.

// Busy-wait (do not sleep) a number of microseconds.
// Useful for short delays in drivers between IO operations.
k_busy_wait(100);
```

Time can also be measured:
```c
// Get the uptime cycles (granularity as coarse as systick on some implementations!).
// Beware of the overflow!
const uint32_t uptime_cycles = k_cycle_get_32();

// Similar, get uptime ticks (increments in system time (e.g. for timers).
const int64_t uptime_ticks = k_uptime_ticks();

// For measuring time differences
const int64_t delta_us = k_cyc_to_us_near32(cycles);
const int64_t delta_us = k_ticks_to_us_near32(ticks);
```

## Timers
Timers are crucial tool to trigger actions after a specified amount of time.

- Explanation: [Timers](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html)
- API: [Timer API](https://docs.zephyrproject.org/latest/doxygen/html/group__timer__apis.html)

```c
void on_timer_expired(struct k_timer *timer);
K_TIMER_DEFINE(my_timer, on_timer_expired, NULL); // Optional stop callback can be provided.

// Start my_timer with an initial delay of 250ms followed by periodic invocations every 500ms.
// Use K_NO_WAIT as second argument for one shot timers.
// K_NO_WAIT as first argument will immediately fire the timer.
k_timer_start(&my_timer, K_MSEC(250), K_MSEC(500));

// Stop the timer.
k_timer_stop(&my_timer);
```

## System Workqueue
Timers run in an ISR context, so blocking tasks are not allowed.
To move blocking tasks out of ISRs, there is a _workqueue thread_ to which items can be sent.

- Explanation: [Workqueues](https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html)
- API: [Workqueue API](https://docs.zephyrproject.org/latest/doxygen/html/group__workqueue__apis.html)

Basic usage:
```c
struct my_work {
    struct k_work work; // Add work item into your context struct.
    struct k_work_delayable dwork; // Optionally, a work item can be scheduled - you only need one.

    int my_context; // Whatever you need.
};

// Work handler
void work_handler(struct k_work *work)
{
    // You get a pointer to the work item inside your struct my_work item.
    // Now, get back your original struct:
    struct my_work *actual = CONTAINER_OF(work, struct my_work, work);

    // Do your stuff here...
}

// For delayable work handlers it is similar
void dwork_handler(struct k_delayable_work *dwork)
{
    // You get a pointer to the work item inside your struct my_work item.
    // Now, get back your original struct:
    struct my_work *actual = CONTAINER_OF(dwork, struct my_work, dwork);

    // Do your stuff here...
}

// Now, call it:
struct my_work item;
k_work_init(&item.work, work_handler);
int ret = k_work_submit(&item.work);
// Or
k_work_delayable_init(&item.dwork, dwork_handler);
int ret = k_work_schedule(&item.dwork, K_SECONDS(2));
```

## Synchronization
These do not need much explanation:

```c
// Semaphores
K_SEM_DEFINE(my_sem, 0, 1); // Semaphore with initial and max value.
int err = k_sem_take(&my_sem, K_FOREVER);
k_sem_give(&my_sem);

// Mutexes are recursive, they can be taken multiple times inside the same thread.
K_MUTEX_DEFINE(my_mutex);
int err = k_mutex_lock(&my_mutex, K_FOREVER);
k_mutex_unlock(&my_utex);
```

There is also the [Polling API](https://docs.zephyrproject.org/latest/kernel/services/polling.html)
for those interested but we do not really need it in our case.

## Threads
Threads allow us to separate different execution flows from each other, so each component can focus
on its task.

- Explanation: [Threads](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html)
- API: [Thread API](https://docs.zephyrproject.org/latest/doxygen/html/group__thread__apis.html)
- Examples: [Dining Philosophers](https://docs.zephyrproject.org/latest/samples/philosophers/README.html#dining-philosophers)

> **Note:** Enabling `CONFIG_THREAD_NAME=y` in `prj.conf` can help with error messages on crashes!

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

# Logging and printing
This chapter is a short one:

- Explanation: [Logging](https://docs.zephyrproject.org/latest/services/logging/index.html)
- API: [API](https://docs.zephyrproject.org/latest/doxygen/html/group__log__api.html)
- Settings in `prj.conf`: `CONFIG_LOG=y`, `CONFIG_LOG_BACKEND_UART=y`

> **Note:** To get logs immediately when an event occurs, use `CONFIG_LOG_MODE_IMMEDIATE=y`

```c
#include <zephyr/logging/log.h>

// Register a log module.
LOG_MODULE_REGISTER(my_mod, CONFIG_LOG_DEFAULT_LEVEL);

// In another file if you want the same log module to be used:
LOG_MODULE_DECLARE(my_mod);

// In code:
LOG_INF("INFO %d", 42);
LOG_WRN("WARNING");
LOG_ERR("ERROR");
LOG_DBG("Only printed when CONFIG_LOG_DEFAULT_LEVEL=3");


// You can also use printk:
printk("Beware the newline!\n");
```

# Devices, Sensors, and co.

## Device handles
Devices defined inside the devicetree can be retrieved in the code as follows:

```c
#include <zephyr/device.h>

// Say you have
// devicetree_node: devicetree_node {/* some node */};
const struct device *some_device = DEVICE_DT_GET(DT_NODELABEL(devicetree_node));

// Check readiness
if (!device_is_ready(some_device)) {/* Error handling */}
```

Make sure to always use the correct API with the correct device handle!
Otherwise, it gets messy...

## GPIOs
GPIOs are general purpose digital pins that can be either input or output.

- Explanation: [GPIOs](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
- API: [GPIO API](https://docs.zephyrproject.org/latest/doxygen/html/group__gpio__interface.html)

```c
#include <zephyr/drivers/gpio.h>

// These come from the devicetree
#define LED0_NODE DT_ALIAS(led0)

// Get the LED node.
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Check readiness
if (!gpio_is_ready_dt(&led)) {/* Error handling */}

// Configure as output.
int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
```

For interrupt configuration, see [this sample](https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/drivers/gpio/button_interrupt/src/main.c).

## Sensors
For sensors, always use the older `sensor_sample_fetch` and `sensor_channel_get` APIs!

- Explanation: [Sensors](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/index.html)
- API and Samples: [Fetch and Get](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/fetch_and_get.html)

```c
// Also contains the definitions of all the available channels...
#include <zephyr/drivers/sensor.h>

const struct device *sensor = DEVICE_DT_GET(DT_NODELABEL(my_sensor));

int err = sensor_sample_fetch(sensor);

// value.val1 contains the integer part of the sensor value.
// value.val2 contains the decimal part, scaled by 1e6.
struct sensor_value value;
int err = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &value);
// Convert it to float maybe?
const float fval = sensor_value_to_float(&value);
```

# ZBUS Messaging system
ZBUS is a generic pub-sub message bus which can be used to decouple and synchronize different
modules.

- Explanation: [ZBUS](https://docs.zephyrproject.org/latest/services/zbus/index.html)
- API: [ZBUS API](https://docs.zephyrproject.org/latest/doxygen/html/group__zbus__apis.html)
- Settings in `prj.conf`: `CONFIG_ZBUS=y`, `CONFIG_ZBUS_MSG_SUBSCRIBER=y`

```c
#include <zephyr/zbus/zbus.h>

// Message structure.
struct message {
    int val;
    float content;
};

// Define a channel (in ONE source file where it is owned)
ZBUS_CHAN_DEFINE(my_chan, struct message, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

// Expose your channel in an appropriate header:
ZBUS_CHAN_DECLARE(my_chan);

// Define a message subscriber (called in different context).
ZBUS_MSG_SUBSCRIBER_DEFINE(my_sub);

// Define a listener (called in publishing context).
static void listen(const struct zbus_channel *chan)
{
    // Get message.
    struct message msg = zbus_chan_const_msg(chan);
    // Do sth.
}

ZBUS_LISTENER_DEFINE(my_listener, listen);

// Add observers:
// You can add an observer to multiple channels.
ZBUS_CHAN_ADD_OBS(my_chan, my_listener, 0); // last one is prio.
ZBUS_CHAN_ADD_OBS(my_chan, my_sub, 1);

// Listen for a message
struct message msg;
const struct zbus_channel *source_chan;
int err = zbus_sub_wait_msg(&my_sub, &source_chan, &msg, K_FOREVER);
```

