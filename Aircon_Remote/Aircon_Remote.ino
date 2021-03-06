// Sketch to control a Seville Classics UltraSlimline 40-inch Tower Fan
// Author: Robbie Trencheny
// https://github.com/robbiet480/ESPAircon
// Based on:
// - https://github.com/zeroflow/ESPAircon
// - https://gist.github.com/balloob/daf310faa80112817d6826fbe5fc399d
// Get ESP8266 going with Arduino IDE
// - https://github.com/esp8266/Arduino#installing-with-boards-manager
// Required libraries (sketch -> include library -> manage libraries)
// - PubSubClient (https://github.com/knolleary/pubsubclient)
// - IRemoteESP8266 (https://github.com/markszabo/IRremoteESP8266)
// - ArduinoJSON (https://github.com/bblanchon/ArduinoJson)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Seville_Aircon.h"
#include <IRremoteESP8266.h>
#include <ArduinoJson.h>

#include "config.h"

#define MQTT_MAX_PACKET_SIZE 384

IRsend irsend(IR_PIN);

DynamicJsonBuffer  jsonBuffer;

seville_aircon_msg_t msg;

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, callback, espClient);

unsigned int fan_oscillate_eco[131] = {9350,4800, 550,550, 600,550, 550,550, 550,1700, 550,550, 550,550, 550,550, 600,1650, 550,550, 600,500, 600,1600, 600,550, 550,550, 600,550, 550,550, 550,550, 600,550, 600,500, 600,1600, 600,1650, 550,550, 550,550, 600,550, 550,1650, 550,550, 600,1600, 600,550, 600,500, 600,550, 550,550, 550,1650, 550,1650, 600,550, 600,1600, 600,500, 600,1650, 550,550, 600,550, 600,500, 550,550, 600,550, 550,550, 600,500, 600,550, 550,550, 600,550, 550,550, 600,550, 600,550, 550,1650, 550,1700, 550,550, 600,500, 600,500, 600,550, 550,550, 600,550, 550,1650, 600,1600, 550,1650, 600,550, 550,550, 600,1600, 600,1650, 550};
unsigned int fan_oscillate_low[131] = {9350,4800, 550,550, 600,550, 550,550, 550,1650, 600,550, 550,550, 600,500, 600,1650, 550,550, 550,550, 600,1650, 550,550, 600,500, 600,550, 550,550, 600,500, 600,550, 600,500, 600,1600, 600,1650, 550,550, 550,550, 600,550, 600,1600, 600,500, 600,1650, 550,550, 600,500, 600,550, 600,500, 600,500, 600,550, 600,500, 600,1600, 600,500, 600,1650, 550,550, 600,500, 600,550, 550,550, 600,500, 600,550, 550,550, 600,500, 600,550, 550,550, 600,550, 550,550, 600,550, 550,1650, 600,1650, 600,500, 600,550, 550,550, 600,500, 600,550, 600,500, 550,1650, 600,1600, 600,1650, 600,500, 600,500, 600,550, 600,500, 600};
unsigned int fan_oscillate_medium[131] = {9350,4800, 550,600, 550,550, 600,500, 600,1650, 550,550, 600,550, 550,550, 550,1650, 600,500, 600,550, 550,1650, 600,500, 600,550, 600,500, 600,550, 550,550, 600,500, 600,550, 550,1650, 600,1600, 600,500, 600,550, 550,550, 600,1650, 550,550, 550,1650, 600,550, 550,550, 600,500, 600,500, 600,550, 550,1650, 550,550, 600,1650, 600,500, 550,1650, 600,550, 600,500, 600,550, 550,550, 600,500, 600,550, 550,550, 550,550, 600,550, 550,550, 600,550, 600,500, 600,550, 550,1650, 600,1650, 600,500, 600,550, 550,550, 600,500, 600,550, 600,500, 600,1600, 600,1600, 600,1650, 600,500, 600,550, 550,550, 600,1600, 600};
unsigned int fan_oscillate_high[131] = {9350,4750, 600,550, 600,550, 550,500, 600,1650, 600,500, 600,550, 550,550, 600,1600, 600,550, 550,550, 600,1600, 600,550, 600,500, 600,550, 550,550, 600,550, 550,550, 600,500, 600,1600, 600,1600, 600,500, 600,550, 600,550, 550,1600, 600,550, 600,1600, 600,550, 550,550, 600,550, 550,550, 550,1600, 650,500, 600,550, 550,1600, 650,500, 600,1600, 600,500, 600,550, 600,550, 550,550, 600,550, 550,550, 550,550, 600,550, 550,550, 550,550, 600,550, 550,550, 600,550, 600,1600, 600,1600, 600,550, 600,550, 550,550, 600,500, 600,550, 550,550, 600,1600, 600,1600, 600,1600, 600,550, 600,550, 550,1600, 600,550, 600};
unsigned int fan_eco[131] = {9350,4800, 550,550, 550,550, 550,600, 550,1650, 600,500, 550,600, 550,550, 550,1650, 550,550, 550,600, 550,1650, 550,550, 550,550, 600,550, 550,550, 600,550, 550,550, 550,550, 600,1600, 600,1650, 550,550, 600,500, 600,550, 550,550, 550,600, 550,1650, 550,550, 550,550, 600,550, 550,550, 550,1650, 550,1650, 550,600, 550,1650, 600,500, 550,1650, 600,550, 600,500, 550,600, 550,550, 600,550, 550,550, 550,550, 550,600, 550,550, 600,550, 550,550, 600,500, 600,550, 600,1600, 600,1650, 600,500, 600,500, 600,550, 600,500, 600,550, 550,550, 600,1600, 600,1650, 550,1650, 600,500, 550,550, 600,1650, 550,550, 550};
unsigned int fan_low[131] = {9400,4750, 600,500, 600,550, 600,500, 600,1650, 600,500, 600,500, 600,550, 600,1600, 600,550, 550,550, 600,1600, 600,500, 600,550, 550,550, 600,500, 600,550, 600,500, 600,550, 550,1650, 600,1600, 600,550, 600,500, 600,500, 600,550, 600,500, 600,1600, 600,500, 600,550, 600,500, 600,500, 600,550, 600,500, 600,550, 600,1600, 600,500, 600,1600, 600,550, 550,550, 600,500, 600,550, 600,500, 600,550, 600,500, 600,500, 650,500, 600,550, 600,500, 600,500, 600,550, 600,1600, 600,1650, 550,550, 600,500, 600,500, 600,550, 600,500, 600,550, 600,1600, 550,1650, 600,1600, 600,500, 650,500, 600,500, 600,1650, 600};
unsigned int fan_medium[131] = {9350,4750, 600,550, 550,550, 600,550, 550,1700, 550,500, 600,550, 600,500, 600,1600, 600,550, 550,550, 600,1600, 600,550, 550,550, 600,500, 600,550, 600,500, 600,500, 600,550, 600,1600, 600,1600, 600,550, 600,500, 600,500, 600,550, 550,550, 600,1600, 600,550, 600,500, 600,500, 600,550, 600,500, 600,1600, 600,550, 600,1600, 600,500, 550,1650, 600,550, 550,550, 600,500, 600,550, 600,500, 600,500, 600,550, 600,500, 600,500, 650,500, 600,550, 600,500, 600,550, 550,1650, 600,1650, 550,550, 550,550, 600,550, 600,500, 600,500, 600,550, 600,1600, 600,1600, 600,1650, 550,550, 600,500, 600,550, 600,500, 550};
unsigned int fan_high[131] = {9300,4800, 600,500, 600,550, 600,500, 600,1650, 600,500, 600,500, 600,550, 550,1650, 600,500, 600,550, 600,1600, 600,500, 600,550, 600,500, 600,500, 600,550, 600,500, 600,550, 600,1600, 600,1600, 600,500, 600,550, 600,500, 600,550, 550,550, 600,1600, 600,500, 600,550, 600,500, 600,500, 600,1650, 550,550, 600,500, 600,1600, 600,550, 600,1600, 600,500, 600,550, 600,500, 600,500, 600,550, 600,500, 600,550, 550,550, 600,500, 600,550, 600,550, 600,500, 600,500, 600,1650, 550,1650, 600,550, 600,500, 600,500, 600,550, 600,500, 600,550, 550,1650, 600,1600, 600,1600, 600,550, 600,500, 550,1650, 600,1650, 600};
unsigned int fan_off[131] = {9400,4800, 600,500, 600,500, 600,550, 600,1600, 600,500, 600,550, 600,500, 550,550, 600,550, 550,550, 600,1600, 600,500, 600,550, 550,550, 600,500, 600,550, 600,500, 600,550, 550,1650, 600,1600, 600,500, 550,550, 600,550, 600,500, 600,500, 600,1650, 550,550, 600,500, 600,550, 600,500, 600,550, 550,550, 600,500, 600,1600, 600,550, 550,1650, 600,500, 600,550, 550,550, 600,500, 600,550, 600,500, 550,550, 600,550, 600,500, 550,550, 600,550, 600,500, 600,500, 600,1600, 600,1650, 550,550, 600,500, 600,550, 600,500, 600,500, 600,550, 550,1650, 550,1650, 550,1650, 550,550, 600,550, 600,500, 600,500, 600};

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  payload[length] = '\0';
  String str_payload = String((char*)payload);

  String publishing_topic = "";
  String publishing_payload = "";

  if (String(topic) == String(JSON_SET_TOPIC)) {
    JsonObject& root = jsonBuffer.parseObject((char*)payload);

    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    if (root.containsKey("on")) {
      msg.on = root["on"];
      publish_to_mqtt(ON_STATE_TOPIC, root["on"]);
    }

    if (root.containsKey("oscillate")) {
      msg.oscillate = root["oscillate"];
      publish_to_mqtt(OSCILLATE_STATE_TOPIC, root["oscillate"]);
    }

    if (root.containsKey("speed")) {
      String speed = String(root["speed"].asString());
      if (speed == "eco") {
        msg.speed = 3;
      } else if (speed == "low") {
        msg.speed = 0;
      } else if (speed == "medium") {
        msg.speed = 1;
      } else if (speed == "high") {
        msg.speed = 2;
      }
      publish_to_mqtt(SPEED_STATE_TOPIC, root["speed"]);
    }
  } else if (String(topic) == String(ON_SET_TOPIC)) {
    msg.on = (str_payload == "true");
    publishing_topic = ON_STATE_TOPIC;
    publishing_payload = (msg.on ? "true" : "false");
  } else if (String(topic) == String(OSCILLATE_SET_TOPIC)) {
    msg.oscillate = (str_payload == "true");
    publishing_topic = OSCILLATE_STATE_TOPIC;
    publishing_payload = (msg.oscillate ? "true" : "false");
  } else if (String(topic) == String(SPEED_SET_TOPIC)) {
    if (str_payload == "eco") {
      msg.speed = 3;
    } else if (str_payload == "low") {
      msg.speed = 0;
    } else if (str_payload == "medium") {
      msg.speed = 1;
    } else if (str_payload == "high") {
      msg.speed = 2;
    }
    publishing_topic = SPEED_STATE_TOPIC;
    publishing_payload = String(str_payload).c_str();
  } else {
    Serial.println("No topic matched!");
  }

  if (msg.on) {
    if (msg.oscillate) {
      if (msg.speed == 3) {
        // On, Oscillate, Eco
        send_raw_ir(fan_oscillate_eco);
      } else if (msg.speed == 0) {
        // On, Oscillate, Low
        send_raw_ir(fan_oscillate_low);
      } else if (msg.speed == 1) {
        // On, Oscillate, Medium
        send_raw_ir(fan_oscillate_medium);
      } else if (msg.speed == 2) {
        // On, Oscillate, High
        send_raw_ir(fan_oscillate_high);
      }
    } else {
      if (msg.speed == 3) {
        // On, Eco
        send_raw_ir(fan_eco);
      } else if (msg.speed == 0) {
        // On, Low
        send_raw_ir(fan_low);
      } else if (msg.speed == 1) {
        // On, Medium
        send_raw_ir(fan_medium);
      } else if (msg.speed == 2) {
        // On, High
        send_raw_ir(fan_high);
      }
    }
  } else {
    // Off
    send_raw_ir(fan_off);
  }

  if (publishing_topic != "" && publishing_payload != "") {
    publish_to_mqtt(publishing_topic.c_str(), publishing_payload.c_str());
  }

  JsonObject& publish_root = jsonBuffer.createObject();

  publish_root["on"] = msg.on;
  publish_root["oscillate"] = msg.oscillate;

  String human_speed = "";

  if (msg.speed == 0) {
    human_speed = "eco";
  } else if (msg.speed == 1) {
    human_speed = "low";
  } else if (msg.speed == 2) {
    human_speed = "medium";
  } else if (msg.speed == 3) {
    human_speed = "high";
  }
  publish_root["speed"] = human_speed;

  char buf[256];
  publish_root.printTo(buf, sizeof(buf));

  publish_to_mqtt(JSON_STATE_TOPIC, buf);
}

