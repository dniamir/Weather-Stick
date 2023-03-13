# include <WiFi.h>

class WIFI_SS {

  public:
    // Constructor
    WIFI_SS();
    WIFI_SS(const char* ssid, const char* password);

    const char* server_ip_address = "192.168.4.102";
    const uint16_t server_port = 8090;
    bool status = false;

    void connect_to_wifi(const char* ssid, const char* password);
    void disconnect();
    void send_message(String message[], bool print_data=false);

};