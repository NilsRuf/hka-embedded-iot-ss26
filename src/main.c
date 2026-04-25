
#include "env_reading.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

// Define a log module and assign it a log priority.
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

// DEVICE_DT_GET can be used to retrieve a sensor device.
// The grove ranger ultrasonic sensor has been implemented as a Zephyr sensor driver.
static const struct device *ultrasonic_sensor = DEVICE_DT_GET(DT_NODELABEL(grove_ranger));

// This semaphore is needed to signal the main thread from inside an ISR that a measurement has been
// requested. It is not possible to perform blocking operations (like our ultrasonic sensor read)
// inside an ISR.
K_SEM_DEFINE(trigger_measurment, 0, 1);

// Handles to all our LEDs.
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// Handles to all our buttons.
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);

// GPIO callback handlers for the buttons
static struct gpio_callback button0_callback;
static struct gpio_callback button1_callback;

static void on_env_measurement(const struct zbus_channel *chan);
ZBUS_LISTENER_DEFINE(env_listener, on_env_measurement);
ZBUS_CHAN_ADD_OBS(env_reading_measurement_channel, env_listener, 0);

static int configure_led(const struct gpio_dt_spec *led, const bool on)
{
    // Basically never fails. Each device has a function to check if it is ready.
    // If not, it indicates an error during initialization.
    if (!gpio_is_ready_dt(led)) {
        return -EINVAL;
    }

    // The following function is used to set a GPIO's direction to output. GPIO_OUTPUT_ACTIVE means
    // to set the pin to its ACTIVE level.
    // The ACTIVE level is defined in the devicetree as well and can be either 1 (active high)
    // or 0 (active low).
    return gpio_pin_configure_dt(led, on ? GPIO_OUTPUT_ACTIVE : GPIO_OUTPUT_INACTIVE);
}

static int configure_button(const struct gpio_dt_spec *button, struct gpio_callback *callback_data,
                            gpio_callback_handler_t handler, const bool both_edges)
{
    __ASSERT((callback_data != NULL) && (handler != NULL), "invalid arguments");

    if (!gpio_is_ready_dt(button)) {
        return -EINVAL;
    }

    int ret;

    // First, we set the direction to input.
    ret = gpio_pin_configure_dt(button, GPIO_INPUT);
    if (ret != 0) {
        return ret;
    }

    // Next, we enable interrupts on the pin.
    ret = gpio_pin_interrupt_configure_dt(button, both_edges ? GPIO_INT_EDGE_BOTH
                                                             : GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        return ret;
    }

    // Finally, we need to enable the callback.
    // Zephyr defines the actual GPIO interrupt handler and just dispatches to our callback.
    // IMPORTANT: BIT(button->pin) is used here, because Zephyr wants the GPIO bit mask instead of
    // the bit number. We could also define one handler for multiple pins here.
    gpio_init_callback(callback_data, handler, BIT(button->pin));
    return gpio_add_callback(button->port, callback_data);
}

static void on_button_pressed(__maybe_unused const struct device *port, struct gpio_callback *cb,
                              __maybe_unused gpio_port_pins_t pins)
{
    if (cb == &button0_callback) {
        // Here, we notify a waiter on the semaphore that we request a sensor sample.
        // We also signal this through LED0 which has no blink timer any more.
        k_sem_give(&trigger_measurment);
        (void)gpio_pin_toggle_dt(&led0);
    }

    if (cb == &button1_callback) {
        // Here, we notify a waiter on the semaphore that we request a sensor sample.
        // We also signal this through LED0 which has no blink timer any more.
        int msg = 1;
        int err = zbus_chan_pub(&env_reading_measurement_trigger, &msg, K_NO_WAIT);
        if (err != 0) {
            LOG_ERR("Failed to publish measurement trigger: %d", err);
        }
    }
}

int main(void)
{
    int ret;

    ret = configure_led(&led0, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure led0: %d", ret);
        return 0;
    }

    ret = configure_button(&button0, &button0_callback, on_button_pressed, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure button0: %d", ret);
        return 0;
    }

    ret = configure_button(&button1, &button1_callback, on_button_pressed, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure button1: %d", ret);
        return 0;
    }

    if (!device_is_ready(ultrasonic_sensor)) {
        LOG_ERR("Environment sensor is not ready!");
        return 0;
    }

    LOG_INF("Sensors are ready :)");

    struct sensor_value distance;
    for (;;) {
        // Here, we wait until button0 has been pressed and we can perform a measurement.
        ret = k_sem_take(&trigger_measurment, K_FOREVER);
        if (ret != 0) {
            continue;
        }

        // This triggers a measurement of the ultrasonic sensor.
        ret = sensor_sample_fetch(ultrasonic_sensor);
        if (ret != 0) {
            LOG_ERR("Failed to request distance sample: %d", ret);
            continue;
        }

        // Fetching a measurement and reading its results are independent operations.
        // Here, we fetch the SENSOR_CHAN_DISTANCE sensor value - the only one we define.
        ret = sensor_channel_get(ultrasonic_sensor, SENSOR_CHAN_DISTANCE, &distance);
        if (ret != 0) {
            LOG_ERR("Failed to read distance sample: %d", ret);
            continue;
        }

        // Disable LED0 and print the sensor value.
        // Sensor values are represented as two integers.
        // val1 is the integer and val2 is the fractional part scaled by 1e6.
        // Our unit here is meters.
        (void)gpio_pin_toggle_dt(&led0);
        LOG_INF("Read distance: %d.%dm", distance.val1, distance.val2);
    }

    return 0;
}

static void on_env_measurement(const struct zbus_channel *chan)
{
    const struct env_reading *reading = zbus_chan_const_msg(chan);
    LOG_INF("Sample is %s", reading->status == ENV_READING_VALID ? "valid" : "invalid");
}
