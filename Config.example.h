#pragma once
// Copy to Config.h. Keep your actual Config.h private.
#define SPROUT_WIFI_SSID "YOUR_WIFI"
#define SPROUT_WIFI_PASSWORD "YOUR_PASSWORD"
#define SPROUT_BRIDGE_URL "http://192.168.1.100:8765"
#define SPROUT_BRIDGE_TOKEN "REPLACE_WITH_YOUR_BRIDGE_TOKEN"
// 0 hardware; 1 deterministic simulator fixtures, no Wi-Fi / codec calls.
#ifndef SPROUT_SIMULATION
#define SPROUT_SIMULATION 0
#endif
