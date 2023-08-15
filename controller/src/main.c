/*
 * Copyright (c) 2023 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

// bluetooth headers
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

#include "lib/accelerometer.h"

LOG_MODULE_REGISTER(Servo_Controller, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

// advertising packet
static const struct bt_data ad[] = {
	// advertising flags
	// act as Bluetooth LE peripheral
	// - no classic bluetooth (BR/EDR)
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

	// advertising packet data
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

// scan response packet
static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123))
};

int setup() {
	if (initMPU6050() != 0) {
		return 1;
	}
	return 0;
}

int main(void)
{
	int blink_status = 0;
	int err = setup();
	if (err) {
		LOG_ERR("Setup failed");
		return 1;
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return 1;
	}

	bt_addr_le_t addr;
    err = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
    if (err) {
        printk("Invalid BT address (err %d)\n", err);
    }

    err = bt_id_create(&addr, NULL);
    if (err < 0) {
        printk("Creating new ID failed (err %d)\n", err);
    }

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d\n)", err);
		return 1;
	}
	LOG_INF("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return 1;
	}
	LOG_INF("Advertising sucessfully started\n");


	while(true) {
		struct mpu6050_reading reading = readMPU6050();
		if (reading.status != 0) {
			break;
		}
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}

	return 0;
}
