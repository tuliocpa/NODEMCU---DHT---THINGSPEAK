//OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

//include wifi
#include <ESP8266WiFi.h>
const char* ssid = "   ";
const char* password = "   ";
WiFiClient nodemcuClient;

//Thingspeak
#define INTERVALO_ENVIO_THINGSPEAK  30000
char EnderecoAPIThingSpeak[] = "api.thingspeak.com";
String ChaveEscritaThingSpeak = "  ";
long lastConnectionTime;
char FieldTemperatura[11];
char FieldUmidade[11];

/*
//MQTT
#include <PubSubClient.h>
//raspberry : const char* mqtt_server = "192.168.0.105"; 
const char* mqtt_server = "m10.cloudmqtt.com";
const char* mqtt_ClientID = "termo1";
PubSubClient client(nodemcuClient);
const char* topicoTemperatura = "lab/temperatura";
const char* topicoUmidade = "lab/umidade";
int porta = 16700;
const char* mqtt_user = "encktxur";
const char* mqtt_pass = "Rzvc5InVlteX";
*/
//include display
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

//include DHT
#include <DHT.h>
#define DHTPIN D3
#define DHTTYPE DHT22 //Seleciona o tipo de sensor
float umidade;
float temperatura;
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  //Serial.begin(9600);
  configurarDisplay();
  conexaoWifi();
  iniciaOTA();
  lastConnectionTime = 0;
  //pinMode(LED_BUILTIN, OUTPUT); 
  //client.setServer(mqtt_server, porta);

}
void loop(){
 /* if(!client.connected()){
    reconectarMQTT();
  }*/
  ArduinoOTA.handle();
  medirTempUmid();
  mostraTempUmid();
//  publicarMQTT();
  carregaTempThingSpeak();
  nodemcuClient.stop();
}


void conexaoWifi(){
  delay(10);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("Conectando");
  display.display();
  
  WiFi.hostname("ESP8266 1");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi connected");
  display.display();
  delay(1000);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("IP address");
  display.display();
  display.println(WiFi.localIP());
  display.display();
  delay(4000);
}
/*
void publicarMQTT(){
  client.publish(topicoTemperatura, String(temperatura).c_str(),true);
  client.publish(topicoUmidade, String(umidade).c_str(),true);
}

void reconectarMQTT(){
   while (!client.connected()) {
    client.connect(mqtt_ClientID, mqtt_user, mqtt_pass);
   }
}
*/
void medirTempUmid() {
  umidade = dht.readHumidity();
  temperatura = dht.readTemperature(false);
  delay(2000);
}
void mostraTempUmid(){
  mostraDisplay("Temperatura",(temperatura),"C");
  delay(2000);
  mostraDisplay("Umidade",(umidade),"%");
}

void configurarDisplay(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  display.clearDisplay();
}

void mostraDisplay(const char* texto1,float medicao,const char* texto2){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(texto1);
  display.setTextSize(3);
  display.setCursor(20,20);
  display.print(medicao);
  display.setTextSize(2);
  display.print(texto2);
  display.display();
  delay(2000);
  
}

void iniciaOTA(){
      ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
  });
  
  ArduinoOTA.begin();
}

void envioThingSpeak(String StringDados1,String StringDados2){

    String StringDados;
    StringDados += StringDados1;
    StringDados += StringDados2;
    
    if (nodemcuClient.connect(EnderecoAPIThingSpeak, 80))
    {        
        //faz a requisição HTTP ao ThingSpeak
        nodemcuClient.print("POST /update HTTP/1.1\n");
        nodemcuClient.print("Host: api.thingspeak.com\n");
        nodemcuClient.print("Connection: close\n");
        nodemcuClient.print("X-THINGSPEAKAPIKEY: "+ChaveEscritaThingSpeak+"\n");
        nodemcuClient.print("Content-Type: application/x-www-form-urlencoded\n");
        nodemcuClient.print("Content-Length: ");
        nodemcuClient.print(StringDados.length());
        nodemcuClient.print("\n\n");
        nodemcuClient.print(StringDados);
  
        lastConnectionTime = millis();

    } 
}

void carregaTempThingSpeak(){
  int temperaturaTrunc = temperatura;
  int umidadeTrunc = umidade;
      if(!nodemcuClient.connected() &&
      (millis() - lastConnectionTime > INTERVALO_ENVIO_THINGSPEAK))
    {
        sprintf(FieldTemperatura,"&field1=%d",temperaturaTrunc);
        sprintf(FieldUmidade,"&field2=%d",umidadeTrunc);
        envioThingSpeak(FieldTemperatura,FieldUmidade);
    }
}

//inicio teste
