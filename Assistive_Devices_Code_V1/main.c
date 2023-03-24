#ifndef F_CPU
#define F_CPU 16000000ul
#endif

#include "h_bridge.h"
#include "servo.h"

#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

//Bleutooth
#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD)-1)//buad rate calculator

#define DEBOUNCE	_delay_ms(20)		// debounce delay voor de knoppen

//schakelaars
#define home_switch         PA0
#define top_switch          PA1
#define middel_switch       PA5

//bediningpaneel
#define home_knop           PF0			// pin A0
#define opbouw_knop         PF1			// pin A1
#define afbouw_knop         PF2			// pin A2
#define volgende_steiger    PF3			// pin A3
#define vorige_steiger      PF5			// pin A5
#define nood_knop	    PF4			// pin A4

int btgetal = 999;//BLUETOOTH GETAL

void init_bluetooth(void)
{
	UBRR0H = (BRC >> 8);
    	UBRR0L = BRC;
    	UCSR0B = (1 << RXEN0) |(1 << RXCIE0);
    	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void init(void)
{
	init_h_bridge();									// init h brug
	init_servo();										// init servo
	init_bluetooth();
	sei();											// eneble global interrupts
}

//Bluetooth interrupt
ISR(USART0_RX_vect)
{
    btgetal = UDR0;
}

int main(void)
{
	init();				// run init function

	// output
	DDRK |= (1<<PK7) | (1<<PK6);                        // knipperlichten pin A15 en A14
	PORTK &= ~(1<<PK7) & ~(1<<PK6);                     // knipperlichten pin A15 en A14 init uit
	DDRD |= (1<<PD0);                                   // buzzer
	PORTD &= ~(1<<PD0);                                 // buzzer init uit
	DDRF |= (1<<PF7);                                   // ledje in de noodknop
	PORTF |= (1<<PF7);                                  // ledje in de noodknop init uit
}
	//bedingingpaneel
	PORTF |= (1<<home_knop) | (1<<opbouw_knop) | (1<<afbouw_knop) | (1<<volgende_steiger) | (1<<vorige_steiger);   		// eneble alle knoppen voor input
	DDRF |= (1<<home_knop) | (1<<opbouw_knop) | (1<<afbouw_knop) | (1<<volgende_steiger) | (1<<vorige_steiger);		// eneble interne pullup resistor

	//schakelaars
	PORTA &= (1<<home_switch);
	PORTA &= (1<<top_switch);
	PORTA &= (1<<middel_switch);
	int status = 0;						// status voor de switch case statement


	while (1)
	{
		switch(btgetal)
	    {
        case 0:
            PORTB &= ~(1 << PK7);
            break;
        case 1:
            PORTB |= (1 << PK7);
            break;
        case 2:
            PORTB &= ~(1 << PK6);
            break;
        case 3:
            PORTB |= (1 << PK6);
	    }
		
		switch(status)
		{
        case 0:
            if(!(PINA & (1<<home_switch)))
            {
		DEBOUNCE;
                status = 2;
                break;
            }
            if (PINA &(1<<home_switch))
            {
		DEBOUNCE;
                if(!(PINF & (1<<home_knop)))
                {
			DEBOUNCE;
			status = 1;
			break;
                }
            }
            break;
        case 1:
            if(!(PINA & (1<<home_switch)))
            {
		DEBOUNCE;
                status = 0;
                break;
            }
            h_bridge_set_percentage(-70);
            break;
        case 2:
            if(!(PINF & (1<<opbouw_knop)))
            {
		DEBOUNCE;
                status = 3;
                break;
            }
            if(!(PINF & (1<<afbouw_knop)))
            {
		DEBOUNCE;
                status = 4;
                break;
            }
            break;

            //Opbouwen
        case 3:
            servo1_set_percentage(50);
            servo2_set_percentage(50);
            status = 5;
            break;
        case 5:
            if (!(PINA & (1<<top_switch)))
            {
		DEBOUNCE;
                h_bridge_set_percentage(0);
                status = 7;
                break;
            }
            h_bridge_set_percentage(70);
            break;
        case 7:
            if(!(PINF & (1<<volgende_steiger)))
            {
		DEBOUNCE;
                status = 9;
            }
            break;
        case 9:
            if(!(PINA & (1<<middel_switch)))
               {
			DEBOUNCE;
			h_bridge_set_percentage(0);
			status = 11;
			break;
               }
               h_bridge_set_percentage(-70);
               break;
        case 11:
            servo1_set_percentage(-50);
            servo2_set_percentage(-50);
            status = 13;
            break;
        case 13:
            if (!(PINA & (1<<home_switch)))
            {
		DEBOUNCE;
                h_bridge_set_percentage(0);
                status = 2;
                break;
            }
            h_bridge_set_percentage(-70);
            break;
        case 4:
            if (!(PINA &(1<<middel_switch)))
            {
		DEBOUNCE;
                h_bridge_set_percentage(0);
                status = 6;
                break;
            }
            h_bridge_set_percentage(70);
            break;
        case 6:
            servo1_set_percentage(50);
            servo2_set_percentage(50);
            status = 8;
            break;
        case 8:
            if(!(PINA & (1<<top_switch)))
            {
		DEBOUNCE;
                h_bridge_set_percentage(0);
                status = 10;
                break;
            }
            h_bridge_set_percentage(70);
            break;
        case 10:
            if(!(PINF &(1<<vorige_steiger)))
            {
		DEBOUNCE;
                status = 12;
            }
            break;
        case 12:
            if(!(PINA & (1<<home_switch)))
            {
		DEBOUNCE;
                h_bridge_set_percentage(0);
                status = 14;
                break;
            }
            h_bridge_set_percentage(-70);
        case 14:
            servo1_set_percentage(-50);
            servo2_set_percentage(-50);
            status = 2;
            break;

		}
	}

	return 0;
}
