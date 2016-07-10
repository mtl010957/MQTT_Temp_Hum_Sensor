#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>

#define wifi_ssid "<SSID>"
#define wifi_password "<WIFI PASSWD>"

#define mqtt_server "<MQTT SERVER>"
#define mqtt_user "<MQTT USER>"
#define mqtt_password "<MQTT PWD>"

// In case you have more than one sensor, make each one a different number here
#define sensor_number "2"
#define client_name "ESP8266Client_" sensor_number
#define humidity_topic "sensor/" sensor_number "/humidity/percentRelative"
#define temperature_c_topic "sensor/" sensor_number "/temperature/degreeCelsius"
#define temperature_f_topic "sensor/" sensor_number "/temperature/degreeFahrenheit"

#define red_led 0
#define blue_led 2

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SHT31 sht31; // I2C

void setup() {
  // Setup the two LED ports to use for signaling status
  pinMode(red_led, OUTPUT);
  pinMode(blue_led, OUTPUT);
  digitalWrite(red_led, HIGH);  
  digitalWrite(blue_led, HIGH);  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Set SDA and SDL ports
  //Wire.begin(2, 14);
  // Using I2C on the HUZZAH board SCK=#5, SDI=#4 by default

  // Start sensor
  if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Could not find a valid SHT31 sensor, check wiring!");
    while (1);
  }
}

void blink_red() {
  digitalWrite(red_led, LOW);
  delay(20);
  digitalWrite(red_led, HIGH);  
}

void blink_blue() {
  digitalWrite(blue_led, LOW);
  delay(20);
  digitalWrite(blue_led, HIGH);  
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    blink_red();
    delay(480);
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
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect(client_name)) {
    if (client.connect(client_name, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      blink_red();
      delay(200);
      blink_red();
      delay(4760);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

long lastMsg = 0;
long lastForceMsg = 0;
bool forceMsg = false;
float temp = 0.0;
float hum = 0.0;
float baro = 0.0;
float diff = 1.0;


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    // MQTT broker could go away and come back at any time
    // so doing a forced publish to make sure something shows up
    // within the first 5 minutes after a reset
    if (now - lastForceMsg > 300000) {
      lastForceMsg = now;
      forceMsg = true;
      Serial.println("Forcing publish every 5 minutes...");
    }

    float newTemp = sht31.readTemperature();
    float newHum = sht31.readHumidity();

    if (checkBound(newTemp, temp, diff) || forceMsg) {
      temp = newTemp;
      float temp_c=temp; // Celsius
      float temp_f=temp*1.8F+32.0F; // Fahrenheit
      Serial.print("New temperature:");
      Serial.print(String(temp_c) + " degC   ");
      Serial.println(String(temp_f) + " degF");
      client.publish(temperature_c_topic, String(temp_c).c_str(), true);
      client.publish(temperature_f_topic, String(temp_f).c_str(), true);
      blink_blue();
    }

    if (checkBound(newHum, hum, diff) || forceMsg) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum) + " %");
      client.publish(humidity_topic, String(hum).c_str(), true);
      blink_blue();
    }

    forceMsg = false;
  }
}
