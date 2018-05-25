#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WIFI_SSID "CamsBay"
#define WIFI_PASS "randal5544"

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "ORunt"
#define MQTT_PASS "ee0c9164f35547eda9046b48ce5843ab"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
Adafruit_MQTT_Subscribe lounge = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/jasonlounge");
Adafruit_MQTT_Subscribe passage = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/jasonpassage");
Adafruit_MQTT_Subscribe kitchen = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/jasonkitchen");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  
  //Connect to WiFi
  Serial.print("\n\nConnecting Wifi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
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
      Serial.println((char*) lounge.lastread);
    }
    else if (subscription == &passage){
      Serial.print("passage: ");
      Serial.println((char*) passage.lastread);
    }
    else if (subscription == &kitchen){
      Serial.print("kitchen: ");
      Serial.println((char*) kitchen.lastread);
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
