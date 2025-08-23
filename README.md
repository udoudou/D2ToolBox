## D2ToolBox Overview

* [中文版](./README_CN.md)

## Quick Reference

### Development Board

You can choose any of the ESP series development boards to use D2ToolBox.

### Setup Environment

#### Setup ESP-IDF Environment

D2ToolBox is developed based on ESP-IDF functions and tools, so ESP-IDF development environment must be set up first, you can refer [Setting up Development Environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#setting-up-development-environment) for the detailed steps.

#### Get Components from ESP Component Registry

If you just want to use the components in D2ToolBox, we recommend you use it from the [ESP Component Registry](https://components.espressif.com/).

The registered components in D2ToolBox are listed below:

<center>

| Component | Version |
| --- | --- |
| [d2_font](https://components.espressif.com/components/udoudou/d2_font) | [![Component Registry](https://components.espressif.com/components/udoudou/d2_font/badge.svg)](https://components.espressif.com/components/udoudou/d2_font) |

</center>

You can directly add the components from the Component Registry to your project by using the `idf.py add-dependency` command under your project's root directory. eg run `idf.py add-dependency "udoudou/d2_font"` to add the `d2_font`, the component will be downloaded automatically during the `CMake` step.

> Please refer to [IDF Component Manager](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html) for details.

#### Get D2ToolBox Repository

If you want to Contribute to the components in D2ToolBox or want to start from the examples in D2ToolBox, you can get the D2ToolBox repository by using the following command:

    ```
    git clone --recursive https://github.com/udoudou/D2ToolBox
    ```

#### Build and Flash Examples

Recommend you [Build Your First Project](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project) to get familiar with ESP-IDF and make sure the environment is set up correctly.

There is no difference between building and flashing the examples in D2ToolBox and in ESP-IDF. In most cases, you can build and flash the examples in D2ToolBox by following the steps:

1. Change the current directory to the example directory, eg `cd examples/d2_font`.
2. Run `idf.py set-target TARGET` to set the target chip. eg `idf.py set-target esp32s3` to set the target chip to ESP32-S3.
3. Run `idf.py build` to build the example.
4. Run `idf.py -p PORT flash monitor` to flash the example, and view the serial output. eg `idf.py -p /dev/ttyUSB0 flash monitor` to flash the example and view the serial output on `/dev/ttyUSB0`.

### Resources

- The [IDF Component Registry](https://components.espressif.com/) is where you can find the components in `D2ToolBox` and other registered components.
- [Check the Issues section on GitHub](https://github.com/udoudou/D2ToolBox/issues) if you find a bug or have a feature request. Please check existing Issues before opening a new one.
- If you're interested in contributing to D2ToolBox, please check the [Contributions Guide](./CONTRIBUTING.rst).