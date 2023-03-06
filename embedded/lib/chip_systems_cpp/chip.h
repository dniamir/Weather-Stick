# include <arduino_i2c.h>
# include <map>

#ifndef _CHIP_
#define _CHIP_

struct Field{
  uint8_t address;
  uint8_t offset;
  uint8_t length; 
  bool twos_comp;
};

class Chip {   
  // Data members of class
public:
  Chip();
  Chip(ArduinoI2C protocol_in);
  Chip(int i2c_address_in, ArduinoI2C protocol_in);

  // I2C Address
  int i2c_address;

  // Register / Field MAP
  std::map<String, Field> field_map;

  // WHO AM I register - register the check upon startup
  Field who_am_i_reg;

  // Communication protocol
  ArduinoI2C comm_protocol; 

  // Overloading write_field
  void write_field(String field, uint8_t field_val);
  void write_field(uint8_t field, uint8_t field_val, uint8_t offset, uint8_t field_length);
  void write_field(uint8_t field, uint8_t field_val);
  void write_field(uint8_t field, uint8_t field_vals[]);

  // Overloading read_field
  uint8_t read_field(String field);
  uint8_t read_field(int field);
  uint8_t read_field(int field, int offset, int field_length);

  void read_field(int field, int bytes_to_read, uint8_t field_out[]);
  void read_field(int field, int bytes_to_read, int8_t field_out[]);
  void read_field(String field, int bytes_to_read, uint8_t field_out[]);
  void read_field(String field, int bytes_to_read, int8_t field_out[]);

  // Averaging
  float average(float array_in[], int size);

};

#endif