#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

#include <lib96gpio.h>

/*
 * During development, it helps to be able to switch all file
 * access to /tmp so all testing can be done on host machine.
 *
 * Remove in production.
 */

#ifdef DEBUG
#define FOLDER_PREFIX "/tmp"
#else
#define FOLDER_PREFIX ""
#endif

#define BUFF_MAX 64

#define PIN_MAP_DEF(n, m) const int PIN_MAP##n[] = {m}
#define PIN_MAP(n) PIN_MAP##n

#define LS_PIN_START  23
#define LS_PIN_END    34
#define HIKEY_PIN_MAP  488, 489, 490, 491, 492, 415, 463, 495, 426, 433, 427, 434
#define DRAGON_PIN_MAP  36,  12,  13,  69, 115, 507,  24,  25,  35,  34,  28,  33
#define BUBBLEGUM_PIN_MAP 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 155, 154

PIN_MAP_DEF(HIKEY,  HIKEY_PIN_MAP);
PIN_MAP_DEF(DRAGON, DRAGON_PIN_MAP);
PIN_MAP_DEF(DRAGON, BUBBLEGUM_PIN_MAP);

/* Globals */
int board_idx = -1;

struct gpio_callback {
	int soc_num;
	void *arg;
	void (*handler)(void *);
};

typedef struct {
	/* Name is useful if we plan to (someday) detect
	 * boards based on some environment variable. */
	const char *name;
	/* Board model as found in /proc/device-tree/model */
	const char *model;
	/* User GPIO pin number to SOC pin number translation  */
	const int pin_start;
	const int pin_end;
	const int *pin_map;
} board_spec_t;

board_spec_t board_spec[BRD_SENTINEL] = {

	[BRD_HIKEY] =  {
		.name      = "96boards_hikey",
		.model      = "HiKey Development Board",
		.pin_start = LS_PIN_START,
		.pin_end   = LS_PIN_END,
		.pin_map   = PIN_MAP(HIKEY),
	},

	[BRD_DRAGON] = {
		.name      = "96boards_dragon",
		.model      = "Qualcomm Technologies, Inc. APQ 8016 SBC",
		.pin_start = LS_PIN_START,
		.pin_end   = LS_PIN_END,
		.pin_map   = PIN_MAP(DRAGON),
	},
	
	[BRD_BUBBLEGUM] = {
		.name      = "96boards_bubblegum",
		.model      = "s900",
		.pin_start = LS_PIN_START,
		.pin_end   = LS_PIN_END,
		.pin_map   = PIN_MAP(BUBBLEGUM),
	},
};

static int board_detect()
{
	char tmp_buf[1024];
	int i, ret_val = -1, brd = -1;

	snprintf(tmp_buf, sizeof(tmp_buf),
		"%s/proc/device-tree/model", FOLDER_PREFIX);
	FILE *ret = fopen(tmp_buf, "r");

	if (ret == NULL) {
		perror("Unable to get board info from device tree\n");
		return ret_val;
	}

	do {
		fgets(tmp_buf, 1024, ret);
		for (i=0; i<BRD_SENTINEL; i++) {
			if (strcmp(tmp_buf, board_spec[i].model) == 0) {
				brd = i;
				break;
			}
		}
		if (brd < 0 || brd > BRD_SENTINEL) {
			fprintf(stderr, "96board: invalid board specified "
					"in env BOARD\n");
			break;
		}
		board_idx = brd;
		ret_val =0;
	} while(0);

	fclose(ret);

	return ret_val;
}

static int get_soc_num(int gpio)
{
	if (board_idx == -1 && board_detect() != 0)
		return -EBOARD_NOT_FOUND;

	board_spec_t *brd = &board_spec[board_idx];

	if (brd == NULL) {
		return -EBOARD_NOT_FOUND;
	}

	if (gpio < brd->pin_start || gpio > brd->pin_end)
		return -EPIN_NOT_FOUND;

	gpio -= brd->pin_start;

	return brd->pin_map[gpio];
}

int gpio_set_mode(int gpio, enum gpio_dir dir)
{
	int soc_num, fd;
	char buffer[BUFF_MAX];

	soc_num = get_soc_num(gpio);	
	if (soc_num < 0)
		return soc_num;

	/* Export the GPIO */
	snprintf(buffer, sizeof(buffer),
		"%s/sys/class/gpio/export", FOLDER_PREFIX);
	fd = open(buffer, O_WRONLY);
	if (fd < 0) {
		perror("Failed to export GPIO\n");
		return -1;
	}

	snprintf(buffer, sizeof(buffer), "%d", soc_num);
	write(fd, buffer, strlen(buffer));
	close(fd);

	/* Configure the GPIO */
	snprintf(buffer, sizeof(buffer),
		"%s/sys/class/gpio/gpio%d/direction", FOLDER_PREFIX, soc_num);
	fd = open(buffer, O_WRONLY);
	if (fd < 0) {
		perror("Failed to open GPIO direction");
		return -1;
	}

	write(fd, dir ? "out" : "in", 3);
	close(fd);

	return 0;
}

