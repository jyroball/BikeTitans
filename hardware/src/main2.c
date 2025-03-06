#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "WiFi.h"
#include "GDrive.h"
#include "TimeSync.h"
#include "SD.h"

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

    // Initialize Time
    ESP_LOGI(MAIN_TAG, "Getting Current Time...");
    sync_time();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Time Synced!");

    // char *jwt = create_jwt();
    // if (jwt)
    // {
    //     ESP_LOGI(MAIN_TAG, "JWT IS %s", jwt);
    //     free(jwt);
    // }

    // vTaskDelay(2000 / portTICK_PERIOD_MS);

    // char *access_token = get_access_token();
    // if (access_token)
    // {
    //     ESP_LOGI(MAIN_TAG, "ACCESS TOKEN IS %s", access_token);
    //     free(access_token);
    // }

    // SD Card Tests
    ESP_LOGI(MAIN_TAG, "Mounting SD Card...");
    mount_sd_card();
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // ESP_LOGI(MAIN_TAG, "Checking SD card files...");
    // list_files_on_sd();
    // vTaskDelay(5000 / portTICK_PERIOD_MS);

    // ESP_LOGI(MAIN_TAG, "Opening all JPG files...");
    // test_open_file();

    ESP_LOGI(MAIN_TAG, "Uploading (Overwriting) file on GDrive...");
    upload_file();

    // ESP_LOGI(MAIN_TAG, "Clearing SD card...");
    // clear_sd_card();

    ESP_LOGI(MAIN_TAG, "Unmounting SD Card...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    unmount_sd_card();

    ESP_LOGI(MAIN_TAG, "All tests completed.");
    return;
}
