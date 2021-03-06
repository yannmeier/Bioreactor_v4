Pin mapping
===========

Port | Pin | Arduino Mask | Bertha V4.1-2 | Bertha V4.3 | Bertha V4.4 | pH V5.x | LCD V0.2/V1.x
-----|-----|--------------|---------------|-------------|-------------|---------|---------------
B | 0 | RXLED/SS | RXLED | RXLED | RXLED | RXLED | SS
B | 1 | SCK | SCK | SCK | SCK | SCK | SCK
B | 2 | MOSI | MOSI | MOSI | MOSI | MOSI | MOSI
B | 3 | MISO | MISO | MISO | MISO | MISO | MISO
B | 4 | D8 | - | Stepper 1 | Stepper 1 | Stepper#1-1 | LCD #11
B | 5 | D9 | - | Stepper 2 | Stepper 2 |  | LCD #6
B | 6 | D10 | SPI flash slave | SPI flash slave | PWM food IN | SPI flash slave | LCD #4
B | 7 | D11 | SPI LCD slave | SPI LCD slave | SPI LCD slave | SPI LCD slave | -
C | 6 | D5 | LoRa RST | LoRa RST | PWM food OUT | LoRa RST | -
C | 7 | D13 | Blink Led | Blink Led | Blink Led | Blink Led | Blink Led
D | 0 | D3/SCL | I2C SCL | I2C SCL | I2C SCL | I2C SCL | -
D | 1 | D2/SDA | I2C SDA | I2C SDA | I2C SDA | I2C SDA | -
D | 2 | D0/RX | LoRa RX | LoRa RX | LoRa RX | LoRa RX | ENC B
D | 3 | D1/TX | LoRa TX | LoRa TX | LoRa TX | LoRa TX | ENC A
D | 4 | D4/A6 | Ext. OneWire | Ext. OneWire | Ext. OneWire | Stepper#1-2 | LCD #14
D | 5 | TXLED | TXLED | TXLED | TXLED | TXLED | -
D | 6 | D12 | PID (hack: D12<>D6) | PID (hack: D12<>D6) | OneWire | Stepper#1-3 | LCD #13
D | 7 | D6 | OneWire (hack: D6<>D12) | OneWire (hack: D6<>D12) | PID on PWM | Stepper#1-4 | LCD #12
E | 2 | D7 | - | - | - | Data HX711 | ENCODER INT
E | 6 | HWB | HWB | HWB | HWB | HWB | HWB
F | 0 | A5 | SCK HX711 | SCK HX711 | SCK HX711 | SCK HX711 | -
F | 1 | A4 | Data HX711 | Data HX711 | Data HX711 | PH NTC TEMP | -
F | 4 | A3 | Food out | Food out | SPI flash slave | Stepper#2-4 | -
F | 5 | A2 | Food in | Food in | LoRa RST | Stepper#2-1 | -
F | 6 | A1 | Stepper 2+4 | Stepper 4 | Stepper 4 | Stepper#2-2 | -
F | 7 | A0 | Stepper 1+3 | Stepper 3 | Stepper 3 | Stepper#2-3 | -
