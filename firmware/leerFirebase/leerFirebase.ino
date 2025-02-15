#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Proveer la información del proceso de generación de token.
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "Can_Saguer_Wifi"
#define WIFI_PASSWORD "cansaguer2"

/* Define la clave API */
#define API_KEY "AIzaSyBSa4fX_-xz3KyVC_knJL4e9VIupRunfII"

/* Define la URL de la base de datos */
#define DATABASE_URL "https://sistemagad-8bc66-default-rtdb.europe-west1.firebasedatabase.app/"

/* Define el correo electrónico y la contraseña del usuario */
#define USER_EMAIL "nicosaga@gmail.com"
#define USER_PASSWORD "xa18xa18"

// Define objeto Firebase Data
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

void setup()
{
  Serial.begin(115200);

  // Conexión Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Conectado con IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // Configurar claves de Firebase
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  // Asignar función de callback para el estado del token
  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectNetwork(true);

  // Iniciar Firebase
  Firebase.begin(&config, &auth);
}

void loop()
{

    // Leer un string desde el nodo /mensaje
    if (Firebase.getString(fbdo, "/mensaje"))
    {
      String mensaje = fbdo.to<String>();
      Serial.print("String recibido desde /mensaje: ");
      Serial.println(mensaje);
    }
    else
    {
      Serial.print("Error al leer string desde /mensaje: ");
      Serial.println(fbdo.errorReason());
    }

    // Esperar un tiempo antes de volver a intentar
    delay(5000);
  }
}
