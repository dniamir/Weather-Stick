# include <TSL2591_SS.h>
# include <logger.h>

TSL2591_SS::TSL2591_SS(ArduinoI2C input_protocol) : TSL2591(input_protocol) {}

void TSL2591_SS::set_np_interrupt(uint16_t low_thresh, uint16_t high_thresh) {

    TSL2591::write_tsl_field("NPAILTL", low_thresh & 0b11111111);      // No persist low threshold low byte
    TSL2591::write_tsl_field("NPAILTH", low_thresh >> 8);              // No persist low threshold high byte

    TSL2591::write_tsl_field("NPAIHTL", high_thresh & 0b11111111);     // No persist high threshold low byte
    TSL2591::write_tsl_field("NPAIHTH", high_thresh >> 8);             // No persist high threshold high byte

}

void TSL2591_SS::configure_system() {

    LOGGER::write_to_log("TSL", "CONFIGURE SYSTEM");

    TSL2591::initialize();
    TSL2591::enable();
    TSL2591::configure_sensor();

    TSL2591::write_tsl_field("AIEN", 0);  // Disable Persist Interrupt
    TSL2591::write_tsl_field("NPIEN", 1);  // Enable No-Persist Interrupt
    
    TSL2591_SS::set_np_interrupt(0, 6000);

    TSL2591::enable_interrupt();
}

void TSL2591_SS::disable_system() {
    TSL2591::disable();
    TSL2591::write_tsl_field("AIEN", 0);  // Disable Persist Interrupt
    TSL2591::write_tsl_field("NPIEN", 0);  // Disable No-Persist Interrupt
}

void TSL2591_SS::read_data(bool print_data) {

    TSL2591::read_full_luminosity();

    if (!print_data) {return;}

    LOGGER::write_to_log("TSLF", TSL2591::light_fs);
    LOGGER::write_to_log("TSLI", TSL2591::light_ir);
    LOGGER::write_to_log("TSLV", TSL2591::light_vis);
}

int TSL2591_SS::read_interrupt(bool debug) {
    bool interrupt_state = digitalRead(TSL2591_SS::interrupt_pin);
    interrupt_state = !interrupt_state;
    if(debug) {LOGGER::write_to_log("TSL_INT", interrupt_state);}
    if (interrupt_state) {TSL2591::clear_interrupt();}
    return(interrupt_state);
}

