#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Wire.h>
#include <AsyncMqttClient.h>
#include <PubSubClient.h>

//declaracion de variables de red wifi
#define WIFI_SSID "FDM TRES"
#define WIFI_PASSWORD "contra33"

//Broker MQTT RBPI

#define MQTT_HOST IPAddress(192,168,30,164)
#define MQTT_PORT 1883

//definición de TOPICS

#define MQTT_PUB_TEMP "esp/dht/temperature"
#define MQTT_PUB_HUM "esp/dht/humidity"


//definicion GPIO DHT
#define DHTPIN 4

//definición tipo de DHT 
#define DHTTYPE DHT22


//inicialización de DHT

DHT dht(DHTPIN,DHTTYPE);

//variables para la lectura del sensor

float temp;
float hum;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis=0; //almacena el último valor de temperatura publicado
const long interval=10000;


void ConectarAWiFi(){
  Serial.println("Conectando a WiFi...");
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
}

void ConectadoAWiFi(const WiFiEventStationModeGotIP& event){
  Serial.println("Conectado a WiFi");
  ConectarAMqtt();
}

void DesconectadoDeWiFi(const WiFiEventStationModeDisconnected& event){
  Serial.println("Desconectado de WiFi");
  mqttReconnectTimer.detach();
  wifiReconnectTimer.once(2,ConectarAWiFi);
}

void ConectarAMqtt(){
  Serial.println("Conectando a MQTT...");
  mqttClient.connect();
}

void ConectadoAMqtt(bool sessionPresent){
  Serial.println("Conectado a MQTT.");
  Serial.print("Sesion presente:");
  Serial.println(sessionPresent);
}

void DesconectadoDeMqtt(AsyncMqttClientDisconnectReason reason){
  Serial.println("Desconectado de MQTT");
  if (WiFi.isConnected()){
    mqttReconnectTimer.once(2,ConectarAMqtt);
    }
  
}

void MqttPublicado(uint16_t packetId){
  Serial.print("Publicacion reconocida");
  Serial.print(" packetId: ");
  Serial.println(packetId); 
  
}


void setup() {
  Serial.begin(115200);
  Serial.println();

  dht.begin();

  wifiConnectHandler=WiFi.onStationModeGotIP(ConectadoAWiFi);
  wifiDisconnectHandler=WiFi.onStationModeDisconnected(DesconectadoDeWiFi);

  mqttClient.onConnect(ConectadoAMqtt);
  mqttClient.onDisconnect(DesconectadoDeMqtt);

  mqttClient.onPublish(MqttPublicado);
  mqttClient.setServer(MQTT_HOST,MQTT_PORT);

  ConectarAWiFi();
  
  
    
  
}

void loop() {
  unsigned long currentMillis=millis();

  if(currentMillis-previousMillis>= interval){
    previousMillis=currentMillis;

    hum=dht.readHumidity();
    temp=dht.readTemperature();
    
    uint16_t packetIdPub1=mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());
    Serial.printf("Publicando en Topic %s con QoS 1, packetId %i: ", MQTT_PUB_TEMP,packetIdPub1);
    Serial.printf("mensaje: %.2f \n",temp);
    
    uint16_t packetIdPub2=mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());
    Serial.printf("Publicando en Topic %s con QoS 1, packetId %i: ", MQTT_PUB_HUM,packetIdPub2);
    Serial.printf("mensaje: %.2f \n",hum);
    

    
  }
  

}
