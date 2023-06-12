# include <MAX31820_SS.h>
# include <logger.h>

MAX31820_SS::MAX31820_SS(OneWire &oneWire) : DallasTemperature(&oneWire) {}

void MAX31820_SS::configure_system() {
    LOGGER::write_to_log("MAXT", "CONFIGURE SYSTEM");
    DallasTemperature::begin();
}

void MAX31820_SS::read_data(bool print_data) {

    DallasTemperature::requestTemperatures(); // Send the command to get temperatures
    float tempC = DallasTemperature::getTempCByIndex(0);
    MAX31820_SS::temperature_100_degc = tempC * 100;

    if (!print_data) {return;}

    LOGGER::write_to_log("MAXT", MAX31820_SS::temperature_100_degc / 100);
}