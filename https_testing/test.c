esp_err_t upload_file()
{
    if (!is_wifi_connected())
    {
        ESP_LOGE(TAG, "Wi-Fi is not connected. Cannot upload file.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Wi-Fi connected, starting file upload...");

    // Open SD card directory and find the first .JPG file
    DIR *dir = opendir(MOUNT_POINT);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open SD card directory: %s", MOUNT_POINT);
        return ESP_FAIL;
    }

    struct dirent *entry;
    char *file_path = malloc(512); // Heap allocation
    if (!file_path)
    {
        ESP_LOGE(TAG, "Memory allocation failed for file path.");
        closedir(dir);
        return ESP_FAIL;
    }

    file_path[0] = '\0'; // Ensure empty string

    while ((entry = readdir(dir)) != NULL)
    {
        size_t len = strlen(entry->d_name);
        if (len > 4 && strcasecmp(&entry->d_name[len - 4], ".JPG") == 0)
        {
            snprintf(file_path, 512, "%s/%s", MOUNT_POINT, entry->d_name);
            ESP_LOGI(TAG, "Found file: %s", file_path);
            break;
        }
    }
    closedir(dir);

    if (file_path[0] == '\0')
    {
        ESP_LOGE(TAG, "No .JPG file found.");
        free(file_path);
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

    ESP_LOGI(TAG, "Access Token obtained!");

    // Open the file
    FILE *file = fopen(file_path, "rb");
    free(file_path); // File path no longer needed
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file.");
        free(access_token);
        return ESP_FAIL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0)
    {
        ESP_LOGE(TAG, "Invalid file size: %ld", file_size);
        fclose(file);
        free(access_token);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "File size: %ld bytes", file_size);

    // Create JSON metadata using cJSON
    cJSON *json_metadata = cJSON_CreateObject();
    cJSON_AddStringToObject(json_metadata, "name", "test_image.jpg");
    char *metadata_str = cJSON_PrintUnformatted(json_metadata);
    cJSON_Delete(json_metadata);

    if (!metadata_str)
    {
        ESP_LOGE(TAG, "Failed to create metadata JSON.");
        fclose(file);
        free(access_token);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Metadata JSON created successfully.");

    // Prepare request headers
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
        fclose(file);
        free(access_token);
        free(metadata_str);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "HTTP client initialized.");

    size_t auth_size = strlen(access_token) + 8; // "Bearer " (7 chars) + null terminator
    char *auth_header = (char *)malloc(auth_size);

    if (!auth_header)
    {
        ESP_LOGE(TAG, "Memory allocation failed for Authorization header.");
        esp_http_client_cleanup(client);
        fclose(file);
        free(access_token);
        return ESP_FAIL;
    }

    // Format the Authorization header dynamically
    snprintf(auth_header, auth_size, "Bearer %s", access_token);
    esp_http_client_set_header(client, "Authorization", auth_header);

    free(auth_header);

    esp_http_client_set_header(client, "Content-Type", "multipart/related; boundary=boundary123");

    // Prepare metadata header dynamically
    size_t metadata_header_size = strlen(metadata_str) + 128;
    char *metadata_header = malloc(metadata_header_size);
    if (!metadata_header)
    {
        ESP_LOGE(TAG, "Memory allocation failed for metadata header.");
        esp_http_client_cleanup(client);
        fclose(file);
        free(access_token);
        free(metadata_str);
        return ESP_FAIL;
    }

    snprintf(metadata_header, metadata_header_size,
             "--boundary123\r\nContent-Type: application/json; charset=UTF-8\r\n\r\n%s\r\n"
             "--boundary123\r\nContent-Type: application/octet-stream\r\n\r\n",
             metadata_str);
    free(metadata_str);

    ESP_LOGI(TAG, "Opening HTTP connection...");

    size_t total_size = file_size + strlen(metadata_header) + strlen("\r\n--boundary123--\r\n");
    esp_err_t err = esp_http_client_open(client, total_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection.");
        esp_http_client_cleanup(client);
        fclose(file);
        free(access_token);
        free(metadata_header);
        return err;
    }

    ESP_LOGI(TAG, "Sending metadata...");
    esp_http_client_write(client, metadata_header, strlen(metadata_header));
    free(metadata_header);

    // Send file data in chunks
    const size_t chunk_size = 2048; // Reduced chunk size
    char *buffer = malloc(chunk_size);
    if (!buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate chunk buffer.");
        esp_http_client_cleanup(client);
        fclose(file);
        free(access_token);
        return ESP_FAIL;
    }

    size_t bytes_read;
    size_t total_bytes_sent = 0;
    while ((bytes_read = fread(buffer, 1, chunk_size, file)) > 0)
    {
        // Convert buffer to hex string and log it
        char hex_str[bytes_read * 3 + 1]; // Each byte will be represented by 2 hex chars + space
        for (size_t i = 0; i < bytes_read; i++)
        {
            sprintf(&hex_str[i * 3], "%02X ", buffer[i]);
        }

        // Log the hex content of the buffer
        ESP_LOGI(TAG, "Buffer content (hex): %s", hex_str);

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

    ESP_LOGI(TAG, "Sending final boundary...");
    const char *end_boundary = "\r\n--boundary123--\r\n";
    esp_http_client_write(client, end_boundary, strlen(end_boundary));

    // Finalize request
    ESP_LOGI(TAG, "Finalizing HTTP request...");
    err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "File uploaded successfully.");
    }
    else
    {
        ESP_LOGE(TAG, "File upload failed.");
    }

    // Cleanup
    esp_http_client_cleanup(client);
    free(access_token);

    return err;
}