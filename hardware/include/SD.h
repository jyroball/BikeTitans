#pragma once

#include <dirent.h>
#include <string.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"
#define TAG_UPLOAD "DEV_SDCARD"

const char *get_sd_card_type(sdmmc_card_t *card);

esp_err_t mount_sd_card();
void unmount_sd_card();

void list_files_on_sd();
esp_err_t clear_sd_card();

esp_err_t test_open_file();
