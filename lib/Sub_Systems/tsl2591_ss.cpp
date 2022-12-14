# include <TSL2591_SS.h>

TSL2591_SS::TSL2591_SS(ArduinoI2C input_protocol) : TSL2591(input_protocol) {}

void TSL2591_SS::set_np_interrupt(uint16_t low_thresh, uint16_t high_thresh) {

    TSL2591::write_tsl_field("NPAILTL", low_thresh & 0b11111111);      // No persist low threshold low byte
    TSL2591::write_tsl_field("NPAILTH", low_thresh >> 8);              // No persist low threshold high byte

    TSL2591::write_tsl_field("NPAIHTL", high_thresh & 0b11111111);     // No persist high threshold low byte
    TSL2591::write_tsl_field("NPAIHTH", high_thresh >> 8);             // No persist high threshold high byte

}

void TSL2591_SS::configure_system() {

    TSL2591::initialize();
    TSL2591::enable();
    TSL2591::configure_sensor();

    TSL2591::write_tsl_field("AIEN", 0);  // Disable Persist Interrupt
    TSL2591::write_tsl_field("NPIEN", 1);  // Enable No-Persist Interrupt
    
    TSL2591_SS::set_np_interrupt(0, 6000);

    TSL2591::enable_interrupt();
}

void TSL2591_SS::read_data(bool print_data) {

    TSL2591::read_full_luminosity();

    if (!print_data) {return;}

    Serial.print("Full Scale: ");
    Serial.print(TSL2591::light_fs);
    Serial.print(", Infrared: ");
    Serial.print(TSL2591::light_ir);
    Serial.print(", Visual: ");
    Serial.print(TSL2591::light_vis);
    Serial.println();
}

int TSL2591_SS::read_interrupt() {
    bool interrupt_state = digitalRead(TSL2591_SS::interrupt_pin);
    interrupt_state = !interrupt_state;
    if (interrupt_state) {TSL2591::clear_interrupt();}
    return(interrupt_state);
}

