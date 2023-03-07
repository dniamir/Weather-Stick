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
# include <HTTPClient.h>

//Your IP address or domain name with URL path
const char* server_ip_address = "192.168.4.102";
const uint16_t server_port = 8090;

#define ONE_WIRE_BUS 27
const int tsl_interrupt_pin = 35;

int I2C_SDA = 33;
int I2C_SCL = 32;
const int LED_PIN = 26;
const int LED_DRIVER_EN_PIN = 17;
const int CHARGER_EN = 15;
const int CHARGER_POK = 14;
const float led_blink_time = 1000;

// Wifi Config
bool wifi_status = false;
const char* ssid = "wifi_name";
const char* password = "wifi_password";

uint8_t cur_rgb_status = 0;
uint8_t prev_rgb_status = -1;

// Define systems
ArduinoI2C arduino_i2c;
SCD4x_SS co2_system;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);
BME680_SS bme_system = BME680_SS(arduino_i2c);
IS31FL3193_SS rgb_system = IS31FL3193_SS(arduino_i2c);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire = OneWire(ONE_WIRE_BUS);
MAX31820_SS temp_sensor = MAX31820_SS(oneWire);

// Other Systems
eink_display_ss display_ss = eink_display_ss();
MAX17260 fuel_gauge = MAX17260(arduino_i2c);

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
    if(cur_rgb_status == prev_rgb_status) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
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

void readResponse(WiFiClient *client) {
  unsigned long timeout = millis();
  while(client->available() == 0){
    if(millis() - timeout > 5000){
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Setup Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Soft Reset Systems
  bme_system.soft_reset();
  delay(200);

  // Setup systems
  light_system.configure_system();
  temp_sensor.configure_system();
  bme_system.configure_system();
  fuel_gauge.configure_system();

  // Setup one-shot mode
  co2_system.begin(Wire);

  // Pin modes
  light_system.interrupt_pin = tsl_interrupt_pin;
  fuel_gauge.charger_ok_pin = CHARGER_POK;
  pinMode(tsl_interrupt_pin, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Configure charging pins
  pinMode(CHARGER_POK, INPUT);
  pinMode(CHARGER_EN, OUTPUT);
  digitalWrite(CHARGER_EN, LOW);  // Enable charging

  // Configure LED Driver Pin
  pinMode(LED_DRIVER_EN_PIN, OUTPUT);
  digitalWrite(LED_DRIVER_EN_PIN, HIGH);  // Enable

  // LED
  bool charger_ok = fuel_gauge.read_charge_source_ok();
  // if (charger_ok) {
  //   xTaskCreate(toggle_led2, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);
  // }
  
  // Display
  display_ss.configure_system();
  // xTaskCreate(display_write, "Display Write",  100000, NULL, 1, NULL);

  // Configure RGB
  pinMode(LED_DRIVER_EN_PIN, OUTPUT);
  digitalWrite(LED_DRIVER_EN_PIN, HIGH);  // Enable
  rgb_system.configure_pwm_mode();
  rgb_system.off();
  // xTaskCreate(rgb_led_task, "Configure RGB",  1000, NULL, 1, NULL);

}

void loop() {

  // Read CO2 Data - discard first entry
  co2_system.wake_up();
  light_system.configure_system();
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
  Serial.println(fuel_gauge.avg_current_ma);
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


  // Send data to host
  // Serial.println("Attempting to connect to Wifi...");
  // WiFiClient client;
  // if (client.connect(server_ip_address, server_port)) {
  //   client.print(bme_system.pressure);
  //   readResponse(&client);
  //   // client.stop();
  //   Serial.println("Wifi send successful");
  // }

  // Send data to host
  Serial.println("Attempting to connect to Wifi...");
  WiFiClient client;
  client.connect(server_ip_address, server_port);
  client.print(" ");
  client.print(co2_system.co2);
  client.print(", ");
  client.print(co2_system.temperature);
  client.print(", ");
  client.print(co2_system.humidity);
  client.print(", ");
  client.print(light_system.light_fs);
  client.print(", ");
  client.print(light_system.light_ir);
  client.print(", ");
  client.print(light_system.light_vis);
  client.print(", ");
  client.print(bme_system.temperature_100);
  client.print(", ");
  client.print(bme_system.pressure);
  client.print(", ");
  client.print(bme_system.humidity_1000);
  client.print(", ");
  client.print(bme_system.gas);
  client.print(", ");
  client.print(bme_system.iaq);
  client.print(", ");
  client.print(fuel_gauge.level_percent);
  client.print(", ");
  client.print(fuel_gauge.level_mah);
  client.print(", ");
  client.print(fuel_gauge.batt_voltage);
  client.stop();

  
  // Put Microcontroller to sleep for 10 min
  if (!charger_ok) {
    // esp_sleep_enable_timer_wakeup(60 * 0.2 * 1e6);  // us
    Serial.println("Putting system to sleep");
    esp_sleep_enable_timer_wakeup(60 * 10 * 1e6);  // us
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0); // If charger detects an OK power source, start up system

    // rtc_gpio_deinit(GPIO_NUM_15);
    // rtc_gpio_set_direction(GPIO_NUM_15, RTC_GPIO_MODE_OUTPUT_ONLY);
    // rtc_gpio_set_level(GPIO_NUM_15, 0);
    gpio_hold_en(GPIO_NUM_15);  // Keep charger enabled while in sleep modeLED_DRIVER_EN_PIN
    gpio_hold_en(GPIO_NUM_17);  // Keep LED Driver enabled while in sleep mode

    esp_deep_sleep_start();
  }

}