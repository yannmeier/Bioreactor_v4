# Installation

## Cloning the project

You know with age we take some habits and it would be easier if you follow the same one ...

All our github projects are always in a folder called `git`
that is at the first level of the home directory. It is followed by the 
name of the github user/organisation and finaly the project.

So to install the project from bash:
```
mkdir -p ~/git/bioreactor/
cd ~/git/bioreactor/
git clone git@github.com:Bioreactor/Bioreactor_v4.git
```

The possiblity to clone the project will require that you have validate your public key
in github !

## Installing the Arduino IDE

Currently we are still using the Arduino IDE. Plase download the last 1.6 version
of the program from: https://www.arduino.cc/en/Main/Software

## Another IDE ...

Ready to try a new IDE to program the Arduino ? Have a look at this eclipse project and install it:
http://eclipse.baeyens.it/

Once it works, start the program, select the Arduino Leonardo and go in the menu `Arduino` -> `New sketch`.
As project name just enter 'BioMain' and for the location: `~/git/bioreactor/Bioreactor_v4/Arduino/BioMain`

After don't select any template (just normal .ino files that are already in the folder). The program
will automatically import the libraries if there are placed correctly (see just after).

## Hardware support for Leonardo 8MHz

The version 4.2 of the bioreactor is opeerating at 8MHz/3V3 instead of the standard 16MHz/5V, thus the boards.txt files that defines the options to burn the bootloader must be adjusted (file can be found under your arduino installation directory, eg: /usr/share/arduino-1.6.13/hardware/arduino/boards.txt) by adding the follwing lines:

leonardo8.name=Arduino Leonardo 8MHz
leonardo8.upload.protocol=avr109
leonardo8.upload.maximum_size=28672
leonardo8.upload.speed=57600
leonardo8.upload.disable_flushing=true
leonardo8.bootloader.low_fuses=0xff
leonardo8.bootloader.high_fuses=0xd8
leonardo8.bootloader.extended_fuses=0xcb
leonardo8.bootloader.path=caterina-LilyPadUSB
leonardo8.bootloader.file=Caterina-LilyPadUSB.hex
leonardo8.bootloader.unlock_bits=0x3F
leonardo8.bootloader.lock_bits=0x2F
leonardo8.build.mcu=atmega32u4
leonardo8.build.f_cpu=8000000L
leonardo8.build.vid=0x2341
leonardo8.build.pid=0x8036
leonardo8.build.core=arduino
leonardo8.build.variant=leonardo

OR IF YOU USE THE ECLIPSE IDE:

leonardo8.name=Arduino Leonardo 8MHz
leonardo8.vid.0=0x2341
leonardo8.pid.0=0x0036
leonardo8.vid.1=0x2341
leonardo8.pid.1=0x8036
leonardo8.vid.2=0x2A03
leonardo8.pid.2=0x0036
leonardo8.vid.3=0x2A03
leonardo8.pid.3=0x8036

leonardo8.upload.tool=avrdude
leonardo8.upload.protocol=avr109
leonardo8.upload.maximum_size=28672
leonardo8.upload.maximum_data_size=2560
leonardo8.upload.speed=57600
leonardo8.upload.disable_flushing=true
leonardo8.upload.use_1200bps_touch=true
leonardo8.upload.wait_for_upload_port=true

leonardo8.bootloader.tool=avrdude
leonardo8.bootloader.low_fuses=0xff
leonardo8.bootloader.high_fuses=0xd8
leonardo8.bootloader.extended_fuses=0xcb
leonardo8.bootloader.file=caterina-LilyPadUSB/Caterina-LilyPadUSB.hex
leonardo8.bootloader.unlock_bits=0x3F
leonardo8.bootloader.lock_bits=0x2F

leonardo8.build.mcu=atmega32u4
leonardo8.build.f_cpu=8000000L
leonardo8.build.vid=0x2341
leonardo8.build.pid=0x8036
leonardo8.build.usb_product="Arduino Leonardo"
leonardo8.build.board=AVR_LEONARDO
leonardo8.build.core=arduino
leonardo8.build.variant=leonardo
leonardo8.build.extra_flags={build.usb_flags}

## Libraries

In this project we use many non standard libraries. By default the Arduino IDE
will look for them in `~/Arduino/libraries`

If you are using only our project it is easier to directly point this folder
to the libraries that are present in the github.

```
mkdir ~/Arduino/
ln -s ~/git/bioreactor/Bioreactor_v4/Arduino/libraries/ ~/Arduino/
```

## That's it

You should now be able to compile the project from the Arduino IDE.
You can now proceed burning the bootloader using the Leonardo 8MHz newly created option and upload your code selecting the same Leonardo 8MHz in the list of available MCUs from the IDE. To do so, you must select the Leonardo 8MHz board under /Tools/Boards/ in the Arduino IDE or selecting the proper device during project creation in Eclipse. Note that the LCD board works under 16MHz/5V and must be flashed as a standard Leonardo board not a 8MHz one. You must also select the right board version when programming the board via USB !
