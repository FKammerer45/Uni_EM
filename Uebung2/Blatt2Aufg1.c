/*
 * Blatt2Aufg1.c
 *
 * Created: 17.11.2021 16:50:18
 * Author : kamme
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

typedef enum {false, true}bool ;
volatile bool run = true;
volatile int counter = 0;
volatile int recounter = 0x7;
	
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

int main(void)
{	DDRD &= ~((1<<PORTD2)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6));//input d2 d4 d5 d6 fuer DIP-schalter
	PORTD |= (1<<PORTD2)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6); //pull-up d2 d4 d5 d6
	DDRB |= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2);//output b0 b1 b2
	PORTC |= (1<<PORTC0);
	EICRA &= ~((1<<ISC01)|(1<<ISC00));
	EICRA |= (1<<ISC01);
	EIMSK |= (1<<INT0);
	
	
	PCMSK1 &= 0;
	PCMSK1 |= (1<<PCINT8);
	PCICR |=(1<< PCIE1);
	sei();
	
	
	
    while (1) 
    {	
		if(run){
			PORTB = (~counter & 0x07);
			counter--;
			if(counter <=0)counter = recounter;
		
		}
		
		_delay_ms(1000);
    }
}

                                                                                                               