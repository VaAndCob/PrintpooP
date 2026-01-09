#include <Arduino.h>
#include "app_config.h"
#include "ui.h"
#include "ui_events.h"
#include "mqtt.h"
#include "accessory.h"

#include "song.h"
#include "nes_audio.h"




// App start here
void wait5second(lv_event_t* e) {
  lv_label_set_text(ui_status_label_wifisymbol, LV_SYMBOL_WIFI);
  lv_label_set_text(ui_setup_label_setup_label1, LV_SYMBOL_CLOSE " CLOSE");
  lv_label_set_text(ui_setup_label_setup_label2, LV_SYMBOL_SAVE " SAVE");
  lv_label_set_text(ui_setup_label_setup_label3, LV_SYMBOL_SAVE " SAVE");
  lv_label_set_text(ui_setup_label_setup_label4, LV_SYMBOL_REFRESH " Rotate 180°");
  

  lv_label_set_text(ui_intro_label_version, current_version.c_str());
  startMusicPlayback(start, 7);
  nonBlockDelaySec(5);
  _ui_screen_change(&ui_screen_status, LV_SCR_LOAD_ANIM_OVER_TOP, 500, 0, &ui_screen_status_screen_init);
  lv_anim_del(ui_intro_image_cap, (lv_anim_exec_xcb_t)_ui_anim_callback_set_image_angle);  //stop spinning
  
#if THEME == KITTEN
  lv_img_set_src(ui_status_image_ppimage, &ui_img_kitten_stage_1_png);
  lv_img_set_src(ui_status_image_swing, &ui_img_kitten_swing_png);
  lv_img_set_pivot(ui_status_image_swing,64,0);
  lv_label_set_text(ui_status_label_ppmessage, "Meow! I am PrintpooP (Swipe left/right for pages)");
#elif THEME == PUPPY
  lv_img_set_src(ui_status_image_ppimage, &ui_img_puppy_stage_1_png);
  lv_img_set_src(ui_status_image_swing, &ui_img_puppy_swing_png);
  lv_img_set_pivot(ui_status_image_swing,64,0);
  lv_label_set_text(ui_status_label_ppmessage, "Woof! I am PrintpooP (Swipe left/right for pages)");
#endif
  wifi_init();
  mqtt_init();
  initRTC();
}


// wifi setup tab
void openSetupScreen(lv_event_t* e) {
  clickSound();
  wm_configmode = true; // stop mqtt attemp to connect
  lv_label_set_text(ui_setup_label_wifissid, WIFI_SSID.c_str());
  lv_textarea_set_text(ui_setup_textarea_password, WIFI_PASS.c_str());
}

void closeSetupScreen(lv_event_t* e) {
  clickSound();
  wm_configmode = false;  //let mqtt attemp to connect
}

static const uint8_t maxPage = 4;

void switch_previous_page(lv_event_t* e) {
  clickSound();
  pageIndex++;
  pageIndex = (pageIndex >= maxPage) ? 0 : pageIndex;
  page_control(pageIndex);
}

void switch_next_page(lv_event_t* e) {
  clickSound();
  pageIndex--;
  pageIndex = (pageIndex < 0) ? maxPage - 1 : pageIndex;
  page_control(pageIndex);
}

