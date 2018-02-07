/* 
Adafruit.IO FONA on Teensy 3.2 GPS Tracker.

Written by james@baldengineer.come, based on Adafruit Adafruit MQTT Library FONA Example
(See related information at bottom.)

Major changes:
   - Removed watchdog support, wasn't clear if Teensy supported it
   - Using Teensy 3.2 board. (Will not work on 8-bit AVR boards like the Uno)
   - Modified the FONA Library itself. I didn't know any better.
*/

#include "Adafruit_FONA.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_FONA.h"

#define halt(s) { Serial.println(F( s )); while(1);  }

/*************************** FONA Pins ***********************************/
#define FONA_RST 3

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
/************************* WiFi Access Point *********************************/

// hologram doesn't use a username and password

#define FONA_APN       "hologram"
#define FONA_USERNAME  ""
#define FONA_PASSWORD  ""

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "baldengineer"
#define AIO_KEY         ""

// Setup the FONA MQTT class by passing in the FONA class and MQTT server and login details.
Adafruit_MQTT_FONA mqtt(&fona, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// FONAconnect is a helper function that sets up the FONA and connects to
// the GPRS network. See the fonahelper.cpp tab above for the source!
boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password);

/*************************** MQTT Feeds **************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

// Send data to the map widget as "csv", only way that seemed to work, json didn't
Adafruit_MQTT_Publish maploc = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/map/csv");

Adafruit_MQTT_Publish altitudeTopic = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/altitude");


/*************************** Sketch Code ************************************/

// How many transmission failures in a row we're willing to be ok with before reset
uint8_t txfailures = 0;
#define MAXTXFAILURES 3

const int statusLED = 13;

void jamesHalt() {
  Serial.println("Halting");
  while(1);
}

unsigned long previousMillis;
unsigned long currentMillis;

// Maps tracker
unsigned long gpsInterval = 10000;
unsigned long gpsPreviousMillis;
float latitude, longitude, speed_kph, heading, speed_mph, altitude;

void handleSerial() {
  byte incomingChar = Serial.read();
  // a way to stop all activity (requires reset)
  if (incomingChar == '!') jamesHalt();

  // force reconnecting, works well when IP fails and getting DEAC errors
  if (incomingChar == '#') FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD));

  // Temp pause all activity
  if (incomingChar == '@') {
    Serial.println(F("Pausing..."));
    boolean pause = true;
    while(pause) if (Serial.read() == '@') pause = false;
    Serial.println(F("Unpausing..."));
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("[Connecting] Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    Serial.println(F("[Connecting] Pausing for 5 seconds"));
    delay(5000);  // wait 5 seconds
    handleSerial();
  }
  Serial.println("MQTT Connected!");
}

void setup() {
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, HIGH);

  // give time for everything to start up
  delay(5000);  

  // Hai
  Serial.begin(115200);
  Serial.println(F("Adafruit.IO FONA on Teensy 3.2 GPS Tracker"));
  Serial.println(F("Build: 1,024"));

 
 // Initialise the FONA module
  while (! FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD)))
    Serial.println("Retrying FONA");

  Serial.println(F("Connected to Cellular!"));

  //Watchdog.reset();
  delay(2500);  // wait a few seconds to stabilize connection
 // Watchdog.reset();
  digitalWrite(statusLED, LOW);

  Serial.print(F("Turning on GPS..."));
  fona.enableGPS(true);
  Serial.println(F("done"));

}

void loop() {
  handleSerial();

  currentMillis = millis();
  
  if (currentMillis - gpsPreviousMillis >= gpsInterval) {
    // get coordinates, if we can
    boolean gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);
    if (gps_success) {
      digitalWrite(statusLED, HIGH);
      speed_mph = speed_kph * 0.621371192; 

      Serial.print(F("GPS lat:")); Serial.println(latitude);
      Serial.print(F("GPS long:")); Serial.println(longitude);
      Serial.print(F("GPS speed KPH:")); Serial.println(speed_kph); 
      Serial.print(F("GPS speed MPH:")); Serial.println(speed_mph);
      Serial.print(F("GPS heading:")); Serial.println(heading);
      Serial.print(F("GPS altitude:")); Serial.println(altitude);
    
      //{"value":1,"lat":-038.350127, "lon":-121.181398}
      // sprintf with floats only works on non-AVR arduino (like the Teensy)
      char locString[128];
      sprintf(locString, "%d,%f,%f,%f", millis(),latitude, longitude, altitude);
      
      MQTT_connect();
      
      Serial.print(F("\nSending to topic 'map': "));
      Serial.print(locString);
      Serial.print("...");
      if (! maploc.publish(locString)) {
        Serial.println(F("Failed to send LOC"));
        txfailures++;
        if (txfailures >= MAXTXFAILURES) {
          Serial.println(F("Resetting Cellular"));
          FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD));
          txfailures = 0;
        }
      } else {
        Serial.println(F("SENT LOC!"));
        txfailures = 0;
      }  

      if (! altitudeTopic.publish(altitude)) {
        Serial.println(F("Failed to send ALTITUDE"));
        txfailures++;
        if (txfailures >= MAXTXFAILURES) {
          Serial.println(F("Resetting Cellular"));
          FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD));
          txfailures = 0;
        }
      } else {
        Serial.println(F("SENT ALT!"));
        txfailures = 0;        
      }
    } else {
      digitalWrite(statusLED, LOW);
      Serial.println(F("### NO GPS FIX!!"));
      Serial.print(F("Turning on GPS...(again)"));
      fona.enableGPS(true);
    }
      gpsPreviousMillis = currentMillis;
  }
}

/* This code doesn't subscribe to anything.
  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  } */

/***************************************************
  This code is heavily based on:
  Adafruit MQTT Library FONA Example

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963
  ----> http://www.adafruit.com/products/2468
  ----> http://www.adafruit.com/products/2542

  These cellular modules use TTL Serial to communicate, 2 pins are
  required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  ---> AT+CPMS="SM","SM","SM"
  <--- +CMS ERROR: operation not allowed
 ****************************************************/
