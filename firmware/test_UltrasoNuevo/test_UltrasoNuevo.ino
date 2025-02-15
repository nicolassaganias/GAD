/*
   @File  : DFRobot_Distance_A02.ino
   @Brief : This example use A02YYUW ultrasonic sensor to measure distance
            With initialization completed, We can get distance value
   @Copyright [DFRobot](https://www.dfrobot.com),2016
              GUN Lesser General Pulic License
   @version V1.0
   @data  2019-8-28
*/


unsigned char data[4] = {};
float distance;
unsigned long previousMillis = 0;   // Stores the last time the loop ran
const long interval = 100;          // Interval in milliseconds (100ms)

void setup() {
  Serial.begin(57600);
  Serial1.begin(9600, SERIAL_8N1, 17, 16);
}

void loop() {
  unsigned long currentMillis = millis();  // Get the current time

  // Only run the code block every 'interval' milliseconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update the last run time

    // Start reading data
    do {
      for (int i = 0; i < 4; i++) {
        data[i] = Serial1.read();   // Read the data from Serial1
      }
    } while (Serial1.read() == 0xff);  // Loop until a non-0xFF byte is received

    Serial1.flush();  // Flush the Serial1 buffer

    // Check if the first byte is 0xFF and calculate checksum
    if (data[0] == 0xff) {
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3]) {
        // Calculate the distance
        distance = (data[1] << 8) + data[2];
        if (distance > 30) {
          Serial.print("distance=");
          Serial.print(distance / 10);  // Convert distance to cm
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
