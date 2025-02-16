#include <esp_sntp.h>
#include <esp_log.h>
#include <time.h>
#include <stdlib.h>

#define TIME_TAG "TIME_SYNC"

void sync_time()
{

    // Set the timezone to US Los Angeles (Pacific Time)
    setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1); // PST8PDT for Pacific Time with DST rules
    tzset();                                   // Apply the timezone settings

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org"); // Use a global NTP server
    esp_sntp_init();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int max_retries = 10;

    while (timeinfo.tm_year < (2020 - 1900) && ++retry < max_retries)
    {
        ESP_LOGI(TIME_TAG, "Waiting for system time to be set... (%d/%d)", retry, max_retries);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year < (2020 - 1900))
    {
        ESP_LOGE(TIME_TAG, "Failed to get NTP time!");
    }
    else
    {
        ESP_LOGI(TIME_TAG, "Time synchronized: %s", asctime(&timeinfo));
    }
}