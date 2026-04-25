// Module for your environmental sensor.

#include "env_reading.h"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(env, CONFIG_LOG_DEFAULT_LEVEL);

static void env_loop(void *a1, void *a2, void *a3);
K_THREAD_DEFINE(env_thread, CONFIG_ENV_THREAD_STACK_SIZE, env_loop, NULL, NULL, NULL,
                CONFIG_ENV_THREAD_PRIO, 0, 0);

const struct device *env_sensor = DEVICE_DT_GET(DT_NODELABEL(bmp280));

// Maybe some ZBUS channels? ;)

static void env_loop(__maybe_unused void *a1, __maybe_unused void *a2, __maybe_unused void *a3)
{
    for (;;) {
    }
}
