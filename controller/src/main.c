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
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

// custom drivers
#include "lib/mpu6050_sensor.h"

LOG_MODULE_REGISTER(Servo_Controller, LOG_LEVEL_DBG);

#define STACKSIZE 				1024
#define THREAD_PRIORITY			7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

#define CONNECTION_STATUS_LED DK_LED2

/** @brief MPU6050 Service Characteristic UUID */
#define BT_UUID_MPU6050S_VAL \
	BT_UUID_128_ENCODE(0xD1B81E50, 0x3C0D, 0x11EE, 0x8F23, 0x0800200C9A66)

/** @brief Temp Characteristic UUID. */
#define BT_UUID_MPU6050S_ROLL_VAL \
	BT_UUID_128_ENCODE(0xD1B81E51, 0x3C0D, 0x11EE, 0x8F23, 0x0800200C9A66)

#define BT_UUID_MPU6050S      BT_UUID_DECLARE_128(BT_UUID_MPU6050S_VAL)
#define BT_UUID_MPU6050S_ROLL BT_UUID_DECLARE_128(BT_UUID_MPU6050S_ROLL_VAL)

static K_SEM_DEFINE(mpu_init_ok, 0, 1);

int roll;

static ssize_t read_roll(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset
) {
	const int *value = attr->user_data;

	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,(void *)conn);

	roll = (int)getRoll();
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		value,
		sizeof(*value)
	);
	return 0;
}

BT_GATT_SERVICE_DEFINE(mpu6050_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_MPU6050S),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_MPU6050S_ROLL,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_roll,
		NULL,
		&roll
	),
);

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

struct bt_conn *bt_connection = NULL;

void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    bt_connection = bt_conn_ref(conn);

	struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
	if (err) {
   		LOG_ERR("bt_conn_get_info() returned %d", err);
   		return;
	}
	double connection_interval = info.le.interval*1.25; // in ms
	uint16_t supervision_timeout = info.le.timeout*10; // in ms
	LOG_INF("Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, info.le.latency, supervision_timeout);



    dk_set_led(CONNECTION_STATUS_LED, 1);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(bt_connection);

    dk_set_led(CONNECTION_STATUS_LED, 0);
}

void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout) {
	double connection_interval = interval*1.25;         // in ms
    uint16_t supervision_timeout = timeout*10;          // in ms
    LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, latency, supervision_timeout);
}

struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
	.le_param_updated		= on_le_param_updated,
};


int main(void)
{
	int err;
	int blink_status = 0;

	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return 1;
	}

	err = initMPU6050();
	if (err) {
		LOG_ERR("Failed to initialise MPU6050\n");
		return 1;
	}

	k_sem_give(&mpu_init_ok);

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

	bt_conn_cb_register(&connection_callbacks);

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return 1;
	}
	LOG_INF("Advertising sucessfully started\n");


	while(true) {
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}

	return 0;
}

void update_thread(void) {
	k_sem_take(&mpu_init_ok, K_FOREVER);

	while (true) {
		update();
		k_msleep(10);
	}
}

K_THREAD_DEFINE(update_thread_id, STACKSIZE, update_thread, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
