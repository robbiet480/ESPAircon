// Get ESP8266 going with Arduino IDE
// - https://github.com/esp8266/Arduino#installing-with-boards-manager
// Required libraries (sketch -> include library -> manage libraries)
// - PubSubClient by Nick 'O Leary
// - IRemoteESP8266

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DL_Aircon.h"
#include <IRremoteESP8266.h>
#include <ArduinoJson.h>

#include "config.h"

#define MQTT_MAX_PACKET_SIZE 384

IRsend irsend(IR_PIN);

DynamicJsonBuffer  jsonBuffer;

dl_aircon_msg_t msg;

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, callback, espClient);
long lastMsg = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  digitalWrite(LED_BLUE, HIGH);

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
      client.publish(ON_STATE_TOPIC, root["on"]);
    }

    if (root.containsKey("oscillate")) {
      msg.oscillate = root["oscillate"];
      client.publish(OSCILLATE_STATE_TOPIC, root["oscillate"]);
    }

    if (root.containsKey("speed")) {
      if (root["speed"] == "eco") {
        msg.speed = 0;
      } else if (root["speed"] == "low") {
        msg.speed = 1;
      } else if (root["speed"] == "medium") {
        msg.speed = 2;
      } else if (root["speed"] == "high") {
        msg.speed = 3;
      }
      client.publish(SPEED_STATE_TOPIC, root["speed"]);
    }

    if (root.containsKey("wind")) {
      if (root["wind"] == "normal") {
        msg.wind = 0;
      } else if (root["wind"] == "natural") {
        msg.wind = 1;
      } else if (root["wind"] == "sleeping") {
        msg.wind = 2;
      }
      client.publish(WIND_STATE_TOPIC, root["wind"]);
    }

    if (root.containsKey("timer")) {
      msg.timer = root["timer"];
      client.publish(TIMER_STATE_TOPIC, root["timer"]);
    }
    if (root.containsKey("timer_value")) {
      msg.timer_value = root["timer_value"];
      client.publish(TIMER_VALUE_STATE_TOPIC, root["timer_value"]);
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
      msg.speed = 0;
    } else if (str_payload == "low") {
      msg.speed = 1;
    } else if (str_payload == "medium") {
      msg.speed = 2;
    } else if (str_payload == "high") {
      msg.speed = 3;
    }
    publishing_topic = SPEED_STATE_TOPIC;
    publishing_payload = String(str_payload).c_str();
  } else if (String(topic) == String(WIND_SET_TOPIC)) {
    if (str_payload == "normal") {
      msg.wind = 0;
    } else if (str_payload == "natural") {
      msg.wind = 1;
    } else if (str_payload == "sleeping") {
      msg.wind = 2;
    }
    publishing_topic = WIND_STATE_TOPIC;
    publishing_payload = String(str_payload).c_str();
  } else if (String(topic) == String(TIMER_SET_TOPIC)) {
    msg.timer = (str_payload == "true");
    publishing_topic = TIMER_STATE_TOPIC;
    publishing_payload = (msg.timer ? "true" : "false");
  } else if (String(topic) == String(TIMER_VALUE_SET_TOPIC)) {
    msg.timer_value = str_payload.toFloat();
    publishing_topic = TIMER_VALUE_STATE_TOPIC;
    publishing_payload = String(msg.timer_value).c_str();
  } else {
    Serial.println("No topic matched!");
  }

  unsigned long data = dl_assemble_msg(&msg);
  irsend.sendNEC(data, 32);

  if (publishing_topic != "" && publishing_payload != "") {
    Serial.print("Will be publishing to topic ");
    Serial.print(publishing_topic);
    Serial.print(" with payload ");
    Serial.println(publishing_payload);

    client.publish(publishing_topic.c_str(), publishing_payload.c_str(), true);
  }

  JsonObject& publish_root = jsonBuffer.createObject();

  publish_root["on"] = msg.on;
  publish_root["oscillate"] = msg.oscillate;

  String human_speed = "";

  if (msg.speed = 0) {
    human_speed = "eco";
  } else if (msg.speed = 1) {
    human_speed = "low";
  } else if (msg.speed = 2) {
    human_speed = "medium";
  } else if (msg.speed = 3) {
    human_speed = "high";
  }
  publish_root["speed"] = human_speed;

  String human_wind = "";

  if (msg.wind = 0) {
    human_wind = "normal";
  } else if (msg.wind = 1) {
    human_wind = "natural";
  } else if (msg.wind = 2) {
    human_wind = "sleeping";
  }

  publish_root["wind"] = human_wind;
  publish_root["timer"] = msg.timer;
  publish_root["timer_value"] = msg.timer_value;

  //char buf[publish_root.measureLength()];
  char buf[250];
  publish_root.printTo(buf, 250);

  client.publish(JSON_STATE_TOPIC, buf, true);
  digitalWrite(LED_BLUE, LOW);
}

void setup() {
  irsend.begin();
  Serial.begin(115200);
  Serial.println("Booting");
  setup_wifi();

  pinMode(IR_PIN, OUTPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  msg.on = false;
  msg.oscillate = false;
  msg.timer = false;
  msg.timer_value = 0.5;
  msg.speed = 2;
  msg.wind = 0;

  digitalWrite(LED_GREEN, HIGH);
  delay(100);
  digitalWrite(LED_GREEN, LOW);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    delay(400);
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
      Serial.println("connected");
      Serial.print("state=");
      Serial.println(client.state());
      client.publish(ALIVE_TOPIC, "alive", true);
      client.subscribe(JSON_SET_TOPIC);
      client.subscribe(ON_SET_TOPIC);
      client.subscribe(OSCILLATE_SET_TOPIC);
      client.subscribe(SPEED_SET_TOPIC);
      client.subscribe(WIND_SET_TOPIC);
      client.subscribe(TIMER_SET_TOPIC);
      client.subscribe(TIMER_VALUE_SET_TOPIC);
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
    Serial.println("Disconnected, starting reconnection!");
    reconnect();
  }
  client.loop();
}
