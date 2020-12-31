#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include "DHT.h"
#define DHTPIN 12
#define DHTTYPE DHT11
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60  

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqttUsername = "";
const char* mqttPassword = "";
const char* mqttClientName = "";
const char* tempTopic = "temp";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup(void) {
  dht.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);

  if (!client.connected()) {
    Serial.println("Connecting to MQTT server...");
    if (client.connect(mqttClientName, mqttUsername, mqttPassword)) {
      Serial.println("Connected to MQTT");
      client.subscribe(tempTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(500);
      return;
    }
  }

  client.loop();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  const String str = String(t);
  byte resp[str.length()];
  str.getBytes(resp, str.length());
  
  client.beginPublish(tempTopic, str.length(), false);
  client.write(resp, str.length());
  client.endPublish();
  Serial.printf("Published");  

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
void loop() {
}
