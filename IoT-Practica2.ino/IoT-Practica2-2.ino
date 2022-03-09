#include "DHT.h"

#define pin1 13
DHT dht1(pin1, DHT11);

void setup() {
  Serial.begin(115200);
  Serial.println("Test de sensores");
  dht1.begin();
}

void loop() {
  delay(5000);
  leerdht1();
}

void leerdht1(){
  float t1 = dht1.readTemperature();
  float h1 = dht1.readHumidity();

  while(isnan(t1) || isnan(h1)){
    Serial.println("Lectura fallida en el sensor DHT11, repitiendo lectura...");
    delay(5000);
    t1 = dht1.readTemperature();
    h1 = dht1.readHumidity();
  }

  Serial.print("Temperatura DHT11:");
  Serial.print(t1);
  Serial.print("ºC.");

  Serial.print("Humedad DHT11:");
  Serial.print(h1);
  Serial.println(" % ");

  Serial.println("-------------------------------");
  Serial.println("08/02/2022 7:43 pm");
    
}
