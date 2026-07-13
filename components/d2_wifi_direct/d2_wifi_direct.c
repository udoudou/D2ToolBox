/*
 * SPDX-FileCopyrightText: 2026 udoudou
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "d2_wifi_direct.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <endian.h>

#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_wps.h"

#ifndef PIN2STR
#define PIN2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7]
#define PINSTR "%c%c%c%c%c%c%c%c"
#endif

#define D2_WIFI_DIRECT_DEVICE_NAME_MAX 32
#define D2_WIFI_DIRECT_ROC_WAIT_MS     3000

ESP_EVENT_DEFINE_BASE(D2_WIFI_DIRECT_EVENT);
ESP_EVENT_DEFINE_BASE(P2P_EVENT);

enum {
    P2P_EVENT_ROC_DONE,
};

static const char *TAG = "wifi_direct";

typedef struct {
    uint8_t element_id;     /**< Should be set to WIFI_VENDOR_IE_ELEMENT_ID (0xDD) */
    uint8_t length;          /**< Length of all bytes in the element data following this field. Minimum 4. */
    uint8_t vendor_oui[3];   /**< Vendor identifier (OUI). 0x50, 0x6F, 0x9A */
    uint8_t vendor_oui_type; /**< Vendor-specific OUI type. 0x09 */

    // ============ ATTRIBUTE 0x02: Capability ============
    uint8_t attr_type_02; /**< Attribute type. 0x02 */
    uint16_t attr_len_02; /**< Attribute length. 2 bytes */
    uint8_t device_cap; /**< Device capability. 1 bytes */
    uint8_t group_cap; /**< Group capability. 1 bytes */

    // ============ ATTRIBUTE 0x0D: P2P Device Info ============
    uint8_t attr_type_0D; /**< Attribute type. 0x0D */
    uint16_t attr_len_0D; /**< Attribute length. 2 bytes */
    uint8_t dev_addr[6]; /**< Device address. 6 bytes */
    uint16_t config_method_be; /**< Configuration method. 2 bytes */
    uint8_t primary_dev_type[8]; /**< Primary device type. 8 bytes */
    uint8_t secondary_dev_type_num; /**< Secondary device type number. 1 bytes */
    uint16_t device_name_attr_type_be; /**< Device name attribute type. 2 bytes 0x1011*/
    uint16_t device_name_attr_len_be; /**< Device name attribute length. max 32 */
    uint8_t device_name[32]; /**< Device name. 32 bytes */
} __attribute__((packed)) wifi_p2p_ie_data_t;

static esp_wps_config_t s_wps_config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);

static unsigned char pin[9] __attribute__((unused)); /* kept for WPS PIN mode compatibility */

static bool s_initialized;
static bool s_go_started;
static bool s_accepting_connection;
static uint8_t s_go_channel = 1;
static char s_device_name[D2_WIFI_DIRECT_DEVICE_NAME_MAX + 1];

static void p2p_listen_once(void);
static void p2p_cancel_roc(void);

static void stop_accepting_window(bool notify_stopped)
{
    if (!s_accepting_connection) {
        return;
    }
    s_accepting_connection = false;
    esp_wifi_ap_wps_disable();
    p2p_cancel_roc();
    if (notify_stopped) {
        esp_event_post(D2_WIFI_DIRECT_EVENT, D2_WIFI_DIRECT_EVENT_ACCEPTING_STOPPED, NULL, 0, 0);
    }
}

