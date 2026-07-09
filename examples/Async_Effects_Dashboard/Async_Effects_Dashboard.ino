#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

void setup() {
  lcd.begin(16, 2);

  lcd.startMarqueeAsync(
      "SYSTEM RUNNING OPTIMAL - TEMPERATURE: 24C - VOLTAGE: 5.01V ", 0, 300);
  lcd.startBlinkingTextAsync("WARNING", 4, 1, 500);
}

void loop() { lcd.updateAsync(); }
