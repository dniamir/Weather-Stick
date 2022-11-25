# include <DallasTemperature.h>
# include <arduino_i2c.h>

class MAX31820_SS : public DallasTemperature {

  public:
    // Constructor
    MAX31820_SS(OneWire &oneWire);

    // Sensor
    void read_data(bool print_data);
    void configure_system();

    // Data
    int16_t temperature_100_degc;
};