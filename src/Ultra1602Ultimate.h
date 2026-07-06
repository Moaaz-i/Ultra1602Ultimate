#ifndef ULTRA1602ULTIMATE_H
#define ULTRA1602ULTIMATE_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>

class Ultra1602Ultimate {
private:
    bool useI2C;
    uint8_t i2cAddr;
    uint8_t i2cBacklight;
    uint8_t displayControl;
    uint8_t displayMode;
    uint8_t shadowBuffer[2][16];

    void i2cStart();
    void i2cStop();
    void i2cWriteByte(uint8_t data);
    void i2cSend(uint8_t value, uint8_t mode);
    void nativePulseEnable(uint8_t value);
    void nativeWrite4Bits(uint8_t value);
    void nativeSend(uint8_t value, uint8_t mode);

public:
    Ultra1602Ultimate(bool i2cMode = false, uint8_t address = 0x27);
    void init();
    void sendCommand(uint8_t cmd);
    void sendData(uint8_t data);
    void clear();
    void setCursor(uint8_t col, uint8_t row);
    void print(const char* str);
    void createChar(uint8_t location, const uint8_t charmap[]);
    void printSmart(const char* str, uint8_t row, bool center = false);
    void drawPreciseProgressBar(uint8_t row, uint8_t percentage);
    void drawBatteryIcon(uint8_t col, uint8_t row, uint8_t percentage);
    void drawSignalIcon(uint8_t col, uint8_t row, uint8_t strength);
};

#endif
