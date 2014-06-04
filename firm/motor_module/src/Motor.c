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

#define PIN_ON(port,pin) (port) |= 1 << (pin);
#define PIN_OFF(port,pin) (port) &= ~(1 << (pin));

long position = 0;
uint8_t speed = 255;

// quadrature encoder interrupt
volatile uint8_t hist_portb = 0xFF;

ISR(PCINT0_vect) {
    uint8_t changed;

    changed = PINB ^ hist_portb;
    hist_portb = PINB;

    if(changed & (1 << PB0)) { // pb0 changed
        // dx 
        if(PINB & (1 << PB0)) { // pb0 rising edge
            if(PINB & (1 << PB1)) { // pb1 high or low?
                position++;
            } else {
                position--;
            }
        }
    }
}

int abs(int n) {
    if(n < 0)
        return -n;
    else
        return n;
}

void motor_init() {
    // pin change interrupt
    // for quadrature encoder
    PCICR = 1 << PCIE0;
    PCMSK0 = 1 << PCINT0;

    // motor control pins
    DDRD |= 1 << PIN_M_EN;
    DDRD |= (1 << PIN_M_1A) | (1 << PIN_M_2A);
    motor_disable();

    // pwm using timer0
    // phase correct, OCR0A top, no prescale
    TCCR0A = (1 << COM0A0) | (1 << COM0A1) | (1 << WGM00);
    TCCR0B = (1 << CS00);

    OCR0A = 255; // off
    OCR0B = 255; // off
}

long motor_get_pos(void) {
    return position;
}

void motor_set_speed(uint8_t s) {
    speed = s;
}

void motor_move(int32_t ticks) {
    long start = position;
    
    motor_enable();
    
    if(ticks > 0) {
        OCR0A = 255 - speed;
        OCR0B = 255;
    } else {
        OCR0A = 255;
        OCR0B = 255 - speed;
    }

    char buffer[32];
    while(abs(position - start) < ticks) {
        sprintf(buffer, "%d < %d\n", abs(position - start), ticks);
        uart_tx(buffer);
        asm volatile("NOP;");
    }

    motor_brake();
}

void motor_enable() {
    PIN_ON(PORTD, PIN_M_EN);
}

void motor_disable() {
    PIN_OFF(PORTD, PIN_M_EN);
}

void motor_brake() {
    motor_enable();
    PIN_OFF(PORTD, PIN_M_1A);
    PIN_OFF(PORTD, PIN_M_2A);
}
