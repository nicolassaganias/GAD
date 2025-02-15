#include <Arduino.h>
#include <SoftwareSerial.h>
#include <FirebaseESP32.h>
#include <WiFi.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Can_Saguer_WIFI"
#define WIFI_PASSWORD "cansaguer2"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyBSa4fX_-xz3KyVC_knJL4e9VIupRunfII"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://sistemagad-8bc66-default-rtdb.europe-west1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "nicosaga@gmail.com"
#define USER_PASSWORD "xa18xa18"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;

//SENSOR Ultrasonido
SoftwareSerial mySerial(12, 13);  // RX, TX
unsigned char data[4] = {};
float distance;

//Caudalimetro
volatile double waterFlow;
const int caudalimetro = 14;

//Relay Simple
const int valvulaRelay = 25;

// Variables para dimensiones del depósito
float alto = 100.0;     // Alto predeterminado en cm
float ancho = 50.0;     // Ancho predeterminado en cm
float profundo = 50.0;  // Profundo predeterminado en cm
float libre = 0;
float litrosDepositoTotal = 0;

// Variables de control de tiempo
unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de 1 segundo

FirebaseData firebaseData;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  waterFlow = 0;
  pinMode(valvulaRelay, OUTPUT);
  pinMode(caudalimetro, INPUT);                                         // Set GPIO caudalimetro as input
  attachInterrupt(digitalPinToInterrupt(caudalimetro), pulse, RISING);  // Attach interrupt to GPIO caudalimetr

  // Conectar a Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a Wi-Fi");

  // Conectar a Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Calcular el volumen total del depósito en litros
  litrosDepositoTotal = (alto * ancho * profundo) / 1000.0;  // Convertir a litros (cm³ a litros)
  Serial.print("El volumen total del depósito es: ");
  Serial.print(litrosDepositoTotal);
  Serial.println(" litros.");

  Serial.println("Sistema listo para medir la distancia.");
}

void loop() {

  // Verifica si hay un comando de configuración en el Serial
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remover espacios y saltos de línea

    if (input.equalsIgnoreCase("config")) {
      Serial.println("Ingrese el alto del depósito en cm:");
      while (Serial.available() == 0) {}
      alto = Serial.parseFloat();

      Serial.println("Ingrese el ancho del depósito en cm:");
      while (Serial.available() == 0) {}
      ancho = Serial.parseFloat();

      Serial.println("Ingrese el profundo del depósito en cm:");
      while (Serial.available() == 0) {}
      profundo = Serial.parseFloat();

      // Recalcular el volumen total del depósito
      litrosDepositoTotal = (alto * ancho * profundo) / 1000.0;  // Convertir a litros (cm³ a litros)
      Serial.print("Nuevo volumen total del depósito es: ");
      Serial.print(litrosDepositoTotal);
      Serial.println(" litros.");
    }
  }

  // Obtener el tiempo actual
  unsigned long currentMillis = millis();

  // Verificar si ha pasado 1 segundo
  if (currentMillis - previousMillis >= interval) {  // tomar datos y enviar dataos a Firebase
    previousMillis = currentMillis;

    if (mySerial.available() >= 4) {
      for (int i = 0; i < 4; i++) {
        data[i] = mySerial.read();
      }

      if (data[0] == 0xff) {
        int sum = (data[0] + data[1] + data[2]) & 0x00FF;
        if (sum == data[3]) {
          distance = (data[1] << 8) + data[2];

          leerCaudalimetro();

          if (distance > 280) {
            Serial.print("Distancia medida: ");
            Serial.print(distance / 10);
            Serial.println(" cm");

            // Calcular la altura del agua en el depósito
            libre = alto - (distance / 10);  // Asumiendo que la distancia es desde la parte superior del depósito

            // Calcular el volumen de agua en el depósito
            float litrosActuales = (libre * ancho * profundo) / 1000.0;  // Convertir a litros
            Serial.print("Volumen actual de agua: ");
            Serial.print(litrosActuales);
            Serial.println(" litros.");

            // Enviar datos a Firebase
            enviarDatosFirebase(litrosDepositoTotal, libre);
          } else {
            Serial.println("Por debajo del límite inferior.");
          }
        } else {
          Serial.println("ERROR: Suma de verificación incorrecta.");
        }
      }
    }
  }
}

// Función para enviar datos a Firebase
void enviarDatosFirebase(float totalLitros, float libreLitros, double waterFlow) {
  // Envía la variable litrosDepositoTotal a Firebase
  if (Firebase.setFloat(firebaseData, "/deposito/totalLitros", totalLitros)) {
    Serial.println("Datos de totalLitros enviados exitosamente.");
  } else {
    Serial.print("Error al enviar totalLitros: ");
    Serial.println(firebaseData.errorReason());
  }

  // Envía la variable waterFlow a Firebase
  if (Firebase.setFloat(firebaseData, "/caudalimetro/waterFlow", waterFlow)) {
    Serial.println("Datos de waterFlow enviados exitosamente.");
  } else {
    Serial.print("Error al enviar waterFlow: ");
    Serial.println(firebaseData.errorReason());
  }
  // Envía la variable libre a Firebase
  if (Firebase.setFloat(firebaseData, "/deposito/libreLitros", libreLitros)) {
    Serial.println("Datos de libreLitros enviados exitosamente.");
  } else {
    Serial.print("Error al enviar libreLitros: ");
    Serial.println(firebaseData.errorReason());
  }
}

void leerCaudalimetro() {

  Serial.print("waterFlow:");
  Serial.print(waterFlow);
  Serial.println("   L");

  if (waterFlow >= 1) {
    digitalWrite(valvulaRelay, HIGH);
  } else {
    digitalWrite(valvulaRelay, LOW);
  }
}
