#ifndef GPIO_H
#define GPIO_H

#define HIGH 1
#define LOW  0

enum gpio_dir {
	GPIO_IN = 0,
	GPIO_OUT,
};

enum gpio_edge {
	EDGE_NONE = 0,
	EDGE_RISING,
	EDGE_FALLING,
	EDGE_BOTH,
	EDGE_SENTINEL
};

enum boards_boards {
	BRD_HIKEY,
	BRD_DRAGON,
	BRD_SENTINEL
};

enum gpio_errors {
	ENO_ERROR,
	EUNKNOWN,
	EBOARD_NOT_FOUND,
	EPIN_NOT_FOUND,
	ESENTINEL
};

int gpio_set_mode(int gpio, enum gpio_dir);
int gpio_write(int gpio, int val);
int gpio_read(int gpio);
int gpio_set_edge(int gpio, enum gpio_edge edge);
int gpio_wait_for_edge(int gpio, enum gpio_edge edge, unsigned long timeout_ms);

#endif
