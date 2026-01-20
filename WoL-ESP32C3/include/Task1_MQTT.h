#ifndef TASK1_MQTT_H
#define TASK1_MQTT_H

#include <Arduino.h> 
#include <WiFi.h>   
#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>
#include <array>

#include "API_WOL.h"

#define MAX_MSG_SIZE 256

void Task1_MQTT(void *pvParams); 


#endif // TASK1_MQTT_H