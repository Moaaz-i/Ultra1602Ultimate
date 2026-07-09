#include <Ultra1602Ultimate.h>

Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

const uint8_t JUMP_BUTTON_PIN = 2;

const char playerPattern[] =
    "  *  "
    " *** "
    "  *  "
    " *** "
    "*****"
    "  *  "
    " * * "
    " * * ";

const char obstaclePattern[] =
    "  *  "
    " *** "
    " *** "
    "*****"
    "  *  "
    "  *  "
    " *** "
    "*****";

uint8_t playerRow = 1;
uint8_t lastPlayerRow = 1;
int8_t obstacleCol = 15;
uint32_t lastGameUpdateTime = 0;
uint16_t gameSpeed = 250;
uint16_t score = 0;
bool isGameOver = false;

void setup() {
    lcd.begin(16, 2);

    pinMode(JUMP_BUTTON_PIN, INPUT_PULLUP);

    lcd.drawShapeByPattern(0, 0, playerPattern, 6);
    lcd.drawShapeByPattern(0, 0, obstaclePattern, 7);

    lcd.hardClear();
    lcd.printAdvanced("Score: 0", 0, 0);
}

void loop() {
    if (isGameOver) {
        if (digitalRead(JUMP_BUTTON_PIN) == LOW) {
            resetGame();
        }
        return;
    }

    if (digitalRead(JUMP_BUTTON_PIN) == LOW) {
        playerRow = 0;
    } else {
        playerRow = 1;
    }

    uint32_t currentTime = millis();

    if (currentTime - lastGameUpdateTime >= gameSpeed) {
        lastGameUpdateTime = currentTime;

        if (obstacleCol >= 0 && obstacleCol < 16) {
            lcd.setCursor(obstacleCol, 1);
            lcd.write(' ');
        }

        obstacleCol--;

        if (obstacleCol < 0) {
            obstacleCol = 15;
            score++;

            char scoreStr[12];
            sprintf(scoreStr, "Score: %d", score);
            lcd.printAdvanced(scoreStr, 0, 0);

            if (gameSpeed > 100) gameSpeed -= 15;
        }

        if (obstacleCol == 2 && playerRow == 1) {
            isGameOver = true;
            lcd.hardClear();
            lcd.printAdvanced("GAME OVER!", 3, 0);
            lcd.printAdvanced("Press to Retry", 1, 1);
            return;
        }

        if (obstacleCol >= 0 && obstacleCol < 16) {
            lcd.setCursor(obstacleCol, 1);
            lcd.write(7);
        }
    }

    if (playerRow != lastPlayerRow) {
        lcd.setCursor(2, lastPlayerRow);
        lcd.write(' ');
        lastPlayerRow = playerRow;
    }

    lcd.setCursor(2, playerRow);
    lcd.write(6);
}

void resetGame() {
    while (digitalRead(JUMP_BUTTON_PIN) == LOW) {
        delay(10);
    }

    score = 0;
    obstacleCol = 15;
    gameSpeed = 250;
    isGameOver = false;

    playerRow = 1;
    lastPlayerRow = 1;

    lcd.hardClear();
    lcd.printAdvanced("Score: 0", 0, 0);

    lcd.setCursor(2, playerRow);
    lcd.write(6);

    lastGameUpdateTime = millis();
}
