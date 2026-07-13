#ifndef Ultra1602Ultimate_h
#define Ultra1602Ultimate_h

#include <Arduino.h>
#include <Print.h>
#include <Wire.h>

struct TextProps {
  uint8_t align = 0;
  uint16_t speed = 50;
};

class Ultra1602Ultimate : public Print {
public:
  Ultra1602Ultimate(uint8_t rs, uint8_t en, uint8_t d4, uint8_t d5, uint8_t d6,
                    uint8_t d7, uint8_t bl_pin = 255);

  Ultra1602Ultimate(uint8_t lcd_addr = 0x27, uint8_t bl_pin = 255);

  void begin(uint8_t cols, uint8_t rows);

  void clear();
  void hardClear();
  void setCursor(uint8_t col, uint8_t row);
  virtual size_t write(uint8_t character) override;

  void updateAsync();

  void printAdvanced(const char *str, uint8_t col, uint8_t row,
                     TextProps props = TextProps());
  void printAdvanced(const __FlashStringHelper *ifsh, uint8_t col, uint8_t row,
                     TextProps props = TextProps());
  void printAdvanced(int num, uint8_t col, uint8_t row,
                     TextProps props = TextProps());
  void printFloat(float num, uint8_t decimals, uint8_t col, uint8_t row,
                  TextProps props = TextProps());
  void printRightToLeft(const char *str, uint8_t col, uint8_t row);

  void startMarqueeAsync(const char *text, uint8_t row, uint16_t interval_ms,
                         uint8_t slot = 0);
  void startMarqueeAsync(const __FlashStringHelper *text, uint8_t row,
                         uint16_t interval_ms, uint8_t slot = 0);
  void stopMarqueeAsync(uint8_t slot = 0);

  void startBlinkingTextAsync(const char *text, uint8_t col, uint8_t row,
                              uint16_t blink_interval_ms, uint8_t slot = 0);
  void stopBlinkingTextAsync(uint8_t slot = 0);

  void startFadeInAsync(const char *text, uint8_t col, uint8_t row,
                        uint16_t char_interval_ms, uint8_t slot = 0);

  void startMenuAsync(const char **items, uint8_t itemCount,
                      uint16_t auto_scroll_ms, uint8_t row, uint8_t slot = 0);
  void stopMenuAsync(uint8_t slot = 0);

  void drawPreciseProgressBar(uint8_t row, uint8_t percentage,
                              uint8_t startCol = 0, uint8_t endCol = 15,
                              bool bordered = false);
  void drawBarGraphHorizontal(uint8_t row, uint8_t val1, uint8_t val2,
                              uint8_t maxVal = 10);
  void drawBatteryIcon(uint8_t col, uint8_t row, uint8_t percentage,
                       uint8_t location = 1);
  void drawWifiIcon(uint8_t col, uint8_t row, uint8_t level,
                    uint8_t location = 2);
  void drawSignalIcon(uint8_t col, uint8_t row, uint8_t strength,
                      uint8_t location = 3);
  void drawLockIcon(uint8_t col, uint8_t row, bool locked,
                    uint8_t location = 4);
  void drawStorageGauge(uint8_t col, uint8_t row, uint8_t usedPercentage,
                        uint8_t location = 5);
  void drawHeartbeat(uint8_t col, uint8_t row, bool state,
                     uint8_t location = 6);
  void drawSpinner(uint8_t col, uint8_t row, uint8_t frame,
                   bool clockwise = true);
  void drawShapeByPattern(uint8_t col, uint8_t row, const char *pattern,
                          uint8_t cg_location = 7);

  void invertRow(uint8_t row);
  void clearRow(uint8_t row);
  void clearRange(uint8_t row, uint8_t startCol, uint8_t endCol);
  void fillScreen(char ch);
  void savePower(bool enable);
  void setBacklightBrightness(uint8_t brightness);
  uint8_t getBacklightBrightness() const { return _bl_brightness; }
  uint32_t getLastUpdateMicros() const { return _lastUpdateDuration; }
  void createCustomChar(uint8_t location, const uint8_t charmap[]);

private:
  void sendCommand(uint8_t cmd);
  void sendData(uint8_t data);
  void write4Bits(uint8_t value);
  void pulseEnable();

  uint8_t _rs_pin, _en_pin, _bl_pin, _bl_brightness;
  uint8_t _data_pins[4];
  uint8_t _cols, _rows;
  uint8_t _curr_col, _curr_row;
  uint8_t _shadowBuffer[4][20];
  uint32_t _lastUpdateDuration;

  uint8_t _iconCacheValue[8];

  uint32_t _marqueeLastTime[2];
  uint16_t _marqueeInterval[2];
  const char *_marqueeText[2];
  const __FlashStringHelper *_marqueeFlashText[2];
  uint8_t _marqueeRow[2], _marqueeStep[2];
  bool _marqueeActive[2], _isMarqueeFlash[2];

  uint32_t _blinkLastTime[4];
  uint16_t _blinkInterval[4];
  const char *_blinkText[4];
  uint8_t _blinkCol[4], _blinkRow[4];
  bool _blinkState[4], _blinkActive[4];

  uint32_t _fadeInLastTime[2];
  uint16_t _fadeInInterval[2];
  const char *_fadeInText[2];
  uint8_t _fadeInCol[2], _fadeInRow[2], _fadeInIndex[2];
  bool _fadeInActive[2];

  const char **_menuItems[2];
  uint8_t _menuItemCount[2], _menuCurrentItem[2], _menuRow[2];
  uint32_t _menuLastTime[2];
  uint16_t _menuInterval[2];
  bool _menuActive[2];

  uint8_t _addr;
  bool _isI2C = false;
  uint8_t _backlight_val = 0x08;
};

#endif
