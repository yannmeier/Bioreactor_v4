# Preparing the heating plate

[Back to index](../../index.md)
[Previous section](../weight)

---

## The heating plate

The heating plate is a standard MK3 plate used in 3D printers. It is a square of 214mm of side.

It is composed on 2 resistors of 2.4Ω and there are 2 ways to connect them:

* in serie : you have a resistor of 4.8Ω and this will be used to power in 24v
* in parallel : you have a resistor of 1.2Ω and this will be used when powerred in 12v

In our case we will power the plate in 12v and we will connect them in parallel.

It is also important to put 2 leds (just to be sure one of them turn on even if you power the plate in the opposite way)
and a resistor of 1kΩ (10kΩ would give a much less bright light).
This will allow to see when the plate is turned on. Nice to debug !

<img src="leds.jpg" width="500">

With a resistance of 1.2Ω in 12v we will need 10A. This is the limit of the powersuppy and it would be nice if
we don't use permanently full power.

Note that the Board is operating at 3V3 and the PID is powered in 12V. Thus the PowerFET used to deliver the required
current must be picked up carefully to have a low threshold voltage Vgs and a low Rds channel resistance at this
operating 3v3 voltage. Good matches are provided [here on Mouser](http://www.mouser.ch/Semiconductors/Discrete-Semiconductors/Transistors/MOSFET/_/N-ax1sf?P=1z0yra8Z1z0y182Z1z0z7ptZ1yzxnagZ1yw76g4Z1yw76ckZ1yw76gb&Rl=ax1sfZgjdhsdZ1yjepp1Z1yjenlnSGTax1sfZgjdhp3Z1yw78huZ1y9kq41SGT&Keyword=MOSFET&OrgTerm=mosfet&FS=True&Ns=Pricing|0).

We have chosen the BUK965R8-100E for its steady Rds at low Vgs (down to 2V) even for high currents (up to 50A).

## Preparing a built-in heating plate on the bioreactor

As explained in the section concerning the [base](../base), a complete bioreactor contains a built-in heating plate.

The first step for assembling this is to assemble the casing that will contain the plate. It consists of four plastic parts on the sides and the heating plate placed horizontally between them.

Tape the *parts for the sides* together and glue them with *dichloromethane*. I advise to try inserting the heating plate between the pieces to test the fit before glueing. See picture for the result:

<img src="pid1.jpg" width="500">

Once the casing is dry, place the *heating plate* (metallic side facing up) in the casing so that it's about 2.5 cm high and laying flat. Make sure that the large rounded sides are properly sitting on the front and on the back. Use a stable support and fixate the plate with *silicone*.

<img src="pid2.jpg" width="500">

It is now time to connect the heating plate to the base.

First, bare two cables and solder them to the heating plate in the following manner:

<img src="pid3.jpg" width="500">

Then, take your *DC 5.5x2.5 cable* as well as *two pieces of wire* (2mm in diameter) and connect then in the following way:

<img src="pid4.jpg" width="500">

Take your base and glue the *metallic contact part* to the lid of the base, on the side opposing the central loading cell (there are two holes on the lid, they correspond to the one on the contact part), pass your cables threw the holes and solder them to the contact plate.

<img src="pid5.jpg" width="500">

Finally, place your heating plate on the base and verify that there is indeed a contact.

<img src="pid6.jpg" width="500">

## Preparing a stand-alone heating plate for testing purposes

You can connect a heating plate directly to a *DC 5.5x2.5 cable* so that you can test your boards heating functions without needing a whole bioreactor. The connection should look like that:

<img src="connection.jpg" width="500">

## Notable informations about the heating plate

### Limit values

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

### Enabling heating

In order to turn on the heating you should check the following parameters:

* A - The liquid temperature. It should be between 1000 and 6000 (10°C to 60°C)
* B - The temperature of the PCB (Printed Circuit Board). It should be between 1000 and 7000
* AA - The target temperature. It should be between 1000 and 6000 (10 to 60°C)
* FLAG_PID_CONTROL - The PARAM_STATUS bit FLAG_PID_CONTROL should be 1

The heating will also depend of the status of the weight.


### Disabling heating

Here are the important parameters:

| PARAM_TEMP_LIQ           |  0 |  A | temperature of the solution      |
| PARAM_TEMP_PCB           |  1 |  B | temperature of the heating plate |
| PARAM_TEMP_TARGET        | 26 | AA | target temperature of the liquid |

In order to disable heating just put at target temperature for the board of 0.
`AA0`

[Next section](../onewire)