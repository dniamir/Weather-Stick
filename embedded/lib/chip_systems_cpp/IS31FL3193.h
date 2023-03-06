# include <protocol.h>
# include <Arduino.h>
# include <arduino_i2c.h>
# include <chip.h>


class IS31FL3193 : public Chip {

  public:
    // Different constructors
    IS31FL3193(ArduinoI2C input_protocol);
    IS31FL3193(int i2c_address_in, ArduinoI2C input_protocol);

    int i2c_address = 0x68;

    // Usage functions
    void set_color(String color, uint8_t brightness, bool update);
    void set_timing(uint8_t t0, uint8_t t1, uint8_t t2, uint8_t t3, uint8_t t4);

    // Lower level functions
    void write_register(String register, uint8_t field_val);
    void write_led_all();
    void update();
    void soft_reset();


  private:

    // Value in default WHO AM I register - value to match during startup check
    int who_am_i_val = 0b00000001;
    Field who_am_i_reg = Field{0x00, 0, 8, false};

    // Setup internal register map
    std::map<uint8_t, uint8_t> current_registers {};
    void setup_register();

  public:

  // I2C Register Map - SPI Map is different
  std::map<String, Field> field_map {
    
    {"Shutdown", Field{0x00, 0, 8, true}},
    {"EN", Field{0x00, 5, 1, true}},
    {"SSD", Field{0x00, 0, 1, true}},

    {"Breathing Control", Field{0x01, 0, 8, true}},
    {"RM", Field{0x01, 5, 1, true}},
    {"HT", Field{0x01, 4, 1, true}},
    {"BME", Field{0x01, 2, 1, true}},
    {"CSS", Field{0x01, 0, 2, true}},

    {"LED Mode", Field{0x02, 0, 8, true}},
    {"RGB", Field{0x02, 5, 1, true}},

    {"Current Setting", Field{0x03, 0, 8, true}},
    {"CS", Field{0x03, 2, 3, true}},

    {"PWM1", Field{0x04, 0, 8, true}},
    {"PWM2", Field{0x05, 0, 8, true}},
    {"PWM3", Field{0x06, 0, 8, true}},

    {"PWM Update", Field{0x07, 0, 8, true}},
    
    {"T01", Field{0x0A, 4, 4, true}},
    {"T02", Field{0x0B, 4, 4, true}},
    {"T03", Field{0x0C, 4, 4, true}},

    {"T11", Field{0x10, 5, 3, true}},
    {"T12", Field{0x11, 5, 3, true}},
    {"T13", Field{0x12, 5, 3, true}},

    {"T21", Field{0x10, 1, 4, true}},
    {"T22", Field{0x11, 1, 4, true}},
    {"T23", Field{0x12, 1, 4, true}},

    {"T31", Field{0x16, 5, 3, true}},
    {"T32", Field{0x17, 5, 3, true}},
    {"T33", Field{0x18, 5, 3, true}},

    {"T41", Field{0x16, 1, 4, true}},
    {"T42", Field{0x17, 1, 4, true}},
    {"T43", Field{0x18, 1, 4, true}},

    {"Time Update", Field{0x1C, 0, 8, true}},

    {"LED Control", Field{0x1D, 0, 3, true}},

    {"Reset", Field{0x2F, 0, 3, true}},

  };

};