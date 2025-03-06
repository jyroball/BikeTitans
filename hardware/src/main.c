//ESP32 Libraries
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

//RTOS Libraries for task management
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//Camera Components
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"

//Include libraries for other peripherals
#include "SD.h"
#include "WiFi.h"
#include "GDrive.h"
#include "TimeSync.h"

//Main tag for logging
static const char *MAIN_TAG = "MAIN";

//Global for JPEG Encode or compression
static bool auto_jpeg_support = false; 

//Initialize Camera Settings
static esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count) {
    //Camera configuration with pinouts for ESP32-Wrover
    camera_config_t camera_config = {
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = 21,
        .pin_sscb_sda = 26,
        .pin_sscb_scl = 27,

        .pin_d7 = 35,
        .pin_d6 = 34,
        .pin_d5 = 39,
        .pin_d4 = 36,
        .pin_d3 = 19,
        .pin_d2 = 18,
        .pin_d1 = 5,
        .pin_d0 = 4,
        .pin_vsync = 25,
        .pin_href = 23,
        .pin_pclk = 22,

        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format,
        .frame_size = frame_size,

        //Range from 0-63 (0 best, 63 worst)
        .jpeg_quality = 10,
        //Frame buffer useful for higher quality image or larger images
        .fb_count = fb_count,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        //frame buffer location in DRAM instead of PRAM cause ESPIDF and PlatformIO couldnt register PRAM
        .fb_location = CAMERA_FB_IN_DRAM
    };

    //Initialize the camera sensor
    esp_err_t ret = esp_camera_init(&camera_config);
    if (ret != ESP_OK) {
        return ret;
    }

    //Camera sensor settings
    sensor_t *s = esp_camera_sensor_get();

    // Apply recommended settings
    s->set_gain_ctrl(s, 0);                         // auto gain off (1 or 0)
    s->set_exposure_ctrl(s, 0);                   // auto exposure off (1 or 0)
    s->set_agc_gain(s, 14);                       // set gain manually (0 - 31)
    s->set_aec_value(s, 600);                     // set exposure manually  (0-1200)
    s->set_quality(s, 10);                        // (0 - 63)
    s->set_gainceiling(s, GAINCEILING_32X);       // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    s->set_brightness(s, 1);                      // (-2 to 2) - set brightness
    s->set_lenc(s, 1);                            // lens correction? (1 or 0)
    s->set_saturation(s, 0);                      // (-2 to 2)
    s->set_contrast(s, 1);                        // (-2 to 2)
    s->set_sharpness(s, 0);                       // (-2 to 2)
    s->set_colorbar(s, 0);                        // (0 or 1) - show a testcard
    s->set_special_effect(s, 0);                  // (0 to 6?) apply special effect
//       s->set_whitebal(s, 0);                        // white balance enable (0 or 1)
//       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
//       s->set_wb_mode(s, 0);                         // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
//       s->set_dcw(s, 0);                             // downsize enable? (1 or 0)?
//       s->set_raw_gma(s, 1);                         // (1 or 0)
//       s->set_aec2(s, 0);                            // automatic exposure sensor?  (0 or 1)
//       s->set_ae_level(s, 0);                        // auto exposure levels (-2 to 2)
    s->set_bpc(s, 0);                             // black pixel correction
    s->set_wpc(s, 0);                             // white pixel correction


    // Additional sensor-specific settings
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);     // Flip the image vertically
    }

    // Additional sensor-specific settings
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);     // Flip the image vertically
    }

    // Get the basic information of the sensor.
    camera_sensor_info_t *s_info = esp_camera_sensor_get_info(&(s->id));

    if (ESP_OK == ret && PIXFORMAT_JPEG == pixel_format && s_info->support_jpeg == true) {
        auto_jpeg_support = true;
    }

    return ret;
}

void app_main()
{
    ESP_LOGI(MAIN_TAG, "Starting ESP32 Application...");

    //checkf alsh
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(MAIN_TAG, "NVS Flash Full. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //init everything
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

    if (ESP_OK != init_camera(10 * 1000000, PIXFORMAT_JPEG, FRAMESIZE_VGA, 2)) {
        ESP_LOGE(MAIN_TAG, "init camrea sensor fail");
        return;
    }

    // take a picture every two seconds and print the size of the picture.
    //while (1) {

        ESP_LOGI(MAIN_TAG, "Mounting SD Card...");
        mount_sd_card();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        
        //clear sdcard before anything
        clear_sd_card();

        ESP_LOGI(MAIN_TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        ESP_LOGI(MAIN_TAG, "Checking SD card files...");
        list_files_on_sd();
        vTaskDelay(5000 / portTICK_PERIOD_MS);


        if (pic) {
            // use pic->buf to access the image
            ESP_LOGI(MAIN_TAG, "Picture taken! Its size was: %zu bytes", pic->len);
            // To enable the frame buffer can be reused again.
            // Note: If you don't call fb_return(), the next time you call fb_get() you may get
            // an error "Failed to get the frame on time!"because there is no frame buffer space available.

            vTaskDelay(pdMS_TO_TICKS(2000));

            //get path
            char path[50];
            sprintf(path, "/sdcard/photo.jpg");
            ESP_LOGI(MAIN_TAG, "Opening file: %s", path);
            
            //Check if sd card is accessivbke
            DIR* dir = opendir("/sdcard");
            if (!dir) {
                ESP_LOGE(MAIN_TAG, "SD Card is NOT accessible!");
            } else {
                ESP_LOGI(MAIN_TAG, "SD Card is accessible.");
                closedir(dir);
            }


            vTaskDelay(pdMS_TO_TICKS(2000));

            //save pic taken into sd card
            FILE *file = fopen(path, "wb+");
            if (file) {
                size_t written = fwrite(pic->buf, 1, pic->len, file);
                fclose(file);

                if (written == pic->len) {
                    ESP_LOGI(MAIN_TAG, "Saved photo to %s", path);
                } else {
                    ESP_LOGE(MAIN_TAG, "Incomplete write: only %zu of %zu bytes written", written, pic->len);
                }
            } else {
                ESP_LOGE(MAIN_TAG, "Failed to open file for writing");
            }

            
            //need to call agin for loop or wont take pic again
            esp_camera_fb_return(pic);

        }

        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(MAIN_TAG, "Uploading (Overwriting) file on GDrive...");
        upload_file();


        ESP_LOGI(MAIN_TAG, "Unmounting SD Card...");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        unmount_sd_card();
    //}

    ESP_LOGI(MAIN_TAG, "All tests completed.");
}