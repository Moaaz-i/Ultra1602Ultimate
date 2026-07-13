#include "Ultra1602Ultimate.h"

Ultra1602Ultimate::Ultra1602Ultimate(uint8_t rs, uint8_t en, uint8_t d4,
                                     uint8_t d5, uint8_t d6, uint8_t d7,
                                     uint8_t bl_pin) {
  _rs_pin = rs;
  _en_pin = en;
  _bl_pin = bl_pin;
  _data_pins[0] = d4;
  _data_pins[1] = d5;
  _data_pins[2] = d6;
  _data_pins[3] = d7;
  _isI2C = false;
  _bl_brightness = 255;
  _lastUpdateDuration = 0;
  _backlight_val = 0x00;
  _curr_col = 0;
  _curr_row = 0;
  _cols = 0;
  _rows = 0;

  for (uint8_t i = 0; i < 4; i++)
    _blinkActive[i] = false;
  for (uint8_t i = 0; i < 2; i++) {
    _marqueeActive[i] = false;
    _menuActive[i] = false;
    _fadeInActive[i] = false;
  }
  memset(_iconCacheValue, 0xFF, sizeof(_iconCacheValue));
}

Ultra1602Ultimate::Ultra1602Ultimate(uint8_t lcd_addr, uint8_t bl_pin) {
  _addr = lcd_addr;
  _bl_pin = bl_pin;
  _isI2C = true;
  _backlight_val = 0x08;
  _bl_brightness = 255;
  _lastUpdateDuration = 0;
  _curr_col = 0;
  _curr_row = 0;
  _cols = 0;
  _rows = 0;

  for (uint8_t i = 0; i < 4; i++)
    _blinkActive[i] = false;
  for (uint8_t i = 0; i < 2; i++) {
    _marqueeActive[i] = false;
    _menuActive[i] = false;
    _fadeInActive[i] = false;
  }
  memset(_iconCacheValue, 0xFF, sizeof(_iconCacheValue));
}

void Ultra1602Ultimate::begin(uint8_t cols, uint8_t rows) {
  _cols = (cols > 20) ? 20 : cols;
  _rows = (rows > 4) ? 4 : rows;
  _curr_col = 0;
  _curr_row = 0;

  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 20; c++)
      _shadowBuffer[r][c] = ' ';
  }

  uint32_t ts = millis();
  if (_isI2C) {
    Wire.begin();
    Wire.setClock(400000);
    while (millis() - ts < 50)
      ;
  } else {
    pinMode(_rs_pin, OUTPUT);
    pinMode(_en_pin, OUTPUT);
    for (uint8_t i = 0; i < 4; i++)
      pinMode(_data_pins[i], OUTPUT);
    if (_bl_pin != 255)
      pinMode(_bl_pin, OUTPUT);
    digitalWrite(_rs_pin, LOW);
    digitalWrite(_en_pin, LOW);
    while (millis() - ts < 50)
      ;
  }

  uint32_t tus;
  write4Bits(0x03);
  tus = micros();
  while (micros() - tus < 4500)
    ;
  write4Bits(0x03);
  tus = micros();
  while (micros() - tus < 4500)
    ;
  write4Bits(0x03);
  tus = micros();
  while (micros() - tus < 150)
    ;
  write4Bits(0x02);

  sendCommand(0x28);
  sendCommand(0x0C);
  sendCommand(0x06);
  hardClear();

  if (_isI2C)
    setBacklightBrightness(_bl_brightness);
  else if (_bl_pin != 255)
    analogWrite(_bl_pin, _bl_brightness);
}

size_t Ultra1602Ultimate::write(uint8_t character) {
  uint32_t startTime = micros();
  if (_curr_col >= _cols) {
    _curr_col = 0;
    _curr_row++;
    if (_curr_row >= _rows)
      _curr_row = 0;
    setCursor(_curr_col, _curr_row);
  }
  if (_shadowBuffer[_curr_row][_curr_col] != character) {
    _shadowBuffer[_curr_row][_curr_col] = character;
    sendData(character);
    _curr_col++;
  } else {
    _curr_col++;
    setCursor(_curr_col, _curr_row);
  }
  _lastUpdateDuration = micros() - startTime;
  return 1;
}

void Ultra1602Ultimate::setCursor(uint8_t col, uint8_t row) {
  static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  if (row >= _rows)
    row = _rows - 1;
  if (col >= _cols)
    col = _cols - 1;
  _curr_col = col;
  _curr_row = row;
  sendCommand(0x80 | (_curr_col + row_offsets[_curr_row]));
}

