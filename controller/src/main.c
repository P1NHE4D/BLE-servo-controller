/*
 * Copyright (c) 2023 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "mpu6050.h"

#if !DT_NODE_EXISTS(DT_NODELABEL(status_led))
#error "Overlay for power output node not properly defined."
#endif

static const struct gpio_dt_spec status_led =
	GPIO_DT_SPEC_GET_OR(DT_NODELABEL(status_led), gpios, {0});

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
	/*

	int err;

	if (!gpio_is_ready_dt(&status_led)) {
		printf("The load switch pin GPIO port is not ready.\n");
		return 0;
	}

	printf("Initializing pin with inactive level.\n");

	err = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_ACTIVE);
	if (err != 0) {
		printf("Configuring GPIO pin failed: %d\n", err);
		return 0;
	}

	printf("Waiting one second.\n");

	k_sleep(K_MSEC(1000));

	printf("Setting pin to active level.\n");

	err = gpio_pin_set_dt(&status_led, 1);
	if (err != 0) {
		printf("Setting GPIO pin level failed: %d\n", err);
	}
	return 0;
	*/
}
