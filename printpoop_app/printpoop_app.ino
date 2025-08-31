/**
 * @file printpoop_app.ino
 * @author Va&Cob
 * @date 2025-05-01
 * @copyright Copyright (c) 2025 Va&Cob
 *
 * Hardware: CYD 2.4" ESP32 Dev Board
 * IDE: Arduino IDE 2.3.6
 * ESP32 Core: 2.0.7
 * Partition scheme: Max App Only (3.9MB)
**/

//------ Use 2.4" or 2.8" ----------------
/*
# to use 2.4" screen -> Edit "User_Setup_Select.h"
#include <User_Setups/Setup_CYD_24.h>
//#include <User_Setups/Setup_CYD_28_1.h>
//#include <User_Setups/Setup_CYD_28_2.h>

# to use 2.8" screen Variant 1 -> Edit "User_Setup_Select.h"
//#include <User_Setups/Setup_CYD_24.h>
#include <User_Setups/Setup_CYD_28_1.h>
//#include <User_Setups/Setup_CYD_28_2.h>

# to use 2.8" screen Variant 2 (Randomnerd) -> Edit "User_Setup_Select.h"
//#include <User_Setups/Setup_CYD_24.h>
//#include <User_Setups/Setup_CYD_28_1.h>
#include <User_Setups/Setup_CYD_28_2.h>

*/
#include <Arduino.h>
//-------------------------------------------------------
const String version = "1.4.6";
const String compile_date = __DATE__ " - " __TIME__;

#define USE_TFT_28  //comment out this line to use CYD2.4"
//-------------------------------------------------------

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <freertos/task.h>
#include <freertos/semphr.h>
}

#include <ArduinoJson.h>
#include <SPI.h>

#define LV_CONF_INCLUDE_SIMPLE 1  // Tell LVGL to look for custom lv_conf.
#include "lv_conf.h"              // Include your custom configuration FIRST before lvgl.h
#include "lvgl.h"
#include <TFT_eSPI.h>  // Setup_CYD_2_4.h
#include "ui.h"

//Pin configuration
#define SELECTOR_PIN 0
#define LDR_PIN 34
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17

// SOUNDER and RTC
#include "accessory.h"
#include "mqtt.h"
#include "nes_audio.h"

TFT_eSPI tft = TFT_eSPI(TFT_HEIGHT, TFT_WIDTH); /* TFT instance */

//---------------
#include "touch.h"

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp_drv);
}
/* Touch pad callback */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = getTouch(&touchX, &touchY);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
#ifdef SERIAL_DEBUG
    Serial.printf("%d - %d\n", touchX, touchY);
#endif
  }
}

//------------------------------------------------------
void setup() {
  //init communication
  Serial.begin(115200);
  delay(500);
  Serial.print(F("\n<< PrintpooP >> by Va&Cob\nVersion "));
  Serial.print(version);
  Serial.print(F(" | "));
  Serial.println(compile_date);


#ifdef USE_TFT_28
#define ROTATION 2
#define SDA_PIN 27
#define SCL_PIN 22
  GAIN = 2.0;//speaker volume (recommend connect AMP IC pin 4 and 5 with R 1K ohm)
  setVersion(version, "printpoop28_1_manifest.json");
#else
#define ROTATION 1
#define SDA_PIN 21
#define SCL_PIN 22
  GAIN = 5.0;//speaker volume
  setVersion(version, "printpoop24_manifest.json");
#endif

  //pin configuration
  analogSetAttenuation(ADC_0db);        // 0dB(1.0 ครั้ง) 0~800mV   for LDR
  pinMode(LDR_PIN, ANALOG);             //ldr analog input read brightness
  pinMode(SELECTOR_PIN, INPUT_PULLUP);  //button
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  mpu_init(SDA_PIN, SCL_PIN);// init MPU6050

  //init TFT
  tft.begin();
  tft.setRotation(ROTATION);  //Portrait
  init_TFT_BL(TFT_BL);        // attach backlight after tft.begin

  //init touch
  touchscreenSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchscreenSPI);
  ts.setRotation(ROTATION);

  touch_calibrate();  //press RESET then press & hold BOOT button for a second (GPIO0) to manually enter calibration

  lv_init();
  // LVGL display buffers
  static lv_color_t buf1[TFT_WIDTH * TFT_HEIGHT /6];  // Buffer for 40 rows (~19.2 KB)
  static lv_color_t buf2[TFT_WIDTH * TFT_HEIGHT /6];  // Optional second buffer for double-buffering
  static lv_disp_draw_buf_t draw_buf;

  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, TFT_WIDTH * TFT_HEIGHT /6);  // 40 rows buffered

  // Register display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Touch input driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();

 // lv_label_set_text(ui_status_label_printstage,"Idle"); //test swing
}

//------------------------------------------------------
void loop() {

  lv_timer_handler();
  autoDim();
  wm.process();
  wifi_status();
  mqtt_handler();
  update_clock();
  idle_animation();
  print_animation();
}
//------------------------------------------------------
