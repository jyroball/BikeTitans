#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"
#include "mbedtls/base64.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "Credentials.h"
#include "dev_wifi.h"

#define JWT_EXPIRATION_SECONDS 3600
#define MAX_RESPONSE_SIZE 2048
#define TAG "GDRIVE"

typedef struct
{
    char *buffer;
    int buffer_len;
    int buffer_size;
} http_response_t;

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    http_response_t *resp = (http_response_t *)evt->user_data;

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
        if (resp && evt->data)
        {
            int required_size = resp->buffer_len + evt->data_len + 1; // +1 for null terminator
            if (required_size > resp->buffer_size)
            {
                // Reallocate buffer to accommodate new data
                char *new_buffer = realloc(resp->buffer, required_size);
                if (!new_buffer)
                {
                    ESP_LOGE(TAG, "Failed to allocate memory for response data");
                    return ESP_FAIL;
                }
                resp->buffer = new_buffer;
                resp->buffer_size = required_size;
            }

            // Append new data
            memcpy(resp->buffer + resp->buffer_len, evt->data, evt->data_len);
            resp->buffer_len += evt->data_len;
            resp->buffer[resp->buffer_len] = '\0'; // Null terminate
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

// Base64 URL encoding function (removes padding and replaces characters)
void base64url_encode(const unsigned char *input, size_t input_len, char *output, size_t output_size)
{
    size_t olen;
    mbedtls_base64_encode((unsigned char *)output, output_size, &olen, input, input_len);

    // Replace URL-unsafe characters and remove padding
    for (size_t i = 0; i < olen; i++)
    {
        if (output[i] == '+')
            output[i] = '-';
        else if (output[i] == '/')
            output[i] = '_';
    }
    while (olen > 0 && output[olen - 1] == '=')
        olen--; // Remove padding
    output[olen] = '\0';
}

// Function to generate JWT
char *create_jwt()
{
    // Get current time
    time_t now;
    time(&now);

    // Create JWT Header JSON
    cJSON *header = cJSON_CreateObject();
    cJSON_AddStringToObject(header, "alg", "RS256");
    cJSON_AddStringToObject(header, "typ", "JWT");
    char *header_str = cJSON_PrintUnformatted(header);
    cJSON_Delete(header);

    if (!header_str)
    {
        ESP_LOGE(TAG, "Failed to create JWT header JSON");
        return NULL;
    }

    // Create JWT Payload JSON
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddStringToObject(payload, "iss", CLIENT_EMAIL);
    cJSON_AddStringToObject(payload, "scope", "https://www.googleapis.com/auth/drive.file");
    cJSON_AddStringToObject(payload, "aud", TOKEN_URI);
    cJSON_AddNumberToObject(payload, "exp", now + JWT_EXPIRATION_SECONDS);
    cJSON_AddNumberToObject(payload, "iat", now);
    char *payload_str = cJSON_PrintUnformatted(payload);
    cJSON_Delete(payload);

    if (!payload_str)
    {
        ESP_LOGE(TAG, "Failed to create JWT payload JSON");
        free(header_str);
        return NULL;
    }

    // Allocate memory for encoded parts
    char *encoded_header = malloc(256);
    char *encoded_payload = malloc(512);
    if (!encoded_header || !encoded_payload)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        free(header_str);
        free(payload_str);
        free(encoded_header);
        free(encoded_payload);
        return NULL;
    }

    // Base64 URL Encode header & payload
    base64url_encode((unsigned char *)header_str, strlen(header_str), encoded_header, 256);
    base64url_encode((unsigned char *)payload_str, strlen(payload_str), encoded_payload, 512);

    free(header_str);
    free(payload_str);

    // Construct the signing input (header.payload)
    size_t signing_input_len = strlen(encoded_header) + strlen(encoded_payload) + 2;
    char *signing_input = malloc(signing_input_len);
    if (!signing_input)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        free(encoded_header);
        free(encoded_payload);
        return NULL;
    }
    snprintf(signing_input, signing_input_len, "%s.%s", encoded_header, encoded_payload);

    // Load the private key
    mbedtls_pk_context pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_init(&pk);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    const char *pers = "jwt_sign";
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers));

    if (mbedtls_pk_parse_key(&pk, (const unsigned char *)PRIVATE_KEY_PEM, strlen(PRIVATE_KEY_PEM) + 1, NULL, 0, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
    {
        ESP_LOGE(TAG, "Failed to parse private key");
        free(encoded_header);
        free(encoded_payload);
        free(signing_input);
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return NULL;
    }

    // Compute SHA-256 hash of signing input
    unsigned char hash[32];
    if (mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (unsigned char *)signing_input, strlen(signing_input), hash) != 0)
    {
        ESP_LOGE(TAG, "Failed to hash signing input");
        free(encoded_header);
        free(encoded_payload);
        free(signing_input);
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return NULL;
    }

    // Sign the hash
    unsigned char signature[512];
    size_t sig_len;
    if (mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, sizeof(hash), signature, sizeof(signature), &sig_len, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
    {
        ESP_LOGE(TAG, "Signing failed");
        free(encoded_header);
        free(encoded_payload);
        free(signing_input);
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return NULL;
    }

    // Allocate memory for the encoded signature
    char *encoded_signature = malloc(1024);
    if (!encoded_signature)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        free(encoded_header);
        free(encoded_payload);
        free(signing_input);
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return NULL;
    }

    base64url_encode(signature, sig_len, encoded_signature, 1024);

    // Construct final JWT
    size_t jwt_len = strlen(encoded_header) + strlen(encoded_payload) + strlen(encoded_signature) + 3;
    char *jwt = malloc(jwt_len);
    if (!jwt)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        free(encoded_header);
        free(encoded_payload);
        free(signing_input);
        free(encoded_signature);
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return NULL;
    }

    snprintf(jwt, jwt_len, "%s.%s.%s", encoded_header, encoded_payload, encoded_signature);

    // Cleanup
    free(encoded_header);
    free(encoded_payload);
    free(signing_input);
    free(encoded_signature);
    mbedtls_pk_free(&pk);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    return jwt;
}

