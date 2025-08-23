## D2ToolBox 概述

* [English Version](./README.md)

## 快速参考

### 硬件准备

您可以选择任意 ESP 系列开发板使用 D2ToolBox。

### 环境搭建

#### 搭建 ESP-IDF 开发环境

由于 D2ToolBox 依赖 ESP-IDF 的基础功能和编译工具，因此首先需要参考 [ESP-IDF 详细安装步骤](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#get-started-step-by-step) 完成 ESP-IDF 开发环境的搭建。

#### 从 ESP 组件注册表获取组件

如果您只想使用 D2ToolBox 中的组件，我们建议您从 ESP 组件注册表 [ESP Component Registry](https://components.espressif.com/) 中使用它。

D2ToolBox 中注册的组件如下:

<center>

| Component | Version |
| --- | --- |
| [d2_font](https://components.espressif.com/components/udoudou/d2_font) | [![Component Registry](https://components.espressif.com/components/udoudou/d2_font/badge.svg)](https://components.espressif.com/components/udoudou/d2_font) |

</center>

可以在项目根目录下使用 `idf.py add-dependency` 命令直接将组件从 Component Registry 添加到项目中。例如，执行 `idf.py add-dependency "udoudou/d2_font"` 命令添加 `d2_font`，该组件将在 `CMake` 步骤中自动下载

> 请参考 [IDF Component Manager](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html) 查看更多关于组件管理器的细节.

#### 从 D2ToolBox 仓库获取组件

如果您想为 `D2ToolBox` 中的组件或示例贡献代码，或者想基于 `D2ToolBox` 中的示例开发项目，您可以通过使用以下指令获取 D2ToolBox 代码仓库:

    ```
    git clone --recursive https://github.com/udoudou/D2ToolBox
    ```

#### 编译和下载示例

建议您 [构建您的第一个项目](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project) 以熟悉 ESP-IDF 并确保环境已经设置正确。

在 D2ToolBox 和 ESP-IDF 中构建和烧录示例没有区别。 在大多数情况下，您可以按照以下步骤在 D2ToolBox 中构建和烧录示例：

1. 将当前目录更改为示例目录，例如 `cd examples/d2_font`；
2. 运行 `idf.py set-target TARGET` 设置目标芯片。 例如 `idf.py set-target esp32s3` 将目标芯片设置为 `ESP32-S3`；
3. 运行 `idf.py build` 来构建示例；
4. 运行 `idf.py -p PORT flash monitor` 烧录示例，并查看串口输出。 例如 `idf.py -p /dev/ttyUSB0 flash monitor` 将示例烧录到 `/dev/ttyUSB0` 端口，并打开串口监视器。

### 其它参考资源

- 可以在 [ESP Component Registry](https://components.espressif.com/)中找到 `D2ToolBox` 中的组件和其他已注册的组件;
- 如果你在使用中发现了错误或者需要新的功能，请先查看 [GitHub Issues](https://github.com/udoudou/D2ToolBox/issues)，确保该问题不会被重复提交；
- 如果你有兴趣为 D2ToolBox 作贡献，请先阅读[贡献指南](./CONTRIBUTING.rst)。