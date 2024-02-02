#include <Wire.h>
#include "DHT.h"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
#endif

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
#define DHTTYPE DHT11 // DHT 22 (AM2302), AM2321

uint8_t DHTPin = D2;
DHT dht(DHTPin, DHTTYPE);

float temperature_Celsius;
float humidity;

#define WIFI_SSID "we21"
#define WIFI_PASSWORD "95603081"
  #define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
  #define INFLUXDB_TOKEN "z51opvAtebAqMb9AWyM6i7_cB9rqD3i7m_2xH_VmQlpAQXre-I0Hli8yPWVfMlEDkjW4nFF2MnFLsUIClB51kA=="
  #define INFLUXDB_ORG "9050137ac88dc9e5"
  #define INFLUXDB_BUCKET "DHT"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
 // Time zone info
  #define TZ_INFO "UTC2"
  
  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
  // Declare Data point
  Point sensor("wifi_status");
  
  void setup() {
    Serial.begin(115200);
     pinMode(DHTPin, INPUT);
  dht.begin();
    // Setup wifi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println();
  
    // Accurate time is necessary for certificate validation and writing in batches
    // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
  
    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
void loop() {
  // Store measured values into points
  sensor.clearFields();

  humidity = dht.readHumidity();
  temperature_Celsius = dht.readTemperature();

  sensor.addField("Temperature",temperature_Celsius);
  sensor.addField("Humidity",humidity);
  
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));

  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }
  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
     Serial.println(humidity);
     Serial.println( temperature_Celsius);
     
    Serial.println(client.getLastErrorMessage());
  }
  Serial.println("");
  Serial.println("Delay 10s");
  delay(10000);
}
