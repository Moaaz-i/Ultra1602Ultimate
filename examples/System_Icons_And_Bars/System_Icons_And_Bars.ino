#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

uint8_t batteryLevel = 10;
uint8_t progress = 0;

void setup() {
    lcd.begin(16, 2);
    lcd.printAdvanced("Status:", 0, 0);
}

void loop() {
    lcd.drawBatteryIcon(13, 0, batteryLevel);

    lcd.drawWifiIcon(15, 0, 3);
    lcd.drawPreciseProgressBar(1, progress, 0, 15);

    batteryLevel += 20;
    if (batteryLevel > 100) batteryLevel = 10;

    progress += 5;
    if (progress > 100) progress = 0;

    delay(800);
}
