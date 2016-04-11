#define THR_LORA_SEND 1

#ifdef THR_LORA_SEND




NIL_WORKING_AREA(waThreadLoraSend, 0);
NIL_THREAD(ThreadLoraSend, arg) {


  while (TRUE) {
    
    sendLoraCompactParameters(&Serial, 16);
    
    nilThdSleepMilliseconds(1000*300);
  }



}

#endif


