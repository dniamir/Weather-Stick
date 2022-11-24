# include <BME680_SS.h>

BME680_SS::BME680_SS(ArduinoI2C input_protocol) : BME680(input_protocol) {}


void BME680_SS::configure_system(uint8_t profile_num) {

    // Read Cal codes
    BME680::read_cal_codes();

    // Other Sensor Settings
    BME680::write_field("osrs_h", 0b101);  // 16x oversampling
    BME680::write_field("osrs_t", 0b101);  // 16x oversampling
    BME680::write_field("osrs_p", 0b101);  // 16x oversampling
    BME680::write_field("filter", 0b010);  // Filter coefficient of 3 - form of averaging filter

    // Gas Sensor Settings
    BME680::write_field("gas_range_r", 4); // Set Gas Range
    BME680::write_field("run_gas", 0b1); // Turn on Gas Sensor
    BME680::write_field("nb_conv", profile_num); // Set Heater profile to profile 0

    // Set time between beginning of the heat phase and start of resistance conversion
    BME680::set_gas_wait(0b00011110, profile_num); // This should be 30ms

    // Set heater temperature
    BME680::set_heater_temp(300, profile_num);  // Set heater profile 0 to 300C
}

void BME680_SS::read_data(bool print_data) {

    int32_t temperature = BME680::read_temperature();
    int32_t pressure = BME680::read_pressure();
    int32_t humidity = BME680::read_humidity();
    int32_t gas = BME680::read_gas();
    int32_t iaq = BME680::calculate_iaq(gas, humidity / 1000);

    if (!print_data) {return;}

    Serial.print("BME680 Temperature: ");
    Serial.print((float)temperature / 100);
    Serial.print(", Pressure: ");
    Serial.print((float)pressure);
    Serial.print(", Humidity: ");
    Serial.print((float)humidity / 1000);
    Serial.print(", Raw Gas: ");
    Serial.print((float)gas / 1000);  // Resistance should range from 0ohm (poor quality) to 50kohm (very good quality)
    Serial.print(", IAQ: ");
    Serial.print((float)iaq);
    Serial.println();
}
