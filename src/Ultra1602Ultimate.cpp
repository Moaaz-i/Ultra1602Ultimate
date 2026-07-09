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

  _marqueeActive = false;
  _marqueeText = NULL;
  _blinkActive = false;
  _blinkText = NULL;

  _curr_col = 0;
  _curr_row = 0;
  _cols = 0;
  _rows = 0;
}

void Ultra1602Ultimate::begin(uint8_t cols, uint8_t rows) {
  _cols = (cols > 20) ? 20 : cols;
  _rows = (rows > 4) ? 4 : rows;
  _curr_col = 0;
  _curr_row = 0;

  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 20; c++)
      _shadowBuffer[r][c] = 0x00;
  }

  pinMode(_rs_pin, OUTPUT);
  pinMode(_en_pin, OUTPUT);
  for (uint8_t i = 0; i < 4; i++)
    pinMode(_data_pins[i], OUTPUT);

  if (_bl_pin != 255) {
    pinMode(_bl_pin, OUTPUT);
  }

  digitalWrite(_rs_pin, LOW);
  digitalWrite(_en_pin, LOW);
  delay(50);

  write4Bits(0x03);
  delayMicroseconds(4500);
  write4Bits(0x03);
  delayMicroseconds(4500);
  write4Bits(0x03);
  delayMicroseconds(150);
  write4Bits(0x02);

  sendCommand(0x28);
  sendCommand(0x0C);
  sendCommand(0x06);

  hardClear();

  if (_bl_pin != 255) {
    analogWrite(_bl_pin, 255);
  }
}

void Ultra1602Ultimate::updateAsync() {
  uint32_t currentTime = millis();

  if (_marqueeActive && _marqueeText != NULL) {
    if (currentTime - _marqueeLastTime >= _marqueeInterval) {
      _marqueeLastTime = currentTime;
      uint8_t len = strlen(_marqueeText);

      setCursor(0, _marqueeRow);
      for (uint8_t j = 0; j < _cols; j++) {
        int idx = _marqueeStep - _cols + j;
        if (idx >= 0 && idx < len)
          write(_marqueeText[idx]);
        else
          write(' ');
      }

      _marqueeStep++;
      if (_marqueeStep >= (len + _cols)) {
        _marqueeStep = 0;
      }
    }
  }

  if (_blinkActive && _blinkText != NULL) {
    if (currentTime - _blinkLastTime >= _blinkInterval) {
      _blinkLastTime = currentTime;
      _blinkState = !_blinkState;

      if (_blinkState) {
        printAdvanced(_blinkText, _blinkCol, _blinkRow);
      } else {
        setCursor(_blinkCol, _blinkRow);
        uint8_t len = strlen(_blinkText);
        for (uint8_t i = 0; i < len; i++)
          write(' ');
      }
    }
  }
}

void Ultra1602Ultimate::startMarqueeAsync(const char *text, uint8_t row,
                                          uint16_t interval_ms) {
  _marqueeText = text;
  _marqueeRow = row;
  _marqueeInterval = interval_ms;
  _marqueeStep = 0;
  _marqueeLastTime = millis();
  _marqueeActive = true;
}

void Ultra1602Ultimate::stopMarqueeAsync() {
  _marqueeActive = false;
  clearRow(_marqueeRow);
}

void Ultra1602Ultimate::startBlinkingTextAsync(const char *text, uint8_t col,
                                               uint8_t row,
                                               uint16_t blink_interval_ms) {
  _blinkText = text;
  _blinkCol = col;
  _blinkRow = row;
  _blinkInterval = blink_interval_ms;
  _blinkState = true;
  _blinkLastTime = millis();
  _blinkActive = true;
  printAdvanced(_blinkText, _blinkCol, _blinkRow);
}

void Ultra1602Ultimate::stopBlinkingTextAsync() {
  _blinkActive = false;
  if (_blinkText != NULL) {
    setCursor(_blinkCol, _blinkRow);
    uint8_t len = strlen(_blinkText);
    for (uint8_t i = 0; i < len; i++)
      write(' ');
  }
}

