# include <SCD4x_SS.h>
# include <logger.h>

SCD4x_SS::SCD4x_SS() : SensirionI2CScd4x() {};

void SCD4x_SS::begin(TwoWire& i2cBus) {

    LOGGER::write_to_log("SCD", "CONFIGURE SYSTEM");

    // Start I2C Measurement
    SensirionI2CScd4x::begin(i2cBus);

    // Stop Previous Measurements if they were ongoing
    uint16_t error = SensirionI2CScd4x::stopPeriodicMeasurement();
    if (error) {
        LOGGER::write_to_log("SCD", "STOP PERIODIC FAILED");
    }
}

void SCD4x_SS::set_serial_number() {
    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    uint16_t error = SensirionI2CScd4x::getSerialNumber(serial0, serial1, serial2);
    if (error) {
        LOGGER::write_to_log("SCD", "SRN RETRIEVAL FAILED");
    } else {
        Serial.print("Serial: 0x");
        String string1 = SCD4x_SS::serial_to_string(serial0);
        String string2 = SCD4x_SS::serial_to_string(serial1);
        String string3 = SCD4x_SS::serial_to_string(serial2);
        SCD4x_SS::serial_number = String(string1) + String(string2) + String(string3);
        Serial.println(serial_number);
    }
}

String SCD4x_SS::serial_to_string(uint16_t value) {
    String string1 = value < 4096 ? "0" : "";
    String string2 = value < 256 ? "0" : "";
    String string3 = value < 16 ? "0" : "";
    return string1 + string2 + string3;
}

void SCD4x_SS::configure_periodic_mode(TwoWire& i2cBus) {
    SCD4x_SS::begin(i2cBus);
    uint16_t error = SensirionI2CScd4x::startPeriodicMeasurement();
    if (error) {
        LOGGER::write_to_log("SCD", "STOP PERIODIC FAILED");
    }
    LOGGER::write_to_log("SCD", "WAITING FOR PERIODIC MEASUREMENT...");
    // delay(5000);
}

void SCD4x_SS::wake_up() {
    uint16_t error = SensirionI2CScd4x::wakeUp();
    if (error) {
        LOGGER::write_to_log("SCD", "WAKE FAILED");
    }
}

void SCD4x_SS::power_down() {
    uint16_t error = SensirionI2CScd4x::powerDown();
    if (error) {
        LOGGER::write_to_log("SCD", "POWER DOWN FAILED");
    }
}

void SCD4x_SS::read_oneshot() {
    uint16_t error = SensirionI2CScd4x::measureSingleShot();
    if (error) {
        LOGGER::write_to_log("SCD", "ONE SHOT FAILED");
    }
}

void SCD4x_SS::read_oneshot_th() {
    uint16_t error = SensirionI2CScd4x::measureSingleShotRhtOnly();
    if (error) {
        LOGGER::write_to_log("SCD", "ONE SHOT FAILED");
    }
}

void SCD4x_SS::read_data(bool print_data) {

    // Check that data is ready
    SCD4x_SS::check_ready_flag();

    if (!SCD4x_SS::ready_flag) {
        return;
    }

    // Set up temporary variables
    uint16_t co2_temp = 0;
    float temperature_temp = 0.0f;
    float humidity_temp = 0.0f;

    // Take measurement
    uint16_t error = SensirionI2CScd4x::readMeasurement(co2_temp, temperature_temp, humidity_temp);

    // Convert floats to int
    int16_t temperature_10_temp = temperature_temp * 10;
    int16_t humidity_10_temp = humidity_temp * 10;

    // Error handling
    if (error) {
        LOGGER::write_to_log("SCD", "READ MEASUREMENT FAILED");
    } else if (co2_temp == 0) {
        LOGGER::write_to_log("SCD", "SAMPLE SKIPPED");
    } else {
        // Set class outputs 
        SCD4x_SS::co2_ppm = co2_temp;
        SCD4x_SS::temperature_10_degc = temperature_10_temp;
        SCD4x_SS::humidity_10_per = humidity_10_temp;

        if (!print_data) {return;}

        SCD4x_SS::print_measurements();
    }
}

void SCD4x_SS::check_ready_flag() {

    uint16_t error = SensirionI2CScd4x::getDataReadyFlag(SCD4x_SS::ready_flag);
    if (error) {
        LOGGER::write_to_log("SCD", "READ MEASUREMENT FAILED");
    }
}

void SCD4x_SS::print_measurements() {
    LOGGER::write_to_log("SCDC", SCD4x_SS::co2_ppm);
    LOGGER::write_to_log("SCDT", SCD4x_SS::temperature_10_degc / 10);
    LOGGER::write_to_log("SCDH", SCD4x_SS::humidity_10_per / 10);
}

