#include "lib/mpu6050_sensor.h"

#define STACKSIZE 				1024
#define THREAD_PRIORITY			-1

#define M_PI 					3.14159265358979323846
#define CALIB_OFFSET_NB_MES 	500
#define DEFAULT_GYRO_COEFF    	0.98
#define DEFAULT_ACC_COEFF		1.0 - DEFAULT_GYRO_COEFF
#define RAD_2_DEG             	57.29578


float gyroXoffset, gyroYoffset, gyroZoffset;
float accXoffset, accYoffset, accZoffset;
float accX, accY, accZ, gyroX, gyroY, gyroZ;
float angleAccX, angleAccY;
float angleX, angleY, angleZ;
unsigned long preInterval;

static float wrap(float angle,float limit){
  while (angle >  limit) angle -= 2*limit;
  while (angle < -limit) angle += 2*limit;
  return angle;
}

struct device* device;

int initMPU6050() {
	device = DEVICE_DT_GET_ONE(invensense_mpu6050);
	if (!device_is_ready(device)) {
		return 1;
	}

	printk("Initial angles: %f, %f, %f\n", angleX, angleY, angleZ);

	update();
	printk("After first update: %f, %f, %f\n", angleX, angleY, angleZ);
	preInterval = k_uptime_get();

	accXoffset = 0.0;
	accYoffset = 0.0;
	accZoffset = 0.0;
	gyroXoffset = 0.0;
	gyroYoffset = 0.0;
	gyroZoffset = 0.0;

	int rc;
	struct sensor_value accel[3];
	struct sensor_value gyro[3];


	float ag[6] = {0, 0, 0, 0, 0, 0};

	for (int i = 0; i < CALIB_OFFSET_NB_MES; i++) {

		int rc = sensor_sample_fetch(device);

		if (rc == 0) {
			rc = sensor_channel_get(device, SENSOR_CHAN_ACCEL_XYZ, accel);
		}
		if (rc == 0) {
			rc = sensor_channel_get(device, SENSOR_CHAN_GYRO_XYZ, gyro);
		}
		if (rc != 0) {
			return 1;
		}

		ag[0] += sensor_value_to_double(&accel[0]) / 16384.0;;
		ag[1] += sensor_value_to_double(&accel[1]) / 16384.0;
		ag[2] += sensor_value_to_double(&accel[2]) / 16384.0;
		ag[3] += sensor_value_to_double(&gyro[0]) / 131.0;
		ag[4] += sensor_value_to_double(&gyro[1]) / 131.0;
		ag[5] += sensor_value_to_double(&gyro[2]) / 131.0;
		k_msleep(1);
	}

	accXoffset = ag[0] / CALIB_OFFSET_NB_MES;
    accYoffset = ag[1] / CALIB_OFFSET_NB_MES;
    accZoffset = ag[2] / CALIB_OFFSET_NB_MES;
	gyroXoffset = ag[3] / CALIB_OFFSET_NB_MES;
    gyroYoffset = ag[4] / CALIB_OFFSET_NB_MES;
    gyroZoffset = ag[5] / CALIB_OFFSET_NB_MES;

	printk("After init complete: %f, %f, %f\n", angleX, angleY, angleZ);
	return 0;
}

float getRoll() {
	return angleX;
}

float getPitch() {
	return angleY;
}

float getYaw() {
	return angleZ;
}

int update() {
	if (!device_is_ready(device)) {
		return;
	}
	struct sensor_value accel[3];
	struct sensor_value gyro[3];

	// raw values
	int rc = sensor_sample_fetch(device);

	if (rc == 0) {
		rc = sensor_channel_get(device, SENSOR_CHAN_ACCEL_XYZ, accel);
	}
	if (rc == 0) {
		rc = sensor_channel_get(device, SENSOR_CHAN_GYRO_XYZ, gyro);
	}
	if (rc != 0) {
		return;
	}

	float accX = sensor_value_to_double(&accel[0]) / 16384.0 - accXoffset;
	float accY = sensor_value_to_double(&accel[1]) / 16384.0 - accYoffset;
	float accZ = sensor_value_to_double(&accel[2]) / 16384.0 - accZoffset;
	float gyroX = sensor_value_to_double(&gyro[0]) / 131.0 - gyroXoffset;
	float gyroY = sensor_value_to_double(&gyro[1]) / 131.0 - gyroYoffset;
	float gyroZ = sensor_value_to_double(&gyro[2]) / 131.0 - gyroZoffset;

	float sgZ = accZ < 0 ? -1 : 1;
	angleAccX =   atan2(accY, sgZ*sqrt(accZ*accZ + accX*accX)) * RAD_2_DEG;
	angleAccY = - atan2(accX,     sqrt(accZ*accZ + accY*accY)) * RAD_2_DEG;

	unsigned long Tnew = k_uptime_get();
	float dt = (Tnew - preInterval) * 1e-3;
	preInterval = Tnew;

	angleX = wrap(DEFAULT_GYRO_COEFF*(angleAccX + wrap(angleX +     gyroX*dt - angleAccX,180)) + (1.0-DEFAULT_GYRO_COEFF)*angleAccX,180);
  	angleY = wrap(DEFAULT_GYRO_COEFF*(angleAccY + wrap(angleY + sgZ*gyroY*dt - angleAccY, 90)) + (1.0-DEFAULT_GYRO_COEFF)*angleAccY, 90);
  	angleZ += gyroZ*dt;
}
