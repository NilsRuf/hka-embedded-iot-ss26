
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

// Define a log module and assign it a log priority.
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

// Here we can access our prj.conf/Kconfig definitions
#define SLEEP_TIME_LED0_MS   CONFIG_LED0_BLINK_INTERVAL_MS

// These come from the devicetree
#define LED0_NODE DT_ALIAS(led0)

// In the next line we retrieve the GPIO node defined in the device tree.
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;
    bool led0_on = true;

    // Basically never fails. Each device has a function to check if it is ready.
    // If not, it indicates an error during initialization.
    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    // The following function is used to set a GPIO's direction to output. GPIO_OUTPUT_ACTIVE means
    // to set the pin to its ACTIVE level.
    // The ACTIVE level is defined in the devicetree as well and can be either 1 (active high)
    // or 0 (active low).
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    while (1) {
        LOG_INF("LED0 is %s", led0_on ? "on" : "off");
        led0_on = !led0_on;

        k_msleep(SLEEP_TIME_LED0_MS);

        // Here we toggle the GPIO
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            return 0;
        }
    }
    return 0;
}
