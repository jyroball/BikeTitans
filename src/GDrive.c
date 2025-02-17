#include "GDrive.h"

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
    ESP_LOGI(TAG, "JWT obtained!");

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

esp_err_t upload_file()
{
    if (!is_wifi_connected())
    {
        ESP_LOGE(TAG, "Wi-Fi is not connected. Cannot upload file.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Wi-Fi connected, starting file upload...");

    // Locate first .JPG file on SD card
    DIR *dir = opendir(MOUNT_POINT);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open SD card directory.");
        return ESP_FAIL;
    }

    struct dirent *entry;
    char *file_path = NULL;

    while ((entry = readdir(dir)) != NULL)
    {
        size_t len = strlen(entry->d_name);
        if (len > 4 && strcasecmp(&entry->d_name[len - 4], ".JPG") == 0)
        {
            file_path = malloc(strlen(MOUNT_POINT) + len + 2);
            if (!file_path)
            {
                ESP_LOGE(TAG, "Memory allocation failed for file path.");
                closedir(dir);
                return ESP_FAIL;
            }
            sprintf(file_path, "%s/%s", MOUNT_POINT, entry->d_name);
            ESP_LOGI(TAG, "Found file: %s", file_path);
            break;
        }
    }
    closedir(dir);

    if (!file_path)
    {
        ESP_LOGE(TAG, "No .JPG file found.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Retrieving access token...");
    char *access_token = get_access_token();
    if (!access_token)
    {
        ESP_LOGE(TAG, "Failed to obtain access token.");
        free(file_path);
        return ESP_FAIL;
    }

    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file.");
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0)
    {
        ESP_LOGE(TAG, "Invalid file size.");
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "File size: %ld bytes", file_size);

    // Create metadata JSON
    cJSON *json_metadata = cJSON_CreateObject();
    cJSON_AddStringToObject(json_metadata, "name", "test_image.jpg");
    char *metadata_str = cJSON_PrintUnformatted(json_metadata);
    cJSON_Delete(json_metadata);

    if (!metadata_str)
    {
        ESP_LOGE(TAG, "Failed to create metadata JSON.");
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }

    // Define boundary
    const char *boundary = "boundary123";

    // Allocate metadata part
    size_t metadata_part_size = strlen(boundary) + strlen(metadata_str) + 128;
    char *metadata_part = malloc(metadata_part_size);
    if (!metadata_part)
    {
        ESP_LOGE(TAG, "Memory allocation failed for metadata.");
        free(metadata_str);
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }
    snprintf(metadata_part, metadata_part_size,
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"metadata\"\r\n"
             "Content-Type: application/json; charset=UTF-8\r\n"
             "\r\n"
             "%s\r\n",
             boundary, metadata_str);
    free(metadata_str);

    // Allocate file header part
    size_t file_header_size = strlen(boundary) + 128;
    char *file_header = malloc(file_header_size);
    if (!file_header)
    {
        ESP_LOGE(TAG, "Memory allocation failed for file header.");
        free(metadata_part);
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }
    snprintf(file_header, file_header_size,
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"file\"; filename=\"test_image.jpg\"\r\n"
             "Content-Type: application/octet-stream\r\n"
             "\r\n",
             boundary);

    // Final boundary
    char *end_boundary = malloc(strlen(boundary) + 10);
    if (!end_boundary)
    {
        ESP_LOGE(TAG, "Memory allocation failed for end boundary.");
        free(metadata_part);
        free(file_header);
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }
    snprintf(end_boundary, strlen(boundary) + 10, "\r\n--%s--\r\n", boundary);

    // Calculate total size
    size_t total_size = strlen(metadata_part) + strlen(file_header) + file_size + strlen(end_boundary);

    // Prepare HTTP client
    esp_http_client_config_t config = {
        .url = GDRIVE_UPLOAD_URL,
        .method = HTTP_METHOD_PATCH,
        .event_handler = _http_event_handler,
        .timeout_ms = 30000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = 8192,
        .buffer_size_tx = 8192};

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        ESP_LOGE(TAG, "Failed to initialize HTTP client.");
        free(metadata_part);
        free(file_header);
        free(end_boundary);
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }

    // Set authorization header
    char *auth_header = malloc(strlen(access_token) + 32);
    if (!auth_header)
    {
        ESP_LOGE(TAG, "Memory allocation failed for auth header.");
        free(metadata_part);
        free(file_header);
        free(end_boundary);
        esp_http_client_cleanup(client);
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }
    snprintf(auth_header, strlen(access_token) + 32, "Bearer %s", access_token);
    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "multipart/form-data; boundary=boundary123");

    esp_err_t err = esp_http_client_open(client, total_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection.");
        free(metadata_part);
        free(file_header);
        free(end_boundary);
        free(auth_header);
        esp_http_client_cleanup(client);
        fclose(file);
        free(file_path);
        free(access_token);
        return err;
    }

    // Send metadata
    esp_http_client_write(client, metadata_part, strlen(metadata_part));
    free(metadata_part);

    // Send file header
    esp_http_client_write(client, file_header, strlen(file_header));
    free(file_header);

    // Send file content
    const size_t chunk_size = 2048;
    char *buffer = malloc(chunk_size);
    if (!buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate chunk buffer.");
        free(auth_header);
        esp_http_client_cleanup(client);
        fclose(file);
        free(file_path);
        free(access_token);
        return ESP_FAIL;
    }

    size_t bytes_read;
    size_t total_bytes_sent = 0;
    while ((bytes_read = fread(buffer, 1, chunk_size, file)) > 0)
    {
        esp_err_t write_err = esp_http_client_write(client, buffer, bytes_read);
        if (write_err <= 0)
        {
            ESP_LOGE(TAG, "Failed to write chunk to HTTP stream.");
            free(buffer);
            esp_http_client_cleanup(client);
            fclose(file);
            free(access_token);
            return ESP_FAIL;
        }
        total_bytes_sent += bytes_read;
        ESP_LOGI(TAG, "Uploaded %zu/%ld bytes...", total_bytes_sent, file_size);
    }

    free(buffer);
    fclose(file);

    // Send final boundary
    esp_http_client_write(client, end_boundary, strlen(end_boundary));
    free(end_boundary);

    // Finalize request
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(auth_header);
    free(file_path);
    free(access_token);

    ESP_LOGI(TAG, "File upload complete!");
    return ESP_OK;
}
