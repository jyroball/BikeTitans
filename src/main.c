#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <dirent.h>

// #include "dev_https.h"
#include "dev_wifi.h"
#include "dev_gdrive.h"
#include "dev_time.h"
// #include "dev_sdcard.h"

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
    ESP_LOGI(MAIN_TAG, "Initializing Clock...");
    sync_time();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Time Synced!");

    char *jwt = create_jwt();
    ESP_LOGI(MAIN_TAG, "JWT: %s", jwt);
    free(jwt);

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    char *token = get_access_token();
    ESP_LOGI(MAIN_TAG, "ACCESS TOKEN: %s", token);
    free(token);

    // // SD Card Tests
    // ESP_LOGI(MAIN_TAG, "Mounting SD Card...");
    // mount_sd_card();
    // vTaskDelay(5000 / portTICK_PERIOD_MS);

    // ESP_LOGI(MAIN_TAG, "Checking SD card files...");
    // list_files_on_sd();
    // vTaskDelay(5000 / portTICK_PERIOD_MS);

    // ESP_LOGI(MAIN_TAG, "Reading hardcoded file...");
    // test_open_file();

    // // ESP_LOGI(MAIN_TAG, "GDrive file upload...");
    // // esp_err_t result = patch_file_on_gdrive("/sdcard/TEST_I~1.JPG");
    // // if (result != ESP_OK)
    // // {
    // //     ESP_LOGE(MAIN_TAG, "Failed to update file on Google Drive");
    // // }

    // // ESP_LOGI(MAIN_TAG, "Clearing SD card...");
    // // clear_sd_card();

    // ESP_LOGI(MAIN_TAG, "HTTPS Posting...");
    // https_post_test();

    // ESP_LOGI(MAIN_TAG, "Unmounting SD Card...");
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // unmount_sd_card();

    ESP_LOGI(MAIN_TAG, "All tests completed.");
    return;
}
