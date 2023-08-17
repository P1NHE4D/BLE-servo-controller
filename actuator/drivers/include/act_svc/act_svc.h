#ifndef DRIVERS_ACT_SVC_H
#define DRIVERS_ACT_SVC_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#define BT_UUID_ACTS_VAL \
    BT_UUID_128_ENCODE(0x9bc57c25, 0x0e55, 0x401e, 0xb238, 0xad7be2de6963)

#define BT_UUID_ACTS_SERVO_VAL \
    BT_UUID_128_ENCODE(0x9bc57c26, 0x0e55, 0x401e, 0xb238, 0xad7be2de6963)

#define BT_UUID_ACTS            BT_UUID_DECLARE_128(BT_UUID_ACTS_VAL)
#define BT_UUID_ACTS_SERVO      BT_UUID_DECLARE_128(BT_UUID_ACTS_SERVO_VAL)


#endif