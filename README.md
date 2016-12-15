# Bioreactor_v4

This repository is dedicated to the version 4 of the Bioreactor.

##Electronics

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

##Mechanics

(Documentation coming soon)

#Bioreactor_v5

In parallel to the development of the version 4 of the Bioreactor, a version 5, relying on the 
more powerful SAMD21 (Cortex M0+ ARM) MCU and FreeRTOS is currently being studied.

References:
-https://www.arduino.cc/en/Main/ArduinoBoardZero
-http://www.freertos.org/Atmel_SAMD20_RTOS.html
-https://www.kickstarter.com/projects/rabidprototypes/neutrino-the-tiny-32-bit-arduino-zero-compatible/updates


#For Developers

It is recommended to use the version 1.0.5 of the arduino IDE packed here: http://blog.spitzenpfeil.org/arduino/mirror_released/ . You might need to install the last version of arduino to have the necessary libs on your computer first.
Then unzip the arduino libraries provided in your local arduino libraries folder: 
eg. /usr/share/arduino/liraries on Fedora sytems

The version 4.2 of the bioreactor is opeerating at 8MHz/3V3 instead of the standard 16MHz/5V, thus the boards.txt files
that defines the options to burn the bootloader must be adjusted (file can be found under your arduino installation directory, eg: /usr/share/arduino-1.0.5/hardware/arduino/boards.txt)
by adding the follwing lines:

leonardo8.name=Arduino Leonardo 8MHz<br />
leonardo8.upload.protocol=avr109<br />
leonardo8.upload.maximum_size=28672<br />
leonardo8.upload.speed=57600<br />
leonardo8.upload.disable_flushing=true<br />
leonardo8.bootloader.low_fuses=0xff<br />
leonardo8.bootloader.high_fuses=0xd8<br />
leonardo8.bootloader.extended_fuses=0xcb<br />
leonardo8.bootloader.path=caterina-LilyPadUSB<br />
leonardo8.bootloader.file=Caterina-LilyPadUSB.hex<br />
leonardo8.bootloader.unlock_bits=0x3F<br />
leonardo8.bootloader.lock_bits=0x2F<br />
leonardo8.build.mcu=atmega32u4<br />
leonardo8.build.f_cpu=8000000L<br />
leonardo8.build.vid=0x2341<br />
leonardo8.build.pid=0x8036<br />
leonardo8.build.core=arduino<br />
leonardo8.build.variant=leonardo<br />

Then only you can proceed burning the bootloader using the Leonardo 8MHz newly openend option and upload your code selecting the same Leonardo 8MHz in the list of available MCUs from the IDE.
To do so, you must select the Leonardo 8MHz board under /Tools/Boards/ . Note that the LCD board works under 16MHz/5V and must be flashed as a standard Leonardo board. You must also select the right board version when programming the board via USB.


