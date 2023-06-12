# include <IS31FL3193_SS.h>
# include <logger.h>

IS31FL3193_SS::IS31FL3193_SS(ArduinoI2C input_protocol) : IS31FL3193(input_protocol) {}

void IS31FL3193_SS::configure_one_shot_mode() {

    IS31FL3193::soft_reset();
    IS31FL3193::write_register("Reset", 0b0); // Reset register map
    IS31FL3193::write_register("EN", 0b1); // Enable All Channels
    IS31FL3193::write_register("SSD", 0b0); // Disable software shutdown
    IS31FL3193::write_register("RM", 0b1); // Enable ramp mode
    IS31FL3193::write_register("HT", 0b0); // Hold on T2
    IS31FL3193::write_register("BME", 0b1); // Enable breathing mark
    IS31FL3193::write_register("CSS", 0b01); // Enable channel 3  - I don't think does anything?
    IS31FL3193::write_register("RGB", 0b0); // Set to PWM programming mode
    IS31FL3193::write_register("CS", 0b001); // Set current limits on all LEDs
    IS31FL3193::write_register("LED Control", 0b111); // Set current limits on all LEDs
    IS31FL3193::write_led_all();
}

void IS31FL3193_SS::configure_pwm_mode() {

    LOGGER::write_to_log("LED", "CONFIGURE SYSTEM");

    IS31FL3193::soft_reset();
    IS31FL3193::write_register("EN", 0b1); // Enable All Channels
    IS31FL3193::write_register("SSD", 0b0); // Disable software shutdown
    IS31FL3193::write_register("RM", 0b0); // Enable ramp mode
    IS31FL3193::write_register("HT", 0b1); // Hold on T2
    IS31FL3193::write_register("BME", 0b1); // Enable breathing mark
    IS31FL3193::write_register("CSS", 0b10); // Enable channel 3  - Not sure what this is doing?
    IS31FL3193::write_register("RGB", 0b1); // Set to one shot programming mode

    // write_led("CS", 0b001); // Set current limits on all LEDs - 10mA
    IS31FL3193::write_register("CS", 0b000); // Set current limits on all LEDs - 42mA

    IS31FL3193::write_register("LED Control", 0b111); // Set current limits on all LEDs
}

void IS31FL3193_SS::low_battery() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("red", 5, false);
    IS31FL3193::set_timing(0b0000, 0b000, 0b0000, 0b000, 0b0100);
}

void IS31FL3193_SS::charging_low_battery() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("green", 100, false);
    IS31FL3193::set_timing(0b0000, 0b001, 0b0000, 0b001, 0b0000);
}

void IS31FL3193_SS::charging_med_battery() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("green", 100, false);
    IS31FL3193::set_timing(0b0000, 0b001, 0b0001, 0b001, 0b0010);
}

void IS31FL3193_SS::charging_high_battery() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("green", 100, false);
    IS31FL3193::set_timing(0b0000, 0b010, 0b0100, 0b010, 0b0001);
}

void IS31FL3193_SS::charging_full_battery() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("green", 40, false);
    IS31FL3193::set_timing(0b0000, 0b0000, 0b0100, 0b0000, 0b0000);
}

void IS31FL3193_SS::air_quality_bad() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("yellow", 40, false);
    IS31FL3193::set_timing(0b0000, 0b000, 0b0000, 0b000, 0b1000);
}

void IS31FL3193_SS::air_quality_very_bad() {

    IS31FL3193_SS::configure_pwm_mode();
    IS31FL3193::set_color("white", 0, false);
    IS31FL3193::set_color("purple", 40, false);
    IS31FL3193::set_timing(0b0000, 0b000, 0b0000, 0b000, 0b1000);
}

void IS31FL3193_SS::off() {

    IS31FL3193_SS::soft_reset();
}

void IS31FL3193_SS::set_status(uint16_t light_vis, 
                               bool charger_pok,
                               bool charger_status,
                               int16_t level_10_percent,
                               uint16_t co2_ppm,
                               bool debug) {

    bool lights_on = light_vis > 500;
    if(lights_on) {

        bool status1 = charger_pok & ~charger_status;
        bool status2 = charger_pok & charger_status & (level_10_percent > 600);
        bool status3 = charger_pok & charger_status & (level_10_percent > 300);
        bool status4 = charger_pok & charger_status;
        bool status5 = level_10_percent < 100;
        bool status6 = co2_ppm > 1500;
        bool status7 = co2_ppm > 1000;

        if(status1) {
            IS31FL3193_SS::charging_full_battery();
            if(debug) {LOGGER::write_to_log("RGB", "1");}
        } 
        else if(status2) {
            IS31FL3193_SS::charging_high_battery();
            if(debug) {LOGGER::write_to_log("RGB", "2");}
        }
        else if(status3) {
            IS31FL3193_SS::charging_med_battery();
            if(debug) {LOGGER::write_to_log("RGB", "3");}
        }
        else if(status4) {
            IS31FL3193_SS::charging_low_battery();
            if(debug) {LOGGER::write_to_log("RGB", "4");}
        }
        else if(status5) {
            IS31FL3193_SS::low_battery();
            if(debug) {LOGGER::write_to_log("RGB", "5");}
        }
        else if(status6) {
            IS31FL3193_SS::air_quality_very_bad();
            if(debug) {LOGGER::write_to_log("RGB", "6");}
        }
        else if(status7) {
            IS31FL3193_SS::air_quality_bad();
            if(debug) {LOGGER::write_to_log("RGB", "7");}
        }
        else {
            IS31FL3193_SS::off();
            if(debug) {LOGGER::write_to_log("RGB", "0");}
        }
    }
    else {
        IS31FL3193_SS::off();
        if(debug) {LOGGER::write_to_log("RGB", "0");}
    }
}