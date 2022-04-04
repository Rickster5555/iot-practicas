#include <DHT.h>
#include <DHT_U.h>
#define pin1 13 //ESPECIFICAR PIN
DHT dht1(pin1, DHT11); //INICIALIZAR DHT
#define PIN_LED 02 //ESPECIFICAR PIN LED
#include "thingProperties.h"

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);
  // Defined in thingProperties.h
  initProperties();
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  /*
  The following function allows you to obtain more information
  related to the state of network and IoT Cloud connection and errors
  the higher number the more granular information you’ll get.
  The default is 0 (only errors).
  Maximum is 4
  */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  pinMode(PIN_LED,OUTPUT); //DEFINIR LED COMO OUTPUT
  dht1.begin(); //INICIAR DHT
}

void loop() {
  ArduinoCloud.update();
  // Your code here
  temperatura = dht1.readTemperature(); //ASIGNAR LA LECTURA DE LA TEMPERATURA A LA VARIABLE "temp" GENERADA EN THING "SMARTHOME"
  Serial.println(temperatura); //IMPRIMIR EN EL MONITOR SERIAL LA TEMPERATURA
  humedad = dht1.readHumidity();
  Serial.println(humedad);
  delay(5000);
}
  /*
  Since LedLight is READ_WRITE variable, onLedLightChange() is
  executed every time a new value is received from IoT Cloud.
  */
void onLuzChange() {
  // Add your code here to act upon LedLight change
  digitalWrite(PIN_LED, luz); //ASIGNAR EL ESTADO DEL LED A LA VARIABLE "ledLight"
}

void onHumedadChange() {
  Serial.println("La humedad ha cambiado");
}
