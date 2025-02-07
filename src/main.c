#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "dev_https.h"
#include "dev_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *MAIN_TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(MAIN_TAG, "Starting ESP32 Application...");


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(MAIN_TAG, "NVS Flash Full. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    // Initialize Wifi
    ESP_LOGI(MAIN_TAG, "Initializing Wi-Fi...");
    wifi_init();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Wi-Fi setup complete!");

    // Perform HTTPS Request
    ESP_LOGI(MAIN_TAG, "Starting HTTPS Request...");
    // https_get_test();
    https_post_test();
    ESP_LOGI(MAIN_TAG, "HTTPS Request Finished!");
}
