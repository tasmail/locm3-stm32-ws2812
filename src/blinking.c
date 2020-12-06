#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "blinking.h"


void initialize_blinking(void)
{
	/* Enable GPIOC clock for LED */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Setup GPIO pin GPIO13 on GPIO port C for LED. */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
}

void led_off(void)
{
    /* LED on */
    gpio_set(GPIOC, GPIO13);
}

void led_on(void)
{
    /* LED off */
    gpio_clear(GPIOC, GPIO13);
}

void led_toggle(void)
{
    /* LED on/off */
    gpio_toggle(GPIOC, GPIO13);	
}
