#include <stdio.h>
#include <unistd.h>
#include <96gpio.h>

void my_func(int num)
{
	printf("Thread: %d\n", num);
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
        gpio_add_event_detect(25, EDGE_RISING, my_func, 12);
	sleep(10);

	return 0;
}
