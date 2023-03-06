# include <TSL2591.h>
# include <Arduino.h>
# include <arduino_i2c.h>

TSL2591::TSL2591() {}

TSL2591::TSL2591(int i2c_address_in, ArduinoI2C input_protocol) : Chip(i2c_address_in, input_protocol) {
    Chip::field_map = field_map;
    Chip::who_am_i_reg = who_am_i_reg;
};

TSL2591::TSL2591(ArduinoI2C input_protocol) : Chip(input_protocol){
    Chip::field_map = field_map;
    Chip::who_am_i_reg = who_am_i_reg;
    Chip::i2c_address = TSL2591::i2c_address;
};

void TSL2591::reset() {
    TSL2591::write_tsl_field("SRESET", 1);
}

void TSL2591::enable() {
    TSL2591::write_tsl_field("PON", 1);
    TSL2591::write_tsl_field("AEN", 1);
    TSL2591::enabled = true;
}

void TSL2591::disable() {
    TSL2591::write_tsl_field("PON", 0);
    TSL2591::write_tsl_field("AEN", 0);
    TSL2591::enabled = false;
}

bool TSL2591::initialize() {

    // Reset device
    TSL2591::reset();
    delay(200); // WHO AM I will not read if there's no delay

    // Check device_id
    uint8_t check_val = read_tsl_field("ID");

    bool sensor_ok = false;

    if (check_val == who_am_i_val) {
        Serial.println("TSL2591 connection was successful");
        sensor_ok = true;
    }
    else{
        Serial.println("TSL2591 connection was NOT successful");
        sensor_ok = false;
    }

    return sensor_ok;
}

void TSL2591::clear_interrupt() {
    uint8_t dummy = TSL2591::read_field(0b11100111);
}

void TSL2591::enable_interrupt() {
    uint8_t dummy = TSL2591::read_field(0b11100100);
}

void TSL2591::configure_sensor() {
    TSL2591::write_tsl_field("AGAIN", 0b01);
    TSL2591::write_tsl_field("ATIME", 0b010);
}

void TSL2591::read_full_luminosity() {

    bool disable_after = false;

    if (!enabled) {
        enable();
        disable_after = true;
        delay(600);  // Maximum integration time
    }

    // CHAN0 must be read before CHAN1
    // See: https://forums.adafruit.com/viewtopic.php?f=19&t=124176
    uint8_t register_out[4];
    TSL2591::read_tsl_field("C0DATAL", 4, register_out);

    light_fs = (register_out[1] << 8) | register_out[0];
    light_ir = (register_out[3] << 8) | register_out[2];
    light_vis = light_fs - light_ir;
    
    // Disable if originally disabled
    if (disable_after) {disable();};

}

uint8_t TSL2591::read_tsl_field(String field) {
    Field field_to_write = field_map[field];
    return TSL2591::read_field(command_bit | field_to_write.address, field_to_write.offset, field_to_write.length);
}

void TSL2591::read_tsl_field(String field, int bytes_to_read, uint8_t field_out[]) {
    Field field_to_write = field_map[field];
    TSL2591::read_field(command_bit | field_to_write.address, bytes_to_read, field_out);
}

void TSL2591::write_tsl_field(String field, uint8_t val) {
    Field field_to_write = field_map[field];
    TSL2591::write_field(command_bit | field_to_write.address, val, field_to_write.offset, field_to_write.length);
}