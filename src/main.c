// Small BLE sample.

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// Define a log module and assign it a log priority.
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

static uint8_t manufacturer_data[] = {0xFF, 0xFF, 0x00, 0x00};

static atomic_t is_connected = ATOMIC_INIT(0);

// Advertizing data
static const struct bt_data advertizing_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, manufacturer_data, sizeof(manufacturer_data)),
};

// Only read when a device explicitly sends a SCAN request.
static const struct bt_data scan_data[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

// Every service and characteristic needs a UUID. Define your own here.

#define BT_UUID_TEMP_SERVICE_VAL                                                                   \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

#define BT_UUID_TEMP_VAL BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1)

// Then create the UUID structs for the BLE API.
static struct bt_uuid_128 bt_uuid_temp_service = BT_UUID_INIT_128(BT_UUID_TEMP_SERVICE_VAL);
static struct bt_uuid_128 bt_uuid_temp = BT_UUID_INIT_128(BT_UUID_TEMP_VAL);

static atomic_t temp_value = ATOMIC_INIT(0);

// This callback is executed when the client decides to enable/disable notifications for your
// characteristic. You need one callback per notify characteristic!
static void temp_ccc_changed(__maybe_unused const struct bt_gatt_attr *attr, uint16_t value)
{
    bool notifications_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Temperature notifications %s", notifications_enabled ? "enabled" : "disabled");
}

// Callback executed for regular read requests.
static ssize_t read_temp(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                         uint16_t len, uint16_t offset)
{
    const int16_t temp = (int16_t)atomic_get(&temp_value);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &temp, sizeof(temp));
}

// Declare your service here
// clang-format off
BT_GATT_SERVICE_DEFINE(temp_svc,
        BT_GATT_PRIMARY_SERVICE(&bt_uuid_temp_service),
        BT_GATT_CHARACTERISTIC(&bt_uuid_temp.uuid,
            BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
            BT_GATT_PERM_READ, read_temp, NULL, &temp_value),
        BT_GATT_CCC(temp_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);
// clang-format on

// Helper for sending notifications.
static void notify_temp(int16_t new_value)
{
    (void)atomic_set(&temp_value, new_value);

    // This actually sends the notification.
    // We use &temp_svc.attrs[1] to access the characteristic value we need.
    // For other notifications, the index changes accordingly.
    int err = bt_gatt_notify(NULL,               /* NULL = all subscribed clients */
                             &temp_svc.attrs[1], /* the characteristic attribute */
                             &new_value, sizeof(new_value));
    if (err == -ENOTCONN) {
        /* No client connected / notifications not enabled — not an error */
    } else if (err != 0) {
        LOG_ERR("Notify failed: %d", err);
    }
}

static const struct device *sensor = DEVICE_DT_GET(DT_NODELABEL(bmp280));

K_SEM_DEFINE(bt_ready, 0, 1);
K_SEM_DEFINE(advertize, 0, 1);

static void restart_advertizing(struct k_work *work);
K_WORK_DEFINE(adv_work, restart_advertizing);

static void on_ble_enable(int err)
{
    if (err != 0) {
        LOG_ERR("Failed to init stack: %d", err);
        return;
    }

    k_sem_give(&bt_ready);
}

static void connected(__maybe_unused struct bt_conn *conn, uint8_t err)
{
    if (err != 0) {
        LOG_ERR("Connection attempt failed: %d", err);
        return;
    }

    (void)atomic_set(&is_connected, 1);
    LOG_INF("Client connected");
}

static void disconnected(__maybe_unused struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Client disconnected - reason %d", reason);
    (void)atomic_set(&is_connected, 0);
    (void)k_work_submit(&adv_work);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

int main(void)
{
    int err = bt_enable(on_ble_enable);
    if (err != 0) {
        LOG_ERR("That did not work :( %d", err);
        return 0;
    }

    if (!device_is_ready(sensor)) {
        LOG_ERR("Sensor not ready");
        return 0;
    }

    (void)k_sem_take(&bt_ready, K_FOREVER);
    LOG_INF("Bluetooth stack is initialized - start advertizing");

    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, advertizing_data, ARRAY_SIZE(advertizing_data),
                          scan_data, ARRAY_SIZE(scan_data));
    if (err != 0) {
        LOG_ERR("Failed to start advertising: %d", err);
        return 0;
    }

    LOG_INF("Advertising! :)");

    for (;;) {
        err = sensor_sample_fetch(sensor);
        if (err != 0) {
            LOG_ERR("Failed to fetch sample: %d", err);
            goto sleep;
        }

        struct sensor_value temp;
        err = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        if (err != 0) {
            LOG_ERR("Failed to get temperature: %d", err);
            goto sleep;
        }

        int16_t adv_temp;
        if (temp.val1 >= 0) {
            adv_temp = (temp.val1 * 100) + (temp.val2 / 10000);
        } else {
            adv_temp = (temp.val1 * 100) - (temp.val2 / 10000);
        }
        LOG_INF("Temperature: %d.%d°C -> %d", temp.val1, temp.val2, adv_temp);

        atomic_set(&temp_value, adv_temp);
        if (atomic_get(&is_connected) == 1) {
            notify_temp(adv_temp);
        } else {
            manufacturer_data[2] = adv_temp >> 8;
            manufacturer_data[3] = adv_temp & 0xFF;

            err = bt_le_adv_update_data(advertizing_data, ARRAY_SIZE(advertizing_data), scan_data,
                                        ARRAY_SIZE(scan_data));
            if (err != 0) {
                LOG_ERR("Failed to update advertizing data: %d", err);
            }
        }

    sleep:
        k_msleep(1000);
    }
    return 0;
}

static void restart_advertizing(__maybe_unused struct k_work *work)
{
    int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, advertizing_data, ARRAY_SIZE(advertizing_data),
                              scan_data, ARRAY_SIZE(scan_data));
    if (err != 0) {
        LOG_ERR("Failed to update advertizing data: %d", err);
    }

    LOG_INF("Advertising again...");
    k_sem_give(&advertize);
}
