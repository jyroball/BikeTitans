#pragma once

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#define WIFI_SSID "BikeTitans"
#define WIFI_PASS "testing123456"
#define WIFI_CONNECTED_BIT BIT0

void wifi_init(void);

bool is_wifi_connected(void);