#include <NewPing.h>

#define TRIGGER_PIN 22
#define ECHO_PIN 23
#define MAX_DISTANCE 200

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 

void setup() {
   Serial.begin(115200);
}
 
void loop() {
   delay(500);
   Serial.println("09/02/2022 3:10 pm");
   Serial.print(sonar.ping_cm());
   Serial.println("cm");   
}

  
