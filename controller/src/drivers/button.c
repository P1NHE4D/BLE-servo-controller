#include "button.h"

#if !DT_NODE_EXISTS(DT_NODELABEL(status_led))
#error "Overlay for status led node not properly defined."
#endif

static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(status_led), gpios, {0});

int initButton() {
    int err;

	if (!gpio_is_ready_dt(&status_led)) {
		printf("The status led pin GPIO port is not ready.\n");
		return 0;
	}

	printf("Initializing pin with inactive level.\n");

	err = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
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
}