// class GxGDEW0371W7 : Display class for GDEW0371W7 e-Paper from Dalian Good Display Co., Ltd.: http://www.e-paper-display.com/products_detail/productId=401.html
//
// based on Demo Example from Good Display, available here: http://www.e-paper-display.com/download_detail/downloadsId=639.html
// Controller: IL0324 : http://www.e-paper-display.com/download_detail/downloadsId=763.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#include "GxGDEW0371W7.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

// Partial Update Delay, may have an influence on degradation
#define GxGDEW0371W7_PU_DELAY 100

GxGDEW0371W7::GxGDEW0371W7(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW0371W7_WIDTH, GxGDEW0371W7_HEIGHT), IO(io),
    _current_page(-1), _using_partial_mode(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEW0371W7::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW0371W7_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW0371W7_WIDTH - x - 1;
      y = GxGDEW0371W7_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW0371W7_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW0371W7_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW0371W7_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW0371W7_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW0371W7_WIDTH / 8;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}


void GxGDEW0371W7::init(uint32_t serial_diag_bitrate)
{
  if (serial_diag_bitrate > 0)
  {
    Serial.begin(serial_diag_bitrate);
    _diag_enabled = true;
  }
  IO.init();
  IO.setFrequency(4000000); // 4MHz
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
  _current_page = -1;
  _using_partial_mode = false;
}

void GxGDEW0371W7::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW0371W7::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_buffer)) ? ~_buffer[i] : 0xFF);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW0371W7::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW0371W7::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_flip_x;
  if (mode & bm_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true;
    _Init_PartialUpdate();
    //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
    { // leave both controller buffers equal
      IO.writeCommandTransaction(0x91); // partial in
      _setPartialRamArea(0, 0, GxGDEW0371W7_WIDTH - 1, GxGDEW0371W7_HEIGHT - 1);
      _writeCommand(0x13);
      for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
      {
        uint8_t data = 0xFF; // white is 0xFF on device
        if (i < size)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&bitmap[i]);
#else
          data = bitmap[i];
#endif
          if (mode & bm_invert) data = ~data;
        }
        _writeData(data);
      }
      _writeCommand(0x12); //display refresh
      _waitWhileBusy("drawBitmap");
      IO.writeCommandTransaction(0x92); // partial out
    } // leave both controller buffers equal
    delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data);
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    _sleep();
  }
}

void GxGDEW0371W7::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true;
    _Init_PartialUpdate();
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW0371W7_WIDTH - 1, GxGDEW0371W7_HEIGHT - 1);
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE * 2; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEW0371W7::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GxGDEW0371W7_WIDTH) return;
  if (y >= GxGDEW0371W7_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0371W7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0371W7_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _Init_PartialUpdate();
  //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
  { // leave both controller buffers equal
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.writeCommandTransaction(0x13);
    for (int16_t y1 = y; y1 <= ye; y1++)
    {
      for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0371W7_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
        IO.writeDataTransaction(~data); // white is 0xFF on device
      }
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("updateWindow");
    IO.writeCommandTransaction(0x92); // partial out
  } // leave both controller buffers equal
  delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
}

void GxGDEW0371W7::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEW0371W7_WIDTH - xs - w;
        xd = GxGDEW0371W7_WIDTH - xd - w;
        break;
      case 2:
        xs = GxGDEW0371W7_WIDTH - xs - w;
        ys = GxGDEW0371W7_HEIGHT - ys - h;
        xd = GxGDEW0371W7_WIDTH - xd - w;
        yd = GxGDEW0371W7_HEIGHT - yd - h;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEW0371W7_HEIGHT - ys  - h;
        yd = GxGDEW0371W7_HEIGHT - yd  - h;
        break;
    }
  }
  if (xs >= GxGDEW0371W7_WIDTH) return;
  if (ys >= GxGDEW0371W7_HEIGHT) return;
  if (xd >= GxGDEW0371W7_WIDTH) return;
  if (yd >= GxGDEW0371W7_HEIGHT) return;
  // the screen limits are the hard limits
  uint16_t xde = gx_uint16_min(GxGDEW0371W7_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GxGDEW0371W7_HEIGHT, yd + h) - 1;
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _Init_PartialUpdate();
  //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
  { // leave both controller buffers equal
    IO.writeCommandTransaction(0x91); // partial in
    // soft limits, must send as many bytes as set by _SetRamArea
    uint16_t yse = ys + yde - yd;
    uint16_t xss_d8 = xs / 8;
    uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde);
    IO.writeCommandTransaction(0x13);
    for (int16_t y1 = ys; y1 <= yse; y1++)
    {
      for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0371W7_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
        IO.writeDataTransaction(~data); // white is 0xFF on device
      }
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("updateToWindow");
    IO.writeCommandTransaction(0x92); // partial out
  } // leave both controller buffers equal
  delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
}

