#include <WiFi.h>
#include <SDS011.h>
#include "secrets.h" // Contains WiFi credentials
#include "ThingSpeak.h" 

const int SDS_TX = 17;  // SDS011 RX2
const int SDS_RX = 16;  // SDS011 TX2

SDS011 mySDS;

float pm25, pm10;
WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID; 
const char * myWriteAPIKey = SECRET_WRITE_APIKEY; 

void setup() {
  Serial.begin(115200);  
  mySDS.begin(SDS_RX, SDS_TX); 
  delay(3000); 
  
  WiFi.mode(WIFI_STA);  
  ThingSpeak.begin(client);  
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Connecting to WiFi...");
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(SECRET_SSID, SECRET_PASS);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected to WiFi.");
  }
  
  int error = mySDS.read(&pm25, &pm10);
  if (error == 0) {
    Serial.println("Data received from SDS011:");
    Serial.print("PM2.5: ");
    Serial.print(pm25);
    Serial.print(" µg/m3, PM10: ");
    Serial.print(pm10);
    Serial.println(" µg/m3");

    ThingSpeak.setField(1, pm25);  
    ThingSpeak.setField(2, pm10);  

    String statusMsg = "Air quality updated.";
    ThingSpeak.setStatus(statusMsg);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code: " + String(x));
    }
  } else {
    Serial.println("Error reading from SDS011 sensor.");
  }

  delay(60000); // Wait for 1 minute before reading again
}
