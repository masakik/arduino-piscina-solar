/*
  TimerM - Library for running pieces of code on specific interval
  using millis()
  This is a free code that you can use as you want
  Created by Masaki, 9/2017
  http://www.git.com/masakik/timerm

*/


#ifndef TIMERM
#define TIMERM


#include "TimerM.h"
#include "Arduino.h"


unsigned long tmr;


TimerM::TimerM() {
  tmr = 0;
}

bool TimerM::run(unsigned long intervalo) {
  if (millis() - tmr > intervalo * 1000) 
  { //roda o timer
    tmr = millis();
    return true;
  } 
  else 
  {
    return false;
  }
}

#endif
