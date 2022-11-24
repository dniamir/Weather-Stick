# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680_ss.h>

#define ONE_WIRE_BUS 27
const int tsl_interrupt_pin = 35;  // the number of the pushbutton pin

int I2C_SDA = 33;
int I2C_SCL = 32;

// Define sensor systems
ArduinoI2C arduino_i2c;
SCD4x_SS co2_system;
TSL2591_SS light_system = TSL2591_SS(arduino_i2c);
BME680_SS bme_system = BME680_SS(arduino_i2c);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire = OneWire(ONE_WIRE_BUS);
MAX31820_SS temp_sensor = MAX31820_SS(oneWire);

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