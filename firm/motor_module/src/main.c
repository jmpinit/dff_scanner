#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Serial.h"
//#include "Motor.h"

#define forever for(;;)

int main(void) {
    uart_init(27); // 19200 baud

    sei();

    forever {
        uart_tx_str("hello world!\r\n");
        _delay_ms(1000);
    }

    return 0;
}
