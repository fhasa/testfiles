/* IOT vertical farming centric*/


#include <ArduinoJson.h>

#include <WiFi.h>
WiFiClient WIFI_CLIENT;

#include <Wire.h>
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;

#include <OneWire.h>
#include <DallasTemperature.h>


#define WIFI_SSID "admin"
#define WIFI_PASSWORD "12345678"


const int big_pump1_spraying_pin = 22; //GPIO26
const int small_pump1_acid_pin = 23; //GPIO27
const int small_pump2_base_pin = 18; //GPIO32
const int small_pump3_nutrient_pin = 19; //GPIO33


#define ph_pin 36 //connected to GPIO 36 (Analog ADC0) 
#define tds_pin 39 //connected to GPIO 39 (Analog ADC3) 
#define ds18b20_pin 27 //connected to GPIO 27


//-------------------------------------------------TDS
int tds_SensorValue = 0;
float tdsValue = 0;
float tdsVoltage = 0;
//-------------------------------------------------


//-------------------------------------------------PH
int ph_sensorValue = 0;
float phValue = 0;
float ph = 0;
//-------------------------------------------------

//-------------------------------------------------ds18b20
OneWire oneWire(ds18b20_pin);         // setup a oneWire instance
DallasTemperature tempSensor(&oneWire); // pass oneWire to DallasTemperature library
float tempCelsius;    // temperature in Celsius
float tempFahrenheit; // temperature in Fahrenheit
//-------------------------------------------------



void setup()
{

  Serial.begin(115200);
  
  
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

  pinMode(tds_pin, INPUT);
  pinMode(ph_pin, INPUT);
  tempSensor.begin();    // initialize the sensor
  
  pinMode(big_pump1_spraying_pin, OUTPUT);
  pinMode(small_pump1_acid_pin, OUTPUT);
  pinMode(small_pump2_base_pin, OUTPUT);
  pinMode(small_pump3_nutrient_pin, OUTPUT);
  
  digitalWrite(big_pump1_spraying_pin, HIGH);
  digitalWrite(small_pump1_acid_pin, HIGH);
  digitalWrite(small_pump2_base_pin, HIGH);
  digitalWrite(small_pump3_nutrient_pin, HIGH);
  
}


String big_pump1_water_flow;
String small_pump1_acid;
String small_pump2_base;
String small_pump3_nutrient;


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
    
    

    MQTT_CLIENT.connect("client_iot_vertical_farming_ppu2022_centric"); // it isn't necessary..
    
    MQTT_CLIENT.subscribe("iot_vertical_farming_ppu2022/centric/big_pump1_water_flow"); // HERE SUBSCRIBE.
    MQTT_CLIENT.subscribe("iot_vertical_farming_ppu2022/centric/small_pump1_acid"); // HERE SUBSCRIBE.
    MQTT_CLIENT.subscribe("iot_vertical_farming_ppu2022/centric/small_pump2_base"); // HERE SUBSCRIBE.
    MQTT_CLIENT.subscribe("iot_vertical_farming_ppu2022/centric/small_pump3_nutrient"); // HERE SUBSCRIBE.
    

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
  
  if(stopic=="iot_vertical_farming_ppu2022/centric/big_pump1_water_flow")
  {
    big_pump1_water_flow=s_message;
  }
  else if(stopic=="iot_vertical_farming_ppu2022/centric/small_pump1_acid")
  {
    small_pump1_acid=s_message;
  }
  else if(stopic=="iot_vertical_farming_ppu2022/centric/small_pump2_base")
  {
    small_pump2_base=s_message;
  }
  else if(stopic=="iot_vertical_farming_ppu2022/centric/small_pump3_nutrient")
  {
    small_pump3_nutrient=s_message;
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
  
  //----------------------------------------------ph
  ph_sensorValue= analogRead(ph_pin);
  Serial.print(ph_sensorValue);
  Serial.print(" | ");
  phValue=ph_sensorValue*(3.3/4095.0);
  ph=14-(3.3*phValue);
  Serial.print("ph = "); 
  Serial.println(ph);
  //----------------------------------------------


  //----------------------------------------------tds
  tds_SensorValue = analogRead(tds_pin);
  tdsVoltage = tds_SensorValue*5/1024.0; //Convert analog reading to tdsVoltage
  tdsValue=(133.42/tdsVoltage*tdsVoltage*tdsVoltage - 255.86*tdsVoltage*tdsVoltage + 857.39*tdsVoltage)*0.5; //Convert voltage value to TDS value
  Serial.print("TDS Value = "); 
  Serial.print(tdsValue);
  Serial.println(" ppm");
  //----------------------------------------------

  //----------------------------------------------ds18b20
  tempSensor.requestTemperatures();             // send the command to get temperatures
  tempCelsius = tempSensor.getTempCByIndex(0);  // read temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(tempCelsius);    // print the temperature in Celsius
  Serial.print("°C");
  //----------------------------------------------
  
  if (millis() - previous_time > delay_ms) 
  {

    previous_time = millis();
    
    /*---------------------------------*/
    DynamicJsonDocument doc(1024);
    String serial_string;
    
    doc["ph"] = String(ph);
    doc["tds"]   = String(tdsValue);
    doc["water_temperature"] = String(tempCelsius);
    serializeJson(doc, serial_string);
    /*---------------------------------*/
    
    
    //------------test set
    do_publish("iot_vertical_farming_ppu2022/centric/sensor_value",String(serial_string));
    //------------
    
  }


  //======================function
  if(String(big_pump1_water_flow)==String("on"))
  {
      digitalWrite(big_pump1_spraying_pin, LOW);
      Serial.println("big_pump1_water_flow ON");
  }
  else if(String(big_pump1_water_flow)==String("off"))
  {
      digitalWrite(big_pump1_spraying_pin, HIGH);
      Serial.println("big_pump1_water_flow OFF");
  }
  
  if(String(small_pump1_acid)==String("on"))
  {
      digitalWrite(small_pump1_acid_pin, LOW);
      Serial.println("small_pump1_acid ON");
  }
  else if(String(small_pump1_acid)==String("off"))
  {
      digitalWrite(small_pump1_acid_pin, HIGH);
      Serial.println("small_pump1_acid OFF");
  }
  
  
  if(String(small_pump2_base)==String("on"))
  {
      digitalWrite(small_pump2_base_pin, LOW);
      Serial.println("small_pump2_base ON");
  }
  else if(String(small_pump2_base)==String("off"))
  {
      digitalWrite(small_pump2_base_pin, HIGH);
      Serial.println("small_pump2_base OFF");
  }
  
  
  if(String(small_pump3_nutrient)==String("on"))
  {
      digitalWrite(small_pump3_nutrient_pin, LOW);
      Serial.println("small_pump3_nutrient ON");
  }
  else if(String(small_pump3_nutrient)==String("off"))
  {
      digitalWrite(small_pump3_nutrient_pin, HIGH);
      Serial.println("small_pump3_nutrient OFF");
  }

  //======================

  MQTT_CLIENT.loop(); // Check Subscription.
  
  delay(1000);
    
}
