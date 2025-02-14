#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <dirent.h>

#include "dev_https.h"
#include "dev_wifi.h"
// #include "dev_image.h"
#include "dev_upload.h"

static const char *MAIN_TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(MAIN_TAG, "Starting ESP32 Application...");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(MAIN_TAG, "NVS Flash Full. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi
    ESP_LOGI(MAIN_TAG, "Initializing Wi-Fi...");
    wifi_init();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Wi-Fi setup complete!");

    // SD Card Tests
    ESP_LOGI(MAIN_TAG, "Starting SD Card Tests...");
    mount_sd_card();
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // const char *test_image_path = "C:/Users/samhe/Desktop/CodingStuff/BikeTitans/https_testing/test_image.jpg";
    // const char *sd_filename = "test_image.jpg";

    ESP_LOGI(MAIN_TAG, "Checking SD card files...");
    list_files_on_sd();
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // ESP_LOGI(MAIN_TAG, "Clearing SD card...");
    // clear_sd_card();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    unmount_sd_card();

    ESP_LOGI(MAIN_TAG, "All tests completed.");
}
