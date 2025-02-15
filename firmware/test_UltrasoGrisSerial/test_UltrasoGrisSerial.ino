#include <Arduino.h>

#define TRIG_PIN 12
#define RXD2 13
#define ON_OFF_PIN 14

void setup() {
  // Inicializar la comunicación serie para el monitor
  Serial.begin(115200);

  // Inicializar la comunicación serie para el sensor en el puerto serie 2
  Serial2.begin(9600, SERIAL_8N1, RXD2, -1);  // Solo RX, no TX

  // Configurar los pines
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ON_OFF_PIN, OUTPUT);

  // Encender el sensor
  digitalWrite(ON_OFF_PIN, HIGH);

  Serial.println("Sensor Ultrasónico Iniciado");
}

void loop() {
  // Enviar pulso de trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Verificar si hay datos disponibles en Serial2
  if (Serial2.available() > 0) {
    // Leer los datos enviados por el sensor
    String sensorData = Serial2.readStringUntil('\n');
    Serial.print("Datos recibidos del sensor: ");
    Serial.println(sensorData);

    // Separar los datos en distancia y temperatura
    int separatorIndex = sensorData.indexOf(',');
    if (separatorIndex != -1) {
      String distanceStr = sensorData.substring(0, separatorIndex);
      String temperatureStr = sensorData.substring(separatorIndex + 1);

      // Convertir los datos a valores numéricos
      float distance = distanceStr.toFloat();
      float temperature = temperatureStr.toFloat();

      // Imprimir los datos en el monitor serie
      Serial.print("Distancia: ");
      Serial.print(distance);
      Serial.println(" cm");
      Serial.print("Temperatura: ");
      Serial.print(temperature);
      Serial.println(" °C");
    } else {
      Serial.println("Error: Datos no válidos recibidos");
    }
  } else {
    Serial.println("Esperando datos del sensor...");
  }

  // Esperar un segundo antes de la siguiente lectura
  delay(1000);
}
