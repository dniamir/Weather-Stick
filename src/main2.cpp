# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680_ss.h>
# include <MAX17260.h>
# include <eink_display_ss.h>


#define ONE_WIRE_BUS 27
const int tsl_interrupt_pin = 35;

int I2C_SDA = 33;
int I2C_SCL = 32;
const int LED_PIN = 26;
const int CHARGER_EN = 15;
const int CHARGER_POK = 14;
const float led_blink_time = 1000;

// Define sensor systems
ArduinoI2C arduino_i2c;
SCD4x_SS co2_system;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);
BME680_SS bme_system = BME680_SS(arduino_i2c);

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

// void display_write(void * parameter){
//   for(;;) { // infinite loop
//   Serial.println("...Clearing Screen...");
//     // display_ss.display.clearScreen();
//     Serial.println("...Resetting Screen...");
//     display_ss.reset_screen();
//     Serial.println("...Writing Readings...");
//     display_ss.write_readings(&co2_system.co2,
//                             &co2_system.temperature,
//                             &co2_system.humidity,
//                             &light_system.light_fs,
//                             &light_system.light_ir,
//                             &light_system.light_vis,
//                             &temp_sensor.temperature_100_degc,
//                             &bme_system.temperature_100,
//                             &bme_system.pressure,
//                             &bme_system.humidity_1000,
//                             &bme_system.gas,
//                             &bme_system.iaq);
//     delay(30000);
//   }
// }

void setup() {
  Serial.begin(9600);

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
  pinMode(CHARGER_EN, OUTPUT);
  pinMode(CHARGER_POK, INPUT);

  // Set Charging to Enable
  digitalWrite(CHARGER_EN, LOW);

  // LED
  bool charger_ok = fuel_gauge.read_charge_source_ok();
  if (charger_ok) {
    xTaskCreate(toggle_led1, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);
  }
  
  // Display
  display_ss.configure_system();
  // xTaskCreate(display_write, "Display Write",  100000, NULL, 1, NULL);

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

  // Put Microcontroller to sleep for 10 min
  if (!charger_ok) {
    // esp_sleep_enable_timer_wakeup(60 * 0.2 * 1e6);  // us
    esp_sleep_enable_timer_wakeup(60 * 10 * 1e6);  // us
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 1);
    esp_deep_sleep_start();
  }

}