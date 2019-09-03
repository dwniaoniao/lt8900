#include <SPI.h>
#include "lt8900Drive.h"

unsigned char p[] = {0, 1, 2, 3};

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
    sendPackets(4, p);
}
