#include <WiFi.h>
#include <HTTPClient.h>
#include <SDS011.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "secrets.h" // Contains WiFi credentials and Qubitro Device Token

const int SDS_TX = 17;  // SDS011 RX2
const int SDS_RX = 16;  // SDS011 TX2
SDS011 mySDS;
float pm25, pm10;

const int CO2_PIN = 13;
unsigned long th, tl;
int ppm;

Adafruit_AHTX0 aht;

const float PM25_ERROR_PERCENTAGE = 12.4; 
const float PM10_ERROR_PERCENTAGE = 1.17;

float calibrateReading(float reading, float errorPercentage) {
  return reading * (1 - errorPercentage / 100);
}

void setup() {
  Serial.begin(115200);

  mySDS.begin(SDS_RX, SDS_TX);
  Serial.println("SDS011 Air Quality Sensor is starting...");

  pinMode(CO2_PIN, INPUT);
  Serial.println("CO2 Sensor initialized.");

  if (!aht.begin()) {
    Serial.println("Could not find AHT10 sensor. Check wiring.");
    while (1); 
  }
  Serial.println("AHT10 sensor initialized successfully.");

  WiFi.mode(WIFI_STA);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(SECRET_SSID, SECRET_PASS);  // Use credentials from secrets.h
      Serial.print(".");
      delay(5000);  
    }
    Serial.println("\nConnected to WiFi.");
  }

  int error = mySDS.read(&pm25, &pm10);
  if (!error) {
    float pm25_calibrated = calibrateReading(pm25, PM25_ERROR_PERCENTAGE);
    float pm10_calibrated = calibrateReading(pm10, PM10_ERROR_PERCENTAGE);

    Serial.print("Calibrated PM2.5: ");
    Serial.print(pm25_calibrated);
    Serial.print(" µg/m³, Calibrated PM10: ");
    Serial.print(pm10_calibrated);
    Serial.println(" µg/m³");
  } else {
    Serial.println("Error reading from the SDS011 sensor.");
  }

  th = pulseIn(CO2_PIN, HIGH, 2008000) / 1000;
  tl = 1004 - th;
  ppm = 2000 * (th - 2) / (th + tl - 4);
  Serial.print("CO2 Concentration: ");
  Serial.print(ppm);
  Serial.println(" ppm");

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://webhook.qubitro.com/integrations/http";
    Serial.println("Posting to URL: " + url);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Basic " + String(SECRET_QU_BITRO_API_TOKEN)); 

    String payload = "{";
    payload += "\"PM2.5\": " + String(calibrateReading(pm25, PM25_ERROR_PERCENTAGE)) + ",";
    payload += "\"PM10\": " + String(calibrateReading(pm10, PM10_ERROR_PERCENTAGE)) + ",";
    payload += "\"CO2\": " + String(ppm) + ",";
    payload += "\"Temperature\": " + String(temp.temperature) + ",";
    payload += "\"Humidity\": " + String(humidity.relative_humidity);
    payload += "}";

    Serial.println("Payload: " + payload);

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Data sent successfully:");
      Serial.println(response);
    } else {
      Serial.print("Error sending data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected.");
  }

  delay(60000);  // Send data every 60 seconds
}
