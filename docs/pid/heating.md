# Connecting the plate

The heating plate is a standard MK3 plate used in 3D printers. It is a square of 214mm of side.

It is composed on 2 resistors of 2.4Ω and there are 2 ways to connect them:

* in serie : you have a resistor of 4.8Ω and this will be used to power in 24v
* in parallel : you have a resistor of 1.2Ω and this will be used when powerred in 12v

In our case we will power the plate in 12v and we will connect them in parallel. See the picture for more information.

It is also important to put 2 leds (just to be sure one of them turn on even if you power the plate in the opposite way) and a resistor of 10kΩ.
This will allow to see when the plate is turned on. Nice to debug !

<img src='connecion.jpg' width=500>

With a resistance of 1.2Ω in 12v we will need 10A. This is the limit of the powersuppy and it would be nice if we don't use permanently full power.


# Limit values

In order to be able to heat the following conditions have to be met:

* PCB temperature probe (one-wire) must be working and the temperature must be between 10 and 70°C
* Liquid temperature probe (one-wire) must be connected and the temperature must be between 10 and 60°C

The maximal power is 80% of full power that corresponds to around 8A. 

The following parameters are define in PID.ino 
* SAFETY_MAX_PCB_TEMP 7000
* SAFETY_MIN_PCB_TEMP 1000
* SAFETY_MAX_LIQ_TEMP 6000
* SAFETY_MIN_LIQ_TEMP 1000
* PID_OUTPUT_LIMIT    200 


# Disabling heating

Here are the important parameters:

| PARAM_TEMP_LIQ           |  0 |  A | temperature of the solution      |
| PARAM_TEMP_PCB           |  1 |  B | temperature of the heating plate |
| PARAM_TEMP_TARGET        | 26 | AA | target temperature of the liquid |
| PARAM_TEMP_MAX           | 27 | AB | maximal temperature of the liquid |

In order to disable heating just put at maximal temperature for the board of 0.




