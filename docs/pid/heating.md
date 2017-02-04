# Disabling fill in / out


# Disabling heating

Here are the important parameters:

| PARAM_TEMP_LIQ           |  0 |  A | temperature of the solution      |
| PARAM_TEMP_PCB           |  1 |  B | temperature of the heating plate |
| PARAM_TEMP_TARGET        | 26 | AA | target temperature of the liquid |
| PARAM_TEMP_MAX           | 27 | AB | maximal temperature of the liquid |

In order to disable heating just put at maximal temperature for the board of 0.


# Disabling agitation







  #define PARAM_WEIGHT_MIN           29
  #define PARAM_WEIGHT_MAX           30
  #define PARAM_SEDIMENTATION_TIME   31  // MINUTES to wait without rotation before emptying
  #define PARAM_FILLED_TIME          32  // MINUTES to stay in the filled state



