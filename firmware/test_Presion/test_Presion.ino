/***********************************************************
      DFRobot Gravity: Analog Current to Voltage Converter (For 4~20mA Application)
      SKU:SEN0262

      GNU Lesser General Public License.
      See <http://www.gnu.org/licenses/> for details.
      All above must be included in any redistribution
     ****************************************************/

#define ANALOG_PIN 27
#define RANGE 5000 // Depth measuring range 5000mm (for water)
#define VREF 3300 // ADC's reference voltage on your ESP32, typically 3300mV
#define CURRENT_INIT 3.15 // Adjust this to the current reading at 0mm depth (unit: mA)
#define DENSITY_WATER 1 // Pure water density normalized to 1
#define PRINT_INTERVAL 1000
#define BUFFER_SIZE 20 // Define the size of the buffer

float depthBuffer[BUFFER_SIZE]; // Array to store depth readings
int bufferIndex = 0; // Current index in the buffer
bool bufferFilled = false; // Flag to check if buffer is filled

float dataVoltage;
float dataCurrent, depth; // unit: mA
unsigned long timepoint_measure;

void setup()
{
  Serial.begin(115200);
  pinMode(ANALOG_PIN, INPUT);
  timepoint_measure = millis();

  // Initialize depth buffer with zeros
  for (int i = 0; i < BUFFER_SIZE; i++) {
    depthBuffer[i] = 0.0;
  }
}

void loop()
{
  if (millis() - timepoint_measure > PRINT_INTERVAL) {
    timepoint_measure = millis();

    // Read the analog value and calculate depth
    dataVoltage = analogRead(ANALOG_PIN) / 4096.0 * VREF;
    dataCurrent = dataVoltage / 120.0; // Sense Resistor: 120 ohm
    depth = (dataCurrent - CURRENT_INIT) * (RANGE / DENSITY_WATER / 16.0); // Calculate depth from current readings

    if (depth < 0) {
      depth = 0.0;
    }

    // Store the depth reading in the buffer
    depthBuffer[bufferIndex] = depth;
    bufferIndex++;

    // Check if the buffer is filled
    if (bufferIndex >= BUFFER_SIZE) {
      bufferIndex = 0; // Reset buffer index
      bufferFilled = true; // Set buffer filled flag
    }

    // Calculate the average depth if the buffer is filled
    float averageDepth = 0.0;
    if (bufferFilled) {
      for (int i = 0; i < BUFFER_SIZE; i++) {
        averageDepth += depthBuffer[i];
      }
      averageDepth /= BUFFER_SIZE;
    } else {
      for (int i = 0; i < bufferIndex; i++) {
        averageDepth += depthBuffer[i];
      }
      averageDepth /= bufferIndex;
    }

    // Serial print intermediate values and results
    Serial.print("Data Voltage: ");
    Serial.print(dataVoltage);
    Serial.print(" mV, Data Current: ");
    Serial.print(dataCurrent);
    Serial.print(" mA, Depth: ");
    Serial.print(depth);
    Serial.print(" mm, Average Depth: ");
    Serial.print(averageDepth);
    Serial.println(" mm");
  }
}
