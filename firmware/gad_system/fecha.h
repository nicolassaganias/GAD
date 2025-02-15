const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)
time_t fechaON; // Variable que contiene la fecha de instalación (en tiempo UNIX, segundos desde 1 de enero de 1970)
unsigned long dias, horas, minutos, segundos;

void verFecha() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  verFecha();
}

// Función para calcular la diferencia en días, horas, minutos y segundos desde `fechaON`
void diasDesde(unsigned long* dias, unsigned long* horas, unsigned long* minutos, unsigned long* segundos) {
  time_t ahora = time(NULL);  // Obtener la fecha/hora actual en formato UNIX
  unsigned long diferencia = ahora - fechaON;  // Diferencia en segundos desde la instalación

  *dias = diferencia / 86400;            // 1 día = 86400 segundos
  *horas = (diferencia % 86400) / 3600;  // 1 hora = 3600 segundos
  *minutos = (diferencia % 3600) / 60;   // 1 minuto = 60 segundos
  *segundos = diferencia % 60;           // Segundos restantes
}

void startNTP() {
  sntp_set_time_sync_notification_cb(timeavailable);

  // Enable DHCP mode to get the NTP server address automatically (optional)
  sntp_servermode_dhcp(1);

  // Set timezone with automatic daylight saving adjustment for Central European Time
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", ntpServer1, ntpServer2);

  Serial.println("Synchronizing time with NTP...");
}
