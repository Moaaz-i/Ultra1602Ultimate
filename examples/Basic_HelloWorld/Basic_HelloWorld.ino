#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

void setup() {
  lcd.begin(16, 2);

  lcd.startBlinkingTextAsync("Hello, World!", 1, 0, 500, 0);
  lcd.startBlinkingTextAsync("Ultra1602 Boot", 1, 1, 1000, 1);
}

void loop() {
  lcd.updateAsync();
}