static void build_p2p_ie_data(wifi_p2p_ie_data_t *p2p_ie_data, const char *device_name)
{
#if 1
    uint8_t device_name_len = strlen(device_name);
    p2p_ie_data->element_id = WIFI_VENDOR_IE_ELEMENT_ID;
    p2p_ie_data->length = sizeof(wifi_p2p_ie_data_t) - sizeof(p2p_ie_data->device_name) - 2 + device_name_len;
    p2p_ie_data->vendor_oui[0] = 0x50;
    p2p_ie_data->vendor_oui[1] = 0x6F;
    p2p_ie_data->vendor_oui[2] = 0x9A;
    p2p_ie_data->vendor_oui_type = 0x09;

    p2p_ie_data->attr_type_02 = 0x02;
    p2p_ie_data->attr_len_02 = 2;
    p2p_ie_data->device_cap = 0x21;
    p2p_ie_data->group_cap = 0x01;

    p2p_ie_data->attr_type_0D = 0x0D;
    p2p_ie_data->attr_len_0D = offsetof(wifi_p2p_ie_data_t, device_name) - offsetof(wifi_p2p_ie_data_t, dev_addr) + device_name_len;
    esp_wifi_get_mac(WIFI_IF_STA, p2p_ie_data->dev_addr);
    p2p_ie_data->config_method_be = htobe16(0x0080);
    memcpy(p2p_ie_data->primary_dev_type, "\x00\x0A\x00\x50\xF2\x04\x00\x05", 8);
    p2p_ie_data->secondary_dev_type_num = 0x00;
    p2p_ie_data->device_name_attr_type_be = htobe16(0x1011);
    p2p_ie_data->device_name_attr_len_be = htobe16(device_name_len);
    memcpy(p2p_ie_data->device_name, device_name, device_name_len);
#else
    static const uint8_t P2P_response_ie_data[] = {
        // Vendor Specific IE header
        0xDD, 0x25, // Element ID: 0xDD, Length: 37 bytes

        // Wi-Fi Alliance OUI + P2P OUI Type
        0x50, 0x6F, 0x9A, 0x09,

        // ============ ATTRIBUTE 0x02: Capability ============
        0x02,       // Attribute: Capability (0x02)
        0x02, 0x00, // Length: 2 byte
        0x21,       // Capability value:
        0x01,       // Group Capability Bitmap field

        // ============ ATTRIBUTE 0x0D: P2P Device Info ============
        0x0D, 0x19, 0x00,                   // Length: 25 bytes
        0x24, 0x58, 0x7C, 0x61, 0x35, 0xAC, // MAC address
        0x00, 0x80,                         // Config Methods: push button
        0x00, 0x03, 0x00, 0x50, 0xF2, 0x04, 0x00, 0x05, // Primary Device Type
        0x00, // Number of Secondary Device Types
        0x10, 0x11, 0x00, 0x04, 'E', 'S', 'P',
        'A' // Device Name (keep as same as it in WSC IE)
    };

    p2p_ie_data->element_id = WIFI_VENDOR_IE_ELEMENT_ID;
    p2p_ie_data->vendor_oui[0] = 0x50;
    p2p_ie_data->vendor_oui[1] = 0x6F;
    p2p_ie_data->vendor_oui[2] = 0x9A;
    p2p_ie_data->vendor_oui_type = 0x09;

    p2p_ie_data->payload[0] = 0x02;
    p2p_ie_data->payload[1] = 0x02;
    p2p_ie_data->payload[2] = 0x00;
    // p2p_ie_data->payload[3] = 0x27;
    // p2p_ie_data->payload[4] = 0x0;
    p2p_ie_data->payload[3] = 0x21;
    p2p_ie_data->payload[4] = 0x01;

    p2p_ie_data->length = 4 + 5;
#endif

    ESP_LOG_BUFFER_HEX(TAG, p2p_ie_data, sizeof(wifi_p2p_ie_data_t));
}

