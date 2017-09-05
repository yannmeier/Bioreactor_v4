#if defined(FOOD_CTRL) && defined(WEIGHT_DATA) && defined(WEIGHT_CLK)


HX711 scale(WEIGHT_DATA, WEIGHT_CLK);


#ifdef DEBUG_WEIGHT
NIL_WORKING_AREA(waThreadWeight, 96);
#else
NIL_WORKING_AREA(waThreadWeight, 56); // minimum of 32 !
#endif


NIL_THREAD(ThreadWeight, arg) {
  nilThdSleepMilliseconds(1234); // wait a little bit not everything starts at once

  /********************************************
               initialisation
  ********************************************/
  int weight;

  long timeLastEvent = millis() - getParameter(PARAM_WEIGHT_SINCE_LAST_EVENT) * 60000;

  /********************************************
               Thread Loop
  ********************************************/
  while (true) {
    setPumps();
    nilThdSleepMilliseconds(1000);

    int sinceLastEvent = (int)((millis() - timeLastEvent) / 60000);
    weight = getWeight(); //sensor read, better to have a higher value if the weight increase
    setParameter(PARAM_WEIGHT, weight);
    setParameter(PARAM_WEIGHT_G, convertWeightToG(weight));

    if (! isRunning(FLAG_FOOD_CONTROL) || // Foold control is currently disabled
        ! isEnabled(FLAG_FOOD_CONTROL)) { // Food control is disabled
      stop(FLAG_RELAY_FILLING);    //filling  OFF
      stop(FLAG_RELAY_EMPTYING);   //emptying OFF
      continue;
    }

    // are we outside ranges ?

    int error = (getParameter(PARAM_WEIGHT_MAX) - getParameter(PARAM_WEIGHT_MIN)) / 5;

#ifdef DEBUG_WEIGHT
    Serial.print(F("Weight allowed error:"));
    Serial.println(error);
#endif

    if ((error > 0 && ((weight < (getParameter(PARAM_WEIGHT_MIN) - error)) || (weight > (getParameter(PARAM_WEIGHT_MAX) + error)))) ||
        (error < 0 && ((weight > (getParameter(PARAM_WEIGHT_MIN) - error)) || (weight < (getParameter(PARAM_WEIGHT_MAX) + error))))) {
      saveAndLogError(true, FLAG_WEIGHT_RANGE_ERROR);
#ifdef DEBUG_WEIGHT
      Serial.print(F("Weight range error:"));
      Serial.println(error);
#endif
    } else {
      saveAndLogError(false, FLAG_WEIGHT_RANGE_ERROR);
#ifdef DEBUG_WEIGHT
      Serial.print(F("Weight range ok:"));
      Serial.println(error);
#endif
    }

#ifdef DEBUG_WEIGHT
    Serial.print(F("Weight:"));
    Serial.println(weight);
#endif

    if (isError(MASK_WEIGHT_ERROR)) {  // weight outside range
#ifdef DEBUG_WEIGHT
      Serial.print(F("Weight disabled:"));
      Serial.println(weight);
#endif
      stop(FLAG_RELAY_FILLING);     //filling  OFF
      stop(FLAG_RELAY_EMPTYING);    //emptying OFF
      continue;
    }


    if (isRunning(FLAG_SEDIMENTATION)) {
      // just to sure in case more than one flag is active we stop the pumps
      stop(FLAG_RELAY_EMPTYING);
      stop(FLAG_RELAY_FILLING);
      if (sinceLastEvent >= getParameter(PARAM_SEDIMENTATION_TIME)) {  //switch to Emptying
        stop(FLAG_SEDIMENTATION);
        start(FLAG_RELAY_EMPTYING);
        timeLastEvent = millis();
      }
    } else if (isRunning(FLAG_RELAY_EMPTYING)) {
      // just to sure in case more than one flag is active we stop the pumps
      stop(FLAG_RELAY_FILLING);
      if ((error > 0 && weight <= getParameter(PARAM_WEIGHT_MIN)) ||
          (error < 0 && weight >= getParameter(PARAM_WEIGHT_MIN))) {      //switch fo Filling
        stop(FLAG_RELAY_EMPTYING);
        start(FLAG_RELAY_FILLING);
      }
    } else if (isRunning(FLAG_RELAY_FILLING)) {
      if ((error > 0 && weight >= getParameter(PARAM_WEIGHT_MAX)) ||
          (error < 0 && weight <= getParameter(PARAM_WEIGHT_MAX))) {
        stop(FLAG_RELAY_FILLING);
        timeLastEvent = millis();
      }
    } else {
      if (sinceLastEvent >= getParameter(PARAM_FILLED_TIME)) {          //switch to Sedimentation
        start(FLAG_SEDIMENTATION);
        timeLastEvent = millis();
      }
    }

    setParameter(PARAM_WEIGHT_SINCE_LAST_EVENT, sinceLastEvent);
  }
}



