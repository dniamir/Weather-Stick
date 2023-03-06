# include <arduino_i2c.h>
# include <protocol.h>
# include <Arduino.h>
# include <Wire.h>

ArduinoI2C::ArduinoI2C(){};

void ArduinoI2C::write_register(int address, int reg, int data) {

    Wire.beginTransmission(address) ;
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void ArduinoI2C::write_register(int address, int reg, uint8_t data[]) {

    Wire.beginTransmission(address) ;
    Wire.write(reg);
    for(uint8_t idx=0; idx < sizeof(data); idx++) 
        Wire.write(data[idx]);
        // Serial.println(data[idx]);
    Wire.endTransmission();
}

int ArduinoI2C::read_register(int address, int reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, 1);
    uint8_t byte = Wire.read();

    return byte;
}

void ArduinoI2C::read_register(int address, int reg, int bytes_to_read, int8_t reg_out[]) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, bytes_to_read);

    for (int i=0; i<bytes_to_read; ++i) {
        reg_out[i] = Wire.read();
    }
}

void ArduinoI2C::read_register(int address, int reg, int bytes_to_read, uint8_t reg_out[]) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, bytes_to_read);

    for (int i=0; i<bytes_to_read; ++i) {
        reg_out[i] = Wire.read();
    }
}