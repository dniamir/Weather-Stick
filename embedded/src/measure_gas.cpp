# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680_ss.h>
# include <MAX17260.h>
# include <eink_display_ss.h>
# include <wifi.h>


#define ONE_WIRE_BUS 27
const int tsl_interrupt_pin = 35;
const int BOOT_BUTTON = 0;

int I2C_SDA = 33;
int I2C_SCL = 32;
const int LED_PIN = 26;
const int CHARGER_EN_PIN = 15;
const int CHARGER_POK_PIN = 14;
const float led_blink_time = 1000;
int counter = 0;

// Readings
int32_t humidity_reading = 0;
int32_t temperature_reading = 0;
int32_t pressure_reading = 0;
uint16_t co2_reading = 0;
float temperature_co2_reading = 0.0f;
float humidity_co2_reading = 0.0f;

// Define sensor systems
ArduinoI2C arduino_i2c;
SCD4x_SS co2_system;
BME680_SS bme_system = BME680_SS(arduino_i2c);

// Time
struct timeval tv_now;

// Never ending task
void toggle_led1(void * parameter){
  for(;;) { // infinite loop
    digitalWrite(LED_PIN, HIGH);  // // Turn the LED on
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);  // Turn the LED off
    vTaskDelay(*((float*)parameter) / 2 / portTICK_PERIOD_MS);
  }
}

// CO2 Reading
void co2_read_task(void *arg)
{
  for(;;) { // infinite loop
    co2_system.read_oneshot();
    co2_system.read_data(false);
  }
}


void setup() {
  Serial.begin(9600);

  // Set up button
  pinMode(BOOT_BUTTON, INPUT_PULLUP);

  // Setup I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Soft Reset Systems
  bme_system.soft_reset();
  delay(200);

  // Setup systems
  bme_system.configure_system();

  // Setup one-shot mode
  co2_system.begin(Wire);

  // Pin modes
  pinMode(tsl_interrupt_pin, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(CHARGER_EN_PIN, OUTPUT);
  pinMode(CHARGER_POK_PIN, INPUT);

  // Set Charging to Enable
  digitalWrite(CHARGER_EN_PIN, LOW);

  // LED
  xTaskCreate(toggle_led1, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);

  // Wake up CO2
  co2_system.wake_up();
  xTaskCreate(co2_read_task, "CO2 Read",  4096, NULL, 2, NULL);

  Serial.println(" ");
  Serial.print("Time");
  Serial.print(", ");
  Serial.print("Button");
  Serial.print(", ");
  Serial.print("Gas");
  Serial.print(", ");
  Serial.print("Humidity");
  Serial.print(", ");
  Serial.print("Temperature");
  Serial.print(", ");
  Serial.print("Pressure");
  Serial.print(", ");
  Serial.print("CO2");
  Serial.print(", ");
  Serial.print("SCD Temperature");
  Serial.print(", ");
  Serial.print("SCD Humidity");
  Serial.println("");
  
}

void loop() {

  counter += 1;

  gettimeofday(&tv_now, NULL);
  int64_t time_us = ((int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec) / 1000;

  // Read Other Data
  int32_t gas_reading = bme_system.read_gas();

  // Take other measurements
  if(counter % 6 == 0){

    // Measure BME
    humidity_reading = bme_system.read_humidity();
    temperature_reading = bme_system.read_temperature();
    pressure_reading = bme_system.read_pressure();

    // Measure CO2
    co2_reading = co2_system.co2_ppm;
    temperature_co2_reading = co2_system.temperature_10_degc;
    humidity_co2_reading = co2_system.humidity_10_per;
  }

  // Read Button Press
  bool button_press = digitalRead(BOOT_BUTTON);

  // Print CO2 Data
  Serial.print(time_us);
  Serial.print(", ");
  Serial.print(button_press);
  Serial.print(", ");
  Serial.print(gas_reading);
  Serial.print(", ");
  Serial.print(humidity_reading);
  Serial.print(", ");
  Serial.print(temperature_reading);
  Serial.print(", ");
  Serial.print(pressure_reading);
  Serial.print(", ");
  Serial.print(co2_reading);
  Serial.print(", ");
  Serial.print(temperature_co2_reading);
  Serial.print(", ");
  Serial.print(humidity_co2_reading);
  Serial.println("");
  delay(50);

}