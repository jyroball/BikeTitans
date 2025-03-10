#pragma once

// ESP32 Libraries
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

// Camera Components
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"

// SD Card
#include "SD.h"

// Global for JPEG Encode or compression
static bool auto_jpeg_support = false;

// Initialize Camera
esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count);

//Save Picture