void closeSwingAnimation(lv_event_t* e) {  //stop swing animation
  clickSound();
  _ui_flag_modify(ui_status_image_swing, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
  _ui_flag_modify(ui_status_image_ppimage, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
  lv_anim_del(ui_status_image_swing, NULL);
  long_idle = false;
  idle_swing_now = false;
  print_started = false;
  print_swing_now = false;
  lv_img_set_angle(ui_status_image_swing, 0);  //reset position
  lv_label_set_text(ui_status_label_ppmessage, "Meow! I am PrintpooP (Swipe left/right for pages)");
}

void saveSetup(lv_event_t* e) {
  beep();
}

void togglekeyboard(lv_event_t* e) {
  clickSound();
  lv_obj_clear_flag(ui_setup_keyboard_keyboard1, LV_OBJ_FLAG_HIDDEN);
}


//setup tab selected
void setupTabChanged(lv_event_t* e) {
  static int last_tab_idx = -1;  // last_tab_idx value will be persist in this funciton
  uint8_t tab_idx = lv_tabview_get_tab_act(ui_setup_tabview_tabview1);
  if (tab_idx == last_tab_idx) return;  // ignore duplicates
  last_tab_idx = tab_idx;
  clickSound();
  switch (tab_idx) {
    case 0:  // WiFi
      lv_label_set_text(ui_setup_label_wifissid, WIFI_SSID.c_str());
      lv_textarea_set_text(ui_setup_textarea_password, WIFI_PASS.c_str());
      break;
    case 1:  // Printer

      lv_textarea_set_text(ui_setup_textarea_printerip, MQTT_SERVER_IP.c_str());
      lv_textarea_set_text(ui_setup_textarea_accesscode, MQTT_SERVER_PASS.c_str());
      lv_textarea_set_text(ui_setup_textarea_printerserial, MQTT_SERVER_SERIAL.c_str());
      lv_textarea_set_text(ui_setup_textarea_timezone, TZoffset.c_str());

      break;
    case 2:



      break;
  }
}

//---------------------------------------
// scan wifi network
void listNetwork(lv_event_t* e) {
  clickSound();
  Serial.println(F("\r\nAP Scan Start..."));
  WiFi.disconnect();
  byte networks = 0;
  networks = WiFi.scanNetworks();
  Serial.print(networks);
  Serial.println(F(" networks found"));

  String options_buffer = "";
  for (byte i = 0; i < networks; ++i) {
    options_buffer += WiFi.SSID(i);                // add SSID
    if (i < networks - 1) options_buffer += "\n";  // add newline except last
  }
  //load ssid list to roller
  lv_roller_set_options(ui_setup_roller_ssid, options_buffer.c_str(), LV_ROLLER_MODE_NORMAL);
  String txt = "Found " + String(networks) + " networks";
  lv_label_set_text(ui_setup_label_networkcount, txt.c_str());
}

// keyboard click OK then hide
void readKeyboard(lv_event_t* e) {
  lv_obj_t* kb = lv_event_get_target(e);
  // Which button was pressed
  uint16_t btn_id = lv_btnmatrix_get_selected_btn(kb);
  const char* txt = lv_btnmatrix_get_btn_text(kb, btn_id);
  if (txt == NULL) return;
  if (strcmp(txt, LV_SYMBOL_OK) == 0) {
    // ✅ OK pressed → hide keyboard
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    beepbeep();
  } else {
    clickSound();
  }
}

// wifi tab save button
void saveWiFiSetup(lv_event_t* e) {
  beep();
  char buf[32];
  lv_roller_get_selected_str(ui_setup_roller_ssid, buf, sizeof(buf));
  lv_label_set_text(ui_setup_label_wifissid, buf);
  WIFI_SSID = String(buf);
  WIFI_PASS = lv_textarea_get_text(ui_setup_textarea_password);
  pref.begin("config", false);  //load configuration first
  pref.putString("ssid", WIFI_SSID);
  pref.putString("wifipass", WIFI_PASS);
  pref.end();
  wifiConnected = true;// force wifi_status function to reconnect the wifi
}

// printer tab save button
void savePrinterSetup(lv_event_t* e) {
  beep();
  MQTT_SERVER_IP = lv_textarea_get_text(ui_setup_textarea_printerip);
  MQTT_SERVER_PASS = lv_textarea_get_text(ui_setup_textarea_accesscode);
  MQTT_SERVER_SERIAL = lv_textarea_get_text(ui_setup_textarea_printerserial);
  TZoffset = lv_textarea_get_text(ui_setup_textarea_timezone);
  pref.begin("config", false);  //load configuration first
  pref.putString("printer_ip", MQTT_SERVER_IP);
  pref.putString("printer_pass", MQTT_SERVER_PASS);
  pref.putString("printer_serial", MQTT_SERVER_SERIAL);
  pref.putString("timezone", TZoffset);
  pref.end();
}
// Photo tab Load button
void rotateScreen(lv_event_t* e) {
  clickSound();
  flip = !flip;//toggle flip 
  if (flip) rotation = rotation + 2;
  else rotation = rotation - 2;
  tft.setRotation(rotation); // Portrait
  lv_obj_invalidate(lv_scr_act());
  pref.begin("config", false);  //load configuration first
  pref.putBool("flip", flip);
  pref.end();
}
