
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

// Define a log module and assign it a log priority.
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

// Here we can access our prj.conf/Kconfig definitions
#define BLINK_TIME_LED0_MS   CONFIG_LED0_BLINK_INTERVAL_MS
#define BLINK_TIME_LED1_MS   CONFIG_LED1_BLINK_INTERVAL_MS
#define BLINK_TIME_LED2_MS   CONFIG_LED2_BLINK_INTERVAL_MS
#define BLINK_TIME_LED3_MS   CONFIG_LED3_BLINK_INTERVAL_MS

// Handles to all our LEDs.
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

// Handles to all our buttons.
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);

// GPIO callback handlers for the buttons
static struct gpio_callback button0_callback;
static struct gpio_callback button1_callback;
static struct gpio_callback button2_callback;
static struct gpio_callback button3_callback;

static void on_timer_expired(struct k_timer *timer);
static void on_timer_stopped(struct k_timer *timer);
K_TIMER_DEFINE(led0_timer, on_timer_expired, on_timer_stopped);
K_TIMER_DEFINE(led1_timer, on_timer_expired, on_timer_stopped);
K_TIMER_DEFINE(led2_timer, on_timer_expired, on_timer_stopped);
K_TIMER_DEFINE(led3_timer, on_timer_expired, on_timer_stopped);

static bool led0_on = false;
static bool led1_on = false;
static bool led2_on = false;
static bool led3_on = false;

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
    ret = gpio_pin_interrupt_configure_dt(button,
            both_edges ? GPIO_INT_EDGE_BOTH : GPIO_INT_EDGE_TO_ACTIVE);
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

static void on_button_pressed(__maybe_unused const struct device *port,
                              struct gpio_callback *cb,
                              __maybe_unused gpio_port_pins_t pins)
{
    if (cb == &button0_callback) {
        if (led0_on) {
            LOG_INF("Turn LED 1 off");
            k_timer_stop(&led0_timer);
        } else {
            LOG_INF("Turn LED 1 on");
            k_timer_start(&led0_timer, K_NO_WAIT, K_MSEC(BLINK_TIME_LED0_MS));
        }

        led0_on = !led0_on;

    } else if (cb == &button1_callback) {
        if (led1_on) {
            LOG_INF("Turn LED 2 off");
            k_timer_stop(&led1_timer);
        } else {
            LOG_INF("Turn LED 2 on");
            k_timer_start(&led1_timer, K_NO_WAIT, K_MSEC(BLINK_TIME_LED1_MS));
        }
        led1_on = !led1_on;
    } else if (cb == &button2_callback) {
        if (led2_on) {
            LOG_INF("Turn LED 3 off");
            k_timer_stop(&led2_timer);
        } else {
            LOG_INF("Turn LED 3 on");
            k_timer_start(&led2_timer, K_NO_WAIT, K_MSEC(BLINK_TIME_LED2_MS));
        }
        led2_on = !led2_on;
    } else if (cb == &button3_callback) {
        if (led3_on) {
            LOG_INF("Turn LED 4 off");
            k_timer_stop(&led3_timer);
        } else {
            LOG_INF("Turn LED 4 on");
            k_timer_start(&led3_timer, K_NO_WAIT, K_MSEC(BLINK_TIME_LED3_MS));
        }
        led3_on = !led3_on;
    } else {
        LOG_ERR("Invalid gpio callback");
    }
}

static void on_timer_expired(struct k_timer *timer)
{
    if (timer == &led0_timer) {
        (void)gpio_pin_toggle_dt(&led0);
    } else if (timer == &led1_timer) {
        (void)gpio_pin_toggle_dt(&led1);
    } else if (timer == &led2_timer) {
        (void)gpio_pin_toggle_dt(&led2);
    } else if (timer == &led3_timer) {
        (void)gpio_pin_toggle_dt(&led3);
    } else {
        // Never happens
    }
}

static void on_timer_stopped(struct k_timer *timer)
{
    if (timer == &led0_timer) {
        (void)gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    } else if (timer == &led1_timer) {
        (void)gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    } else if (timer == &led2_timer) {
        (void)gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    } else if (timer == &led3_timer) {
        (void)gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    } else {
        // Never happens
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

    ret = configure_led(&led1, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure led1: %d", ret);
        return 0;
    }

    ret = configure_led(&led2, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure led2: %d", ret);
        return 0;
    }

    ret = configure_led(&led3, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure led3: %d", ret);
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

    ret = configure_button(&button2, &button2_callback, on_button_pressed, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure button2: %d", ret);
        return 0;
    }

    ret = configure_button(&button3, &button3_callback, on_button_pressed, false);
    if (ret != 0) {
        LOG_ERR("Failed to configure button3: %d", ret);
        return 0;
    }

    return 0;
}
