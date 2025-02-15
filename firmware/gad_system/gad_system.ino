#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <time.h>
#include "sntp.h"
#include "Firebase_Settings.h"
#include "Config_y_Sensores.h"
#include "fecha.h"
#include "valvula.h"

// Objeto FirebaseData global
FirebaseData firebaseData;
// Objeto JSON para enviar múltiples datos a la vez
FirebaseJson jsonData;
String pathMessage = "/mediciones/" + String(idCliente) + "/!mensaje";

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 17, 16);  // Configuración de Serial1 para sensor de UltraSonido

  Serial.print("Sensores listos. Conexión a base de datos..");
  delay(200);
  Serial.print(".");
  delay(250);
  Serial.println(".");


  pinMode(caudalimetro, INPUT);  // Set GPIO caudalimetro as input
  attachInterrupt(digitalPinToInterrupt(caudalimetro), pulse, RISING);
  aguaAcumulada = 0;
  pulseCount = 0;
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Ensure motor pins start LOW (no pulses)
  stopPins();
  delay(200);
  initFirebase();
  startNTP();
  verFecha();

  // Registrar la fecha de instalación (guardar el tiempo actual)
  fechaON = time(NULL);
  initReadFirebase();
  enviarDatosFirebase();
}

void loop() {
  // Obtener el tiempo actual
  currentMillisFirebase = millis();
  currentMillisPrint = millis();

  if (estado == "control") {
    abrirElectrovalvula(11, 0, 1);
    abrirElectrovalvula(12, 0, 1);
    abrirElectrovalvula(12, 15, 2);
    abrirElectrovalvula(13, 0, 2);
    abrirElectrovalvula(14, 0, 2);
    abrirElectrovalvula(15, 0, 2);
  }

  if (estado == "abrirEV") {
    abrirElectrovalvulaAhora();
  }

  if (estado == "cerrarEV") {
    cerrarElectrovalvulaAhora();
  }
  // Leer los sensores
  leerCaudalimetro();
  leerUltraSonido();

  // Control de impresión de datos
  if (currentMillisPrint - previousMillisPrint >= intervalPrint) {
    previousMillisPrint = currentMillisPrint;
    hacerCalculos();
    leerFirebase();
    printInfo();
    printLocalTime();
  }

  // Enviar datos a Firebase
  if (currentMillisFirebase - previousMillisFirebase >= intervalFirebase) {
    previousMillisFirebase = currentMillisFirebase;
    hacerCalculos();
    leerFirebase();
    enviarDatosFirebase();
  }

  // Configuración del tanque (solo si se recibe comando por serial)
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.equalsIgnoreCase("config")) {
      configureTank();
    }
  }
}


void enviarDatosFirebase() {
  // Convertir fechaON de formato UNIX a formato legible "DD-MM-YYYY HH:MM:SS"
  time_t fechaOnUnix = fechaON;
  struct tm *fechaOnInfo = localtime(&fechaOnUnix);
  char fechaOnLegible[20];
  strftime(fechaOnLegible, sizeof(fechaOnLegible), "%d-%m-%Y %H:%M:%S", fechaOnInfo);

  // Limpiar el objeto jsonData antes de rellenarlo
  jsonData.clear();

  // Variables fijas
  jsonData.set("/sistema/idCliente", idCliente);
  jsonData.set("/sistema/fechaON", String(fechaOnLegible));
  jsonData.set("/sistema/localizacion", localizacion);
  jsonData.set("/sistema/dispositivo", dispositivo);
  jsonData.set("/sistema/configuracion", configuracion);
  jsonData.set("/sistema/tipoTanque", tipoTanque);
  jsonData.set("/deposito/volumenTotal", String(volumenTotal, 2));
  jsonData.set("/deposito/reserva", String(reserva, 2));

  // Variables dinámicas
  jsonData.set("/deposito/volumenAguaDisponible", String(volumenAguaDisponible, 2));
  jsonData.set("/deposito/flechita", flechita);  // No es necesario redondear para strings como "flechita"
  jsonData.set("/deposito/porcentajeVolumenTotal", String(porcentajeVolumenTotal, 2));
  jsonData.set("/deposito/consumoDiarioPromedio", String(consumoDiarioPromedio, 2));
  jsonData.set("/deposito/aguaAcumuladaTotal", String(aguaAcumulada, 2));
  jsonData.set("/deposito/aguaAcumuladaPeriodo", String(aguaAcumuladaPeriodo, 2));
  jsonData.set("/deposito/estado", estado);

  // Obtener la hora actual en formato UNIX (segundos desde 1970) y la hora formateada
  time_t now = time(NULL);  // Obtener el tiempo actual en formato UNIX
  struct tm timeinfo;
  char dateStr[11];  // Buffer para almacenar la fecha (YYYY-MM-DD)
  char timeStr[9];   // Buffer para almacenar la hora (HH-MM-SS)

  if (getLocalTime(&timeinfo)) {
    strftime(dateStr, sizeof(dateStr), "%d-%m-%Y", &timeinfo);  // Obtener la fecha actual
    strftime(timeStr, sizeof(timeStr), "%H-%M-%S", &timeinfo);  // Obtener la hora actual
  }

  // Añadir la hora actual al JSON como timestamp (en formato legible y UNIX)
  jsonData.set("/timestamp/legible", String(dateStr) + " " + String(timeStr));  // Timestamp legible en formato "YYYY-MM-DD HH:MM:SS"

  // Crear una ruta única para cada set de datos, usando la fecha y hora actual
  String path = "/mediciones/" + String(idCliente) + "/" + String(dateStr) + "/" + String(timeStr);  // Carpeta del día y dataset por hora actual

  // Enviar el objeto JSON a Firebase
  if (Firebase.setJSON(firebaseData, path, jsonData)) {
    Serial.println("Datos enviados exitosamente a Firebase.");
  } else {
    Serial.print("Error al enviar los datos: ");
    Serial.println(firebaseData.errorReason());
  }
}

