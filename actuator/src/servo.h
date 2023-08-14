#ifndef DRIVERS_SERVO_H
#define DRIVERS_SERVO_H

#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

int8_t setAngle(uint16_t angle);

#endif