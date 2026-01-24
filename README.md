# PrintpooP – Retro Pixel Smart Display for Bambu Lab A1
![Version](https://img.shields.io/badge/version-1.7.1-blue)
- WORK BOTH 2.4" and 2.8" Resistive Touch Screen
- Firmware version: display on the top left-most on the welcome screen
- [Version history](https://github.com/VaAndCob/PrintpooP/blob/main/CHANGELOG.md)
## Quick start, flash and go, no code needed
- [Flash Firmware Online for 2.4" and 2.8"](https://vaandcob.github.io/webpage/src/index.html)
--------------------------  
🎬[Watch on Youtube](https://www.youtube.com/playlist?list=PLtzfxJvwUDCUxc5Is3-2UeWByNE8M8oDE)

![PrintpooP](/picture/animation.gif)

Introduction: This doesn’t make my prints any better… but it does make my printer look way cooler 😂

A compact, ESP32-powered accessory designed specifically for the Bambu Lab A1, A1 Mini 3D printer. It replaces the original hotend faceplate with a custom 3D-printed enclosure (the "Box") that houses a vibrant 2.4" touchscreen display.

📡 Live Status Display
PrintpooP connects to your A1 via Wi-Fi and presents real-time print data, such as print progress, temperatures, fan speeds, and Clock. Right at the print head. This localised status view makes monitoring more intuitive and visually engaging.

🎮 8-Bit Aesthetic
All visuals are styled in charming 8-bit pixel art, giving your printer a retro personality. Animations and icons are custom-designed to blend nostalgic design with functional UX and sound effects.
Choose between two adorable themes: a playful **Kitten** or a loyal **Puppy** to accompany your printing journey!
🛠️ Easy Mounting
The enclosure is engineered to replace the Bambu Lab A1's default hotend faceplate, securing PrintpooP in a clean, integrated form factor.


HOW TO USE: 
1. Touch screen calibration will be displayed during the first run, but can also be manually entered by pressing the RESET button and releasing, then pressing the BOOT (GPIO_0) button within one second and holding it for a second.
Use a touch pen to touch all 4 corners of the screen.
2. SETUP → Wi-Fi Tab to configure Wi-Fi credentials.
3. SETUP → Printer Tab to configure the printer's IP address, access code, serial number, and time zone.
4. Enter the printer's IP address, access code, serial number, and time zone on the SETUP → Printer Tab
   
    How to get printer serial number:  https://wiki.bambulab.com/en/general/find-sn
   
    How to get printer IP address & Access Code:  https://wiki.bambulab.com/en/software/bambu-studio/failed-to-send-print-files
5. Currently, there are 4 status display pages
   
   Page 1 - PrintpooP in action (displays a cute kitten or puppy for each print stage)

      - After one minute of inactivity, trigger a "PrintpooP" swinging animation for the idle state.
 
      - After one minute of printing, initiate a "PrintpooP" swinging animation that follows the print head's movement.
   
         (This animation needs an MPU6050 gyro sensor attached as an option. The other functions work normally without this sensor.)
   
   Page 2 - Temperature / Fan speed/progress
   
   Page 3 - Print progress / remain / layer / clock
   
   Page 4 - AMS filament type and colour / current slot

![Swing](/picture/swing.gif)
![Screen](/picture/ui.png)



## [💾 Parts assembly & soldering](https://github.com/VaAndCob/PrintpooP/tree/main/hardware)

FAQ: 
- Why is support for custom images or user-defined graphics currently unavailable?
- Why is Over-The-Air (OTA) firmware update functionality not included?
Answer: It's the hardware limitation.

---------------------------------------------------------------------------------------------------

## Part list:

- [3D Print part at MakerWorld](https://makerworld.com/en/models/1432974-PrintpooP-faceplate-kit#profileId-1490390)
- [ESP32 2.4" 240 x 320 Resistive Touch Display](https://s.click.aliexpress.com/e/_omgP1zh)
- [1W 8R 2809 Loud speaker 8 ohms 1 Watt 8R 1W 28*9*3.6MM](https://s.click.aliexpress.com/e/_oDSKVf9)
- [GY-521 MPU6050 3-Axis gyro accelerometer](https://s.click.aliexpress.com/e/_oF7jqOa)

## License

- Code: MIT License (Non-Commercial)
- 3D Designs: CC BY-NC-SA 4.0

Commercial use is strictly prohibited. For licensing inquiries, contact [Va&Cob](https://www.facebook.com/VaAndCob)


---------------------------------------------------------------------------------------------------
[![Buy Me a Coffee](https://img.buymeacoffee.com/button-api/?text=Buy%20me%20a%20coffee&emoji=☕&slug=vaandcob&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff)](https://www.buymeacoffee.com/vaandcob)



