#ifndef sensorBoard_h
#define sensorBoard_h

#include <Arduino.h>

class sensorBoard {
  public:
// Constructor
    sensorBoard();
    void on();
    void off();
    int read_light();
    float read_temp();
    int read_soil();
    int read_tank();
   private:
  // Digital output
    int sensorEnable = 6;
  // Digital input
    int tank = 5;
  // Analog input
    int Soil = A3;
    int NTC = A4;
    int FR = A5;
  // conversion
    float levelToDegrees(float level);
};

#endif sensorBoard_h
