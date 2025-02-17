#pragma once

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
#include "WiFi.h"
#include "SD.h"

#define JWT_EXPIRATION_SECONDS 3600
#define MAX_RESPONSE_SIZE 2048
#define TAG "GDRIVE"

typedef struct
{
    char *buffer;
    int buffer_len;
    int buffer_size;
} http_response_t;

// Base64 URL encoding function (removes padding and replaces characters)
void base64url_encode(const unsigned char *input, size_t input_len, char *output, size_t output_size);

// GDrive Oath
char *create_jwt();
char *get_access_token(void);

esp_err_t upload_file(void);