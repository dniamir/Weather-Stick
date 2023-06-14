# include <wifi_ss.h>
# include <logger.h>

WIFI_SS::WIFI_SS() {}

WIFI_SS::WIFI_SS(const char* ssid, const char* password) {
    WIFI_SS::connect_to_wifi(ssid, password);
}

void WIFI_SS::connect_to_wifi(const char* ssid, const char* password) {

    if(!WIFI_SS::status) {WiFi.begin(ssid, password);}

    // Make 5 attempts to connect to wifi
    uint8_t attempts = 5;
    uint8_t attempt = 1;
    while (!WIFI_SS::status) {
        delay(500);
        LOGGER::write_to_log("WIFI_CONNECT", attempt);

        if(attempt == attempts) {break;}

        attempt++;
        WIFI_SS::status = (WiFi.status() == WL_CONNECTED);
    }

    if(WIFI_SS::status) {
        // Print local IP address and start web server
        LOGGER::write_to_log("WIFI_DIP", (char)WiFi.localIP());
    }
    else {
        LOGGER::write_to_log("WIFI_CONNECT", "FAILED");
    }
    
}

void WIFI_SS::disconnect() {
    WiFi.disconnect();
    delay(200);  // Delay needed otherwise Wifi.status() is not updated in time
    WIFI_SS::status = (WiFi.status() == WL_CONNECTED);
}

void WIFI_SS::send_message(String transmission_message[], bool print_data) {

    WiFiClient client;
    client.connect(WIFI_SS::server_ip_address, WIFI_SS::server_port);
    delay(500);

    if(print_data) {
        LOGGER::write_to_log("WIFI_SIP", WIFI_SS::server_ip_address);
        LOGGER::write_to_log("WIFI_PORT", WIFI_SS::server_port);
    }

    String transmission_values = "";
    transmission_values += "SOT, ";
    for (int i=0; i<16; i++) {
        transmission_values += transmission_message[i];
        transmission_values += ", ";
    }
    transmission_values += "EOT";
    client.print(transmission_values);
    delay(500);
    client.stop();
    delay(500);
}