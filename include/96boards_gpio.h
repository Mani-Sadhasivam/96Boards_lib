#ifndef GPIO_H
#define GPIO_H

#define BUFF_MAX 64

enum dir {
	IN = 0,
	OUT,
};

enum val {
	LOW = 0,
	HIGH,
};

enum edge {
	NONE = 0,
	RISING,
	FALLING,
	BOTH,
};

enum gpio_names {
	HIKEY = 0,
	DRAGONBOARD,
};

int gpio_setup(void);
int gpio_mode(int gpio, enum dir _dir);
int gpio_output(int gpio, enum val _val);
int gpio_input(int gpio);
int gpio_add_event_detect(int gpio, enum edge _edge, void (*callback)(void *), void *arg);
int gpio_wait_for_edge(int gpio, enum edge _edge, unsigned long timeout);

#endif
