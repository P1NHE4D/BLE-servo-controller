/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

#include <bluetooth/scan.h>
#include <bluetooth/gatt_dm.h>

#include <dk_buttons_and_leds.h>

#include "servo/servo.h"

LOG_MODULE_REGISTER(Servo_Actuator, LOG_LEVEL_DBG);

#define STACK_SIZE 1024
#define THREAD_PRIORITY 7
#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 500
#define CONNECTION_STATUS_LED DK_LED2
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define BT_UUID_CTRLS_VAL BT_UUID_128_ENCODE(0xD1B81E50, 0x3C0D, 0x11EE, 0x8F23, 0x0800200C9A66)
#define BT_UUID_CTRLS BT_UUID_DECLARE_128(BT_UUID_CTRLS_VAL)
#define BT_UUID_CTRLS_ROLL_VAL \
	BT_UUID_128_ENCODE(0xD1B81E51, 0x3C0D, 0x11EE, 0x8F23, 0x0800200C9A66)
#define BT_UUID_CTRLS_ROLL BT_UUID_DECLARE_128(BT_UUID_CTRLS_ROLL_VAL)

static K_SEM_DEFINE(servo_angle_lock, 1, 1);

static int blink_status = 0;
static int servo_angle = 90;

static struct bt_conn *default_conn;

static struct bt_uuid_128 discover_uuid = BT_UUID_INIT_128(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;

static uint8_t notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (!data) {
		LOG_INF("[UNSUBSCRIBED]\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}

	int roll = sys_get_le32(((int *)data));

	uint16_t angle;
	if (roll < 0) {
		angle = 90 - (roll * -1 / 2);
	} else {
		angle = 90 + (roll / 2);
	}
	LOG_INF("Angle: %d Roll: %d", angle, roll);

	setAngle(angle);

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
							 const struct bt_gatt_attr *attr,
							 struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		LOG_INF("Discover complete\n");
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	LOG_INF("[ATTRIBUTE] handle %u\n", attr->handle);


	if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_CTRLS)) {
		memcpy(&discover_uuid, BT_UUID_CTRLS_ROLL, sizeof(discover_uuid));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			LOG_INF("Discover failed (err %d)\n", err);
		}
		LOG_INF("Handle: %d", attr->handle);
	} else if (!bt_uuid_cmp(discover_params.uuid,
				BT_UUID_CTRLS_ROLL)) {
		memcpy(&discover_uuid, BT_UUID_GATT_CCC, sizeof(discover_uuid));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.start_handle = attr->handle + 2;
		discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
		subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

		err = bt_gatt_discover(conn, &discover_params);
		LOG_INF("Handle: %d", attr->handle);
		if (err) {
			LOG_INF("Discover failed (err %d)\n", err);
		}
	} else {
		subscribe_params.notify = notify_func;
		subscribe_params.value = BT_GATT_CCC_NOTIFY;
		subscribe_params.ccc_handle = attr->handle;

		err = bt_gatt_subscribe(conn, &subscribe_params);
		if (err && err != -EALREADY) {
			LOG_INF("Subscribe failed (err %d)\n", err);
		} else {
			LOG_INF("[SUBSCRIBED]\n");
		}

		return BT_GATT_ITER_STOP;
	}
	

	return BT_GATT_ITER_STOP;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err)
	{
		LOG_INF("Failed to connect to %s (%d)", addr, conn_err);

		if (default_conn == conn)
		{
			bt_conn_unref(default_conn);
			default_conn = NULL;

			err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
			if (err)
			{
				LOG_ERR("Scanning failed to start (err %d)",
						err);
			}
		}

		return;
	}

	LOG_INF("Connected: %s", addr);

	memcpy(&discover_uuid, BT_UUID_CTRLS, sizeof(discover_uuid));
	discover_params.uuid = &discover_uuid.uuid;
	discover_params.func = discover_func;
	discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
	discover_params.type = BT_GATT_DISCOVER_PRIMARY;

	err = bt_gatt_discover(default_conn, &discover_params);
	if (err) {
		LOG_ERR("Discover failed(err %d)\n", err);
		return;
	}

	err = bt_scan_stop();
	if ((!err) && (err != -EALREADY))
	{
		LOG_ERR("Stop LE scan failed (err %d)", err);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Disconnected: %s (reason %u)", addr, reason);

	if (default_conn != conn)
	{
		return;
	}

	bt_conn_unref(default_conn);
	default_conn = NULL;

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err)
	{
		LOG_ERR("Scanning failed to start (err %d)",err);
	}
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected};

static void scan_filter_match(struct bt_scan_device_info *device_info,
							  struct bt_scan_filter_match *filter_match,
							  bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	LOG_INF("Filters matched. Address: %s connectable: %d",
			addr, connectable);
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	LOG_WRN("Connecting failed");
}

static void scan_connecting(struct bt_scan_device_info *device_info,
							struct bt_conn *conn)
{
	LOG_INF("Connecting...");
	default_conn = bt_conn_ref(conn);
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL, scan_connecting_error, scan_connecting);

static int scan_init(void)
{
	int err;
	struct bt_scan_init_param scan_init = {
		.connect_if_match = 1,
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_CTRLS);
	if (err)
	{
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return err;
	}

	err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
	if (err)
	{
		LOG_ERR("Filters cannot be turned on (err %d)", err);
		return err;
	}

	LOG_INF("Scan module initialized");
	return err;
}

int main(void)
{
	int err;
	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}

	setAngle(servo_angle);

	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d\n)", err);
		return 1;
	}
	LOG_INF("Bluetooth initialized\n");

	err = scan_init();
	if (err != 0)
	{
		LOG_ERR("scan_init failed (err %d)", err);
		return 0;
	}

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err)
	{
		LOG_ERR("Scanning failed to start (err %d)", err);
		return 0;
	}

	LOG_INF("Scanning successfully started");


	while (true)
	{
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}

	return 0;
}
