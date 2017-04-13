#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

#include <96boards_gpio.h>

unsigned int gpio_idx;
pthread_mutex_t lock;

struct gpio_callback {
	int num;
	void *arg;
	void (*callback)(void *);
};

int gpio_table[2][12][2] = {
        /* HiKey */
        {{23, 488}, {24, 489}, {25, 490}, {26, 491}, {27, 492}, {28, 415}, {29, 463}, {30, 495}, {31, 426}, {32, 433}, {33, 427}, {34, 434}},
        /* DragonBoard */
        {{23, 36}, {24, 12}, {25, 13}, {26, 69}, {27, 115}, {28, 507}, {29, 24}, {30, 25}, {31, 35}, {32, 34}, {33, 28}, {34, 33}},
};

int get_soc_num(int gpio)
{
	int i, err = 1;
	int soc_num;

        for (i = 0;i < 12; i++) {
                if (gpio == gpio_table[gpio_idx][i][0]) {
                        soc_num = gpio_table[gpio_idx][i][1];
                        err = 0;
                }
        }

        if (err) {
                perror("Wrong GPIO specified\n");
                return -1;
        }

	return soc_num;
}

int gpio_setup(void)
{
	FILE *ret;
	char board[1024];
	char *hikey = "HiKey Development Board";
	char *dboard = "Qualcomm Technologies, Inc. APQ 8016 SBC";
	
	ret = fopen("/proc/device-tree/model", "r");
	if (ret == NULL) {
		perror("Unable to get board info from device tree\n");
		return -1;
	}

	while(fgets(board, 1024, ret) != NULL);
	if (!strcmp(board, hikey)) {
		gpio_idx = HIKEY;
	} else if (!strcmp(board, dboard)) {
		gpio_idx = DRAGONBOARD;
	}

	fclose(ret);	

	return 0;
}

int gpio_mode(int gpio, enum dir _dir)
{
	int ret, soc_num;
	int open_fd, rw_fd;
	char buffer[BUFF_MAX];
	
	/* Export the GPIO */
	open_fd = open("/sys/class/gpio/export", O_WRONLY);
	if (open_fd < 0) {
		perror("Failed to export GPIO\n");
		return -1;
	}
	
	soc_num = get_soc_num(gpio);	
        if (soc_num < 0)
                return soc_num;

	ret = snprintf(buffer, sizeof(buffer), "%d", soc_num);
	write(open_fd, buffer, ret);

	/* Configure the GPIO */
	snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", soc_num);
	rw_fd = open(buffer, O_WRONLY);
	if (rw_fd < 0) {
		perror("Failed to open GPIO direction");
		return -1;
	}
	
	write(rw_fd, _dir ? "out" : "in", 3);
	close(open_fd);
	close(rw_fd);

	return 0;
}

int gpio_output(int gpio, enum val _val)
{
	int open_fd;
	int soc_num;
	char buffer[BUFF_MAX];

	soc_num = get_soc_num(gpio);
        if (soc_num < 0)
                return soc_num;

	snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", soc_num);
	open_fd = open(buffer, O_WRONLY);
	if (open_fd < 0) {
		perror("Failed to open GPIO value");
		return -1;
	}
	
	write(open_fd, _val ? "1" : "0", 3);
	close(open_fd);
	
	return 0;
}

int gpio_input(int gpio)
{
	int soc_num;
	int open_fd;
	char buffer[BUFF_MAX];
	char val;
	
	soc_num = get_soc_num(gpio);
        if (soc_num < 0)
                return soc_num;

	snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", soc_num);
	open_fd = open(buffer, O_RDWR);
	if (open_fd < 0) {
		perror("Failed to open GPIO value");
		return -1;
	}
	
	read(open_fd, &val, 1);
	close(open_fd);

	return val;	
}

int gpio_edge(int soc_num, enum edge _edge)
{
	int open_fd;
	char buffer[BUFF_MAX];

        snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/edge", soc_num);
        open_fd = open(buffer, O_WRONLY);
        if (open_fd < 0) {
                perror("Failed to open GPIO value");
                return -1;
        }

	switch (_edge) {
	case NONE:
		write(open_fd, "none", 8);
		break;
	case RISING:
		write(open_fd, "rising", 8);
		break;
	case FALLING:
		write(open_fd, "falling", 8);
		break;
	case BOTH:
		write(open_fd, "both", 8);
		break;
	default:
		perror("Wrong edge specified\n");
	}

	close(open_fd);
	
	return 0;
}	

int gpio_poll(int soc_num, unsigned long timeout)
{
	int open_fd, ret;
	char buffer[BUFF_MAX];
	char ch;
	struct pollfd pfd;

	snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", soc_num);
        open_fd = open(buffer, O_RDWR);
        if (open_fd < 0) {
                perror("Failed to open GPIO value");
                return -1;
        }
	pfd.fd = open_fd;
	pfd.events = POLLPRI | POLLERR;
	pfd.revents = 0;

	/* Dummy read */
	read(open_fd, &ch, 1);
	
	/* Wait until timeout */
	ret = poll(&pfd, 1, timeout);
	if (ret < 0) {
		perror("GPIO poll error\n");
		return -1;
	} else if (pfd.revents & POLLPRI) {
		lseek(pfd.fd, 0, SEEK_SET);
		read(pfd.fd, &ch, 1);
		return 0;
	}

	perror("GPIO poll timeout\n");
	
	return ret;	
}

int gpio_wait_for_edge(int gpio, enum edge _edge, unsigned long timeout)
{
	int ret, soc_num;

	soc_num = get_soc_num(gpio);
	if (soc_num < 0)
		return soc_num;

	ret = gpio_edge(soc_num, _edge);
	if (ret < 0)
		return ret;
	
	return gpio_poll(soc_num, timeout);
}

void gpio_interrupt_routine(struct gpio_callback *__gpio_callback)
{
	int ret;

	pthread_mutex_lock(&lock);
		
	if (gpio_poll(__gpio_callback->num, -1) == 0) {
		printf("GPIO interrupt\n");
		__gpio_callback->callback(__gpio_callback->arg);
	}
}

int gpio_add_event_detect(int gpio, enum edge _edge, void (*callback)(void *), void *arg)
{
	int ret, soc_num;
	pthread_t thread_id;
	struct gpio_callback *_gpio_callback = malloc(sizeof(struct gpio_callback));

        soc_num = get_soc_num(gpio);
        if (soc_num < 0)
                return soc_num;

        ret = gpio_edge(soc_num, _edge);
        if (ret < 0)
                return ret;

	if (pthread_mutex_init(&lock, NULL) != 0) {
		perror("Mutex init failed\n");
		return -1;
	}

	_gpio_callback->num = soc_num;
	_gpio_callback->callback = callback;
	_gpio_callback->arg = arg;

	ret = pthread_create(&thread_id, NULL, gpio_interrupt_routine, _gpio_callback);
	if (ret) {
		perror("Error while creating interrupt thread\n");
		return -1;
	}

	return 0;		
}
