# include <IS31FL3193.h>
# include <arduino_i2c.h>

class IS31FL3193_SS : public IS31FL3193 {

  public:
    // Constructor
    IS31FL3193_SS(ArduinoI2C input_protocol);

    void configure_one_shot_mode();
    void configure_pwm_mode();

    void off();                   // State 0
    void low_battery();           // State 1
    void charging_low_battery();  // State 2
    void charging_med_battery();  // State 3
    void charging_high_battery(); // State 4
    void charging_full_battery(); // State 5
    void air_quality_bad();       // State 6
    void air_quality_very_bad();  // State 7

};