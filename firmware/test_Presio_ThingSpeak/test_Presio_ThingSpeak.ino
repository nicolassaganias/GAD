#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "Can_Saguer_Wifi_EXT";
const char* password = "cansaguer2";

// ThingSpeak server information
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "4PPP0U5U780922W9";  // Replace with your ThingSpeak API key

#define ANALOG_PIN 32
#define RANGE 500          // Depth measuring range 5000mm (for water)
#define VREF 3300          // ADC's reference voltage on your ESP32, typically 3300mV
#define CURRENT_INIT 3.15  // Adjust this to the current reading at 0mm depth (unit: mA)
#define DENSITY_WATER 1    // Pure water density normalized to 1
#define PRINT_INTERVAL 1000
#define SEND_INTERVAL 60000

#define BUFFER_SIZE 20  // Define the size of the buffer

float depthBuffer[BUFFER_SIZE];  // Array to store depth readings
int bufferIndex = 0;             // Current index in the buffer
bool bufferFilled = false;       // Flag to check if buffer is filled

float dataVoltage;
float dataCurrent, depth;  // unit: mA
unsigned long timepoint_measure, timepoint_measure2;

void setup() {
  Serial.begin(115200);
  pinMode(ANALOG_PIN, INPUT);
  analogReadResolution(12);  // Set ADC resolution
  timepoint_measure = millis();

  // Initialize depth buffer with zeros
  for (int i = 0; i < BUFFER_SIZE; i++) {
    depthBuffer[i] = 0.0;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Add a delay to stabilize ADC readings
  delay(1000);
}

void loop() {
  if (millis() - timepoint_measure > PRINT_INTERVAL) {
    timepoint_measure = millis();

    // Read the analog value and calculate depth
    int rawAnalogValue = analogRead(ANALOG_PIN);
    dataVoltage = rawAnalogValue / 4096.0 * VREF;
    dataCurrent = dataVoltage / 120.0;                                           // Sense Resistor: 120 ohm
    depth = (dataCurrent - CURRENT_INIT) * (RANGE / DENSITY_WATER / 16.0)+1;  // Calculate depth from current readings

    if (depth < 0) {
      depth = 0.0;
    }

    // Store the depth reading in the buffer
    depthBuffer[bufferIndex] = depth;
    bufferIndex++;

    // Check if the buffer is filled
    if (bufferIndex >= BUFFER_SIZE) {
      bufferIndex = 0;      // Reset buffer index
      bufferFilled = true;  // Set buffer filled flag
    }

    // Calculate the average depth if the buffer is filled
    float averageDepth = 0.0;
    if (bufferFilled) {
      for (int i = 0; i < BUFFER_SIZE; i++) {
        averageDepth += depthBuffer[i];
      }
      averageDepth /= BUFFER_SIZE;
    } else {
      for (int i = 0; i < bufferIndex; i++) {
        averageDepth += depthBuffer[i];
      }
      averageDepth /= bufferIndex;
    }

    int litrosTotal = (averageDepth * 500) / 45;

    // Serial print intermediate values and results
    Serial.print("Depth: ");
    Serial.println(depth);
    Serial.print("Average Depth: ");
    Serial.println(averageDepth);
    Serial.print("Litros total: ");
    Serial.println(litrosTotal);

    if (millis() - timepoint_measure2 > SEND_INTERVAL) {
      timepoint_measure2 = millis();
      sendDataToThingSpeak(averageDepth);
    }
  }
}

void sendDataToThingSpeak(float depthValue) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = String(serverName) + "?api_key=" + apiKey + "&field1=" + String(depthValue);
    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}