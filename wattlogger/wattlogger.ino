#include <Adafruit_INA260.h>
#include "WiFi.h"
#include <PubSubClient.h>
#include "time.h"

// Sensor
Adafruit_INA260 ina260 = Adafruit_INA260();
unsigned long long loggingInterval = 1000; // ms
unsigned long long lastReadMillis = 0; 
float ss;

// WiFi
const char *WIFI_SSID = "aBuffalo-T-E510";
const char *WIFI_PASSWORD = "penguink";
// const char *WIFI_SSID = "koidelab";
// const char *WIFI_PASSWORD = "nni-8ugimrjnmw";
WiFiClient espClient;

// Define NTP Client to get time
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 9;  // GMT offset for Japan Standard Time (JST)
const int daylightOffset_sec = 0;

// MQTT Broker
const char* mqtt_broker = "server.local";
const char* topic = "cli/watts";
const int mqtt_port = 31883;
PubSubClient client(espClient);

char buffer[100];

static time_t align_to_nearest_10_seconds(time_t t) {
	return (t / 10) * 10;
}

static void display_time(const char* label, time_t t) {
	char buffer[64];
	struct tm tm_time;

	setenv("TZ", "JST-9", 1);
	tzset();
	localtime_r(&t, &tm_time);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_time);
	Serial.printf("%s: %s\n", label, buffer);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Serial Connected");
  
  // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(10000);
  }
  Serial.println("WiFi Connected");

  // Initialize INA260 sensor
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }
  Serial.println("Found INA260 chip");

  ina260.setAveragingCount(INA260_COUNT_64);
  ina260.setCurrentConversionTime(INA260_TIME_8_244_ms);
  ina260.setVoltageConversionTime(INA260_TIME_8_244_ms);

  client.setServer(mqtt_broker, mqtt_port);

  // Initialize and synchronize time with NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  lastReadMillis = millis() + 20000; // Capture the starting millis. In 20s.
  
  Serial.println("Setup complete");
}

void loop() {
  unsigned long long currentMillis = millis();
  
  if (currentMillis >= loggingInterval + lastReadMillis) {
    lastReadMillis += loggingInterval;

    float cur = ina260.readCurrent();
    float vol = ina260.readBusVoltage();
    float pow = ina260.readPower();

    if (client.connected() || client.connect("abc")) {
      sprintf(buffer, "{\"ts\":%lu,\"cur\":%f,\"vol\":%f,\"pow\":%f}", (unsigned long)time(NULL), cur, vol, pow);
      Serial.println(buffer);
      client.publish(topic, buffer);

      client.loop();
    }
  }
}
