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

    // checkf alsh
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(MAIN_TAG, "NVS Flash Full. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // init everything
    //  Initialize WiFi
    ESP_LOGI(MAIN_TAG, "Initializing Wi-Fi...");
    wifi_init();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Wi-Fi setup complete!");

    // Initialize Time
    ESP_LOGI(MAIN_TAG, "Getting Current Time...");
    sync_time();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(MAIN_TAG, "Time Synced!");

    if (ESP_OK != init_camera(10 * 1000000, PIXFORMAT_JPEG, FRAMESIZE_VGA, 1))
    {
        ESP_LOGE(MAIN_TAG, "init camrea sensor fail");
        return;
    }

    // take a picture every two seconds and print the size of the picture.
    // while (1) {

    ESP_LOGI(MAIN_TAG, "Mounting SD Card...");
    mount_sd_card();
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // clear sdcard before anything
    clear_sd_card();

    ESP_LOGI(MAIN_TAG, "Taking picture...");
    camera_fb_t *pic = esp_camera_fb_get();

    ESP_LOGI(MAIN_TAG, "Checking SD card files...");
    list_files_on_sd();
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    if (pic)
    {
        // use pic->buf to access the image
        ESP_LOGI(MAIN_TAG, "Picture taken! Its size was: %zu bytes", pic->len);
        // To enable the frame buffer can be reused again.
        // Note: If you don't call fb_return(), the next time you call fb_get() you may get
        // an error "Failed to get the frame on time!"because there is no frame buffer space available.

        vTaskDelay(pdMS_TO_TICKS(2000));

        // get path
        char path[50];
        sprintf(path, "/sdcard/photo.jpg");
        ESP_LOGI(MAIN_TAG, "Opening file: %s", path);

        // Check if sd card is accessivbke
        DIR *dir = opendir("/sdcard");
        if (!dir)
        {
            ESP_LOGE(MAIN_TAG, "SD Card is NOT accessible!");
        }
        else
        {
            ESP_LOGI(MAIN_TAG, "SD Card is accessible.");
            closedir(dir);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));

        // save pic taken into sd card
        FILE *file = fopen(path, "wb+");
        if (file)
        {
            size_t written = fwrite(pic->buf, 1, pic->len, file);
            fclose(file);

            if (written == pic->len)
            {
                ESP_LOGI(MAIN_TAG, "Saved photo to %s", path);
            }
            else
            {
                ESP_LOGE(MAIN_TAG, "Incomplete write: only %zu of %zu bytes written", written, pic->len);
            }
        }
        else
        {
            ESP_LOGE(MAIN_TAG, "Failed to open file for writing");
        }

        // need to call agin for loop or wont take pic again
        esp_camera_fb_return(pic);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));

    // ESP_LOGI(MAIN_TAG, "Uploading (Overwriting) file on GDrive...");
    upload_file();

    ESP_LOGI(MAIN_TAG, "Unmounting SD Card...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    unmount_sd_card();
    //}

    ESP_LOGI(MAIN_TAG, "All tests completed.");
}