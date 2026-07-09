#ifndef Ultra1602Ultimate_h
#define Ultra1602Ultimate_h

#include <Arduino.h>
#include <Print.h>

struct TextProps {
  uint8_t align = 0;
  uint16_t speed = 50;
};

class Ultra1602Ultimate : public Print {
public:
  Ultra1602Ultimate(uint8_t rs, uint8_t en, uint8_t d4, uint8_t d5, uint8_t d6,
                    uint8_t d7, uint8_t bl_pin = 255);

  void begin(uint8_t cols, uint8_t rows);
  void clear();
  void hardClear();
  void setCursor(uint8_t col, uint8_t row);
  virtual size_t write(uint8_t character) override;

  void updateAsync();

  void printAdvanced(const char *str, uint8_t col, uint8_t row,
                     TextProps props = TextProps());
  void printAdvanced(int num, uint8_t col, uint8_t row,
                     TextProps props = TextProps());
  void printRightToLeft(const char *str, uint8_t col, uint8_t row);

  void startMarqueeAsync(const char *text, uint8_t row, uint16_t interval_ms);
  void stopMarqueeAsync();

  void startBlinkingTextAsync(const char *text, uint8_t col, uint8_t row,
                              uint16_t blink_interval_ms);
  void stopBlinkingTextAsync();

  void drawPreciseProgressBar(uint8_t row, uint8_t percentage,
                              uint8_t startCol = 0, uint8_t endCol = 15);
  void drawVerticalBar(uint8_t col, uint8_t row, uint8_t height,
                       uint8_t location = 0);
  void drawBatteryIcon(uint8_t col, uint8_t row, uint8_t percentage,
                       uint8_t location = 1);
  void drawSignalIcon(uint8_t col, uint8_t row, uint8_t strength,
                      uint8_t location = 2);
  void drawSpinner(uint8_t col, uint8_t row, uint8_t frame);
  void drawHeartbeat(uint8_t col, uint8_t row, bool state,
                     uint8_t location = 3);
  void drawWifiIcon(uint8_t col, uint8_t row, uint8_t level,
                    uint8_t location = 4);
  void drawLockIcon(uint8_t col, uint8_t row, bool locked,
                    uint8_t location = 4);
  void drawShapeByPattern(uint8_t col, uint8_t row, const char *pattern,
                          uint8_t cg_location = 7);

  void invertRow(uint8_t row);
  void savePower(bool on);
  void setBacklightBrightness(uint8_t brightness);
  void fillScreen(char ch);
  void clearRow(uint8_t row);
  void createCustomChar(uint8_t location, const uint8_t charmap[]);

private:
  void sendCommand(uint8_t cmd);
  void sendData(uint8_t data);
  void write4Bits(uint8_t value);
  void pulseEnable();

  uint8_t _rs_pin, _en_pin, _bl_pin;
  uint8_t _data_pins[4];
  uint8_t _cols, _rows;
  uint8_t _curr_col, _curr_row;
  uint8_t _shadowBuffer[4][20];

  uint32_t _marqueeLastTime;
  uint16_t _marqueeInterval;
  const char *_marqueeText;
  uint8_t _marqueeRow;
  uint8_t _marqueeStep;
  bool _marqueeActive;

  uint32_t _blinkLastTime;
  uint16_t _blinkInterval;
  const char *_blinkText;
  uint8_t _blinkCol, _blinkRow;
  bool _blinkState;
  bool _blinkActive;
};

#endif
