#ifndef DRIVERS_MPU6050_SENSOR_H
#define DRIVERS_MPU6050_SENSOR_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <math.h>

int initMPU6050();
int update();
float getRoll();
float getPitch();
float getYaw();

#endif