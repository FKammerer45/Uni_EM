/*
 * Blatt3Aufg1Labor.c
 *
 * Created: 02.12.2021 10:08:43
 *  Author: kaf43138
 */ 

#define F_CPU 16000000UL
#define EVENT_TIMER50ms1 0x01
#define EVENT_TIMER50ms2 0x02
#define EVENT_TIMER1000ms 0x04

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "events.h"

typedef enum {false, true}bool ;
volatile bool run = true;
volatile uint8_t counter = 0;
volatile uint8_t recounter = 0x7;
	

ISR(PCINT2_vect){// PCINT16
	
	counter = recounter;
	//PORTB |= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2);
	
}

ISR(TIMER0_OVF_vect){
	static uint16_t counter = 0;
	TCNT0 = 6;
	counter++;
	if(!(counter%50)){
		
		setEvent(EVENT_TIMER50ms1);
		setEvent(EVENT_TIMER50ms2);
	}
	if (counter==1000){
		counter= 0;
		setEvent(EVENT_TIMER1000ms);
	}
	
}

int main(void)
{	DDRD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6));//input d2 d3 taster, d4 d5 d6 fuer DIP-schalter 
	PORTD |= (1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6); //pull-up d2 d3 d4 d5 d6
	
	DDRB |= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2)|(1<<PORTB3);//output b0 b1 b2 , interrupt b3
	DDRD|= (1<<PORTD0); //interrupt d0
	//interrupts von aufg2.2
	/*PORTC |= (1<<PORTC0); //pull-up c0
	EICRA &= ~((1<<ISC01)|(1<<ISC00));
	EICRA |= (1<<ISC01);
	EIMSK |= (1<<INT0);
	
	PCMSK0 &= 0;
	PCMSK1 &= 0;
	PCMSK1 |= (1<<PCINT8);
	PCMSK0 |= (1<<PCINT3);
	
	PCICR |=(1<< PCIE1)|(1<< PCIE0);*/
	
	//software int
	//pcint 3
	PCICR |= (1<<PCIE0);
	PCMSK0 |= (1<<PCINT3);
	
	//pcint16
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<PCINT16);
	//timer config
	TCNT0 = 6;
	TCCR0B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
	TCCR0B |= (1<<CS01)|(1<<CS00);
	TIMSK0 |= (1<<TOIE0);
	sei();
	
	bool previous1 = true, previous2 = true; 
	
	
    while (1) 
    {	if(run){
			if(eventIsSet(EVENT_TIMER1000ms)){
				
				PORTB = (~counter & 0x07);
				/*if (counter == 0){counter = recounter;}
				else{counter--;}*/
				counter--;
				clearEvent(EVENT_TIMER1000ms);
				
				if(counter <= 0){ //Software Interrupt auslösen
					PORTD ^= (1<<PORTD0);
					_delay_ms(1);
					PORTD ^= (1<<PORTD0);
				}
			}
			if(eventIsSet(EVENT_TIMER50ms1)){
				if(PIND & (1<<PIND3)){
					if(!previous1){
						run = false;
						recounter = 0;
						if(PIND & (1<<PIND4))recounter |= 0x01;
						if(PIND & (1<<PIND5))recounter |= 0x02;
						if(PIND & (1<<PIND6))recounter |= 0x04;
						counter = recounter;
					}
					previous1 = true;
				}
				else previous1=false;
			}
			clearEvent(EVENT_TIMER50ms1);
		}
		
		
		if(!run){
			if(eventIsSet(EVENT_TIMER50ms2)){
				if(PIND & (1<<PIND2)){
					if(!previous2){
						run= true;
						
					}
					previous2 = true;
				}
				else previous2=false;
			}
			clearEvent(EVENT_TIMER50ms2);
		}
		
		
		
		_delay_ms(100);
    }
}