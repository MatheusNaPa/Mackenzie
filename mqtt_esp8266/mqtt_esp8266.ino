/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>  //Biblioteca do UDP.

//#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>



WiFiUDP udp;  //Cria um objeto "UDP".
//NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);//Cria um objeto "NTP" com as configurações.
NTPClient ntp(udp, "a.st1.ntp.br");  //Cria um objeto "NTP" com as configurações.



// Update these with values suitable for your network.

const char* ssid = "Fiore";
const char* password = "19761976";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "test.mosquitto.org";
//const char * mqtt_server = "localhost";
int pinSensor = A0;
int pinBuzzer = 4;     //D2
int pinSensorDig = 5;  // D1
uint8_t pinLed = D8;
int gas = 0;
int dig = HIGH;
const char* topic = "fioresoft/gas";

WiFiClient espClient;
PubSubClient client(espClient);
//PubSubClient client(mqtt_server,(uint16_t)1883,espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
StaticJsonDocument<256> doc;
char buffer[256];
uint8_t last_signal = 99;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  //WiFi.mode(WIFI_AP);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  /*
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    char *msg = (char *)payload;
    msg[length] = 0;
    Serial.println(msg);
  }
  */
  // cast payload to pointer-to-const to disable ArduinoJson's zero-copy mode
  deserializeJson(doc, (const byte*)payload, length);

  uint8_t signal = doc["signal"];
  unsigned long t = doc["time"];
  unsigned long now = 0;

  ntp.update();
  now = ntp.getEpochTime();
  Serial.print("delta t = ");
  Serial.println(now - t);
  Serial.print("signal = ");
  Serial.println(signal);
  // Switch on the LED if an 1 was received as first character
  if (signal == 1) { // buzzer on
    //digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    tone(pinBuzzer, 1000);
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else if (signal == 2) { //  led on
    digitalWrite(pinLed, HIGH);
  } else if (signal == 3) { // led off
    digitalWrite(pinLed, LOW); 
  } else if (signal == 0) { // buzzer off
    // digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    noTone(pinBuzzer);
  }
}

void reconnect_mqtt() {
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Loop until we're reconnected
  Serial.println("Attempting MQTT connection...");
  while (!client.connected()) {
    Serial.print(".");
    if (client.connect(clientId.c_str())) {
      Serial.println("mqtt client connected");
    } else {
      Serial.println(client.state());
      delay(1000);
    }
  }
  client.subscribe("fioresoft/gas");
}


void setup() {
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  Serial.begin(9600);
  while (!Serial)
    continue;
  while (!Serial.availableForWrite())
    ;
  delay(1000);
  Serial.println("sensor aquecendo...");
  delay(10000);
  //
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_wifi();
  ntp.begin();
  ntp.forceUpdate();
  if (client.connect(clientId.c_str())) {
    Serial.println("ok");
    client.subscribe(topic);
  }
  if (!client.connected()) {
    reconnect_mqtt();
  }
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinSensor, INPUT);
  pinMode(pinSensorDig, INPUT);
  pinMode(pinLed, OUTPUT);
}

void loop() {
  
  gas = analogRead(pinSensor);
  dig = digitalRead(pinSensorDig);

  ntp.update();
  doc["time"] = ntp.getEpochTime();

  if (dig == LOW) {
      doc["signal"] = 1; // buzzer on

      size_t n = serializeJson(doc, buffer);
      client.publish(topic, buffer, n);
      doc["signal"] = 2;                  // led on
      n = serializeJson(doc, buffer);
      client.publish(topic, buffer, n); 
  } else {
      doc["signal"] = 0;                  // buzzer off
      size_t n = serializeJson(doc, buffer);
      client.publish(topic, buffer, n);
      //client.publish(topic,"3");
  }
  gas = map(gas,0,1023,1,10000);
  Serial.println(gas);
  delay(1000);

  client.loop();
}