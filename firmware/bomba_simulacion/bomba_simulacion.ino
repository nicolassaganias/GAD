#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

const char* ssid = "Can_Saguer_Wifi";
const char* password = "cansaguer2";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;  // GMT+1 para Madrid
const int   daylightOffset_sec = 3600; // Horario de verano (cambiar a 0 en invierno)

const int ledPin = 13;

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado a WiFi");
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Error obteniendo la hora");
        delay(10000);
        return;
    }
    
    int currentMinute = timeinfo.tm_min;
    int currentSecond = timeinfo.tm_sec;
    
    if (currentMinute == 25 && currentSecond == 0) { // Justo a la hora en punto
        int delayTime = random(20, 61) * 10000; // Entre 10 y 60 segundos
        Serial.printf("Encendiendo LED por %d segundos\n", delayTime / 1000);
        digitalWrite(ledPin, HIGH);
        delay(delayTime);
        digitalWrite(ledPin, LOW);
    }
    
    delay(1000); // Espera 1 segundo antes de volver a comprobar
}
