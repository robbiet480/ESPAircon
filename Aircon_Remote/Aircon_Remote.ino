#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "DL_Aircon.h"
#include <IRremoteESP8266.h>
#include <ArduinoJson.h>

#define MQTT_MAXBUFFERSIZE 250
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define IR_PIN 14

#define LED_GREEN 12
#define LED_BLUE 13
#define LED_RED 15
#define BUTTON 4

#include "config.h"

IRsend irsend(IR_PIN); //an IR led is connected to GPIO pin 14
DynamicJsonBuffer  jsonBuffer;

dl_aircon_msg_t msg;
bool recv = false;
unsigned long millis_last = 0;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, mqtt_server, mqtt_port, mqtt_user, "");

Adafruit_MQTT_Subscribe mq_aircon_set = Adafruit_MQTT_Subscribe(&mqtt, set_topic);
Adafruit_MQTT_Publish mq_aircon_status = Adafruit_MQTT_Publish(&mqtt, state_topic);

void MQTT_connect();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(112500);
  Serial.println("Booting");

  WiFi.begin(wifi_ssid, wifi_password);
  Serial.println("");

  pinMode(IR_PIN, OUTPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  //Default settings
  irsend.begin();
  msg.on = false;
  msg.oscillate = false;
  msg.timer = false;
  msg.timer_value = 1;
  msg.speed = 2;
  msg.wind = 2;

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    delay(400);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifi_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&mq_aircon_set);

  Serial.println("Go!");

  digitalWrite(LED_GREEN, HIGH);
  delay(100);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {

  if (millis() - millis_last > 500){
    MQTT_connect();

    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(500))) {
      if (subscription == &mq_aircon_set) {
        digitalWrite(LED_BLUE, HIGH);
        Serial.println("Got MQTT");
        JsonObject& root = jsonBuffer.parseObject((char*)mq_aircon_set.lastread);

        if (!root.success()) {
          Serial.println("parseObject() failed");
          return;
        }

        if (root.containsKey("on")) msg.on = root["on"];

        if (root.containsKey("oscillate")) msg.oscillate = root["oscillate"];

        if (root.containsKey("speed")) msg.speed = root["speed"];
        if (root.containsKey("wind")) msg.wind = root["wind"];

        if (root.containsKey("timer")) msg.timer = root["timer"];
        if (root.containsKey("timer_value")) msg.timer_value = root["timer_value"];

        unsigned long data = dl_assemble_msg(&msg);
        irsend.sendNEC(data, 32);
        recv = true;
      }
    }

    millis_last = millis();
  }

  if (recv){
    Serial.println("Answer");
    JsonObject& root = jsonBuffer.createObject();

    root["on"] = msg.on;
    root["oscillate"] = msg.oscillate;
    root["speed"] = msg.speed;
    root["wind"] = msg.wind;
    root["timer"] = msg.timer;
    root["timer_value"] = msg.timer_value;

    //char buf[root.measureLength()];
    char buf[250];
    root.printTo(buf, 250);

    mq_aircon_status.publish(buf);
    recv = false;
    digitalWrite(LED_BLUE, LOW);
  }

  yield();
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
