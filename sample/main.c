#include <stdio.h>
#include <unistd.h>

#include <lib96gpio.h>

void my_func(void *data)
{
	int *num = (int *)data;
	if (num)
		printf("Thread: %d\n", *num);
	else
		printf("Thread: null\n");
}

int main(void)
{
	/* Configure GPIO as output */
	gpio_set_mode(23, GPIO_OUT);

	/* Configure GPIO as input */
	gpio_set_mode(25, GPIO_IN);

	/* Output high */
	gpio_write(23, HIGH);

	/* Wait for GPIO to change state */
	int num = 12;
	gpio_register_event_cb(25, EDGE_RISING, my_func, (void *)&num);
	sleep(10);

	return 0;
}
