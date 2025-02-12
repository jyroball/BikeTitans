#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

    const char *test_image_path = "../https_testing/test_image.jpg";
    const char *sd_filename = "test_image.jpg";

    // Test 1: Upload image to SD card
    ESP_LOGI(MAIN_TAG, "Uploading image to SD card...");
    save_image_to_sd(test_image_path, sd_filename);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    ESP_LOGI(MAIN_TAG, "Clearing SD card...");
    clear_sd_card();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    unmount_sd_card();

    // // Test 3: Re-upload image to SD card
    // ESP_LOGI(MAIN_TAG, "Re-uploading image to SD card...");
    // if (save_image_to_sd(test_image_path, sd_filename))
    // {
    //     ESP_LOGI(MAIN_TAG, "Image re-uploaded to SD card successfully.");
    // }
    // else
    // {
    //     ESP_LOGE(MAIN_TAG, "Failed to re-upload image to SD card.");
    // }

    // // Test 4: Upload image from SD card to Google Drive
    // ESP_LOGI(MAIN_TAG, "Uploading image from SD card to Google Drive...");
    // if (upload_image_to_drive(sd_filename))
    // {
    //     ESP_LOGI(MAIN_TAG, "Image uploaded to Google Drive successfully.");
    // }
    // else
    // {
    //     ESP_LOGE(MAIN_TAG, "Failed to upload image to Google Drive.");
    // }

    // // Test 5: Clear SD card again
    // ESP_LOGI(MAIN_TAG, "Clearing SD card again...");
    // if (clear_sd_card())
    // {
    //     ESP_LOGI(MAIN_TAG, "SD card cleared successfully (final check).");
    // }
    // else
    // {
    //     ESP_LOGE(MAIN_TAG, "Failed to clear SD card (final check).");
    // }

    ESP_LOGI(MAIN_TAG, "All tests completed.");
}
