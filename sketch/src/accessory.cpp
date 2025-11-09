// Helper functions

#include "accessory.h"
#include "ui.h"
#include <stdio.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>
//======================== RTC =================================
#include <ESP32Time.h>  //user ESP32 Internal Realtime Clock
ESP32Time rtc;          //offset GMT+7
//mytime currentTime;

String TZoffset = "0";
#define UTC_OFFSET_DST 0
#define NTP_SERVER "pool.ntp.org"

void initRTC() {

  // Convert GMT string to int and validate
  int gmtOffset = TZoffset.toInt();
  if (gmtOffset < -12 || gmtOffset > 14) {
    Serial.println(F("Invalid GMT offset, defaulting to 0"));
    gmtOffset = 0;
  }

  int32_t UTC_OFFSET = 3600 * TZoffset.toInt();
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  delay(100);
  //---------set with NTP---------------
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  } else {
    Serial.print(F("Unable to setup RTC!\n"));
  }
}

//get current time
String get_current_time() {

  struct tm timeinfo = rtc.getTimeStruct();  //get time from esp32 RTC
#ifdef SERIAL_DEBUG
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");  //  (tm struct) Sunday, January 17 2021 07:24:38
#endif
  /*
//Serial.println(&timeinfo, "%H:%M:%S - %d/%m/%Y   %Z");
  currentTime.h = timeinfo.tm_hour;
  currentTime.m = timeinfo.tm_min;
  currentTime.s = timeinfo.tm_sec; */
  char buffer[16];  // Enough for "HH:MM:SS\0"
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}


/*------------ SOUND EFFECT ----------*/
#define speakerChannel 2
#define SPEAKER_PIN 26  //speaker

void initSpeaker() {
  pinMode(SPEAKER_PIN, OUTPUT);                //speaker
  ledcSetup(speakerChannel, 1500, 10);         //speaker 10 bit
  ledcAttachPin(SPEAKER_PIN, speakerChannel);  //attach speaker
}

void beepbeep() {  //2 shot beep
  initSpeaker();
  ledcWriteTone(speakerChannel, 2000);
  delay(50);
  ledcWriteTone(speakerChannel, 0);
  delay(50);
  ledcWriteTone(speakerChannel, 2000);
  delay(50);
  ledcWriteTone(speakerChannel, 0);
}
void beep() {
  initSpeaker();
  ledcWriteTone(speakerChannel, 2000);
  delay(500);
  ledcWriteTone(speakerChannel, 0);
}
void clickSound() {
  initSpeaker();
  ledcWriteTone(speakerChannel, 5000);
  delay(5);
  ledcWriteTone(speakerChannel, 0);
}

//-----------  auto dim backlight function ------------
#define LDR_PIN 34  //LDR sensor
#define backlightChannel 0
static uint8_t low_count = 0;
static uint8_t high_count = 0;
static bool dim = false;                   //back light dim flag
static const uint16_t LIGHT_LEVEL = 1500;  //backlight control by light level Higher = darker light

void autoDim() {
  int light = analogRead(LDR_PIN);  //read light
  // Serial.printf("LDR: %d\n",light);
  if (light > LIGHT_LEVEL) {  //low light
    low_count++;
    if (low_count > 10) {
      low_count = 10;  //low amb light 10 times count
      if (!dim) {
         tft.setBrightness(50);  //dim light
        dim = true;
      }  //if !dim
    }    //if low_count
    high_count = 0;

  } else {
    high_count++;
    if (high_count > 10) {
      high_count = 10;  //high amb light 10 times count
      if (dim) {
        tft.setBrightness(200);
        dim = false;
      }  //if dim
    }    //if high_count
    low_count = 0;
  }  //if light > lightlevel
}  //autodim

void init_TFT_BL(uint8_t BL_PIN) {
  pinMode(BL_PIN, OUTPUT);
  ledcSetup(backlightChannel, 12000, 8);
  ledcAttachPin(BL_PIN, backlightChannel);
  ledcWrite(backlightChannel, 127);  //set screen brightness
}

//------------ Check new firmware update from web flasher --------------------
/*manifest.json
{
    "name": "PrintpooP",
    "version": "1.3.2",
    "new_install_prompt_erase": true, 
    "new_install_improv_wait_time": 0,
    "builds": [
      {
        "chipFamily": "ESP32",
        "parts": [
          { "path": "bootloader.bin", "offset": 4096 },
          { "path": "partitions.bin", "offset": 32768},
          { "path": "boot_app0.bin", "offset": 57344 },
          { "path": "printpoop24_app.ino.bin", "offset": 65536 }
        ]
      }
    ]
}*/

String current_version = "";    // current firmware version
bool firmware_checked = false;  //check firmware flag
String firmware_manifest = "";
//------------------------------------------------
void setVersion(const String ver, String manifest) {
  current_version = ver;
  firmware_manifest = manifest;
}

