#include <SoftwareSerial.h>
char col;// for storing the data read from the serial port
unsigned long duration;
int datapin = 13;
int val = 0;

int LED_BUILTIN = 12;

void setup() {
        Serial.begin(57600);     //Enable the serial port and set band rate to 57600 bps
        pinMode(LED_BUILTIN, OUTPUT);
        pinMode(datapin, INPUT);
        digitalWrite(LED_BUILTIN, HIGH);
}

void loop(){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(57); 
        digitalWrite(LED_BUILTIN, LOW);
        delay(4);                         //give level signal          
        duration = pulseIn(datapin, HIGH);//read the high level pulse on the pin, the maximum pulse interval is 55ms; assign the result to the variable "duration"
        delay(3);
        digitalWrite(LED_BUILTIN, HIGH);//Pull up the signal pin 
        duration = duration/ 58.00; //convert time to distance    
        Serial.print(duration);//serial port print data
        Serial.println("cm");
        Serial.println("=========== ");        
        } 
