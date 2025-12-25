# Compiling the Source Code for PrintPooP Project

This guide provides instructions on how to set up your environment and compile the source code for the PrintPooP project using PlatformIO.

## 1. Prerequisites

* **IDE:** Visual Studio Code with the PlatformIO IDE extension.
* **Hardware:** CYD 2.4" ESP32 Dev Board (or compatible 2.8" version).

## 2. Project Setup

1.  Clone this repository to your local machine.
2.  Open the `sketch` folder in Visual Studio Code.
3.  PlatformIO should automatically detect the project and ask to install the dependencies. If not, you can install them manually using the PlatformIO sidebar.

The required libraries are listed in `platformio.ini` and will be installed automatically:
* `lovyan03/LovyanGFX`
* `lvgl/lvgl`
* `electroniccats/MPU6050`
* `knolleary/PubSubClient`
* `bblanchon/ArduinoJson`
* `fbiego/ESP32Time`

## 3. Partition Scheme

The project is configured to use a custom partition scheme `partitions/max_app_only.csv`. This is set in `platformio.ini` and no manual action is required.

## 4. Application Configuration

Before compiling, you can configure the application by editing `include/app_config.h`:

*   **Theme:** Choose between `KITTEN` and `PUPPY`.
*   **Board:** Select your CYD board model (`USE_CYD_24`, `USE_CYD_28_1`, `USE_CYD_28_2`).

```c++
//1. Select theme options
#define KITTEN 0
#define PUPPY 1

//Enter display theme you want to compile
#define THEME KITTEN //<- KITTEN or PUPPY
//-------------------------------------------

//2. Select board model options
#define USE_CYD_24 0   // for CYD 2.4"
#define USE_CYD_28_1 1 // for CYD 2.8" Variant 1
#define USE_CYD_28_2 2 // for CYD 2.8" Variant 2

// Enter the CYD model you want to compile
#define BOARD USE_CYD_28_1 //<- Set board here
//-------------------------------------------
```

## 5. LVGL Configuration

The `lv_conf.h` file is included in the `include` directory. The project is configured to use it automatically. No manual steps are required.

## 6. Compile and Upload

1.  Click on the PlatformIO icon in the VSCode activity bar.
2.  Under "Project Tasks", you can:
    *   **Build:** Compile the code.
    *   **Upload:** Compile and upload to your connected device.
    *   **Monitor:** Open the serial monitor.

---