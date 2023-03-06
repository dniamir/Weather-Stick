# include <protocol.h>
# include <Arduino.h>
# include <arduino_i2c.h>
# include <chip.h>
# include <vector>


class BME680 : public Chip {

  public:
    // Different constructors
    BME680(ArduinoI2C input_protocol);
    BME680(int i2c_address_in, ArduinoI2C input_protocol);

    // Sensor config
    void set_heater_temp(int16_t target_temp, uint8_t profile_num);
    void set_gas_wait(int16_t wait_time, uint8_t profile_num);
    
    // I2C addresses
    int i2c_address = 0b1110110;

    // WHO AM I register - register the check upon startup
    Field who_am_i_reg = Field{0xD0, 0, 8, false};
    
    // Read data
    int32_t read_temperature();
    int32_t read_pressure();
    int32_t read_humidity();
    int32_t read_gas();
    int16_t calculate_iaq(int32_t gas_reading, int32_t humidity_reading);

    // Calibrations
    void read_cal_codes();
    int32_t calibrate_temperature(uint32_t press_adc);
    int32_t calibrate_pressure(uint32_t press_adc);
    int32_t calibrate_humidity(uint16_t humid_adc);

    void soft_reset();  // Soft reset

  private:

    // Value in default WHO AM I register - value to match during startup check
    int who_am_i_val = 0x61;

    // Intermediate calibration values
    int32_t t_fine;
    int32_t temp_comp;

    // Last one-shot reading
    struct BME680_cal_codes {

      // Pressure Codes
      uint8_t par_p10;
      int16_t par_p9;
      int16_t par_p8;
      int8_t par_p7;
      int8_t par_p6;
      int16_t par_p5;
      int16_t par_p4;
      int8_t par_p3;
      int16_t par_p2;
      uint16_t par_p1;

      // Temperature Codes
      int16_t par_t3;
      int16_t par_t2;
      uint16_t par_t1;

      // Humidity Codes
      int8_t par_h7;
      uint8_t par_h6;
      int8_t par_h5;
      int8_t par_h4;
      int8_t par_h3;
      uint16_t par_h2;
      uint16_t par_h1;

      // Gas Codes
      int8_t par_g3;
      int16_t par_g2;
      int8_t par_g1;

      // Misc Codes
      int8_t res_heat_range;
      int8_t res_heat_val;

      int16_t gas_adc;
      int8_t gas_range;
      int8_t range_switching_error;

      int64_t const_array1_int[16] = {2147483647, 2147483647, 2147483647, 2147483647,
	                2147483647, 2126008810, 2147483647, 2130303777, 2147483647,
	                2147483647, 2143188679, 2136746228, 2147483647, 2126008810,
	                2147483647, 2147483647};
	
      int64_t const_array2_int[16] = {4096000000, 2048000000, 1024000000, 512000000,
                      255744255, 127110228, 64000000, 32258064,
                      16016016, 8000000, 4000000, 2000000,
                      1000000, 500000, 250000, 125000};

    };

    struct BME680_cal_codes cal_codes;

  protected:

  public:

