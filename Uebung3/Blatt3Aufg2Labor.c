/*
 * Blatt3Aufg2Labor.c
 *
 * Created: 02.12.2021 12:20:58
 *  Author: kaf43138
 */ 


#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "events.h"
#include "timer.h"
#define EVENT_TIMER_50ms 0x01
#define EVENT_TIMER_1s 0x02
#define TIMER_50ms 0x00
#define TIMER_1s 0x01

volatile bool run = true;
volatile uint8_t counter = 0;
volatile uint8_t recounter = 0x7;
volatile uint64_t anzahl_micro_secs= 0;


void test(){
	PORTB = 0x0;
	_delay_ms(1000);
	PORTB = 0x07;
}
ISR(PCINT2_vect){// PCINT16
	
	counter = recounter;
	//PORTB |= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2);
	
}

ISR(TIMER1_OVF_vect){
	
	TCNT1 = 49536;
	anzahl_micro_secs++;
	for(int i = 0;i<5;i++){
		if(!stoppuhren[i].canceled){
			if(stoppuhren[i].started+stoppuhren[i].laufende_zeit<=anzahl_micro_secs){
				//stoppuhren[i].callback();
				setEvent(stoppuhren[i].ev);
				stoppuhren[i].started=anzahl_micro_secs;
				
			}	
		}
	}
}

void set_50MS_event()
{
	
	setEvent(EVENT_TIMER_50ms);
}

void set_1s_event()
{
	setEvent(EVENT_TIMER_1s);
}


int main(void)
{	DDRD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6));//input d2 d3 taster, d4 d5 d6 fuer DIP-schalter 
	PORTD |= (1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6); //pull-up d2 d3 d4 d5 d6
	
	DDRB |= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2);//output b0 b1 b2 , 
	DDRD|= (1<<PORTD0); //interrupt d0
	
	//pcint16
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<PCINT16);
	
	sei();
	
	bool previous1 = true, previous2 = true; 
	inittimer();
	declareTimer(TIMER_50ms,50, set_50MS_event,EVENT_TIMER_50ms );
	declareTimer(TIMER_1s,1000, set_1s_event,EVENT_TIMER_1s );
	startTimer(TIMER_50ms);
	startTimer(TIMER_1s);
    while (1) 
    {	
		if(run){
			if(eventIsSet(EVENT_TIMER_1s)){
				
				PORTB = (~counter & 0x07);
				/*if (counter == 0){counter = recounter;}
				else{counter--;}*/
				counter--;
				clearEvent(EVENT_TIMER_1s);
				
				if(counter <= 0){ //Software Interrupt auslösen
					PORTD ^= (1<<PORTD0);
					_delay_ms(1);
					PORTD ^= (1<<PORTD0);
				}
			}
			if(eventIsSet(EVENT_TIMER_50ms)){
				
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
			clearEvent(EVENT_TIMER_50ms);
		}
		
		
		if(!run){
			if(eventIsSet(EVENT_TIMER_50ms)){
				
				if(PIND & (1<<PIND2)){
					if(!previous2){
						run= true;
						
					}
					previous2 = true;
				}
				else previous2=false;
			}
			clearEvent(EVENT_TIMER_50ms);
		}
		
		
		
		_delay_ms(100);
    }
}