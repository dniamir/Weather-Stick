# include <wifi_ss.h>

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
        Serial.print("Attempt ");
        Serial.print(attempt);
        Serial.println(" to connect to Wifi...");

        if(attempt == attempts) {break;}

        attempt++;
        WIFI_SS::status = (WiFi.status() == WL_CONNECTED);
    }

    if(WIFI_SS::status) {
        // Print local IP address and start web server
        Serial.print("WiFi connected! ");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else {
        Serial.println("WiFi connection failed! ");
    }
    
}

void WIFI_SS::disconnect() {
    WiFi.disconnect();
    delay(200);  // Delay needed otherwise Wifi.status() is not updated in time
    WIFI_SS::status = (WiFi.status() == WL_CONNECTED);
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

    String transmission = "";
    for (int i=0; i<14; i++) {
        transmission += message[i];
        transmission += ", ";
    }
    transmission += "EOT";
    client.print(transmission);
    delay(500);
    client.stop();
    delay(500);
}