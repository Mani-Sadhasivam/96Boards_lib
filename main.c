#include "96boards_gpio.h"
#include <stdio.h>
#include <unistd.h>

void my_func(int num)
{
	printf("Thread: %d\n", num);
}

int main(void)
{
	/* Setup GPIO for the board */
	gpio_setup();

	/* Configure GPIO as output */
	gpio_mode(23, OUT);

	/* Configure GPIO as input */
	gpio_mode(25, IN);

	/* Output high */
	gpio_output(23, HIGH);

	/* Wait for GPIO to change state */
        gpio_add_event_detect(25, RISING, my_func, 12);
	sleep(10);

	return 0;
}
