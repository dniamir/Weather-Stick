# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680.h>

#define ONE_WIRE_BUS 27
const int tsl_interrupt_pin = 35;  // the number of the pushbutton pin

int I2C_SDA = 33;
int I2C_SCL = 32;

// Define sensor systems
ArduinoI2C arduino_i2c;
SCD4x_SS co2_system;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);
BME680 bme_sensor = BME680(arduino_i2c);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire = OneWire(ONE_WIRE_BUS);
MAX31820_SS temp_sensor = MAX31820_SS(oneWire);

void setup() {
  Serial.begin(9600);

  Wire.begin(I2C_SDA, I2C_SCL);

  // Soft Reset Systems

  // Setup systems
  light_system.configure_system();
  co2_system.configure_system(Wire);
  temp_sensor.configure_system();

  // Setup BME
  bme_sensor.soft_reset();
  delay(200);
  Serial.println(bme_sensor.read_field(0xD0));

  Serial.println("Start Cal code read");
  bme_sensor.read_cal_codes();
  bme_sensor.write_field("osrs_h", 0b101);
  bme_sensor.write_field("osrs_t", 0b101);
  bme_sensor.write_field("osrs_p", 0b101);
  bme_sensor.write_field("filter", 0b010);
  bme_sensor.write_field("run_gas", 0b1);
  bme_sensor.write_field("nb_conv", 0b0000);
  bme_sensor.write_field("mode", 0b01);

  // Pin modes
  light_system.interrupt_pin = tsl_interrupt_pin;
  pinMode(tsl_interrupt_pin, INPUT);

}

void loop() {

  delay(1000);

  // Read Data
  co2_system.read_data(true);
  light_system.read_data(true);
  temp_sensor.read_data(true);

    // Read BME Data
  int32_t bme_temperature = bme_sensor.read_temperature();
  int32_t bme_pressure = bme_sensor.read_pressure();
  int32_t bme_humidity = bme_sensor.read_humidity();
  Serial.print("BME 680 Temperature: ");
  Serial.println((float)bme_temperature / 100);
  Serial.print("BME 680 Pressure: ");
  Serial.println((float)bme_pressure);
  Serial.print("BME 680 Humidity: ");
  Serial.println((float)bme_humidity / 1000);
  Serial.println();

  // // Interrupts
  Serial.print("Light Sensor Interrupt: ");
  Serial.println(light_system.read_interrupt());
  Serial.println();

}