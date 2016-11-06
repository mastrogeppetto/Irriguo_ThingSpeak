#include <myRelais.h>

myRelais relais;

void setup() {
  Serial.begin(115200);
}

void loop() {
  relais.irrigationCycle(10);
  delay(5000);
}
