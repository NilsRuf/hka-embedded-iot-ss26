// Handles the environment sensor.

#include "env_reading.h"

#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(env_reading, CONFIG_LOG_DEFAULT_LEVEL);

// Prototypes for our needed functions.

// Prototype for our thread function.
static void env_reading_loop(void *arg1, void *arg2, void *arg3);

// Periodic timer for our measurements.
static void on_measurement_interval_elapsed(struct k_timer *timer);

// Define our thread
K_THREAD_DEFINE(env_reading_thread, CONFIG_ENV_READING_STACK_SIZE, env_reading_loop, NULL, NULL,
                NULL, CONFIG_ENV_READING_THREAD_PRIORITY, 0, 0);

// Timer for triggering measurements.
K_TIMER_DEFINE(measurement_timer, on_measurement_interval_elapsed, NULL);

// Define the ZBUS channel for meaasurement publication.
ZBUS_CHAN_DEFINE(env_reading_measurement_channel, struct env_reading, NULL, NULL,
                 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

// Internal trigger channel.
ZBUS_CHAN_DEFINE(env_reading_measurement_trigger, int, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
                 ZBUS_MSG_INIT(0));

// Message subscriber listening to measurement timer.
ZBUS_MSG_SUBSCRIBER_DEFINE(trigger_subscriber);
// Assign our subscriber to our trigger channel.
ZBUS_CHAN_ADD_OBS(env_reading_measurement_trigger, trigger_subscriber, 0);

// Our pressure and temperature sensor.
static const struct device *env_sensor = DEVICE_DT_GET(DT_NODELABEL(bmp280));

// Our actual thread handler.
static void env_reading_loop(__maybe_unused void *arg1, __maybe_unused void *arg2,
                             __maybe_unused void *arg3)
{
    if (!device_is_ready(env_sensor)) {
        LOG_ERR("Environment sensor is not ready - aborting thread!");
        return;
    }

    k_timer_start(&measurement_timer, K_MSEC(CONFIG_ENV_READING_DEFAULT_MEASUREMENT_INTERVAL_MS),
                  K_FOREVER);

    while (true) {
        // We do not care about the content of this field.
        int msg = 0;

        const struct zbus_channel *sending_channel;
        int ret;
        ret = zbus_sub_wait_msg(&trigger_subscriber, &sending_channel, &msg, K_FOREVER);
        if (ret != 0) {
            LOG_WRN("Failed to read channel: %d", ret);
            continue;
        }

        __ASSERT(sending_channel == &env_reading_measurement_trigger, "unexpected channel");

        struct env_reading reading = {.status = ENV_READING_INVALID};
        struct sensor_value pressure = {0};
        struct sensor_value temperature = {0};

        ret = sensor_sample_fetch(env_sensor);
        if (ret != 0) {
            LOG_ERR("Failed to request environment measurement: %d", ret);
            goto send;
        }

        ret = sensor_channel_get(env_sensor, SENSOR_CHAN_PRESS, &pressure);
        if (ret != 0) {
            LOG_ERR("Failed to read pressure measurement: %d", ret);
            goto send;
        }

        ret = sensor_channel_get(env_sensor, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
        if (ret != 0) {
            LOG_ERR("Failed to read pressure measurement: %d", ret);
            goto send;
        }

        reading.temperature_celsius = sensor_value_to_float(&temperature);
        reading.pressure_kilopascal = sensor_value_to_float(&pressure);
        reading.status = ENV_READING_VALID;
        LOG_INF("Temp: %f°C - Press: %fkPa", (double)reading.temperature_celsius,
                (double)reading.pressure_kilopascal);

    send:
        ret = zbus_chan_pub(&env_reading_measurement_channel, &reading, K_FOREVER);
        if (ret != 0) {
            LOG_ERR("Failed to publish: %d", ret);
            continue;
        }

        LOG_INF("Published %s reading", reading.status == ENV_READING_VALID ? "valid" : "invalid");
    }
}

static void on_measurement_interval_elapsed(struct k_timer *timer)
{
    int trigger = 1;
    int ret = zbus_chan_pub(&env_reading_measurement_trigger, &trigger, K_NO_WAIT);
    if (ret != 0) {
        LOG_ERR("Failed to send trigger: %d", ret);
    }

    k_timer_start(timer, K_MSEC(CONFIG_ENV_READING_DEFAULT_MEASUREMENT_INTERVAL_MS), K_FOREVER);
}
