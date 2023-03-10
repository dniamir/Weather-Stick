# include <wifi_ss.h>

WIFI_SS::WIFI_SS() {}

WIFI_SS::WIFI_SS(const char* ssid, const char* password) {
    WIFI_SS::connect_to_wifi(ssid, password);
}

void WIFI_SS::connect_to_wifi(const char* ssid, const char* password) {

    WiFi.begin(ssid, password);

    // Make 5 attempts to connect to wifi
    uint8_t attempts = 5;
    uint8_t attempt = 1;
    while (!WIFI_SS::wifi_status) {
        delay(500);
        Serial.print("Attempt ");
        Serial.print(attempt);
        Serial.println(" to connect to Wifi...");

        if(attempt == attempts) {
            break;
        }

        attempt++;
        WIFI_SS::wifi_status = (WiFi.status() == WL_CONNECTED);
    }
    // Print local IP address and start web server
    Serial.print("WiFi connected! ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void WIFI_SS::send_message(int32_t message[], bool print_data) {

    WiFiClient client;
    client.connect(WIFI_SS::server_ip_address, WIFI_SS::server_port);

    if(print_data) {
        Serial.print("IP Address: ");
        Serial.print(WIFI_SS::server_ip_address);
        Serial.print(", Port: ");
        Serial.println(WIFI_SS::server_port);
    }

    for (int i=0; i<14; i++) {
        client.print(message[i]);
        client.print(", ");
    }
    client.stop();

}