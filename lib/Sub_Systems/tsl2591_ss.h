# include <TSL2591.h>
# include <arduino_i2c.h>

class TSL2591_SS : TSL2591 {

  public:
    // Constructor
    TSL2591_SS(ArduinoI2C input_protocol);

    void configure_system();
    void read_data(bool print_data);
    void set_np_interrupt(uint16_t low_thresh, uint16_t high_thresh);
    int read_interrupt();
    int interrupt_pin;

};