void GxGDEW0371W7::powerDown()
{
  _using_partial_mode = false; // force _wakeUp()
  _sleep();
}

uint16_t GxGDEW0371W7::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  IO.writeCommandTransaction(0x90); // partial window
  IO.writeDataTransaction(x % 256);
  IO.writeDataTransaction(xe % 256);
  IO.writeDataTransaction(y / 256);
  IO.writeDataTransaction(y % 256);
  IO.writeDataTransaction(ye / 256);
  IO.writeDataTransaction(ye % 256);
  IO.writeDataTransaction(0x00);
  return (7 + xe - x) / 8; // number of bytes to transfer per line
}

void GxGDEW0371W7::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEW0371W7::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEW0371W7::_writeDataPGM(const uint8_t* data, uint16_t n, int16_t fill_with_zeroes)
{
  for (uint16_t i = 0; i < n; i++)
  {
    IO.writeDataTransaction(pgm_read_byte(&*data++));
  }
  while (fill_with_zeroes > 0)
  {
    IO.writeDataTransaction(0x00);
    fill_with_zeroes--;
  }
}

void GxGDEW0371W7::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (digitalRead(_busy) == 1) break;
    delay(1);
    if (micros() - start > 10000000)
    {
      if (_diag_enabled) Serial.println("Busy Timeout!");
      break;
    }
  }
  if (comment)
  {
#if !defined(DISABLE_DIAGNOSTIC_OUTPUT)
    if (_diag_enabled)
    {
      unsigned long elapsed = micros() - start;
      Serial.print(comment);
      Serial.print(" : ");
      Serial.println(elapsed);
    }
#endif
  }
  (void) start;
}

void GxGDEW0371W7::_wakeUp()
{
  // reset required for wakeup
  if (_rst >= 0)
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }
  _writeCommand(0x01); // power setting
  _writeData (0x07);
  _writeData (0x07);   // VGH=20V,VGL=-20V
  _writeData (0x3f);   // VDH=15V
  _writeData (0x3f);   // VDL=-15V
  _writeCommand(0x06); // boost soft start
  _writeData (0x17);   // A
  _writeData (0x17);   // B
  _writeData (0x1d);   // C
  _writeCommand(0x04);
  _waitWhileBusy("_wakeUp Power On");
  _writeCommand(0x00); // panel setting
  _writeData(0x1f);    // LUT from OTP, 128x296
  _writeCommand(0x61); // resolution setting
  _writeData (GxGDEW0371W7_WIDTH);
  _writeData (GxGDEW0371W7_HEIGHT / 256);
  _writeData (GxGDEW0371W7_HEIGHT % 256);
  _writeCommand(0x82); // vcom_DC setting
  _writeData (0x1C);
  _writeCommand(0x50); // VCOM AND DATA INTERVAL SETTING
  _writeData(0x29);    // LUTKW, N2OCP: copy new to old
  _writeData(0x07);
  _Init_FullUpdate();
}

void GxGDEW0371W7::_sleep(void)
{
  _writeCommand(0x02);      //power off
  _waitWhileBusy("_sleep Power Off");
  if (_rst >= 0)
  {
    _writeCommand(0x07); // deep sleep
    _writeData (0xa5);
  }
}

// experimental partial screen update LUTs with balanced charge
// LUTs are filled with zeroes

#define T1 35 // charge balance pre-phase
#define T2  0 // optional extension
#define T3 35 // color change phase (b/w)
#define T4  0 // optional extension for one color