size_t Ultra1602Ultimate::write(uint8_t character) {
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
  }
  _curr_col++;
  return 1;
}

void Ultra1602Ultimate::clear() {
  for (uint8_t r = 0; r < _rows; r++)
    clearRow(r);
  setCursor(0, 0);
}

void Ultra1602Ultimate::hardClear() {
  sendCommand(0x01);
  delayMicroseconds(2000);
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 20; c++)
      _shadowBuffer[r][c] = ' ';
  }
  _curr_col = 0;
  _curr_row = 0;
}

void Ultra1602Ultimate::setCursor(uint8_t col, uint8_t row) {
  uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  if (row >= _rows)
    row = _rows - 1;
  if (col >= _cols)
    col = _cols - 1;
  _curr_col = col;
  _curr_row = row;
  sendCommand(0x80 | (_curr_col + row_offsets[_curr_row]));
}

void Ultra1602Ultimate::printAdvanced(const char *str, uint8_t col, uint8_t row,
                                      TextProps props) {
  uint8_t len = strlen(str);
  uint8_t startCol = col;

  if (props.align == 1)
    startCol = (len >= _cols) ? 0 : (_cols - len) / 2;
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

void Ultra1602Ultimate::drawPreciseProgressBar(uint8_t row, uint8_t percentage,
                                               uint8_t startCol,
                                               uint8_t endCol) {
  if (percentage > 100)
    percentage = 100;
  if (endCol >= _cols)
    endCol = _cols - 1;

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

void Ultra1602Ultimate::drawBatteryIcon(uint8_t col, uint8_t row,
                                        uint8_t percentage, uint8_t location) {
  uint8_t bat[8] = {0x0E, 0x1B, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F};

  if (percentage > 20) {
    bat[6] = 0x1F;
  }
  if (percentage > 40) {
    bat[5] = 0x1F;
  }
  if (percentage > 60) {
    bat[4] = 0x1F;
  }
  if (percentage > 80) {
    bat[3] = 0x1F;
  }
  if (percentage >= 95) {
    bat[2] = 0x1F;
  }

  createCustomChar(location, bat);
  setCursor(col, row);
  write(location);
}

void Ultra1602Ultimate::drawWifiIcon(uint8_t col, uint8_t row, uint8_t level,
                                     uint8_t location) {
  uint8_t wifi[8] = {0x1F, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00};
  createCustomChar(location, wifi);
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
      if (pattern[index] != ' ') {
        rowByte |= (0x10 >> c);
      }
    }
    customChar[r] = rowByte;
  }

  createCustomChar(cg_location, customChar);
  setCursor(col, row);
  write(cg_location);
}

void Ultra1602Ultimate::createCustomChar(uint8_t location,
                                         const uint8_t charmap[]) {
  location &= 0x07;
  sendCommand(0x40 | (location << 3));
  for (uint8_t i = 0; i < 8; i++)
    sendData(charmap[i]);
}

void Ultra1602Ultimate::clearRow(uint8_t row) {
  setCursor(0, row);
  for (uint8_t c = 0; c < _cols; c++) {
    write(' ');
  }
}

void Ultra1602Ultimate::setBacklightBrightness(uint8_t brightness) {
  if (_bl_pin != 255)
    analogWrite(_bl_pin, brightness);
}

void Ultra1602Ultimate::pulseEnable() {
  digitalWrite(_en_pin, LOW);
  delayMicroseconds(1);
  digitalWrite(_en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(_en_pin, LOW);
  delayMicroseconds(100);
}

void Ultra1602Ultimate::write4Bits(uint8_t value) {
  for (uint8_t i = 0; i < 4; i++)
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  pulseEnable();
}

void Ultra1602Ultimate::sendCommand(uint8_t cmd) {
  digitalWrite(_rs_pin, LOW);
  write4Bits(cmd >> 4);
  write4Bits(cmd & 0x0F);
}

void Ultra1602Ultimate::sendData(uint8_t data) {
  digitalWrite(_rs_pin, HIGH);
  write4Bits(data >> 4);
  write4Bits(data & 0x0F);
}
