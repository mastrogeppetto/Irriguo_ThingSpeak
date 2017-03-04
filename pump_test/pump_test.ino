#include <SD.h>


 /*
  Sensor board.
   Power from D6 pin (red) only when sensing.
   Photoresistor between A4 (27, orange) and gnd, 100Kohm resistor between a4 and 5V pin.
   NTC 10K between A5 (28, yellow) and gnd, 18Kohm resistor between a5 and 5 V pin.
   Analog output of YL soil humidity sensor on pin A3 (green).
   Tank empty sensor D5 (blue).
  Relay board
   Relay control on pin D8 (13)
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

//#include <FishinoSockBuf.h>
#include <Fishino.h>
#include <SPI.h>
#include <sensorBoard.h>
#include <myRelais.h>
//#include <math.h>
#include "secret.h"

// Initialize the Fishino client library
FishinoClient client;
sensorBoard sensor;
myRelais relais;
// Digital output
int led = 0;
// constants
unsigned long int ontime_min = 10; // irrigation length (seconds)
unsigned long int ontime_max= 30; // irrigation length (seconds)
unsigned long int ontime = 20; // irrigation length (seconds)
int FR_threshold = 300; // night detection threshold
int Soil_threshold = 500; // soil dry detection
// state during last sense cycle
boolean is_dark; // true if light below threshold
boolean is_dry;  // true if soil dryness above threshold
// timing
unsigned long int lastCycle = 0;
unsigned long int lastWakeUp = 0;
// General purpose buffer
#define BUFLEN 150
char buffer[BUFLEN+1];


// the setup routine runs once when you press reset:
void setup() {
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
//unsigned long int lapse(unsigned long int t0) {
//  return (millis()-t0)/1000L;
//}
//
// convert seconds to minutes/hours/days
//unsigned long int seconds(unsigned long int s) { return s; }
//unsigned long int minutes(unsigned long int s) { return s*60; }
//unsigned long int hours(unsigned long int s) { return s*60*60; }
//unsigned long int days(unsigned long int s) { return s*24*60*60; }


//void report()
//{
//  int i=0; // buffer index
//  // Reading sensors
//  Serial.print("Report: ");
//  sensor.on();
//  int l=-(sensor.read_light()-FR_threshold);
//  float t=sensor.read_temp();
//  int s=sensor.read_soil();
//  int w=sensor.read_tank();
//  sensor.off();
//  
//  // Produce report
//  char uri[]="GET http://api.thingspeak.com/update?api_key=";
//  strcpy(buffer+i,uri);
//  i+=sizeof(uri)-1;
//
//  char apikey[]=API_KEY;
//  strcpy(buffer+i,apikey);
//  i+=sizeof(apikey)-1;
//
//  sprintf(buffer+i,"&field1=");
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  itoa(l,buffer+i,10);
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  
//  sprintf(buffer+i,"&field2=");
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  dtostrf(t,4,2,buffer+i);
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//
//  sprintf(buffer+i,"&field3=");
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  dtostrf(lapse(lastCycle)/(60.0*60.0),4,2,buffer+i);
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
// 
//  sprintf(buffer+i,"&field4=");
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  itoa(s,buffer+i,10);
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
// 
//  sprintf(buffer+i,"&field5=");
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  itoa(w,buffer+i,10);
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
// 
//  sprintf(buffer+i,"&field6=");
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
//  itoa(ontime,buffer+i,10);
//  while (buffer[i] != '\0' && i < BUFLEN ) i++;
// 
///*  String report="&field1="+String(l,DEC)+ \
//                "&field2="+String(t,2)+ \
//                "&field3="+String(lapse(lastCycle)/(60.0*60.0),2)+   \
//                "&field4="+String(s,DEC)+
//                "&field5="+String(w,DEC)+
//                "&field6="+String(ontime,DEC);
//
//*/ 
//  Serial.println(buffer);
//  // safe reset of Wifi module
//  client.stop();
//  // Set system state variables
//  is_dark = ( l < 0 );
//  is_dry = ( s > Soil_threshold );
//  // Connect with ThingSpeak server and send report
//  if (client.connect("api.thingspeak.com",80))
//  {
////    client.println("GET http://api.thingspeak.com/update?api_key="+String(API_KEY)+String(buffer));
//    client.println(buffer);
//    Serial.println("Report sent");
//    client.stop();
//  }
//  else
//  {
//    Serial.println("Connection failed");
//  }
//}

//int readCommand(char* buffer) {
//  char* ptr=buffer;
//  int linelength=0;
//  boolean startline=true;
//  boolean body=false;
//  char correct_startline[]="HTTP/1.1 200 OK";
//  // TCP connection
//  Serial.println("connecting...");
//  if (client.connect("api.thingspeak.com", 80)) {
//    Serial.println("connected");
//    // Make a HTTP GET request:
//    client.println("GET https://api.thingspeak.com/talkbacks/10449/commands/execute?api_key="+String(API_KEY)+" HTTP/1.1");
////    client.println("Host: 184.106.153.149");
//    client.println("Connection: close");
//    client.println();
//  } else {
//    // if you didn't get a connection to the server:
//    Serial.println("connection failed");
//    return 0;
//  }
//  // Wait response
//  long int starttime=millis();
//  while ( millis()-starttime < 10000 ) {
//  // if there are incoming bytes available
//  // from the server, read them and print them:
//    if (client.available()) {
//      *ptr = client.read();
//      if ( *ptr == '\n' ) {
//        // Process a new line
//        Serial.print(".");
//        if ( startline ) {
//          *ptr='\0';
//          int i=0;
//          while ( buffer[i] != '\0' ) {
//            if ( atoi(buffer[i]) != atoi(correct_startline[i]) ) {
//              Serial.println("Negative response");
//              return 0;
//            }
//            i++;
//          }
//          Serial.println("Response OK");
//          startline=false;
//        }
//        ptr=buffer;
//        linelength=0;
//      } else {
//        ptr++;
//        linelength++;
//      }
//// in the buffer the command
//    }
//  // if the server's disconnected, stop the client:
//    if (!client.connected()) {
//      *ptr='\0';
//      Serial.print("\n -->");
//      Serial.println(buffer);
//      Serial.println("disconnecting.");
//      client.stop();
//      return 1;
//    }
//  }
//  // Timeout
//  Serial.println("Timeout");
//  return 0;
//}

void loop() {
  Serial.println("New loop");
  relais.irrigationCycle(30); /* pump stays on for 30 secs */
  delay(10000,true);
}

