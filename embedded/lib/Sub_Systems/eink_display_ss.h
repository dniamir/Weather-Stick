# include <arduino_i2c.h>

#include <GxEPD.h>
// #include <BitmapGraphics.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include <GxEPD2_BW.h> // including both doesn't use more code or ram
// #include "epd/GxEPD2_213_B72.h"
// #include "epd/GxEPD2_290_BS.h"
// #include "epd/GxEPD2_290.h"
// #include "epd/GxEPD2_290_I6FD.h"
// #include "epd/GxEPD2_290_M06.h"
// #include "epd/GxEPD2_290_T5.h"
#include "epd/GxEPD2_213_B72.h"

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

    GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display = GxEPD2_213_B72(5, 22, 21, 4); // GxEPD2_213_B72
    void configure_system();
    void set_image();
    void reset_screen();
    void write_text(String text, uint16_t x, uint16_t y);

    void write_readings(uint16_t *co2_ppm, 
                        int16_t *temperature_10_degc, 
                        int16_t *humidity_10_per, 
                        uint16_t *light_ir,
                        uint16_t *light_vis,
                        int16_t *temperature_100_degc,
                        int32_t *temperature_100,
                        int32_t *pressure,
                        int32_t *humidity_1000,
                        int32_t *gas,
                        int32_t *iaq,
                        int16_t *level_percent,
                        int16_t *level_mah,
                        int16_t *batt_voltage);

};