static esp_err_t set_p2p_vendor_ie(bool enable)
{
    if (!enable) {
        esp_err_t err1 = esp_wifi_set_vendor_ie(false, WIFI_VND_IE_TYPE_PROBE_RESP, WIFI_VND_IE_ID_0, NULL);
        esp_err_t err2 = esp_wifi_set_vendor_ie(false, WIFI_VND_IE_TYPE_BEACON, WIFI_VND_IE_ID_0, NULL);
        return (err1 != ESP_OK) ? err1 : err2;
    }

    wifi_p2p_ie_data_t p2p_ie_data;
    memset(&p2p_ie_data, 0, sizeof(p2p_ie_data));
    build_p2p_ie_data(&p2p_ie_data, s_device_name);
    esp_err_t err = esp_wifi_set_vendor_ie(true, WIFI_VND_IE_TYPE_PROBE_RESP, WIFI_VND_IE_ID_0, &p2p_ie_data);
    if (err != ESP_OK) {
        return err;
    }
    return esp_wifi_set_vendor_ie(true, WIFI_VND_IE_TYPE_BEACON, WIFI_VND_IE_ID_0, &p2p_ie_data);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == P2P_EVENT) {
        switch (event_id) {
        case P2P_EVENT_ROC_DONE:
            if (s_go_started && s_accepting_connection) {
                p2p_listen_once();
            }
            break;
        default:
            break;
        }
        return;
    }

    switch (event_id) {
    case WIFI_EVENT_AP_WPS_RG_SUCCESS: {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_WPS_RG_SUCCESS");
        wifi_event_ap_wps_rg_success_t *evt = (wifi_event_ap_wps_rg_success_t *)event_data;
        ESP_LOGI(TAG, "station "MACSTR" WPS successful",
                 MAC2STR(evt->peer_macaddr));
        if (s_accepting_connection) {
            d2_wifi_direct_sta_connected_t connected = {0};
            memcpy(connected.mac, evt->peer_macaddr, sizeof(connected.mac));
            s_accepting_connection = false;
            esp_wifi_ap_wps_disable();
            p2p_cancel_roc();
            esp_event_post(D2_WIFI_DIRECT_EVENT, D2_WIFI_DIRECT_EVENT_STA_CONNECTED,
                           &connected, sizeof(connected), 0);
            esp_event_post(D2_WIFI_DIRECT_EVENT, D2_WIFI_DIRECT_EVENT_ACCEPTING_STOPPED,
                           NULL, 0, 0);
        }
    }
    break;
    case WIFI_EVENT_AP_WPS_RG_FAILED: {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_WPS_RG_FAILED");
        wifi_event_ap_wps_rg_fail_reason_t *evt = (wifi_event_ap_wps_rg_fail_reason_t *)event_data;
        ESP_LOGI(TAG, "station "MACSTR" WPS failed, reason=%d",
                 MAC2STR(evt->peer_macaddr), evt->reason);
        stop_accepting_window(true);
    }
    break;
    case WIFI_EVENT_AP_WPS_RG_TIMEOUT: {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_WPS_RG_TIMEOUT");
        stop_accepting_window(true);
    }
    break;
    case WIFI_EVENT_AP_WPS_RG_PIN: {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_WPS_RG_PIN");
        /* display the PIN code */
        wifi_event_ap_wps_rg_pin_t* event = (wifi_event_ap_wps_rg_pin_t *) event_data;
        ESP_LOGI(TAG, "WPS_PIN = " PINSTR, PIN2STR(event->pin_code));
    }
    break;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
    case WIFI_EVENT_HOME_CHANNEL_CHANGE: {
        wifi_event_home_channel_change_t* event = (wifi_event_home_channel_change_t *) event_data;
        ESP_LOGW(TAG, "new channel = %d", event->new_chan);
    }
    break;
#endif
    default:
        break;
    }
}

