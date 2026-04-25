// Module for your environmental sensor.

#include "comm.h"
// Maybe other includes as well?

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(comm, CONFIG_LOG_DEFAULT_LEVEL);

static void comm_loop(void *a1, void *a2, void *a3);
K_THREAD_DEFINE(comm_thread, CONFIG_COMM_THREAD_STACK_SIZE, comm_loop, NULL, NULL, NULL,
                CONFIG_COMM_THREAD_PRIO, 0, 0);

// Maybe some ZBUS channels? ;)

static void comm_loop(__maybe_unused void *a1, __maybe_unused void *a2, __maybe_unused void *a3)
{
    for (;;) {
    }
}
