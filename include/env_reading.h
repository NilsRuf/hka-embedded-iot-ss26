// Define the interface for the environment measurement.

#ifndef ENV_READING_H
#define ENV_READING_H

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

/// Declares a measurement valid or invalid.
enum env_reading_status {
    ENV_READING_VALID,
    ENV_READING_INVALID,
};

/// The actual reading.
struct env_reading {
    enum env_reading_status status;
    float temperature_celsius;
    float pressure_kilopascal;
};

/// Channel where environment measurements are published.
ZBUS_CHAN_DECLARE(env_reading_measurement_channel);

/// Channel where environment measurements can be triggered.
ZBUS_CHAN_DECLARE(env_reading_measurement_trigger);

#endif