void Ultra1602Ultimate::updateAsync() {
  uint32_t currentTime = millis();

  for (uint8_t i = 0; i < 2; i++) {
    if (_marqueeActive[i] &&
        (currentTime - _marqueeLastTime[i] >= _marqueeInterval[i])) {
      _marqueeLastTime[i] = currentTime;
      uint8_t len = _isMarqueeFlash[i] ? strlen_P((PGM_P)_marqueeFlashText[i])
                                       : strlen(_marqueeText[i]);
      setCursor(0, _marqueeRow[i]);
      for (uint8_t j = 0; j < _cols; j++) {
        int idx = _marqueeStep[i] - _cols + j;
        if (idx >= 0 && idx < len) {
          write(_isMarqueeFlash[i]
                    ? pgm_read_byte((PGM_P)_marqueeFlashText[i] + idx)
                    : _marqueeText[i][idx]);
        } else {
          write(' ');
        }
      }
      _marqueeStep[i]++;
      if (_marqueeStep[i] >= (len + _cols))
        _marqueeStep[i] = 0;
    }
  }

  for (uint8_t i = 0; i < 4; i++) {
    if (_blinkActive[i] && _blinkText[i] != NULL &&
        (currentTime - _blinkLastTime[i] >= _blinkInterval[i])) {
      _blinkLastTime[i] = currentTime;
      _blinkState[i] = !_blinkState[i];
      if (_blinkState[i]) {
        printAdvanced(_blinkText[i], _blinkCol[i], _blinkRow[i]);
      } else {
        clearRange(_blinkRow[i], _blinkCol[i],
                   _blinkCol[i] + strlen(_blinkText[i]) - 1);
      }
    }
  }

  for (uint8_t i = 0; i < 2; i++) {
    if (_fadeInActive[i] && _fadeInText[i] != NULL &&
        (currentTime - _fadeInLastTime[i] >= _fadeInInterval[i])) {
      _fadeInLastTime[i] = currentTime;
      if (_fadeInIndex[i] < strlen(_fadeInText[i]) &&
          (_fadeInCol[i] + _fadeInIndex[i]) < _cols) {
        setCursor(_fadeInCol[i] + _fadeInIndex[i], _fadeInRow[i]);
        write(_fadeInText[i][_fadeInIndex[i]]);
        _fadeInIndex[i]++;
      } else {
        _fadeInActive[i] = false;
      }
    }
  }

  for (uint8_t i = 0; i < 2; i++) {
    if (_menuActive[i] && _menuItems[i] != NULL &&
        (currentTime - _menuLastTime[i] >= _menuInterval[i])) {
      _menuLastTime[i] = currentTime;
      clearRow(_menuRow[i]);
      TextProps props;
      props.align = 1;
      printAdvanced(_menuItems[i][_menuCurrentItem[i]], 0, _menuRow[i], props);
      _menuCurrentItem[i] = (_menuCurrentItem[i] + 1) % _menuItemCount[i];
    }
  }
}

void Ultra1602Ultimate::printAdvanced(const __FlashStringHelper *ifsh,
                                      uint8_t col, uint8_t row,
                                      TextProps props) {
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  size_t len = strlen_P(p);
  uint8_t startCol = col;
  if (props.align == 1)
    startCol = (len >= _cols) ? 0 : (_cols - len) >> 1;
  else if (props.align == 2)
    startCol = (len >= _cols) ? 0 : _cols - len;
  setCursor(startCol, row);
  for (size_t i = 0; i < len; i++) {
    if ((startCol + i) >= _cols)
      break;
    write(pgm_read_byte(p + i));
  }
}

void Ultra1602Ultimate::printAdvanced(const char *str, uint8_t col, uint8_t row,
                                      TextProps props) {
  uint8_t len = strlen(str);
  uint8_t startCol = col;
  if (props.align == 1)
    startCol = (len >= _cols) ? 0 : (_cols - len) >> 1;
  else if (props.align == 2)
    startCol = (len >= _cols) ? 0 : _cols - len;
  setCursor(startCol, row);
  for (uint8_t i = 0; i < len; i++) {
    if ((startCol + i) >= _cols)
      break;
    write(str[i]);
  }
}

void Ultra1602Ultimate::printAdvanced(int num, uint8_t col, uint8_t row,
                                      TextProps props) {
  char buf[12];
  itoa(num, buf, 10);
  printAdvanced(buf, col, row, props);
}

void Ultra1602Ultimate::printFloat(float num, uint8_t decimals, uint8_t col,
                                   uint8_t row, TextProps props) {
  char buf[16];
  dtostrf(num, 4, decimals, buf);
  printAdvanced(buf, col, row, props);
}

