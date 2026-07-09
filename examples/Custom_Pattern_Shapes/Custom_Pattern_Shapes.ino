#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

const char spaceInvader[] = "*   *"
                            " * * "
                            "*****"
                            "* * *"
                            "*****"
                            "* * *"
                            "*   *"
                            " * * ";

const char pacman[] = " *** "
                      "*****"
                      "**   "
                      "**** "
                      "*****"
                      "*****"
                      " *** "
                      "     ";

void setup() {
  lcd.begin(16, 2);

  lcd.printAdvanced("Custom Shapes:", 0, 0);

  lcd.drawShapeByPattern(5, 1, spaceInvader, 6);
  lcd.drawShapeByPattern(10, 1, pacman, 7);
}

void loop() {}
