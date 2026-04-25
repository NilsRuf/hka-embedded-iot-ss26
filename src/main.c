#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

// This should be moved somewhere else...
const struct device *ultrasonic_sensor = DEVICE_DT_GET(DT_NODELABEL(grove_ranger));

int main(void)
{
    // Think about what can go into main and what you might put somewhere else.
    return 0;
}
