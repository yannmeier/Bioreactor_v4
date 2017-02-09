Parameters
==========

All the functionalities and the communication between processes is ensured by a common list of parameters.
The first 26 parameters (A -> Z) will be saved regularly in the log. Those values will be recovered when the
bioreactor reboot.



id | p | name                          | description
---|---|-------------------------------|---------------------------------------------------------------------------------
0  | A | PARAM_TEMP_LIQ                | temperature of the solution
1  | B | PARAM_TEMP_PCB                | temperature of the heating plate
2  | C | PARAM_WEIGHT                  | in unit of the balance
3  | D | PARAM_WEIGHT_SINCE_LAST_EVENT | 
4  | E | PARAM_PH                      | current pH
5  | F | PARAM_PH_STATE                | 0: Pause 1 : normal acquisition, 2 : purge of pipes,  4: calibration pH=4, 7: calibration pH=7, 10: calibration pH=10
6  | G | PARAM_FLUX_GAS1               |
7  | H | PARAM_FLUX_GAS2               |
8  | I | PARAM_FLUX_GAS3               |
9  | J | PARAM_FLUX_GAS4               |
10 | K | PARAM_CONDUCTO                |
24 | Y | PARAM_ERROR                   |
25 | Z | PARAM_STATUS                  | currently active service
26 | AA | PARAM_TEMP_TARGET            | target temperature of the liquid
28 | AC | PARAM_STEPPER_STEPS          | number of steps before changing the direction of the motor
29 | AD | PARAM_WEIGHT_MIN             |
30 | AE | PARAM_WEIGHT_MAX             |
31 | AF | PARAM_SEDIMENTATION_TIME     | number of minutes to wait without rotation before emptying
32 | AG | PARAM_FILLED_TIME            | number of mintues to stay in the filled state
33 | AH | PARAM_WEIGHT_FACTOR          | weight calibration: conversion factor digital -> gr
34 | AI | PARAM_WEIGHT_OFFSET          | weight calibration: digital offset value when bioreactor is empty
35 | AJ | PARAM_TARGET_PH              | desired pH
36 | AK | PARAM_PH_FACTOR_A            |
37 | AL | PARAM_PH_FACTOR_B            |
38 | AM | PARAM_STEPPER_SPEED          | motor speed
39 | AN | PARAM_DESIRED_FLUX_GAS1      |
40 | AO | PARAM_DESIRED_FLUX_GAS2      |
41 | AP | PARAM_DESIRED_FLUX_GAS3      |
42 | AQ | PARAM_DESIRED_FLUX_GAS4      |
43 | AR | PARAM_ANEMO_OFFSET1          | anemometer calibration: offset of the digital value (digital value when no gas is flowing)
44 | AS | PARAM_ANEMO_OFFSET2          |
45 | AT | PARAM_ANEMO_OFFSET3          |
46 | AU | PARAM_ANEMO_OFFSET4          |
47 | AV | PARAM_ANEMO_FACTOR1          | anemometer calibration factor: conversion between gas flux (of air) and digital unit
48 | AW | PARAM_ANEMO_FACTOR2          |
49 | AX | PARAM_ANEMO_FACTOR3          |
50 | AY | PARAM_ANEMO_FACTOR4          |
51 | AZ | PARAM_ENABLED                | enabled service (set by user)


State machine
=============

There are 3 important variables that will manage the state of the bioreactor

* PARAM_STATUS : the current status of the bioreactor
* PARAM_ERROR : if there is any error in one of the process
* PARAM_ENABLED : the function that are currently enabled

The PARAM_ENABLED will allow to activate or deactivate some function of the bioreactor. It is for example possible
to disable heating while keeping all the other functionalities active.

PARAM_STATUS
------------

`PARAM_STATUS` will display the currently active functionalities. It is compose of different bits that can
be enabled or disabled using the method `start` and `stop`. You may also check the status of one of the function using
`getStatus`.



Bit  | PARAM_STATUS         | Comment
-----|----------------------|----------------------------------
0    | FLAG_STEPPER_CONTROL | enable/disable agitation control
1    | FLAG_FOOD_CONTROL    | enable/disable food control
2    | FLAG_PID_CONTROL     | enable/disable heating
3    | FLAG_PH_CONTROL      | enable/disable pH control
4    | FLAG_GAS_CONTROL     | enable/disable gas control
7    | FLAG_SEDIMENTATION   | enable/disable sedimentation (one of the phase of food control)
8    | FLAG_RELAY_FILLING   | enable/disable filling pump (one of the phase of food control)
9    | FLAG_RELAY_EMPTYING  | enable/disable emptying pump (one of the phase of food control)
11   | FLAG_PH_CALIBRATE    | enable/disable pH calibration
12   | FLAG_RELAY_ACID      | enable/disable add acid
13   | FLAG_RELAY_BASE      | enable/disable add base

The status is currently the `Z` parameter. You can change the status by chaging this value. for example
if you want to force the bioreactor to go in the emptying state you should do ensure that the bit
`FLAG_RELAY_EMPTYING` is set. In other word you may have to add 2^9 (512) to your value of the
parameter `Z` (in the case it was not yet enable).


PARAM_ENABLED
-------------

`PARAM_ENABLED` allows to enable or disable some functionalities of the bioreactor. Currrently it can control
heating, food control and agitation.

Bit  | PARAM_STATUS         | Comment
-----|----------------------|----------------------------------
0    | FLAG_STEPPER_CONTROL | enable/disable agitation control
1    | FLAG_FOOD_CONTROL    | enable/disable food control
2    | FLAG_PID_CONTROL     | enable/disable heating

If you want to control everything the value of `PARAM_ENABLED` should be 7.

