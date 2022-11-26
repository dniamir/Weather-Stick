# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680_ss.h>
# include <eink_display_ss.h>


#define ONE_WIRE_BUS 27
const int tsl_interrupt_pin = 35;

int I2C_SDA = 33;
int I2C_SCL = 32;
const int LED_PIN = 26;
const float led_blink_time = 1000;

// Define sensor systems
ArduinoI2C arduino_i2c;
SCD4x_SS co2_system;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);
BME680_SS bme_system = BME680_SS(arduino_i2c);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire = OneWire(ONE_WIRE_BUS);
MAX31820_SS temp_sensor = MAX31820_SS(oneWire);

// Display class
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

void display_write(void * parameter){
  for(;;) { // infinite loop
  Serial.println("...Clearing Screen...");
    display_ss.display.clearScreen();
    Serial.println("...Resetting Screen...");
    display_ss.reset_screen();
    Serial.println("...Writing Readings...");
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
                            &bme_system.iaq);
    delay(3000);
  }
}

void setup() {
  Serial.begin(9600);

  // Setup I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Soft Reset Systems
  bme_system.soft_reset();
  delay(200);

  // Setup systems
  light_system.configure_system();
  co2_system.configure_system(Wire);
  temp_sensor.configure_system();
  bme_system.configure_system();

  // Pin modes
  light_system.interrupt_pin = tsl_interrupt_pin;
  pinMode(tsl_interrupt_pin, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // LED
  xTaskCreate(toggle_led1, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);

  // Display
  display_ss.configure_system();
  // display_ss.set_image();
  xTaskCreate(display_write, "Display Write",  100000, NULL, 1, NULL);

  

}

void loop() {

  delay(1000);

  // Read Data
  co2_system.read_data(true);
  light_system.read_data(true);
  temp_sensor.read_data(true);
  bme_system.read_data(true);

  // // Interrupts
  Serial.print("Light Sensor Interrupt: ");
  Serial.println(light_system.read_interrupt());
  Serial.println();

}