//main logic

#include "app_config.h"
#include "Print.h"
#include "mqtt.h"
#include "accessory.h"
#include "song.h"
#include "nes_audio.h"



//#define SHOW_MQTT_MSG

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

int8_t pageIndex = 0;
long clockTimer = 0;
//flag
bool print_finish_flag = false;
bool printing_flag = false;
bool ams_flag = false;
// long print, long idle
bool long_idle = false;       //idle longer than 1 min
static long idle_timer = 0;   //idle timer counter
bool idle_swing_now = false;  // swing animation flag

bool long_print = false;
long print_timer = 0;

Preferences pref;

const char* AP_NAME = "PrintpooP_Setup";
bool wifiConnected = false;  // wifi connection status flag (use internally)
bool wm_configmode = false;

// MQTT
WiFiClientSecure printpoop_wiFiClientSecure;
PubSubClient mqttClient(printpoop_wiFiClientSecure);

String WIFI_SSID = "";
String WIFI_PASS = "";
String MQTT_SERVER_IP = "";
String MQTT_SERVER_PASS = "";
String MQTT_SERVER_SERIAL = "";

static const char* MQTT_USER = "bblp";
static const char* client_id = "PrintpooP";
static const uint16_t MQTT_PORT = 8883;
String pubsubtopic = "";  //"device/MQTT_PRINTER_SERIAL/report";
String pushtopic = "";    //"device/MQTT_PRINTER_SERIAL/request";
bambuMqtt printerStatus;

// state of disconnected
int mqtt_state = 0;
const StageMapEntry DISCONNECT_STATE_ID[] = {
  { -4, "mqtt server didn't respond within the keepalive time" },
  { -3, "mqtt network connection was broken" },
  { -2, "mqtt network connection failed" },
  { -1, "mqtt client is disconnected cleanly" },
  { 0, "mqtt client is connected" },
  { 1, "mqtt server doesn't support the requested version of MQTT" },
  { 2, "mqtt server rejected the client identifier" },
  { 3, "mqtt server was unable to accept the connectionv" },
  { 4, "mqtt username/password were rejected" },
  { 5, "mqtt client was not authorized to connect" }
};

// Stage map entries with human-readable printer states
const StageMapEntry CURRENT_STAGE_IDS[] = {
  { -1, "Idle" },  // Often used as a default or initial state
  { 0, "Printing" },
  { 1, "Auto Bed Leveling" },
  { 2, "Heatbed Preheating" },
  { 3, "Homing" },
  { 4, "Changing Filament" },
  { 5, "Paused" },
  { 6, "Filament Runout" },
  { 7, "Heating Nozzle" },
  { 8, "Calibrating Extrusion" },
  { 9, "Scanning Bed Surface" },
  { 10, "Inspecting First Layer" },
  { 11, "Identifying Build Plate Type" },
  { 12, "Calibrating Micro Lidar" },
  { 13, "Homing Toolhead" },
  { 14, "Cleaning Nozzle Tip" },
  { 15, "Checking Extruder Temperature" },
  { 16, "User Paused" },
  { 17, "Toolhead Cover Falling Off" },
  { 18, "Calibrating Micro Lidar" },
  { 19, "Calibrating Extruder Flow" },
  { 20, "Nozzle Temperature Malfunction" },
  { 21, "Heatbed Temperature Malfunction" },
  { 22, "Chamber Temperature Malfunction" },
  { 23, "First Layer Inspection" },
  { 24, "Filament Loading" },
  { 25, "Bed Leveling" },
  { 26, "Motor Noise Calibration" },
  { 27, "Vibration Calibration" },
  { 28, "Max Stage" },
  { 255, "Print job finished" }
};


