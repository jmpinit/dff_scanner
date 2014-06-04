#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Serial.h"
#include "Motor.h"

#define forever for(;;)

int main(void) {
    uart_init(25); // 19200 baud
    motor_init();

    sei();

    int dir = 1;
    forever {
        while(uart_available()) {
            char c = uart_read_buff();
            
            switch(c) {
                case '-':
                    dir = -1;
                    break;
                case '+':
                    dir = 1;
                    break;

                case '1':
                    motor_move(dir);
                    break;
                case '2':
                    motor_move(dir*10);
                    break;
                case '3':
                    motor_move(dir*100);
                    break;
                case '4':
                    motor_move(dir*1000);
                    break;
                case '5':
                    motor_move(dir*10000);
                    break;
                case '6':
                    motor_move(dir*100000);
                    break;
            }
        }
    }

    return 0;
}
