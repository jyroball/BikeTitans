#include <stdio.h>
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"  // FAT filesystem functions

#define TAG "ESP_CAM"

// Camera pin definitions
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     21
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       19
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

static void init_camera() {
    camera_config_t config = {
        .ledc_channel = LEDC_CHANNEL_0,
        .ledc_timer = LEDC_TIMER_0,
        .pin_d0 = Y2_GPIO_NUM,
        .pin_d1 = Y3_GPIO_NUM,
        .pin_d2 = Y4_GPIO_NUM,
        .pin_d3 = Y5_GPIO_NUM,
        .pin_d4 = Y6_GPIO_NUM,
        .pin_d5 = Y7_GPIO_NUM,
        .pin_d6 = Y8_GPIO_NUM,
        .pin_d7 = Y9_GPIO_NUM,
        .pin_xclk = XCLK_GPIO_NUM,
        .pin_pclk = PCLK_GPIO_NUM,
        .pin_vsync = VSYNC_GPIO_NUM,
        .pin_href = HREF_GPIO_NUM,
        .pin_sccb_sda = SIOD_GPIO_NUM,
        .pin_sccb_scl = SIOC_GPIO_NUM,
        .pin_pwdn = PWDN_GPIO_NUM,
        .pin_reset = RESET_GPIO_NUM,
        .xclk_freq_hz = 20000000,
        .pixel_format = PIXFORMAT_JPEG,
        .grab_mode = CAMERA_GRAB_LATEST,
        .frame_size = FRAMESIZE_UXGA,
        .jpeg_quality = 10,
        .fb_count = 1
    };
    
    if (esp_camera_init(&config) != ESP_OK) {
        ESP_LOGE(TAG, "Camera initialization failed");
    } else {
        ESP_LOGI(TAG, "Camera initialized successfully");
    }
}

static void init_sd_card() {
    ESP_LOGI(TAG, "Initializing SD card...");
    
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "SD card mounted successfully");
}

static void capture_and_save_photo() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return;
    }
    
    char path[50];
    sprintf(path, "/sdcard/photo_%ld.jpg", esp_log_timestamp());
    FILE *file = fopen(path, "wb");
    if (file) {
        fwrite(fb->buf, 1, fb->len, file);
        fclose(file);
        ESP_LOGI(TAG, "Saved photo to %s", path);
    } else {
        ESP_LOGE(TAG, "Failed to open file for writing");
    }
    esp_camera_fb_return(fb);
}

void app_main() {
    nvs_flash_init();
    init_camera();
    init_sd_card();
    while (1) {
        //capture_and_save_photo();
        //vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
