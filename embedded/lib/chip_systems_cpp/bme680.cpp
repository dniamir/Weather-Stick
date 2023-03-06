# include <bme680.h>
# include <Arduino.h>
# include <arduino_i2c.h>

BME680::BME680(int i2c_address_in, ArduinoI2C input_protocol) : Chip(i2c_address_in, input_protocol) {
    Chip::field_map = field_map;
    Chip::who_am_i_reg = who_am_i_reg;
};

BME680::BME680(ArduinoI2C input_protocol) : Chip(input_protocol) {
    Chip::field_map = field_map;
    Chip::who_am_i_reg = who_am_i_reg;
    Chip::i2c_address = BME680::i2c_address;
};

void BME680::soft_reset() {
    BME680::write_field("reset", 0xB6);
}

void BME680::read_cal_codes() {
    BME680::cal_codes.par_t1 = BME680::read_field("par_t1") | (BME680::read_field(0xea) << 8);
    BME680::cal_codes.par_t2 = BME680::read_field("par_t2") | (BME680::read_field(0x8b) << 8);
    BME680::cal_codes.par_t3 = BME680::read_field("par_t3");

    BME680::cal_codes.par_p1 = BME680::read_field("par_p1") | (BME680::read_field(0x8f) << 8);
    BME680::cal_codes.par_p2 = BME680::read_field("par_p2") | (BME680::read_field(0x91) << 8);
    BME680::cal_codes.par_p3 = BME680::read_field("par_p3");
    BME680::cal_codes.par_p4 = BME680::read_field("par_p4") | (BME680::read_field(0x95) << 8);
    BME680::cal_codes.par_p5 = BME680::read_field("par_p5") | (BME680::read_field(0x97) << 8);
    BME680::cal_codes.par_p6 = BME680::read_field("par_p6");
    BME680::cal_codes.par_p7 = BME680::read_field("par_p7");
    BME680::cal_codes.par_p8 = BME680::read_field("par_p8") | (BME680::read_field(0x9d) << 8);
    BME680::cal_codes.par_p9 = BME680::read_field("par_p9") | (BME680::read_field(0x9f) << 8);
    BME680::cal_codes.par_p10 = BME680::read_field("par_p10");

    BME680::cal_codes.par_h1 = (BME680::read_field("par_h1") & 0b1111) | (BME680::read_field(0xe3) << 4);
    BME680::cal_codes.par_h2 = (BME680::read_field("par_h2") << 4) | (BME680::read_field(0xe2) >> 4);
    BME680::cal_codes.par_h3 = BME680::read_field("par_h3");
    BME680::cal_codes.par_h4 = BME680::read_field("par_h4");
    BME680::cal_codes.par_h5 = BME680::read_field("par_h5");
    BME680::cal_codes.par_h6 = BME680::read_field("par_h6");
    BME680::cal_codes.par_h7 = BME680::read_field("par_h7");

    BME680::cal_codes.par_g1 = BME680::read_field("par_g1");
    BME680::cal_codes.par_g2 = BME680::read_field("par_g2") | (BME680::read_field(0xec) << 8);
    BME680::cal_codes.par_g3 = BME680::read_field("par_g3");
}

int32_t BME680::read_temperature() {

    BME680::write_field("mode", 0b01);

    uint8_t temp_out[3];
    BME680::read_field("temp_msb", 3, temp_out);

    // Temperature conversion is only 20 bits if filter is enabled
    // Need to update if filter is turned off
    uint32_t temp_adc = (temp_out[0] << 12) | (temp_out[1] << 4) | (temp_out[2] >> 4);
    int32_t temp_comp = BME680::calibrate_temperature(temp_adc);

    return temp_comp;
}

int32_t BME680::read_humidity() {

    BME680::write_field("mode", 0b01);

    uint8_t humid_out[2];
    BME680::read_field("hum_msb", 2, humid_out);
    uint16_t humid_adc = (humid_out[0] << 8) | (humid_out[1]);
    int32_t humid_comp = BME680::calibrate_humidity(humid_adc);

    return humid_comp;
}

int32_t BME680::read_pressure() {

    BME680::write_field("mode", 0b01);

    uint8_t press_out[3];
    BME680::read_field("press_msb", 3, press_out);
    uint32_t press_adc = (press_out[0] << 12) | (press_out[1] << 4) | (press_out[2] >> 4);
    int32_t press_comp = BME680::calibrate_pressure(press_adc);
    
    return press_comp;
}

int32_t BME680::read_gas() {
    
    BME680::write_field("mode", 0b01); // Take a forced measurement

    uint8_t gas_out[2];
    BME680::read_field("gas_r_msb", 2, gas_out);
    uint32_t gas_adc = (gas_out[0] << 2) | (gas_out[1] >> 6);

    // Intermediates
    int64_t range_switching_error = BME680::read_field("range_switching_error");
    uint8_t gas_range = BME680::read_field("gas_range_r");

    int64_t const_array1_int[16];
    int64_t const_array2_int[16];
    for(int i=0; i < 16; ++i) {
        const_array1_int[i] = BME680::cal_codes.const_array1_int[i];
        const_array2_int[i] = BME680::cal_codes.const_array2_int[i];
    }

    int64_t var1 = (int64_t)(((1340 + (5 * (int64_t)range_switching_error)) * ((int64_t)const_array1_int[gas_range])) >> 16);
    int64_t var2 = (int64_t)(gas_adc << 15) - (int64_t)(1 << 24) + var1;
    int32_t gas_res = (int32_t)((((int64_t)(const_array2_int[gas_range] * (int64_t)var1) >> 9) + (var2 >> 1)) / var2);

    return gas_res;

}

