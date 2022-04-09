#include <WiFi.h>
#include <PubSubClient.h>

#define WIFISSID "Lentisimo" //WIFI SSID aqui
#define PASSWORD "WiFi1361" // WIFI pwd 
#define TOKEN "BBFF-zYZGrqqAov1v3srMpvQ3OWr6Z7d0XF" // Ubidots TOKEN name el mismo que usamos en clase
#define MQTT_CLIENT_NAME "666aaa666" //ID del cliente, 8 a 12 chars alfanumericos(ASCII), debe ser random y unico dif a otros devices
#define VARIABLE_LABEL_flow "flujo" // Variable Temperatura
#define VARIABLE_LABEL_volume "volumen" // Variable Humedad
#define DEVICE_LABEL "Flux" // Nombre del dispositivo a crear
char mqttBroker[] = "industrial.api.ubidots.com";
char payload[200]; // Leer y entender el payload aqui una de tantas referencias "https://techterms.com/definition/payload"
char topic[150]; //Espacio para el nombre del topico

//hacer cambios para que almacenen el volumen y el flujo
char str_flow[10];
char str_vol[10];

WiFiClient ubidots;
PubSubClient client(ubidots);

const int sensorPin = 13; //sensor que va a recibir los datos/pulsos del sensor
#define LED1  15
#define LED2  2
#define LED3  4 //pin de salida del foquito LED

// use first channel of 16 channels (started from zero) for the LED control
#define LEDC_CHANNEL_0     0
#define LEDC_CHANNEL_1     1
#define LEDC_CHANNEL_2     2

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

const int measureInterval = 2500; //intervalo de medicion del caudal/pulso
volatile int pulseConter; //contador de pulsos
float volumen=0;
long dt=0; //variación de tiempo por cada bucle
long t0=0; //millis() del bucle anterior
 
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    Serial.write(payload, length);
    Serial.println(topic);
  }

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");
      // Attemp to connect
      if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
        Serial.println("Connected");
      } else {
        Serial.print("Failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 2 seconds");
        // Wait 2 seconds before retrying
        delay(2000);
      }
    }
  }

// ARD-370 - Modelo de nuestro caudalimetro
const float factorK = 7.5; //factor de la frecuencia (Hz( del pulso o flujo del caudal
//segun el fabricante, en este caso Steren
 

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

//funcion que cuenta y suma los pulsos
void ISRCountPulse()
{
   pulseConter++;
}

//funcion para obtener la cantidad de pulsos en el intervalo establecido al principio
float GetFrequency()
{
   pulseConter = 0;
 
   interrupts();
   delay(measureInterval);
   noInterrupts();
 
   return (float)pulseConter * 1000 / measureInterval;
}
 
void setup()
{
   Serial.begin(115200);
   attachInterrupt(digitalPinToInterrupt(sensorPin), ISRCountPulse, RISING);
   Serial.println ("Envie 'r' para restablecer el volumen a 0 Litros"); 
   t0=millis();
   //pone a cada pin de LED en modo output y lo pone en estado low/apagado/luz baja
   /*
   pinMode(LED1, OUTPUT);
   digitalWrite(LED1, LOW);
   pinMode(LED2, OUTPUT);
   digitalWrite(LED2, LOW);
   pinMode(LED3, OUTPUT);
   digitalWrite(LED3, LOW); 
   */

   // Setup timer and attach timer to a led pin
   ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
   ledcAttachPin(LED1, LEDC_CHANNEL_0);

   ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
   ledcAttachPin(LED2, LEDC_CHANNEL_1);

   ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
   ledcAttachPin(LED3, LEDC_CHANNEL_2);

   WiFi.begin(WIFISSID, PASSWORD);
    Serial.println();
    Serial.print("Wait for WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("WiFi Connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);
}

void loopLED(float volumen) {
  
  if (volumen >= 0.2) {ledcAnalogWrite(LEDC_CHANNEL_0, brightness);}
  //else digitalWrite(LED1, LOW);
  
  if (volumen >= 0.5) {ledcAnalogWrite(LEDC_CHANNEL_1, brightness);}
  //else digitalWrite(LED2, LOW);
  
  if (volumen >= 1.0) {ledcAnalogWrite(LEDC_CHANNEL_2, brightness);}
  //else digitalWrite(LED3, LOW);
  
  // set the brightness on LEDC channel 0
  /*
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);
  ledcAnalogWrite(LEDC_CHANNEL_1, brightness);
  ledcAnalogWrite(LEDC_CHANNEL_2, brightness);
  */

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
}


float loopFlow()
{
   if (Serial.available()) {
    if(Serial.read()=='r'){
      volumen=0;//restablecemos el volumen si recibimos 'r'
      ledcAnalogWrite(LEDC_CHANNEL_0, 0);
      ledcAnalogWrite(LEDC_CHANNEL_1, 0);
      ledcAnalogWrite(LEDC_CHANNEL_2, 0);
      }
   }
   
   // obtener frecuencia en Hz
   float frequency = GetFrequency();
 
   // calcular caudal L/min
   float flow_Lmin = frequency / factorK;

   // obtener volumen en Litros
   dt=millis()-t0; //calculamos la variación de tiempo
   t0=millis();
   volumen=volumen+(flow_Lmin/60)*(dt/1000); // volumen(L)=caudal(L/s)*tiempo(s)

   /*
   if (volumen >= 0.2) {digitalWrite(LED1, HIGH);}
   if (volumen >= 0.5) {digitalWrite(LED2, HIGH);}
   if (volumen >= 1.0) {digitalWrite(LED3, HIGH);}
   */
   
   Serial.print("Frecuencia: ");
   Serial.print(frequency, 0);
   Serial.print(" (Hz)\tCaudal: ");
   Serial.print(flow_Lmin, 3);
   Serial.println(" (L/min)");
   Serial.print (volumen,3); 
   Serial.println (" L");
   return flow_Lmin;
}

float loopVol(){
   // obtener frecuencia en Hz
   float frequency = GetFrequency();
 
   // calcular caudal L/min
   float flow_Lmin = frequency / factorK;

   // obtener volumen en Litros
   dt=millis()-t0; //calculamos la variación de tiempo
   t0=millis();
   volumen=volumen+(flow_Lmin/60)*(dt/1000); // volumen(L)=caudal(L/s)*tiempo(s)
   return volumen;
  }

void mqttPublish(float flow, float vol) {
    if (!client.connected()) {
      reconnect();
    }
    // Publica en el topic de flujo
    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_flow); // Adds the variable label
    /* numero maximo 4 precision 2 y convierte el valor a string*/
    dtostrf(flow, 4, 2, str_flow);
    sprintf(payload, "%s {\"value\": %s", payload, str_flow); // formatea el mensaje a publicar
    sprintf(payload, "%s } }", payload); // cierra el mensaje
    Serial.println("Publicando flujo en Ubidots cloud");
    client.publish(topic, payload);
    // Publica en el topic de volumen
    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_volume); // Adds the variable label
    /* numero maximo 4 precision 2 y convierte el valor a string*/
    dtostrf(vol, 4, 2, str_vol);
    sprintf(payload, "%s {\"value\": %s", payload, str_vol); // formatea el mensaje a publicar
    sprintf(payload, "%s } }", payload); // cierra el mensaje
    Serial.println("Publicando volumen en Ubidots cloud");
    client.publish(topic, payload);
    client.loop();
    delay(5000); // 15 segundos en milisegundos entre publicaciones en ubidots
  }

void loop()
{
  float flow = loopFlow();
  float vol = loopVol();
  loopLED(vol);
  mqttPublish(flow, vol);
}