//--------- Functions ---------------------
//display printpoop swing animation during idle stage longer than 1 min and stop when entering another stage
void idle_animation() {
  //Idle printpoop swing animation
  if (strcmp(lv_label_get_text(ui_status_label_printstage), "Idle") == 0) {
    if (!long_idle) {
      long_idle = true;
      idle_timer = millis();  //reset timer
    } else {
      if (!idle_swing_now && (millis() - idle_timer > 60000)) {  //time out
        idle_swing_now = true;
        lv_obj_add_flag(ui_status_image_ppimage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_status_image_swing, LV_OBJ_FLAG_HIDDEN);
        swing_Animation(ui_status_image_swing, 0);  //swing animation
        Serial.println("Swing");
      
      }
    }
  } else {            //not idle
    if (long_idle) {  //stop swing animation
      long_idle = false;
      idle_swing_now = false;
      lv_obj_add_flag(ui_status_image_swing, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_status_image_ppimage, LV_OBJ_FLAG_HIDDEN);
      lv_anim_del(ui_status_image_swing, NULL);
      
    }
  }
}
//-----------------------------
void page_control(uint8_t page) {
  switch (page) {
    case 0:
      lv_obj_add_flag(ui_status_panel_page3, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_status_panel_page0, LV_OBJ_FLAG_HIDDEN);
      break;
    case 1:
      lv_obj_add_flag(ui_status_panel_page0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page3, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_status_panel_page1, LV_OBJ_FLAG_HIDDEN);
      break;
    case 2:
      lv_obj_add_flag(ui_status_panel_page0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page3, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_status_panel_page2, LV_OBJ_FLAG_HIDDEN);
      break;
    case 3:
      lv_obj_add_flag(ui_status_panel_page0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_status_panel_page2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_status_panel_page3, LV_OBJ_FLAG_HIDDEN);
      break;
  }
}
//-----------
//update clock
void update_clock() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - clockTimer > 1000) {
    String text = get_current_time();
    lv_label_set_text(ui_status_label_clock, text.c_str());
    clockTimer = millis();
  }
}
//-----------
//minute to time
String formatMinutesToTime(int totalMinutes) {
  int hours = totalMinutes / 60;
  int minutes = totalMinutes % 60;

  char timeStr[6];  // "HH:MM" + null terminator
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hours, minutes);
  return String(timeStr);
}
//-----------
// Map temperature (0–300°C) to a 24-bit RGB color (0xRRGGBB)
uint32_t temperatureToColor(int temperatureC) {
  if (temperatureC < 25) temperatureC = 25;
  if (temperatureC > 300) temperatureC = 300;

  float hue;
  if (temperatureC <= 100) {
    hue = 240.0f - (temperatureC - 25.0f) * (60.0f / 75.0f);  // Blue to Cyan
  } else if (temperatureC <= 180) {
    hue = 180.0f - (temperatureC - 100.0f) * (60.0f / 80.0f);  // Cyan to Green
  } else if (temperatureC <= 250) {
    hue = 120.0f - (temperatureC - 180.0f) * (60.0f / 70.0f);  // Green to Yellow
  } else if (temperatureC <= 270) {
    hue = 60.0f - (temperatureC - 250.0f) * (60.0f / 20.0f);  // Yellow to Red
  } else {
    hue = 360.0f - (temperatureC - 270.0f) * (60.0f / 30.0f);  // Red to Violet
    if (hue < 0) hue += 360.0f;
  }

  float s = 1.0f, v = 1.0f;
  float h = hue / 60.0f;
  int i = (int)h;
  float f = h - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  float r, g, b;
  switch (i % 6) {
    case 0:
      r = v;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = v;
      b = p;
      break;
    case 2:
      r = p;
      g = v;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = v;
      break;
    case 4:
      r = t;
      g = p;
      b = v;
      break;
    case 5:
      r = v;
      g = p;
      b = q;
      break;
    default: r = g = b = 0; break;
  }

  return ((uint32_t)(r * 255) << 16) | ((uint32_t)(g * 255) << 8) | (uint32_t)(b * 255);
}

//-----------
// change printpoop image by stage
static int cur_stage_id = -1;  // idle
void changeStageImageByStage(int stgId) {
  const lv_img_dsc_t* image_to_set = NULL;  //set image sources
 switch (stgId) {
  #if THEME == KITTEN
    case 0:   image_to_set = &ui_img_kitten_stage0_png;   break;  // printing
    case 1:   image_to_set = &ui_img_kitten_stage1_png;   break;  // auto bed leveling
    case 2:   image_to_set = &ui_img_kitten_stage2_png;   break;  // printbed preheat
    case 4:   image_to_set = &ui_img_kitten_stage4_png;   break;  // changing filament
    case 5:   image_to_set = &ui_img_kitten_stage5_png;   break;  // paused
    case 8:   image_to_set = &ui_img_kitten_stage8_png;   break;  // calibrating extrusion
    case 13:  image_to_set = &ui_img_kitten_stage13_png;  break;  // homing toolhead
    case 14:  image_to_set = &ui_img_kitten_stage14_png;  break;  // cleaning nozzle tip
    case 24:  image_to_set = &ui_img_kitten_stage24_png;  break;  // filament loading
    case 255: image_to_set = &ui_img_kitten_stage255_png; break;  // print job finish
    default:  image_to_set = &ui_img_kitten_stage_1_png;  break;  // idle -1
  #elif THEME == PUPPY
    case 0:   image_to_set = &ui_img_puppy_stage0_png;   break;  // printing
    case 1:   image_to_set = &ui_img_puppy_stage1_png;   break;  // auto bed leveling
    case 2:   image_to_set = &ui_img_puppy_stage2_png;   break;  // printbed preheat
    case 4:   image_to_set = &ui_img_puppy_stage4_png;   break;  // changing filament
    case 5:   image_to_set = &ui_img_puppy_stage5_png;   break;  // paused
    case 8:   image_to_set = &ui_img_puppy_stage8_png;   break;  // calibrating extrusion
    case 13:  image_to_set = &ui_img_puppy_stage13_png;  break;  // homing toolhead
    case 14:  image_to_set = &ui_img_puppy_stage14_png;  break;  // cleaning nozzle tip
    case 24:  image_to_set = &ui_img_puppy_stage24_png;  break;  // filament loading
    case 255: image_to_set = &ui_img_puppy_stage255_png; break;  // print job finish
    default:  image_to_set = &ui_img_puppy_stage_1_png;  break;  // idle -1
  #endif
}     
  if (image_to_set != NULL) {
    lv_img_set_src(ui_status_image_ppimage, image_to_set);
  }
}

