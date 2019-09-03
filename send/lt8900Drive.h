#include <SPI.h>

#define RSTPin 2 
#define CSPin 4

unsigned char TXCHANNEL = 0;
unsigned char RXCHANNEL = 0;
unsigned char RBUFF[32];

void SPI_WriteReg(unsigned char addr, unsigned char h, unsigned char l){
    digitalWrite(CSPin, LOW);
    SPI.transfer(addr);
    SPI.transfer(h);
    SPI.transfer(l);
    digitalWrite(CSPin, HIGH);
}

unsigned int SPI_ReadReg(unsigned char addr){
    digitalWrite(CSPin, LOW);
    SPI.transfer(addr | 0x80);
    unsigned int highByte = SPI.transfer(0x00);
    unsigned int lowByte = SPI.transfer(0x00);
    digitalWrite(CSPin, HIGH);
    return (highByte << 8 | lowByte);
}

void setTXChannel(unsigned char channel){
    SPI_WriteReg(7, 0x01, channel);
}

void setRXChannel(unsigned char channel){
    SPI_WriteReg(7, 0x00, (0x80 | channel));
}

bool sendPackets(int length, unsigned char* packets){
    if (length < 1 || length > 255)
        return false;
    SPI_WriteReg(52, 0x80, 0x00);   // clear TX_FIFO
    SPI_WriteReg(50, length, packets[0]);
    length--;
    int i = 1;
    while(length > 0){
        length -= 2;
        if(length >= 0)
            SPI_WriteReg(50, packets[i], packets[i+1]);
        else
            SPI_WriteReg(50, packets[i], 0x00);
        i += 2;
    }
    setTXChannel(TXCHANNEL);
    unsigned int r = 0;
    while(true){                   // reg48[6], pkt_flag
        r = SPI_ReadReg(48);
        if((r & 0x40) >> 6)
            break;
    }
    Serial.println("Packets sent success.");
    r = SPI_ReadReg(52);
    if(r & 0x3fff == 0){            // ACK received
        Serial.println("ACK received.");
        return true;
    }
    return false;
}

bool receivePackets(){
    SPI_WriteReg(52, 0x00, 0x80);   // clear RX_FIFO
    setRXChannel(RXCHANNEL);
    unsigned int r = 0;
    while(true){
        r = SPI_ReadReg(48);
        if((r & 0x20 >> 5))
            break;
    }
    Serial.println("Packets received.");
    unsigned char length;
    r = SPI_ReadReg(50);
    RBUFF[0] = r & 0x0f;
    length = r >> 8;
    length--;
    unsigned char i = 1;
    while(length){
        length -= 2;
        r = SPI_ReadReg(50);
        RBUFF[i] = r >> 8;
        if(length >= 0){
            RBUFF[i+1] = r & 0x0f;
        }
        i += 2;
    }
    
    r = SPI_ReadReg(48);
    if((r >> 15) == 0){
        Serial.println("CRC OK.");
        return true;
    }
    return false;
}

void SPI_Init(){
    digitalWrite(RSTPin, LOW);
    delay(2);
    digitalWrite(RSTPin, HIGH);
    delay(5);
    SPI_WriteReg(0, 0x6f, 0xe0);
    SPI_WriteReg(1, 0x56, 0x81);
    SPI_WriteReg(2, 0x66, 0x17);
    SPI_WriteReg(4, 0x9c, 0xc9);
    SPI_WriteReg(5, 0x66, 0x37);
    SPI_WriteReg(7, 0x00, 0x30);
    SPI_WriteReg(8, 0x6c, 0x90);
    SPI_WriteReg(9, 0x18, 0x40);
    SPI_WriteReg(10, 0x7f, 0xfd);
    SPI_WriteReg(11, 0x00, 0x08);
    SPI_WriteReg(12, 0x00, 0x00);
    SPI_WriteReg(13, 0x48, 0xbd);
    SPI_WriteReg(22, 0x00, 0xff);
    SPI_WriteReg(23, 0x80, 0x05);
    SPI_WriteReg(24, 0x00, 0x67);
    SPI_WriteReg(25, 0x16, 0x59);
    SPI_WriteReg(26, 0x19, 0xe0);
    SPI_WriteReg(27, 0x13, 0x00);
    SPI_WriteReg(28, 0x18, 0x00);
    SPI_WriteReg(32, 0x48, 0x00);
    SPI_WriteReg(33, 0x3f, 0xc7);
    SPI_WriteReg(34, 0x20, 0x00);
    SPI_WriteReg(35, 0x03, 0x00);

    SPI_WriteReg(36, 0x03, 0x80);
    SPI_WriteReg(37, 0x03, 0x80);
    SPI_WriteReg(38, 0x5a, 0x5a);
    SPI_WriteReg(39, 0x03, 0x80);

    SPI_WriteReg(40, 0x21, 0x02);
    SPI_WriteReg(41, 0xb0, 0x00);
    SPI_WriteReg(42, 0xfd, 0xb0);
    SPI_WriteReg(43, 0x00, 0x0f);

    unsigned int r = SPI_ReadReg(40);
    Serial.println(r);
    if(r == 0x2102)
        Serial.println("Initial success.");
}
