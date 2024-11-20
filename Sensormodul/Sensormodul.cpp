/*
 * Sensormodul.cpp
 *
 * Created: 2022-11-02 08:20:26
 *  Author: axeha009
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <string.h>
#include "../Include/types.hpp"

volatile SensorData data = {0, 0, 0, 0};
volatile SensorData to_send;
volatile bool converting_gyro = false;
volatile bool PB1_low = false, PB0_low = false;
volatile uint16_t PB0_count = 0, PB1_count = 0;
volatile float angle = 0.0f;
volatile uint8_t i2c_write_count = 0;

// Trial and error...
#define TIME_FACTOR 0.00000123288
#define ANGLE_THREESHOLD 2

#define SET_BIT_TO(reg, pos, val) reg = ((~(1 << pos)) & reg) | ((val) << pos)

ISR(TWI_vect) {
	uint8_t status = TWSR & 0xF8;
	switch (status) {
		case 0x00: {
			// Clear error
			SET_BIT_TO(TWCR, TWSTO, 1);
			SET_BIT_TO(TWCR, TWEA, 1);
			break;
		}
		case 0xa8: {
			to_send.dist = data.dist;
			to_send.vel_l = data.vel_l;
			to_send.vel_r = data.vel_r;
			to_send.angle = data.angle;
			TWDR = reinterpret_cast<volatile uint8_t*>(&to_send)[i2c_write_count];
			PORTD = reinterpret_cast<volatile uint8_t*>(&to_send)[i2c_write_count];
			++i2c_write_count;
			break;
		}
		case 0xB8: {
			TWDR = reinterpret_cast<volatile uint8_t*>(&to_send)[i2c_write_count];
			PORTD = reinterpret_cast<volatile uint8_t*>(&to_send)[i2c_write_count];
			++i2c_write_count;
			if (i2c_write_count == sizeof(SensorData)) {
				SET_BIT_TO(TWCR, TWEA, 0);
			}
			break;
		}
		case 0xC0: {
			SET_BIT_TO(TWCR, TWEA, 1);
			i2c_write_count = 0;
			break;
		}
		case 0xC8: {
			SET_BIT_TO(TWCR, TWEA, 1);
			i2c_write_count = 0;
			break;
		}
	}
	SET_BIT_TO(PORTB, PORTB2, status & 1);
	SET_BIT_TO(PORTB, PORTB3, (status >> 1) & 1);
	SET_BIT_TO(PORTB, PORTB4, (status >> 2) & 1);
	SET_BIT_TO(PORTB, PORTB5, (status >> 3) & 1);
	SET_BIT_TO(PORTB, PORTB6, (status >> 4) & 1);
	SET_BIT_TO(PORTB, PORTB7, (status >> 5) & 1);
	SET_BIT_TO(PORTC, PORTC6, (status >> 6) & 1);
	SET_BIT_TO(PORTC, PORTC7, (status >> 7) & 1);
	SET_BIT_TO(TWCR, TWINT, 1);	
}


ISR(ADC_vect) {
	
	// sei()?
	uint16_t result = 0;
	result = ADC;


	if (converting_gyro) { 
		// Most significant bit unused?
		// 300 deg/s max angular velocity.
		// Change input pin to ADC0
		uint16_t time = 0;
		time = TCNT3;
		TCNT3 = 0;
		converting_gyro = false;
		ADMUX = (1 << REFS0) | (0 << REFS1) |  (0 << ADLAR) | (0 << MUX2) | (0 << MUX1) |(0 << MUX0);

		float res = ((float)result) - 512.0;
		

		if (res >= ANGLE_THREESHOLD || res <= -ANGLE_THREESHOLD) {
			float vel = (300.0 / 512.0) * res;
			angle += vel * time * TIME_FACTOR;
			if (data.angle > 360.0) data.angle -= 360.0;
			if (data.angle < -360.0) data.angle += 360.0;
			data.angle = angle;
		}
		
	} else {
		
		// Change input pin to ADC1
		converting_gyro = true;
		ADMUX = (1 << REFS0) | (0 << REFS1) |  (0 << ADLAR) | (1 << MUX2) | (1 << MUX1) |(1 << MUX0);
		
		data.dist =  85 * exp(-0.004 * result);
		
	}
	ADCSRA |= (1 << ADSC); // Start next AD-conversion.
	
}

ISR(PCINT1_vect) {
	if (((PINB >> PINB0) & 1) == 0) {
		if (!PB0_low) {
			++PB0_count;
			PB0_low = true;
		}
	} else {
		PB0_low = false;
	}
	if (((PINB >> PINB1) & 1) == 0) {
		if (!PB1_low) {
			++PB1_count;
			PB1_low = true;
		}
	} else {
		PB1_low = false;
	}
}

ISR(TIMER1_COMPA_vect) {
	data.vel_r = PB0_count;
	data.vel_l = PB1_count;
	PB0_count = 0;
	PB1_count = 0;
}

void I2C_init() {
	TWAR = (1 << TWA0);
	TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
}

// Initialize the AD converter
void ADC_init() {
	
	//         Reference 5V AVCC    Right adjust     Single ended input from ADC0
	ADMUX = (1 << REFS0) | (0 << REFS1) |  (0 << ADLAR)   |        (0 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // Enable ADC + ADC interrupts.
	//    Ignore digital input on ADC1 and ADC0
	DIDR0 = (1 << ADC1D) | (1 << ADC0D);
}

// Initialize counter/timer3
void CT_init() {
	// Start timer3 with prescaler to 8.
	TCCR3B = (0 << CS32) | (1 << CS31) | (0 << CS30);

	// Start timer1 in Clear on Compare match mode, with prescaler to 256.
	TCCR1B = (1 << WGM12) | (0 << WGM13) | (1 << CS12) | (0 << CS11) | (0 << CS10);
	// Interrupt enable for timer 1 compare match A.
	TIMSK1 |= (1 << OCIE1A);
	// Set compare match value to trigger interrupt twice per second.
	OCR1A = 15625;
}

// Initialize pin change interrupt
void PCI_init() {
	// Enable pin change interrupt for pin (8:15).
	PCICR = (1 << PCIE1);
	// Configure pin change interrupt to trigger on pin PCINT8 (PB0) and PCINT9 (PB1).
	PCMSK1 = (1 << PCINT8) | (1 << PCINT9);	
}


int main(void){
	ADC_init();
	
	CT_init();
	
	PCI_init();
	
	I2C_init();
	
	DDRD = 0b11111111;
	DDRB |= (1 << DDB2) | (1 << DDB3) | (1 << DDB4) | (1 << DDB5) | (1 << DDB6) | (1 << DDB7);
	DDRC |= (1 << DDC6) | (1 << DDC7);
	
	PORTD = sizeof(SensorData);
	sei();

	ADCSRA |= (1 << ADSC); // Start next AD-conversion.

    while(1){
		
    }
}