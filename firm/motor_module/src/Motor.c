#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "Motor.h"

#define CLK_NONE	1
#define CLK_8		2
#define CLK_64		3
#define CLK_256		4
#define CLK_1024	5

#define PIN_OFF(port,pin) (port) |= 1 << (pin);
#define PIN_ON(port,pin) (port) &= ~(1 << (pin));

#define MOTOR_DISABLE() PIN_OFF(PORTD, PIN_M_EN);
#define MOTOR_ENABLE() PIN_ON(PORTD, PIN_M_EN);

uint16_t dt_i = 0;
uint32_t pos_i = 0;

// quadrature encoder interrupt
volatile uint8_t hist_portb = 0xFF;

volatile long overflows = 0;
ISR(TIMER1_COMPA_vect) {
	/*switch(motor.mode) {
		case POSITION:
			motor.target_vel = pid_calc(&motor.pid, motor.pos, motor.target_pos);
			break;
		case VELOCITY:
			break;
	}*/

	float correction = 0; //pid_calc(&motor.pid, motor.vel, motor.target_vel);
	if(motor.vel > motor.target_vel) {
		correction = -1;
	} else {
		correction = 1;
	}

	if(motor.target_vel < 0) {
		TCCR0A &= ~(1 << COM0B0);
		TCCR0A &= ~(1 << COM0B1);
		TCCR0A |= (1 << COM0A0);
		TCCR0A |= (1 << COM0A1);

		//OCR0A = 128;

		/*if() {
			
		} else {

		}*/
		float new = OCR0A - correction;
		if(new > 255) new = 255;
		if(new < 0) new = 0;
		OCR0A = new;
	} else {
		TCCR0A &= ~(1 << COM0A0);
		TCCR0A &= ~(1 << COM0A1);
		TCCR0A |= (1 << COM0B0);
		TCCR0A |= (1 << COM0B1);

		OCR0B = 128;
		/*if() {

		} else {

		}*/
		float new = OCR0B - correction;
		if(new > 255) new = 255;
		if(new < 0) new = 0;
		OCR0B = new;
	}
	
	// calc velocity
	motor.vel = motor.pos - pos_i;
	pos_i = motor.pos;

	//overflows++;
}

ISR(PCINT0_vect) {
	uint8_t changed;

	changed = PINB ^ hist_portb;
	hist_portb = PINB;

	if(changed & (1 << PB0)) { // pb0 changed
		// dx 
		if(PINB & (1 << PB0)) { // pb0 rising edge
			if(PINB & (1 << PB1)) { // pb1 high or low?
				motor.pos--;
			} else {
				motor.pos++;
			}
		}
		
		/*
		// dt 
		uint16_t dt_f = (TCNT1 & 0x7FFF);
		double dt = dt_f - dt_i;
		if(dt < 0) {
			// we did not go back in time
			dt += overflows * 0x7FFF;
			overflows = 0;
		}
		dt_i = dt_f;

		// velocity
		if(dt != 0) motor.vel = 0x7FFF * (motor.pos - pos_i) / dt;
		pos_i = motor.pos;
		*/
	}
}

float pid_calc(PidData* prefs, float current, float target) {
	float error = target - current;

	if(fabs(error) > prefs->epsilon)
		prefs->integral = prefs->integral + error*prefs->dt;
	float derivative = (error - prefs->pre_error)/prefs->dt;
	float output = prefs->Kp*error + prefs->Ki*prefs->integral + prefs->Kd*derivative;

	// update error
	prefs->pre_error = error;

	return output;
}

void pid_reset() {
	motor.pid.integral	= 0.0;
	motor.pid.pre_error	= 0.0;
}

void motor_init() {
	motor.mode			= VELOCITY;
	motor.target_vel	= 0;

	motor.pid.Kp		= 3.0;
	motor.pid.Kd		= 0.1;
	motor.pid.Ki		= 0.014;

	/*motor.pid.Kp		= 1.0;
	motor.pid.Kd		= 1.0;
	motor.pid.Ki		= 0.0;*/

	// pin change interrupt
	// for quadrature encoder
	/*PCICR = 1 << PCIE0;
	PCMSK0 = 1 << PCINT0;*/

	// motor control pins
	DDRD |= 1 << PIN_M_EN;
	DDRD |= (1 << PIN_M_1A) | (1 << PIN_M_2A);
	MOTOR_DISABLE();

	// setup 16 bit timer
	TCCR1B |= CLK_8;
	TIMSK1 |= 1 << OCIE1A; // interrupt on OCR1A compare match
	OCR1A = 0xFFF;

	// pwm using timer0
	// phase correct, OCR0A top, no prescale
	TCCR0A = (1 << COM0A0) | (1 << COM0A1) | (1 << WGM00);
	TCCR0B = (1 << CS00);

	OCR0A = 255; // off
	OCR0B = 255; // off
}

void motor_set_pos(uint32_t pos) {
	motor.mode = POSITION;
	motor.target_pos = pos;
}

void motor_set_vel(int32_t vel) {
	motor.mode = VELOCITY;
	motor.target_vel = vel;
	MOTOR_ENABLE();
}

void motor_stop() {
	pid_reset();
	PIN_OFF(PORTD, PIN_M_1A);
	PIN_OFF(PORTD, PIN_M_2A);
}

void motor_brake() {
	MOTOR_DISABLE();

	pid_reset();
	PIN_OFF(PORTD, PIN_M_1A);
	PIN_OFF(PORTD, PIN_M_2A);
}
