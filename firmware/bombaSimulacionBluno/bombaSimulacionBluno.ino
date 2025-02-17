#define BUTTON_PIN A0
#define RELAY_PIN 7

bool relayState = false;
bool lastButtonState = HIGH;

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Relé inicialmente apagado
}

void loop() {
    bool currentButtonState = digitalRead(BUTTON_PIN);
    
    if (currentButtonState == LOW && lastButtonState == HIGH) { // Detecta flanco de bajada
        relayState = !relayState; // Alterna el estado del relé
        digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
        delay(50); // Pequeño debounce por software
    }
    
    lastButtonState = currentButtonState;
}
