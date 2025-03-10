// ESP32 Libraries
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

// RTOS Libraries for task management
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Include libraries for other peripherals
#include "SD.h"
#include "WiFi.h"
#include "GDrive.h"
#include "TimeSync.h"
#include "Cam.h"

// Main tag for logging
static const char *MAIN_TAG = "MAIN";

void app_main()
{
    // Log program Starting
    ESP_LOGI(MAIN_TAG, "Starting ESP32 Application...");

    //
    //  INitialize EVerything
    //

    // checkf falsh
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(MAIN_TAG, "NVS Flash Full. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //  Initialize WiFi
    ESP_LOGI(MAIN_TAG, "Initializing Wi-Fi...");
    wifi_init();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Wi-Fi setup complete!");

    // Initialize Time Synchro
    ESP_LOGI(MAIN_TAG, "Getting Current Time...");
    sync_time();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Time Synced!");

    // Initialize Wifi
    if (ESP_OK != init_camera(10 * 1000000, PIXFORMAT_JPEG, FRAMESIZE_VGA, 1))
    {
        ESP_LOGE(MAIN_TAG, "Camera Setup Failed!");
        return;
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Camera setup complete!");

    // Loop For State Machien
    //while (1) {
    //Mount SD CArd
    ESP_LOGI(MAIN_TAG, "Mounting SD Card...");
    mount_sd_card();
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // clear sdcard before anything
    clear_sd_card();

    // Take Picture
    take_pic();

    // Upload to google Drive
    upload_file();

    //Unmount SD Card
    ESP_LOGI(MAIN_TAG, "Unmounting SD Card...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    unmount_sd_card();
    //}

    ESP_LOGI(MAIN_TAG, "All tests completed.");
}