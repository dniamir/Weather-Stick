# include <SCD4x_SS.h>

SCD4x_SS::SCD4x_SS() : SensirionI2CScd4x() {};

void SCD4x_SS::begin(TwoWire& i2cBus) {

    // Start I2C Measurement
    SensirionI2CScd4x::begin(i2cBus);

    // Stop Previous Measurements if they were ongoing
    uint16_t error = SensirionI2CScd4x::stopPeriodicMeasurement();
    if (error) {
        Serial.println("Error trying to execute stopPeriodicMeasurement(): ");
    }
}

void SCD4x_SS::set_serial_number() {
    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    uint16_t error = SensirionI2CScd4x::getSerialNumber(serial0, serial1, serial2);
    if (error) {
        Serial.println("Error trying to execute getSerialNumber()");
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
        Serial.println("Error trying to execute startPeriodicMeasurement(): ");
    }
    Serial.println("Waiting for first measurement... (5 sec)");
    // delay(5000);
}

void SCD4x_SS::wake_up() {
    uint16_t error = SensirionI2CScd4x::wakeUp();
    if (error) {
        Serial.println("Error trying to wake up");
    }
}

void SCD4x_SS::power_down() {
    uint16_t error = SensirionI2CScd4x::powerDown();
    if (error) {
        Serial.println("Error trying to power down");
    }
}

void SCD4x_SS::read_oneshot() {
    uint16_t error = SensirionI2CScd4x::measureSingleShot();
    if (error) {
        Serial.println("Error trying to perfom a one shot");
    }
}

void SCD4x_SS::read_oneshot_th() {
    uint16_t error = SensirionI2CScd4x::measureSingleShotRhtOnly();
    if (error) {
        Serial.println("Error trying to perform a th one shot");
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

    // Error handling
    if (error) {
        Serial.println("Error trying to execute readMeasurement()");
    } else if (co2_temp == 0) {
        Serial.println("Invalid sample detected, skipping.");
    } else {
        // Set class outputs 
        co2 = co2_temp;
        temperature = temperature_temp;
        humidity = humidity_temp;

        if (!print_data) {return;}

        SCD4x_SS::print_measurements();
    }
}

void SCD4x_SS::check_ready_flag() {

    uint16_t error = SensirionI2CScd4x::getDataReadyFlag(SCD4x_SS::ready_flag);
    if (error) {
        Serial.println("Error trying to execute readMeasurement(): ");
    }

}

void SCD4x_SS::print_measurements() {
    Serial.print("Co2:");
    Serial.print(SCD4x_SS::co2);
    Serial.print("\t");
    Serial.print("Temperature:");
    Serial.print(SCD4x_SS::temperature);
    Serial.print("\t");
    Serial.print("Humidity:");
    Serial.println(SCD4x_SS::humidity);
}

