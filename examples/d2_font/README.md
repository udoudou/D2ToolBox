| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- |

# D2_font Example

This example show how to use the d2_font font engine. This tutorial covers the use of both .c and .bin formats. The .bin format demonstrates two methods: loading directly from a partition, and loading by the mmap access address from [esp_mmap_assets](https://components.espressif.com/components/espressif/esp_mmap_assets).

## How to use the example

This example uses the following settings menu to select different methods.
Run `idf.py menuconfig` and in `Example Configuration → Chinese font support` select.

If you select `Put in a partition`, please execute the following command to burn the font data into the `font` partition.

```
esptool.py -p PORT -b 921600 write_flash 0x110000 ./main/fonts/d2_font_demo_14.bin
```

### Hardware Required

* An ESP development board
* An ST7789 LCD panel, with SPI interface
* An USB cable for power supply and programming

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
       ESP Board                           ST7789 Panel
┌──────────────────────┐              ┌────────────────────┐
│             GND      ├─────────────►│ GND                │
│                      │              │                    │
│             3V3      ├─────────────►│ VCC                │
│                      │              │                    │
│             PCLK     ├─────────────►│ SCL                │
│                      │              │                    │
│             MOSI     ├─────────────►│ MOSI               │
│                      │              │                    │
│             MISO     |◄─────────────┤ MISO               │
│                      │              │                    │
│             RST      ├─────────────►│ RES                │
│                      │              │                    │
│             DC       ├─────────────►│ DC                 │
│                      │              │                    │
│             LCD CS   ├─────────────►│ LCD CS             │
│                      │              │                    │
│             BK_LIGHT ├─────────────►│ BLK                │
└──────────────────────┘              └────────────────────┘
```

The GPIO number used by this example can be changed in [main.c](main/main.c).

### Build and Flash

Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project. A fancy animation will show up on the LCD as expected.

The first time you run `idf.py` for the example will cost extra time as the build system needs to address the component dependencies and downloads the missing components from the ESP Component Registry into `managed_components` folder.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

### Example Output

```bash
...
I (423) app_start: Starting scheduler on CPU0
I (428) app_start: Starting scheduler on CPU1
I (428) main_task: Started on CPU0
I (438) main_task: Calling app_main()
I (438) example: Turn off LCD backlight
I (448) gpio: GPIO[46]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (458) example: Initialize SPI bus
I (458) example: Install panel IO
I (468) gpio: GPIO[16]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (468) example: Install ST7789 panel driver
I (478) gpio: GPIO[3]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (518) example: Turn on LCD backlight
I (518) example: Initialize LVGL library
I (518) example: Install LVGL tick timer
I (518) example: Register io panel event callback for LVGL flush ready notification
I (528) example: Create LVGL task
I (528) example: Starting LVGL task
I (598) example: Display LVGL Meter Widget
I (598) main_task: Returned from app_main()
...
```
