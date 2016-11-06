/* sensorBoard.cpp Library for my Sensor Board
   by Augusto Ciuffoletti 22/10/2016
*/
#include <Arduino.h>
#include "sensorBoard.h"

sensorBoard::sensorBoard() {
  // initialize the digital pin as an output.
  pinMode(tank, INPUT);
  pinMode(sensorEnable, OUTPUT);
}

void sensorBoard::on() {
  digitalWrite(sensorEnable, HIGH);
}

void sensorBoard::off() {
  digitalWrite(sensorEnable, LOW);
}

int sensorBoard::read_light() {
  int i;
  int sum=0;
  for ( i=0; i<5; i++ ) {
    sum = analogRead(FR) + sum;
    delay(100);
  }
  return sum/5;
}

float sensorBoard::levelToDegrees(float level) {
        float r = 12000;
        float a = -19.39;
        float b = 201.38+3.4;
        return a*log(r/((1024.0/level)-1))+b;
}

float sensorBoard::read_temp() {
  int i;
  float sum=0;
  for ( i=0; i<5; i++ ) {
    sum = analogRead(NTC) + sum;
    delay(100);
  }
  return levelToDegrees(sum/5);
}

int sensorBoard::read_soil() {
  int i;
  int sum=0;
  // YL power supply only when used (electrolysis)
  delay(500);
  for ( i=0; i<5; i++ ) {
    sum = analogRead(Soil) + sum;
    delay(100);
  }
  return sum/5;
}

int sensorBoard::read_tank() {
  return digitalRead(tank);
}
