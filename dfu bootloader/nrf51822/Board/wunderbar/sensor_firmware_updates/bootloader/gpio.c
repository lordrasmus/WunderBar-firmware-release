#include "gpio.h"

void gpio_set_pin_digital_input(uint8_t pin, PIN_PULL pull_mode) {
	GPIO0->PIN_CNF[pin] = (pull_mode << 2);
}

void gpio_set_pin_digital_output(uint8_t pin, PIN_DRIVE drive_mode) {
	GPIO0->PIN_CNF[pin] = (1 | (drive_mode << 8));
}

void gpio_disconnect_pin(uint8_t pin) {
	GPIO0->PIN_CNF[pin] = 2;	//reset to input, disconnected
}

bool gpio_read (uint8_t pin) {
	return (GPIO0->IN & (1 << pin)) ? true : false;
}

void gpio_write (uint8_t pin, bool value) {
	*(value ? &(GPIO0->OUTSET) : &(GPIO0->OUTCLR)) = 1 << pin;
}