//-----------
// get status name
static const char* getStageName(int stageId) {
  for (auto& entry : CURRENT_STAGE_IDS) {
    if (entry.id == stageId) return entry.name;
  }
  return "unknown";  // Default fallback
}

//---------------------------------
//Macro parsing mqtt json object to internal variable
#define SET_IF_EXISTS(type, key, dest) \
  if (!incomingJson["print"][#key].isNull()) printerStatus.dest = incomingJson["print"][#key].as<type>();

#define SET_IF_EXISTS_2(type, key1, key2, dest) \
  if (!incomingJson["print"][#key1].isNull()) printerStatus.dest = incomingJson["print"][#key1][#key2].as<type>();
//---------------------------------

// Parse mqtt message and update display
static void update_print_status(JsonDocument& incomingJson, uint8_t page) {

  if (incomingJson != NULL && incomingJson["print"].is<JsonObject>()) {  // if non print key , skip all
                                                                         // Show full print object
#ifdef SHOW_MQTT_MSG
    String printString;
    serializeJsonPretty(incomingJson["print"], printString);
    Serial.println(printString);
#endif

    //1. ------------- Print Status --------------------
    String currentPrintStatus = "";                        // For UI header like "Idle", "Printing", "Paused"                         // For specific details like "Bed Leveling", "95%"
    if (!incomingJson["print"]["gcode_state"].isNull()) {  //Prioritizes gcode_state for overall machine status ("RUNNING", "PAUSED", "IDLE", "FINISH").
      SET_IF_EXISTS(String, gcode_state, gcode_state);
      if (printerStatus.gcode_state == "IDLE") {
        currentPrintStatus = "Idle";
        cur_stage_id = -1;
      } else if (printerStatus.gcode_state == "PAUSE") {
        SET_IF_EXISTS(int, print_error, print_error);
        if (printerStatus.print_error == 0) {
          currentPrintStatus = "Printing paused";
          cur_stage_id = -1;
        } else {
          currentPrintStatus = "Error paused";
          cur_stage_id = 5;
        }
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_x);  //stop print head animation
      } else if (printerStatus.gcode_state == "FAILED") {
        currentPrintStatus = "Failed";
        cur_stage_id = 5;
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_x);  //stop print head animation
      } else if (printerStatus.gcode_state == "FINISH") {
        currentPrintStatus = "Print Job Completed";
        cur_stage_id = 255;
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_x);  //stop print head animation
      } else if (printerStatus.gcode_state == "PREPARE") {
        currentPrintStatus = "Preparing";
        cur_stage_id = 8;
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_x);  //stop print head animation
      } else if (printerStatus.gcode_state == "RUNNING") {               //If gcode_state is "RUNNING", it uses stg_cur with CURRENT_STAGE_IDS_MAP to determine the specific operation.
        if (!incomingJson["print"]["stg_cur"].isNull()) {
          SET_IF_EXISTS(int, stg_cur, stg_cur);
          currentPrintStatus = getStageName(printerStatus.stg_cur);
          cur_stage_id = printerStatus.stg_cur;
        } else {
          cur_stage_id = 0;                              //printing
          sway_Animation(ui_status_image_printhead, 0);  //start print head animation
        }
      } else {
        currentPrintStatus = "Unknown";
        cur_stage_id = -1;
      }
      Serial.println(currentPrintStatus);
      lv_label_set_text(ui_status_label_printstage, currentPrintStatus.c_str());
      changeStageImageByStage(cur_stage_id);

    } else {  // If gcode_state is not present in the message, it falls back to checking stg_cur directly, then mc_print_stage.

      if (!incomingJson["print"]["stg_cur"].isNull()) {
        SET_IF_EXISTS(int, stg_cur, stg_cur);
        currentPrintStatus = getStageName(printerStatus.stg_cur);
        cur_stage_id = printerStatus.stg_cur;
        Serial.println(currentPrintStatus);
        lv_label_set_text(ui_status_label_printstage, currentPrintStatus.c_str());
        changeStageImageByStage(printerStatus.stg_cur);
      }
    }  // else gcode_state not present

    //2. ------------ Printing parameter ---------------------------
    SET_IF_EXISTS(float, nozzle_temper, nozzle_temper);
    SET_IF_EXISTS(int, nozzle_target_temper, nozzle_target_temper);
    SET_IF_EXISTS(float, bed_temper, bed_temper);
    SET_IF_EXISTS(int, bed_target_temper, bed_target_temper);
    SET_IF_EXISTS(float, chamber_temper, chamber_temper);
    SET_IF_EXISTS(int, heatbreak_fan_speed, heatbreak_fan_speed);
    SET_IF_EXISTS(int, cooling_fan_speed, cooling_fan_speed);
    SET_IF_EXISTS(int, mc_percent, mc_percent);
    SET_IF_EXISTS(int, mc_remaining_time, mc_remaining_time);
    SET_IF_EXISTS(int, layer_num, layer_num);
    SET_IF_EXISTS(int, total_layer_num, total_layer_num);
    //update display
    String text = "";
    uint32_t tempColour = temperatureToColor(printerStatus.nozzle_temper);
    //border color
    lv_obj_set_style_border_color(ui_screen_status, lv_color_hex(tempColour), LV_PART_MAIN | LV_STATE_DEFAULT);
    // Page 0  printpoop page
    lv_bar_set_value(ui_status_bar_mcpercent2, printerStatus.mc_percent, LV_ANIM_ON);
    // Page 1  temperature
    lv_obj_set_style_text_color(ui_status_label_nozzletemp, lv_color_hex(tempColour), LV_PART_MAIN | LV_STATE_DEFAULT);
    text = String((int)round(printerStatus.nozzle_temper)) + "/" + String(printerStatus.nozzle_target_temper);
    lv_label_set_text(ui_status_label_nozzletemp, text.c_str());
    text = String((int)round(printerStatus.bed_temper)) + "/" + String(printerStatus.bed_target_temper);
    lv_label_set_text(ui_status_label_bedtemp, text.c_str());
    text = String((int)round(printerStatus.chamber_temper));
    lv_label_set_text(ui_status_label_chambertemp, text.c_str());
    lv_bar_set_value(ui_status_bar_heatbreakfan, printerStatus.heatbreak_fan_speed, LV_ANIM_ON);
    lv_bar_set_value(ui_status_bar_coolingfan, printerStatus.cooling_fan_speed, LV_ANIM_ON);
    lv_bar_set_value(ui_status_bar_mcpercent, printerStatus.mc_percent, LV_ANIM_ON);
    // Page 2 progress
    lv_arc_set_value(ui_status_arc_mcpercentarc, printerStatus.mc_percent);
    text = String(printerStatus.layer_num);
    lv_label_set_text(ui_status_label_layernum, text.c_str());
    text = String(printerStatus.total_layer_num);
    lv_label_set_text(ui_status_label_layertotal, text.c_str());
    text = formatMinutesToTime(printerStatus.mc_remaining_time);
    lv_label_set_text(ui_status_label_mcremaintime, text.c_str());


    // Print head animation
    if (!printing_flag && printerStatus.gcode_state == "RUNNING") {
      sway_Animation(ui_status_image_printhead, 0);  //start print head animation
      printing_flag = true;
    }
    if (printerStatus.gcode_state != "RUNNING") {
      lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_x);  //stop print head animation
      printing_flag = false;
    }

    // play finish music
    if (!print_finish_flag && printerStatus.gcode_state == "FINISH" && printerStatus.mc_percent == 100) {
      print_finish_flag = true;
      //startMusicPlayback(finish, 6); // non block run in Core0
      player.play_nes(finish, false, GAIN);  // blocked run normal Core1
    }
    if (print_finish_flag && printerStatus.mc_percent == 0) print_finish_flag = false;  //reset flag

    //3. ------------ AMS -------------------------------
    if (!incomingJson["print"]["ams"].isNull()) {  // check if mqtt message available
                                                   // update current slot
      if (!incomingJson["print"]["ams"]["tray_now"].isNull()) {
        //stop all SLOT label blinking
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
        lv_obj_set_style_opa(ui_status_label_filament1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(ui_status_label_filament2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(ui_status_label_filament3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(ui_status_label_filament4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        printerStatus.ams_tray_now = incomingJson["print"]["ams"]["tray_now"].as<int>();
        switch (printerStatus.ams_tray_now) {
          case 0:
            slowblink_Animation(ui_status_label_filament1, 0);
            break;
          case 1:
            slowblink_Animation(ui_status_label_filament2, 0);
            break;
          case 2:
            slowblink_Animation(ui_status_label_filament3, 0);
            break;
          case 3:
            slowblink_Animation(ui_status_label_filament4, 0);
            break;
          default:  // ams_tray_now = 255
            break;
        }
      }
      //update AMS information
      SET_IF_EXISTS_2(int, ams, tray_now, ams_tray_now);               // get current slot
      SET_IF_EXISTS_2(long int, ams, ams_exist_bits, ams_exist_bits);  // check if AMS attached
      //Serial.printf("tray_now %d , ams_exist_bits %d\n", printerStatus.ams_tray_now, printerStatus.ams_exist_bits);
      JsonArray amsUnits = incomingJson["print"]["ams"]["ams"];  // This is the array of AMS units (usually just one)
      for (JsonObject amsUnit : amsUnits) {                      // Loop through each AMS unit
        JsonArray trays = amsUnit["tray"];                       // Get the 'tray' array for the current AMS unit
        if (trays) {                                             // Modern ArduinoJson v6+ way to check for null/invalid/empty array
          for (JsonObject tray : trays) {                        // Loop through each individual tray reported
            JsonVariant trayIdVariant = tray["id"];
            if (!trayIdVariant) {  // Skip if 'id' is missing for some reason
              continue;
            }
            int trayId = trayIdVariant.as<int>();               // Convert "id" (e.g., "0") to integer 0
            if (trayId >= 0 && trayId < 4) {                    // Ensure the trayId is a valid index for your storage array (0-3 for 4 slots)
              JsonVariant trayTypeVariant = tray["tray_type"];  // Get JsonVariant for "tray_type"
              if (trayTypeVariant) {                            // Check if "tray_type" key exists and is not null
                // This slot contains filament information
                printerStatus.amsTrays[trayId].type = trayTypeVariant.as<String>();      // check filament type
                printerStatus.amsTrays[trayId].color = tray["tray_color"].as<String>();  // color bytes RRGGBBAA
                String colorStr = printerStatus.amsTrays[trayId].color;
                uint32_t color = strtoul(colorStr.c_str(), NULL, 16);
                uint8_t r = (color >> 24) & 0xFF;
                uint8_t g = (color >> 16) & 0xFF;
                uint8_t b = (color >> 8) & 0xFF;
                uint8_t hexAlpha = color & 0xFF;
                lv_color_t hexColor = lv_color_make(r, g, b);

                switch (trayId) {  // set colour and label in AMS picture color & alpha send from printer
                  case 0:
                    lv_obj_set_style_bg_color(ui_status_button_slot1, hexColor, LV_PART_MAIN);
                    lv_obj_set_style_bg_opa(ui_status_button_slot1, hexAlpha, LV_PART_MAIN);
                    lv_label_set_text(ui_status_label_filament1, printerStatus.amsTrays[0].type.c_str());
                    if (printerStatus.ams_tray_now == 0) {
                      lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
                      lv_obj_set_style_opa(ui_status_label_filament2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      slowblink_Animation(ui_status_label_filament1, 0);
                    }
                    break;
                  case 1:
                    lv_obj_set_style_bg_color(ui_status_button_slot2, hexColor, LV_PART_MAIN);
                    lv_obj_set_style_bg_opa(ui_status_button_slot2, hexAlpha, LV_PART_MAIN);
                    lv_label_set_text(ui_status_label_filament2, printerStatus.amsTrays[1].type.c_str());
                    if (printerStatus.ams_tray_now == 1) {
                      lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
                      lv_obj_set_style_opa(ui_status_label_filament1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      slowblink_Animation(ui_status_label_filament2, 0);
                    }
                    break;
                  case 2:
                    lv_obj_set_style_bg_color(ui_status_button_slot3, hexColor, LV_PART_MAIN);
                    lv_obj_set_style_bg_opa(ui_status_button_slot3, hexAlpha, LV_PART_MAIN);
                    lv_label_set_text(ui_status_label_filament3, printerStatus.amsTrays[2].type.c_str());
                    if (printerStatus.ams_tray_now == 2) {
                      lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
                      lv_obj_set_style_opa(ui_status_label_filament1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      slowblink_Animation(ui_status_label_filament3, 0);
                    }
                    break;
                  case 3:
                    lv_obj_set_style_bg_color(ui_status_button_slot4, hexColor, LV_PART_MAIN);
                    lv_obj_set_style_bg_opa(ui_status_button_slot4, hexAlpha, LV_PART_MAIN);
                    lv_label_set_text(ui_status_label_filament4, printerStatus.amsTrays[3].type.c_str());
                    if (printerStatus.ams_tray_now == 3) {
                      lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
                      lv_obj_set_style_opa(ui_status_label_filament1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      lv_obj_set_style_opa(ui_status_label_filament3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                      slowblink_Animation(ui_status_label_filament4, 0);
                    }
                    break;
                }  // switch

              } else {  // no filament install

                switch (trayId) {  // show checker picture instead of color
                  case 0:
                    lv_label_set_text(ui_status_label_filament1, "");
                    lv_obj_set_style_bg_opa(ui_status_button_slot1, LV_OPA_TRANSP, LV_PART_MAIN);
                    break;
                  case 1:
                    lv_label_set_text(ui_status_label_filament2, "");
                    lv_obj_set_style_bg_opa(ui_status_button_slot2, LV_OPA_TRANSP, LV_PART_MAIN);
                    break;
                  case 2:
                    lv_label_set_text(ui_status_label_filament3, "");
                    lv_obj_set_style_bg_opa(ui_status_button_slot3, LV_OPA_TRANSP, LV_PART_MAIN);
                    break;
                  case 3:
                    lv_label_set_text(ui_status_label_filament4, "");
                    lv_obj_set_style_bg_opa(ui_status_button_slot4, LV_OPA_TRANSP, LV_PART_MAIN);
                    break;
                }  // switch
              }    //if traytypevariant
            }      // End of trayId validation
          }        // End of forEach tray
        }          // End of if trays
      }            // End of forEach amsUnit
      //AMS icon
      if (!ams_flag && printerStatus.ams_exist_bits == 1) {  // show
        lv_obj_clear_flag(ui_status_image_ams, LV_OBJ_FLAG_HIDDEN);
        ams_flag = true;
      }
      if (ams_flag && printerStatus.ams_exist_bits == 0) {  // hide
        lv_obj_add_flag(ui_status_image_ams, LV_OBJ_FLAG_HIDDEN);
        ams_flag = false;
      }
    }  //if ams
    //4. --------  External Spool v_tray ----------------------------------
    if (!incomingJson["print"]["vt_tray"].isNull()) {  // if there is vt_tray key data -> update external spool

      if (!incomingJson["print"]["vt_tray"]["tray_color"].isNull()) {  // color
        printerStatus.vt_tray_color = incomingJson["print"]["vt_tray"]["tray_color"].as<String>();
        String colorStr = printerStatus.vt_tray_color;
        uint32_t color = strtoul(colorStr.c_str(), NULL, 16);
        uint8_t r = (color >> 24) & 0xFF;
        uint8_t g = (color >> 16) & 0xFF;
        uint8_t b = (color >> 8) & 0xFF;
        uint8_t hexAlpha = color & 0xFF;
        lv_color_t hexColor = lv_color_make(r, g, b);
        lv_obj_set_style_bg_color(ui_status_button_slot0, hexColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_status_button_slot0, hexAlpha, LV_PART_MAIN);
      }
      if (!incomingJson["print"]["vt_tray"]["tray_type"].isNull()) {  // type
        printerStatus.vt_tray_type = incomingJson["print"]["vt_tray"]["tray_type"].as<String>();
        lv_label_set_text(ui_status_label_filament0, printerStatus.vt_tray_type.c_str());
      }
      if (printerStatus.ams_tray_now == 254) slowblink_Animation(ui_status_label_filament0, 0);  // external pool blink

    } else if (incomingJson["print"]["ams_id"] == 255) {  // change only type or color

      if (!incomingJson["print"]["tray_color"].isNull()) {
        printerStatus.vt_tray_color = incomingJson["print"]["tray_color"].as<String>();
        String colorStr = printerStatus.vt_tray_color;
        uint32_t color = strtoul(colorStr.c_str(), NULL, 16);
        uint8_t r = (color >> 24) & 0xFF;
        uint8_t g = (color >> 16) & 0xFF;
        uint8_t b = (color >> 8) & 0xFF;
        uint8_t hexAlpha = color & 0xFF;
        lv_color_t hexColor = lv_color_make(r, g, b);
        lv_obj_set_style_bg_color(ui_status_button_slot0, hexColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_status_button_slot0, hexAlpha, LV_PART_MAIN);
      }
      if (!incomingJson["print"]["tray_type"].isNull()) {
        printerStatus.vt_tray_type = incomingJson["print"]["tray_type"].as<String>();
        lv_label_set_text(ui_status_label_filament0, printerStatus.vt_tray_type.c_str());
      }
    }  //else if
  }    // if "printer" not null
}  //Update_display
//------------------------------------------------------------------------------------------------------
// get the disconnect reason
static const char* getDisconnectState(int stageId) {
  for (auto& entry : DISCONNECT_STATE_ID) {
    if (entry.id == stageId) return entry.name;
  }
  return "unknown";  // Default fallback
}
//-----------
// helper function to delay without blocking
void nonBlockDelaySec(const unsigned timer) {
  const unsigned long counterTimer = millis();
  while (millis() - counterTimer < timer * 1000) {
    lv_timer_handler();
    delay(5);
    update_clock();
  }
}

//================== MQTT =========================

// initialize mqtt
void mqtt_init() {
  printpoop_wiFiClientSecure.flush();
  printpoop_wiFiClientSecure.stop();
  printpoop_wiFiClientSecure.setInsecure();

  mqttClient.setServer(MQTT_SERVER_IP.c_str(), MQTT_PORT);
  mqttClient.setBufferSize(MQTT_MAX_PACKET_SIZE);

  mqttClient.setCallback(mqttCallback);
  mqttClient.setSocketTimeout(5);  //Prevents connection or read operations from hanging too long
  mqttClient.setKeepAlive(15);     //Sends periodic PINGREQ to keep session alive

  pubsubtopic = "device/" + MQTT_SERVER_SERIAL + "/report";
  pushtopic = "device/" + MQTT_SERVER_SERIAL + "/request";
}
//-----------
// mqtt publish message
static void mqttPushMessage(JsonDocument doc) {
  if (mqttClient.beginPublish(pushtopic.c_str(), measureJson(doc), false)) {
    serializeJson(doc, mqttClient);
    mqttClient.endPublish();
    Serial.println("Message published");
  } else {
    Serial.println("Fail publish message");
  }
}
//-----------
// request printer status
static void request_printer_status() {
  /*{
  "pushing": {
    "sequence_id": "0",
    "command": "pushall",
    "version": 1,
    "push_target": 1
  }
}*/
  JsonDocument doc;
  JsonObject pushing = doc["pushing"].to<JsonObject>();
  pushing["sequence_id"] = "0";
  pushing["command"] = "pushall";
  pushing["version"] = 1;
  pushing["push_target"] = 1;
  mqttPushMessage(doc);  // Publish JSON object
  doc.clear();           // Optional cleanup
}
//-----------
// mqtt reconnect helper function
static boolean reconnect() {
  Serial.println("Attemp to connect MQTT server (TLS)");
  if (mqttClient.connect(client_id, MQTT_USER, MQTT_SERVER_PASS.c_str())) {
    mqttClient.subscribe(pubsubtopic.c_str());
    Serial.println("✅ MQTT CONNECTED (TLS)");
  } else {
    Serial.print("❌ MQTT CONNECT failed, state: ");
    Serial.println(mqttClient.state());
  }
  return mqttClient.connected();
}


// handle mqtt
void mqtt_handler() {
  if (!wm_configmode) {
    if (!mqttClient.connected() && wifiConnected) {  //mqtt not connect

      //show connecting to printer status
      Serial.print("Connecting to Printer: ");
      lv_label_set_text(ui_status_label_printstage, "Connecting to Printer...");
      slowblink_Animation(ui_status_image_printer, 0);
      nonBlockDelaySec(1);

      // Attempt to reconnect
      if (reconnect()) {  // connected
        beepbeep();
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
        lv_obj_set_style_opa(ui_status_image_printer, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(ui_status_button_networksetup, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_status_label_printstage, "Gathering printer status...");

        request_printer_status();  //request printer status
      } else {                     // disconnected
        lv_label_set_text(ui_status_label_printstage, "Click SETUP button to setup the network.");
        mqtt_state = mqttClient.state();
        String disconnect_state = String(getDisconnectState(mqtt_state));
        Serial.println(disconnect_state);
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_x);  //stop swaying
      }
      nonBlockDelaySec(15);  //give time to click SETUP button

    } else {              // mqtt still connected
      mqttClient.loop();  // handle mqtt event
    }
  }
}


// mqtt client call back function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (length >= MQTT_MAX_PACKET_SIZE) {
    Serial.println(F("Payload too large, skipping"));
    return;  // Avoid overflow
  }

  // Serial.printf("Payload Length: %d\n", length);
  JsonDocument doc;
  JsonDocument amsFilter;
  amsFilter["print"]["*"] = true;
  auto deserializeError = deserializeJson(doc, payload, length, DeserializationOption::Filter(amsFilter));

  if (deserializeError) {
    Serial.print(F("JSON parse failed: "));
    return;
  }

#ifdef SERIAL_DEBUG
  serializeJsonPretty(doc, Serial);
  Serial.println();
#endif

  update_print_status(doc, pageIndex);
}


//========================= WIFI =========================
// init wifi manager
void wifi_init() {

  slowblink_Animation(ui_status_label_wifisymbol, 0);
  lv_label_set_text(ui_status_label_printstage, "Connecting to WiFi...");
  nonBlockDelaySec(1);

  pref.begin("config", true);  //load configuration first
  WIFI_SSID = pref.getString("ssid", "");
  WIFI_PASS = pref.getString("wifipass", "");

  MQTT_SERVER_IP = pref.getString("printer_ip", "");
  MQTT_SERVER_PASS = pref.getString("printer_pass", "");
  MQTT_SERVER_SERIAL = pref.getString("printer_serial", "");
  TZoffset = pref.getString("timezone", "");
  pref.end();

  Serial.println("WiFi SSID: " + WIFI_SSID);
  Serial.println("WiFi Pass: " + WIFI_PASS);
  Serial.println("Printer IP: " + MQTT_SERVER_IP);
  Serial.println("Access Code: " + MQTT_SERVER_PASS);
  Serial.println("Serial No: " + MQTT_SERVER_SERIAL);
  Serial.println("Timezone: " + TZoffset);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  uint8_t trial = 0;
  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(F("."));
    delay(500);
    trial++;
    if (trial > 6) {
      wifiConnected = false;
      break;
    } else {
      wifiConnected = true;
    }
  }


  if (wifiConnected) {
    WiFi.setAutoReconnect(true);  //set to auto reconnect when lost wifi
    WiFi.persistent(true);        //keep connect to last ssid
    Serial.println("✅ WiFi connected!");
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
    lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
    lv_obj_set_style_opa(ui_status_label_wifisymbol, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_status_label_printstage, "WiFi Connected");
    //check firmware update
    if (!firmware_checked && newFirmwareAvailable()) {
      lv_label_set_text(ui_status_label_printstage, "Update Available");
      lv_label_set_text(ui_status_label_wifisymbol, LV_SYMBOL_REFRESH);
      nonBlockDelaySec(1);
      player.play_nes(error, false, GAIN);  //warning sound
    }

  } else {
    Serial.println("❌ Failed to connect or timed out");
    slowblink_Animation(ui_status_label_wifisymbol, 0);
    lv_label_set_text(ui_status_label_printstage, "Connecting to WiFi...");
  }
}

// update wifi status display
void wifi_status() {
  //wifi disconnect from connected
  if (WiFi.status() != WL_CONNECTED && wifiConnected) {  // wifi not connect
    wifiConnected = false;
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Connecting to WiFi...");
    slowblink_Animation(ui_status_label_wifisymbol, 0);
    lv_label_set_text(ui_status_label_printstage, "Connecting to WiFi...");

  }
  //wifi connect from disconnected
  if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
    beepbeep();
    wifiConnected = true;
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
    lv_anim_del(NULL, (lv_anim_exec_xcb_t)_ui_anim_callback_set_opacity);
    lv_obj_set_style_opa(ui_status_label_wifisymbol, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_status_label_printstage, "Connected to WiFi");
  }  // wifi connected
}


//-------------------------------------------------------------------------
/*
{
    "nozzle_temper": 125.0625,
    "nozzle_target_temper": 140,
    "mc_print_stage": "2", //2
    "mc_remaining_time": 7,
    "ams_rfid_status": 3,
    "gcode_state": "RUNNING",
    "stg": [
      2,
      13,
      24,
      4,
      8,
      14,
      1
    ],
    "stg_cur": 2, //heatbed preheating
    "home_flag": 863847864,
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1830"
  },
  {
    "nozzle_temper": 126.5,
    "bed_temper": 64.03125,
    "heatbreak_fan_speed": "8",
    "mc_percent": 51,
    "mc_remaining_time": 0,
    "ams_rfid_status": 2,
    "wifi_signal": "-49dBm",
    "stg_cur": 13, // Homing toolhead
    "mc_print_sub_stage": 1, //1
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1831"
  },
  {
    "nozzle_temper": 140.0625,
    "nozzle_target_temper": 220,
    "heatbreak_fan_speed": "8",
    "mc_percent": 1,
    "mc_remaining_time": 7,
    "wifi_signal": "-55dBm",
    "stg_cur": 24, // Filament loading
    "mc_print_sub_stage": 0, //0
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1850"
  },
    {
    "nozzle_temper": 141.09375,
    "bed_temper": 64.40625,
    "ams_status": 258,
    "wifi_signal": "-53dBm",
    "stg_cur": 4,// Chaning filament
    "mc_print_sub_stage": 2, // 2
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1851"
  },
    {
    "nozzle_temper": 243.0,
    "stg_cur": 8, // Calibrating extrusion
    "mc_print_sub_stage": 2, //2
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1896"
  },
    {
    "nozzle_temper": 218.71875,
    "cooling_fan_speed": "15",
    "big_fan1_speed": "15",
    "big_fan2_speed": "15",
    "wifi_signal": "-54dBm",
    "stg_cur": 14, // Clearning nozzle tip
    "mc_print_sub_stage": 0, //0
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1947"
  },
    {
    "nozzle_temper": 140.90625,
    "cooling_fan_speed": "6",
    "big_fan1_speed": "6",
    "big_fan2_speed": "6",
    "wifi_signal": "-52dBm",
    "stg_cur": 1, // Auto bed leveling
    "mc_print_sub_stage": 1,// 1
    "command": "push_status",
    "msg": 1,
    "sequence_id": "1996"
  },
    {
    "nozzle_temper": 218.9375,
    "bed_temper": 65.09375,
    "wifi_signal": "-52dBm",
    "stg_cur": 0, //Printing
    "command": "push_status",
    "msg": 1,
    "sequence_id": "2023"
  },
    {
    "nozzle_temper": 220.03125,
    "cooling_fan_speed": "6",
    "big_fan1_speed": "6",
    "big_fan2_speed": "6",
    "mc_percent": 96,
    "wifi_signal": "-54dBm",
    "stg_cur": 255,//finished
    "fan_gear": 87,
    "command": "push_status",
    "msg": 1,
    "sequence_id": "2056"
  },
  {
  "print": {
    "nozzle_temper": 255.03125,
    "bed_temper": 80.0625,
    "heatbreak_fan_speed": "9",
    "ams_status": 263,
    "wifi_signal": "-49dBm",
    "ams": {
      "tray_now": "1",//select slot 1
      "version": 55
    },
    "command": "push_status",
    "msg": 1,
    "sequence_id": "3680"
  }
}
{
  "print": {
    "nozzle_temper": 255.53125,
    "bed_temper": 79.875,
    "heatbreak_fan_speed": "9",
    "ams_status": 261,
    "ams": {
      "tray_now": "255",//no slot selected
      "version": 54
    },
    "command": "push_status",
    "msg": 1,
    "sequence_id": "3673"
  }
}*/