void Ultra1602Ultimate::startMarqueeAsync(const char *text, uint8_t row,
                                          uint16_t interval_ms, uint8_t slot) {
  if (slot >= 2)
    return;
  _marqueeText[slot] = text;
  _marqueeRow[slot] = row;
  _marqueeInterval[slot] = interval_ms;
  _marqueeStep[slot] = 0;
  _marqueeLastTime[slot] = millis();
  _marqueeActive[slot] = true;
  _isMarqueeFlash[slot] = false;
}

void Ultra1602Ultimate::startMarqueeAsync(const __FlashStringHelper *text,
                                          uint8_t row, uint16_t interval_ms,
                                          uint8_t slot) {
  if (slot >= 2)
    return;
  _marqueeFlashText[slot] = text;
  _marqueeRow[slot] = row;
  _marqueeInterval[slot] = interval_ms;
  _marqueeStep[slot] = 0;
  _marqueeLastTime[slot] = millis();
  _marqueeActive[slot] = true;
  _isMarqueeFlash[slot] = true;
}

void Ultra1602Ultimate::stopMarqueeAsync(uint8_t slot) {
  if (slot < 2) {
    _marqueeActive[slot] = false;
    clearRow(_marqueeRow[slot]);
  }
}

void Ultra1602Ultimate::startBlinkingTextAsync(const char *text, uint8_t col,
                                               uint8_t row,
                                               uint16_t blink_interval_ms,
                                               uint8_t slot) {
  if (slot >= 4)
    return;
  _blinkText[slot] = text;
  _blinkCol[slot] = col;
  _blinkRow[slot] = row;
  _blinkInterval[slot] = blink_interval_ms;
  _blinkState[slot] = true;
  _blinkLastTime[slot] = millis();
  _blinkActive[slot] = true;
  printAdvanced(_blinkText[slot], _blinkCol[slot], _blinkRow[slot]);
}

void Ultra1602Ultimate::stopBlinkingTextAsync(uint8_t slot) {
  if (slot < 4)
    _blinkActive[slot] = false;
}

void Ultra1602Ultimate::startFadeInAsync(const char *text, uint8_t col,
                                         uint8_t row, uint16_t char_interval_ms,
                                         uint8_t slot) {
  if (slot >= 2)
    return;
  _fadeInText[slot] = text;
  _fadeInCol[slot] = col;
  _fadeInRow[slot] = row;
  _fadeInInterval[slot] = char_interval_ms;
  _fadeInIndex[slot] = 0;
  _fadeInLastTime[slot] = millis();
  _fadeInActive[slot] = true;
}

void Ultra1602Ultimate::startMenuAsync(const char **items, uint8_t itemCount,
                                       uint16_t auto_scroll_ms, uint8_t row,
                                       uint8_t slot) {
  if (slot >= 2)
    return;
  _menuItems[slot] = items;
  _menuItemCount[slot] = itemCount;
  _menuInterval[slot] = auto_scroll_ms;
  _menuRow[slot] = row;
  _menuCurrentItem[slot] = 0;
  _menuLastTime[slot] = millis();
  _menuActive[slot] = true;
}

void Ultra1602Ultimate::stopMenuAsync(uint8_t slot) {
  if (slot < 2) {
    _menuActive[slot] = false;
    clearRow(_menuRow[slot]);
  }
}