char *get_access_token(void)
{
    if (!is_wifi_connected())
    {
        ESP_LOGE(TAG, "Wi-Fi is not connected. Cannot make HTTPS request.");
        return NULL;
    }

    char *jwt = create_jwt();
    if (!jwt)
    {
        ESP_LOGE(TAG, "Failed to create JWT.");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (!json)
    {
        ESP_LOGE(TAG, "Failed to create JSON object.");
        free(jwt);
        return NULL;
    }

    cJSON_AddStringToObject(json, "grant_type", "urn:ietf:params:oauth:grant-type:jwt-bearer");
    cJSON_AddStringToObject(json, "assertion", jwt);
    free(jwt);

    char *post_data = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (!post_data)
    {
        ESP_LOGE(TAG, "Failed to stringify JSON.");
        return NULL;
    }

    http_response_t response = {
        .buffer = malloc(1),
        .buffer_len = 0,
        .buffer_size = 1};

    if (!response.buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for response.");
        free(post_data);
        return NULL;
    }

    esp_http_client_config_t config = {
        .url = TOKEN_URI,
        .event_handler = _http_event_handler,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .user_data = &response,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    ESP_LOGI(TAG, "Sending HTTPS Request...");
    esp_err_t err = esp_http_client_perform(client);

    free(post_data);
    esp_http_client_cleanup(client);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTPS POST request failed: %s", esp_err_to_name(err));
        free(response.buffer);
        return NULL;
    }

    cJSON *resp_json = cJSON_Parse(response.buffer);
    free(response.buffer);

    if (!resp_json)
    {
        ESP_LOGE(TAG, "Failed to parse response JSON.");
        return NULL;
    }

    cJSON *access_token_json = cJSON_GetObjectItem(resp_json, "access_token");
    if (!cJSON_IsString(access_token_json))
    {
        ESP_LOGE(TAG, "Access token not found in response.");
        cJSON_Delete(resp_json);
        return NULL;
    }

    char *access_token = strdup(access_token_json->valuestring);
    cJSON_Delete(resp_json);

    if (!access_token)
    {

        ESP_LOGE(TAG, "Failed to obtain access token");
        free(access_token);
    }

    return access_token;
}

void upload_file(char *filename)
{
    return;
}