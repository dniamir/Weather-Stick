#include <Arduino.h>

#include <GxEPD.h>
#include <GxGDEW0154M10/GxGDEW0154M10.h>      // 1.54" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <BitmapGraphics.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define ONBOARD_LED 2

#include <Wire.h>
#include <Arduino.h>

// E-Ink Wiring
// Busy: D4
// RST: D21
// DC: D22
// CS: D5
// CLK: D18
// DIN: D23
//  GND: GND
// 3.3V: 3.3V

// I2C
// CLK: D32
// SDA: D33

int address = 0x76;
int reg = 0xD0;
float wait_time = 100;
int rotation_idx = 1;

GxIO_Class io (SPI, 5, 22, 21);
GxEPD_Class display(io, 16, 4);

const char* name = "FreeSansBold9pt7b";
const GFXfont* f = &FreeSansBold9pt7b;

int I2C_SDA = 33;
int I2C_SCL = 32;

void toggleLED1(void *parameter){
  for(;;){ // infinite loop

    // Turn the LED on
    digitalWrite(ONBOARD_LED, HIGH);

    // Pause the task for 500ms
    vTaskDelay(*((float*)parameter) / portTICK_PERIOD_MS);

    // Turn the LED off
    digitalWrite(ONBOARD_LED, LOW);

    // Pause the task again for 500ms
    vTaskDelay(*((float*)parameter) / portTICK_PERIOD_MS);

    // Print which core the task is running on
    // Serial.print("Task 1 is running on: ");
    // Serial.println(xPortGetCoreID());
    // Serial.println(*((float*)parameter));
    // Serial.println();
  }
}
 

void setup_lcd(void * parameter){
  Serial.println("Starting...");
  display.init();
  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  display.update();
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(50, 15);
  display.println();
  display.println(" Daily Quote");
  display.update();
  delay(1000);
  display.eraseDisplay();
}

void setup() {
  pinMode(ONBOARD_LED, OUTPUT);
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.begin(9600);

  delay(2000);

  // Start Blinking Lights
  xTaskCreate(toggleLED1, "Toggle LED1", 1000, (void*)&wait_time,  1, NULL);

  // Setup Screen
  // xTaskCreate(setup_lcd, "Setup LCD", 1000, NULL,  1, NULL);
  setup_lcd(NULL);

}

void loop() {
  
  // Display Setup
  display.init();
  Serial.println("Update Screen...");
  display.eraseDisplay();
  display.setRotation(rotation_idx);
  display.drawBitmap(gImage_test, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(200);

  display.setRotation(0);
  display.setFont(f);
  display.setCursor(15, 10);
  display.println();
  display.println("Darien: 1");
  display.update();
  Serial.println("Finished Updating Screen...");
  Serial.println("");

  rotation_idx += 1;

  delay(200);

  // I2C to BMP390L Pressure Sensor
  Serial.println("Check BMP390L...");
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  uint8_t byte = Wire.read();
  Serial.println("The slave address / reg address / reg value are:");

  // Dev values
  Serial.print(address);
  Serial.print(" / ");
  Serial.print(reg);
  Serial.print(" / ");
  Serial.print(byte);
  Serial.println("");

  // Hex values
  Serial.print("0x");
  Serial.print(String(address, HEX));
  Serial.print(" / 0x");
  Serial.print(String(reg, HEX));
  Serial.print(" / 0x");
  Serial.print(String(byte, HEX));
  Serial.println("");

  Serial.println("Finishing Checking BMP390L...");
  Serial.println("");

  delay(500);
}