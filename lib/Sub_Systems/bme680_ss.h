# include <BME680.h>
# include <arduino_i2c.h>

class BME680_SS : BME680 {

  public:
    // Constructor
    BME680_SS(ArduinoI2C input_protocol);
    using BME680::soft_reset;

    void configure_system(uint8_t profile_num=0);
    void read_data(bool print_data);

};