int16_t BME680::calculate_iaq(int32_t gas_reading, int32_t humidity_reading) {

    int8_t hum_reference = 40;
    int16_t humidity_score;

    // Humidity Score
    if (humidity_reading >= 38 && humidity_reading <= 42) {
        humidity_score = 0.25 * 100; // Humidity +/-5% around optimum 
    }
    else { //sub-optimal
        if (humidity_reading < 38) 
            humidity_score = 0.25 / hum_reference * humidity_reading * 100;
        else
        {
            humidity_score = ((-0.25 / (100 - hum_reference) * humidity_reading) + 0.416666) * 100;
        }
    }

    uint16_t gas_lower_limit = 5000;   // Bad air quality limit
    uint16_t gas_upper_limit = 50000;  // Good air quality limit 
    if (gas_reading > gas_upper_limit) gas_reading = gas_upper_limit; 
    if (gas_reading < gas_lower_limit) gas_reading = gas_lower_limit;
    int16_t gas_score = (0.75 / (gas_upper_limit - gas_lower_limit) * gas_reading - (gas_lower_limit * (0.75 / (gas_upper_limit-gas_lower_limit)))) * 100;

    int16_t iaq = humidity_score + gas_score;
    iaq = (100 - iaq) * 5;
    return iaq;
}

void BME680::set_gas_wait(int16_t wait_time, uint8_t profile_num) {
    if (profile_num == 0) {BME680::write_field("gas_wait_0", wait_time);}
    else if (profile_num == 1) {BME680::write_field("gas_wait_1", wait_time);}
    else if (profile_num == 2) {BME680::write_field("gas_wait_2", wait_time);}
    else if (profile_num == 3) {BME680::write_field("gas_wait_3", wait_time);}
    else if (profile_num == 4) {BME680::write_field("gas_wait_4", wait_time);}
    else if (profile_num == 5) {BME680::write_field("gas_wait_5", wait_time);}
    else if (profile_num == 6) {BME680::write_field("gas_wait_6", wait_time);}
    else if (profile_num == 7) {BME680::write_field("gas_wait_7", wait_time);}
    else if (profile_num == 8) {BME680::write_field("gas_wait_9", wait_time);}
    else if (profile_num == 9) {BME680::write_field("gas_wait_9", wait_time);}
}

void BME680::set_heater_temp(int16_t target_temp, uint8_t profile_num) {
    // Heater num must be 0 through 9

    // Get Calibration Codes
    int8_t par_g1 = BME680::cal_codes.par_g1;
    int16_t par_g2 = BME680::cal_codes.par_g2;
    int8_t par_g3 = BME680::cal_codes.par_g3;

    int32_t temp_comp = BME680::temp_comp;
    if (temp_comp == 0) {BME680::read_temperature();} 
    temp_comp = BME680::temp_comp;
    int32_t amb_temp = temp_comp / 100;

    // Intermediates
    uint8_t res_heat_range = BME680::read_field("res_heat_range"); 
    int8_t res_heat_val = BME680::read_field("res_heat_val"); 

    int32_t  var1 = (((int32_t)amb_temp * par_g3) / 10) << 8;
    int32_t  var2 = (par_g1 + 784) * (((((par_g2 + 154009) * target_temp * 5) / 100) + 3276800) / 10);
    int32_t  var3 = var1 + (var2 >> 1);
    int32_t  var4 = (var3 / (res_heat_range + 4));
    int32_t  var5 = (131 * res_heat_val) + 65536;
    int32_t  res_heat_x100 = (int32_t)(((var4 / var5) - 250) * 34);
    uint8_t res_heat_x = (uint8_t)((res_heat_x100 + 50) / 100);

    // Set heater temperature - takes 20-30ms to reach temperature
    if (profile_num == 0) {BME680::write_field("res_heat_0", res_heat_x);}
    else if (profile_num == 1) {BME680::write_field("res_heat_1", res_heat_x);}
    else if (profile_num == 2) {BME680::write_field("res_heat_2", res_heat_x);}
    else if (profile_num == 3) {BME680::write_field("res_heat_3", res_heat_x);}
    else if (profile_num == 4) {BME680::write_field("res_heat_4", res_heat_x);}
    else if (profile_num == 5) {BME680::write_field("res_heat_5", res_heat_x);}
    else if (profile_num == 6) {BME680::write_field("res_heat_6", res_heat_x);}
    else if (profile_num == 7) {BME680::write_field("res_heat_7", res_heat_x);}
    else if (profile_num == 8) {BME680::write_field("res_heat_8", res_heat_x);}
    else if (profile_num == 9) {BME680::write_field("res_heat_9", res_heat_x);}

}

