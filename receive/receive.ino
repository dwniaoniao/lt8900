#include <SPI.h>
#include "lt8900Drive.h"

unsigned char receivePacketsLength;

void setup(){
    pinMode(CSPin, OUTPUT);
    pinMode(RSTPin, OUTPUT);
    digitalWrite(CSPin, HIGH);
    digitalWrite(RSTPin, LOW);
    Serial.begin(9600);
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    SPI_Init(); 
    delay(3000);
}

void loop(){
    receivePacketsLength = receivePackets();
    if(receivePacketsLength){
        Serial.print("receivePacketsLength = ");
        Serial.println(receivePacketsLength);
        Serial.println(RBUFF[0]);
        Serial.println(RBUFF[1]);
        Serial.println(RBUFF[2]);
        Serial.println(RBUFF[3]);
    }
}
