#pragma once

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"
#include <inttypes.h> // For PRId64 macro

#include "dev_wifi.h"

static const char *TAG = "HTTPS_CLIENT";

#define DEV_HTTPS_LINK "https://httpstat.us/random/200,201,500-504"
#define DEV_HTTPS_POST "https://httpbin.org/post"

static inline esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP Event: ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP Event: CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP Event: HEADER SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP Event: HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP Event: DATA (%d bytes)", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            printf("%.*s", evt->data_len, (char *)evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP Event: FINISHED");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP Event: DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP Event: REDIRECT");
        break;
    }
    return ESP_OK;
}

// HTTPS Get
static inline void https_get_test(void)
{
    // Ensure Wi-Fi is connected
    if (!is_wifi_connected())
    {
        ESP_LOGE(TAG, "Wi-Fi is not connected. Cannot make HTTPS request.");
        return;
    }

    esp_http_client_config_t config = {
        .url = DEV_HTTPS_LINK,
        .event_handler = _http_event_handler,
        .timeout_ms = 5000,                         // Increase timeout to 5 seconds
        .crt_bundle_attach = esp_crt_bundle_attach, // Use ESP32 CA bundle
        .transport_type = HTTP_TRANSPORT_OVER_SSL,  // Ensure HTTPS
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Accept", "application/json");

    // Sent Request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTPS GET Status = %d, Content length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTPS GET request failed: %s", esp_err_to_name(err));
    }

    // Cleanup
    esp_http_client_cleanup(client);
}

// HTTPS POST
static inline void https_post_test(void)
{
    if (!is_wifi_connected())
    {
        ESP_LOGE(TAG, "Wi-Fi is not connected. Cannot make HTTPS request.");
        return;
    }

    const char *post_data = "{\"message\":\"Hello from ESP32\"}";

    esp_http_client_config_t config = {
        .url = DEV_HTTPS_POST,
        .event_handler = _http_event_handler,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTPS POST Status = %d, Content length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTPS POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
