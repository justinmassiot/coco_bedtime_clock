/*
MIT License
Copyright (c) 2025 Justin MASSIOT

This is a program built for Arduino UNO R4.
Inspired by the "RTC_NTPSync" and "DisplaySingleFrame" examples.

LED matrix pictures and animations can be built at: https://ledmatrix-editor.arduino.cc/
*/

#define DEBUG_COCO 1

#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h> // needs the NTPClient library to be installed
#include "RTC.h"
#include "Arduino_LED_Matrix.h" // LED_Matrix library

//#include "clock_agenda.h"
#include "screen_night.h"
#include "screen_day.h"
#include "wifi_info.h"
char ssid[] = SECRET_SSID; // from "wifi_info.h"
char pass[] = SECRET_PASS; // from "wifi_info.h"

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient NtpClient(Udp);
ArduinoLEDMatrix LedMatrix;
bool flag_update = false;


// attempt to connect to WiFi network
bool connectToWiFi() {
  if (DEBUG_COCO) {
    Serial.print("[WIFI] Attempting to connect to SSID: ");
    Serial.println(ssid);
    Serial.flush();
  }

  // change the below line if using Open (unsecured) or WEP network
  return (WiFi.begin(ssid, pass) == WL_CONNECTED); // true if connected, false otherwise
                                                   // note: this routine is blocking and makes several attempts based on WL_MAX_ATTEMPT_CONNECTION
}

