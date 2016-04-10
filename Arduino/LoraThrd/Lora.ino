
#define LORA_RESET_PIN 9

void processLoraCommand(char command, char* data, Print* output) {
  switch (command) {
  case 'i':
    infoLora(output);
    break;
  case 'r':
    resetLora(output);
    break;
  case 's':
    sendLoraMessage(data, output);
    break;
  case 'a':
    if (data[0]!='\0') {
      writeEEPROM(EE_LORA_APPSKEY, data, 32);
    } 
    readEEPROM(EE_LORA_APPSKEY, EE_LORA_APPSKEY+32, output);
    output->println("");
    break;
  case 'n':
    if (data[0]!='\0') {
      writeEEPROM(EE_LORA_NWKSKEY, data, 32);
    } 
    readEEPROM(EE_LORA_NWKSKEY, EE_LORA_NWKSKEY+32, output);
    output->println("");
    break;
  }
}



void resetLora(Print* output) {
  pinMode(9, OUTPUT); // we put the reset to pin 9
  digitalWrite(9,LOW);
  nilThdSleepMilliseconds(300);
  digitalWrite(9,HIGH);
  loraAnswer(3000, output);
}


void infoLora(Print* output) {
  output->println(F("SYS"));
  infoSysLora("ver", output);
  infoSysLora("vdd", output);
  infoSysLora("hweui", output);

  output->println(F("MAC"));
  infoMacLora("devaddr", output);
  infoMacLora("deveui", output);
  infoMacLora("appeui", output);
  infoMacLora("dr", output);
  infoMacLora("band", output);
  infoMacLora("pwridx", output);
  infoMacLora("adr", output);
  infoMacLora("retx", output);
  infoMacLora("rxdelay1", output);
  infoMacLora("rxdelay2", output);
  infoMacLora("ar", output);
  infoMacLora("dcycleps", output);
  infoMacLora("mrgn", output);
  infoMacLora("gwnb", output);
  infoMacLora("status", output);

  output->println(F("RADIO"));
  infoRadioLora("bt", output);
  infoRadioLora("mod", output);   
  infoRadioLora("freq", output);   
  infoRadioLora("pwr", output);
  infoRadioLora("sf", output);
  infoRadioLora("afcbw", output);
  infoRadioLora("rxbw", output);
  infoRadioLora("bitrate", output);
  infoRadioLora("fdev", output);
  infoRadioLora("prlen", output);
  infoRadioLora("crc", output);
  infoRadioLora("iqi", output);
  infoRadioLora("cr", output);
  infoRadioLora("wdt", output);
  infoRadioLora("bw", output);
  infoRadioLora("snr", output);
}

void infoSysLora(char* data, Print* output) {
  Serial1.print(F("sys get "));
  infoLoraSS(data, output);
}

void infoMacLora(char* data, Print* output) {
  Serial1.print(F("mac get "));
  infoLoraSS(data, output);
}

void infoRadioLora(char* data, Print* output) {
  Serial1.print(F("radio get "));
  infoLoraSS(data, output);
}


void infoLoraSS(char* data, Print* output) {
  Serial1.println(data);
  output->print("  ");
  output->print(data);
  output->print(": ");
  loraAnswer(300, output);
}

void loraAnswer(int waitTime, Print* output) {
  while(--waitTime>0) {
    while(Serial1.available()) {
      output->print((char)Serial1.read());
    }
    nilThdSleepMilliseconds(1);
  } 
}


void initLora(Print* output) {
  Serial1.print(F("mac set nwkskey "));
  printNwksKey(&Serial1);
  Serial1.println("");
  loraAnswer(300, output);
  Serial1.print(F("mac set appskey "));
  printAppsKey(&Serial1);
  Serial1.println("");
  loraAnswer(300, output);
  Serial1.println(F("mac set adr on"));  
  loraAnswer(300, output);
  Serial1.println(F("mac set devaddr 02031202"));
  loraAnswer(300, output);
  Serial1.println(F("mac save"));  
  loraAnswer(3000, output);
}

void sendLoraMessage(char* message, Print* output) {
  Serial1.println(F("radio set pwr -3"));
  loraAnswer(300, output);
  Serial1.println(F("mac join abp"));
  loraAnswer(1000, output);
  Serial.println("SEND");
  Serial1.println(F("mac tx uncnf 1 414243"));
  Serial.println("DONE");
  // Serial1.println(message,HEX);
  loraAnswer(2000, output);
}

void printNwksKey(Print* output) {
  readEEPROM(EE_LORA_NWKSKEY, EE_LORA_NWKSKEY+32, output);
}

void printAppsKey(Print* output) {
  readEEPROM(EE_LORA_APPSKEY, EE_LORA_APPSKEY+32, output);
}


