//function return firmware version to integer number
int versionToNumber(String version) {
  int major = version.substring(0, version.indexOf('.')).toInt();
  version = version.substring(version.indexOf('.') + 1);
  int minor = version.substring(0, version.indexOf('.')).toInt();
  version = version.substring(version.indexOf('.') + 1);
  int patch = version.toInt();
  // Combine the parts into a single integer for comparison
  return major * 10000 + minor * 100 + patch;
}
//----------------------------------------
//check a new firmware version available on github
bool newFirmwareAvailable() {
  Serial.print(F("\nChecking firmware updates\nCurrent version: "));
  Serial.println(current_version);
  // fetch the latest version from manifest.json
  String latestURL = String("https://vaandcob.github.io/webpage/firmware/printpoop/") + firmware_manifest;
  HTTPClient http;
  String payload = "";

  // Serial.print(F("Fetching manifest from: "));
  // Serial.println(latestURL);

  http.begin(latestURL);              // Specify the target server
  int httpResponseCode = http.GET();  // Send the GET request

  if (httpResponseCode > 0) {  // Check if the request was successful
    // Handle potential redirection
    if (httpResponseCode == HTTP_CODE_MOVED_PERMANENTLY || httpResponseCode == HTTP_CODE_FOUND) {
      String newUrl = http.header("Location");
      http.end();                     // End the current connection before starting a new one
      http.begin(newUrl);             // Begin new request with the new URL
      httpResponseCode = http.GET();  // Send the GET request to the new URL

      if (httpResponseCode > 0) {
        payload = http.getString();  // Get payload from redirected URL
      } else {
        http.end();
        return false;  // Failed after redirect
      }
    } else {
      payload = http.getString();  // Get payload from original URL
    }

    // --- ArduinoJson Parsing ---
    if (!payload.isEmpty()) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        http.end();
        return false;
      }

      // Check if the "version" key exists
      if (!doc["version"].isNull()) {
        const char* latestVersion_cstr = doc["version"];    // Get as const char*
        String latestVersion = String(latestVersion_cstr);  // Convert to Arduino String
        Serial.printf("Latest firmware: %s\n", latestVersion);
        firmware_checked = true;

        // version comparision
        if (versionToNumber(latestVersion) > versionToNumber(current_version)) {
          Serial.println(F("-> New firmware version available"));
          http.end();
          return true;
        } else {
          Serial.println(F("-> Firmware is up to date"));
          http.end();
          return false;
        }
      } else {
        Serial.println(F("Manifest JSON does not contain 'version' key."));
        http.end();
        return false;
      }
    } else {
      Serial.println(F("Received empty payload."));
      http.end();
      return false;
    }
    // --- End ArduinoJson Parsing ---
  } else {  // Initial HTTP Request failed
    Serial.print(F("Initial HTTP Request failed. Error code: "));
    Serial.println(httpResponseCode);
  }
  http.end();    // Close connection
  return false;  // Default return if something went wrong or no new version
}

//======================== MPU6050 =================================
bool print_started = false;
bool print_swing_now = false;
static long print_timer = 0;
static const float maxAcc = 4.0;  // Max acceleration in g (±8g)
static const int maxAngle = 350;
static float devider = 8192.0;  //for 4g accelometer
bool mpu_install = false;

MPU6050 mpu;
//-------------------------------
void mpu_init(byte sda, byte scl) {
  Wire.begin(sda, scl);
  Wire.setClock(400000);  // Set I2C clock speed to 100kHz
  mpu.initialize();
  delay(100);  // Let it stabilize
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
  uint8_t id = mpu.getDeviceID();
  if (id != 0) {
    Serial.printf("✅ MPU6050 detected: 0x%02X\n", id);
    mpu_install = true;
  } else {
    Serial.printf("❌ MPU6050 Error: 0x%02X\n", id);
    mpu_install = false;
  }
}
//---------------------------------
// read gate level
int16_t mpu_read() {
  static float xAccFiltered = 0;  // <-- persistent filter state
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  //float acx = ax / devider;
  float acy = ay / devider;
  // float acz = az / devider;
  return acy * -100;
}
//---------------------------------
//printpoop swing animation while printing
void print_animation() {
  if (mpu_install) {
    if (strcmp(lv_label_get_text(ui_status_label_printstage), "Printing") == 0) {
      if (!print_started) {
        print_started = true;
        print_timer = millis();  //reset timer
      } else {
        if (!print_swing_now && (millis() - print_timer > 60000)) {  //time out show swing animation
          print_swing_now = true;
          Serial.println("print swing");

          lv_obj_add_flag(ui_status_image_ppimage, LV_OBJ_FLAG_HIDDEN);
          lv_obj_clear_flag(ui_status_image_swing, LV_OBJ_FLAG_HIDDEN);
        }
      }
    } else {  // switch back to normal animation
      if (print_swing_now) {
        print_started = false;
        print_swing_now = false;
        lv_obj_add_flag(ui_status_image_swing, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_status_image_ppimage, LV_OBJ_FLAG_HIDDEN);
        lv_anim_del(ui_status_image_swing, NULL);
   
      }
    }

    if (print_swing_now) {  //animate swing animation
      lv_img_set_angle(ui_status_image_swing, mpu_read());
    }
  }
}
//----------------------------------
// Init SD CARD
SPIClass sd_hspi(HSPI);  // Initialize a new SPIClass object for the SD card.
void initSDCard() {
  sd_hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
}
//-----------------------------------------