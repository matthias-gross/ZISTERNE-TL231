#include <Arduino.h>

/*
D1-Mini ESP8266 
   --> 0-1023 --> 10 Bit
   --> D1 liefert bis 1024. Warum?
   --> 3,3 Volt wird am Board auf 1 Volt umgerechnet
   --> 3,3 Volt = 0 / 0 Volt = 1.024
NodeMCU ESP32
   --> 0-4095 --> 12 Bit
   --> 3,3 Volt = 0 / 0 Volt = 4095

*/


#if (DEVICE == 1)          // ESP32 NodeMCU
  #define ADC_PIN      34  // Pin zum Messen der Spannung
  #define RANGE      4096  // Digitalwert am Eingang (12Bit --> 2^12)
  #define WIDERSTAND  165  // Verbauter Widerstand in Ohm
  #define MAX_VOLTAGE 3.3  // Maximale Spannung am ADC
#elif (DEVICE == 2)        // ESP8266 D1 Mini
  #define ADC_PIN      A0  // Pin zum Messen der Spannung
  #define RANGE      1023  // Digitalwert am Eingang (10Bit --> 2^10)
  #define WIDERSTAND  165  // Verbauter Widerstand in Ohm
  #define MAX_VOLTAGE 3.3  // Maximale Spannung am ADC
  /*
     --> 0-1023 --> 10 Bit
     --> D1 liefert bis 1024. Warum?
  */
#elif (DEVICE == 3)        // Arduino Nano atmega328
  #define ADC_PIN      A0  // Pin zum Messen der Spannung
  #define RANGE      1023  // Digitalwert am Eingang (10Bit --> 2^10)
  #define WIDERSTAND  250  // Verbauter Widerstand in Ohm
  #define MAX_VOLTAGE   5  // Maximale Spannung am ADC
#endif

  #define WERT_STEIGUNG   RANGE/MAX_VOLTAGE
  #define WERT_MIN        0.004*WIDERSTAND*WERT_STEIGUNG
  #define WERT_MAX        0.02*WIDERSTAND*WERT_STEIGUNG

//Methode um Wasserstand umzurechnen
 float wasserstand(float digitalIn){
    digitalIn = RANGE-digitalIn;
    if(digitalIn < WERT_MIN){
      return 0;
    }else if(digitalIn > WERT_MAX){
      return 1;
    }else{
      return (digitalIn-WERT_MIN)/(WERT_MAX-WERT_MIN);
    }
 }


int digitalValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(WERT_MIN);
  Serial.println(WERT_MAX);
  Serial.println(WERT_STEIGUNG);
  delay(5000);
}

void loop() {
  digitalValue = analogRead(ADC_PIN);
  Serial.print(digitalValue);
  Serial.print(" - ");
  Serial.print(wasserstand(digitalValue));
  Serial.print(" - ");
  Serial.print(wasserstand(digitalValue)*500);
  Serial.println("cm ");
  //Serial.println(digitalValue);
  delay(100);
}