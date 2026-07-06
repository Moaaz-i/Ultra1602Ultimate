#include "Ultra1602Ultimate.h"
#include <Arduino.h>

#define LCD_RS 8
#define LCD_EN 9
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

Ultra1602Ultimate::Ultra1602Ultimate(bool i2cMode, uint8_t address) {
    useI2C = i2cMode;
    i2cAddr = address << 1;
    i2cBacklight = 0x08;
    displayControl = 0x04;
    displayMode = 0x02;
    for (uint8_t r = 0; r < 2; r++)
        for (uint8_t c = 0; c < 16; c++) shadowBuffer[r][c] = ' ';
}

void Ultra1602Ultimate::i2cStart() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void Ultra1602Ultimate::i2cStop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void Ultra1602Ultimate::i2cWriteByte(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void Ultra1602Ultimate::i2cSend(uint8_t value, uint8_t mode) {
    uint8_t highnib = value & 0xF0;
    uint8_t lownib = (value << 4) & 0xF0;
    i2cStart();
    i2cWriteByte(i2cAddr);
    i2cWriteByte(highnib | mode | i2cBacklight | 0x04);
    i2cWriteByte(highnib | mode | i2cBacklight);
    i2cWriteByte(lownib | mode | i2cBacklight | 0x04);
    i2cWriteByte(lownib | mode | i2cBacklight);
    i2cStop();
}

void Ultra1602Ultimate::nativePulseEnable(uint8_t value) {
    digitalWrite(LCD_EN, HIGH);
    delayMicroseconds(5);
    digitalWrite(LCD_EN, LOW);
    delayMicroseconds(100);
}

void Ultra1602Ultimate::nativeWrite4Bits(uint8_t value) {
    digitalWrite(LCD_D4, (value >> 4) & 0x01);
    digitalWrite(LCD_D5, (value >> 5) & 0x01);
    digitalWrite(LCD_D6, (value >> 6) & 0x01);
    digitalWrite(LCD_D7, (value >> 7) & 0x01);
    nativePulseEnable(0);
}

void Ultra1602Ultimate::nativeSend(uint8_t value, uint8_t mode) {
    digitalWrite(LCD_RS, mode);

    digitalWrite(LCD_D4, (value >> 4) & 0x01);
    digitalWrite(LCD_D5, (value >> 5) & 0x01);
    digitalWrite(LCD_D6, (value >> 6) & 0x01);
    digitalWrite(LCD_D7, (value >> 7) & 0x01);
    nativePulseEnable(0);

    digitalWrite(LCD_D4, value & 0x01);
    digitalWrite(LCD_D5, (value >> 1) & 0x01);
    digitalWrite(LCD_D6, (value >> 2) & 0x01);
    digitalWrite(LCD_D7, (value >> 3) & 0x01);
    nativePulseEnable(0);
}

void Ultra1602Ultimate::init() {
    if (useI2C) {
        TWBR = 2;
        TWSR = 0;
        i2cSend(0x30, 0);
        i2cSend(0x30, 0);
        i2cSend(0x30, 0);
        i2cSend(0x20, 0);
    } else {
        pinMode(LCD_RS, OUTPUT);
        pinMode(LCD_EN, OUTPUT);
        pinMode(LCD_D4, OUTPUT);
        pinMode(LCD_D5, OUTPUT);
        pinMode(LCD_D6, OUTPUT);
        pinMode(LCD_D7, OUTPUT);

        digitalWrite(LCD_RS, LOW);
        digitalWrite(LCD_EN, LOW);

        delay(100);
        nativeWrite4Bits(0x30);
        delay(10);
        nativeWrite4Bits(0x30);
        delayMicroseconds(200);
        nativeWrite4Bits(0x30);
        nativeWrite4Bits(0x20);
    }
    sendCommand(0x28);
    sendCommand(0x08 | displayControl);
    sendCommand(0x04 | displayMode);
    clear();

    static const uint8_t PROGMEM barChars[5][8] = {
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
        {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
        {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
        {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
        {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
    };
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t temp[8];
        for (uint8_t j = 0; j < 8; j++) temp[j] = pgm_read_byte(&barChars[i][j]);
        createChar(i + 1, temp);
    }
}

void Ultra1602Ultimate::sendCommand(uint8_t cmd) {
    if (useI2C) i2cSend(cmd, 0);
    else nativeSend(cmd, 0);
}

void Ultra1602Ultimate::sendData(uint8_t data) {
    if (useI2C) i2cSend(data, 1);
    else nativeSend(data, 1);
}

void Ultra1602Ultimate::clear() {
    sendCommand(0x01);
    delay(5);
    for (uint8_t r = 0; r < 2; r++)
        for (uint8_t c = 0; c < 16; c++) shadowBuffer[r][c] = ' ';
}

void Ultra1602Ultimate::setCursor(uint8_t col, uint8_t row) {
    uint8_t offsets[] = { 0x00, 0x40 };
    sendCommand(0x80 | (col + offsets[row & 1]));
}

void Ultra1602Ultimate::print(const char* str) {
    while (*str) sendData(*str++);
}

void Ultra1602Ultimate::createChar(uint8_t location, const uint8_t charmap[]) {
    location &= 0x07;
    sendCommand(0x40 | (location << 3));
    for (uint8_t i = 0; i < 8; i++) sendData(charmap[i]);
}

void Ultra1602Ultimate::printSmart(const char* str, uint8_t row, bool center) {
    uint8_t len = 0;
    while (str[len]) len++;
    if (len > 16) len = 16;
    uint8_t startCol = center ? (16 - len) / 2 : 0;
    setCursor(startCol, row);
    for (uint8_t i = 0; i < len; i++) {
        if (shadowBuffer[row & 1][startCol + i] != str[i]) {
            shadowBuffer[row & 1][startCol + i] = str[i];
            sendData(str[i]);
        } else {
            setCursor(startCol + i + 1, row);
        }
    }
}

void Ultra1602Ultimate::drawPreciseProgressBar(uint8_t row, uint8_t percentage) {
    if (percentage > 100) percentage = 100;
    uint8_t totalPixels = (percentage * 80) / 100;
    uint8_t fullBars = totalPixels / 5;
    uint8_t remainder = totalPixels % 5;
    setCursor(0, row);
    for (uint8_t i = 0; i < 16; i++) {
        if (i < fullBars) sendData(5);
        else if (i == fullBars && remainder > 0) sendData(remainder);
        else sendData(' ');
    }
}

void Ultra1602Ultimate::drawBatteryIcon(uint8_t col, uint8_t row, uint8_t percentage) {
    uint8_t bat[8] = {0x0E, 0x1B, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F};
    if (percentage > 20) bat[6] = 0x1F;
    if (percentage > 40) bat[5] = 0x1F;
    if (percentage > 60) bat[4] = 0x1F;
    if (percentage > 80) bat[3] = 0x1F;
    createChar(6, bat);
    setCursor(col, row);
    sendData(6);
}

void Ultra1602Ultimate::drawSignalIcon(uint8_t col, uint8_t row, uint8_t strength) {
    uint8_t sig[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
    if (strength > 1) sig[6] = 0x18;
    if (strength > 2) sig[5] = 0x1C;
    if (strength > 3) sig[4] = 0x1E;
    if (strength > 4) sig[3] = 0x1F;
    createChar(7, sig);
    setCursor(col, row);
    sendData(7);
}
