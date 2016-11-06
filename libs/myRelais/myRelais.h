/* Ad uso esclusivo di Irriguo_Thingspeak */ 

#ifndef myRelais_h
#define myRelais_h

#include <Arduino.h>

class myRelais {
  public:
// Constructor
    myRelais();
    void irrigationCycle(int durata);
  private:
    int _motor=8;
    int _led=0;
};

#endif myRelais_h

