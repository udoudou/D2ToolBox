idf_component_register(SRCS "d2_wifi_direct.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_event
                       PRIV_REQUIRES esp_wifi wpa_supplicant)
