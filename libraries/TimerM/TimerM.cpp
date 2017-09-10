/*
  TimerM - Library for running pieces of code on specific interval
  using millis()
  This is a free code that you can use as you want
  Created by Masaki, 9/2017
  http://www.git.com/masakik/timerm

*/


#ifndef TIMERM
#define TIMERM

#include "Arduino.h"
#include "TimerM.h"

unsigned long tmr = 0, _tempo;

TimerM::TimerM() {}

bool TimerM::run(unsigned long intervalo) {
  if (tmr == 0 || millis() - tmr > intervalo * 1000) //roda o timer
  { 
    tmr = millis();
    return true;
  } 
  else 
  {
    return false;
  } 
}

bool TimerM::countdown(const char action[], unsigned long tempo) {
  if (strcmp(action,"start")==0) // inicia o countdown com o tempo fornecido
  {
    if (tmr == 0) {
      tmr = millis();
      _tempo = tempo;
      return true;
    }
  } 
  else if (strcmp(action,"finish")==0) // se terminou retorna true
  {
    if (tmr > 0 && millis() - tmr > _tempo * 1000) {
      tmr = 0;
      return true;
    } else {
      return false;
    }
  } 
  else if (strcmp(action,"status")==0)  // verifica se o timer esta rodando
  { 
    if (tmr > 0) return true;
    else return false;
  }
}


#endif
