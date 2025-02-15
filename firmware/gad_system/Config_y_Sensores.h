//Caudalimetro
volatile double aguaAcumulada;
volatile double aguaAcumuladaAnterior, aguaAcumuladaPeriodo = 0;  // Total de agua acumulada (L)
const int caudalimetro = 14;                                      // Pin Caudalimetro
volatile double caudalAgua;                                       // Caudal de agua por minuto (L/min)

// Valve Pin Assignments
const int IN1 = 25;
const int IN2 = 26;
const unsigned long pulsoTiempo = 100;  // ms for the short open/close pulse
// Definición del cliente
#define CLIENTE 23

#if CLIENTE == 23

String idCliente = "00023";
String tipoTanque = "CASA";
String configuracion = "II";
String localizacion = "-34.607283, -58.516216";
String dispositivo = "11-E-254";

// Variables para dimensiones del depósito
float alto = 70.0;     // Alto predeterminado en cm
float area = 28600.0;   // Área predeterminada en cm2 (ancho * alto)
float libre = 5.0;     // Espacio libre en cm
float reserva = 5.0;   // Reserva en cm
float nivelMinimo = alto - reserva;

#elif CLIENTE == 24

String idCliente = "00024";
String tipoTanque = "RESERVA";
String configuracion = "I";
String localizacion = "-34.607102, -58.516208";
String dispositivo = "11-E-255";

// Variables para dimensiones del depósito
float alto = 70.0;     // Alto predeterminado en cm
float area = 28600.0;   // Área predeterminada en cm2 (ancho * alto)
float libre = 5.0;     // Espacio libre en cm
float reserva = 5.0;   // Reserva en cm
float nivelMinimo = alto - reserva;

#else
#error "Cliente no definido o no soportado."
#endif

String mensaje;  // para leer desde FB
String estado;

// Variables de control de tiempo
unsigned long previousMillisCaudal, previousMillisPrint, previousMillisFirebase = 0;
unsigned long currentMillisFirebase, currentMillisPrint;
const long intervalFirebase = 300000;  // Intervalo de 5 minutos
const long intervalPrint = 5000;       // Intervalo de 1 segundo

// Variable to count the number of pulses
volatile int pulseCount = 0;

float margen = 5;                                                                                                                                                        // rango para que volumenAguaDisponible == volumenAguaDisponibleAnterior funcione. ver su funcionamiento, está solo esbozada la idea
double volumenTotal, volumenAdministrable, porcentajeVolumenTotal, consumoDiarioPromedio, volumenAguaDisponible, volumenAguaDisponibleAnterior, acumumuladoMedioCaudal;  // ver si double o float
unsigned long diasDesdeON;
String flechita, alert;

//SENSOR Ultrasonido
unsigned char data[4] = {};
float distance, distancia;
unsigned long previousMillis, currentMillis = 0;  // Stores the last time the loop ran

// This function will be called every time a pulse is detected
void IRAM_ATTR pulse() {
  pulseCount++;
  aguaAcumulada += 1.0 / 75.0;  // 75 pulses = 1L (refer to product specification)
}
