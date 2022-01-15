/*
 * Blatt2Aufg2.c
 *
 * Created: 25.11.2021 11:30:30
 *  Author: kaf43138
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

typedef enum {false, true}bool ;
volatile bool run = true;
volatile uint8_t counter = 0;
volatile uint8_t recounter = 0x7;


void test(){
	PORTB = 0x0;
	_delay_ms(1000);
	PORTB = 0x07;
}
	
ISR(INT0_vect)			//PD2
{	
	
	run = true;
}

//PCINT1  -> PCINT14 bisPCINT8  , PC0-> PCINT8
ISR(PCINT1_vect){
	
	run = false;
	recounter = 0;
	if(PIND & (1<<PIND4))recounter |= 0x01;
	if(PIND & (1<<PIND5))recounter |= 0x02;
	if(PIND & (1<<PIND6))recounter |= 0x04;
	counter = recounter;
}

ISR(PCINT2_vect){// PCINT16
	
	counter = 0x7;
	
}


int main(void)
{	DDRD &= ~((1<<PORTD2)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6));//input d2 d4 d5 d6 fuer DIP-schalter
	PORTD |= (1<<PORTD2)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6); //pull-up d2 d4 d5 d6
	PORTB |= (1<<PORTB3); //pullup b3 software inter
	
	DDRB |= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2);//output b0 b1 b2
	PORTC |= (1<<PORTC0);						//pull-up c0
	
	EICRA &= ~((1<<ISC01)|(1<<ISC00));
	EICRA |= (1<<ISC01);
	EIMSK |= (1<<INT0);
	
	PCICR |= (1<<PCIE0);
	PCMSK0 &= 0;
	PCMSK1 &= 0;
	PCMSK1 |= (1<<PCINT8);
	PCMSK0 |= (1<<PCINT3); //pb3 software int
	
	PCICR |=(1<< PCIE1)|(1<< PCIE0);;
	sei();
	
	
	
    while (1) 
    {	
		if(run){
			
			if(counter &0x04)PORTB&=~(1<<PORTB2);
			else PORTB|=(1<<PORTB2);
			if(counter &0x02)PORTB&=~(1<<PORTB1);
			else PORTB|=(1<<PORTB1);
			if(counter &0x01)PORTB&=~(1<<PORTB0);
			else PORTB|=(1<<PORTB0);
		
			counter--;
			if(counter == 0){ //Software Interrupt auslösen
				
				PORTB ^= (1<<PORTB3);
				_delay_ms(1);
				PORTB ^= (1<<PORTB3);
			}
		
		}
		
		_delay_ms(1000);
    }
}

                                                                                                               