void configureTank() {
  Serial.println("Ingrese el libre del depósito en m:");
  while (Serial.available() == 0) {}
  libre = Serial.parseFloat();

  Serial.println("Ingrese el alto del depósito en m:");
  while (Serial.available() == 0) {}
  alto = Serial.parseFloat();

  Serial.println("Ingrese la reserva del depósito en m:");
  while (Serial.available() == 0) {}
  reserva = Serial.parseFloat();

  Serial.println("Ingrese el area del depósito en m2:");
  while (Serial.available() == 0) {}
  area = Serial.parseFloat();
}

void printInfo() {
  Serial.println();
  // Calcular cuántos días han pasado desde la instalación
  diasDesde(&dias, &horas, &minutos, &segundos);
  Serial.printf("Han pasado %lu días, %lu horas, %lu minutos, %lu segundos desde la instalación.\n", dias, horas, minutos, segundos);
  Serial.println();

  Serial.print("caudalAgua: ");
  Serial.print(caudalAgua);
  Serial.println(" L/min");

  Serial.print("aguaAcumulada: ");
  Serial.print(aguaAcumulada);
  Serial.println(" L");
  Serial.print("aguaAcumuladaPeriodo: ");
  Serial.print(aguaAcumuladaPeriodo);
  Serial.println(" L");
  Serial.println();

  Serial.print("volumenTotal: ");
  Serial.println(volumenTotal);
  Serial.print("volumenAdministrable: ");
  Serial.println(volumenAdministrable);
  Serial.print("porcentajeVolumenTotal: ");
  Serial.println(porcentajeVolumenTotal);
  Serial.print("volumenAguaDisponible: ");
  Serial.println(volumenAguaDisponible);
  Serial.println();

  // Diferencia entre el volumen actual y el volumen anterior
  float diferencia = abs(volumenAguaDisponible - volumenAguaDisponibleAnterior);

  // Lógica de la flechita según el cambio en el volumen
  if (volumenAguaDisponible > volumenAguaDisponibleAnterior) {
    flechita = "Arriba";
  } else if (diferencia <= margen) {  // generar un margen de seguridad para el estado estable
    flechita = "Estable";
  } else if (volumenAguaDisponible < volumenAguaDisponibleAnterior) {
    flechita = "Abajo";
  }
  // Imprimir estado de la flechita
  Serial.print("Flechita:");
  Serial.println(flechita);

  Serial.println();

  Serial.print("Distancia:");
  Serial.print(distancia);
  Serial.println(" Cm");
  Serial.print("Libre:");
  Serial.print(libre);
  Serial.println(" Cm");
  Serial.print("Alto:");
  Serial.print(alto);
  Serial.println(" Cm");
  Serial.print("Reserva:");
  Serial.print(reserva);
  Serial.println(" Cm");
  Serial.print("Area:");
  Serial.print(area);
  Serial.println(" Cm2");
  Serial.print("Cliente:");
  Serial.println(idCliente);
  Serial.print("mensaje:");
  Serial.println(mensaje);
  Serial.print("estado:");
  Serial.println(estado);
  Serial.println();

  // Actualizar volumenAguaDisponibleAnterior solo después de imprimir los datos y calcular la flechita
  volumenAguaDisponibleAnterior = volumenAguaDisponible;
  aguaAcumuladaAnterior = aguaAcumulada;
}

