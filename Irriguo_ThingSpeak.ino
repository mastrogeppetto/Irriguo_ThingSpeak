 /*
  Analog output of YL soil humidity sensor on pin A3. Power with 5V from arduino.
  YL power supply (5V) on pin A6 (only when sensing)
  Photoresistor between a4 (27) and gnd, 100Kohm resistor between a4 and 5V pin.
  NTC 10K between A5 (28) and gnd, 18Kohm resistor between a5 and 5 V pin.
  Relay control on pin D9 (13)
  Control LED on pin D8 (12) 
  Pin D12 (16) non usable (MISO)
  Pin D7 not usable (?)
  When a4 above threshold value (e.g. 700), detect night darkness.

  IMPORTANT: secret.h header file contains credentials. Example:
  === cut here ===
  // SECRETS
  #define MY_SSID "homewifi" // wifi ssid
  #define MY_PASS "123"      // wifi key
  #define API_KEY "ABABABABABABABAB" // ThingSpeak WRITE KEY
  === end ===
 */

#include <FishinoSockBuf.h>
#include <Fishino.h>
#include <SPI.h>
#include "secret.h"

// Initialize the Fishino client library
FishinoClient client;

// PIN assigments
// Digital output
int led = 8;
int motor = 9;
int soilEnable = 6;
// Digital input
int tank = 0;
// Analog input
int Soil = A3;
int FR = A4;
int NTC = A5;
// constants
unsigned long int ontime = 10; // irrigation length (seconds)
unsigned long int lastCycle = 0;
int FR_threshold = 300; // night detection threshold
int Soil_threshold =500; // soil dry detection
 
// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  pinMode(motor, OUTPUT);
  pinMode(soilEnable, OUTPUT);
  Serial.begin(115200);
  // === WIFI SETUP ===
  // Initialize SPI for Wifi use
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  // Reset module
  Fishino.reset();
  Serial.println("Wifi module OK");
  // Setup Wifi mode and register with AP
  Fishino.setMode(STATION_MODE);
  while(!Fishino.begin(MY_SSID, MY_PASS))
    Serial.println("Connection with AP failed: retrying...\n");
  // Run DHCP
  Fishino.staStartDHCP();
  Serial.print("Waiting DHCP server offer..");
  while(Fishino.status() != STATION_GOT_IP)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(Fishino.localIP());
  // === END WIFI SETUP ===
}

int read_light() {
  int i;
  int sum=0;
  for ( i=0; i<5; i++ ) {
    sum = analogRead(FR) + sum;
    delay(100);
  }
  return sum/5;
}

float read_temp() {
  int i;
  int sum=0;
  for ( i=0; i<5; i++ ) {
    sum = analogRead(NTC) + sum;
    delay(100);
  }
  return ((0.00488 * (sum/5))*(-16.9))+56.7;
}

int read_soil() {
  int i;
  int sum=0;
  // YL power supply only when used (electrolysis)
  digitalWrite(soilEnable, HIGH);
  delay(500);
  for ( i=0; i<5; i++ ) {
    sum = analogRead(Soil) + sum;
    delay(100);
  }
  digitalWrite(soilEnable, LOW);
  return sum/5;
}


// TRUE if dark
boolean is_dark() {
  return read_light() > FR_threshold;
}

// Delay for given time (in secs) flashing or not the LED
void delay( unsigned long int s, boolean flash) {
  int i=0;
  unsigned long int th = s * 1000;
  unsigned long initial = millis();
  while ( millis() - initial < th ) {
    if ( flash ) {
        if ( (i % 10 == 0)) {
        digitalWrite(led, HIGH);
      } else {
        digitalWrite(led,LOW);
      }
    }
    delay(100); // wait for 0.1 second
    i++;
  }
}

// Returns time elapsed from timestamp (in secs) 
unsigned long int lapse(unsigned long int t0) {
  return (millis()-t0)/1000L;
}

// convert seconds to minutes/hours/days
unsigned long int seconds(unsigned long int s) { return s; }
unsigned long int minutes(unsigned long int s) { return s*60; }
unsigned long int hours(unsigned long int s) { return s*60*60; }
unsigned long int days(unsigned long int s) { return s*24*60*60; }


// Duration in seconds
void IrrigationCycle(int durata) {
  digitalWrite(led, HIGH);  // turn the LED on (HIGH is the voltage level)
  digitalWrite(motor, HIGH);  // turn the MOTOR on (HIGH is the voltage level)
  Serial.println("Motore acceso");
  delay(seconds(durata), false);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(motor, LOW);  // turn the MOTOR off
  Serial.println("Motore spento"); 
  lastCycle=millis();
}

void report()
{
  Serial.print("Report: ");
  String l=String(-(read_light()-FR_threshold),DEC);
  String t=String(read_temp(),2);
  String s=String(read_soil(),DEC);
  String d=String(lapse(lastCycle)/(60.0*60.0),2);
  String report="&field1="+l+"&field2="+t+"&field3="+d+"&field4="+s;
  Serial.println(report);
  // safe reset of Wifi module
  client.stop();
  // Connect with ThingSpeak server
  if (client.connect("api.thingspeak.com",80))
  {
    client.println("GET http://api.thingspeak.com/update?api_key="+String(API_KEY)+report);
    Serial.println("Report sent");
    client.stop();
  }
  else
  {
    Serial.println("Connection failed");
  }
}


void loop() {
  Serial.println("New loop");
  report();
  Serial.println(lapse(lastCycle));
  if ( (lapse(lastCycle) > hours(23)) || (lastCycle == 0) ) {
    if ( is_dark() ) { 
      IrrigationCycle(ontime);
    }
  }
  delay(minutes(15),true);
}

