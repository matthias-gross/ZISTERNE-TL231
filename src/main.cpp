/*
#############################################
##        Allgemeine Dokumentation         ##
#############################################

Verkabelung
 PIN 34
 GND

*/

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>


//#############################################
//##         Allgemeine Definitionen         ##
//#############################################
#define uS_TO_S_FACTOR 1000000                          // Umwandeln von Mikrosekunden zu Sekunden
#define TIME_TO_SLEEP  20                               // Zeit, die der ESP32 schläft (in Sekunden) 900 Sekunden = 15 Minuten
#define ADC_PIN 34                                      // Pin zum Messen der Spannung
#define RANGE 4095                                      // Digitalwert am Eingang (12Bit --> 2^12)
#define WIDERSTAND 165                                  // Verbauter Widerstand in Ohm
#define MAX_VOLTAGE 3.3                                 // Maximale Spannung am ADC
#define WERT_STEIGUNG RANGE / MAX_VOLTAGE               // Berechnung der Steigung um den Wasserstand zu ermitteln
#define WERT_MIN 0.004 * WIDERSTAND * WERT_STEIGUNG     // Minimalwert des Wasserstands --> 0cm
#define WERT_MAX 0.02  * WIDERSTAND * WERT_STEIGUNG     // Maximalwert des Wasserstands --> 500cm
const char* device_name         = "ESP32-Zisterne";     // Name des Geräts (Wird für Hostname und MQTT versendet)
const char* wlan_ssid           = "AT-15";              // Wlan Name
const char* wlan_password       = "xxxx";               // Wlan Passwort
const char* mqtt_server         = "192.168.1.2";        // MQTT-Server IP
const int   mqtt_port           = 1883;                 // MQTT-Port
const char* mqtt_topic_status   = "ZISTERNE/STATUS";    // MQTT-Topic zum Status
const char* mqtt_topic_werte    = "ZISTERNE/WERTE";     // MQTT-Topic für die Werteübermittlung
char        mqtt_json[300];                             // MQTT-Inhalt

int digitalValue[20];                                   // Array um mehrere Werte einzulesen und später den Median zu übermitteln
int laengeArray = sizeof(digitalValue) / sizeof(int);   // Länge des Arrays ermitteln.
int digitalValueMedian = 0;                             // Variable um den Median zu speichern

//#############################################
//##   Funktion um Wasserstand umzurechnen   ##
//#############################################
float wasserstand(float digitalIn){
  if (digitalIn < WERT_MIN){
    return 0;
  } else if (digitalIn > WERT_MAX){
    return 1;
  } else {
    return (digitalIn - WERT_MIN) / (WERT_MAX - WERT_MIN);
  }
}

//#############################################
//##     Funktion um Werte zu verbessern     ##
//#############################################
//Polynomfunktion zur Verbesserung der Ergebnisse
//Quelle: https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
float KorrekturADC(int digitalIn){
  if(digitalIn < 1 || digitalIn > 4095) return 0;
  return RANGE / MAX_VOLTAGE * (-0.000000000000016 * pow(digitalIn,4) + 0.000000000118171 * pow(digitalIn,3)- 0.000000301211691 * pow(digitalIn,2)+ 0.001109019271794 * digitalIn + 0.034143524634089);
}

