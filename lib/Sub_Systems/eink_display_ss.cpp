# include <eink_display_ss.h>
# include <gui_test.h>

const GFXfont* f = &FreeSansBold9pt7b;

eink_display_ss::eink_display_ss() {
    Serial.println("Test1");
}

void eink_display_ss::configure_system() {

    eink_display_ss::display.init();
    // eink_display_ss::display.setRotation(1);
    // eink_display_ss::display.fillScreen(GxEPD_WHITE);
    // // eink_display_ss::display.update();
    // eink_display_ss::display.setTextColor(GxEPD_BLACK);
    // eink_display_ss::display.setFont(f);
    // eink_display_ss::display.setCursor(50, 15);
    // eink_display_ss::display.println();
    // eink_display_ss::display.println(" Daily Quote");
    // eink_display_ss::display.update();
    // delay(5000);
    // delay(1000);
    // display.eraseDisplay();

}

void eink_display_ss::set_image() {

    uint16_t height = 286;
    uint16_t width = 118;
    
    eink_display_ss::display.firstPage();
    eink_display_ss::display.setRotation(2);

    do
    {
        eink_display_ss::reset_screen();
        eink_display_ss::display.drawBitmap(0, 0, gImage_gui, width, height, GxEPD_BLACK);
        
    }
    while (eink_display_ss::display.nextPage());

    // eink_display_ss::display.update();
    delay(2000);

    // eink_display_ss::display.setRotation(1);
    // eink_display_ss::display.setFont(f);
    // eink_display_ss::display.setCursor(15, 10);
    // eink_display_ss::display.println();
    // eink_display_ss::display.println("Darien: 1");
    // eink_display_ss::display.update();
    // delay(5000);

}

void eink_display_ss::reset_screen() {
    eink_display_ss::display.fillScreen(GxEPD_WHITE);
}

void eink_display_ss::write_text(String text, uint16_t x, uint16_t y) {

    eink_display_ss::display.setTextColor(GxEPD_BLACK);
    eink_display_ss::display.firstPage();

    eink_display_ss::display.setRotation(3);
    eink_display_ss::display.setFont(f);
    eink_display_ss::display.setCursor(x, y);

    do
    {
        eink_display_ss::reset_screen();
        eink_display_ss::display.println(text);
        
    }
    while (eink_display_ss::display.nextPage());

    // eink_display_ss::display.firstPage();

    // do
    // {

    // Serial.println("Test0");
    // eink_display_ss::display.setRotation(1);
    // Serial.println("Test1");
    // eink_display_ss::display.setFont(f);
    // Serial.println("Test2");

    // eink_display_ss::display.setCursor(x, y);
    // Serial.println("Test3");
    // eink_display_ss::display.print(text);
    // Serial.println("Test4");
    // // eink_display_ss::display.update();
    // Serial.println("Test5");
    // }
    // while (display.nextPage());

}