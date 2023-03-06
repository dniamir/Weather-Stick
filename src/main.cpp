# include <Arduino.h>
# include <OneWire.h>
# include <Wire.h>
# include <scd4x_ss.h>
# include <tsl2591_ss.h>
# include <max31820_ss.h>
# include <bme680_ss.h>
# include <MAX17260.h>
# include <eink_display_ss.h>

const int LED_PIN = 26;
const float led_blink_time = 1000;

int I2C_SDA = 33;
int I2C_SCL = 32;

const int CHARGER_EN = 15;
const int CHARGER_POK = 14;

SCD4x_SS co2_system;

// Never ending task
void toggle_led1(void * parameter){
  for(;;) { // infinite loop
    digitalWrite(LED_PIN, HIGH);  // // Turn the LED on
    vTaskDelay(200.0 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);  // Turn the LED off
    vTaskDelay(200.0 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Awake!");

  // Setup I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // LED
  Serial.println("Creating LED blink");
  pinMode(LED_PIN, OUTPUT);
  xTaskCreate(toggle_led1, "Toggle LED1",  1000, (void*)&led_blink_time, 1, NULL);

  co2_system.begin(Wire);

  pinMode(CHARGER_EN, OUTPUT);
  pinMode(CHARGER_POK, INPUT);

  // Set Charging to Enable
  Serial.println("Enable Charging");
  digitalWrite(CHARGER_EN, LOW);

}

void loop() {

  Serial.println("Starting Loop");
  // delay(10000);
  // Serial.println("Going into Sleep Mode");
  // esp_sleep_enable_timer_wakeup(100 * 60 * 1e6);
  // esp_deep_sleep_start();

  // Check charging source
  Serial.print("Charger OK? - (0 is yes, 1 is no) - ");
  Serial.println(digitalRead(CHARGER_POK));

  // CO2 Loop
  delay(6000);
  Serial.println("Waking Sensor Up");
  co2_system.wake_up();
  delay(6000);
  Serial.println("Capturing Data 1 ");
  co2_system.read_oneshot();
  co2_system.read_data(true);
  co2_system.power_down();

  Serial.println(" ");

}