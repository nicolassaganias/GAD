/*
       @File  : DFRobot_Distance_A01.ino
       @Brief : This example use A01NYUB ultrasonic sensor to measure distance
                With initialization completed, We can get distance value
       @Copyright [DFRobot](https://www.dfrobot.com),2016
                  GUN Lesser General Pulic License
       @version V1.0
       @data  2019-8-28
*/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(17, 16); // RX, TX
unsigned char data[4] = {};
float distance;

void setup()
{
  Serial.begin(57600);
  mySerial.begin(9600);
}

void loop() {
  // Wait for data to become available
  if (mySerial.available() > 0) {
    // Try to synchronize with a valid packet by looking for the 0xFF start byte
    if (mySerial.read() == 0xFF) {
      data[0] = 0xFF; // Manually set the first byte since we've already read it

      // Read the next 3 bytes
      for (int i = 1; i < 4; i++) {
        data[i] = mySerial.read();
      }

      // Filter out invalid packets (e.g., if any of the data bytes are 0xFF)
      if (data[1] == 0xFF || data[2] == 0xFF || data[3] == 0xFF) {
        Serial.println("Invalid packet: Skipping");
        return; // Skip processing for this packet
      }

      // Print raw data for debugging
      Serial.print("Raw Data: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(data[i], HEX);  // Print each byte in hexadecimal
        Serial.print(" ");
      }
      Serial.println();

      // Calculate the checksum (sum of first 3 bytes should match the 4th byte)
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3]) {
        // Combine high and low bytes to form distance
        distance = (data[1] << 8) + data[2];
        Serial.print("distance=");
        Serial.print(distance / 10);  // Convert to cm
        Serial.println("cm");
      } else {
        Serial.println("Checksum ERROR");
      }
    }
  }
  delay(100);  // Delay to slow down readings
}