void printWifiStatus() {
  if (DEBUG_COCO) {
    // print the SSID of the network you're attached to:
    Serial.print("[WIFI] SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    Serial.print("[WIFI] IP Address: ");
    Serial.println(WiFi.localIP());

    // print the received signal strength:
    Serial.print("[WIFI] Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
}

// get the current date and time from NTP and push it to the RTC
bool setRtcFromNtp() {
  bool ntp_time_ok = false;
  bool rtc_time_ok = false;

  if (DEBUG_COCO) Serial.println("[NTP]  Getting time from NTP and copying it to RTC...");
  
  NtpClient.begin(); // starts the underlying UDP client with the default local port
  ntp_time_ok = NtpClient.forceUpdate(); // has in internal timeout of 1000ms
  if (ntp_time_ok) {
    auto timezone_offset_hours = 0;
    auto unix_time = NtpClient.getEpochTime() + (timezone_offset_hours * 3600);
    if (DEBUG_COCO) Serial.print("[NTP]  Unix time = ");
    Serial.println(unix_time);

    // TODO: set timezone and DST
    // https://github.com/arduino-libraries/NTPClient/blob/master/NTPClient.h#L92C10-L92C40
    // https://github.com/JChristensen/Timezone/blob/master/examples/HardwareRTC/HardwareRTC.ino

    RTCTime time_to_set = RTCTime(unix_time);
    rtc_time_ok = RTC.setTime(time_to_set);
  }
  NtpClient.end();

  return (ntp_time_ok && rtc_time_ok);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // wait for the Serial to really be ready
  Serial.println("==========");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  LedMatrix.begin();

  Serial.println("INITIALIZING...");

  // TODO: in case of permanent error, show it on the screen
  
  // initialize external WiFi module
  Serial.println("[INIT] WiFi module...");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("[ERROR] Communication with WiFi module failed!");
    for(;;){} // can't go further
  }
  Serial.println("[INIT] WiFi module OK");

  // initialize the RTC
  Serial.println("[INIT] Real-time Clock...");
  if (!RTC.begin()) {
    Serial.println("[ERROR] RTC init failed!");
    for(;;){} // can't go further
  }
  Serial.println("[INIT] Real-time Clock OK");
  
  if (!RTC.isRunning()) {
    LedMatrix.loadSequence(LEDMATRIX_ANIMATION_WIFI_SEARCH); // https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/#frame-gallery
    LedMatrix.play(true);

    if (!connectToWiFi()) {
      Serial.println("[ERROR] Can't connect to WiFi!");
      for(;;){} // can't go further
    }
    // else:
    if (DEBUG_COCO) Serial.println("[WIFI] Successfully connected to WiFi");
    printWifiStatus();

    if (!setRtcFromNtp()) {
      Serial.println("[ERROR] Can't set time to RTC!");
      for(;;){} // can't go further
    }
    // else:
    if (DEBUG_COCO) Serial.println("[NTP]  Successfully set RTC from NTP time");

    LedMatrix.play(false);
    LedMatrix.clear();
  }
  Serial.println("[INIT] The RTC should now be running.");

  // set a callback every day at 3:00 for clock update
  RTCTime UpdateAlarmTime;
  AlarmMatch UpdateMatchTime;
  UpdateAlarmTime.setHour(3); // match when hour = 3 // TODO: apply timezone setting?
  UpdateMatchTime.addMatchHour();
  if (!RTC.setAlarmCallback(rtcUpdateCallback, UpdateAlarmTime, UpdateMatchTime)) { // https://docs.arduino.cc/tutorials/uno-r4-minima/rtc/#alarm-callback
    Serial.println("[ERROR] Time update callback can't be set!");
    for(;;){} // can't go further
  }
}

void loop() {
  static bool is_day_set = false;
  static bool is_day_up = false;

  RTCTime CurrentTime;
  RTCTime wakeup (1, Month::JANUARY, 1970, 05, 45, 0, DayOfWeek::MONDAY, SaveLight::SAVING_TIME_INACTIVE);
  RTCTime gosleep(1, Month::JANUARY, 1970, 19, 15, 0, DayOfWeek::MONDAY, SaveLight::SAVING_TIME_INACTIVE);

  // clock update routine
  if (flag_update) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("[RTC] Waking up for datetime update");
    if (connectToWiFi()) {
      if (DEBUG_COCO) Serial.println("[NTP]  Successfully connected to WiFi");
      setRtcFromNtp();
    }
    WiFi.end(); // we don't need to stay connected
    digitalWrite(LED_BUILTIN, LOW);
    flag_update = false;
  }

  // time comparison
  if (DEBUG_COCO) Serial.println("---");
  RTC.getTime(CurrentTime);
  //if (DEBUG_COCO) Serial.println(DayOfWeek2int(CurrentTime.getDayOfWeek(), false));
  if (DEBUG_COCO) Serial.print(CurrentTime.getUnixTime());
  if (DEBUG_COCO) Serial.println(" / "+ String(CurrentTime));
  CurrentTime.setDayOfMonth(1);
  CurrentTime.setMonthOfYear(Month::JANUARY);
  CurrentTime.setYear(1970);
  if (DEBUG_COCO) Serial.print(CurrentTime.getUnixTime());
  if (DEBUG_COCO) Serial.println(" / "+ String(CurrentTime));

  if (DEBUG_COCO) Serial.print(wakeup.getUnixTime());
  if (DEBUG_COCO) Serial.println(" / "+ String(wakeup));
  if (DEBUG_COCO) Serial.print(gosleep.getUnixTime());
  if (DEBUG_COCO) Serial.println(" / "+ String(gosleep));
  
  if ((!is_day_set || is_day_up) && (CurrentTime.getUnixTime() < wakeup.getUnixTime() || CurrentTime.getUnixTime() >= gosleep.getUnixTime())) {
    // day -> night transition
    is_day_set = true;
    is_day_up = false;
    if (DEBUG_COCO) Serial.println("It's time to sleep");
    LedMatrix.loadSequence(anim_asleep);
    LedMatrix.play(true);
  }
  else if ((!is_day_set || !is_day_up) && (wakeup.getUnixTime() <= CurrentTime.getUnixTime() && CurrentTime.getUnixTime() < gosleep.getUnixTime())) {
    // night -> day transition
    is_day_set = true;
    is_day_up = true;
    if (DEBUG_COCO) Serial.println("It's time to wake up");
    LedMatrix.loadSequence(anim_awake);
    LedMatrix.play(true);
  }
  // else: continue to play the current animation

  delay(5000); // 5 seconds update rate
}

void rtcUpdateCallback() {
  flag_update = true;
}
