#ifndef API_WOL_H
#define API_WOL_H
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>

extern WiFiUDP udp; 
extern WakeOnLan wol;

bool turnOnDevice(); 

#endif // API_WOL_H