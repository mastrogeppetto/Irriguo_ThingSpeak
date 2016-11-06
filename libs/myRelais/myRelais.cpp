/* sensorBoard.cpp Library for my Sensor Board
 * !!! REVERSE LOGIC !!!
 *   by Augusto Ciuffoletti 22/10/2016
*/
#include <Arduino.h>
#include "myRelais.h"

myRelais::myRelais() {
  pinMode(_led, OUTPUT);
  pinMode(_motor, OUTPUT);
  digitalWrite(_led, LOW);  // turn the LED off
  digitalWrite(_motor, HIGH);  // turn the MOTOR off 

}

// Duration in seconds
void myRelais::irrigationCycle(int durata) {
  digitalWrite(_led, HIGH);  // turn the LED on 
  digitalWrite(_motor, LOW);  // turn the MOTOR on (HIGH is the voltage level)
  Serial.println("Motore acceso");
  delay(durata*1000);
  digitalWrite(_led, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(_motor, HIGH);  // turn the MOTOR off by making voltage HIGH
  Serial.println("Motore spento");
}

