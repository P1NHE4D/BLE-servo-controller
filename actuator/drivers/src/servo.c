#include "servo/servo.h"

LOG_MODULE_DECLARE(Servo_Actuator);

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);


int8_t setAngle(uint16_t angle) {
    if (!device_is_ready(servo.dev)) {
		LOG_ERR("Error: PWM device %s is not ready\n", servo.dev->name);
		return 1;
	}

    if (angle > 180) {
        angle = 180;
    }

    uint32_t pulse_width = ((angle * (max_pulse - min_pulse)) / 180) + min_pulse;
    int ret = pwm_set_pulse_dt(&servo, pulse_width);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to set pulse width\n", ret);
        return 1;
    }
    return 0;
}