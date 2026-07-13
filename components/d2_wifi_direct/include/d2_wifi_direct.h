/*
 * SPDX-FileCopyrightText: 2026 udoudou
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(D2_WIFI_DIRECT_EVENT);

typedef enum {
    D2_WIFI_DIRECT_EVENT_STA_CONNECTED,       /**< New device joined via WPS */
    D2_WIFI_DIRECT_EVENT_ACCEPTING_STOPPED,   /**< No longer accepting; call go_start_new_connection to resume */
} d2_wifi_direct_event_id_t;

typedef struct {
    uint8_t mac[6];
} d2_wifi_direct_sta_connected_t;

/**
 * @brief Initialize Wi-Fi Direct (P2P) stack.
 *
 * Must be called after Wi-Fi has been started in APSTA mode.
 * Stores the P2P device name and registers internal event handlers.
 *
 * @param device_name P2P device name, max 32 bytes
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if device_name is NULL/empty/too long
 *      - ESP_ERR_INVALID_STATE if already initialized
 */
esp_err_t d2_wifi_direct_init(const char *device_name);

/**
 * @brief Start as P2P Group Owner on the given operating channel.
 *
 * Adds P2P Vendor IE to beacon/probe response.
 * Call d2_wifi_direct_go_start_new_connection() to accept clients
 * (enables WPS and remain-on-channel for Provision Discovery).
 *
 * @param channel Operating channel (1-14)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG / ESP_ERR_INVALID_STATE on bad usage
 */
esp_err_t d2_wifi_direct_start_go(uint8_t channel);

/**
 * @brief Start accepting a new P2P client connection (WPS PBC).
 *
 * Enables WPS registrar and remain-on-channel listening.
 * When accepting ends, D2_WIFI_DIRECT_EVENT_ACCEPTING_STOPPED is posted;
 * call this API again to accept another device.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if GO is not started or already accepting
 */
esp_err_t d2_wifi_direct_go_start_new_connection(void);

/**
 * @brief Stop accepting new P2P client connections.
 *
 * Disables WPS, cancels remain-on-channel, and posts
 * D2_WIFI_DIRECT_EVENT_ACCEPTING_STOPPED.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if GO is not started
 */
esp_err_t d2_wifi_direct_go_stop_new_connection(void);

/**
 * @brief Stop P2P Group Owner mode.
 *
 * Stops WPS (if active), cancels remain-on-channel, and removes P2P Vendor IE.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if GO is not started
 */
esp_err_t d2_wifi_direct_stop_go(void);
