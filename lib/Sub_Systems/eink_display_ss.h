# include <arduino_i2c.h>

#include <GxEPD.h>
#include <BitmapGraphics.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include <GxEPD2_BW.h> // including both doesn't use more code or ram
// #include "epd/GxEPD2_213_B72.h"
#include "epd/GxEPD2_290_BS.h"

// E-Ink Wiring
// Busy: D4
// RST: D21
// DC: D22
// CS: D5
// CLK: D18
// DIN: D23
//  GND: GND
// 3.3V: 3.3V

class eink_display_ss {

  public:
    // Constructor
    eink_display_ss();

    // GxIO_Class io = GxIO_Class(SPI, 5, 22, 21);
    // GxEPD_Class display = GxEPD_Class(eink_display_ss::io, 16, 4);

    GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display = GxEPD2_290_BS(5, 22, 21, 4); // GxEPD2_290_BS
    void configure_system();
    void set_image();
    void reset_screen();
    void write_text(String text, uint16_t x, uint16_t y);

};