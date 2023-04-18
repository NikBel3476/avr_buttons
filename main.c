#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUTTON1 PE0
#define BUTTON2 PE1
#define BUTTON3 PE2
#define BUTTON4 PE3
#define RESET_BUTTON PE4
#define LED_YELLOW PE5
#define LED_GREEN PE6
#define LED_RED PF1
#define LOCK PF0
#define ATTEMPTS_LIMIT 2

volatile uint8_t is_locked = 1;
volatile uint8_t led_red_on = 1;

int main ()
{
	TCCR1A = 0;
	TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10);
	OCR1A = 0x3D09;
	TCNT1 = 0;
	TIMSK1 = (1 << OCIE1A);

	SREG |= (1 << 7);

	DDRE &= 0;
	DDRE |= 0b11100000;
	PORTE &= 0;

	DDRF &= 0;
	DDRF |= 1 << LED_RED;
	PORTF &= 0;

	uint8_t secret_code = (1 * 8 / 4) + 4;
	uint8_t code = 0;
	uint8_t attempts = 0;
	uint8_t click_counter = 0;

	uint8_t button1_ticks_counter = 0;
	uint8_t button2_ticks_counter = 0;
	uint8_t button3_ticks_counter = 0;
	uint8_t button4_ticks_counter = 0;
	uint8_t reset_button_ticks_counter = 0;

	while (1) {
		if (!is_locked) {
			// button 1
			if ((PINE & (1 << BUTTON1)) == 0) {
				_delay_ms(10);
				if (button1_ticks_counter == 0) {
					code = 1;
					click_counter++;
				}
				if (button1_ticks_counter < 10) {
					button1_ticks_counter++;
				}
			} else {
				button1_ticks_counter = 0;
			}

			// button 2
			if ((PINE & (1 << BUTTON2)) == 0) {
				_delay_ms(10);
				if (button2_ticks_counter == 0) {
					code *= 8;
					click_counter++;
				}
				if (button2_ticks_counter < 10) {
					button2_ticks_counter++;
				}
			} else {
				button2_ticks_counter = 0;
			}

			// button 3
			if ((PINE & (1 << BUTTON3)) == 0) {
				_delay_ms(10);
				if (button3_ticks_counter == 0) {
					code += 4;
					click_counter++;
				}
				if (button3_ticks_counter < 10) {
					button3_ticks_counter++;
				}
			} else {
				button3_ticks_counter = 0;
			}

			// button 4
			if ((PINE & (1 << BUTTON4)) == 0) {
				_delay_ms(10);
				if (button4_ticks_counter == 0) {
					code /= 4;
					click_counter++;
				}
				if (button4_ticks_counter < 10) {
					button4_ticks_counter++;
				}
			} else {
				button4_ticks_counter = 0;
			}
		}

		// reset button
		if ((PINE & (1 << RESET_BUTTON)) == 0) {
			_delay_ms(10);
			if (reset_button_ticks_counter == 0) {
				click_counter = 0;
				attempts = 0;
				code = 0;
				PORTE &= ~(1 << LED_GREEN);
				PORTF &= ~(1 << LED_RED);
			}
			if (reset_button_ticks_counter < 10) {
				reset_button_ticks_counter++;
			}
		} else {
			reset_button_ticks_counter = 0;
		}

		if (click_counter == 4) {
			if (code == secret_code) {
				PORTE |= 1 << LED_GREEN;
				PORTF &= ~(1 << LED_RED);
				attempts = 0;
				click_counter = 0;
			} else {
				attempts++;
				click_counter = 0;
				if (attempts == ATTEMPTS_LIMIT) {
					PORTF |= 1 << LED_RED;
					PORTE &= ~(1 << LED_GREEN);
					attempts = 0;
				}
			}
			code = 0;
		}

		// lock handler
		if ((PINF & (1 << LOCK)) == 0) {
			is_locked = 1;
			click_counter = 0;
			attempts = 0;
			code = 0;
			PORTF &= ~(1 << LED_RED);
			PORTE &= ~(1 << LED_GREEN);
		} else {
			is_locked = 0;
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	if (is_locked) {
		if (led_red_on) {
			PORTE &= ~(1 << LED_YELLOW);
			led_red_on = 0;
		} else {
			PORTE |= 1 << LED_YELLOW;
			led_red_on = 1;
		}
	} else {
		PORTE &= ~(1 << LED_YELLOW);
		led_red_on = 0;
	}
	TCNT1 = 0;
}
