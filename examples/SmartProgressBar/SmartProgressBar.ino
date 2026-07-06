/**
 * SmartProgressBar.ino
 * * This example demonstrates the core features of the Ultra1602Ultimate library:
 * 1. Flicker-free smart text refreshing (Smart Refresh Buffer).
 * 2. Automatic text centering using printSmart.
 * 3. Smooth sub-character precise progress bar.
 * 4. Built-in system icons (Battery and Signal).
 * * Circuit Connections (Native 4-bit Mode):
 * - LCD RS pin to digital pin 8
 * - LCD Enable pin to digital pin 9
 * - LCD D4 pin to digital pin 4
 * - LCD D5 pin to digital pin 5
 * - LCD D6 pin to digital pin 6
 * - LCD D7 pin to digital pin 7
 * - LCD R/W pin to Ground
 * - LCD VSS pin to Ground
 * - LCD VCC pin to 5V
 * - LCD VO pin to Potentiometer (Contrast)
 */

#include <Ultra1602Ultimate.h>

// Initialize the library in Native Mode (false = No I2C)
Ultra1602Ultimate lcd(false);

void setup() {
    // Initialize display hardware and load custom characters
    lcd.init();
}

void loop() {
    // Loop to increment the progress percentage from 0 to 100
    for (uint8_t i = 0; i <= 100; i++) {

        // Allocate a buffer for 3 digits + trailing space to clean ghost characters
        char numBuffer[5];

        // Convert the integer 'i' to a string base 10
        itoa(i, numBuffer, 10);

        // Append a space to clear any leftover digits from previous cycles (e.g., from 100 to 0)
        uint8_t len = 0;
        while (numBuffer[len] != '\0') len++;
        numBuffer[len] = ' ';
        numBuffer[len + 1] = '\0';

        // Print the formatted string centered on the first row (row 0)
        lcd.printSmart(numBuffer, 0, true);

        // Draw the precise pixel-by-pixel progress bar on the second row (row 1)
        lcd.drawPreciseProgressBar(1, i);

        // Redraw system icons on the edges over the progress bar to keep them static
        lcd.drawBatteryIcon(0, 1, 100);  // Battery icon at column 0, row 1
        lcd.drawSignalIcon(15, 1, 4);   // Signal icon at column 15, row 1

        // Delay to make the progress visual and comfortable to track
        delay(200);
    }

    // Hold the screen at 100% for 3 seconds before resetting the loop
    delay(3000);
}
