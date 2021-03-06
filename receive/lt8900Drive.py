import RPi.GPIO as GPIO
import spidev
from time import sleep

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

RSTPin = 23
CSPin = 24

TXCHANNEL = 0
RXCHANNEL = 0

GPIO.setup((RSTPin, CSPin), GPIO.OUT, initial=GPIO.LOW)
GPIO.output(CSPin, GPIO.HIGH)

bus = 0
device = 0
spi = spidev.SpiDev()
spi.open(bus, device)
spi.max_speed_hz = 1000000
spi.mode = 0b01
spi.cshigh = True
spi.threewire = True

def SPI_WriteReg(addr, h, l):
    GPIO.output(CSPin, GPIO.LOW)
    spi.writebytes([addr, h, l])
    GPIO.output(CSPin, GPIO.HIGH)

def SPI_ReadReg(addr):
    GPIO.output(CSPin, GPIO.LOW)
    spi.writebytes([addr | 0x80])
    l = spi.readbytes(2)
    GPIO.output(CSPin, GPIO.HIGH)
    print(l)
    r = l[0] << 8 | l[1]
    return r

def setTXChannel(channel):
    SPI_WriteReg(7, 0x01, channel)

def setRXChannel(channel):
    SPI_WriteReg(7, 0x00, (0x80 | channel))

def sendPackets(length, packets):
    if length < 1 or length > 255:
        return False
    SPI_WriteReg(52, 0x80, 0x00)
    SPI_WriteReg(50, length, packets[0])
    length -= 1
    i = 1
    while length > 0:
        length -= 2
        if length >= 0:
            SPI_WriteReg(50, packets[i], packets[i+1])
        else:
            SPI_WriteReg(50, packets[i], 0x00)
        i += 2
        
    setTXChannel(TXCHANNEL)
    while True:
        r = SPI_ReadReg(48)
        if (r & 0x40) >> 6:
            break
    print("Packets sent success.")
    r = SPI_ReadReg(52)
    if not r & 0x3fff:
        print("ACK received.")
        return True
    return False

def receivePackets():
    RBUFF = []
    SPI_WriteReg(52, 0x00, 0x80)
    setRXChannel(RXCHANNEL)
    while True:
        r = SPI_ReadReg(48)
        if (r & 0x40) >> 6:
            break
    print("Packets received.")
    r = SPI_ReadReg(50)
    RBUFF.append(r & 0x0f)
    length = r >> 8
    length -= 1 
    while length:
        length -= 2
        r = SPI_ReadReg(50)
        RBUFF.append(r >> 8)
        if length >= 0:
            RBUFF.append(r & 0x0f)

    r = SPI_ReadReg(48)
    if not r >> 15:
        print("CRC OK.")
        return RBUFF
    return None

def SPI_Init():
    GPIO.output(RSTPin, GPIO.LOW)
    sleep(0.002)
    GPIO.output(RSTPin, GPIO.HIGH)
    sleep(0.005)
    
    SPI_WriteReg(0, 0x6f, 0xe0)
    SPI_WriteReg(1, 0x56, 0x81)
    SPI_WriteReg(2, 0x66, 0x17)
    SPI_WriteReg(4, 0x9c, 0xc9)
    SPI_WriteReg(5, 0x66, 0x37)
    SPI_WriteReg(7, 0x00, 0x30)
    SPI_WriteReg(8, 0x6c, 0x90)
    SPI_WriteReg(9, 0x18, 0x40)
    SPI_WriteReg(10, 0x7f, 0xfd)
    SPI_WriteReg(11, 0x00, 0x08)
    SPI_WriteReg(12, 0x00, 0x00)
    SPI_WriteReg(13, 0x48, 0xbd)
    SPI_WriteReg(22, 0x00, 0xff)
    SPI_WriteReg(23, 0x80, 0x05)
    SPI_WriteReg(24, 0x00, 0x67)
    SPI_WriteReg(25, 0x16, 0x59)
    SPI_WriteReg(26, 0x19, 0xe0)
    SPI_WriteReg(27, 0x13, 0x00)
    SPI_WriteReg(28, 0x18, 0x00)
    SPI_WriteReg(32, 0x48, 0x00)
    SPI_WriteReg(33, 0x3f, 0xc7)
    SPI_WriteReg(34, 0x20, 0x00)
    SPI_WriteReg(35, 0x03, 0x00)

    SPI_WriteReg(36, 0x03, 0x80)
    SPI_WriteReg(37, 0x03, 0x80)
    SPI_WriteReg(38, 0x5a, 0x5a)
    SPI_WriteReg(39, 0x03, 0x80)

    SPI_WriteReg(40, 0x21, 0x02)
    SPI_WriteReg(41, 0xb0, 0x00)
    SPI_WriteReg(42, 0xfd, 0xb0)
    SPI_WriteReg(43, 0x00, 0x0f)

    r = SPI_ReadReg(40)
    print(r)
    if r == 0x2102:
        print("Initial success.")


if __name__ == "__main__":
    p = [1, 2, 3, 4]
    SPI_Init()
    sleep(3)

    while True:
        receivePackets()
        sendPackets(4, p)
