#include <PubSubClient.h>
#include <WiFi.h> 
#include <SDS011.h>

// Ensure correct credentials to connect to your WiFi Network.
char ssid[] = "MySSID";  // Replace MySSID with your WiFi network name
char pass[] = "MyPassword";     // Replace MyPassword with your WiFi network name

const int SDS_TX = 17;  // SDS011 RX2
const int SDS_RX = 16;  // SDS011 TX2

SDS011 mySDS;

float pm25, pm10;

// Ensure that the credentials here allow you to publish and subscribe to the ThingSpeak channel.
#define channelID 2470174
const char mqttUserName[] = "00000"; // Replace 00000 with actual token
const char clientID[] = "00000";    // Replace 00000 with actual token
const char mqttPass[] = "00000";   // Replace 00000 with actual token

#define mqttPort 1883
WiFiClient client;

const char* server = "mqtt3.thingspeak.com";
int status = WL_IDLE_STATUS; 
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 20;
PubSubClient mqttClient( client );

// Subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID ){
  String myTopic = "channels/"+String( subChannelID )+"/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}

// Publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) {
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}

void connectWifi()
{
  Serial.print( "Connecting to Wi-Fi..." );
  while ( WiFi.status() != WL_CONNECTED ) {
    WiFi.begin( ssid, pass );
    delay( connectionDelay*1000 );
    Serial.print( WiFi.status() ); 
  }
  Serial.println( "Connected to Wi-Fi." );
}

void mqttConnect() {
  while ( !mqttClient.connected() )
  {
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) ) {
      Serial.println("Connected to MQTT broker...");
    } else {
      Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );
    }
  }
}

void setup() {
  Serial.begin( 115200 );
  mySDS.begin(SDS_RX, SDS_TX);
  delay(3000);
  connectWifi();
  mqttClient.setServer( server, mqttPort ); 
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
  }
  
  if (!mqttClient.connected()) {
     mqttConnect(); 
     mqttSubscribe( channelID );
  }
  
  mqttClient.loop(); 
  

  int error = mySDS.read(&pm25, &pm10);

  if (error == 0) {
    Serial.println("Data received from SDS011:");
    Serial.print("PM2.5: ");
    Serial.print(pm25);
    Serial.print(" µg/m3, PM10: ");
    Serial.print(pm10);
    Serial.println(" µg/m3");
    
    String message = "field1=" + String(pm25) + "&field2=" + String(pm10);
    
    String topicString = "channels/" + String(channelID) + "/publish";
    mqttClient.publish(topicString.c_str(), message.c_str());
  }

  delay(15000);
}
