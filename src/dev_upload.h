#ifndef DEV_UPLOAD_H
#define DEV_UPLOAD_H

#include <stdio.h>
#include <dirent.h>
#include <sys/unistd.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"
#define TAG_UPLOAD "DEV_UPLOAD"

static sdmmc_card_t *sd_card;

// Function to print SD card type
const char *get_sd_card_type(sdmmc_card_t *card)
{
    uint32_t capacity = card->csd.capacity; // Capacity in sectors (512 bytes per sector)
    if (capacity < 4096 * 1024)
    { // Less than 2GB (SDSC)
        return "SDSC (Standard Capacity)";
    }
    else if (capacity < 65536 * 1024)
    { // Less than 32GB (SDHC)
        return "SDHC (High Capacity)";
    }
    else
    { // 32GB or larger (SDXC)
        return "SDXC (Extended Capacity)";
    }
}

esp_err_t mount_sd_card()
{
    esp_err_t ret;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Update GPIO pins for ESP32-WROVER
    slot_config.clk = GPIO_NUM_14;
    slot_config.cmd = GPIO_NUM_15;
    slot_config.d0 = GPIO_NUM_2;
    slot_config.d1 = GPIO_NUM_4;
    slot_config.d2 = GPIO_NUM_12;
    slot_config.d3 = GPIO_NUM_13;

    // Enable internal pullups on the SD card lines
    slot_config.width = 4; // Use 4-bit mode
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG_UPLOAD, "Mounting SD card...");
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &sd_card);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_UPLOAD, "Mount failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG_UPLOAD, "SD card mounted successfully.");
    sdmmc_card_print_info(stdout, sd_card);

    // Print SD card type
    const char *card_type = get_sd_card_type(sd_card);
    ESP_LOGI(TAG_UPLOAD, "SD card type: %s", card_type);

    return ESP_OK;
}

esp_err_t save_image_to_sd(const char *source_path, const char *dest_name)
{
    char dest_path[128];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", MOUNT_POINT, dest_name);

    FILE *src = fopen(source_path, "rb");
    if (!src)
    {
        ESP_LOGE(TAG_UPLOAD, "Failed to open source file: %s", source_path);
        return ESP_FAIL;
    }

    FILE *dst = fopen(dest_path, "wb");
    if (!dst)
    {
        fclose(src);
        ESP_LOGE(TAG_UPLOAD, "Failed to create destination file: %s", dest_path);
        return ESP_FAIL;
    }

    uint8_t buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        fwrite(buffer, 1, bytes_read, dst);
    }

    fclose(src);
    fclose(dst);
    ESP_LOGI(TAG_UPLOAD, "File saved successfully: %s", dest_path);
    return ESP_OK;
}

esp_err_t clear_sd_card()
{
    DIR *dir = opendir(MOUNT_POINT);
    if (!dir)
    {
        ESP_LOGE(TAG_UPLOAD, "Failed to open directory: %s", MOUNT_POINT);
        return ESP_FAIL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_DIR)
        { // Skip directories
            char full_path[FILENAME_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, entry->d_name);
            unlink(full_path);
            ESP_LOGI(TAG_UPLOAD, "Deleted file: %s", full_path);
        }
    }
    closedir(dir);
    ESP_LOGI(TAG_UPLOAD, "SD card cleared successfully.");
    return ESP_OK;
}

void unmount_sd_card()
{
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, sd_card);
    ESP_LOGI(TAG_UPLOAD, "SD card unmounted");
}

#endif