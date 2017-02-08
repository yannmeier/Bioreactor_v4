Starting a new bioreactor
=========================

When you have a new bioreactor you should start by configuring it

Initializing parameters
-----------------------

Connect via the terminal and reset all the parameters to the defualt parameters

`r1234`

It is also important to set a unique qualifier for the bioreactor. You should fill the [list](qualifiers.md) with a free
code.


Setup the weight
----------------

There is now a special menu for the weight calibration `w`.

You should do the following step in order to callibrate the weight:

1. Empty bioreactor : `we`
2. Empty bioreactor + 1kg : `wk`
3. Bioreactor filled at low level : `wl`
4. Bioreactor filled at high level : `wh`

You may now check the reproducibility and the weight in g of any object using `wt`

It is important to note that an error (stored in [PARAM_ERROR](/arduino/parameters.md#PARAM_ERROR) will be generated if
the weight is either 20% under the minimal value or 20% over the maximal value and this should stop all the functions.

