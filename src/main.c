    /*
* This file is part of the libopencm3 project.
*
* Copyright (C) 2015 Piotr Esden-Tempski <piotr@esden.net>
* Copyright (C) 2015 Jack Ziesing <jziesing@gmail.com>
*
* This library is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library. If not, see <http://www.gnu.org/licenses/>.
*/
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/dma.h>
#include <libopencmsis/core_cm3.h>

#include "ws2812.h"
#include "blinking.h"
#include "usb.h" 
#include "systick.h" 
#include "uart.h" 

#define LED_COUNT (64)

struct led_status {
    ws2812_led_t leds[LED_COUNT];
    struct {
        int r;
        int g;
        int b;
    } dir[LED_COUNT];

    uint32_t timer;
} led_status;

void clock_setup(void);
void leds_init(void);
void leds_run(void);

void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
}

void leds_init(void) {
    int i;

#if 0
    int8_t color;
    int led_increment = (255/LED_COUNT) * 2;
#endif

    for(i=0; i < LED_COUNT; i++) {
      led_status.leds[i].grbu = 0;
    }

    for (i = 0; i < LED_COUNT; i++) {
#if 0
        color = -255 + (led_increment * i);
        if (color < 0) color = color * -1;
        led_status.leds[i].colors.r = color;
        if ((i % 6) < 3) {
            led_status.dir[i].r = 1;
        } else {
            led_status.dir[i].r = -1;
        }
#else
        led_status.leds[i].colors.r = 0;
        led_status.dir[i].r = 0;
#endif

#if 0
        color = -255 + (led_increment * (LED_COUNT / 3) ) + (led_increment * i);
        if (color < 0) color = color * -1;
        led_status.leds[i].colors.g = color;
        if ((i % 6) < 3) {
            led_status.dir[i].g = -1;
        } else {
            led_status.dir[i].g = 1;
        }
#else
        led_status.leds[i].colors.g = 0;
        led_status.dir[i].g = 0;
#endif

#if 0
        color = -255 + (led_increment * (LED_COUNT / 3) * 2) + (led_increment * i);
        if (color < 0) color = color * -1;
        led_status.leds[i].colors.b = color;
        if ((i % 6) < 3) {
            led_status.dir[i].b = 1;
        } else {
            led_status.dir[i].b = -1;
        }
#else
        led_status.leds[i].colors.b = 0;
        led_status.dir[i].b = 0;
#endif
    }

    led_status.timer = 0;

    ws2812_send(led_status.leds, LED_COUNT);
}

void leds_run(void) {
    int i;

    /* This time has to be at least 40us so that the led string is reset. */
    for(i=0; i<50000; i++){
        __asm("nop");
    }

    if(!ws2812_is_sending()) {
        for(i = 0; i < LED_COUNT; i++) {
            led_status.leds[i].colors.r += led_status.dir[i].r;
            if(led_status.dir[i].r != 0) {
                if(led_status.leds[i].colors.r == 255) led_status.dir[i].r = -1;
                if(led_status.leds[i].colors.r == 0) led_status.dir[i].r = 1;
            }
            led_status.leds[i].colors.g += led_status.dir[i].g;
            if(led_status.dir[i].g != 0) {
                if(led_status.leds[i].colors.g == 255) led_status.dir[i].g = -1;
                if(led_status.leds[i].colors.g == 0) led_status.dir[i].g = 1;
            }
            led_status.leds[i].colors.b += led_status.dir[i].b;
            if(led_status.dir[i].b != 0) {
                if(led_status.leds[i].colors.b == 255) led_status.dir[i].b = -1;
                if(led_status.leds[i].colors.b == 0) led_status.dir[i].b = 1;
            }
        }
        ws2812_send(led_status.leds, LED_COUNT);
    }    
}

void leds_color(int r, int g, int b);
void leds_color(int r, int g, int b)
{
    int i = 0;
    if(!ws2812_is_sending()) {
        for(i = 0; i < LED_COUNT; i++) {
            led_status.leds[i].colors.r = r;
            led_status.leds[i].colors.g = g;
            led_status.leds[i].colors.b = b;
        }
        ws2812_send(led_status.leds, LED_COUNT);
    }    
}

void do_flash(int delay);
void do_flash(int delay)
{
    leds_color(255, 0, 0);
    sleep(delay);
    leds_color(0, 255, 0);
    sleep(delay);
    leds_color(0, 0, 255);
    sleep(delay);
}

void do_send_data_leds(void);
void do_send_data_leds(void)
{
    while(ws2812_is_sending()) 
    {
        sleep(1);
    }

    ws2812_send(led_status.leds, LED_COUNT);
}

void running_led(int r, int g, int b, int width, int delay);
void running_led(int r, int g, int b, int width, int delay)
{
    int i;
    for(i = -1; i < LED_COUNT + width; i++) {
        if (i > i - width){
            led_status.leds[i-width].colors.r = 0;
            led_status.leds[i-width].colors.g = 0;
            led_status.leds[i-width].colors.b = 0;
        }

        led_status.leds[i].colors.r = r;
        led_status.leds[i].colors.g = g;
        led_status.leds[i].colors.b = b;

        do_send_data_leds();
        sleep(delay);
    }

    for (i = LED_COUNT; i >= LED_COUNT - width; i--)
    {
        led_status.leds[i-1].colors.r = 0;
        led_status.leds[i-1].colors.g = 0;
        led_status.leds[i-1].colors.b = 0;
    }

    do_send_data_leds();

}

#define COLOR_BRIGHTNESS 8

int main(void)
{
    clock_setup();

    systick_init();

	uart_init();

	uart_send_str("\n***** Starting (UART) ...\n");

	usb_vcp_init();

	uart_send_str("\n***** Starting (USB) ...\n");
	
    initialize_blinking();

	uart_send_str("\n***** Starting (LED) ...\n");

    led_on();

    ws2812_init();

	uart_send_str("\n***** Starting (WS2812) ...\n");

    leds_init();
   
    led_off();

    running_led(COLOR_BRIGHTNESS, COLOR_BRIGHTNESS, COLOR_BRIGHTNESS, 2, 100);

	uint32_t last_millis = system_millis;
	uint32_t blink = 0;

	uart_send_str("\n***** System started *****\n");
    usb_vcp_printf("\n***** System started *****\n");

	while (1) {

		char buf[128];
		size_t len = 0;
        uint32_t blink_period = 100;
		while (1) {
			if (len >= sizeof(buf)) {
				break;
			}
			if (usb_vcp_avail()) {
				led_toggle();
				char ch = usb_vcp_recv_byte();
				usb_vcp_send_byte(ch);
				if (ch == '\r') {
					usb_vcp_send_byte('\n');
				}
				if (ch == '\r' || ch == '\n') {
					break;
				}
				buf[len++] = ch;
			}

            if (usb_vcp_is_connected()) {
                blink_period = 1000;
            } else {
                do_flash(677);
                blink_period = 100;
            }
            
            if (system_millis - last_millis > blink_period) {
                if (blink <= 3) {
                    led_toggle();
                }
                blink = (blink + 1) % 10;
                last_millis = system_millis;
            }

			__WFI();
		}
		uart_send_strn("Line: ", 6);
		uart_send_strn(buf, len);
		uart_send_strn("\r\n", 2);

		usb_vcp_send_strn("Line: ", 6);
		usb_vcp_send_strn(buf, len);
		usb_vcp_send_strn("\r\n", 2);
	}

    return 0;
}





