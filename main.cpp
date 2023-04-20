//include libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>


// WiFi variables
const char* ssid = "Network Name";  // Enter your WiFi name
const char* password = "password";  // Enter WiFi password
const char* host = "raspberrypi_ip_address"; //ip address of rapsberry pi


// MQTT variables
const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* publishTopic = "testtopic/SMD/PROJECT/1";   // outTopic where ESP publishes
const char* subscribeTopic = "Subscribe/Topic";  // inTopic where ESP has subscribed to
#define publishTimeInterval // in seconds


// Definitions
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
#define recPulse D1
#define sentPulse D8
#define blueRGB D2
#define greenRGB D3
#define redRGB D4
#define misfire D6


int comTaken = 0;
char msg[MSG_BUFFER_SIZE];


unsigned long startTime;
unsigned long currentTime;


WiFiClient espClient;
PubSubClient client(espClient); // define MQTTClient

//connect to wifi function
void setup_wifi() {
 delay(10);
 // We start by connecting to a WiFi network
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);


 WiFi.mode(WIFI_STA);
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




 //------------------------------------------
void HTTPrequest(int comTaken) {
 WiFiClient client;
 if (!client.connect(host, 80)) { //check for connection to raspberry pi (host)
   Serial.println("Connection to host failed");
   return;
 }
 //Setup URL to be Requested
 String url = "http://" + String(host) + "/subtract.php?FeederID=4&Count=" + String(comTaken);
 Serial.println(url);
 // Send the HTTP GET request to the Raspberry Pi
 client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Connection: close\r\n\r\n");
 delay(10);


 // Read the response from the server
 while(client.available()){
   String line = client.readStringUntil('\r');
   Serial.print(line);
 }


 Serial.println();
 Serial.println("Data Entry Successful!");
 client.stop();


 comTaken = 0;
}


void callback(char* topic, byte* payload, int length) {
 String message = "";
 Serial.print("Message arrived ["); // Received the incoming message
 Serial.print(topic);
 Serial.print("] ");
 for (int i = 0; i < length; i++) {
   message += (char)payload[i];
   Serial.print((char)payload[i]);  // the received value is in (char)payload[i]
   //numComponents = ((char)payload[i]);
 }
 Serial.println();
   int numComponents = message.toInt();
   int comTaken = numComponents;
   while (numComponents > 0) {
       digitalWrite(sentPulse,HIGH);
       digitalWrite(greenRGB,LOW);
       digitalWrite(redRGB, HIGH);
       delay(100);
       digitalWrite(sentPulse,LOW);
       delay(900);
       numComponents = numComponents - 1;
       Serial.println(numComponents);
       if (numComponents == 0){
         HTTPrequest(comTaken);
       }
   }
  
}


 //------------------------------------------


void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
   Serial.print("Attempting MQTT connection...");
   // Create a random client ID
   String clientId = "ESP8266Client-";
   clientId += String(random(0xffff), HEX);
   // Attempt to connect
   if (client.connect(clientId.c_str())) {
     Serial.println("connected");
     // ... and resubscribe
     client.subscribe(subscribeTopic);
   } else {
     Serial.print("failed, rc=");
     Serial.print(client.state());
     Serial.println(" try again in 5 seconds");
     // Wait 5 seconds before retrying
     delay(5000);
   }
 }
}
//------------------------------------------


void setup() {
 pinMode(recPulse, INPUT);
 pinMode(sentPulse, OUTPUT);
 pinMode(blueRGB, OUTPUT);
 pinMode(greenRGB, OUTPUT);
 pinMode(redRGB, OUTPUT);


 Serial.begin(9600);
 setup_wifi();
 client.setServer(mqtt_server, 1883);
 client.setCallback(callback);
 startTime = millis();
}
//------------------------------------------
void loop() {


 if (!client.connected()) {
   reconnect(); // check for the latest value in inTopic
 }
 if (digitalRead(recPulse) == HIGH){
     //delay(500);
     digitalWrite(blueRGB, HIGH);
     digitalWrite(sentPulse, HIGH);
     //comTaken = comTaken + 1;
     delay(500);
     digitalWrite(sentPulse, LOW);
     digitalWrite(blueRGB, LOW);
     Serial.print("Components Dispensed: ");
     Serial.println(comTaken);
     delay(400);
   }
  if (digitalRead(misfire) == LOW){
   comTaken = comTaken + 1;
   Serial.print("Components Dispensed: ");
   Serial.println(comTaken);
   delay(100);
 }


 if (comTaken == 0){
   digitalWrite(greenRGB, HIGH);
   digitalWrite(redRGB, LOW);
   delay(100);
 }


 if (comTaken > 0){
   digitalWrite(greenRGB, LOW);
   digitalWrite(redRGB, HIGH);
   delay(100);
 }
 currentTime = millis();


 // check if xxx seconds have passed
 if ((currentTime - startTime) >= 10000) {
   // call your function here
   HTTPrequest(comTaken);
   // reset the start time to the current time
   startTime = millis();
   // reset components taken variable
   comTaken = 0;
 }
 client.loop();
}