void setup() {
  irsend.begin();
  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(IR_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  send_raw_ir(fan_off);

  msg.on = false;
  msg.oscillate = false;
  msg.speed = 0;

  setup_wifi();
}

void setup_wifi() {
  // Disable built in access point
  WiFi.mode(WIFI_STA);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print("state=");
    Serial.println(client.state());

    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, ALIVE_TOPIC, 0, 1, "dead")) {
      Serial.print("Connected to MQTT Broker (");
      Serial.print(MQTT_SERVER);
      Serial.println(")");
      Serial.print("MQTT connection state: ");
      Serial.println(client.state());
      publish_to_mqtt(ALIVE_TOPIC, "alive");
      client.subscribe(JSON_SET_TOPIC);
      client.subscribe(ON_SET_TOPIC);
      client.subscribe(OSCILLATE_SET_TOPIC);
      client.subscribe(SPEED_SET_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Disconnected from MQTT, starting reconnection!");
    Serial.print("Current WiFi state is: ");
    Serial.println(WiFi.status());

    // Breathing from https://arduining.com/2015/08/20/nodemcu-breathing-led-with-arduino-ide/

    //ramp increasing intensity, Inhalation:
    for (int i=1;i<BRIGHT;i++){
      digitalWrite(RED_LED, LOW);          // turn the LED on.
      delayMicroseconds(i*10);             // wait
      digitalWrite(RED_LED, HIGH);         // turn the LED off.
      delayMicroseconds(PULSE-i*10);       // wait
      delay(0);                            // to prevent watchdog firing.
    }

    //ramp decreasing intensity, Exhalation (half time):
    for (int i=BRIGHT-1;i>0;i--){
      digitalWrite(RED_LED, LOW);          // turn the LED on.
      delayMicroseconds(i*10);             // wait
      digitalWrite(RED_LED, HIGH);         // turn the LED off.
      delayMicroseconds(PULSE-i*10);       // wait
      i--;
      delay(0);                            // to prevent watchdog firing.
    }
    delay(REST);                           // take a rest...
    reconnect();
  }
  client.loop();
}

void send_raw_ir(unsigned int* raw_data) {
  digitalWrite(BLUE_LED, LOW);
  irsend.sendRaw(raw_data, sizeof(raw_data) / sizeof(raw_data[0]), 38);
  digitalWrite(BLUE_LED, HIGH);
}

void publish_to_mqtt(const char* topic, const char* payload) {
  digitalWrite(RED_LED, LOW);
  client.publish(topic, payload, true);
  digitalWrite(RED_LED, HIGH);
}
