#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "VNPT T2";
const char* password = "18061996";

const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);
char notification[50];
int value = 0;
const char* _topic = "ltnc/ledcontrol";

/*
 4 = GPIO 4
 2 = GPIO 2
 13 = GPIO13
 12 = GPIO12 
 */
const char ledPin[4] = {2, 14, 15, 13};

void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  for(char i = 0; i < 4; i++){
    pinMode(ledPin[i], OUTPUT);
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.print(messageTemp);
  int message_decode = messageTemp.toInt();
  Serial.print(" ");
  Serial.println(message_decode);

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == _topic) {
    switch(message_decode){
      case 0: {
        for(int i = 0; i < 4; i++)
          digitalWrite(ledPin[i], 1);
        Serial.println("Turn all leds off");
        break;
      }
      case 1: {
        for(int i = 0; i < 4; i++)
          digitalWrite(ledPin[i], 0);
        Serial.println("Turn all leds on");
        break;
      }
      default:{
        char led_impact = message_decode/10 - 1;
        char turn_to = message_decode%10 ? 0 : 1;
        digitalWrite(ledPin[led_impact], turn_to);
        sprintf(notification, "Turn led %d to %d", led_impact + 1, turn_to);
        Serial.println(notification);
      }
        
        
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32 dat")) {
      client.publish(_topic, "ESP32 dat connected");
      Serial.println("connected");
      // Subscribe
      client.subscribe(_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
