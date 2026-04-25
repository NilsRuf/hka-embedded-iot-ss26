// Driver for the Grove Ultrasonic Ranger.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sonic, CONFIG_LOG_DEFAULT_LEVEL);

#define DT_DRV_COMPAT grove_ranger

#define SPEED_OF_SOUND_AIR_UM_US ((uint64_t)343)
#define TRIGGER_PULSE_US 10
#define SENSOR_TIMEOUT_MS 200

struct ranger_config {
    struct gpio_dt_spec sig;
};

struct ranger_data {
    struct gpio_callback echo_callback;
    struct k_sem msmt_ready;
    uint32_t cycles_response_start;
    uint32_t cycles_response_end;
    uint32_t echo_toggle_count;
};

// Function prototype for GPIO interrupt.
static void on_echo_toggle(__maybe_unused const struct device *port,
                           struct gpio_callback *cb,
                           __maybe_unused gpio_port_pins_t pins);

static int sample_fetch(const struct device *ranger, __maybe_unused enum sensor_channel chan)
{
    struct ranger_data *data = ranger->data;
    const struct ranger_config *config = ranger->config;

    data->echo_toggle_count = 0;

    int err;
    err = gpio_pin_configure_dt(&config->sig, GPIO_OUTPUT_ACTIVE);
    if (err != 0) {
        LOG_ERR("Failed to configure trigger mode: %d", err);
        return err;
    }

    k_busy_wait(TRIGGER_PULSE_US);

    err = gpio_pin_configure_dt(&config->sig, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("Failed to configure trigger mode: %d", err);
        return err;
    }

    err = gpio_pin_configure_dt(&config->sig, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("Failed to activate echo pin: %d", err);
        return err;
    }

    err = gpio_pin_interrupt_configure_dt(&config->sig, GPIO_INT_EDGE_BOTH);
    if (err != 0) {
        LOG_ERR("Failed to activate echo pin interrupt: %d", err);
        return err;
    }

    err = k_sem_take(&data->msmt_ready, K_MSEC(SENSOR_TIMEOUT_MS));
    if (err != 0) {
        LOG_ERR("Failed to get echo response: %d", err);
        return err;
    }

    err = gpio_pin_interrupt_configure_dt(&config->sig, GPIO_INT_DISABLE);
    if (err != 0) {
        LOG_ERR("Failed to turn off interrupts on echo pin: %d", err);
        return err;
    }

    err = gpio_pin_configure_dt(&config->sig, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("Failed to configure pin back as output: %d", err);
        return err;
    }

    const uint64_t duration_us =
        k_cyc_to_us_near64(data->cycles_response_end - data->cycles_response_start);
    LOG_DBG("Successfully measured sample duration: %lluus!", duration_us);
    return 0;
}

static int channel_get(const struct device *ranger, enum sensor_channel chan,
                       struct sensor_value *val)
{
    struct ranger_data *data = ranger->data;

    if (chan != SENSOR_CHAN_DISTANCE) {
        return -ENOTSUP;
    }

    if (data->echo_toggle_count != 2) {
        return -EINVAL;
    }

    uint32_t duration_cycles = data->cycles_response_end - data->cycles_response_start;
    // Overflow handling.
    if (data->cycles_response_end < data->cycles_response_start) {
        duration_cycles = 0xffffffff - data->cycles_response_start + data->cycles_response_end + 1;
    }

    const uint64_t duration_us = k_cyc_to_us_near64(duration_cycles);

    const uint64_t distance_um = duration_us * SPEED_OF_SOUND_AIR_UM_US / 2;
    return sensor_value_from_micro(val, (int64_t)distance_um);
}


static void on_echo_toggle(__maybe_unused const struct device *port,
                           struct gpio_callback *cb,
                           __maybe_unused gpio_port_pins_t pins)
{
    const uint32_t current_cycles = k_cycle_get_32();

    // Now, get the data structure back...
    struct ranger_data *data = CONTAINER_OF(cb, struct ranger_data, echo_callback);
    data->echo_toggle_count++;

    if (data->echo_toggle_count == 1) {
        data->cycles_response_start = current_cycles;
    } else {
        data->cycles_response_end = current_cycles;
        k_sem_give(&data->msmt_ready);
    }
}

static int grove_ranger_init(const struct device *ranger)
{
    struct ranger_data *data = ranger->data;
    const struct ranger_config *config = ranger->config;

    k_sem_init(&data->msmt_ready, 0, 1);
    gpio_init_callback(&data->echo_callback, on_echo_toggle, BIT(config->sig.pin));

    return gpio_add_callback(config->sig.port, &data->echo_callback);
}

static DEVICE_API(sensor, grove_ranger_driver_api) = {
    .sample_fetch = sample_fetch,
    .channel_get = channel_get,
};


#define GROVE_RANGER_DEFINE(inst)                                       \
    static const struct ranger_config ranger_config##inst = {           \
        .sig = GPIO_DT_SPEC_INST_GET(inst, sig_gpios),                  \
    };                                                                  \
                                                                        \
    static struct ranger_data ranger_data##inst = {                     \
        .cycles_response_start = 0LLU,                                  \
        .cycles_response_end = 0LLU,                                    \
    };                                                                  \
                                                                        \
    SENSOR_DEVICE_DT_INST_DEFINE(inst, grove_ranger_init, NULL,         \
            &ranger_data##inst, &ranger_config##inst, POST_KERNEL,      \
            CONFIG_SENSOR_INIT_PRIORITY, &grove_ranger_driver_api)      \

DT_INST_FOREACH_STATUS_OKAY(GROVE_RANGER_DEFINE);

#undef DT_DRV_COMPAT
