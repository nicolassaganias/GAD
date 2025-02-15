enum ValveState {
  WAITING_FOR_TIME,  // Haven't reached the target hour:minute yet
  OPEN_PULSING,      // Sending a positive pulse
  VALVE_OPEN,        // Valve is considered open (waiting for X minutes)
  CLOSE_PULSING,     // Sending a negative pulse
  VALVE_CLOSED       // Valve is fully closed
};

ValveState valveState = WAITING_FOR_TIME;

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

void abrirElectrovalvula(int horaObjetivo, int minutoObjetivo, int intervaloMinutos) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get local time (valve function).");
        return;
    }

    valveOpenIntervalMs = (unsigned long)intervaloMinutos * 60UL * 1000UL;
    unsigned long nowMillis = millis();

    //Serial.printf("Hora actual: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
    //Serial.printf("Objetivo: %02d:%02d\n", horaObjetivo, minutoObjetivo);
    //Serial.printf("Estado actual: %d\n", valveState);

    switch (valveState) {
        case WAITING_FOR_TIME:
            if ((timeinfo.tm_hour == horaObjetivo) && (timeinfo.tm_min == minutoObjetivo)) {
                digitalWrite(IN1, LOW);
                digitalWrite(IN2, HIGH);
                Serial.println("== Positive pulse START ==");
                pulseStartMillis = nowMillis;
                valveState = OPEN_PULSING;
            }
            break;

        case OPEN_PULSING:
            if (nowMillis - pulseStartMillis >= pulsoTiempo) {
                stopPins();
                Serial.println("== Positive pulse END. Valve is OPEN ==");
                valveState = VALVE_OPEN;
                valveOpenStartMillis = nowMillis;
            }
            break;

        case VALVE_OPEN:
            if (nowMillis - valveOpenStartMillis >= valveOpenIntervalMs) {
                digitalWrite(IN1, HIGH);
                digitalWrite(IN2, LOW);
                Serial.println("== Negative pulse START ==");
                pulseStartMillis = nowMillis;
                valveState = CLOSE_PULSING;
            }
            break;

        case CLOSE_PULSING:
            if (nowMillis - pulseStartMillis >= pulsoTiempo) {
                stopPins();
                Serial.println("== Negative pulse END. Valve is CLOSED ==");
                valveState = WAITING_FOR_TIME;  // Regresar al estado inicial para seguir escuchando
            }
            break;
    }
}

void abrirElectrovalvulaAhora() {
  // Inmediatamente mandamos pulso de abrir
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  pulseStartMillis = millis();
  valveState = OPEN_PULSING;
  Serial.println("== Positive pulse START. Valve is OPENING ==");
}

void cerrarElectrovalvulaAhora() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  pulseStartMillis = millis();
  valveState = CLOSE_PULSING;
  Serial.println("== Negative pulse START. Valve is CLOSING ==");
}
