# include <SensirionI2CScd4x.h>
#include <Arduino.h>

class SCD4x_SS : public SensirionI2CScd4x{

  public:
    SCD4x_SS();

    void begin(TwoWire& i2cBus);
    void configure_periodic_mode(TwoWire& i2cBus);
    void set_serial_number();
    String serial_to_string(uint16_t value);
    void read_data(bool print_data);
    void check_ready_flag();
    void print_measurements();

    // One shot experiment
    void wake_up();
    void power_down();
    void read_oneshot();
    void read_oneshot_th();

    // Measurements
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool ready_flag;

    String serial_number;

  private:

  protected:

};