//#############################################
//##       Funktion um Wifi aufzubauen       ##
//#############################################
void connectToWifi() {
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(wlan_ssid);
   
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(device_name); 
  WiFi.begin(wlan_ssid, wlan_password);

  Serial.print("Connecting to WiFi ..");
  int tryCount = 0;
  while (WiFi.status() != WL_CONNECTED && tryCount <= 12) {
    delay(500);
    Serial.print(F("."));
    tryCount++;
  }
  if (WiFi.status() != WL_CONNECTED){
    Serial.print("ESP startet neu.");
    ESP.restart();
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}


//#############################################
//##       Funktion um MQTT aufzubauen       ##
//#############################################
WiFiClient espClient;
PubSubClient client(espClient);
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  client.setKeepAlive( 400 );
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      //connect         (clientID   , willTopic         , willQoS, willRetain, willMessage)
      client.setKeepAlive( 400 );
      if (client.connect(device_name, mqtt_topic_status , 0      , "true"    , "OFF")) {
        Serial.println("MQTT connected");
        client.publish(mqtt_topic_status, "ON", "true");
        Serial.println("MQTT PUBLISH Status");
        
        // Subscribe --> Hier Topic eingeben, die zu empfangen sind...
        //client.subscribe(topic_set_intervall2);
      } else {
        //Serial.print("failed, rc=");
        //Serial.print(client.state());
        //Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
}


void setup() {
  Serial.begin(115200);

  //Werte mehrmals auslesen, dass später der Median gebildet werden kann
  for (int i = 0; i < laengeArray; i++ ){
    digitalValue[i] = analogRead(ADC_PIN);

    /*
    Serial.print(digitalValue);
    Serial.print(" - ");
    Serial.print(digitalKorrektur);
    Serial.print(" - ");
    Serial.print(wasserstand(digitalKorrektur));
    Serial.print(" - ");
    Serial.print(wasserstand(digitalKorrektur) * 500);
    Serial.println("cm ");
    */

    //Serial.println(digitalValue);
    delay(1000); //1 Sekunde warten
  }


  //Array sortieren
  int temp = 0;
  for(int i = 0; i < laengeArray; i++) {
    for(int j = i+1; j < laengeArray; j++) {
      if(digitalValue[j] < digitalValue[i]) {
         temp = digitalValue[i];
         digitalValue[i] = digitalValue[j];
         digitalValue[j] = temp;
      }
    }
  }

  //Median ermitteln
  if (laengeArray % 2 != 0){
    digitalValueMedian = digitalValue[laengeArray/2];
  } else {
    digitalValueMedian = (digitalValue[(laengeArray-1)/2] + digitalValue[laengeArray/2])/2;
  }

  connectToWifi();
  connectToMqtt();

  float digitalKorrektur = 0;
  digitalKorrektur = KorrekturADC(digitalValueMedian);
  strcat(mqtt_json, "{\"ZISTERNE\":{");
    strcat(mqtt_json, "\"WASSERSTAND\":");
    strcat(mqtt_json, String(wasserstand(digitalKorrektur)*500).c_str());
    strcat(mqtt_json, ",\"DIGTIAL_IN\":");
    strcat(mqtt_json, String(digitalValueMedian).c_str());
    strcat(mqtt_json, ",\"DIGTIAL_KORREKTUR\":");
    strcat(mqtt_json, String(digitalKorrektur).c_str());
    /*
    strcat(mqtt_json, ",\"Values\":{");
      for(int i = 0; i < laengeArray; i++) {
        strcat(mqtt_json, "\"");
        strcat(mqtt_json, String(i+1).c_str());
        strcat(mqtt_json, "\":{");
          strcat(mqtt_json, "\"WASSERSTAND\":");
          digitalKorrektur = KorrekturADC(digitalValue[i]);
          strcat(mqtt_json, String(wasserstand(digitalKorrektur)*500).c_str());
          strcat(mqtt_json, ",\"DIGTIAL_IN\":");
          strcat(mqtt_json, String(digitalValue[i]).c_str());
          strcat(mqtt_json, ",\"DIGTIAL_KORREKTUR\":");
          strcat(mqtt_json, String(digitalKorrektur).c_str());
        if(i == laengeArray-1){
          strcat(mqtt_json, "}}");
        } else {
          strcat(mqtt_json, "},");
        }
      }
      */
  strcat(mqtt_json, ",\"RSSI\":");
  strcat(mqtt_json, String(WiFi.RSSI()).c_str());
  strcat(mqtt_json, "}}");

  Serial.println(mqtt_json);
  client.publish(mqtt_topic_werte, mqtt_json);
  
  delay(15000);
  Serial.println("ESP geht in den Deep Sleep");
  Serial.flush(); //Wartet, bis die Übertragung der ausgehenden seriellen Daten abgeschlossen ist. 
  WiFi.disconnect();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Hier wird nie etwas ausgeführt
}