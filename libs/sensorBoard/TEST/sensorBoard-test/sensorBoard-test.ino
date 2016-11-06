#include "sensorBoard.h"

sensorBoard sensor;

void setup() {                
    Serial.begin(115200);
}

void loop () {
  Serial.println("Hallo");
//  digitalWrite(13,HIGH);
  sensor.on();
  float l=sensor.read_light();
  Serial.println(l);
  float t=sensor.read_temp();
  Serial.println(t);
  float s=sensor.read_soil();
  Serial.println(s);
  int f=sensor.read_tank();
  sensor.off();
//  digitalWrite(13,LOW);
  delay(10000);
  }
