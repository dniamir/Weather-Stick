# include <IS31FL3193.h>
# include <Arduino.h>
# include <arduino_i2c.h>
#include <math.h>

IS31FL3193::IS31FL3193(int i2c_address_in, ArduinoI2C input_protocol) : Chip(i2c_address_in, input_protocol) {
    Chip::field_map = field_map;
    Chip::who_am_i_reg = who_am_i_reg;
    IS31FL3193::setup_register();
}

IS31FL3193::IS31FL3193(ArduinoI2C input_protocol) : Chip(input_protocol) {
    Chip::field_map = field_map;
    Chip::who_am_i_reg = who_am_i_reg;
    Chip::i2c_address = IS31FL3193::i2c_address;
    IS31FL3193::setup_register();
}

void IS31FL3193::setup_register() {
     uint8_t num_registers = IS31FL3193::field_map.size();
     for(uint8_t i=0; i < num_registers; i++) {
        IS31FL3193::current_registers[i] = 0b00000000;
    }
}

void IS31FL3193::soft_reset() {
    IS31FL3193::write_field(0x2f, (uint8_t)0);
    IS31FL3193::setup_register();
}

void IS31FL3193::write_register(String register_address, uint8_t field_val) {
    if (field_map.count(register_address) == 1) {
        uint8_t reg_address = IS31FL3193::field_map[register_address].address;
        uint8_t field_offset = IS31FL3193::field_map[register_address].offset;
        uint8_t field_length = IS31FL3193::field_map[register_address].length;
        uint8_t curr_field_val = IS31FL3193::current_registers[reg_address];

        // Zero-ing mask
        uint8_t mask1 = pow(2, field_length - 1);
        mask1 = mask1 << field_offset;
        mask1 = (uint8_t)(pow(2, 8) - 1) & ~mask1;
        
        // Mask adding actual field_val
        uint8_t mask2 = field_val << field_offset;
        
        // Final masking
        uint8_t field_val = (curr_field_val & mask1) | mask2;


        IS31FL3193::current_registers[reg_address] = field_val;
    }
}

void IS31FL3193::write_led_all() {
    for (const auto &key_value : IS31FL3193::current_registers) {
        uint8_t key = key_value.first;
        uint8_t value = key_value.second;
        if (key == 0x2F) { continue; }  // Reset Register
        if (key == 0x07) { continue; }  // Load PWM registers and LED control registers
        if (key == 0x1C) { continue; }  // Update time registers
        uint8_t value_temp[2];
        value_temp[0] = value;
        value_temp[1] = 0;
        IS31FL3193::write_field(key, value_temp);
    }
}

void IS31FL3193::update() {
    IS31FL3193::write_field(0x1C, (uint8_t)0b0);  // Update time registers
    IS31FL3193::write_field(0x1D, (uint8_t)0b111);  // Enable LED controls
    // interface.WriteRegister16(0x07, 0);  // Load PWM registers and LED control registers DATA
}

void IS31FL3193::set_color(String color, uint8_t brightness, bool update) {
    color.toUpperCase();

    brightness = uint8_t(brightness * 255 / 100);

    if (color == "RED") { write_register("PWM1", brightness); }  // Write PWM settings for Red Light
    else if (color == "GREEN") { IS31FL3193::write_register("PWM2", brightness); }  // Write PWM settings for Green Light
    else if (color == "BLUE") { IS31FL3193::write_register("PWM3", brightness); }  // Write PWM settings for Blue Light

    else if (color == "YELLOW") { 
        IS31FL3193::write_register("PWM1", brightness);
        IS31FL3193::write_register("PWM2", brightness); 
    } 
    else if (color == "PURPLE") { 
        IS31FL3193::write_register("PWM1", brightness);
        IS31FL3193::write_register("PWM3", brightness); 
    } 
    else if (color == "TEAL") { 
        IS31FL3193::write_register("PWM2", brightness);
        IS31FL3193::write_register("PWM3", brightness); 
    } 
    else if (color == "ALL") { 
        IS31FL3193::write_register("PWM1", brightness);
        IS31FL3193::write_register("PWM2", brightness); 
        IS31FL3193::write_register("PWM3", brightness); 
    } 
    else if (color == "WHITE") { 
        IS31FL3193::write_register("PWM1", brightness);
        IS31FL3193::write_register("PWM2", brightness); 
        IS31FL3193::write_register("PWM3", brightness); 
    } 

    if (update) {
        IS31FL3193::write_led_all();
        IS31FL3193::update();
    }
}

void IS31FL3193::set_timing(uint8_t t0, uint8_t t1, uint8_t t2, uint8_t t3, uint8_t t4) {

    IS31FL3193::write_register("T01", t0);  // Set T0 in shot mode
    IS31FL3193::write_register("T02", t0);  // Set T0 in shot mode
    IS31FL3193::write_register("T03", t0);  // Set T0 in shot mode

    IS31FL3193::write_register("T11", t1);  // Set T1 in shot mode
    IS31FL3193::write_register("T12", t1);  // Set T1 in shot mode
    IS31FL3193::write_register("T13", t1);  // Set T1 in shot mode

    IS31FL3193::write_register("T21", t2);  // Set T2 in shot mode
    IS31FL3193::write_register("T22", t2);  // Set T2 in shot mode
    IS31FL3193::write_register("T23", t2);  // Set T2 in shot mode

    IS31FL3193::write_register("T31", t3);  // Set T3 in shot mode
    IS31FL3193::write_register("T32", t3);  // Set T3 in shot mode
    IS31FL3193::write_register("T33", t3);  // Set T3 in shot mode

    IS31FL3193::write_register("T41", t4);  // Set T4 in shot mode
    IS31FL3193::write_register("T42", t4);  // Set T4 in shot mode
    IS31FL3193::write_register("T43", t4);  // Set T4 in shot mode

    IS31FL3193::write_led_all();
    IS31FL3193::update();

}

