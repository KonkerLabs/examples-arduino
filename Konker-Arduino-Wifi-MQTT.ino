
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

//Arduino GPIOs
int sensorPin = A0;

//WiFi variables
char ssid[] = "";     //  your network SSID (name)
char pass[] = "";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//MQTT Variables
const char* mqtt_server = "mqtt.demo.konkerlabs.net"; // MQTT address of the Konker platform
const char* PUB = ""; // MQTT Topic to publish messages
const char* SUB = ""; // MQTT Topic subscribed to receive messages
const char* USER = ""; // MQTT username (Randomly created for your device in the Konker platform)
const char* PWD = ""; // MQTT password (Randomly created for your device in the Konker platform)

const char connectedmsg[] = "{\"MSG\":\"Reconnected\"}"; // Message that will be sent once the MQTT connection with the platform was successful

//Now let's create a callback function to do something when receiving an MQTT message (in this case, to print it on the Serial)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//Defining the WiFi and MQTT client
WiFiClient ArduClient;
PubSubClient client(mqtt_server, 1883, callback, ArduClient);


//Now let's create a function to connect/reconnect to the MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect("ArduClient", USER, PWD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(PUB, connectedmsg);
      // ... and resubscribe
      if(client.subscribe(SUB))
        Serial.println(F("Subscribed to channel"));
      else
        Serial.println(F("PROBLEM ! Not subscribed to channel"));
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Now we create a function to read the analog "sensorPin" and publish the value using MQTT
void sendvoltage(){
  char* messageC;
  String message = F("{\"id\":\"DeviceID\", \"value\":\"");
  message+= String(analogRead(sensorPin));
  message+= F("\" , \"metric\":\"voltage\", \"unit\":\"ADC units\"}"); 
  message.toCharArray(messageC,message.length()+1);
  Serial.println(messageC);
  client.publish(PUB,messageC);

}

void setup() {
  Serial.begin(115200);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue: This will loop forever
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 5 seconds for connection:
    delay(5000);
  }

}

void loop() {
  delay(5000);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  sendvoltage();
}


