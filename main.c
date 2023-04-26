/*
 * Projekt1.c
 * Author : Adrian R.
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "lcd_displ.h"
#include "ds18b20.h"
#include "onewire.h"
#include "romsearch.h"
#include <stddef.h>

//Inicjalizacja ADC na portcie ADC0 - PC0
void ADCInit()
{
	ADMUX|=(1<<REFS0);
	ADMUX|=(0<<MUX3) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0); //ADC0 -> PC0
	ADCSRA|=(1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	
	DDRC&=~(1<<PC0);
}

//inkrementacja zmiennej i, gdy przycisk(PB0) jest w stanie wysokim, dwa tryby pracy - i=0 (OFF), i=1 (ON)
int OnOff(int i)
{
		if(!(PINB & (1<<PB0)))
		{
			i++;
			_delay_ms(150);
			if(i==2)
			{
				i=0;
			}
		}

	return i;
}

//inkrementacja zmiennej j, gdy przycisk(PB5) jest w stanie wysokim, dwa tryby pracy - j=0 (Manual), j=1 (Auto)
int ManualAuto(int j)
{
	if(!(PINB & (1<<PB5)))
	{
		j++;
		_delay_ms(150);
		if(j==2)
		{
			j=0;
		}
	}

	return j;
}

//Generowanie charaktrerystyki sterowania automatycznego na podstawie œredniej temperatury, charakt. sk³ada siê z 5 prostych ograniczonych 6 punktami, które u¿ytkownik mo¿e sam zdefiniowaæ
uint16_t SetRPMAuto(int TOP, int16_t average_temperature, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5)
{
	//TOP=255
	//x(i) - temp['C]
	//y(i) - pwm[%]
	
	uint16_t OCR=0;
	float a0,b0,a1,b1,a2,b2,a3,b3,a4,b4;
	
	//przejœcie z pwm[%] na wartoœæ OCR
	y0=TOP-(TOP*y0/100);
	y1=TOP-(TOP*y1/100);
	y2=TOP-(TOP*y2/100);
	y3=TOP-(TOP*y3/100);
	y4=TOP-(TOP*y4/100);
	y5=TOP-(TOP*y5/100);
	
	if (average_temperature<x1)
	{
		a0=(y0-y1)/(x0-x1);
		b0=y1-a0*x1;
		OCR=a0*average_temperature+b0;
		if(average_temperature<x0)
		{
			OCR=y0;
		}
	}	
	else if (average_temperature>=x1 && average_temperature<x2)
	{
		a1=(y1-y2)/(x1-x2);
		b1=y2-a1*x2;
		OCR=a1*average_temperature+b1;					
	}
	else if (average_temperature>=x2 && average_temperature<x3)
	{
		a2=(y2-y3)/(x2-x3);
		b2=y3-a2*x3;
		OCR=a2*average_temperature+b2;				
	}
	else if (average_temperature>=x3 && average_temperature<x4)
	{
		a3=(y3-y4)/(x3-x4);
		b3=y4-a3*x4;
		OCR=a3*average_temperature+b3;
	}
	else if (average_temperature>=x4)
	{
		a4=(y4-y5)/(x4-x5);
		b4=y5-a4*x5;
		OCR=a4*average_temperature+b4;		
		if(average_temperature>x5)
		{
			OCR=y5;
		}
	}
	
	return(OCR);
}

//funkcja zwracaj¹ca wartoœæ temp. z czujnika nr.1 powiêkszon¹ dziesiêciokrotnie
int16_t czuj1()
{
	int16_t temp1;
	uint16_t temperature1;

	//czujnik 1 PB1
	ds18b20wsp(&PORTD, &DDRD, &PIND, ( 1 << 6 ), NULL, 0, 100, DS18B20_RES09);
	ds18b20convert( &PORTD, &DDRD, &PIND, ( 1 << 6 ), NULL);
	ds18b20read( &PORTD, &DDRD, &PIND, ( 1 << 6 ), NULL, &temp1 );
	_delay_ms(20);
	temperature1=temp1*10/16;
	
	return(temperature1);
}

//funkcja zwracaj¹ca wartoœæ temp. z czujnika nr.2 powiêkszon¹ dziesiêciokrotnie
int16_t czuj2()
{
	int16_t temp2;
	uint16_t temperature2;
	
	//czujnik 2 PD7
	ds18b20wsp(&PORTD, &DDRD, &PIND, ( 1 << 7 ), NULL, 0, 100, DS18B20_RES09);
	ds18b20convert( &PORTD, &DDRD, &PIND, ( 1 << 7 ), NULL);
	ds18b20read( &PORTD, &DDRD, &PIND, ( 1 << 7 ), NULL, &temp2 );
	_delay_ms(20);
	temperature2=temp2*10/16;
	
	return(temperature2);
}

//obliczenie wartoœci ca³kowitej temperatury œredniej i jej zwrócenie
int16_t avr_temp_calk(int16_t temperature1, int16_t temperature2)
{
	int16_t average_temperature2=0;
	int16_t average_temperature1=0;

	average_temperature2=(temperature1+temperature2)/2;				
	average_temperature1=average_temperature2/10; //calk
	
	return(average_temperature1);
}

//obliczenie wartoœci setnej temperatury œredniej i jej zwrócenie
int16_t avr_temp_dzies(int16_t temperature1, int16_t temperature2)
{
	int16_t average_temperature2;
	int16_t average_temperature1;
	int16_t average_temperature;

	average_temperature2=(temperature1+temperature2)/2;
	average_temperature1=average_temperature2/10;
	average_temperature1=average_temperature1*10; // zmienna ta nie posiada juz informacji o wartoœci temp po przecinku
	average_temperature=average_temperature2-average_temperature1;
	
	return(average_temperature);
}

//Inicjalizacja fast pwm na PB1
void Init_PWM()
{
	TCCR1A|= (1<<COM1A0) | (1<<COM1A1); //set
	TCCR1A|= (1<<WGM10) | (1<<WGM12); //8bit fast PWM
	TCCR1B|= (1<<CS00); //no prescaler
	
	DDRB|= (1<<PB1);
}

int main(void)
{
	
DDRC|=(1<<PC5); //Dioda zielona PC5 (ON/OFF)
DDRC|=(1<<PC4); //Dioda czerw. PC4 (Manual)
DDRB|=(1<<PB2); //Dioda niebieska (Auto)
DDRB&=~(1<<PB0); //Przycisk ON/OFF DO MASY PRZEZ REZYSTOR PULL-DOWN
PORTB|=(1<<PB0); //stan wysoki na PB0
DDRB&=~(1<<PB5); //Przycisk Manual/Auto
PORTB|=(1<<PB5); //stan wysoki na PB5

int16_t temperature1=0;
int16_t temperature2=0;

int16_t average_temperature=0;
int16_t average_temperature1=0;

int i=0;
int j=0;

uint32_t p;
uint32_t SetRPM=0;

//Sterowanie manualne
//PWM - potencjometr -> wype³nienie 10-100%

Init_PWM();

while(1)
{
	lcd_init();
	lcd_clear();
	_delay_ms(2);
	
	if (i==0) //off
	{		

		PORTC&=~(1<<PC5);
		PORTC&=~(1<<PC4);
		PORTB&=~(1<<PB2);
		lcd_clear();
		_delay_ms(10);
		
		while(i==0)
		{

			temperature1=czuj1();
			_delay_ms(10);
			temperature2=czuj2();
			_delay_ms(10);
			
			average_temperature=avr_temp_dzies(temperature1,temperature2);
			average_temperature1=avr_temp_calk(temperature1,temperature2);	
				
			OCR1A=205; //pwm 
			SetRPM=OCR1A*(-1800/255)+1800;

			lcd_gotoxy(0,0);
			lcd_printf("Temp: %d.%d'C",average_temperature1,average_temperature);		

			lcd_gotoxy(0,1);
			lcd_printf("SetRPM: %d",SetRPM);
			
			i=OnOff(i);
			j=ManualAuto(j);
			
		}

	}

	else if (i==1) //on
	{
		if(j==0) //ster. manualne
		{
			PORTC|=(1<<PC4);
			PORTB&=~(1<<PB2);
			PORTC|=(1<<PC5);
			PORTD&=~(1<<PD6);		
			PORTD|=(1<<PD7);
			lcd_init();
			Init_PWM();			
			lcd_clear();
			_delay_ms(10);
			ADCInit();
			ADCSRA|=(1<<ADSC);
			
			while((i==1)&&(j==0))
			{
				
				temperature1=czuj1();
				_delay_ms(10);
				temperature2=czuj2();
				_delay_ms(10);		
						
				average_temperature=avr_temp_dzies(temperature1,temperature2);
				average_temperature1=avr_temp_calk(temperature1,temperature2);
							
				p=ADC;
				OCR1A=(-229*p/1023)+229;
				SetRPM=OCR1A*(-1800/255)+1800;
				
				i=OnOff(i);
				j=ManualAuto(j);
				
				_delay_ms(15);
				lcd_clear();
				_delay_ms(2);
				
				lcd_gotoxy(0,0);
				lcd_printf("Temp: %d.%d'C",average_temperature1,average_temperature);
				
				lcd_gotoxy(0,1);
				lcd_printf("SetRPM: %d",SetRPM);
				
			}
		}
		else if ((i==1)&&(j==1)) //ster. auto
		{
			PORTC|=(1<<PC5);
			PORTC&=~(1<<PC4);
			PORTB|=(1<<PB2);
			ADCSRA&=~(1<<ADSC);
			ADCSRA&=~(1<<ADEN);
			Init_PWM();
			lcd_clear();
			_delay_ms(10);
						
			while((i==1)&&(j==1))
			{
				
				OCR1A=SetRPMAuto(255,average_temperature1,0,0,10,5,20,8,25,15,40,45,50,90);
				SetRPM=OCR1A*(-1800/255)+1800;
				
				temperature1=czuj1();
				_delay_ms(10);
				
				temperature2=czuj2();
				_delay_ms(10);
				
				average_temperature=avr_temp_dzies(temperature1,temperature2);
				average_temperature1=avr_temp_calk(temperature1,temperature2);
				
				lcd_gotoxy(0,0);
				lcd_printf("Temp: %d.%d'C",average_temperature1,average_temperature);
				
				lcd_gotoxy(0,1);
				lcd_printf("SetRPM: %d",SetRPM);
					
				i=OnOff(i);
				j=ManualAuto(j);	

			}			
		}		
	}	
}

}

