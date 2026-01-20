#include "API_WOL.h"

WiFiUDP udp;
WakeOnLan wol(udp);

bool turnOnDevice(){
    wol.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
    return wol.sendMagicPacket(TARGET_MAC_ADDRESS);
}