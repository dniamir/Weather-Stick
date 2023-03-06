# include <protocol.h>
# include <Arduino.h>
# include <arduino_i2c.h>
# include <chip.h>


class MAX17260 : public Chip {

  public:
    // Different constructors
    MAX17260(ArduinoI2C input_protocol);
    MAX17260(int i2c_address_in, ArduinoI2C input_protocol);

    int i2c_address = 0x36;
    int charger_ok_pin;

    // WHO AM I register - register the check upon startup
    Field who_am_i_reg = Field{0x21, 0, 8, false};

    void configure_system();
    void soft_reset();  // Soft reset - requires 45s
    bool check_por();

    float read_level_percent();
    float read_level_mahrs();
    float read_tte();
    float read_ttf();
    float read_current(bool avg=false);
    float read_temperature(bool avg=false);

    float read_batt_voltage();
    bool read_charge_source_ok();

    void read_data(bool print_data);

    // Overloading write_field
    void write_field16(String field, uint16_t field_val);
    void write_field16(uint8_t field, uint16_t field_val, uint8_t offset, uint8_t field_length);
    void write_field16(uint8_t field, uint16_t field_val);

    // Overloading read_field
    uint16_t read_field16(String field);
    uint16_t read_field16(uint8_t field);
    uint16_t read_field16(uint8_t field, uint8_t offset, uint8_t field_length);

    // Save important battery readings
    float level_percent;
    float level_mah;
    float batt_voltage;
    float avg_current_ma;


  private:

    // Value in default WHO AM I register - value to match during startup check
    int who_am_i_val = 0x61;

    uint16_t DesignCap = 0x1450;
    uint16_t IchgTerm = 0x333;
    uint16_t VEmpty = 0xa561;
    // uint16_t DesignCap = 0x1450;

    // Conversions
    float sec_per_lsb = 5.625;
    float per_per_lsb = 1.0 / 256.0;
    float mah_per_lsb = 0.5;
    float ua_per_lsb = 156.25;
    float v_per_lsb =  78.125 * 1e-6;
    float v_empty_per_lsb = 10.0 * 1e-3;  // V / LSB
    float v_recovery_per_lsb = 40.0 * 1e-3;  // V / LSB
    float i_term_per_lsb = 1.0 / 6.4;  // mA / LSB
    float degc_per_lsb = 1.0 / 256.0;


  protected:

  public:

  // I2C Register Map - SPI Map is different
  std::map<String, Field> field_map {
    
    {"Status", Field{0x00, 0, 16, false}},
    {"Br", Field{0x00, 15, 1, false}},
    {"Smx", Field{0x00, 14, 1, false}},
    {"Tmx", Field{0x00, 13, 1, false}},
    {"Vmx", Field{0x00, 12, 1, false}},
    {"Bi", Field{0x00, 11, 1, false}},
    {"Smn", Field{0x00, 10, 1, false}},
    {"Tmn", Field{0x00, 9, 1, false}},
    {"Vmn", Field{0x00, 8, 1, false}},
    {"dSOCi", Field{0x00, 7, 1, false}},
    {"Imx", Field{0x00, 6, 1, false}},
    {"Bst", Field{0x00, 3, 1, false}},
    {"Imn", Field{0x00, 2, 1, false}},
    {"POR", Field{0x00, 1, 1, false}},

    {"Current", Field{0x0A, 0, 16, false}},
    {"AvgCurrent", Field{0x0B, 0, 16, false}},

    {"Temperature", Field{0x08, 0, 16, false}},
    {"AvgTemperature", Field{0x16, 0, 16, false}},

    {"VAlrtTh", Field{0x01, 0, 16, false}},

    {"IChgTerm", Field{0x1E, 0, 16, false}},
    {"DesignCap", Field{0x18, 0, 16, false}},

    {"VEmpty", Field{0x03A, 0, 16, false}},
    {"VE", Field{0x03A, 7, 9, false}},
    {"VR", Field{0x01, 0, 7, false}},
    {"VAlrtTh", Field{0x01, 0, 16, false}},

    {"ModelCFG", Field{0xDB, 0, 16, false}},
    {"Refresh", Field{0xDB, 15, 1, false}},
    {"R100", Field{0xDB, 13, 1, false}},
    {"VChg", Field{0xDB, 10, 1, false}},
    {"ModelID", Field{0xDB, 4, 4, false}},
    {"CSEL", Field{0xDB, 2, 1, false}},

    {"Config", Field{0x1D, 0, 16, false}},
    {"TSel", Field{0x1D, 15, 1, false}},
    {"SS", Field{0x1D, 14, 1, false}},
    {"TS", Field{0x1D, 13, 1, false}},
    {"VS", Field{0x1D, 12, 1, false}},
    {"IS", Field{0x1D, 11, 1, false}},
    {"THSH", Field{0x1D, 10, 1, false}},
    {"Ten", Field{0x1D, 9, 1, false}},
    {"Tex", Field{0x1D, 8, 1, false}},
    {"SHDN", Field{0x1D, 7, 1, false}},
    {"COMMSH", Field{0x1D, 6, 1, false}},
    {"ETHRM", Field{0x1D, 4, 1, false}},
    {"FTHRM", Field{0x1D, 3, 1, false}},
    {"Aen", Field{0x1D, 2, 1, false}},
    {"Bei", Field{0x1D, 1, 1, false}},
    {"Ber", Field{0x1D, 0, 1, false}},

    {"Config2", Field{0xBB, 0, 16, false}},
    {"AltRateEn", Field{0xBB, 13, 1, false}},
    {"DPEn", Field{0xBB, 12, 1, false}},
    {"POWR", Field{0xBB, 8, 4, false}},
    {"dSOCen", Field{0xBB, 7, 1, false}},
    {"TAIrtEn", Field{0xBB, 6, 1, false}},
    {"LDMdl", Field{0xBB, 5, 1, false}},
    {"DRCfg", Field{0xBB, 2, 2, false}},
    {"CPMode", Field{0xBB, 1, 1, false}},

    {"RepCap", Field{0x05, 0, 16, false}},
    {"RepSOC", Field{0x06, 0, 16, false}},
    {"FullCapRep", Field{0x10, 0, 16, false}},
    {"TTE", Field{0x11, 0, 16, false}},
    {"TTF", Field{0x20, 0, 16, false}},

    {"VCell", Field{0x09, 0, 16, false}},
    {"AvgVCell", Field{0x19, 0, 16, false}},
    {"MaxMinVolt", Field{0xFF, 0, 16, false}},
    {"MaxVCELL", Field{0xFF, 8, 8, false}},
    {"MinVCELL", Field{0xFF, 0, 8, false}},

    {"FStat", Field{0x3D, 0, 16, false}},
    {"HibCfg", Field{0xBA, 0, 16, false}},

  };

};