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

# include <WiFi.h>
# include <wifi_ss.h>

// Initialize I2C
int I2C_SDA = 33;
int I2C_SCL = 32;
ArduinoI2C arduino_i2c;

// Initialize LEDs and LED driver
const int LED_PIN = 26;
const int LED_DRIVER_EN_PIN = 17;
const float led_blink_time = 1000;
uint8_t cur_rgb_status = 0;
int8_t prev_rgb_status = -1;
IS31FL3193_SS rgb_system = IS31FL3193_SS(arduino_i2c);

// Initialize light sensor
const int tsl_interrupt_pin = 35;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);

// Initialize temperature sensor
# define ONE_WIRE_BUS 27
OneWire oneWire = OneWire(ONE_WIRE_BUS);
MAX31820_SS temp_sensor = MAX31820_SS(oneWire);

// Initialize charger and fuel gauge
const int CHARGER_EN = 15;
const int CHARGER_POK = 14;
MAX17260 fuel_gauge = MAX17260(arduino_i2c);

// Initilaize Wifi
bool wifi_status = false;
const char* ssid = "Canucks";
const char* password = "stanford";
WIFI_SS wifi_system;

// Initialize other systems 
SCD4x_SS co2_system;
BME680_SS bme_system = BME680_SS(arduino_i2c);
eink_display_ss display_ss = eink_display_ss();

// Never ending task
void toggle_led1(void * parameter){
  for(;;) { // infinite loop
    digitalWrite(LED_PIN, HIGH);  // // Turn the LED on
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);  // Turn the LED off
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
  }
}

// Almost never ending task
void toggle_led2(void * parameter) {
  for(uint8_t i=0; i < 20; i++) { // infinite loop
    digitalWrite(LED_PIN, HIGH);  // // Turn the LED on
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);  // Turn the LED off
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
  }
}

// RGB LED Mode
void rgb_led_task(void *args){
  for(;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if(cur_rgb_status == prev_rgb_status) {
      continue;
    }
    if(cur_rgb_status == 0) {rgb_system.off();}
    else if(cur_rgb_status == 1) {rgb_system.low_battery();}
    else if(cur_rgb_status == 2) {rgb_system.charging_low_battery();}
    else if(cur_rgb_status == 3) {rgb_system.charging_med_battery();}
    else if(cur_rgb_status == 4) {rgb_system.charging_high_battery();}
    else if(cur_rgb_status == 5) {rgb_system.charging_full_battery();}
    else if(cur_rgb_status == 6) {rgb_system.air_quality_bad();}
    else if(cur_rgb_status == 7) {rgb_system.air_quality_very_bad();}
    prev_rgb_status = cur_rgb_status;
  }
}

void setup() {
  Serial.begin(9600);

  // Setup I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Gas sensor config
  bme_system.soft_reset();
  bme_system.configure_system();
  delay(200);

  // Charger and fuel gauge config
  fuel_gauge.configure_system();
  fuel_gauge.charger_ok_pin = CHARGER_POK;
  bool charger_ok = fuel_gauge.read_charge_source_ok();
  pinMode(CHARGER_POK, INPUT);
  pinMode(CHARGER_EN, OUTPUT);
  digitalWrite(CHARGER_EN, LOW);  // Enable charging

  // Wifi config
  wifi_system.connect_to_wifi(ssid, password);

  // Light sensor config
  light_system.interrupt_pin = tsl_interrupt_pin;
  light_system.configure_system();

  // Temp sensor config
  temp_sensor.configure_system();
  
  // CO2 sensor config
  co2_system.begin(Wire);

  // Light sensor config
  pinMode(tsl_interrupt_pin, INPUT);

  // LEDs and LED driver config
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_DRIVER_EN_PIN, OUTPUT);
  digitalWrite(LED_DRIVER_EN_PIN, HIGH);  // Enable
  rgb_system.configure_pwm_mode();
  rgb_system.off();

  // Display config
  display_ss.configure_system();
  
  // if (charger_ok) {
  //   xTaskCreate(toggle_led2, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);
  // }
    
  // xTaskCreate(display_write, "Display Write",  100000, NULL, 1, NULL);  
  // xTaskCreate(rgb_led_task, "Configure RGB",  1000, NULL, 1, NULL);

}

