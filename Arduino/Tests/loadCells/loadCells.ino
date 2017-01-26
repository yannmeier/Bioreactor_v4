#include "HX711.h"
  #define WEIGHT_DATA        22
  #define WEIGHT_CLK         23     
  
HX711 scale(WEIGHT_DATA, WEIGHT_CLK);
float weight;

void setup() {
  // put your setup code here, to run once:
    delay(1000);
  Serial.begin(9600);
  delay(1000);
}

void loop() {
   uint16_t offset=39552;
   uint16_t factor=32849;
     //to be improved (change calib values for the HX711)
    weight= (int)round((float)(scale.read_average(50)+(long)offset*10)/(-1*(float)factor/100));
    Serial.println(weight);
}