void printWeightHelp(Print* output) {
  output->println(F("Weight help"));
  output->println(F("(we) Empty (tare)"));
  output->println(F("(wk) Empty + 1kg"));
  output->println(F("(wl) Low level"));
  output->println(F("(wh) High level"));
  output->println(F("(wt) Test"));
}

int getWeight() { // we can not avoid to have some errors measuring the weight
  // and currently we don't know where it is coming from
  // so we need to find out what are the correct values and what are the wrong one
  // if there is an error it always end with 00000001
  // we will also need 4 consecutive values that differ less than 10%
  byte counter = 0;
  long weight = 0;
  while (counter < 4) {
    while (!scale.is_ready()) {
      nilThdSleepMilliseconds(10);
    }
    protectThread();
    long currentWeight = scale.read();
    unprotectThread();

    if ((currentWeight & 0b11111111) != 1) {
      if (weight == 0) {
        weight += currentWeight;
        counter++;
      } else {
        int difference = abs(100 - (weight * 100 / counter) / currentWeight);
        if (difference < 10) {
          weight += currentWeight;
          counter++;
        } else {
          weight = 0;
          counter = 0;
        }
      }
      nilThdSleepMilliseconds(10);
    }
  }
  return weight / counter / 100;
}

int convertWeightToG(int weight) {
  return ((long)(weight - getParameter(PARAM_WEIGHT_OFFSET)) * 1000) / getParameter(PARAM_WEIGHT_FACTOR);
}


void processWeightCommand(char command, char* data, Print * output) {
  int weight = getWeight();
  switch (command) {
    case 'e': // define the empty level
      setAndSaveParameter(PARAM_WEIGHT_OFFSET, weight);
      output->println(weight);
      break;
    case 'k': // define the weight one kg over the tare
      setAndSaveParameter(PARAM_WEIGHT_FACTOR, weight - getParameter(PARAM_WEIGHT_OFFSET));
      output->println(weight);
      break;
    case 'l': // define low level
      setAndSaveParameter(PARAM_WEIGHT_MIN, weight);
      output->println(weight);
      break;
    case 'h': // define high level
      setAndSaveParameter(PARAM_WEIGHT_MAX, weight);
      output->println(weight);
      break;
    case 't':
      output->println(F("Current weight"));
      for (byte i = 0; i < 10; i++) {
        int weight = getWeight();
        output->print(weight);
        output->print(F(" = "));
        output->print(convertWeightToG(weight));
        output->println("g");
      }
      break;
    default:
      printWeightHelp(output);
  }
}
#endif

/********************
    Utilities
*********************/

void setPumps() {
  //stop pumping on startup and put down the flags
#ifdef FOOD_IN
  pinMode(FOOD_IN, OUTPUT);
  digitalWrite(FOOD_IN, getStatus(FLAG_RELAY_FILLING));
#endif
#ifdef FOOD_OUT
  pinMode(FOOD_OUT, OUTPUT);
  digitalWrite(FOOD_OUT, getStatus(FLAG_RELAY_EMPTYING));
#endif
}


