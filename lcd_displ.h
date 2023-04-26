

#define set_lcd_d4 PORTD|=(1<<PD5)
#define clr_lcd_d4 PORTD&=~(1<<PD5)
#define ddr_d4 DDRD|=(1<<PD5)           // tak jak set_lcd

#define set_lcd_d5 PORTD|=(1<<PD4)
#define clr_lcd_d5 PORTD&=~(1<<PD4)
#define ddr_d5 DDRD|=(1<<PD4)

#define set_lcd_d6 PORTD|=(1<<PD3)
#define clr_lcd_d6 PORTD&=~(1<<PD3)
#define ddr_d6 DDRD|=(1<<PD3)

#define set_lcd_d7 PORTD|=(1<<PD2)
#define clr_lcd_d7 PORTD&=~(1<<PD2)
#define ddr_d7 DDRD|=(1<<PD2)

#define set_lcd_en PORTB|=(1<<PB3)
#define clr_lcd_en PORTB&=~(1<<PB3)
#define ddr_en DDRB|=(1<<PB3)
 
#define set_lcd_rs PORTB|=(1<<PB4)
#define clr_lcd_rs PORTB&=~(1<<PB4)
#define ddr_rs DDRB|=(1<<PB4)

// podswietlenie , jesli uzywane 
/*#define set_lcd_light PORTB|=(1<<5)
#define clr_lcd_light PORTB&=~(1<<5)
#define ddr_light DDRB|=(1<<5)
*/



void lcd_init(void);
void lcd_send_data(unsigned char b);
void lcd_send_com(unsigned char b);
void lcd_send_4(unsigned char b);
void lcd_send_8(unsigned char b);
void lcd_control( unsigned char on, unsigned char cur, unsigned char blink);
void lcd_clear(void);
void lcd_home(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_swrite( char *s);
void lcd_iwrite(int i);
int lcd_printf( char *format, ... );


