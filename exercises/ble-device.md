# Bluetooth Low Energy

Finally, it is time to talk to the outside world using Bluetooth Low Energy!

## Setup
We will discuss BLE together in class and you will have some working advertizing going before you
can proceed.

## Exercise 1: Find your device
Configure a custom device name using `CONFIG_BT_DEVICE_NAME` and try to find your device using your
phone or computer.

## Exercise 2: How warm is it?
Advertising data can be dynamically updated using
```c
/**
 * @brief Update advertising
 *
 * Update advertisement and scan response data.
 *
 * @param ad Data to be used in advertisement packets.
 * @param ad_len Number of elements in ad
 * @param sd Data to be used in scan response packets.
 * @param sd_len Number of elements in sd
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_adv_update_data(const struct bt_data *ad, size_t ad_len,
                          const struct bt_data *sd, size_t sd_len);
```

Use ZBUS to pass read the ambient temperature and pack it inside the so called _manufacturer data_.
The manufacturer data is some advertising data that is vendor specific. Its first two bytes indicate
the company name (you can make those up).
Then, you can put arbitrary payload in there, we use a two-byte integer for the temperature.
You can decide the format.

```c
// Payload in the last two bytes
static uint8_t manufacutrer_data[] = { 0xFF, 0xFF, 0x00, 0x00 };

// Extend your advertising data
static struct bt_data advertising_data[] = {
    // ...
    BT_DATA(BT_DATA_MANUFACTURER_DATA, manufacutrer_data, sizeof(manufacutrer_data)),
};
```

## Exercise 3: Connect to your peripheral
So far, we can only scan and see some broadcasted sensor data hacked into the manufacturer data.
Time to change that!

In order to be able to connect to your device, you must define two callbacks, `connect` and
`disconnect`:
```c
static void connected(struct bt_conn *conn, uint8_t err) {
    // Check error code

    // You could also update connection parameters here using bt_conn_le_param_update...
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    // Go back to advertising
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};
```

## Exercise 4: Expose a service
Next up, we are defining a service to expose our measurement values.
Our service, as well as each characteristic need _UUIDs_ that can be declared using
```c
// Some random hex values...
const struct bt_uuid_128 = BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x1234, 0x1234, 0x123456789abc));
```

You need to define callbacks for reading the data which calls:
```c
/** @brief Generic Read Attribute value helper.
 *
 *  Read attribute value from local database storing the result into buffer.
 *
 *  @param conn Connection object.
 *  @param attr Attribute to read.
 *  @param buf Buffer to store the value.
 *  @param buf_len Buffer length.
 *  @param offset Start offset.
 *  @param value Attribute value.
 *  @param value_len Length of the attribute value.
 *
 *  @return number of bytes read in case of success or negative values in
 *          case of error.
 */
ssize_t bt_gatt_attr_read(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t buf_len, uint16_t offset,
                          const void *value, uint16_t value_len);

```

The function signature for the read callback is
```c
static ssize_t read_callback(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, uint16_t len, uint16_t offset);

```

After that, you need to define the service hierarchy:
```c
BT_GATT_SERVICE_DEFINE(sensor_svc,
    BT_GATT_PRIMARY_SERVICE(&sensor_svc_uuid),

    BT_GATT_CHARACTERISTIC(&temp_char_uuid.uuid,
        BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        read_callback, NULL, NULL),
);
```

## Exercise 4 (ADVANCED):
You have already gained some experience here!
Our device has a few issues, though:

1. Our connected device needs to periodically poll for new data.
2. The connection is not really secure...

Take a look at `zephyr/samples/bluetooth/peripheral_dis` and find out, how to notify your connected
client of any value changes.

For the security part, study `zephyr/samples/bluetooth/peripheral` to see how to implement
authentication.
Combine this with an LED indication and a button press to confirm pairing on your side.
