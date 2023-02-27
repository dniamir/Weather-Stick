# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680_ss.h>
# include <MAX17260.h>
# include <is31fl3193_ss.h>
# include <eink_display_ss.h>
# include <driver/rtc_io.h>

// Define systems
ArduinoI2C arduino_i2c;
IS31FL3193_SS rgb_system = IS31FL3193_SS(arduino_i2c);

const int LED_PIN = 26;
const int LED_DRIVER_EN_PIN = 17;
const float led_blink_time = 1000;

int I2C_SDA = 33;
int I2C_SCL = 32;

// Never ending task
void toggle_led1(void * parameter){
  for(;;) { // infinite loop
    digitalWrite(LED_PIN, HIGH);  // // Turn the LED on
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);  // Turn the LED off
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(LED_PIN, OUTPUT);

  // Configure LED Driver Pin
  pinMode(LED_DRIVER_EN_PIN, OUTPUT);
  digitalWrite(LED_DRIVER_EN_PIN, HIGH);  // Enable

  // xTaskCreate(toggle_led1, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);

  // Configure RGB
  Serial.println(" ");
  Serial.println("Configure RGB");
  rgb_system.configure_pwm_mode();
  rgb_system.charging_high_battery();

}

void loop() {

}