static int esp_p2p_rx_action(uint8_t *hdr, uint8_t *payload, size_t len, uint8_t channel);
static esp_err_t esp_send_action_frame(uint8_t *dest_mac, const uint8_t *buf, uint32_t len,
                                       uint8_t channel, uint32_t wait_time_ms)
{
    wifi_action_tx_req_t *req = calloc(1, sizeof(*req) + len);
    if (!req) {
        return ESP_FAIL;
    }

    req->ifx = WIFI_IF_STA;
    memcpy(req->dest_mac, dest_mac, sizeof(req->dest_mac));
    req->no_ack = false;
    req->data_len = len;
    req->rx_cb = esp_p2p_rx_action;
    req->channel = channel;
    req->wait_time_ms = wait_time_ms;
    req->type = WIFI_OFFCHAN_TX_REQ;
    memcpy(req->data, buf, req->data_len);

    ESP_LOGI(TAG, "P2P: Mgmt Tx - MAC:" MACSTR ", Channel-%"PRIu8", WaitT-%"PRIu32,
             MAC2STR(dest_mac), channel, wait_time_ms);

    if (ESP_OK != esp_wifi_action_tx_req(req)) {
        ESP_LOGI(TAG, "P2P: Failed to perform offchannel operation");
        free(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "P2P: Sent action frame %d", req->op_id);
    // s_current_tx_op_id = req->op_id;
    free(req);
    return ESP_OK;
}

// #include "../src/common/ieee802_11_defs.h"

// struct action_rx_param {
//     u8 sa[ETH_ALEN];
//     u32 channel;
//     u32 frm_len;
//     u32 vendor_data_len;
//     struct ieee80211_action action_frm[];
// };
#define WLAN_FC_GET_STYPE(fc)   (((fc) & 0x00f0) >> 4)
static int esp_p2p_rx_action(uint8_t *hdr, uint8_t *payload, size_t len, uint8_t channel)
{
    int ret = ESP_ERR_NOT_SUPPORTED;

    ESP_LOGI(TAG, "RX ACTION");
    // ESP_LOG_BUFFER_HEX(TAG, hdr, 24);
    // ESP_LOG_BUFFER_HEX(TAG, payload, len);

    if (WLAN_FC_GET_STYPE(*(uint16_t *)hdr) == 13) { //ACTION
        if (*(uint8_t *)payload == 0x4) { //WLAN_ACTION_PUBLIC
            if (*(uint8_t *)(payload + 1) == 0x09) { //WLAN_PA_VENDOR_SPECIFIC
                uint8_t oui[3] = {0x50, 0x6F, 0x9A};
                if (memcmp(payload + 2, oui, sizeof(oui)) == 0) {
                    if (*(uint8_t *)(payload + 5) == 0x09) { //P2P管理帧
                        const char *p2p_action_str[] = {
                            "GO Negotiation Request",
                            "GO Negotiation Response",
                            "GO Negotiation Confirmation",
                            "P2P Invitation Request",
                            "P2P Invitation Response",
                            "Device Discoverability Request",
                            "Device Discoverability Response",
                            "Provision Discovery Request",
                            "Provision Discovery Response",
                        };
                        uint8_t p2p_action_type = *(uint8_t*)(payload + 6);
                        uint8_t action_token = *(uint8_t*)(payload + 7);
                        const uint8_t *p2p_ie_ele_data = (payload + 8);
                        size_t ele_len = len - 8;
                        ESP_LOGI(TAG, "RX P2P ACTION %s", p2p_action_str[p2p_action_type]);
                        ESP_LOG_BUFFER_HEX(TAG, p2p_ie_ele_data, ele_len);
                        if (p2p_action_type == 0x7) {
                            uint8_t oui[3] = {0x00, 0x50, 0xF2};
                            while (ele_len > 2 + 3) {
                                uint8_t len = *(p2p_ie_ele_data + 1);
                                if (ele_len < 2 + len) {
                                    ESP_LOGI(TAG, "error len");
                                    break;
                                }
                                if (*p2p_ie_ele_data == 0xDD && memcmp(p2p_ie_ele_data + 2, oui, sizeof(oui)) == 0) {
                                    if (*(p2p_ie_ele_data + 5) == 0x4) { //WPS
                                        ESP_LOGI(TAG, "RX WPS IE");
                                        ESP_LOG_BUFFER_HEX(TAG, p2p_ie_ele_data + 2, len);
                                        const uint8_t *data = (p2p_ie_ele_data + 6);
                                        if (*(uint16_t *)data == 0x0810 && *(uint16_t *)(data + 2) == 0x0200) { //WPC
                                            uint16_t methods = *(uint16_t*)(data + 4);
                                            ESP_LOGI(TAG, "WPS methods: %04x", methods);
                                            if (methods & 0x8000) {  //push button
                                                ESP_LOGI(TAG, "WPS push button");
                                                uint8_t tx_data[] = {
                                                    0x04, 0x09,
                                                    0x50, 0x6F, 0x9A,
                                                    0x09,
                                                    0x08,       //Provision Discovery Response
                                                    action_token,   //token

                                                    0xDD,
                                                    0x0A,
                                                    0x00, 0x50, 0xF2,
                                                    0x04,
                                                    0x10, 0x08,  //WPC
                                                    0x00, 0x02,
                                                    0x00, 0x80,   //push button
                                                };
                                                esp_send_action_frame(hdr + 10, tx_data, sizeof(tx_data), s_go_channel, 300);
                                            }
                                        }
                                    }
                                    break;
                                }
                                p2p_ie_ele_data += (2 + len);
                                ele_len -= (2 + len);
                            }
                        }
                    }
                }
            }
        }
    }

    // struct ieee80211_hdr *rx_hdr = (struct ieee80211_hdr *)hdr;
    // struct ieee80211_action *frm = (struct ieee80211_action *)payload;

    // if (WLAN_FC_GET_STYPE(rx_hdr->frame_control) == WLAN_FC_STYPE_ACTION) {
    //     if (frm->category != WLAN_ACTION_PUBLIC) {
    //         ESP_LOGW(TAG, "Not public action frame");
    //         return -1;
    //     }

    //     // rx_param = os_zalloc(sizeof(struct action_rx_param) + len);
    //     // if (!rx_param) {
    //     //     ESP_LOGE(TAG, "Failed to allocate memory for Rx Action");
    //     //     return ESP_ERR_NO_MEM;
    //     // }
    //     // os_memcpy(rx_param->sa, rx_hdr->addr2, ETH_ALEN);
    //     // rx_param->channel = channel;
    //     // rx_param->frm_len = len;
    //     // os_memcpy(rx_param->action_frm, payload, len);

    //     // eloop_register_timeout(0, 0, esp_dpp_rx_action, rx_param, NULL);
    //     ESP_LOGI(TAG, "RX ACTION");
    // }

    return ret;
}

static void wifi_action_roc_done_cb(uint32_t context, uint8_t op_id, wifi_roc_done_status_t status)
{
    ESP_LOGI(TAG, "roc done, op_id=%d, status=%d", op_id, status);
    esp_event_post(P2P_EVENT, P2P_EVENT_ROC_DONE, NULL, 0, 0);
}

static void p2p_listen_once(void)
{
    esp_err_t ret;
    wifi_roc_req_t req = {0};
    req.ifx = WIFI_IF_STA;
    req.type = WIFI_ROC_REQ;
    req.channel = s_go_channel;
    req.wait_time_ms = D2_WIFI_DIRECT_ROC_WAIT_MS;
    req.rx_cb = esp_p2p_rx_action;
    req.done_cb = wifi_action_roc_done_cb;
    ret = esp_wifi_remain_on_channel(&req);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_remain_on_channel failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "esp_wifi_remain_on_channel over");
}

