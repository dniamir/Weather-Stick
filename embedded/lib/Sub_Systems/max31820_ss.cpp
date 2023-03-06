# include <MAX31820_SS.h>

MAX31820_SS::MAX31820_SS(OneWire &oneWire) : DallasTemperature(&oneWire) {}

void MAX31820_SS::configure_system() {
    DallasTemperature::begin();
}

void MAX31820_SS::read_data(bool print_data) {

    DallasTemperature::requestTemperatures(); // Send the command to get temperatures
    float tempC = DallasTemperature::getTempCByIndex(0);
    MAX31820_SS::temperature_100_degc = tempC * 100;

    if (!print_data) {return;}

    Serial.print("MAX 31820 Temperature: ");
    Serial.print(tempC);
    Serial.print("degC");
    Serial.println();
}