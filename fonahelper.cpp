#include "Adafruit_FONA.h"

#define halt(s) { Serial.println(F( s )); while(1);  }

extern Adafruit_FONA fona;
//extern HardwareSerial fonaSerial;
HardwareSerial *fonaSerial = &Serial1;


uint8_t type;

boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password) {
 // Watchdog.reset();

  Serial.println(F("Initializing FONA....(May take 3 seconds)"));
  
  fonaSerial->begin(115200); // if you're using software serial
  
  if (! fona.begin(*fonaSerial)) {           // can also try fona.begin(Serial1) 
    Serial.println(F("Couldn't find FONA"));
    return false;
  } 

  fonaSerial->println("AT+CMEE=2");
  Serial.println(F("FONA is OK"));
  //Watchdog.reset();
  type = fona.type();
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }


  Serial.println(F("Checking for network..."));
  //while (fona.getNetworkStatus() != 1) {
  // delay(500);
  //}
  uint8_t n;
  boolean success=false;
  do {
    n = fona.getNetworkStatus();
    Serial.print(F("Network status "));
    Serial.print(n);
    Serial.print(F(": "));
    if (n == 0) Serial.println(F("Not registered"));
    if (n == 1) Serial.println(F("Registered (home)"));
    if (n == 2) Serial.println(F("Not registered (searching)"));
    if (n == 3) Serial.println(F("Denied"));
    if (n == 4) Serial.println(F("Unknown"));
    if (n == 5) Serial.println(F("Registered roaming"));
    delay(500);
    if ((n == 1) || (n==5))
      success = true;
  } while(success == false);
  

  Serial.println("5 second pause...");
  delay(5000);  // wait a few seconds to stabilize connection

   // replaced "enableGPRS with my own"
   fona.jamesStart();
  return true;
}
