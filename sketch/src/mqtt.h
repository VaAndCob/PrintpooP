#ifndef MQTT_H
#define MQTT_H


#define MQTT_MAX_PACKET_SIZE 8192 // if too low, packet will lost and didn't receive

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>


#include "freertos/timers.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "ui.h"

//necessary variable to use global
extern Preferences pref;

extern bool long_idle;
extern bool idle_swing_now;

extern bool long_print;
extern long print_timer;

extern const String version;
extern int8_t pageIndex;
extern long clockTimer;

extern const char* AP_NAME;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String MQTT_SERVER_IP;
extern String MQTT_SERVER_PASS;
extern String MQTT_SERVER_SERIAL;

extern bool wifiConnected;
extern bool wm_configmode;
extern String pubsubtopic;
extern String pushtopic; 

struct TrayInfo {
  String  type;
  String  color; // Hex color string
  // Add other fields like 'remain', 'nozzle_temp_max', etc., if you use this struct
};


typedef struct BambuMQTTPayload {
//  char print_type[32];
//  int home_flag;
//  int hw_switch_state;
//  int mc_left_time;
  int mc_percent;
  int mc_remaining_time;
// int mc_print_sub_stage;
  int stg[15];
  int stg_cur;
  int mc_print_stage;
  int mc_print_error_code;
  int mc_print_line_number;
  int print_error;
//  char printer_type[32];
//  char subtask_name[32];
  int layer_num;
  int total_layer_num;
  int print_status;
//  int queue_number;
//  int gcode_file_prepare_percent;
//  char obj_subtask_id[32];
//  char project_id_[32];
//  char profile_id_[32];
//  char subtask_id_[32];
//  char gcode_file[128];
  String gcode_state;
//  int plate_index;
//  char task_id[32];
  float bed_temper;
  int bed_target_temper;
//  int frame_temp;
  float nozzle_temper;
  int nozzle_target_temper;
  float chamber_temper;
//  int wifi_signal;
  int heatbreak_fan_speed;
  int cooling_fan_speed;
//  int big_fan1_speed;
//  int big_fan2_speed;
//  int printing_speed_lvl;
//  int printing_speed_mag;
//  bool chamberLed;
//  float nozzle_diameter;
//  bool camera_recording_when_printing;
//  bool camera_timelapse;
//  bool has_ipcam;
  TrayInfo amsTrays[4]; 
  long int ams_exist_bits;
  bool ams;
//  int ams_status_sub;
//  int ams_status_main;
//  long int ams_version;
//  bool ams_support_use_ams;
//  int ams_rfid_status;
//  int ams_humidity;
 // int ams_user_setting_hold_count;
 // bool ams_insert_flag;
 // bool ams_power_on_flag;
 // bool ams_calibrate_remain_flag;
 // bool ams_support_virtual_tray;
 // bool is_ams_need_update;
 // long tray_exist_bits;
 // long tray_is_bbl_bits;
 // long tray_read_done_bits;
 // long tray_reading_bits;
  int ams_tray_id;   // local tray id : "0" ~ "3"
  int ams_tray_now;  // tray_now : "0" ~ "15" or "254", "255"
  //int ams_tray_tar;  // tray_tar : "0" ~ "15" or "255"
  String vt_tray_type;//external spool
  String vt_tray_color;
  
} bambuMqtt;

extern bambuMqtt printerStatus;

struct StageMapEntry {
  int id;
  const char* name;
};

void idle_animation();
void page_control(uint8_t page);
void update_clock();
void nonBlockDelaySec(const unsigned timer);
void mqtt_init();
void wifi_status();
void mqtt_handler();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void wifi_init();



#endif