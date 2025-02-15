/*
       @File  : DFRobot_Distance_A01.ino
       @Brief : This example use A01NYUB ultrasonic sensor to measure distance
                With initialization completed, We can get distance value
       @Copyright [DFRobot](https://www.dfrobot.com),2016
                  GUN Lesser General Pulic License
       @version V1.0
       @data  2019-8-28
*/

unsigned char data[4] = {};
float distance;
unsigned long previousMillis = 0;  // Variable para almacenar el último tiempo en el que se leyó el sensor
const long interval = 150;         // Intervalo de tiempo en milisegundos

void setup()
{
  Serial.begin(57600);                   // Comunicación Serial para depuración
  Serial1.begin(9600, SERIAL_8N1, 17, 16); // Configuración de Serial1 (UART1) en los pines 16 (RX) y 17 (TX)
}

void loop()
{
  unsigned long currentMillis = millis();  // Obtener el tiempo actual

  // Verificar si ha pasado el intervalo (150 ms)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Guardar el tiempo actual como el último tiempo leído

    // Se lee un paquete de 4 bytes desde Serial1
    do {
      for (int i = 0; i < 4; i++) {
        data[i] = Serial1.read();
      }
    } while (Serial1.read() == 0xff);

    // Se vacía el buffer de Serial1
    Serial1.flush();

    // Verificar si el primer byte es 0xff
    if (data[0] == 0xff) {
      // Calcular la suma de verificación
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;

      // Comprobar si la suma de verificación es correcta
      if (sum == data[3]) {
        // Calcular la distancia
        distance = (data[1] << 8) + data[2];
        distance = distance / 10; // distancia en cm
        
        // Si la distancia es mayor que 20 cm, mostrarla
        if (distance> 20) {
          Serial.print("distance=");
          Serial.print(distance);
          Serial.println("cm");
        } else {
          Serial.println("Below the lower limit");
        }
      } else {
        Serial.println("ERROR");
      }
    }
  }
}
