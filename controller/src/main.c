/*
 * Copyright (c) 2023 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <dk_buttons_and_leds.h>

#include "drivers/mpu6050.h"

int setup() {
	if (initMPU6050() != 0) {
		return 1;
	}
	return 0;
}

void error() {
	dk_set_leds_state(DK_ALL_LEDS_MSK, DK_NO_LEDS_MSK);

	while (true) {
		k_sleep(K_MSEC(1000));
	}
}

int main(void)
{
	int err = 0;
	if (setup() != 0) {
		return 1;
	}

	while(true) {
		struct mpu6050_reading reading = readMPU6050();
		if (reading.status != 0) {
			break;
		}
		k_sleep(K_SECONDS(2));
	}

	return 0;
}
