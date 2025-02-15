/************************************************************************** 
 * Example ESP32 Code:
 *  - Connects to WiFi
 *  - Synchronizes time via NTP
 *  - Implements electrovalve control using the original state machine
 *  - Prints local time every second
 **************************************************************************/
// OOJJOOOOO QUE TIENE HASTRIGGERED_TODAY

#include <WiFi.h>
#include <time.h>
#include <Arduino.h>

// -------------------------------
// WiFi Configuration
// -------------------------------
const char* ssid = "Can_Saguer_Wifi";  // <-- REPLACE with your network SSID
const char* password = "cansaguer2";   // <-- REPLACE with your network Password

// -------------------------------
// NTP Time Configuration
// -------------------------------
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // Change according to your timezone (e.g. -18000 for EST)
const int daylightOffset_sec = 3600;  // Set this if you have DST in your location

// -------------------------------
// Valve Pin Assignments
// -------------------------------
const int IN1 = 25;
const int IN2 = 26;

// -------------------------------
// Pulse/Timing Configuration
// -------------------------------
const unsigned long pulsoTiempo = 100;  // ms for the short open/close pulse

// -------------------------------
// State Machine
// -------------------------------
enum ValveState {
  WAITING_FOR_TIME,  // Haven't reached the target hour:minute yet
  OPEN_PULSING,      // Sending a positive pulse
  VALVE_OPEN,        // Valve is considered open (waiting for X minutes)
  CLOSE_PULSING,     // Sending a negative pulse
  VALVE_CLOSED       // Valve is fully closed
};

ValveState valveState = WAITING_FOR_TIME;

// For controlling one-shot per day
bool hasTriggeredToday = false;

// -------------------------------
// Timing Variables
// -------------------------------
unsigned long pulseStartMillis = 0;      // When the current pulse started
unsigned long valveOpenStartMillis = 0;  // When the valve finished opening
unsigned long valveOpenIntervalMs = 0;   // X minutes in ms (passed to function)

// We'll print the time once every second
unsigned long lastTimePrint = 0;
const unsigned long timePrintInterval = 1000;

// -------------------------------
// Helpers
// -------------------------------

// Set both IN1 and IN2 LOW (stop signals)
void stopPins() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// Print local time if available
void printLocalTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.printf("Current time: %02d:%02d:%02d\n",
                  timeinfo.tm_hour,
                  timeinfo.tm_min,
                  timeinfo.tm_sec);
  } else {
    Serial.println("Failed to get local time (during print).");
  }
}

// -------------------------------
// Main valve control function
// -------------------------------
void abrirElectrovalvula(int horaObjetivo, int minutoObjetivo, int intervaloMinutos) {
  // 1) Get the current time from NTP / RTC
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // If we fail to get time, do nothing
    Serial.println("Failed to get local time (valve function).");
    return;
  }

  // 2) Convert the user’s "intervaloMinutos" to milliseconds
  valveOpenIntervalMs = (unsigned long)intervaloMinutos * 60UL * 1000UL;

  unsigned long nowMillis = millis();

  // 3) State machine
  switch (valveState) {

    case WAITING_FOR_TIME:
      // Check if hour:minute is reached AND we haven't triggered yet today
      if (!hasTriggeredToday && (timeinfo.tm_hour == horaObjetivo) && (timeinfo.tm_min == minutoObjetivo)) {
        // Send a positive pulse to open
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);

        Serial.println("== Positive pulse START ==");
        pulseStartMillis = nowMillis;
        valveState = OPEN_PULSING;

        // Mark as triggered so we don't open multiple times in the same day
        hasTriggeredToday = true;
      }
      break;

    case OPEN_PULSING:
      // Keep pulsing until pulsoTiempo has elapsed
      if (nowMillis - pulseStartMillis >= pulsoTiempo) {
        stopPins();
        Serial.println("== Positive pulse END. Valve is OPEN ==");
        valveState = VALVE_OPEN;

        // Mark time when the valve finished opening
        valveOpenStartMillis = nowMillis;
      }
      break;

    case VALVE_OPEN:
      // Wait for the user-defined interval (X minutes)
      if (nowMillis - valveOpenStartMillis >= valveOpenIntervalMs) {
        // Now time to send a negative pulse to close
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);

        Serial.println("== Negative pulse START ==");
        pulseStartMillis = nowMillis;
        valveState = CLOSE_PULSING;
      }
      break;

    case CLOSE_PULSING:
      // Keep pulsing negative until pulsoTiempo has elapsed
      if (nowMillis - pulseStartMillis >= pulsoTiempo) {
        stopPins();
        Serial.println("== Negative pulse END. Valve is CLOSED ==");
        valveState = VALVE_CLOSED;
      }
      break;

    case VALVE_CLOSED:
      // Once closed, we do nothing for the rest of the day
      break;
  }

  // 4) Reset the daily trigger if it's midnight (00:00) to allow a new trigger tomorrow
  //    (Optionally check seconds so you don't reset multiple times in that minute)
  if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
    hasTriggeredToday = false;
    // If you want it to open again tomorrow, you might do:
    // valveState = WAITING_FOR_TIME;
  }
}

// -------------------------------
// SETUP
// -------------------------------
void setup() {
  Serial.begin(115200);
  delay(100);

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait for time to be set
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP time sync...");
    delay(2000);
  }
  Serial.println("Time initialized!");

  // Pin configuration
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Ensure motor pins start LOW (no pulses)
  stopPins();

  Serial.println("Setup complete. Waiting for time to open valve...");
}

// -------------------------------
// LOOP
// -------------------------------
void loop() {
  // Example usage:
  //   Open valve at 15:45 for 10 minutes
  abrirElectrovalvula(18, 0, 3);

  // Print the time once every second
  if (millis() - lastTimePrint >= timePrintInterval) {
    lastTimePrint = millis();
    printLocalTime();
  }
  // Small delay so we don't spin the loop too fast
  delay(100);
}