int32_t BME680::calibrate_temperature(uint32_t temp_adc) {

    // Get Calibration Codes
    int16_t par_t1 = BME680::cal_codes.par_t1;
    int16_t par_t2 = BME680::cal_codes.par_t2;
    uint16_t par_t3 = BME680::cal_codes.par_t3;

    int64_t var1;
    int64_t var2;
    int64_t var3;
    int16_t calc_temp;

    // Calibrate temperature measurements - Copied from datasheet
    var1 = ((int32_t)temp_adc >> 3) - ((int32_t)par_t1 << 1);
    var2 = (var1 * (int32_t)par_t2) >> 11;
    var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14;
    int32_t t_fine = var2 + var3;
    int32_t temp_comp = ((t_fine * 5) + 128) >> 8;

    // Save intermeidate calibration values
    BME680::t_fine = t_fine;
    BME680::temp_comp = temp_comp;

    return temp_comp;

}

int32_t BME680::calibrate_pressure(uint32_t press_adc) {

    // Get Calibration Codes
    uint16_t par_p1 = BME680::cal_codes.par_p1;
    int16_t par_p2 = BME680::cal_codes.par_p2;
    int8_t par_p3 = BME680::cal_codes.par_p3;
    int16_t par_p4 = BME680::cal_codes.par_p4;
    int16_t par_p5 = BME680::cal_codes.par_p5;
    int8_t par_p6 = BME680::cal_codes.par_p6;
    int8_t par_p7 = BME680::cal_codes.par_p7;
    int16_t par_p8 = BME680::cal_codes.par_p8;
    int16_t par_p9 = BME680::cal_codes.par_p9;
    uint8_t par_p10 = BME680::cal_codes.par_p10;

    // Intermediates
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t press_comp;

    int32_t t_fine = BME680::t_fine;

    // Calibrate pressure measurements - Copied from datasheet
    var1 = ((int32_t)t_fine >> 1) - 64000;
    var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)par_p6) >> 2;
    var2 = var2 + ((var1 * (int32_t)par_p5) << 1);
    var2 = (var2 >> 2) + ((int32_t)par_p4 << 16);
    var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) *
    ((int32_t)par_p3 << 5)) >> 3) + (((int32_t)par_p2 * var1) >> 1);

    var1 = var1 >> 18;
    var1 = ((32768 + var1) * (int32_t)par_p1) >> 15;
    press_comp = 1048576 - press_adc;
    press_comp = (uint32_t)((press_comp - (var2 >> 12)) * ((uint32_t)3125));
    if (press_comp >= (1 << 30))
    press_comp = ((press_comp / (uint32_t)var1) << 1);
    else
    press_comp = ((press_comp << 1) / (uint32_t)var1);
    var1 = ((int32_t)par_p9 * (int32_t)(((press_comp >> 3) *
    (press_comp >> 3)) >> 13)) >> 12;
    var2 = ((int32_t)(press_comp >> 2) * (int32_t)par_p8) >> 13;
    var3 = ((int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) *
    (int32_t)(press_comp >> 8) * (int32_t)par_p10) >> 17;
    press_comp = (int32_t)(press_comp) +
    ((var1 + var2 + var3 + ((int32_t)par_p7 << 7)) >> 4);

    return press_comp;

}

int32_t BME680::calibrate_humidity(uint16_t humid_adc) {

    // Get Calibration Codes
    uint16_t par_h1 = BME680::cal_codes.par_h1;
    uint16_t par_h2 = BME680::cal_codes.par_h2;
    int8_t par_h3 = BME680::cal_codes.par_h3;
    int8_t par_h4 = BME680::cal_codes.par_h4;
    int8_t par_h5 = BME680::cal_codes.par_h5;
    uint8_t par_h6 = BME680::cal_codes.par_h6;
    int8_t par_h7 = BME680::cal_codes.par_h7;

    // Intermediates
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    int32_t var5;
    int32_t var6;
    int32_t humid_comp;

    int32_t temp_comp = BME680::temp_comp;

    // Calibrate temperature measurements - Copied from datasheet
    int32_t temp_scaled = (int32_t)temp_comp;
    var1 = (int32_t)humid_adc - (int32_t)((int32_t)par_h1 << 4) -
        (((temp_scaled * (int32_t)par_h3) / ((int32_t)100)) >> 1);
    var2 = ((int32_t)par_h2 * (((temp_scaled *
        (int32_t)par_h4) / ((int32_t)100)) +
        (((temp_scaled * ((temp_scaled * (int32_t)par_h5) /
        ((int32_t)100))) >> 6) / ((int32_t)100)) + ((int32_t)(1 << 14)))) >> 10;
    var3 = var1 * var2;
    var4 = (((int32_t)par_h6 << 7) +
        ((temp_scaled * (int32_t)par_h7) / ((int32_t)100))) >> 4;
    var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
    var6 = (var4 * var5) >> 1;
    humid_comp = (((var3 + var6) >> 10) * ((int32_t) 1000)) >> 12;

    return humid_comp;

}