# include <BME680.h>
# include <arduino_i2c.h>

class BME680_SS : public BME680 {

  public:
    // Constructor
    BME680_SS(ArduinoI2C input_protocol);
    // using BME680::soft_reset;

    void configure_system(uint8_t profile_num=0);
    void read_data(bool print_data);

    // Saved data
    int32_t temperature_100_degc;
    int32_t pressure;
    int32_t humidity_1000;
    int32_t gas;
    int32_t iaq;

};