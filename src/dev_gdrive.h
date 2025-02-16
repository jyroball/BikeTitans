#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"
#include "mbedtls/base64.h"
#include "mbedtls/rsa.h"
#include "mbedtls/md.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "esp_log.h"

#include "dev_credentials.h"

#define JWT_EXPIRATION_SECONDS 3600

#define TAG "GDRIVE"

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
    time_t now = time(NULL);
    if (now == ((time_t)-1))
    {
        ESP_LOGE(TAG, "Failed to get time");
        return NULL;
    }

    ESP_LOGI(TAG, "STARTING JWT HEADER JSON");

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

    ESP_LOGI(TAG, "ENCODING HEADER AND PAYLOAD");

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

    ESP_LOGI(TAG, "SIGNING");

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

    ESP_LOGI(TAG, "PRIVATE KEY LOADING");

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

    ESP_LOGI(TAG, "HASHING PRIVATE KEY");

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

    ESP_LOGI(TAG, "SIGNING HASH");

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

    ESP_LOGI(TAG, "FINAL JWT");

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
    ESP_LOGI(TAG, "FINISHED! FREEING MEMORY");
    free(encoded_header);
    free(encoded_payload);
    free(signing_input);
    free(encoded_signature);
    mbedtls_pk_free(&pk);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    return jwt;
}
