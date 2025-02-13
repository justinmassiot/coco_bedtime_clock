/*
MIT License
Copyright (c) 2025 Justin MASSIOT

This is a program built for Arduino UNO R4.
Inspired by the "RTC_NTPSync" and "DisplaySingleFrame" examples.

LED matrix pictures and animations can be built at: https://ledmatrix-editor.arduino.cc/
*/

// log output level
#define DEBUG_COCO 1 // debug => 2,
                     // info => 1,
                     // error & init => 0 (or unset)

#define NTP_MAX_CONN_ATTEMPT 3

#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h> // needs the NTPClient library to be installed
#include "RTC.h"
#include "Arduino_LED_Matrix.h" // LED_Matrix library

#include "clock_agenda.h"
#include "screen_night.h"
#include "screen_day.h"
#include "wifi_info.h"


//========


char ssid[] = SECRET_SSID; // from "wifi_info.h"
char pass[] = SECRET_PASS; // from "wifi_info.h"

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient NtpClient(Udp);
ArduinoLEDMatrix LedMatrix;
bool flag_update = false;


//========


void printWifiStatus() {
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


// get the current date and time from NTP and push it to the RTC
bool setRtcFromNtp() {
  bool ntp_time_ok = false;
  bool rtc_time_ok = false;
  unsigned int ntp_conn_attempts = 0;

  NtpClient.begin(); // starts the underlying UDP client with the default local port
  do {
    if (DEBUG_COCO >= 1) Serial.println("[NTP ] Getting time from NTP...");
    ntp_time_ok = NtpClient.forceUpdate(); // has an internal timeout of 1000ms
  } while (!ntp_time_ok && ++ntp_conn_attempts < NTP_MAX_CONN_ATTEMPT);

  if (ntp_time_ok) {
    auto timezone_offset_hours = 0;
    auto unix_time = 0;

    if (DEBUG_COCO >= 1) Serial.println("[NTP ] NTP connection established");
    unix_time = NtpClient.getEpochTime() + (timezone_offset_hours * 3600);
    if (DEBUG_COCO >= 2) Serial.print("[NTP ] Unix time = ");
    if (DEBUG_COCO >= 2) Serial.println(unix_time);

    // TODO: set timezone and DST
    // https://github.com/arduino-libraries/NTPClient/blob/master/NTPClient.h#L92C10-L92C40
    // https://github.com/JChristensen/Timezone/blob/master/examples/HardwareRTC/HardwareRTC.ino

    if (DEBUG_COCO >= 1) Serial.println("[RTC ] Setting RTC from NTP...");
    RTCTime time_to_set = RTCTime(unix_time);
    rtc_time_ok = RTC.setTime(time_to_set);
  }
  NtpClient.end();

  return (ntp_time_ok && rtc_time_ok);
}


void getCurrentTime(RTCTime& currentTime) {
  RTC.getTime(currentTime);

  // raw RTC date-time
  if (DEBUG_COCO >= 2) Serial.print(currentTime.getUnixTime());
  if (DEBUG_COCO >= 2) Serial.println(" / "+ String(currentTime));

  // drop the day/month/year parts from the current date-time to only keep the time
  currentTime.setDayOfMonth(1);
  currentTime.setMonthOfYear(Month::JANUARY);
  currentTime.setYear(1970);
  //currentTime.setSaveLight(SaveLight::SAVING_TIME_INACTIVE);

  // minimal RTC time (without date)
  if (DEBUG_COCO >= 2) Serial.print(currentTime.getUnixTime());
  if (DEBUG_COCO >= 2) Serial.println(" / "+ String(currentTime));
}


uint8_t currentDayOfWeek(RTCTime currentTime) {
  if (DEBUG_COCO >= 2) Serial.print("Day of week: ");
  if (DEBUG_COCO >= 2) Serial.println((uint8_t)currentTime.getDayOfWeek()); // we don't use getDayOfWeek() but that's on purpose
  return (uint8_t)currentTime.getDayOfWeek(); // we don't use getDayOfWeek() but that's on purpose
}


bool isDayUp(RTCTime current, RTCTime wakeup, RTCTime gosleep) {
  // check if current time is between wake-up time and go-to-sleep time
  if (DEBUG_COCO >= 2) Serial.print(wakeup.getUnixTime());
  if (DEBUG_COCO >= 2) Serial.println(" / "+ String(wakeup));
  if (DEBUG_COCO >= 2) Serial.print(gosleep.getUnixTime());
  if (DEBUG_COCO >= 2) Serial.println(" / "+ String(gosleep));
  if (DEBUG_COCO >= 2) Serial.print("Is day up? ");
  if (DEBUG_COCO >= 2) Serial.println( wakeup.getUnixTime() <= current.getUnixTime() && current.getUnixTime() < gosleep.getUnixTime() );
  return (wakeup.getUnixTime() <= current.getUnixTime() && current.getUnixTime() < gosleep.getUnixTime());
}


//========


void setup() {
  Serial.begin(115200);
  delay(1000); // wait for the Serial to really be ready
  Serial.println("==========");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // setup sequence started
  
  LedMatrix.begin();
  LedMatrix.loadSequence(LEDMATRIX_ANIMATION_INFINITY_LOOP_LOADER); // https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/#frame-gallery
  LedMatrix.play(true);

  Serial.println("INITIALIZING...");

  // TODO: in case of permanent error, show it on the screen
  
  // initialize external WiFi module
  Serial.println("[INIT] WiFi module...");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("*ERROR* Communication with WiFi module failed!");
    for(;;){} // can't go further
  }
  Serial.println("[INIT] WiFi module OK");

  // initialize the RTC
  Serial.println("[INIT] Real-time Clock...");
  if (!RTC.begin()) {
    Serial.println("*ERROR* RTC init failed!");
    for(;;){} // can't go further
  }
  Serial.println("[INIT] Real-time Clock OK");
  
  if (!RTC.isRunning() || DEBUG_COCO >= 2) {
    LedMatrix.loadSequence(LEDMATRIX_ANIMATION_WIFI_SEARCH); // https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/#frame-gallery
    LedMatrix.play(true);

    if (DEBUG_COCO >= 1) Serial.println("[WIFI] Attempting to connect...");
    // modify the below begin() call for Open (unsecured) or WEP networks
    if (WiFi.begin(ssid, pass) != WL_CONNECTED) { // begin() is blocking and always exit after `WL_MAX_ATTEMPT_CONNECTION` attempts
      Serial.println("*ERROR* Can't connect to WiFi!");
      for(;;){} // can't go further
    }
    // else:
    if (DEBUG_COCO >= 1) Serial.println("[WIFI] Successfully connected to WiFi");
    if (DEBUG_COCO >= 2) printWifiStatus();

    if (!setRtcFromNtp()) {
      Serial.println("*ERROR* Can't set time from NTP to RTC!");
      for(;;){} // can't go further
    }
    // else:
    if (DEBUG_COCO >= 1) Serial.println("[RTC ] Successfully set RTC from NTP time");
    
    if (DEBUG_COCO >= 1) Serial.println("[WIFI] Turning WiFi Off.");
    WiFi.end(); // we don't need to stay connected

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
    Serial.println("*ERROR* Time update callback can't be set!");
    for(;;){} // can't go further
  }
  
  digitalWrite(LED_BUILTIN, LOW); // setup sequence finished
}


