idf_component_register(SRCS "main.c" "lvgl_demo_ui.c" "d2_font_demo_14.c" "lvgl_font_demo_14.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_lcd)

if(CONFIG_EXAMPLE_FONT_IN_MMAP_ASSETS)
    spiffs_create_partition_assets(
        "font"
        "./fonts"
        FLASH_IN_PROJECT
        MMAP_FILE_SUPPORT_FORMAT ".bin"
    )
endif()
