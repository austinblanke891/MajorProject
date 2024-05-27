#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include "ThingSpeak.h"
#include <math.h>
#include <Servo.h>
#define TS_ENABLE_SSL

Servo myservo;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

char ssid[] = SECRET_SSID;
char usrnm[] = SECRET_USRNM;
char pass[] = SECRET_PASS; 
WiFiClient  client;

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;
const int pinSoilSensor = A1;
const int pinLightSensor = A2;
const int pinWater = 6;
int home = 120;

#if defined(ARDUINO_ARCH_AVR)
#define debug  Serial
#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
#define debug  SerialUSB
#else
#define debug  Serial
#endif



void setup() {
  // put your setup code here, to run once:

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  Serial.begin(9600);
  myservo.attach(3);
  myservo.write(home);

  ThingSpeak.begin(client);

}

void loop() {
  
  unsigned long time = millis();
  LightOff();
  delay(1000);
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet;
  int light = analogRead(pinLightSensor);
  float soilMoisture = analogRead(pinSoilSensor);

  if( (((time/60000)%15) < 2)){

    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(SECRET_SSID);
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, pass);
        Serial.print(".");
        delay(6000);
      } 
      Serial.println("\nConnected.");
    }

    int w = ThingSpeak.writeField(myChannelNumber, 1, temperature, myWriteAPIKey);
    int x = ThingSpeak.writeField(myChannelNumber, 2, light, myWriteAPIKey);
    int y = ThingSpeak.writeField(myChannelNumber, 3, soilMoisture, myWriteAPIKey);

    if(light < 50){
      LightOn();
      int z = ThingSpeak.writeField(myChannelNumber, 4, 1, myWriteAPIKey);
      }

    delay(120000);
  }

  if(soilMoisture < 700){
    Water(3000);
    }
  
  delay(6000);
}


void Water(int Amount) {
  digitalWrite(pinWater, HIGH);
  delay(Amount);
  digitalWrite(pinWater, LOW);
  delay(5000);
  }

void LightOn(){
    myservo.write(90);
    delay(500);
    myservo.write(home);
  }

void LightOff(){
    myservo.write(90);
    delay(3000);
    myservo.write(home);
  }
