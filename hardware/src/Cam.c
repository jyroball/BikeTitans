#include "Cam.h"

// Initialize Camera Settings
esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count)
{
    // Camera configuration with pinouts for ESP32-Wrover
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

        // Range from 0-63 (0 best, 63 worst)
        .jpeg_quality = 10,
        // Frame buffer useful for higher quality image or larger images
        .fb_count = fb_count,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        // frame buffer location in DRAM instead of PRAM cause ESPIDF and PlatformIO couldnt register PRAM
        .fb_location = CAMERA_FB_IN_DRAM};

    // Initialize the camera sensor
    esp_err_t ret = esp_camera_init(&camera_config);
    if (ret != ESP_OK)
    {
        return ret;
    }

    // Camera sensor settings
    sensor_t *s = esp_camera_sensor_get();

    // Apply recommended settings
    s->set_gain_ctrl(s, 0);                 // auto gain off (1 or 0)
    s->set_exposure_ctrl(s, 0);             // auto exposure off (1 or 0)
    s->set_agc_gain(s, 14);                 // set gain manually (0 - 31)
    s->set_aec_value(s, 600);               // set exposure manually  (0-1200)
    s->set_quality(s, 10);                  // (0 - 63)
    s->set_gainceiling(s, GAINCEILING_32X); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    s->set_brightness(s, 1);                // (-2 to 2) - set brightness
    s->set_lenc(s, 1);                      // lens correction? (1 or 0)
    s->set_saturation(s, 0);                // (-2 to 2)
    s->set_contrast(s, 1);                  // (-2 to 2)
    s->set_sharpness(s, 0);                 // (-2 to 2)
    s->set_colorbar(s, 0);                  // (0 or 1) - show a testcard
    s->set_special_effect(s, 0);            // (0 to 6?) apply special effect
    //       s->set_whitebal(s, 0);                        // white balance enable (0 or 1)
    //       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
    //       s->set_wb_mode(s, 0);                         // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    //       s->set_dcw(s, 0);                             // downsize enable? (1 or 0)?
    //       s->set_raw_gma(s, 1);                         // (1 or 0)
    //       s->set_aec2(s, 0);                            // automatic exposure sensor?  (0 or 1)
    //       s->set_ae_level(s, 0);                        // auto exposure levels (-2 to 2)
    s->set_bpc(s, 0);                       // black pixel correction
    s->set_wpc(s, 0);                       // white pixel correction

    // Additional sensor-specific settings
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1); // Flip the image vertically
    }

    // Additional sensor-specific settings
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1); // Flip the image vertically
    }

    // Get the basic information of the sensor.
    camera_sensor_info_t *s_info = esp_camera_sensor_get_info(&(s->id));

    if (ESP_OK == ret && PIXFORMAT_JPEG == pixel_format && s_info->support_jpeg == true)
    {
        auto_jpeg_support = true;
    }

    return ret;
}
