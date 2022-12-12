/* IOT vertical farming branch2*/


#include <ArduinoJson.h>

#include <WiFi.h>
WiFiClient WIFI_CLIENT;

#include <Wire.h>
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;


#define WIFI_SSID "admin"
#define WIFI_PASSWORD "12345678"


//-------------------------------
#define ldr_pin 36 //connected to GPIO 36 (Analog ADC0) 
#define DHTPin 18 
#define light_pin 19 
//-------------------------------



#include "DHT.h"
#define DHTTYPE DHT11 // DHT 11 (AM2302), AM2321
//DHT Sensor;
DHT dht(DHTPin, DHTTYPE);
float Temperature;
float Humidity;
float Temp_Fahrenheit;




float ldrValue = 0;






void setup()
{

  Serial.begin(115200);
  dht.begin();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  MQTT_CLIENT.setCallback(callback);

  pinMode(ldr_pin, INPUT);
  pinMode(DHTPin, INPUT);
  pinMode(light_pin, OUTPUT);

  digitalWrite(light_pin, HIGH);

  
}



String light;


// Reconnect con MQTT broker
void reconnect() 
{
  MQTT_CLIENT.setServer("broker.hivemq.com", 1883);  
  //MQTT_CLIENT.setServer("mqtt.eclipse.org", 1883);
  MQTT_CLIENT.setClient(WIFI_CLIENT);

  // Trying connect with broker.
  while (!MQTT_CLIENT.connected()) 
  {
    Serial.println("Trying to connect with Broker MQTT.");
    
    

    MQTT_CLIENT.connect("client_iot_vertical_farming_ppu2022_branch2"); // it isn't necessary..
    

    MQTT_CLIENT.subscribe("iot_vertical_farming_ppu2022/branch2/light"); // HERE SUBSCRIBE.
 

    // Wait to try to reconnect again...
    delay(1000);
  }

  Serial.println("Conectado a MQTT.");
}



String message = "";
// What to do when it receives the data. 
void callback(char* topic, byte* payload, unsigned int length) 
{
  String stopic = String(topic);
  
  Serial.print("Message received: ");
  Serial.print(stopic);
  Serial.print("   ");
  message = "";
  for (int i=0;i<length;i++) 
  {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  String s_message=String(message);
  
  if(stopic=="iot_vertical_farming_ppu2022/branch2/light")
  {
    light=s_message;
  }
  

}

void do_publish(String topic,String msg)
{
    char msg_char[100];
    msg.toCharArray(msg_char, 100);

    char topic_char[100];
    topic.toCharArray(topic_char, 100);

    MQTT_CLIENT.publish(topic_char, msg_char);
}



int delay_ms=5*1000;// 5*1000 = 5 sec
uint32_t previous_time=0;



void loop()
{
  // Check MQTT Broker connection
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
  
  //----------------------------------------------dht
  Humidity = dht.readHumidity();
  Temperature = dht.readTemperature();
  Serial.print(F("Humidity: "));
  Serial.println(Humidity);
  Serial.print(F("%  Temperature: "));
  Serial.println(Temperature);
  if (isnan(Humidity) || isnan(Temperature)) 
  {
      Serial.println(F("Failed to read from DHT sensor!"));
      
  }
  //----------------------------------------------


  //----------------------------------------------ldr
  ldrValue = analogRead(ldr_pin);
  Serial.print("ldr Value = "); 
  Serial.print(ldrValue);
  Serial.println(" ppm");
  //----------------------------------------------


  if (millis() - previous_time > delay_ms) 
  {

    previous_time = millis();
    
    /*---------------------------------*/
    DynamicJsonDocument doc(1024);
    String serial_string;
    
    doc["air_temperature"] = String(Humidity);
    doc["air_humidity"] = String(Temperature);
    doc["light_intensity"] = String(ldrValue);
    serializeJson(doc, serial_string);
    /*---------------------------------*/
    
    
    //------------test set
    do_publish("iot_vertical_farming_ppu2022/branch2/sensor_value",String(serial_string));
    //------------
    
  }


  //======================function
  if(String(light)==String("on"))
  {
      digitalWrite(light_pin, LOW);
      Serial.println("light ON");
  }
  else if(String(light)==String("off"))
  {
      digitalWrite(light_pin, HIGH);
      Serial.println("light OFF");
  }
  //======================
  
  

  MQTT_CLIENT.loop(); // Check Subscription.
  
  delay(1000);
    
}