void loop() {

  // Read CO2 Data - discard first entry
  co2_system.wake_up();
  co2_system.read_oneshot();
  co2_system.read_data(true);
  delay(6000);
  co2_system.read_oneshot();
  co2_system.read_data(true);
  co2_system.power_down();

  // Take light sensor data
  light_system.configure_system();
  light_system.read_data(true);
  light_system.disable_system();

  // Read Other Data
  temp_sensor.read_data(true);
  bme_system.read_data(true);
  fuel_gauge.read_data(true);

  // Read Charger Status
  Serial.print("Charger Status is: ");
  bool charger_ok = fuel_gauge.read_charge_source_ok();
  Serial.println(charger_ok);

  // Interrupts
  Serial.print("Light Sensor Interrupt: ");
  Serial.println(light_system.read_interrupt());
  Serial.println();

  // Update Screen
  display_ss.write_readings(&co2_system.co2,
                            &co2_system.temperature,
                            &co2_system.humidity,
                            &light_system.light_fs,
                            &light_system.light_ir,
                            &light_system.light_vis,
                            &temp_sensor.temperature_100_degc,
                            &bme_system.temperature_100,
                            &bme_system.pressure,
                            &bme_system.humidity_1000,
                            &bme_system.gas,
                            &bme_system.iaq,
                            &fuel_gauge.level_percent,
                            &fuel_gauge.level_mah,
                            &fuel_gauge.batt_voltage);  
  
  // Set LED Mode
  bool lights_on = light_system.light_vis > 500;
  bool charging_status = (fuel_gauge.avg_current_ma > 100);
  if(lights_on) {
    if((charging_status) & (fuel_gauge.level_percent > 95)) {rgb_system.charging_full_battery();}
    else if((charging_status) & (fuel_gauge.level_percent > 60)) {rgb_system.charging_high_battery();}
    else if((charging_status) & (fuel_gauge.level_percent > 30)) {rgb_system.charging_med_battery();}
    else if(charging_status) {rgb_system.charging_low_battery();}
    else if(fuel_gauge.level_percent < 20) {rgb_system.low_battery();}
    else if(co2_system.co2 > 1500) {rgb_system.air_quality_very_bad();}
    else if(co2_system.co2 > 1000) {rgb_system.air_quality_bad();}
    else {rgb_system.off();}
  }
  else {rgb_system.off();}
  // if(lights_on) {
  //   if((charging_status) & (fuel_gauge.level_percent > 95)) {cur_rgb_status = 5;}
  //   else if((charging_status) & (fuel_gauge.level_percent > 60)) {cur_rgb_status = 4;}
  //   else if((charging_status) & (fuel_gauge.level_percent > 30)) {cur_rgb_status = 3;}
  //   else if(charging_status) {cur_rgb_status = 2;}
  //   else if(fuel_gauge.level_percent < 20) {cur_rgb_status = 1;}
  //   else if(co2_system.co2 > 1500) {cur_rgb_status = 7;}
  //   else if(co2_system.co2 > 1000) {cur_rgb_status = 6;}
  //   else {cur_rgb_status = 0;}
  // }

  int32_t wifi_message[14];
  wifi_message[0] = co2_system.co2;
  wifi_message[1] = co2_system.temperature;
  wifi_message[2] = co2_system.humidity;
  wifi_message[3] = light_system.light_fs;
  wifi_message[4] = light_system.light_ir;
  wifi_message[5] = light_system.light_vis;
  wifi_message[6] = bme_system.temperature_100;
  wifi_message[7] = bme_system.pressure;
  wifi_message[8] = bme_system.humidity_1000;
  wifi_message[9] = bme_system.gas;
  wifi_message[10] = bme_system.iaq;
  wifi_message[11] = fuel_gauge.level_percent;
  wifi_message[12] = fuel_gauge.level_mah;
  wifi_message[13] = fuel_gauge.batt_voltage;
  wifi_system.send_message(wifi_message, true);
  
  // Put Microcontroller to sleep for 10 min
  if (!charger_ok) {
    Serial.println("Putting system to sleep");
    esp_sleep_enable_timer_wakeup(60 * 10 * 1e6);  // us
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0); // If charger detects an OK power source, start up system
    gpio_hold_en(GPIO_NUM_15);  // Keep charger enabled while in sleep modeLED_DRIVER_EN_PIN
    gpio_hold_en(GPIO_NUM_17);  // Keep LED Driver enabled while in sleep mode
    esp_deep_sleep_start();
  }
}