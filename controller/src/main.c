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

#include "drivers/mpu6050.h"

int setup() {
	if (initMPU6050() != 0) {
		return 1;
	}
	return 0;
}

int main(void)
{
	if (setup() != 0) {
		return 1;
	}

	while(1) {
		struct mpu6050_reading reading = readMPU6050();
		if (reading.status != 0) {
			break;
		}
		k_sleep(K_SECONDS(2));
	}

	return 0;
}
