#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

const uint8_t CONTROL_BUTTON_PIN = 2;

const char planeNormal[] =
    "**   "
    "**** "
    "******"
    " *   *"
    " *   *"
    "******"
    "**** "
    "**   ";

const char planeUp[] =
    "   **"
    "  ***"
    " ****"
    "*****"
    "*****"
    " *   "
    " *   "
    "     ";

const char planeDown[] =
    "     "
    " *   "
    " *   "
    "*****"
    "*****"
    " ****"
    "  ***"
    "   **";

const char asteroidPattern[] =
    "  ** "
    " ****"
    "*****"
    " *** "
    "*****"
    "*****"
    " ****"
    "  ** ";

uint8_t shipRow = 0;
uint8_t lastShipRow = 0;
uint8_t activePlaneSprite = 5;

int8_t asteroidCol = 15;
uint8_t asteroidRow = 0;

uint32_t lastAsteroidMoveTime = 0;
uint32_t lastIdleAnimTime = 0;
uint16_t gameSpeed = 180;
uint16_t score = 0;
bool isGameOver = false;

int8_t shipColOffset = 0;
bool moveRightDir = true;

void updateScoreDisplay() {
    lcd.setCursor(9, 0);
    lcd.print("Scr:");
    lcd.print(score);
}

void setup() {
    lcd.begin(16, 2);

    pinMode(CONTROL_BUTTON_PIN, INPUT_PULLUP);

    lcd.drawShapeByPattern(0, 0, planeDown, 4);
    lcd.drawShapeByPattern(0, 0, planeNormal, 5);
    lcd.drawShapeByPattern(0, 0, planeUp, 6);
    lcd.drawShapeByPattern(0, 0, asteroidPattern, 7);

    lcd.hardClear();
    randomSeed(analogRead(A0));

    resetGame();
}

void loop() {
    if (isGameOver) {
        if (digitalRead(CONTROL_BUTTON_PIN) == LOW) {
            resetGame();
        }
        return;
    }

    uint32_t currentTime = millis();

    if (digitalRead(CONTROL_BUTTON_PIN) == LOW) {
        shipRow = 0;
        if (lastShipRow == 1) {
            activePlaneSprite = 6; // Up
        } else {
            activePlaneSprite = 5; // Normal
        }
    } else {
        shipRow = 1;
        if (lastShipRow == 0) {
            activePlaneSprite = 4; // Down
        } else {
            activePlaneSprite = 5; // Normal
        }
    }

    if (currentTime - lastIdleAnimTime >= 300) {
        lastIdleAnimTime = currentTime;

        lcd.setCursor(shipColOffset, shipRow);
        lcd.write(' ');

        if (moveRightDir) {
            shipColOffset = 1;
            moveRightDir = false;
        } else {
            shipColOffset = 0;
            moveRightDir = true;
        }
    }

    if (shipRow != lastShipRow) {
        lcd.setCursor(0, lastShipRow); lcd.write(' ');
        lcd.setCursor(1, lastShipRow); lcd.write(' ');
        lastShipRow = shipRow;
    }

    lcd.setCursor(shipColOffset, shipRow);
    lcd.write(activePlaneSprite);

    if (currentTime - lastAsteroidMoveTime >= gameSpeed) {
        lastAsteroidMoveTime = currentTime;

        if (asteroidCol >= 0 && asteroidCol < 16) {
            lcd.setCursor(asteroidCol, asteroidRow);
            lcd.write(' ');

            if (asteroidRow == 0 && asteroidCol >= 9) {
                updateScoreDisplay();
            }
        }

        asteroidCol--;

        if (asteroidCol < 0) {
            score++;
            updateScoreDisplay();

            if (gameSpeed > 60) gameSpeed -= 8;

            asteroidCol = 15;
            asteroidRow = random(0, 2);
        }

        if ((asteroidCol == shipColOffset) && asteroidRow == shipRow) {
            isGameOver = true;
            lcd.hardClear();

            lcd.setCursor(3, 0);
            lcd.print("GAME OVER!");

            lcd.setCursor(4, 1);
            lcd.print("Score: ");
            lcd.print(score);
            return;
        }

        if (asteroidCol >= 0 && asteroidCol < 16) {
            lcd.setCursor(asteroidCol, asteroidRow);
            lcd.write(7);
        }
    }
}

void resetGame() {
    while (digitalRead(CONTROL_BUTTON_PIN) == LOW) {
        delay(10);
    }

    score = 0;
    asteroidCol = 15;
    asteroidRow = random(0, 2);
    gameSpeed = 180;
    isGameOver = false;
    shipRow = 1;
    lastShipRow = 1;
    shipColOffset = 0;
    activePlaneSprite = 5;

    lcd.hardClear();
    updateScoreDisplay();

    lcd.setCursor(shipColOffset, shipRow);
    lcd.write(activePlaneSprite);

    lcd.setCursor(asteroidCol, asteroidRow);
    lcd.write(7);

    lastAsteroidMoveTime = millis();
    lastIdleAnimTime = millis();
}