//========


void loop() {
  static bool is_day_set = false;
  static bool is_day_up_prev = false;

  bool is_day_up;
  unsigned int day_of_week;
  RTCTime CurrentTime;
  RTCTime wakeup, gosleep;

  // clock re-sync routine
  if (flag_update) {
    flag_update = false;
    digitalWrite(LED_BUILTIN, HIGH);
    if (DEBUG_COCO >= 1) Serial.println("[RTC ] Waking up for datetime update");
    if (DEBUG_COCO >= 1) Serial.println("[WIFI] Attempting to connect...");
    // modify the below begin() call for Open (unsecured) or WEP networks
    if (WiFi.begin(ssid, pass) == WL_CONNECTED) { // begin() is blocking and always exit after `WL_MAX_ATTEMPT_CONNECTION` attempts
      if (DEBUG_COCO >= 1) Serial.println("[WIFI] Successfully connected to WiFi");
      if (DEBUG_COCO >= 2) printWifiStatus();
      setRtcFromNtp(); // TODO: error handling
    }
    else {
      Serial.println("*ERROR* Can't connect to WiFi!");
    }
    if (DEBUG_COCO >= 1) Serial.println("[WIFI] Turning WiFi Off.");
    WiFi.end(); // we don't need to stay connected
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (DEBUG_COCO >= 2) Serial.println("---");
  
  getCurrentTime(CurrentTime);
  day_of_week = currentDayOfWeek(CurrentTime);
  wakeup = clock_agenda[day_of_week][0];
  gosleep = clock_agenda[day_of_week][1];
  
  // time comparison
  is_day_up = isDayUp(CurrentTime, wakeup, gosleep);
  if ((!is_day_set || is_day_up_prev) && !is_day_up) {
    // day -> night transition
    is_day_set = true;
    is_day_up_prev = false;
    if (DEBUG_COCO >= 1) Serial.println("It's time to sleep");
    LedMatrix.loadSequence(anim_asleep);
    LedMatrix.play(true);
  }
  else if ((!is_day_set || !is_day_up_prev) && is_day_up) {
    // night -> day transition
    is_day_set = true;
    is_day_up_prev = true;
    if (DEBUG_COCO >= 1) Serial.println("It's time to wake up");
    LedMatrix.loadSequence(anim_awake);
    LedMatrix.play(true);
  }
  // else: continue to play the current animation

  delay(5000); // 5 seconds update rate
}


//========


void rtcUpdateCallback() {
  flag_update = true;
}
