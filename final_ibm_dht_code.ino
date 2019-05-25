//----------------------OLED-----------------------------------//
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SSD1306_LCDHEIGHT 64
#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(-1);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
//-----------------------WIFI-----------------------------------//
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//-----------------------ARDUINO TRANSFER-----------------------------------//
#include<SoftwareSerial.h>
SoftwareSerial myserial(D7,D8);
//--------------------------------------------------------------//

void callback(char* topic, byte* payload, unsigned int payloadLength);

// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "E=MC^2";
const char* password = "Incorrect@2019";

// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX
#define ORG "i4lugi"
#define DEVICE_TYPE "NODE_MCU"
#define DEVICE_ID "1024"
#define TOKEN "nodemcu007" //  Authentication Token OF THE DEVICE

//------------------LDR AND MOTOR--------------------------//
#define ldr A0
#define motor D6
//---------------------DHT------------------------------//
#include "DHT.h"  
#define DHTPIN D3   
#define DHTTYPE DHT11   
DHT dht (DHTPIN, DHTTYPE);
float temperature;
int humidity;
//--------------------------------------------------------//
String data3;
String data="";
String data1;
int val;
String dataval;

//-------- Customise the above values --------
const char publishTopic[] = "iot-2/evt/Data/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/data/fmt/String";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;


WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 5000; // 30 seconds
long lastPublishMillis;
void publishData();

void setup() {
 pinMode(D3,OUTPUT);
 pinMode(D6,OUTPUT);
 Serial.begin(115200);
 myserial.begin(115200);
 Serial.println();
 dht.begin(); 
 wifiConnect();
 mqttConnect();
//--------------------------OLED--------------------------//
display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(0,10);
//-----------------------------------------------------//
}

void loop() {
if(myserial.available()){
myserial.find('#');
data1=myserial.readStringUntil('@');
val=String(data1).toInt();
Serial.println(val);
delay(500);}
if (millis() - lastPublishMillis > publishInterval)
{
publishData(data1);
lastPublishMillis = millis();
}
if (!client.loop()) {
mqttConnect();
}
 if(dataval=="true"){
  if(val<=65){
   digitalWrite(D6,HIGH);  
Serial.println(".......MOTOR is ON..........");
display.clearDisplay();
display.setCursor(0,5);
display.print("Soil moisture low\n");
display.setCursor(0,20);
display.print("Motor on");
display.display();
delay(1000);}
  
  if(val>65){
    digitalWrite(D6,LOW);  
Serial.println(".......MOTOR is OFF..........");
display.clearDisplay();
display.setCursor(0,5);
display.print("Soil moisture normal\n");
display.setCursor(0,20);
display.print("Motor off");
display.display();
  }

}
}


void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    initManagedDevice();
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe(topic)) {
   // Serial.println(client.subscribe(topic));
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
Serial.print("callback invoked for topic: ");
Serial.println(topic);
for (int i = 0; i < payloadLength; i++) {
//Serial.print((char)payload[i]);
data3 += (char)payload[i];
}
Serial.print("data: "+ data3);
control_func();
data3 = "";
}

void control_func()
{
dataval=data3;
if(data3 == "MOTORON")
{
digitalWrite(D6,HIGH);  
Serial.println(".......MOTOR is ON..........");
}
else if(data3=="MOTOROFF")
{
digitalWrite(D6,LOW); 
Serial.println(".......MOTOR is OFF..........");
}
if(data3=="false"){
  digitalWrite(D6,LOW); 
  display.clearDisplay();
display.setCursor(0,10);
display.print("water automation:off");
display.display();
delay(1000);
}

}



void publishData(String data1) 
{
humidity = dht.readHumidity();
temperature = dht.readTemperature();
float ldr_val=analogRead(ldr);

if (isnan(humidity) || isnan(temperature)) {
Serial.println("Failed to read from DHT sensor!");
return;
}
display.clearDisplay();
display.setCursor(0,10);
display.print("humidity:"+String(humidity)+"%"+"\n");
display.display();
delay(1000);

display.clearDisplay();
display.setCursor(0,10);
display.print("temperature:"+String(temperature)+"C"+"\n");
display.display();
delay(1000);

display.clearDisplay();
display.setCursor(0,10);
display.print("LDR value:"+String(ldr_val)+"\n");
display.display();
delay(1000);

display.clearDisplay();
display.setCursor(0,10);
display.print("moisture value:"+data1+"\n");
display.display();
delay(1000);
  
  String payload = "{\"d\":{\"temperature\":";
  payload += temperature;
  payload += ",""\"humidity\":";
  payload +=  humidity;
  payload += ",""\"LDR\":";
  payload +=  ldr_val;
  payload += ",""\"soilmoisture\":";
  payload +=data1;
  payload += "}}";


  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}
