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
# include <logger.h>

# include <WiFi.h>
# include <wifi_ss.h>

// Serial version by parent ID
std::string serial_id = "ffc010e6fd"; 

// Initialize I2C
int I2C_SDA = 33;
int I2C_SCL = 32;
ArduinoI2C arduino_i2c;

// Initialize LEDs and LED driver
const int LED_PIN = 26;
const int LED_DRIVER_EN_PIN = 17;
const float led_blink_time = 1000;
int16_t cur_rgb_status = 0;
int16_t prev_rgb_status = -1;
IS31FL3193_SS rgb_system = IS31FL3193_SS(arduino_i2c);

// Initialize light sensor
const int tsl_interrupt_pin = 35;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);

// Initialize temperature sensor
# define ONE_WIRE_BUS 27
OneWire oneWire = OneWire(ONE_WIRE_BUS);
MAX31820_SS temp_sensor = MAX31820_SS(oneWire);

// Initialize charger and fuel gauge
const int CHARGER_EN_PIN = 15;
const int CHARGER_POK_PIN = 14;
const int CHARGER_STATUS_PIN = 36;
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
    vTaskDelay(*((float*)parameter) * 1 / 20 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);  // Turn the LED off
    vTaskDelay(*((float*)parameter) * 2 / portTICK_PERIOD_MS);
  }
}

// RGB status checker
void start_led_tracker(void * pvParameter) {

  for(;;) { // infinite loop

    bool charger_pok = fuel_gauge.read_charger_pok(true);
    bool charger_status = fuel_gauge.read_charger_status(true);
    light_system.read_data(true);
    fuel_gauge.read_data(true);

    // Do not update if device is not plugged in and CO2 has not been read
    if((co2_system.co2_ppm == 0) & (~charger_pok)){
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    } 

    // Check relevant statuses
    rgb_system.set_status(light_system.light_vis, 
                          charger_pok,
                          charger_status,
                          fuel_gauge.level_10_percent, 
                          co2_system.co2_ppm);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

  }
}

