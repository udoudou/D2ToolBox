/*
 * SPDX-FileCopyrightText: 2025 udoudou
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "src/font/lv_font.h"

/**
 * Loads a `lv_font_t` object from partition.
 * @param label Partition label where d2_font bin is stored.
 * @param[out] out_font Store lv_font_t pointer. IF failed, it will be set to NULL.
 * @return
 *     - ESP_OK: succeed
 *     - ESP_ERR_NOT_FOUND: no partitions found
 *     - ESP_ERR_INVALID_ARG: invalid argument
 *     - ESP_ERR_INVALID_CRC: data validation error
 *     - ESP_ERR_NO_MEM: Memory allocation failure
 */
esp_err_t d2_font_load_from_partition(const char* label, lv_font_t **out_font);

/**
 * Loads a `lv_font_t` object from partition.
 *
 * Note: The memory address space passed in should remain accessible until the `lv_font_t` object is freed.
 *
 * @param bin_ptr a pointer to d2_font bin. It can be the address obtained by mmap.
 * @param size d2_font bin size.
 * @param[out] out_font Store lv_font_t pointer. IF failed, it will be set to NULL.
 * @return
 *     - ESP_OK: succeed
 *     - ESP_ERR_INVALID_ARG: invalid argument
 *     - ESP_ERR_INVALID_CRC: data validation error
 *     - ESP_ERR_NO_MEM: Memory allocation failure
 */
esp_err_t d2_font_load_from_mem(const uint8_t *bin_ptr, size_t size, lv_font_t **out_font);

/**
 * Unload a `lv_font_t` object from `d2_font_load_xx`.
 * @param font `lv_font_t` object.
 */
void d2_font_unload(lv_font_t * font);

#ifdef __cplusplus
} /*extern "C"*/
#endif
