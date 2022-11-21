# include <TSL2591.h>
# include <arduino_i2c.h>

class TSL2591_SS {

  public:
    // Constructor
    TSL2591_SS();
    TSL2591_SS(ArduinoI2C input_protocol);

    // Sensor
    TSL2591 tsl2591;

    void configure_system();
    void read_data(bool print_data);
    int read_interrupt();

    int interrupt_pin;

};