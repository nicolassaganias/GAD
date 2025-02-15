// Pines de control
const int IN1 = 26;
const int IN2 = 27;
const int ENA = 12; // Opcional, si tu módulo L298N requiere habilitación externa.

// Duración del pulso en milisegundos
const int pulsoTiempo = 1000; // 1 segundo, ajustar según necesidad

void setup() {
  // Configuración de pines
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Activamos el canal
  digitalWrite(ENA, HIGH);

  // Inicialmente, sin aplicar tensión
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  Serial.begin(115200);
  Serial.println("Sistema iniciado.");
}

void loop() {
  // Ejemplo: Generar un pulso positivo (abrir la válvula)
  Serial.println("Pulso positivo...");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  delay(pulsoTiempo); // Mantenemos el pulso el tiempo necesario
  // Parar
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(2000); // Esperamos 2 segundos antes del siguiente pulso

  // Ejemplo: Generar un pulso negativo (cerrar la válvula)
  Serial.println("Pulso negativo...");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  delay(pulsoTiempo);
  // Parar
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(2000); // Esperamos 2 segundos antes de repetir
}
