#include <Arduino.h>
#include <Wire.h>
//Skeleton code for RFR battery temp sensor prototyped on Arduino Uno dev board 09/28/2026. Revised 10/14/2026 CKK 

#define FAULT_LED 8
#define N1 7
#define N2 6
#define N3 5
#define N4 4

//millis delay variables
bool overHeatToggle;
bool LEDToggle;
unsigned long previousMillis;

uint8_t interval = 250; //dictates how long fault LED blinks
uint8_t inputs = 16; //!!!!!!!!!!!!CHANGE TO 8 FOR FINAL DELIVERABLE. WE R USING DA CD4051!!!!!!!!!!!!!!!!!
uint8_t tableSize = 33; //how many rows are in the matrix
uint8_t gain = 1; // dictated by amplifier gain feeding into A0 Input. 

// Matrix  for function to pull data from
// From a 5V reference, each ADC bit equates to ~4.89mV. 
int16_t lookUpTable[tableSize][2] = {
    {499, -40}, // 499 * 0.00489 = 2.44V. 2.44V = -40C per sensor datasheet 
    {495, -35}, // so on so fourth...
    {491, -30}, // this is what makes gain 
    {487, -25}, // before the A0 pin important.
    {481, -20}, // As it allows for more range of the ADC to be used. 
    {475, -15}, // Currently, at 1x gain, we have a range of 233 bits
    {464, -10}, // If we were to double the gain we would have 466 bits. 
    {456,  -5}, // Increasing the resolution and accuracy of the device
    {444,   0}, // If we look at the bottom 2 rows. We see there is a
    {432,   5}, // difference of 2 bits between 115C and 120C. Thats wild.
    {419,  10}, // If there was a gain of 2x before the A0 pin we would have
    {407,  15}, // a range of 4 bits. Increasing the accuracy of our digital readout. 
    {393,  20}, 
    {381,  25},
    {368,  30},
    {356,  35},
    {344,  40},
    {333,  45},
    {325,  50},
    {317,  55},
    {309,  60},
    {303,  65},
    {297,  70},
    {293,  75},
    {286,  80},
    {282,  85},
    {280,  90},
    {276,  95},
    {274, 100},
    {272, 105},
    {270, 110},
    {268, 115},
    {266, 120}
};

// bitread converts decimal -> binary. X-X-X-X -> N4-N3-N2-N1 -> 5 = 1-0-0-1
// This function counts up in binary. For each count it takes input from the A0 pin. Allows for ease for if we decide to output the data of each individual sensor.
void binary(byte channel){
    digitalWrite(N1, bitRead(channel, 0));
    digitalWrite(N2, bitRead(channel, 1));
    digitalWrite(N3, bitRead(channel, 2));
    digitalWrite(N4, bitRead(channel, 3));
}


float adcToTemp(int16_t adc)
{
    float temperature;

    if (adc > gain * 499 || adc < gain * 266)
        return NAN; // ADC is out of range - replace is an error code. 

    //ADC is in range. for loop checks every 
    for (uint8_t i = 0; i < tableSize-1; i++){
        int16_t adc1 = gain * lookUpTable[i][0];
        int16_t adc2 = gain * lookUpTable[i+1][0];
        int16_t temp1 = lookUpTable[i][1];
        int16_t temp2 = lookUpTable[i + 1][1];

        //Happens only once in the loop when the ADC value is < adc1 & > adc2. 
        if(adc < adc1 && adc > adc2){
        // linear interpolation. Approximates temperature given the datasheet only provided 5C intervals
        temperature = temp1 + (adc - adc1) * (temp2 - temp1) / (adc2 - adc1);   
        }
    }
    return temperature;
}


void setup() {
//Serial is only temporary. Remove once I2C communication gets resolved 
    Serial.begin(9600);
    pinMode(A0, INPUT);
    Wire.begin();
}


void loop() {


for(uint8_t i = 0; i < inputs; i++){

binary(i); //triggers binary sequence 

//Serial  UART is only temporary. Remove once I2C communication gets resolved 
Serial.print("Sensor ");
Serial.print(i + 1);
Serial.print(" Temp: ");
Serial.println( adcToTemp(analogRead(A0)));

// implement I2C wire library to output the bytes to bus
// Send fault code
// Send temperature data alongside their respective sensor #

//activates fault code
if(analogRead(A0) > gain * 309) //1.51V = 60 degree C. 
    overHeatToggle = 0;
if(analogRead(A0) < gain * 300) //allows for some jitter to happen in the ADC before flipping off again. Digital filtering.
    overHeatToggle = 1; //
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


