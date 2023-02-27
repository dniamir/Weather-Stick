# include <IS31FL3193.h>
# include <arduino_i2c.h>

class IS31FL3193_SS : public IS31FL3193 {

  public:
    // Constructor
    IS31FL3193_SS(ArduinoI2C input_protocol);

    void configure_one_shot_mode();
    void configure_pwm_mode();
    void low_battery();
    void charging_low_battery();
    void charging_med_battery();
    void charging_high_battery();
    void charging_full_battery();

};