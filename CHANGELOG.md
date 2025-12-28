## Version history

### 1.6.1 (Dec 28th, 2025)
- Access Code can enter upper/lower case for other printer models.
### 1.6.0 (Nov 10th, 2025)
- "Kitten" and "Puppy" theme added
- Network configuration on screen
### 1.5.0 (Oct 10th, 2025)
- Replace the TFT_eSPI library to LovyanGFX library for easy configuration
- Disable brownout detection to prevent randomly reset when the ESP32 attemp to connect to WiFi implemented
- SD Card implemented for user custom image replacement in the future release.
### 1.4.6
- Add printing layer label
### 1.4.5
- The AMS current slot display is fixed.
- Temperature display fixed.
- RGB LED initialisation added
### 1.4.4 
- minor bug fixed
###1.4.3
- MQTT connection is difficult to connect, fixed.
- Primthead animation fixed
### 1.4.2 
- swing icon switch to animation when print stage change fixed
### 1.4.1 
- The AMS icon disappeared when print failed, and was fixed
### 1.4.0
- Integrate the MPU6050 gyro sensor.
- After one minute of inactivity, trigger a "PrintpooP" swinging animation for the idle state.
- After one minute of printing, initiate a "PrintpooP" swinging animation that follows the print head's movement.
https://youtu.be/ZBQTilp9lm4?si=et-ml-sNmkkoqPh6
