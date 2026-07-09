#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

void setup() {
    lcd.begin(16, 2);
    lcd.printAdvanced("Hello, World!", 1, 0);

    TextProps props;
    props.align = 1;
    lcd.printAdvanced("Ultra1602 Boot", 0, 1, props);
}

void loop() {

}