static void p2p_cancel_roc(void)
{
    wifi_roc_req_t req = {0};
    req.ifx = WIFI_IF_STA;
    req.type = WIFI_ROC_CANCEL;
    esp_wifi_remain_on_channel(&req);
}

esp_err_t d2_wifi_direct_init(const char *device_name)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (device_name == NULL || device_name[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    size_t name_len = strlen(device_name);
    if (name_len > D2_WIFI_DIRECT_DEVICE_NAME_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(s_device_name, device_name, name_len);
    s_device_name[name_len] = '\0';

    esp_err_t err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    if (err != ESP_OK) {
        return err;
    }
    err = esp_event_handler_register(P2P_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    if (err != ESP_OK) {
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler);
        return err;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "wifi direct initialized, device_name=%s", s_device_name);
    return ESP_OK;
}

esp_err_t d2_wifi_direct_start_go(uint8_t channel)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_go_started) {
        return ESP_ERR_INVALID_STATE;
    }
    if (channel < 1 || channel > 14) {
        return ESP_ERR_INVALID_ARG;
    }

    s_go_channel = channel;

    esp_err_t err = set_p2p_vendor_ie(true);
    if (err != ESP_OK) {
        return err;
    }

    s_go_started = true;

    ESP_LOGI(TAG, "P2P GO started on channel %d", s_go_channel);
    return ESP_OK;
}

esp_err_t d2_wifi_direct_go_start_new_connection(void)
{
    if (!s_go_started) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_accepting_connection) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "start wps...");

    esp_err_t err = esp_wifi_ap_wps_enable(&s_wps_config);
    if (err != ESP_OK) {
        return err;
    }

    err = esp_wifi_ap_wps_start(NULL);
    if (err != ESP_OK) {
        esp_wifi_ap_wps_disable();
        return err;
    }

    s_accepting_connection = true;
    p2p_listen_once();
    return ESP_OK;
}

esp_err_t d2_wifi_direct_go_stop_new_connection(void)
{
    if (!s_go_started) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!s_accepting_connection) {
        return ESP_OK;
    }

    stop_accepting_window(true);
    return ESP_OK;
}

esp_err_t d2_wifi_direct_stop_go(void)
{
    if (!s_go_started) {
        return ESP_ERR_INVALID_STATE;
    }

    /* Do not post public accepting-stopped event; GO itself is stopping */
    stop_accepting_window(false);

    s_go_started = false;
    p2p_cancel_roc();
    set_p2p_vendor_ie(false);
    ESP_LOGI(TAG, "P2P GO stopped");
    return ESP_OK;
}