void leerUltraSonido() {
  const int paqueteSize = 4;               // Tamaño del paquete de datos
  unsigned long timeout = millis() + 200;  // Tiempo máximo para esperar datos (200 ms)

  // Limpiar el búfer de Serial1 antes de comenzar la lectura
  while (Serial1.available() > 0) {
    Serial1.read();
  }

  // Esperar a que llegue el byte inicial 0xFF
  while (millis() < timeout && Serial1.available() < 1) {
    // Esperar hasta que haya al menos 1 byte disponible
  }

  if (Serial1.available() > 0 && Serial1.peek() == 0xFF) {
    // Leer el paquete completo solo si hay suficientes datos
    while (Serial1.available() < paqueteSize) {
      if (millis() > timeout) {
        Serial.println("Timeout esperando el paquete completo");
        return;
      }
    }

    // Leer los 4 bytes del paquete
    for (int i = 0; i < paqueteSize; i++) {
      data[i] = Serial1.read();
    }

    // Depuración: Mostrar los datos recibidos
    //Serial.print("Datos recibidos: ");
    for (int i = 0; i < paqueteSize; i++) {
      //Serial.print(data[i], HEX);
      //Serial.print(" ");
    }
    //Serial.println();

    // Verificar el byte inicial y el checksum
    if (data[0] == 0xFF) {
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3]) {
        int distance = (data[1] << 8) + data[2];
        distancia = distance / 10.0;  // Convertir a cm

        // Mostrar la distancia para depuración
        //Serial.print("Distancia medida: ");
        //Serial.print(distancia);
        //Serial.println(" cm");
      } else {
        Serial.println("ERROR: Checksum incorrecto");
      }
    } else {
      Serial.println("ERROR: Byte inicial incorrecto");
    }
  } else {
    Serial.println("ERROR: No se encontró el byte inicial 0xFF");
  }
}

void leerCaudalimetro() {
  unsigned long currentMillis = millis();  // Obtener el tiempo actual
  // Calculate flow rate (L/min) based on the pulse count in the last second
  if (currentMillis - previousMillisCaudal >= 150) {  // Verificar si ha pasado el intervalo (150 ms)
    previousMillisCaudal = currentMillis;
    caudalAgua = (pulseCount / 75.0) * 60.0;  // 75 pulses = 1L, convert to liters per minute
    pulseCount = 0;                           // Reset the pulse count for the next interval
  }
}

void hacerCalculos() {
  // Variables FIJAS
  nivelMinimo = alto - reserva;
  volumenTotal = (alto - libre) * area;                    // en cm3. en el ejemplo volumenTotal=4.8m3
  volumenAdministrable = (alto - libre - reserva) * area;  // en cm3. en este ejemplo volumenAdministrable = 3.2m3
  aguaAcumuladaPeriodo = aguaAcumulada - aguaAcumuladaAnterior;
  // sensor de distancia

  // % de Agua en el tanque
  porcentajeVolumenTotal = (1 - ((distancia - libre) / (alto - libre))) * 100;
  volumenAguaDisponible = (porcentajeVolumenTotal * volumenTotal) / 100;

  // Calcular el tiempo transcurrido desde la instalación
  diasDesde(&dias, &horas, &minutos, &segundos);

  // Calcular el consumo diario promedio
  if (dias > 0) {  // Prevenir divisiones por 0
    consumoDiarioPromedio = aguaAcumulada / (dias + horas / 24.0 + minutos / 1440.0 + segundos / 86400.0);
  } else {
    consumoDiarioPromedio = 0;  // Si no han pasado días
  }
}

void initReadFirebase() {
  if (Firebase.getString(firebaseData, pathMessage)) {
    // Éxito: el nodo /mensaje existe y se ha obtenido un valor
    String mensajeLeido = firebaseData.to<String>();
    Serial.print("Nodo ya existe. Mensaje: ");
    Serial.println(mensajeLeido);
  } else {
    // Error al leer
    Serial.print("Error al leer string: ");
    Serial.println(firebaseData.errorReason());

    // Verificar si el error es "path not exist"
    if (firebaseData.errorReason() == "path not exist") {
      // (2) Crear el nodo SOLO una vez
      if (Firebase.setString(firebaseData, pathMessage, "Mensaje inicial")) {
        Serial.println("Nodo /mensaje creado con valor 'Mensaje inicial'.");
      } else {
        Serial.print("Error al crear nodo /mensaje: ");
        Serial.println(firebaseData.errorReason());
      }
    }
  }
}
void leerFirebase() {
  // ----------- LECTURA DEL STRING "/mensaje" DESDE FIREBASE -----------
  if (Firebase.getString(firebaseData, pathMessage)) {
    mensaje = firebaseData.to<String>();
    // Serial.print("String recibido desde ");
    // Serial.print(pathMessage);
    // Serial.print(": ");
    // Serial.println(mensaje);
    if (mensaje == "control") {
      estado = "control";
    } else if (mensaje == "lectura") {
      estado = "lectura";
    } else if (mensaje == "abrirEV") {
      estado = "abrirEV";
    } else if (mensaje == "cerrarEV") {
      estado = "cerrarEV";
    } else {
      estado = "Mensaje desconocido";
      Serial.println("Mensaje desconocido");
    }
  } else {
    Serial.print("Error al leer string desde ");
    Serial.print(pathMessage);
    Serial.print(": ");
    Serial.println(firebaseData.errorReason());
  }
}