int gpio_write(int gpio, int val)
{
	int fd, soc_num;
	char buffer[BUFF_MAX];

	soc_num = get_soc_num(gpio);
	if (soc_num < 0)
		return soc_num;

	snprintf(buffer, sizeof(buffer),
		"%s/sys/class/gpio/gpio%d/value", FOLDER_PREFIX, soc_num);
	fd = open(buffer, O_WRONLY);
	if (fd < 0) {
		perror("Failed to open GPIO value");
		return -1;
	}

	write(fd, val ? "1" : "0", 3);
	close(fd);

	return 0;
}

int gpio_read(int gpio)
{
	int soc_num;
	int open_fd;
	char buffer[BUFF_MAX];
	char val;

	soc_num = get_soc_num(gpio);
	if (soc_num < 0)
		return soc_num;

	snprintf(buffer, sizeof(buffer),
		"%s/sys/class/gpio/gpio%d/value", FOLDER_PREFIX, soc_num);
	open_fd = open(buffer, O_RDWR);
	if (open_fd < 0) {
		perror("Failed to open GPIO value");
		return -1;
	}

	read(open_fd, &val, 1);
	close(open_fd);

	if (val < '0' || val > '1')
		return -1;

	return (val - '0');
}

int gpio_set_edge(int soc_num, enum gpio_edge edge)
{
	int fd;
	char buffer[BUFF_MAX];

	snprintf(buffer, sizeof(buffer),
		"%s/sys/class/gpio/gpio%d/edge", FOLDER_PREFIX, soc_num);
	fd = open(buffer, O_WRONLY);
	if (fd < 0) {
		perror("Failed to open GPIO value");
		return -1;
	}

	switch (edge) {
	case EDGE_NONE:
		write(fd, "none", 8);
		break;
	case EDGE_RISING:
		write(fd, "rising", 8);
		break;
	case EDGE_FALLING:
		write(fd, "falling", 8);
		break;
	case EDGE_BOTH:
		write(fd, "both", 8);
		break;
	default:
		perror("Unknown edge\n");
		return -EUNKNOWN_EDGE;

	}

	close(fd);

	return 0;
}

static int gpio_poll(int soc_num, unsigned long timeout_ms)
{
	int open_fd, ret;
	char buffer[BUFF_MAX];
	char ch;
	struct pollfd pfd;

	snprintf(buffer, sizeof(buffer),
		"%s/sys/class/gpio/gpio%d/value", FOLDER_PREFIX, soc_num);
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
	ret = poll(&pfd, 1, timeout_ms);
	if (ret < 0) {
		perror("GPIO poll error\n");
		return -1;
	}
	
	if (ret == 0 || ((pfd.revents & POLLPRI) == 0)) {
		fprintf(stderr, "96Boards: GPIO read timed out\n");
		return ret;	
	}

	lseek(pfd.fd, 0, SEEK_SET);
	read(pfd.fd, &ch, 3);

	return 0;
}

int gpio_wait_for_edge(int gpio, enum gpio_edge edge, unsigned long timeout_ms)
{
	int soc_num = get_soc_num(gpio);
	if (soc_num < 0)
		return -1;

	int ret = gpio_set_edge(soc_num, edge);
	if (ret < 0)
		return -1;

	return gpio_poll(soc_num, timeout_ms);
}

void *gpio_interrupt_routine(void *data)
{
	struct gpio_callback *cb = (struct gpio_callback *)data;
	if (gpio_poll(cb->soc_num, -1) == 0) {
		printf("GPIO interrupt\n");
		cb->handler(cb->arg);
	}
	free(cb);
	return NULL;
}

int gpio_register_event_cb(int gpio, enum gpio_edge edge,
	void (*handler)(void *), void *arg)
{
	int ret, soc_num;
	pthread_t thread_id;

	soc_num = get_soc_num(gpio);
 	if (soc_num < 0)
		return soc_num;

	ret = gpio_set_edge(soc_num, edge);
	if (ret < 0)
		return ret;

	struct gpio_callback *cb = malloc(sizeof(struct gpio_callback));
	if (cb == NULL) {
		perror("Out of memory\n");
		return -1;
	}
	cb->soc_num = soc_num;
	cb->handler = handler;
	cb->arg = arg;

	ret = pthread_create(&thread_id, NULL,
		gpio_interrupt_routine, (void *)cb);
	if (ret) {
		perror("Error while creating interrupt thread\n");
		free(cb);
		return -1;
	}

	return 0;
}
