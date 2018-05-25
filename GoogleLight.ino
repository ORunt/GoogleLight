#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

WiFiManager wifiManager;

#define WIFI_SSID "CamsBay"
#define WIFI_PASS "randal5544"

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "ORunt"
#define MQTT_PASS "ee0c9164f35547eda9046b48ce5843ab"

#define SERIAL_BAUDRATE   115200
#define RELAY_PASSAGE   4
#define RELAY_KITCHEN   5
#define RELAY_LOUNGE    16
const int  btn_passage = 14;
const int  btn_kitchen = 12;
const int  btn_lounge = 13;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
Adafruit_MQTT_Subscribe lounge = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/jasonlounge");
Adafruit_MQTT_Subscribe passage = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/jasonpassage");
Adafruit_MQTT_Subscribe kitchen = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/jasonkitchen");

void MQTT_connect();

void setup() {
  pinMode(RELAY_PASSAGE, OUTPUT);
  pinMode(btn_passage, INPUT);
  pinMode(RELAY_KITCHEN, OUTPUT);
  pinMode(btn_kitchen, INPUT);
  pinMode(RELAY_LOUNGE, OUTPUT);
  pinMode(btn_lounge, INPUT);
  
  digitalWrite(RELAY_PASSAGE, LOW);
  digitalWrite(RELAY_KITCHEN, LOW);
  digitalWrite(RELAY_LOUNGE, LOW);
  
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);

  // Wifi
  wifiSetup();
  
  mqtt.subscribe(&lounge);
  mqtt.subscribe(&passage);
  mqtt.subscribe(&kitchen);
  Serial.printf("\nSubscribed\n\n");
}

void loop() {
  MQTT_connect();

  //Read from our subscription queue until we run out, or
  //wait up to 5 seconds for subscription to update
  Adafruit_MQTT_Subscribe * subscription;
  
  while ((subscription = mqtt.readSubscription(5000)))
  {
    if(subscription == &lounge){
      Serial.print("lounge: ");
      Serial.print((char*) lounge.lastread);
      SwitchRelay((char*) lounge.lastread, RELAY_LOUNGE);
    }
    else if (subscription == &passage){
      Serial.print("passage: ");
      Serial.print((char*) passage.lastread);
      SwitchRelay((char*) passage.lastread, RELAY_PASSAGE);
    }
    else if (subscription == &kitchen){
      Serial.print("kitchen: ");
      Serial.print((char*) kitchen.lastread);
      SwitchRelay((char*) kitchen.lastread, RELAY_KITCHEN);
    }
  }

  // ping the server to keep the mqtt connection alive
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }
}

void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  { 
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!\n");
}

void wifiman(){
    WiFiManager wifiManager;
    
    if (!wifiManager.startConfigPortal("SharpTech0001")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    Serial.println("connected...yeey :)");
}

uint8_t CheckSetupPins(){
  if (( digitalRead(btn_passage) == HIGH ) && (digitalRead(btn_lounge) == HIGH ))
  {
    delay(3000);
    return ( digitalRead(btn_passage) == HIGH ) && (digitalRead(btn_lounge) == HIGH );
  }
  return 0;
}

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Trying to connect ");

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        if(CheckSetupPins()){
          wifiman();
        }
        delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void SwitchRelay(char* state, int relay){
  if (!strcmp(state, "on")){
    digitalWrite(relay, HIGH);
    Serial.println(" - Relay went on");
  }
  else if (!strcmp(state, "off")){
    digitalWrite(relay, LOW);
    Serial.println(" - Relay went off");
  }
  else{
    Serial.print(" - Relay didn't trigger:");
    Serial.println(state);
  }
}

