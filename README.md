# PrintpooP – Retro Pixel Smart Display for Bambu Lab A1
## WORK BOTH 2.4" and 2.8" Resistive Touch Screen
## Firmware version: 1.4.6 (display on the top left-most on the welcome screen)

Important Notice:

Heads up! This firmware is still a work in progress
It's not officially released yet, so things might break or not work as expected.
We’re working hard to get it ready — thanks for your understanding!
- Develop progress: ████████▒▒ 80%
- [Flash Firmware Online for 2.4" and 2.8"](https://vaandcob.github.io/webpage/src/index.html)
- [Version history](https://github.com/VaAndCob/PrintpooP/blob/main/CHANGELOG.md)
--------------------------  
🎬[Watch on Youtube](https://www.youtube.com/playlist?list=PLtzfxJvwUDCUxc5Is3-2UeWByNE8M8oDE)

![PrintpooP](/picture/animation.gif)

Introduction: This doesn’t make my prints any better… but it does make my printer look way cooler 😂

A compact, ESP32-powered accessory designed specifically for the Bambu Lab A1, A1 Mini 3D printer. It replaces the original hotend faceplate with a custom 3D-printed enclosure (the "Box") that houses a vibrant 2.4" touchscreen display.

📡 Live Status Display
PrintpooP connects to your A1 via Wi-Fi and presents real-time print data, such as print progress, temperatures, fan speeds, and Clock. Right at the print head. This localised status view makes monitoring more intuitive and visually engaging.

🎮 8-Bit Aesthetic
All visuals are styled in charming 8-bit pixel art, giving your printer a retro personality. Animations and icons are custom-designed to blend nostalgic design with functional UX and sound effects.

🛠️ Easy Mounting
The enclosure is engineered to replace the Bambu Lab A1's default hotend faceplate, securing PrintpooP in a clean, integrated form factor.


HOW TO USE: 
1. Touch screen calibration will be shown at the first run, but can be manually entered by pressing the RESET button and then pressing BOOT (GPIO_0) button within a second and hold
2. Follow the screen instructions. (Click the SETUP button on the screen to enter network configuration)
3. Access the setup web portal by connecting to the SSID PirntpooP_Setup using a smartphone or laptop.
4. Configure Wi-Fi credentials on the Configure Wi-Fi page.
5. Enter the printer's IP address, access code, serial number, and time zone on the SETUP page.
   
    How to get printer serial number:  https://wiki.bambulab.com/en/general/find-sn
   
    How to get printer IP address & Access Code:  https://wiki.bambulab.com/en/software/bambu-studio/failed-to-send-print-files
6. Currently, there are 4 status display pages
   
   Page 1 - PrintpooP in action  (display a cute cat on each print stage)

      - After one minute of inactivity, trigger a "PrintpooP" swinging animation for the idle state.
 
      - After one minute of printing, initiate a "PrintpooP" swinging animation that follows the print head's movement.
   
        (Optional →  need MPU6050 gyro sensor attached)
   
   Page 2 - Temperature / Fan speed/progress
   
   Page 3 - Print progress / remain / layer / clock
   
   Page 4 - AMS filament type and colour / current slot

![Swing](/picture/swing.gif)
![Screen](/picture/screen1.jpg)
![Network Setup](/picture/screen2.jpg)


[💾 Parts assembly & soldering](https://github.com/VaAndCob/PrintpooP/tree/main/hardware)

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


