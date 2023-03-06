# include <protocol.h>
# include <Arduino.h>
# include <arduino_i2c.h>
# include <chip.h>

class TSL2591 : public Chip {

  public:
  // Different constructors
    TSL2591(int i2c_address_in, ArduinoI2C input_protocol);
    TSL2591(ArduinoI2C input_protocol);
    TSL2591();

    // Dfault I2C address
    int i2c_address = 0x29;

    // WHO AM I register - register the check upon startup
    Field who_am_i_reg = Field{0x12, 0, 8, false};

    // Configure sensor
    void configure_sensor();

    // Interrupt
    void clear_interrupt();
    void enable_interrupt();

    // Special Read/Write protocols for this chip
    uint8_t read_tsl_field(String field);
    void read_tsl_field(String field, int bytes_to_read, uint8_t field_out[]);
    void write_tsl_field(String field, uint8_t val);

    // Initialize the device
    bool initialize();
    bool enabled = false;
    void reset();

    // Initial readings
    uint16_t light_fs = 0;
    uint16_t light_ir = 0;
    uint16_t light_vis = 0;

    // enable and disable
    void enable();
    void disable();

    // Read all light readings from the sensor
    void read_full_luminosity();

  private:

    
    int who_am_i_val = 0x50;  // Value in default WHO AM I register - value to match during startup check
    uint8_t command_bit = 0xA0;  // 1010 0000: bits 7 and 5 for 'command normal'


  protected:

  public:

  // Register Map
  std::map<String, Field> field_map {

    // Enable Register
    {"ENABLE", Field{0x00, 0, 8, false}},
    {"NPIEN", Field{0x00, 7, 1, false}},
    {"SAI", Field{0x00, 6, 1, false}},
    {"AIEN", Field{0x00, 4, 1, false}},
    {"AEN", Field{0x00, 1, 1, false}},
    {"PON", Field{0x00, 0, 1, false}},

    // Control Register
    {"CONTROL", Field{0x01, 0, 8, false}},
    {"ATIME", Field{0x01, 0, 3, false}},
    {"AGAIN", Field{0x01, 4, 2, false}},
    {"SRESET", Field{0x01, 7, 1, false}},

    // ALS Data Register
    {"C0DATAL", Field{0x14, 0, 8, false}},
    {"C0DATAH", Field{0x15, 0, 8, false}},
    {"C1DATAL", Field{0x16, 0, 8, false}},
    {"C1DATAH", Field{0x17, 0, 8, false}},

    // Interrupts and Persists
    {"PERSIST", Field{0x0C, 0, 4, false}},
    {"AILTL", Field{0x04, 0, 8, false}},
    {"AILTH", Field{0x05, 0, 8, false}},
    {"AIHTL", Field{0x06, 0, 8, false}},
    {"AIHTH", Field{0x07, 0, 8, false}},
    {"NPAILTL", Field{0x08, 0, 8, false}},
    {"NPAILTH", Field{0x09, 0, 8, false}},
    {"NPAIHTL", Field{0x0A, 0, 8, false}},
    {"NPAIHTH", Field{0x0B, 0, 8, false}},

    // Status Register
    {"STATUS", Field{0x13, 0, 8, false}},
    {"NPINTR", Field{0x13, 5, 1, false}},
    {"AINT", Field{0x13, 4, 1, false}},
    {"AVALID", Field{0x13, 0, 1, false}},

    // ID Register
    {"ID", Field{0x12, 0, 8, false}},

  };

};