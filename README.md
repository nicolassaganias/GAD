# GAD
# Plataforma Web para Gestión Inteligente de Tanques de Agua

Esta plataforma web se comunica con dispositivos basados en **ESP32** a través de WiFi, permitiendo una gestión inteligente de tanques de agua residenciales. Los dispositivos envían datos en tiempo real a un sistema central, que controla el llenado del tanque abriendo o cerrando el suministro de agua según sea necesario.

## Características del Dispositivo

Cada dispositivo incluye:
- **Sensor ultrasónico de distancia**: Mide el nivel de agua en el tanque.
- **Medidor de caudal**: Registra el volumen de agua que entra al tanque.

Además, los dispositivos almacenan:
- Un **ID único**.
- **Dimensiones del tanque** y otros datos fijos relevantes.

## Interfaz Web

La interfaz consta de tres pantallas principales y utiliza **Firebase Realtime Database** para mostrar información en tiempo real.

### 1. Selección de Dispositivo
En esta pantalla, el usuario selecciona el **ID** del dispositivo que desea monitorear.

### 2. Vista General del Dispositivo
En esta pantalla, se muestra:
- **Información detallada** del dispositivo seleccionado.
- **Capacidad actual del tanque** y el **volumen de agua**.

También incluye un botón para alternar entre dos modos:
- **Modo en espera (Standby)**: El dispositivo recopila datos, pero no interviene en el sistema de agua.
- **Modo de control**: El dispositivo gestiona activamente el flujo de agua al tanque.

### 3. Panel de Análisis
Esta pantalla proporciona dos gráficos interactivos:
- **Nivel de agua en el tanque**: Muestra datos en tiempo real e históricos.
- **Ingreso de agua**: Indica el volumen de agua que entra al tanque, con opciones para visualizar datos de días, semanas u otros períodos específicos.

## Beneficios
- **Gestión eficiente del agua**: Automatización del llenado de tanques.
- **Datos en tiempo real**: Monitoreo constante con información actualizada.
- **Historial de datos**: Visualización de patrones de uso y análisis histórico.

Esta plataforma combina automatización y análisis en tiempo real para ofrecer una solución completa y accesible para la gestión de tanques de agua residenciales.
