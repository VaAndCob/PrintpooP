# Compiling the Source Code for PrintPooP Project

This guide provides instructions on how to set up your environment and compile the source code for the PrintPooP project.

## 0. Add an additional Partition Scheme

https://github.com/VaAndCob/PrintpooP/blob/main/printpoop_app/addon/partitions/readme.md

## 1. Prerequisites & Setup

Ensure your environment matches the following requirements:

* **Hardware:** CYD 2.4" ESP32 Dev Board (or compatible 2.8" version, see specific setup below)
* **IDE:** Arduino IDE 2.3.6 (or compatible)
* **ESP32 Core:** Version 2.0.17 (or compatible)
* **Board Selection in IDE:** "ESP32 Dev Module"
* **Partition Scheme:** "Max App Only (3.9MB App)"
    * **Action:** You must add this custom partition scheme to your Arduino IDE *before* compiling. Instructions for this are located in the `/addon/partitions` folder of this project.
    https://github.com/VaAndCob/PrintpooP/tree/main/printpoop_app/addon/partitions

* **Required Libraries:**
    * `lovyanGFX 1.2.7`
    * `lvgl 8.3.11`
    * *(And any other libraries specified as needed by the sketch)*
    * **Action:** Ensure all necessary libraries are installed in your Arduino IDE.

---

## 2. Configure the `lvgl` Library

copy `lv_config.h` from addon folder into arduino library folder `<username>\Arduino\sketch\libraries`


## 3. Select Target Screen Size for Compilation

2.  **Edit `printpoop_app.ino`** (your main sketch file):
    * Go to approximately line 29 in `printpoop_app.ino`and set macro is defined (uncommented):
 
        ```cpp
        #define USE_CYD_24      //for CYD 2.4"
        #define USE_CYD_28_1     //for CYD 2.8" Variant 1
        #define USE_CYD_28_2    //for CYD 2.8" Variant 2
        ```        

---

## 4. Compile and Upload

After completing the above steps:

1.  Open the `printpoop_app.ino` sketch in your Arduino IDE.
2.  Ensure the correct Board ("ESP32 Dev Module") and Partition Scheme ("Max App Only (3.9MB App)") are selected under the `Tools` menu.
3.  Compile and upload the sketch to your ESP32 CYD board.

---