const unsigned char GxGDEW0371W7::lut_20_LUTC_partial[] PROGMEM =
{
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char GxGDEW0371W7::lut_21_LUTWW_partial[] PROGMEM =
{ // 10 w
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char GxGDEW0371W7::lut_22_LUTKW_partial[] PROGMEM =
{ // 10 w
  0x48, T1, T2, T3, T4, 1, // 01 00 10 00
  //0x5A, T1, T2, T3, T4, 1, // 01 01 10 10 more white
};

const unsigned char GxGDEW0371W7::lut_23_LUTWK_partial[] PROGMEM =
{ // 01 b
  0x84, T1, T2, T3, T4, 1, // 10 00 01 00
  //0xA5, T1, T2, T3, T4, 1, // 10 10 01 01 more black
};

const unsigned char GxGDEW0371W7::lut_24_LUTKK_partial[] PROGMEM =
{ // 01 b
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char GxGDEW0371W7::lut_25_LUTBD_partial[] PROGMEM =
{
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

void GxGDEW0371W7::_Init_FullUpdate(void)
{
  _writeCommand(0x00); // panel setting
  _writeData(0x1f);    // full update LUT from OTP
  _writeCommand(0x50); // VCOM AND DATA INTERVAL SETTING
  _writeData(0x29);    // LUTKW, N2OCP: copy new to old
  _writeData(0x07);
}

void GxGDEW0371W7::_Init_PartialUpdate(void)
{
  _writeCommand(0x00); //panel setting
  _writeData(0x3f); // partial update LUT from registers
  _writeCommand(0x50); // VCOM AND DATA INTERVAL SETTING
  _writeData(0x39);    // LUTBD, N2OCP: copy new to old
  _writeData(0x07);
  _writeCommand(0x20);
  _writeDataPGM(lut_20_LUTC_partial, sizeof(lut_20_LUTC_partial), 42 - sizeof(lut_20_LUTC_partial));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_LUTWW_partial, sizeof(lut_21_LUTWW_partial), 42 - sizeof(lut_21_LUTWW_partial));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_LUTKW_partial, sizeof(lut_22_LUTKW_partial), 42 - sizeof(lut_22_LUTKW_partial));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_LUTWK_partial, sizeof(lut_23_LUTWK_partial), 42 - sizeof(lut_23_LUTWK_partial));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_LUTKK_partial, sizeof(lut_24_LUTKK_partial), 42 - sizeof(lut_24_LUTKK_partial));
  _writeCommand(0x25);
  _writeDataPGM(lut_25_LUTBD_partial, sizeof(lut_25_LUTBD_partial), 42 - sizeof(lut_25_LUTBD_partial));
}

void GxGDEW0371W7::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW0371W7_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0371W7_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0371W7_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW0371W7::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW0371W7_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0371W7_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0371W7_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW0371W7::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW0371W7_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0371W7_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0371W7_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW0371W7::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW0371W7_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0371W7_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0371W7_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW0371W7::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEW0371W7_WIDTH - x - w;
      break;
    case 2:
      x = GxGDEW0371W7_WIDTH - x - w;
      y = GxGDEW0371W7_HEIGHT - y - h;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEW0371W7_HEIGHT - y - h;
      break;
  }
}

void GxGDEW0371W7::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0371W7_WIDTH) return;
  if (y >= GxGDEW0371W7_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0371W7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0371W7_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _using_partial_mode = true;
  _Init_PartialUpdate();
  //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
  { // leave both controller buffers equal
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.writeCommandTransaction(0x13);
    for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0371W7_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0371W7_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback();
        uint16_t ys = yds % GxGDEW0371W7_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0371W7_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            IO.writeDataTransaction(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
    IO.writeCommandTransaction(0x92); // partial out
  } // leave both controller buffers equal
  delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
}

void GxGDEW0371W7::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0371W7_WIDTH) return;
  if (y >= GxGDEW0371W7_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0371W7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0371W7_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _using_partial_mode = true;
  _Init_PartialUpdate();
  //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
  { // leave both controller buffers equal
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.writeCommandTransaction(0x13);
    for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0371W7_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0371W7_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback(p);
        uint16_t ys = yds % GxGDEW0371W7_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0371W7_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            IO.writeDataTransaction(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
    IO.writeCommandTransaction(0x92); // partial out
  } // leave both controller buffers equal
  delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
}

void GxGDEW0371W7::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0371W7_WIDTH) return;
  if (y >= GxGDEW0371W7_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0371W7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0371W7_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _using_partial_mode = true;
  _Init_PartialUpdate();
  //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
  { // leave both controller buffers equal
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.writeCommandTransaction(0x13);
    for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0371W7_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0371W7_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback(p);
        uint16_t ys = yds % GxGDEW0371W7_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0371W7_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            IO.writeDataTransaction(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
    IO.writeCommandTransaction(0x92); // partial out
  } // leave both controller buffers equal
  delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
}

void GxGDEW0371W7::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0371W7_WIDTH) return;
  if (y >= GxGDEW0371W7_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0371W7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0371W7_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _using_partial_mode = true;
  _Init_PartialUpdate();
  //for (uint16_t twice = 0; twice < 2; twice++) // done by N2OCP
  { // leave both controller buffers equal
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.writeCommandTransaction(0x13);
    for (_current_page = 0; _current_page < GxGDEW0371W7_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0371W7_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0371W7_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback(p1, p2);
        uint16_t ys = yds % GxGDEW0371W7_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0371W7_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            IO.writeDataTransaction(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
    IO.writeCommandTransaction(0x92); // partial out
  } // leave both controller buffers equal
  delay(GxGDEW0371W7_PU_DELAY); // don't stress this display
}

void GxGDEW0371W7::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0371W7_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // white is 0xFF on device
  }
  _writeCommand(0x13);
  for (uint32_t y = 0; y < GxGDEW0371W7_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW0371W7_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW0371W7_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW0371W7_WIDTH / 8 - 4) && (y > GxGDEW0371W7_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW0371W7_HEIGHT - 33)) data = 0x00;
      _writeData(data);
    }
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}