void setup() {
  Serial.begin(9600);
  Serial2.end();  // Needs to be called otherwise GPIO16 and GPIO17 will have conflict
  Serial.println("");

  // Print FW Version
 
  LOGGER::write_to_log("SRLV", serial_id);  // Print serial ID
  LOGGER::write_to_log("MACA", (std::string)WiFi.macAddress().c_str());  // Print mac address

  // Return wakeup source - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#_CPPv418esp_sleep_source_t
  LOGGER::write_to_log("WAKE", (int32_t)esp_sleep_get_wakeup_cause());
  // 0 = ESP_SLEEP_WAKEUP_ALL - normal wakeup, or when EN is pressed
  // 2 - ESP_SLEEP_WAKEUP_EXT0 - When GPIO, such as charging POK, is triggered

  // Toggle LED
  LOGGER::write_to_log("LOG", "CREATE BLINK");
  pinMode(LED_PIN, OUTPUT);
  xTaskCreate(toggle_led1, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);

  // Setup I2C
  LOGGER::write_to_log("LOG", "BEGIN I2C");
  Wire.begin(I2C_SDA, I2C_SCL);

  // Gas sensor config
  bme_system.soft_reset();
  delay(200);
  bme_system.configure_system();

  // Charger and fuel gauge config
  fuel_gauge.configure_system();
  fuel_gauge.charger_pok_pin = CHARGER_POK_PIN;
  fuel_gauge.charger_status_pin = CHARGER_STATUS_PIN;
  pinMode(CHARGER_POK_PIN, INPUT);
  pinMode(CHARGER_EN_PIN, OUTPUT);
  pinMode(CHARGER_STATUS_PIN, INPUT);
  digitalWrite(CHARGER_EN_PIN, LOW);  // Enable charging

  // Light sensor config
  light_system.interrupt_pin = tsl_interrupt_pin;
  light_system.configure_system();
  pinMode(tsl_interrupt_pin, INPUT);

  // Temp sensor config
  temp_sensor.configure_system();
  
  // CO2 sensor config
  co2_system.begin(Wire);

  // LEDs and LED driver config
  pinMode(LED_DRIVER_EN_PIN, OUTPUT);
  digitalWrite(LED_DRIVER_EN_PIN, HIGH);  // Enable
  xTaskCreate(start_led_tracker, "Set RGB",  2048, NULL, 1, NULL);

  // Display config
  display_ss.configure_system();

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
  light_system.read_data(true);
  light_system.disable_system();

  // Read Other Data
  temp_sensor.read_data(true);
  bme_system.read_data(true);

  // Interrupts
  light_system.read_interrupt(true);

  // Update Screen
  display_ss.write_readings(&co2_system.co2_ppm,
                            &co2_system.temperature_10_degc,
                            &co2_system.humidity_10_per,
                            &light_system.light_ir,
                            &light_system.light_vis,
                            &temp_sensor.temperature_100_degc,
                            &bme_system.temperature_100_degc,
                            &bme_system.pressure,
                            &bme_system.humidity_1000,
                            &bme_system.gas,
                            &bme_system.iaq,
                            &fuel_gauge.level_10_percent,
                            &fuel_gauge.level_10_mah,
                            &fuel_gauge.batt_10_voltage,
                            false);  

  // Wifi config
  wifi_system.connect_to_wifi(ssid, password);  // Only connects if it's not already connected

  String wifi_message[16];
  wifi_message[0] = (String)"scd_co2_ppm " + co2_system.co2_ppm;
  wifi_message[1] = (String)"scd_temperature_10_degc " + co2_system.temperature_10_degc;
  wifi_message[2] = (String)"scd_humidity_10_percent " + co2_system.humidity_10_per;
  wifi_message[3] = (String)"tsl_light_fs " + light_system.light_fs;
  wifi_message[4] = (String)"tsl_light_ir " + light_system.light_ir;
  wifi_message[5] = (String)"tsl_light_vis " + light_system.light_vis;
  wifi_message[6] = (String)"bme_temperature_100_degc " + bme_system.temperature_100_degc;
  wifi_message[7] = (String)"bme_pressure_pa " + bme_system.pressure;
  wifi_message[8] = (String)"bme_humidity_1000_percent " + bme_system.humidity_1000;
  wifi_message[9] = (String)"bme_gas " + bme_system.gas;
  wifi_message[10] = (String)"bme_iaq " + bme_system.iaq;
  wifi_message[11] = (String)"max_level_10_percent " + fuel_gauge.level_10_percent;
  wifi_message[12] = (String)"max_level_10_percent " + fuel_gauge.level_10_percent;
  wifi_message[13] = (String)"max_batt_10_voltage " + fuel_gauge.batt_10_voltage;
  wifi_message[14] = (String)"max_avg_current_ua " + fuel_gauge.avg_current_ua;
  wifi_message[15] = (String)"max_temperature_100_degc " + temp_sensor.temperature_100_degc;
  wifi_system.send_message(wifi_message, true);

  wifi_system.disconnect();
  
  // Put Microcontroller to sleep for 10 min
  if (!fuel_gauge.charger_pok) {
    LOGGER::write_to_log("LOG", "SLEEP");
    Serial.println("");
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    gpio_deep_sleep_hold_en();
    esp_sleep_enable_timer_wakeup(60 * 10 * 1e6);  // us
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0); // If charger detects an OK power source, start up system
    gpio_hold_en(GPIO_NUM_15);  // Keep charger enabled while in sleep mode
    gpio_hold_en(GPIO_NUM_26);  // Keep LED off while in sleep mode
    gpio_hold_en(GPIO_NUM_17);  // Keep LED_DRIVER_EN_PIN high (won't work with non-RTC pin)
    esp_deep_sleep_start();
  }
  else {
    Serial.println("");
  }
}