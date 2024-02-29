# Logicrom OpenCPU SDK - UART example

## How to build

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install [PlatformIO Extension for VSCode](https://platformio.org/platformio-ide)
3. Install Logicrom Platform:

   * Open PlatformIO Home
   * Go to Platforms -> Embedded (Top Navigation)
   * Type "Logicrom" in the search box
   * Click Install

4. Download and Extract examples from github https://github.com/waybyte/example-uart/archive/master.zip
5. Extract and Open example folder with *VSCode*
6. Run following command from terminal or use PlatformIO interface to build:

```bash
   # Build Project
   $ platformio run

   # Upload Project
   $ platformio run --target upload
```


## Resources

* [Quick Start Guide](https://docs.logicrom.com/en/latest/book/quick_start.html)
* [API Reference](https://docs.logicrom.com/)
