# Bioreactor_v4

This repository is dedicated to the version 4 of the Bioreactor.

## Electronics

The architecture for v4 is articulated around one central board powered by 
an ATmega32u4 MCU relying on nilRTOS and external boards for interfacing
to extra sensors and/or LCD screeens.

The main board comes with the following features:
- stepper motor control with Neodimium magnet
- Load cell based on HX711
- PID heater control
- DS18B20+ temperature sensing
- LoRaWAN mid-range wireless communication
- 2 peristaltic pumps control
- SPI port to control a slave LCD board
- 2 I2C ports for master to slave communication to external modules
- Serial over USB
- 12V DC jack supply

There are already a few external extra modules:
- LCD SPI slave board on ATmega32u4
- pH & Conductommetry regulation I2C slave, also works as a stand alone (in development phase)
- gas flux and mix control with analogic valves, also works as a stand alone (in development phase)
- spectrophotometer immersed in the bioreactor, also works as a stand alone (in development phase)

References:
-https://www.arduino.cc/en/Main/ArduinoBoardLeonardo
-https://github.com/greiman/NilRTOS-Arduino

Revision notes:
Bioreactor V4.2:
- Changed operating frequency from 16MHz to 8MHz
- Changed operating voltage from 5V to 3.3V
- Removed 5V regulation
- Routed LoRa RST pin to MCU with additionnal pull-up
- Hack to be performed on hardware (D6<>D12) for software compatibility
Bioreactor V4.3:
- Stepper motor has individually controllable phases
- LoRa SMA PCB edge Antenna support
- Hack to be performed on hardware (D6<>D12) for software compatibility
Bioreactor V4.4:
- D6<>D12 exchanged to provide PWM PID control of the heater

## Mechanics

(Documentation coming soon)

#Bioreactor_v5

In parallel to the development of the version 4 of the Bioreactor, a version 5, relying on the 
more powerful SAMD21 (Cortex M0+ ARM) MCU and FreeRTOS is currently being studied.

References:
-https://www.arduino.cc/en/Main/ArduinoBoardZero
-http://www.freertos.org/Atmel_SAMD20_RTOS.html
-https://www.kickstarter.com/projects/rabidprototypes/neutrino-the-tiny-32-bit-arduino-zero-compatible/updates
