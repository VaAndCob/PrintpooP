/**
 @file printpoop_app
 @author Va&Cob
 @date 2025-12-01
 @copyright Copyright (c) 2025 Va&Cob

 # Hardware: ESP32 CYD 2.4", 2.8"
 # PlatformIO
 #
 #
 # Partition scheme: Max App Only (3.9MB)
**/
#include "app_config.h" //<- open this file in "include" folder to config compile option

//-------------------------------------------------------
#include <Arduino.h>
const String version = "1.7.0";
const String compile_date = __DATE__ " - " __TIME__;
//-------------------------------------------------------
#include "soc/rtc_cntl_reg.h" // Disable brownout problems
#include "soc/soc.h"          // Disable brownout problems

#include "LGFX_CYD.h"
#include "ui.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <lvgl.h>

// Pin configuration
#define SELECTOR_PIN 0
#define LDR_PIN 34
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPARE_PIN 35

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

// SOUNDER and RTC
#include "accessory.h"
#include "mqtt.h"
#include "nes_audio.h"


LGFX tft; /* LGFX instance */

//---------------

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushPixels((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp_drv);
}
/* Touch pad callback */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);

  if (touched) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

//------------------------------------------------------
void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detection
  // init communication
  Serial.begin(115200);
  delay(500);
  Serial.print(F("\n<< PrintpooP >> by Va&Cob\nVersion "));
  Serial.print(version);
  Serial.print(F(" | "));
  Serial.println(compile_date);
  //load screen flip setting
  pref.begin("config", true);
  flip = pref.getBool("flip", false);
  brightness = pref.getChar("brightness", 128);
  auto_dim = pref.getBool("autodim", true);
  pref.end();

// Pin configuration by board type
#if (BOARD == USE_CYD_24_1 || BOARD == USE_CYD_24_2)
if (flip) rotation = 6; else rotation = 4;//rotation = 4; // 6 rotate 180
#define SDA_PIN 21
#define SCL_PIN 22
  GAIN = 5.0; // speaker volume
  setVersion(version, "printpoop24_kitten_manifest.json");
#elif BOARD == USE_CYD_28_1
if (flip) rotation = 2; else rotation = 0;//rotation = 0; //or 2 rotate 180
#define SDA_PIN 27
#define SCL_PIN 22
  GAIN = 2.0; // speaker volume (recommend connect AMP IC pin 4 and 5 with R 1K ohm)
  setVersion(version, "printpoop28_1_kitten_manifest.json");
#elif BOARD == USE_CYD_28_2
if (flip) rotation = 6; else rotation = 4;//rotation = 4;// or 6 rotate 180
#define SDA_PIN 27
#define SCL_PIN 22
  GAIN = 2.0; // speaker volume
  setVersion(version, "printpoop28_2_kitten_manifest.json");
#endif

  // pin configuration
  analogSetAttenuation(ADC_0db);       // 0dB(1.0 ครั้ง) 0~800mV   for LDR
  pinMode(LDR_PIN, ANALOG);            // ldr analog input read brightness
  pinMode(SELECTOR_PIN, INPUT_PULLUP); // button
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  mpu_init(SDA_PIN, SCL_PIN); // init MPU6050

  // init TFT
  tft.begin();
  tft.setRotation(rotation); // Portrait
  tft.setBrightness(128);

  // screen calibration
  if (digitalRead(SELECTOR_PIN) == LOW) {
    uint16_t calData[8];
    tft.calibrateTouch(calData, TFT_WHITE, TFT_BLACK, 15);
  }

  lv_init();// init lvgl

  // LVGL display buffers
  static lv_disp_draw_buf_t draw_buf;
  // Allocate buffer for 1/6 of the screen size
  static lv_color_t buf1[TFT_WIDTH * TFT_HEIGHT / 6];
  static lv_color_t buf2[TFT_WIDTH * TFT_HEIGHT / 6];
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, TFT_WIDTH * TFT_HEIGHT / 6);

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
  
  //brightness
  if (auto_dim) {
    lv_obj_add_state(ui_setup_checkbox_autodim, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_setup_slider_brightness, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_state(ui_setup_checkbox_autodim, LV_STATE_CHECKED);
    lv_slider_set_value(ui_setup_slider_brightness, brightness, LV_ANIM_ON);
    lv_obj_clear_flag(ui_setup_slider_brightness, LV_OBJ_FLAG_HIDDEN);
    tft.setBrightness(brightness);
  }

  // lv_label_set_text(ui_status_label_printstage,"Idle"); //test swing
    Serial.printf(
        "Heap free: %u | Largest block: %u\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
    );
}

//------------------------------------------------------
void loop() {

  autoDim();
  wifi_status();
  mqtt_handler();
  update_clock();
  idle_animation();
  print_animation();

  lv_task_handler();
}
//------------------------------------------------------