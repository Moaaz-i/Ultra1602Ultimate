# Ultra1602Ultimate (v2.0.0)

A lightweight, blazing-fast, and flicker-free LCD 1602 library for Arduino architectures. It optimizes screen updates using a smart shadow buffer, rendering only changed characters to completely eliminate display flickering while offering powerful concurrent background tasks.

## ✨ Key Features

- **Smart Refresh Buffer:** No more annoying screen flashes caused by `lcd.clear()`. It compares text updates and only overwrites modified characters.
- **Multi-Slot Asynchronous Engines:** (New in v2.0.0) Run multiple text marquees, fading text, menus, and blinking texts _simultaneously_ in different slots/rows without blocking execution or canceling each other out.
- **Smooth Progress Bar:** Pixel-perfect control over custom characters to draw an ultra-smooth 0-100% precise progress bar.
- **Advanced Text Layouts:** Built-in advanced text properties supporting Auto-Centering (Middle alignment), Right alignment, and Right-to-Left (RTL) rendering.
- **Built-in System Icons:** Out-of-the-box support for drawing dynamic Battery, Wi-Fi signal bars, Heartbeat, and Custom matrix shapes based on string patterns.

---

## 📂 Repository Structure

```text
Ultra1602Ultimate/
├── src/
│   ├── Ultra1602Ultimate.h
│   └── Ultra1602Ultimate.cpp
├── examples/
│   ├── Basic_HelloWorld/
│   │   └── Basic_HelloWorld.ino
│   ├── Async_Effects_Dashboard/
│   │   └── Async_Effects_Dashboard.ino
│   ├── System_Icons_And_Bars/
│   │   └── System_Icons_And_Bars.ino
│   └── Custom_Pattern_Shapes/
│       └── Custom_Pattern_Shapes.ino
├── library.properties
└── README.md
```

## 🚀 Quick Start Example

### Here is how easy it is to run concurrent non-blocking effects on different rows using the new Multi-Slot architecture:

```cpp
#include <Ultra1602Ultimate.h>

// Initialize using your pin configuration (RS, EN, D4, D5, D6, D7, BL)
Ultra1602Ultimate lcd(8, 9, 4, 5, 6, 7, 10);

void setup() {
  lcd.begin(16, 2);

  // Start Blinking Text on Row 0, Slot 0
  lcd.startBlinkingTextAsync("System: Active", 1, 0, 500, 0);

  // Start a scrolling Marquee on Row 1, Slot 1 concurrently!
  lcd.startMarqueeAsync("Loading heavy background tasks... ", 1, 300, 1);
}

void loop() {
  // Processes all slots for all async engines concurrently without delay()
  lcd.updateAsync();
}
```

## 🛠 Multi-Slot Capability Limits

### To optimize RAM usage on microcontrollers like ATmega328P, the library provides hardware-allocated slot bounds:

- Blinking Engine: 4 Independent slots (slot 0 to 3)
- Marquee Engine: 2 Independent slots (slot 0 to 1)
- Fade-In Engine: 2 Independent slots (slot 0 to 1)
- Menu Engine: 2 Independent slots (slot 0 to 1)

> **Note:** Leaving the slot parameter blank defaults to slot = 0.
