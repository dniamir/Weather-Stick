# include <Arduino.h>
# include <eink_display_ss.h>
# include <logger.h>
// # include <gui_test.h>

const GFXfont* f = &FreeSansBold9pt7b;

eink_display_ss::eink_display_ss() {
    Serial.println("Test1");
}

void eink_display_ss::configure_system() {

    LOGGER::write_to_log("EINK", "CONFIGURE SYSTEM");
    eink_display_ss::display.init();
}

// void eink_display_ss::set_image() {

//     uint16_t height = 286;
//     uint16_t width = 118;
    
//     eink_display_ss::display.firstPage();
//     eink_display_ss::display.setRotation(2);

//     do
//     {
//         eink_display_ss::reset_screen();
//         eink_display_ss::display.drawBitmap(0, 0, gImage_gui, width, height, GxEPD_BLACK);
        
//     }
//     while (eink_display_ss::display.nextPage());

//     delay(2000);
// }

void eink_display_ss::reset_screen() {

    // eink_display_ss::display.clearScreen();
    eink_display_ss::display.firstPage
    ();
    do
    {
        // eink_display_ss::display.clearScreen();
        eink_display_ss::display.fillScreen(GxEPD_WHITE);
        
    }
    while (eink_display_ss::display.nextPage());
}

void eink_display_ss::write_text(String text, uint16_t x, uint16_t y) {

    eink_display_ss::display.setTextColor(GxEPD_BLACK);
    eink_display_ss::reset_screen();
    eink_display_ss::display.firstPage();

    eink_display_ss::display.setRotation(3);
    eink_display_ss::display.setFont(f);
    eink_display_ss::display.setCursor(x, y);

    do
    {
        eink_display_ss::display.println(text);
        
    }
    while (eink_display_ss::display.nextPage());

}

void eink_display_ss::write_readings(uint16_t *co2_ppm, 
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
                                     int16_t *level_10_percent,
                                     int16_t *level_10_mah,
                                     int16_t *batt_10_voltage,
                                     bool debug) {

    LOGGER::write_to_log("EINK", "WRITE TO SCREEN");
    
    if(debug) {
        Serial.println("E-INK Write these values on screen...");
        Serial.println(String(*co2_ppm));
        Serial.println(String((float)*temperature_10_degc / 10));
        Serial.println(String((float)*humidity_10_per / 10));
        Serial.println(String(*light_ir));
        Serial.println(String(*light_vis));
        Serial.println(String((float)*temperature_100_degc / 100));
        Serial.println(String((float)*temperature_100 / 100));
        Serial.println(String(*pressure));
        Serial.println(String((float)*humidity_1000 / 1000));
        Serial.println(String(*iaq));
        Serial.println(String((float)*level_10_percent / 10));
        Serial.println(String((float)*level_10_mah / 10));
        Serial.println(String((float)*batt_10_voltage / 10));
        Serial.println("");
    }
 
    eink_display_ss::display.setTextColor(GxEPD_BLACK);
    eink_display_ss::display.firstPage();
    eink_display_ss::display.setRotation(3);
    eink_display_ss::display.setFont(f);
    eink_display_ss::display.setCursor(0, 15);

    do
    {
        eink_display_ss::display.println("CO2: " + String(*co2_ppm) + ", T: " + String((float)*temperature_10_degc / 10) + ", H:" + String((float)*humidity_10_per / 10));
        eink_display_ss::display.println("IR: " + String(*light_ir) + ", VIS:" + String(*light_vis));
        eink_display_ss::display.println("Temperature: " + String((float)*temperature_100_degc / 100));
        eink_display_ss::display.println("T: " + String((float)*temperature_100 / 100) + ", P: " + String(*pressure) + ", H:" + String((float)*humidity_1000 / 1000));
        eink_display_ss::display.println("G: " + String(*gas) + ", IAQ: " + String(*iaq));     
        eink_display_ss::display.println("Battery: " + String((float)*level_10_percent / 10) + ", " + String((float)*level_10_mah / 10) + ", " + String((float)*batt_10_voltage / 10));  
    }
    while (eink_display_ss::display.nextPage());

}