#include <Ubidots.h>
#include "DHT.h"
#define pin1 13
#define LED 12
DHT dht1(pin1, DHT11);
String estado = "";

// "IoT"
// "1t3s0IoT18"
// "Lentisimo"
// "WiFi1361"

const char* UBIDOTS_TOKEN = "BBFF-zYZGrqqAov1v3srMpvQ3OWr6Z7d0XF";
const char* WIFI_SSID = "IoT";
const char* WIFI_PASS = "1t3s0IoT18";
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

void setup() {
  Serial.begin(115200);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  ubidots.setDebug(true);
  dht1.begin();
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void loop() {
  //float h1 = random(0, 9) * 100;
  //float t1 = random(0, 9) * 30;
  
  float t1 = dht1.readTemperature();
  float h1 = dht1.readHumidity();
  bool led = 0;

  ubidots.add("humedad", h1); 
  ubidots.add("temperatura", t1);
  
  if (t1 > 28) {digitalWrite(LED, HIGH); estado = "Encendido"; led = 1;}
  if (t1 < 26){digitalWrite(LED, LOW); estado = "Apagado"; led = 0;}
  Serial.println("Temperatura");
  Serial.println(t1);
  ubidots.add("LED", led);
  
  bool bufferSent = false;
  bufferSent = ubidots.send();  // Will send data to a device label that matches the device Id

  if (bufferSent) {
    // Do something if values were sent properly
    //Serial.println("humedad: ");
    //Serial.print(h1);
    //Serial.println("temperatura: ");
    //Serial.print(t1);
    Serial.println("Values successfully sent");
    }
    Serial.println(estado);

  delay(5000);

}
