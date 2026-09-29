#include <Arduino.h>

//Bare code for RFR battery temp sensor prototyped on Arduino Uno dev board 09/28/2026 CKK

#define FAULT_LED 8
#define N1 7
#define N2 6
#define N3 5
#define N4 4


bool overHeatToggle;
bool LEDToggle;
unsigned long previousMillis;
uint8_t interval = 250; 


float tempConversion(uint8_t rawTemp){

float convTemp = rawTemp * 2 + 69; // Create function. This is just a test formula DELETE 
return convTemp;
} 


// bitread converts decimal -> binary. X-X-X-X -> N4-N3-N2-N1 -> 5 = 1-0-0-1
// This function counts up in binary. For each count it takes input from the A0 pin. Allows for ease for if we decide to output the data of each individual sensor.
float tempReading(byte channel){
    digitalWrite(N1, bitRead(channel, 0));
    digitalWrite(N2, bitRead(channel, 1));
    digitalWrite(N3, bitRead(channel, 2));
    digitalWrite(N4, bitRead(channel, 3));

    uint16_t temp = analogRead(A0);
    return temp * (5.0 / 1023.0); //converts ADC to voltage. 5.0 = ADC voltage range. 1023 = ADC resolution
}


void setup() {
//Serial is only temporary. Remove once I2C communication gets resolved 
    Serial.begin(9600);
    pinMode(A0, INPUT);
}

void loop() {


for(uint8_t i = 0; i < 16; i++){

float Voltage = tempReading(i);

//Serial is only temporary. Remove once I2C communication gets resolved 
Serial.print("Sensor ");
Serial.print(i + 1);
Serial.print(" Voltage: ");
Serial.println(Voltage);

Serial.print("Sensor ");
Serial.print(i + 1);
Serial.print(" Temp: ");
Serial.println(tempConversion(Voltage));

// implement I2C wire library to output the bytes to bus
// Send fault code
// Send temperature data alongside their respective sensor #

if(tempReading(i) > 1.51) //1.51V = 60 degree C. 
    overHeatToggle = 1;
if(tempReading(i) < 1.53) //allows for some jitter to happen in the ADC before flipping off again. 
    overHeatToggle = 0;
}


//millis() delay. Looks confusing and it is at first. Read this dude: https://arduinogetstarted.com/faq/how-to-use-millis-instead-of-delay
if(overHeatToggle){
    if(millis() - previousMillis >= interval){
        previousMillis = millis();
        LEDToggle = !LEDToggle;
        digitalWrite(FAULT_LED, LEDToggle);
    }
}
else{
LEDToggle = 0;
digitalWrite(FAULT_LED, 0);
}
}


