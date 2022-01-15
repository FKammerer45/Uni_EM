#define F_CPU 16000000UL
#define BAUDRATE 9600
#define BAUD_CONST (((F_CPU/(BAUDRATE*16UL)))-1)
#define SIZE 32

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
typedef enum{false, true} bool;

typedef struct Ringpuffer{
	unsigned char arr[SIZE];
	unsigned int last;
	unsigned int first;
	unsigned int fuellstand;
	uint16_t size;
	bool full;
}Ringpuffer;

void set_size(uint16_t size, Ringpuffer* k){
	k->size = size;
	//k->arr = (unsigned char*) malloc(size);
	k->last = 0;
	k->first = 0;
	k->fuellstand = 0;
	k->full = false;
}
Ringpuffer RingpufferInit(uint16_t size){
	Ringpuffer k;// = (Ringpuffer*) malloc(sizeof(Ringpuffer));
	set_size(size,&k);
	return k;
}

Ringpuffer Receiver;
volatile bool Pause_sender = false;
volatile bool Pause_receiver = false;

unsigned char insert(Ringpuffer* k, unsigned char to_insert){
	if(!k->full){
		//printf("\nInserten: %c", to_insert);
		k->arr[k->last] = to_insert;
		k->last++;
		k->fuellstand++;
		if(k->last >= k->size)
			k->last = 0;
	}
	if(/*k->last == k->first*/ k->fuellstand >= SIZE - 3){//Puffer läuft über bzw last will first überrunden.
		/*if(k->full)
			printf("\n%c kann nicht eingefügt werden da der Puffer voll ist.", to_insert);*/
			if(k->fuellstand == SIZE)
				k->full = true;
			//Pause_sender = true;//last stand
			//USART_Transmit(0x13);
			return 0;
	}
	return to_insert;
}
unsigned char watch_last_without_consuming(Ringpuffer* k){
	if(k->last == 0)
		return k->arr[(k->size)-1];
	else
		return k->arr[k->last-1];

}
unsigned char watch_first_without_consuming(Ringpuffer* k){
	if(k->first != k->last || k->full){
		return k->arr[k->first];
	}
	return 0;
}
unsigned char take_first(Ringpuffer* k){
	if(k->fuellstand > 0){
		if(k->first != k->last || k->full){
			k->full = false;
			k->fuellstand--;
			unsigned char c = k->arr[k->first];
			k->first++;//Notiz blöd eigentlich sollte ich das hier abfangen weil ich vielleicht wo hinpointe wo ich nicht sollte
			if((k->first) >= k->size)//fang von vorne an
				k->first = 0;
			return c;
		}
		else//Notiz unnötig, da man hier nur ankommt wenn k->first == k->last aber es verdeutlicht das first last nicht überholen kann
			k->first = k->last;
	}
	return 0;
}
unsigned char take_last(Ringpuffer* k){
	if(k->fuellstand > 0){
		k->full = false;
		k->fuellstand--;
		if(k->last == 0)
			k->last = k->size - 1;
		else
			k->last--;
		return k->arr[k->last];
	}
	return 0;
}
bool is_full(Ringpuffer* k){
	return k->full;
}
bool is_empty(Ringpuffer* k){
	if(watch_first_without_consuming(k) == 0)
	return true;
	return false;
}
void print_fuellstand(Ringpuffer* k){
	printf("\nDer Füllstand beträgt: %d", k->fuellstand);
}
int give_fuellstand(Ringpuffer* k){
	return k->fuellstand;
}

void USART_Init(){
	UBRR0H = (unsigned char)(BAUD_CONST >> 8);
	UBRR0L = (unsigned char)BAUD_CONST;
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR0B |= (1<<RXCIE0);
}
void USART_Transmit(unsigned char data){
	while(Pause_receiver == true) ;
	while(!(UCSR0A & (1<<UDRE0))) ;
	UDR0 = data;
}
unsigned char USART_Receive(){
	//while(!(UCSR0A & (1<<RXC0))) ;
	if(Pause_sender == true && give_fuellstand(&Receiver) < SIZE - 10){
		Pause_sender = false;
		USART_Transmit(0x11);
	}
	return take_first(&Receiver);
}

ISR(USART_RX_vect){
	if(insert(&Receiver, UDR0) == 0){
		Pause_sender = true;
		USART_Transmit(0x13);//Wir wollen Pause
	}
	if(watch_last_without_consuming(&Receiver) == 0x13){//Sender will Pause
		Pause_receiver = true;
		take_last(&Receiver);
	}

	else if(watch_last_without_consuming(&Receiver) == 0x11){//Sender will weiter machen
		Pause_receiver = false;
		take_last(&Receiver);
	}
}

int main(void){
	USART_Init();
	Receiver = RingpufferInit(SIZE);
	Ringpuffer Ringpuffer1 = RingpufferInit(SIZE);
	const char meldung[]="Hier ATmega. Wer da? ";
	sei();
	for(int f=0;meldung[f]!='\0';f++)
		USART_Transmit(meldung[f]);

	while (true){
		insert(&Ringpuffer1, USART_Receive());
		//USART_Transmit(watch_last_without_consuming(&Ringpuffer1));
		//Fehler vorher
		if(watch_last_without_consuming(&Ringpuffer1) == 0){//solange Pufferleer reinschreibt schreibe an die selbe stelle
			take_last(&Ringpuffer1);
			continue;
		}
		
		if(watch_last_without_consuming(&Ringpuffer1)!= 0x0d && Ringpuffer1.fuellstand <= Ringpuffer1.size - 3){
			/*sprintf(data, "Fuellstand: %d",Ringpuffer1.fuellstand);
			for(int i g= 0; data[i] != '\0'; i++)
				USART_Transmit(data[i]); */
			USART_Transmit(watch_last_without_consuming(&Ringpuffer1));//sehen was geschrieben wurde
			continue;
		}
		USART_Transmit(watch_last_without_consuming(&Ringpuffer1));
		USART_Transmit('\n');
		USART_Transmit(0x0d);
		USART_Transmit('H');
		USART_Transmit('i');
		USART_Transmit(' ');

		while(watch_first_without_consuming(&Ringpuffer1)){
			if(watch_first_without_consuming(&Ringpuffer1) <= 96 || watch_first_without_consuming(&Ringpuffer1) >= 123){
				if(watch_first_without_consuming(&Ringpuffer1) == 0x0d){
					take_first(&Ringpuffer1);
					USART_Transmit('\n');
					USART_Transmit(0x0d);
				}
				else 
					USART_Transmit(take_first(&Ringpuffer1));
			}
			else 
				USART_Transmit(take_first(&Ringpuffer1)-32);
		}
		USART_Transmit('\n');
		USART_Transmit(0x0d);	
	}
	return 0;
}