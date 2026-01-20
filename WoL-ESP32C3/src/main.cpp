#include <Arduino.h>
// #include <WiFi.h>
#include "Task1_MQTT.h"
// #include "API_WOL.h"


TaskHandle_t mqttTaskHandle = NULL;


void setup() {
  Serial.begin(115200);
  for (auto i = 0; i < 10; i++) {
    Serial.println("[SERIAL] init serial...");
    delay(500);
  }
  vTaskDelay(pdMS_TO_TICKS(2000));
  
  Serial.println("Starting Wake-on-LAN Device...");
  
  // Create MQTT task
  xTaskCreate(
    Task1_MQTT,
    "MQTT Task",
    8192,
    NULL,
    1,
    &mqttTaskHandle
  );
  
}


void loop() {

}
