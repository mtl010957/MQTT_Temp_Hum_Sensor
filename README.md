# MQTT_Temp_Hum_Sensor

This code reads values from a combination Temperature and Humidity
sensor board using an SHT31 with an ESP8266 board, and publishes the readings using MQTT.
Borrows from various examples by Adafruit and Arduino libraries.

The sensor module used was an Adafruit Sensiron SHT31-D Temperature & Humidity Sensor Breakout.
The ESP8266 module was an Adafruit HUZZAH ESP8266 breakout.

The configuration of the WiFi connection and the MQTT server are
setup by connecting to an access point the ESP8266 creates on first
startup or when forced by holding the flash button for 5 seconds or so then releasing it.
More details on how this works are
documented in the example code for the WiFIManager library at:
https://github.com/tzapu/WiFiManager

