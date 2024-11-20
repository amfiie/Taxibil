/*
 * Styrmodul.cpp
 *
 * Created: 2022-11-02 10:29:40
 *  Author: axeha0
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "../Include/types.hpp"

#define IO_CLOCK_SPEED 8000000
#define TIMER_3_TARGET_SPEED 50
#define TIMER_3_TOP IO_CLOCK_SPEED / (8 *  TIMER_3_TARGET_SPEED * 2)
// Convert pulse with in microseconds to appropriate value.
#define PULSE_WIDTH(w) TIMER_3_TOP / 10000 * (w / 2)
#define SET_BIT_TO(reg, pos, val) reg = ((~(1 << pos)) & reg) | ((val) << pos)

volatile SteeringData data = {Direction::FORWARDS, 1000, 1500, false};                                                                                                                                                      
volatile uint8_t i2c_read_count = 0;

ISR(TWI_vect) {
	uint8_t status = TWSR & 0xF8;
	switch (status) {
		case 0x60: {
			i2c_read_count = 0;
			break;
		}
		case 0x80: {
			reinterpret_cast<volatile uint8_t*>(&data)[i2c_read_count] = TWDR;
			++i2c_read_count;
			break;
		}
		case 0xA0: {
			// Deal with potentialy missed on / off if timer misses compare?
			OCR3A = PULSE_WIDTH(data.turn); // Turn
			PORTD = data.turn;
			OCR3B = PULSE_WIDTH(data.intensity); // Intensity
			SET_BIT_TO(PORTB, PORTB0, static_cast<uint8_t>(data.is_breaking));
			SET_BIT_TO(PORTB, PORTB1, static_cast<uint8_t>(data.direction));
			break;
		}
	}
	SET_BIT_TO(TWCR, TWINT, 1);	
}

void I2C_init() {
	// Enable TWI in slave mode.
	TWAR = (1 << TWA1);
	TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
}

/*
 * 
 * Use phase correct PWM, clock_io, prescale factor 8, TOP in ICR3 = 12500
 */
void PWM_init() {
	TCCR3A = (0 << COM3A0) | (1 << COM3A1) | (0 << COM3B0) | (1 << COM3B1) | (1 << WGM31) | (0 << WGM30); // page 132, table 14-4
	TCCR3B = (0 << WGM32) | (1 << WGM33) | (0 << CS32) | (1 << CS31) | (0 << CS30);
	
	ICR3 = TIMER_3_TOP;
	
	OCR3A = PULSE_WIDTH(data.turn); // Turn
	OCR3B = PULSE_WIDTH(data.intensity); // Intensity
	
	// Enable writing to PB6 + PB7
	DDRB |= (1 << DDB6) | (1 << DDB7);
}

int main(void)
{	
	I2C_init();
	PWM_init();
	
	DDRB |= (1 << DDB0) | (1 << DDB1);
	DDRD = 0b11111111;
	// PB0 = Break (active high), PB1 = Dir (0 = forwards, 1 = backwards)
	PORTB |= (data.is_breaking << PORTB0) | (static_cast<uint8_t>(data.direction) << PORTB1);

	PORTD = 0b10101010;
	sei();
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}