void Ultra1602Ultimate::drawPreciseProgressBar(uint8_t row, uint8_t percentage,
                                               uint8_t startCol, uint8_t endCol,
                                               bool bordered) {
  if (percentage > 100)
    percentage = 100;
  if (bordered) {
    setCursor(startCol, row);
    write('[');
    setCursor(endCol, row);
    write(']');
    startCol++;
    endCol--;
  }
  uint8_t barWidth = endCol - startCol + 1;
  uint16_t totalPixels = (percentage * (barWidth * 5)) / 100;
  uint8_t fullBars = totalPixels / 5;
  uint8_t remainder = totalPixels % 5;
  if (remainder > 0) {
    uint8_t partial_char[8] = {0};
    uint8_t mask = 0x10;
    for (uint8_t p = 1; p < remainder; p++)
      mask |= (0x10 >> p);
    for (uint8_t i = 0; i < 8; i++)
      partial_char[i] = mask;
    createCustomChar(0, partial_char);
  }
  uint8_t full_block[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
  createCustomChar(5, full_block);
  for (uint8_t i = 0; i < barWidth; i++) {
    setCursor(startCol + i, row);
    if (i < fullBars)
      write(5);
    else if (i == fullBars && remainder > 0)
      write(0);
    else
      write(' ');
  }
}

void Ultra1602Ultimate::drawBarGraphHorizontal(uint8_t row, uint8_t val1,
                                               uint8_t val2, uint8_t maxVal) {
  uint8_t map1 = (val1 * (_cols >> 1)) / maxVal;
  uint8_t map2 = (val2 * (_cols >> 1)) / maxVal;
  clearRow(row);
  setCursor(0, row);
  for (uint8_t i = 0; i < map1; i++)
    write('=');
  setCursor(_cols >> 1, row);
  for (uint8_t i = 0; i < map2; i++)
    write('#');
}

void Ultra1602Ultimate::drawBatteryIcon(uint8_t col, uint8_t row,
                                        uint8_t percentage, uint8_t location) {
  uint8_t bat[8] = {0x0E, 0x1B, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F};
  if (percentage > 20)
    bat[6] = 0x1F;
  if (percentage > 40)
    bat[5] = 0x1F;
  if (percentage > 60)
    bat[4] = 0x1F;
  if (percentage > 80)
    bat[3] = 0x1F;
  if (percentage >= 95)
    bat[2] = 0x1F;
  if (_iconCacheValue[location] != percentage) {
    _iconCacheValue[location] = percentage;
    createCustomChar(location, bat);
  }
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::drawWifiIcon(uint8_t col, uint8_t row, uint8_t level,
                                     uint8_t location) {
  uint8_t wifi[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  if (level >= 1)
    wifi[7] = 0x04;
  if (level >= 2) {
    wifi[5] = 0x0E;
    wifi[6] = 0x0A;
  }
  if (level >= 3) {
    wifi[3] = 0x1F;
    wifi[4] = 0x11;
  }
  createCustomChar(location, wifi);
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::drawStorageGauge(uint8_t col, uint8_t row,
                                         uint8_t usedPercentage,
                                         uint8_t location) {
  uint8_t gauge[8] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F};
  uint8_t fillLines = (usedPercentage * 6) / 100;
  for (uint8_t i = 1; i <= fillLines; i++)
    gauge[7 - i] = 0x1F;
  createCustomChar(location, gauge);
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::drawSpinner(uint8_t col, uint8_t row, uint8_t frame,
                                    bool clockwise) {
  static const char frames[] = {'|', '/', '-', '\\'};
  setCursor(col, row);
  write(frames[(clockwise ? frame : (4 - frame)) & 3]);
}

void Ultra1602Ultimate::drawHeartbeat(uint8_t col, uint8_t row, bool state,
                                      uint8_t location) {
  static const uint8_t heart[8] = {0x00, 0x0A, 0x1F, 0x1F,
                                   0x1F, 0x0E, 0x04, 0x00};
  static const uint8_t pulse[8] = {0x00, 0x0A, 0x0A, 0x00,
                                   0x04, 0x00, 0x00, 0x00};
  createCustomChar(location, state ? heart : pulse);
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::clearRange(uint8_t row, uint8_t startCol,
                                   uint8_t endCol) {
  setCursor(startCol, row);
  for (uint8_t c = startCol; c <= endCol && c < _cols; c++)
    write(' ');
}

void Ultra1602Ultimate::clearRow(uint8_t row) { clearRange(row, 0, _cols - 1); }

void Ultra1602Ultimate::clear() {
  for (uint8_t r = 0; r < _rows; r++)
    clearRow(r);
  setCursor(0, 0);
}

void Ultra1602Ultimate::hardClear() {
  sendCommand(0x01);
  uint32_t tus = micros();
  while (micros() - tus < 2000)
    ;
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 20; c++)
      _shadowBuffer[r][c] = ' ';
  }
  _curr_col = 0;
  _curr_row = 0;
}

void Ultra1602Ultimate::savePower(bool enable) {
  if (enable) {
    sendCommand(0x08);
    setBacklightBrightness(0);
  } else {
    sendCommand(0x0C);
    setBacklightBrightness(255);
  }
}

void Ultra1602Ultimate::setBacklightBrightness(uint8_t brightness) {
  _bl_brightness = brightness;
  if (_isI2C) {
    _backlight_val = (brightness > 0) ? 0x08 : 0x00;
    Wire.beginTransmission(_addr);
    Wire.write(_backlight_val);
    Wire.endTransmission();
  } else if (_bl_pin != 255) {
    analogWrite(_bl_pin, _bl_brightness);
  }
}

void Ultra1602Ultimate::createCustomChar(uint8_t location,
                                         const uint8_t charmap[]) {
  location &= 0x07;
  sendCommand(0x40 | (location << 3));
  if (_isI2C) {
    for (uint8_t i = 0; i < 8; i++)
      sendData(charmap[i]);
  } else {
    for (uint8_t i = 0; i < 8; i++)
      sendData(charmap[i]);
  }
}

void Ultra1602Ultimate::pulseEnable() {
  uint32_t tus;
  digitalWrite(_en_pin, LOW);
  tus = micros();
  while (micros() - tus < 1)
    ;
  digitalWrite(_en_pin, HIGH);
  tus = micros();
  while (micros() - tus < 1)
    ;
  digitalWrite(_en_pin, LOW);
  tus = micros();
  while (micros() - tus < 100)
    ;
}

void Ultra1602Ultimate::write4Bits(uint8_t value) {
  uint32_t tus;
  if (_isI2C) {
    uint8_t pin_val = value | _backlight_val;
    Wire.beginTransmission(_addr);
    Wire.write(pin_val | 0x04);
    Wire.endTransmission(true);
    tus = micros();
    while (micros() - tus < 1)
      ;
    Wire.beginTransmission(_addr);
    Wire.write(pin_val & ~0x04);
    Wire.endTransmission(true);
    tus = micros();
    while (micros() - tus < 40)
      ;
  } else {
    for (uint8_t i = 0; i < 4; i++)
      digitalWrite(_data_pins[i], (value >> i) & 0x01);
    pulseEnable();
  }
}

void Ultra1602Ultimate::sendCommand(uint8_t cmd) {
  if (_isI2C) {
    uint8_t hi = cmd & 0xF0;
    uint8_t lo = (cmd << 4) & 0xF0;
    write4Bits(hi);
    write4Bits(lo);
  } else {
    digitalWrite(_rs_pin, LOW);
    write4Bits(cmd >> 4);
    write4Bits(cmd & 0x0F);
  }
}

void Ultra1602Ultimate::sendData(uint8_t data) {
  if (_isI2C) {
    uint8_t hi = (data & 0xF0) | 0x01;
    uint8_t lo = ((data << 4) & 0xF0) | 0x01;
    write4Bits(hi);
    write4Bits(lo);
  } else {
    digitalWrite(_rs_pin, HIGH);
    write4Bits(data >> 4);
    write4Bits(data & 0x0F);
  }
}

void Ultra1602Ultimate::printRightToLeft(const char *str, uint8_t col,
                                         uint8_t row) {
  uint8_t len = strlen(str);
  int current_c = col;
  for (uint8_t i = 0; i < len; i++) {
    if (current_c >= 0) {
      setCursor(current_c--, row);
      write(str[i]);
    }
  }
}

void Ultra1602Ultimate::fillScreen(char ch) {
  for (uint8_t r = 0; r < _rows; r++) {
    setCursor(0, r);
    for (uint8_t c = 0; c < _cols; c++)
      write(ch);
  }
}

void Ultra1602Ultimate::drawLockIcon(uint8_t col, uint8_t row, bool locked,
                                     uint8_t location) {
  static const uint8_t lock[8] = {0x0E, 0x11, 0x11, 0x1F,
                                  0x1F, 0x1F, 0x1F, 0x1F};
  static const uint8_t unlock[8] = {0x0E, 0x01, 0x01, 0x1F,
                                    0x1F, 0x1F, 0x1F, 0x1F};
  createCustomChar(location, locked ? lock : unlock);
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::drawSignalIcon(uint8_t col, uint8_t row,
                                       uint8_t strength, uint8_t location) {
  uint8_t sig[8] = {0};
  if (strength >= 1)
    sig[7] = 0x01;
  if (strength >= 2)
    sig[6] = 0x03;
  if (strength >= 3)
    sig[5] = 0x07;
  if (strength >= 4)
    sig[4] = 0x0F;
  createCustomChar(location, sig);
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::drawShapeByPattern(uint8_t col, uint8_t row,
                                           const char *pattern,
                                           uint8_t cg_location) {
  uint8_t customChar[8] = {0};
  for (uint8_t r = 0; r < 8; r++) {
    uint8_t rowByte = 0;
    for (uint8_t c = 0; c < 5; c++) {
      int index = (r * 5) + c;
      if (pattern[index] == '\0') {
        createCustomChar(cg_location, customChar);
        setCursor(col, row);
        write(cg_location);
        return;
      }
      if (pattern[index] != ' ')
        rowByte |= (0x10 >> c);
    }
    customChar[r] = rowByte;
  }
  createCustomChar(cg_location, customChar);
  setCursor(col, row);
  write(cg_location);
}