  // I2C Register Map - SPI Map is different
  std::map<String, Field> field_map {
    
    {"status", Field{0x73, 0, 8, true}},
    {"reset", Field{0xe0, 0, 8, true}},
    {"Config", Field{0x75, 0, 8, true}},
    {"filter", Field{0x75, 2, 3, true}},
    {"ctrl_meas", Field{0x74, 0, 8, true}},
    {"osrs_t", Field{0x74, 5, 3, true}},
    {"osrs_p", Field{0x74, 2, 3, true}},
    {"mode", Field{0x74, 0, 2, true}},
    {"osrs_h", Field{0x72, 0, 3, true}},
    {"ctrl_gas_1", Field{0x71, 0, 8, true}},
    {"ctrl_gas_0", Field{0x70, 4, 2, true}},
    {"run_gas", Field{0x71, 4, 1, true}},
    {"nb_conv", Field{0x71, 0, 4, true}},
    {"heat_off", Field{0x70, 3, 1, true}},
    {"gas_wait_9", Field{0x6d, 0, 8, true}},
    {"gas_wait_8", Field{0x6c, 0, 8, true}},
    {"gas_wait_7", Field{0x6b, 0, 8, true}},
    {"gas_wait_6", Field{0x6a, 0, 8, true}},
    {"gas_wait_5", Field{0x69, 0, 8, true}},
    {"gas_wait_4", Field{0x68, 0, 8, true}},
    {"gas_wait_3", Field{0x67, 0, 8, true}},
    {"gas_wait_2", Field{0x66, 0, 8, true}},
    {"gas_wait_1", Field{0x65, 0, 8, true}},
    {"gas_wait_0", Field{0x64, 0, 8, true}},
    {"res_heat_9", Field{0x63, 0, 8, true}},
    {"res_heat_8", Field{0x62, 0, 8, true}},
    {"res_heat_7", Field{0x61, 0, 8, true}},
    {"res_heat_6", Field{0x60, 0, 8, true}},
    {"res_heat_5", Field{0x5f, 0, 8, true}},
    {"res_heat_4", Field{0x5e, 0, 8, true}},
    {"res_heat_3", Field{0x5d, 0, 8, true}},
    {"res_heat_2", Field{0x5c, 0, 8, true}},
    {"res_heat_1", Field{0x5b, 0, 8, true}},
    {"res_heat_0", Field{0x5a, 0, 8, true}},
    {"gas_r_lsb", Field{0x2b, 0, 8, true}},
    {"gas_r_msb", Field{0x2a, 0, 8, true}},
    {"hum_lsb", Field{0x26, 0, 8, true}},
    {"hum_msb", Field{0x25, 0, 8, true}},
    {"temp_xlsb", Field{0x24, 4, 4, true}},
    {"temp_lsb", Field{0x23, 0, 8, true}},
    {"temp_msb", Field{0x22, 0, 8, true}},
    {"press_xlsb", Field{0x21, 4, 4, true}},
    {"press_lsb", Field{0x20, 0, 8, true}},
    {"press_msb", Field{0x1f, 0, 8, true}},
    {"Id",      Field{0xd0, 0, 8, false}},
    {"chip_id", Field{0xd0, 0, 8, false}},
    {"par_t1",  Field{0xe9, 0, 8, false}},
    {"par_t2", Field{0x8a, 0, 8, false}},
    {"par_t3", Field{0x8c, 0, 8, false}},
    {"par_p1", Field{0x8e, 0, 8, false}},
    {"par_p2", Field{0x90, 0, 8, false}},
    {"par_p3", Field{0x92, 0, 8, false}},
    {"par_p4", Field{0x94, 0, 8, false}},
    {"par_p5", Field{0x96, 0, 8, false}},
    {"par_p6", Field{0x99, 0, 8, false}},
    {"par_p7", Field{0x98, 0, 8, false}},
    {"par_p8", Field{0x9c, 0, 8, false}},
    {"par_p9", Field{0x9e, 0, 8, false}},
    {"par_p10", Field{0xa0, 0, 8, false}},
    {"par_h1", Field{0xe2, 0, 8, false}},
    {"par_h2", Field{0xe1, 0, 8, false}},
    {"par_h3", Field{0xe4, 0, 8, false}},
    {"par_h4", Field{0xe5, 0, 8, false}},
    {"par_h5", Field{0xe6, 0, 8, false}},
    {"par_h6", Field{0xe7, 0, 8, false}},
    {"par_h7", Field{0xe8, 0, 8, false}},
    {"par_g1", Field{0xed, 0, 8, false}},
    {"par_g2", Field{0xeb, 0, 8, false}},
    {"par_g3", Field{0xee, 0, 8, false}},
    {"res_heat_range", Field{0x2, 4, 2, false}},
    {"res_heat_val", Field{0x0, 0, 8, false}},
    {"range_switching_error", Field{0x4, 0, 8, false}},
    {"gas_range_r", Field{0x2b, 0, 4, false}},
    {"heat_stab_r", Field{0x2b, 4, 1, false}},
    {"gas_valid_r", Field{0x2b, 5, 1, false}},
  };

};