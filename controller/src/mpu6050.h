#ifndef DRIVERS_I2C_H
#define DRIVERS_I2C_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

struct mpu6050_reading  {
    int status;
    struct sensor_value temperature;
    struct sensor_value accel[3];
    struct sensor_value gyro[3];
};

int initMPU6050();

struct mpu6050_reading readMPU6050();

#endif