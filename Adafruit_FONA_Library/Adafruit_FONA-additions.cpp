
/********* GPRS **********************************************************/

// Add this method/function to Adafruit_FONA.cpp

boolean Adafruit_FONA::jamesStart() {
  getReply(F("AT+COPS?"));
  getReply(F("AT+CSQ"));

  sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000);
  //attach to GPRS
  if (! sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
   return false;
     
     // single connection at a time
  if (! sendCheckReply(F("AT+CIPMUX=0"), F("OK"), 10000)) return false;


   // manually read data
  if (! sendCheckReply(F("AT+CIPRXGET=1"), F("OK"), 10000)) return false;

   //AT+CSTT command to set APN  
 sendCheckReply(F("AT+CSTT=\"hologram\",\"\",\"\""), F("OK"), 10000);
 // sendCheckReply(F("AT+CSTT=\"wholesale\",\"\",\"\""), F("OK"), 10000);

//bring up wireless connection (GPRS)     
  if (! sendCheckReply(F("AT+CIICR"), F("OK"), 10000))  return false;

   //return assigned ip address. This is necessary, otherwise "operation not allowed" error
  getReply(F("AT+CIFSR"));
 
  return true;
}



/***************************************************
This modifies the original library, which includes the following information:

  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/