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
   // Setup timer and attach timer to a led pin
   ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
   ledcAttachPin(LED1, LEDC_CHANNEL_0);

   ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
   ledcAttachPin(LED2, LEDC_CHANNEL_1);

   ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
   ledcAttachPin(LED3, LEDC_CHANNEL_2);
}

void loop() {
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
   
   Serial.print("Frecuencia: ");
   Serial.print(frequency, 0);
   Serial.print(" (Hz)\tCaudal: ");
   Serial.print(flow_Lmin, 3);
   Serial.println(" (L/min)");
   Serial.print (volumen,3); 
   Serial.println (" L");
  
  //if (volumen >= 0.2) 
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);
  //if (volumen >= 0.5) 
  ledcAnalogWrite(LEDC_CHANNEL_1, brightness);
  //if (volumen >= 1.0) 
  ledcAnalogWrite(LEDC_CHANNEL_2, brightness);
  
  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
}
