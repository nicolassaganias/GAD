volatile double waterFlow;       // Total amount of water passed through the flowmeter
volatile int pulseCount;         // Number of pulses counted within the time interval
const int caudalimetro = 14;
const int valvulaRelay = 25;
double flowRate;                 // Flow rate in liters per minute (L/min)
unsigned long previousMillis = 0;  // Store the last time the flow rate was calculated
const unsigned long interval = 1000;  // Time interval for calculating flow rate (1 second)

// Interrupt function to count pulses
void IRAM_ATTR pulse() {
  pulseCount++;
  waterFlow += 1.0 / 75.0;  // 75 pulses = 1L (refer to product specification)
}

void setup() {
  Serial.begin(9600);  // Set baud rate for serial communication
  waterFlow = 0;
  pulseCount = 0;
  pinMode(valvulaRelay, OUTPUT);
  pinMode(caudalimetro, INPUT);  // Set GPIO caudalimetro as input
  attachInterrupt(digitalPinToInterrupt(caudalimetro), pulse, RISING);  // Attach interrupt to GPIO caudalimetro
}

void loop() {
  unsigned long currentMillis = millis();

  // Calculate flow rate every 1 second
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Calculate flow rate (L/min) based on the pulse count in the last second
    flowRate = (pulseCount / 75.0) * 60.0;  // 75 pulses = 1L, convert to liters per minute

    // Reset the pulse count for the next interval
    pulseCount = 0;

    // Print the real-time flow rate
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.println(" L/min");

    // Print the total water passed through the flowmeter
    Serial.print("Total water flow: ");
    Serial.print(waterFlow);
    Serial.println(" L");
  }



  // Control the relay based on total water flow
  if (waterFlow >= 1) {
    digitalWrite(valvulaRelay, HIGH);
  } else {
    digitalWrite(valvulaRelay, LOW);
  }

  //delay(500);  // Delay for 500ms to slow down serial output (not related to flow rate calculation)
}
