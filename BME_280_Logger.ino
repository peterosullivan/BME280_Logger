#include "Secret.h"
#include <Wire.h>
#include "BlueDot_BME280.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "ThingSpeak.h"
#include <math.h>

BlueDot_BME280 bme280 = BlueDot_BME280();

ESP8266WiFiMulti WiFiMulti;
WiFiClient  client;

float humi, alti, pressure, temp;
const int UPDATE_INTERVAL_MINUTES = 60*5;

void setup()   {                
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);

  bme280.parameter.communication = 0;                  //Set to 0 for I2C (default value)
  bme280.parameter.I2CAddress = 0x76;                  //Available by connecting the SDO pin to ground
  bme280.parameter.sensorMode = 0b11;                   //In normal mode the sensor measures continually (default value)
  
  //*********************************************************************
  //Great! Now set up the internal IIR Filter
  //The IIR (Infinite Impulse Response) filter suppresses high frequency fluctuations
  //In short, a high factor value means less noise, but measurements are also less responsive
  //You can play with these values and check the results!
  //In doubt just leave on default
  bme280.parameter.IIRfilter = 0b101;                    //factor 16 (default value)
  bme280.parameter.humidOversampling = 0b101;            //factor 16 (default value)
  bme280.parameter.tempOversampling = 0b101;             //factor 16 (default value)
  bme280.parameter.pressOversampling = 0b101;            //factor 16 (default value)
  
  
  //*********************************************************************
  //For precise altitude measurements please put in the current pressure corrected for the sea level
  //On doubt, just leave the standard pressure as default (1013.25 hPa);
  bme280.parameter.pressureSeaLevel = 1029.3;           //default value of 1013.25 hPa

  //Also put in the current average temperature outside (yes, really outside!)
  //For slightly less precise altitude measurements, just leave the standard temperature as default (15°C);
  bme280.parameter.tempOutsideCelsius = 15;              //default value of 15°C
  
  if (bme280.init() != 0x60){    
    Serial.println(F("Ops! BME280 could not be found!"));
    Serial.println(F("Please check your connections."));
    while(1);
  }
  else{
    Serial.println(F("BME280 detected!"));
  }
}

//*********************************************************************
void loop() 
{   
    
   temp = roundf(bme280.readTempC() * 10) / 10;
   humi = roundf(bme280.readHumidity() * 10) / 10;
   alti = bme280.readAltitudeMeter();
   pressure = bme280.readPressure();

   Serial.print(F("Duration in Seconds:\t\t"));
   Serial.println(float(millis())/1000);
   Serial.print(F("Temperature in Celsius:\t\t")); 
   Serial.println(temp);
   Serial.print(F("Humidity in %:\t\t\t"));
   Serial.println(humi);
   Serial.print(F("Press in hPa:\t\t\t")); 
   Serial.println(pressure);
   Serial.print(F("Alti in Meters:\t\t\t")); 
   Serial.println(alti);
   
   Serial.println();

   ThingSpeak.setField(3,temp);
   ThingSpeak.setField(4,humi);
   ThingSpeak.writeFields(THINKSPEAK_CHANNEL, THINGSPEAK_API_KEY);
  
   delay(1000 * UPDATE_INTERVAL_MINUTES);
}
