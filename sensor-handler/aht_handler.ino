#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
// SDA: Connect to GPIO 21 (SDA pin on ESP32), Data pin on AHT10
// SCL: Connect to GPIO 22 (SCL pin on ESP32), Clock pin on AHT10

void setup() {
  Serial.begin(115200);

  if (!aht.begin()) {
    Serial.println("Could not find AHT10 sensor. Check wiring.");
    while (1);
  }
  Serial.println("AHT10 sensor initialized successfully.");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  delay(2000);
}
