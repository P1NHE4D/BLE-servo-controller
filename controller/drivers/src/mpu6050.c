#include "drivers/mpu6050.h"

const struct device *mpu6050;

static const char *now_str(void)
{
	static char buf[16]; /* ...HH:MM:SS.MMM */
	uint32_t now = k_uptime_get_32();
	unsigned int ms = now % MSEC_PER_SEC;
	unsigned int s;
	unsigned int min;
	unsigned int h;

	now /= MSEC_PER_SEC;
	s = now % 60U;
	now /= 60U;
	min = now % 60U;
	now /= 60U;
	h = now;

	snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
		 h, min, s, ms);
	return buf;
}

int initMPU6050() {
    mpu6050 = DEVICE_DT_GET_ONE(invensense_mpu6050);

	if (!device_is_ready(mpu6050)) {
		printf("Device %s is not ready\n", mpu6050->name);
		return 1;
	}
	return 0;
}

struct mpu6050_reading readMPU6050() {

    struct mpu6050_reading sensor_reading;
	sensor_reading.status = 0;

	int rc = sensor_sample_fetch(mpu6050);

	if (rc == 0) {
		rc = sensor_channel_get(mpu6050, SENSOR_CHAN_ACCEL_XYZ,
					sensor_reading.accel);
	}
	if (rc == 0) {
		rc = sensor_channel_get(mpu6050, SENSOR_CHAN_GYRO_XYZ,
					sensor_reading.gyro);
	}
	if (rc == 0) {
		rc = sensor_channel_get(mpu6050, SENSOR_CHAN_AMBIENT_TEMP,
					&sensor_reading.temperature);
	}

	if (rc == 0) {
		printf("[%s]:%g Cel\n"
		       "  accel %f %f %f m/s/s\n"
		       "  gyro  %f %f %f rad/s\n",
		       now_str(),
		       sensor_value_to_double(&sensor_reading.temperature),
		       sensor_value_to_double(&sensor_reading.accel[0]),
		       sensor_value_to_double(&sensor_reading.accel[1]),
		       sensor_value_to_double(&sensor_reading.accel[2]),
		       sensor_value_to_double(&sensor_reading.gyro[0]),
		       sensor_value_to_double(&sensor_reading.gyro[1]),
		       sensor_value_to_double(&sensor_reading.gyro[2]));
	} else {
		printf("sample fetch/get failed: %d\n", rc);
		sensor_reading.status = 1;
	}

	return sensor_reading;
}