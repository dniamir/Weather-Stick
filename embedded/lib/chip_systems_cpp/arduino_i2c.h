# include <Arduino.h>
# include <protocol.h>
# include <Wire.h>

#ifndef _ARDUINO_I2C_
#define _ARDUINO_I2C_

class ArduinoI2C : public Protocol {   
    // Data members of class
public:
    ArduinoI2C();

    int i2c_address;

    // Base functions
    void write_register(int address, int reg, int data) override;
    void write_register(int address, int reg, uint8_t data[]) override;
    int read_register(int address, int reg) override;
    void read_register(int address, int reg, int bytes_to_read, int8_t reg_out[]) override;
    void read_register(int address, int reg, int bytes_to_read, uint8_t reg_out[]) override;

};

#endif