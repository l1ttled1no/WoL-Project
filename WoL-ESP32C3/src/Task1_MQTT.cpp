#include "Task1_MQTT.h"

constexpr const char RPC_WOL_METHOD[] = "wakeOnLan-DN-PC-001";
constexpr const char RPC_TEST_METHOD[] = "test";  // Simple test method
constexpr const char RPC_WOL_PARAM_MAC[] = "macAddress";

constexpr uint8_t MAX_RPC_SUBSCRIPTIONS = 2U;  // Changed to 2 for testing
constexpr uint8_t MAX_RPC_RESPONSES = 5U; 

WiFiClient wfcli;
Arduino_MQTT_Client mqttcli(wfcli);

Server_Side_RPC<MAX_RPC_SUBSCRIPTIONS, MAX_RPC_RESPONSES> rpc;
// Initialize APIs array following the official ESP32 example
const std::array<IAPI_Implementation*, 1U> apis = {
    &rpc
};

// Initialize ThingsBoard following the official ESP32 example format
ThingsBoard tb(mqttcli, MAX_MSG_SIZE, MAX_MSG_SIZE, Default_Max_Stack_Size, apis);

bool subscribed = false;

bool cmpString (const char* str1, const char* str2){
    while(*str1 && *str2){
        if(*str1 != *str2){
            return false;
        }
        str1++;
        str2++;
    }
    return (*str1 == '\0' && *str2 == '\0');
}

// Generic RPC handler to catch ANY RPC method (for debugging)
void processGenericRPC(const JsonVariantConst &data, JsonDocument &response){
    Serial.println("\n========== GENERIC RPC RECEIVED ==========");
    Serial.println("[DEBUG][RPC] Received RPC request (generic handler)");
    Serial.print("[DEBUG][RPC] Raw data: ");
    serializeJson(data, Serial);
    Serial.println();
    Serial.print("[DEBUG][RPC] Pretty print: ");
    serializeJsonPretty(data, Serial);
    Serial.println();
    
    response["received"] = true;
    response["message"] = "Generic RPC handler received the call";
    Serial.println("[DEBUG][RPC] Generic RPC handled");
}

void processWolRPC(const JsonVariantConst &data, JsonDocument &response){
    Serial.println("\n========== WOL RPC RECEIVED ==========");
    Serial.println("[DEBUG][RPC] Received Wake-on-LAN RPC request");
    Serial.print("[DEBUG][RPC] Raw data: ");
    serializeJson(data, Serial);
    Serial.println();
    Serial.print("[DEBUG][RPC] Pretty print: ");
    serializeJsonPretty(data, Serial);
    Serial.println();
    
    // Extract MAC address from JSON data
    const char* receivedMACAddress = data[RPC_WOL_PARAM_MAC];
    
    if (receivedMACAddress == nullptr) {
        Serial.println("[DEBUG][RPC] No MAC address parameter found");
        response["success"] = false;
        response["message"] = "No MAC address provided";
        Serial.println("[DEBUG][RPC] Response prepared, sending back to server...");
        return;
    }
    
    Serial.print("[DEBUG][RPC] Received MAC Address: ");
    Serial.println(receivedMACAddress);
    Serial.print("[DEBUG][RPC] Expected MAC Address: ");
    Serial.println(TARGET_MAC_ADDRESS);
    
    if (cmpString(receivedMACAddress, TARGET_MAC_ADDRESS)) {
        Serial.println("[DEBUG][RPC] MAC address matches, sending WOL packet...");
        bool result = turnOnDevice();
        if (result) {
            Serial.println("[DEBUG][RPC] Wake-on-LAN packet sent successfully!");
            response["success"] = true;
            response["message"] = "WOL packet sent successfully";
        } else {
            Serial.println("[DEBUG][RPC] Failed to send Wake-on-LAN packet.");
            response["success"] = false;
            response["message"] = "Failed to send WOL packet";
        }
    } else {
        Serial.println("[DEBUG][RPC] MAC Address mismatch - Invalid MAC Address!");
        response["success"] = false;
        response["message"] = "Invalid MAC address";
    }
    
    Serial.println("[DEBUG][RPC] Response prepared, sending back to server...");
}

const char *wf_ssid = WIFI_SSID;
const char *wf_password = WIFI_PASSWORD;


void Task1_MQTT(void *pvParams) {
    Serial.println("[DEBUG][MQTT] Task1_MQTT started");
    
    // Initialize WiFi - following official example
    Serial.println("Connecting to AP ...");
    WiFi.begin(wf_ssid, wf_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to AP");
    Serial.printf("[DEBUG][MQTT] Device IP: %s\n", WiFi.localIP().toString().c_str());

    while (true) {
        // Delay at the start like the official example
        delay(1000);
        
        // Check if WiFi is still connected (reconnect helper)
        const wl_status_t status = WiFi.status();
        if (status != WL_CONNECTED) {
            Serial.println("WiFi disconnected. Reconnecting...");
            WiFi.begin(wf_ssid, wf_password);
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
            }
            Serial.println("Connected to AP");
        }
        
        // Check MQTT connection
        if (!tb.connected()) {
            // Reconnect to the ThingsBoard server,
            // if a connection was disrupted or has not yet been established
            Serial.printf("Connecting to: (%s) with token (%s)\n", MQTT_BROKER_ADDRESS, MQTT_ACCESS_TOKEN);
            if (!tb.connect(MQTT_BROKER_ADDRESS, MQTT_ACCESS_TOKEN, MQTT_BROKER_PORT)) {
                Serial.println("Failed to connect");
                subscribed = false;
                continue;
            }
            Serial.println("Connected to ThingsBoard!");
            subscribed = false;  // Reset subscription flag after reconnection
        }
        
        // Subscribe to RPC if not already subscribed
        if (!subscribed) {
            Serial.println("Subscribing for RPC...");
            Serial.printf("[DEBUG][RPC] Method 1: '%s'\n", RPC_WOL_METHOD);
            Serial.printf("[DEBUG][RPC] Method 2: '%s'\n", RPC_TEST_METHOD);
            
            // Create callback array following official ESP32 example
            const std::array<RPC_Callback, MAX_RPC_SUBSCRIPTIONS> callbacks = {
                RPC_Callback{ RPC_WOL_METHOD,  processWolRPC },
                RPC_Callback{ RPC_TEST_METHOD, processGenericRPC }
            };
            
            // Perform subscription using iterators (official ESP32 example format)
            if (!rpc.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
                Serial.println("Failed to subscribe for RPC");
                continue;
            }
            
            Serial.println("Subscribe done");
            subscribed = true;
        }
        
        // Process MQTT messages
        tb.loop();
    }
}