/*
 * demo1switch.c
 *
 * Created: 24/3/2021 15:22:47
 * Author : Chang Jiahan
 */ 

// PB0-PB2 connected to rows
// PD2-3 connected to columns
#include <avr/io.h>


//input PORTD2-3
//output PORTB0-2

unsigned char str;
unsigned char data;
//initialize
void usart_init (void){
	UCSR0B=(1<<TXEN0) | (1<<RXEN0);
	UCSR0C=(1<<UCSZ01) | (1<<UCSZ00);
	//UBRR0L=0XCF; //baud rate=4800
	UBRR0L=0X67;//baudrate=9600
}
//send data
void usart_send (unsigned char ch){
	while (! (UCSR0A & (1<<UDRE0)));
	UDR0=ch;
}
//receive data
unsigned char usart_receive(void){
	while (! (UCSR0A & (1<<RXC0)));
	data= UDR0;
	return data; //Can¡¦t directly return UDR0 !!!!!
}

void delay_10ms()
{
	TCCR1A = 0b00000000; // CTC TOP = OCR1A
	TCCR1B = 0b00001011; // Prescaler = 64
	OCR1A = 2500; // 1 / (16 MHz / 64) = 0.004 ms,  10ms / 0.004 ms = 2500
	
	unsigned char count = 0;
	while (count < 1) // 1 x 10 ms = 10ms
	{
		while(!(TIFR1 & (1<<OCF1A))) ;
		TIFR1 = (1<<OCF1A);
		count++;
	}
}


int main(void)
{
	unsigned char colloc,rowloc=5,column=5;
	unsigned int flag;
	unsigned char state=0;
	usart_init();
	DDRD= 0x00 ; //input PORTD2-3
	PORTD=0xFF;
	DDRB=0xFF; //output PORTB0-2
	PORTB=0x00;
	do
	{
		PORTB = 0xF8; //ground all rows at once
		colloc=(PIND & 0x0C); //read the columns
	} while (colloc != 0x0C); //check until all keys released
	while (1)
	{
		PORTB = 0xF8; //ground all rows at once
		
		do
		{
			do
			{
				delay_10ms();  //call delay
				colloc=(PIND & 0x0C); //see if any key is pressed
				if(state==0)
				{
					usart_send('a');
				}
				else state=0;
			} while (colloc == 0x0C); //keep checking for key press
			
			//delay_50ms();  //call delay for debounce
			colloc=(PIND & 0x0C); //read columns
		} while (colloc == 0x0C);
		
		while(1)
		{
			flag=0;
			state=1;
			PORTB = 0xFE; //ground row 0
			delay_10ms();  //call delay
			colloc=(PIND & 0x0C); //read columns
			
			if(colloc != 0x0C) //column detected
			{
				rowloc=0;  //save row location
				if(colloc==0x08){column=0;}
				else{column=1;}
				flag++;
			}
			
			PORTB=0xFD; //ground row 1
			delay_10ms();  //call delay
			colloc=(PIND & 0x0C); //read columns
			
			if(colloc != 0x0C) //column detected
			{
				rowloc=1;  //save row location
				if(colloc==0x08){column=0;}
				else{column=1;}
				flag++;
			}
			
						
			PORTB=0xFB; //ground row 2
			delay_10ms();  //call delay
			colloc=(PIND & 0x0C); //read columns
			
			if(colloc != 0x0C) //column detected
			{
				rowloc=2;  //save row location
				if(colloc==0x08){column=0;}
				else{column=1;}
				flag++;
			}
		
			if(flag==3){
				if(colloc == 0x04){ //PD3 (right up)
					usart_send('5');
					rowloc=3;
					break;
				}
				else if(colloc == 0x08){ //PD2 (left up)
					usart_send('1');
					colloc=0x0C;
					rowloc=3;
					break;
				}
			}
			else{break;}
		
		}
		
		//check column and send result to tera term
		//column 0(PD2)
		if(column==0){
			if(rowloc==0){ //row 0(PB0)
				usart_send('2');
			}
			if(rowloc==1){ //row 1(PB1)
				usart_send('3');
			}
			if (rowloc==2){ //row 2(PB2)
				usart_send('4');
			}
			
		
		}
		//column 1
		else if(column==1){
			if(rowloc==0){ //row 0(PB0)
				usart_send('8');
			}
			if(rowloc==1){ //row 1(PB1)
				usart_send('6');
			}
			if(rowloc==2){ //row 2(PB2)
				usart_send('7');
			}
		}
		
	
	}
	return 0;
}
