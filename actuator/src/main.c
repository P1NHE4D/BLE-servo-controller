/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "servo.h"

int main(void)
{
	printk("Setting angle!");
	setAngle(0);
	k_sleep(K_SECONDS(2));
	printk("Setting angle!");
	setAngle(180);
	k_sleep(K_SECONDS(2));
	printk("Setting angle!");
	setAngle(90);
	return 0;
}
