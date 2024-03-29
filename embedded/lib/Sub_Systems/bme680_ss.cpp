# include <BME680_SS.h>
# include <logger.h>

BME680_SS::BME680_SS(ArduinoI2C input_protocol) : BME680(input_protocol) {}


void BME680_SS::configure_system(uint8_t profile_num) {

    // Read Cal codes
    LOGGER::write_to_log("BME", "READ CAL CODES");
    BME680::read_cal_codes();

    // Other Sensor Settings
    LOGGER::write_to_log("BME", "CONFIGURE HTP SENSORS");
    BME680::write_field("osrs_h", 0b101);  // 16x oversampling
    BME680::write_field("osrs_t", 0b101);  // 16x oversampling
    BME680::write_field("osrs_p", 0b101);  // 16x oversampling
    BME680::write_field("filter", 0b010);  // Filter coefficient of 3 - form of averaging filter

    // Gas Sensor Settings
    LOGGER::write_to_log("BME", "CONFIGURE GAS SENSORS");
    BME680::write_field("gas_range_r", 4); // Set Gas Range
    BME680::write_field("run_gas", 0b1); // Turn on Gas Sensor
    BME680::write_field("nb_conv", profile_num); // Set Heater profile to profile 0

    // Set time between beginning of the heat phase and start of resistance conversion
    BME680::set_gas_wait(0b00011110, profile_num); // This should be 30ms

    // Set heater temperature
    BME680::set_heater_temp(300, profile_num);  // Set heater profile 0 to 300C
}

void BME680_SS::read_data(bool print_data) {

    BME680_SS::temperature_100_degc = BME680::read_temperature();
    BME680_SS::pressure = BME680::read_pressure();
    BME680_SS::humidity_1000 = BME680::read_humidity();
    BME680_SS::gas = BME680::read_gas();
    BME680_SS::iaq = BME680::calculate_iaq(gas, BME680_SS::humidity_1000 / 1000);

    if (!print_data) {return;}

    LOGGER::write_to_log("BMET", BME680_SS::temperature_100_degc, 2);
    LOGGER::write_to_log("BMEP", BME680_SS::pressure);
    LOGGER::write_to_log("BMEH", BME680_SS::humidity_1000, 3);
    LOGGER::write_to_log("BMEG", BME680_SS::gas, 3);   // Resistance should range from 0ohm (poor quality) to 50kohm (very good quality)
    LOGGER::write_to_log("BMEI", BME680_SS::iaq);

}
