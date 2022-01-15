/*
 * Blatt1Aufg3final.cpp
 *
 * Created: 11.11.2021 10:28:40
 *  Author: kaf43138
 */ 



#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
typedef enum {false, true}bool ;
	


int main(void)
{	DDRB = 0x07;
	PORTB = 0x07;
	DDRD = 0x00;
	PORTD = 0x1F;
	bool run = true;
	bool previous1 = false, previous2 = false; 
	int counter = 0;
	int recounter = 0x7;
	int i = 0;
    while (1) 
    {	
		if(run){
			
			if(i==10){
				PORTB = (~counter & 0x07);
				counter--;
				if(counter <=0)counter = recounter;
				i=0;
				}
			i++;
			if(PIND & (1<<PIND3)){
				if(!previous1){
					recounter = 0;
					if(PIND & (1<<PIND0))recounter |= 0x01;
					if(PIND & (1<<PIND1))recounter |= 0x02;
					if(PIND & (1<<PIND2))recounter |= 0x04;
					counter = recounter;
					run = false;
				}
				previous1 = true;
			}
			else previous1=false;
		
		}
		if(!run){
			if(PIND & (1<<PINB4)){
				if(!previous2){
					run= true;	
					
				}
				previous2 = true;
			}
			else previous2=false;
		}
		_delay_ms(100);
    }
}

