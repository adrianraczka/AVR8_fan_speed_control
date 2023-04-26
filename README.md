# AVR8_fan_speed_control
This project was to create a simple fan controller using Atmega328p that consints of two modes of operation: 
- auto (speed controlled automatically by MCU depends on temperature - DS18B20)
- manual (speed controlled by MCU using potentiometer - ADC)

Used elements:
- 2x micro switch
- blue, red, green diod
- LCD 16x2 (HD44780)
- 2x Dallas DS18B20
- potentiometer (10 [kΩ])
- 1x resistor 220 [Ω], 2x resistor 10 [kΩ], 3x resistor 1 [kΩ], 2x resistor 4.7 [kΩ]

Simplified design scheme:

![scheme](https://user-images.githubusercontent.com/109549335/234695388-1c8f0430-99e1-4d36-af4d-27f712ecda66.png)

