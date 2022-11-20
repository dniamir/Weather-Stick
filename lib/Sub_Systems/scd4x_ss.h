# include <SensirionI2CScd4x.h>
#include <Arduino.h>
// #include <Wire.h>

class SCD4x_SS {

  public:
    SCD4x_SS();
    SCD4x_SS(TwoWire& i2cBus);

    SensirionI2CScd4x scd4x;

    void begin(TwoWire& i2cBus);
    void configure_system();
    void set_serial_number();
    String serial_to_string(uint16_t value);
    void read_data();
    void check_ready_flag();

    // Measurements
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool ready_flag;

    String serial_number;

  private:

  protected:

};