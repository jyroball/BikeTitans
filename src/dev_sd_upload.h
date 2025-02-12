#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#define IMAGE_MAX_SIZE (400 * 1024) // Keep this for buffer size when copying
#define MOUNT_POINT "/sdcard"

static const char *UPLOAD_TAG = "DEV_UPLOAD";

// Initialize SD card
static bool init_sd_card()
{
    esp_err_t ret;

    // Options for mounting the filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    // Use settings for SDMMC interface
    sdmmc_card_t *card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    ESP_LOGI(UPLOAD_TAG, "Mounting SD card...");
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        ESP_LOGE(UPLOAD_TAG, "Failed to mount SD card");
        return false;
    }

    ESP_LOGI(UPLOAD_TAG, "SD card mounted");
    return true;
}

// Copies an image from local storage to SD card
bool copy_image_to_sd(const char *source_path, const char *filename)
{
    if (!init_sd_card())
    {
        return false;
    }

    FILE *source = fopen(source_path, "rb");
    if (!source)
    {
        ESP_LOGE(UPLOAD_TAG, "Failed to open source file: %s", source_path);
        return false;
    }

    // Create the full destination path
    char dest_path[64];
    snprintf(dest_path, sizeof(dest_path), MOUNT_POINT "/%s", filename);

    FILE *dest = fopen(dest_path, "wb");
    if (!dest)
    {
        ESP_LOGE(UPLOAD_TAG, "Failed to create destination file: %s", dest_path);
        fclose(source);
        return false;
    }

    // Copy file in chunks to avoid using too much RAM
    uint8_t *buffer = (uint8_t *)malloc(32 * 1024); // 32KB buffer
    if (!buffer)
    {
        ESP_LOGE(UPLOAD_TAG, "Failed to allocate buffer");
        fclose(source);
        fclose(dest);
        return false;
    }

    size_t bytes_read, total_size = 0;
    while ((bytes_read = fread(buffer, 1, 32 * 1024, source)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read)
        {
            ESP_LOGE(UPLOAD_TAG, "Failed to write to SD card");
            free(buffer);
            fclose(source);
            fclose(dest);
            return false;
        }
        total_size += bytes_read;
    }

    free(buffer);
    fclose(source);
    fclose(dest);

    ESP_LOGI(UPLOAD_TAG, "Copied image to SD card (%zu bytes)", total_size);

    // Unmount partition and disable SDMMC peripheral
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(UPLOAD_TAG, "Card unmounted");

    return true;
}