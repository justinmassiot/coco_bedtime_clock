/*
MIT License
Copyright (c) 2025 Justin MASSIOT

This is a program built for Arduino UNO R4.
Inspired by the "RTC_NTPSync" and "DisplaySingleFrame" examples.

LED matrix pictures and animations can be built at: https://ledmatrix-editor.arduino.cc/
*/

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


bool connectToWiFi(unsigned int waiting_time_ms) {
  int wifi_status = WL_IDLE_STATUS;

  // attempt to connect to WiFi network
  Serial.print("[WIFI] Attempting to connect to SSID: ");
  Serial.println(ssid);
  wifi_status = WiFi.begin(ssid, pass); // connect to WPA/WPA2 network; change this line if using open or WEP network

  // TODO: loop on status until timeout
  delay(waiting_time_ms);

  return (wifi_status == WL_CONNECTED); // true if connected, false otherwise
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("[WIFI] SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  Serial.print("[WIFI] IP Address: ");
  Serial.println(WiFi.localIP());

  // print the received signal strength:
  Serial.print("[WIFI] Signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void setRtcFromNtp() {
  Serial.println("[NTP] Getting time from NTP and copying it to RTC...");
  
  // get the current date and time from NTP
  NtpClient.begin();
  NtpClient.update();
  auto timezone_offset_hours = 0;
  auto unix_time = NtpClient.getEpochTime() + (timezone_offset_hours * 3600);
  Serial.print("[NTP] Unix time = ");
  Serial.println(unix_time);
  RTCTime time_to_set = RTCTime(unix_time);
  RTC.setTime(time_to_set);

  // TODO: set timezone and DST with https://github.com/JChristensen/Timezone/blob/master/examples/HardwareRTC/HardwareRTC.ino

  NtpClient.end();
}

void setup() {
  bool is_wifi_connected;

  Serial.begin(115200);
  delay(1000); // wait for the Serial to really be ready
  Serial.println("==========");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  LedMatrix.begin();

  Serial.println("INITIALIZING...");
  
  // initialize external WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    Serial.println("STOP");
    for(;;){} // stop definitely
  }
  Serial.println("[INIT] WiFi module OK");

  // initialize the RTC
  if (!RTC.begin()) {
    Serial.println("RTC init failed!");
    Serial.println("STOP");
    for(;;){} // stop definitely
  }
  Serial.println("[INIT] Real-time Clock OK");
  
  if (!RTC.isRunning()) {
    LedMatrix.loadSequence(LEDMATRIX_ANIMATION_WIFI_SEARCH); // https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/#frame-gallery
    LedMatrix.play(true);

    do {
      is_wifi_connected = connectToWiFi(10000);
    }
    while (!is_wifi_connected); // infinite loop until connection
    Serial.println("[WIFI] Successfully connected to WiFi");
    //printWifiStatus();
    setRtcFromNtp();

    LedMatrix.play(false);
    LedMatrix.clear();
  }
  Serial.println("[INIT] The RTC should now be running.");

  /*
  if (!RTC.setPeriodicCallback(periodicCallback, Period::ONCE_EVERY_2_SEC)) { // https://docs.arduino.cc/tutorials/uno-r4-minima/rtc/#periodic-interrupt
    Serial.println("[ERROR] Periodic callback not set");
  }
  */

  // set a callback every day at 3:00 for clock update
  RTCTime UpdateAlarmTime;
  AlarmMatch UpdateMatchTime;
  UpdateAlarmTime.setHour(3); // match when hour = 3
  UpdateMatchTime.addMatchHour();
  if (!RTC.setAlarmCallback(rtcUpdateCallback, UpdateAlarmTime, UpdateMatchTime)) { // https://docs.arduino.cc/tutorials/uno-r4-minima/rtc/#alarm-callback
    Serial.println("[ERROR] Update callback not set");
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
    if (connectToWiFi(10000)) {
      Serial.println("[NTP] Successfully connected to WiFi");
      setRtcFromNtp();
    }
    WiFi.end(); // we don't need to stay connected
    digitalWrite(LED_BUILTIN, LOW);
    flag_update = false;
  }

  // time comparison
  Serial.println("---");
  RTC.getTime(CurrentTime);
  //Serial.println(DayOfWeek2int(CurrentTime.getDayOfWeek(), false));
  Serial.print(CurrentTime.getUnixTime());
  Serial.println(" / "+ String(CurrentTime));
  CurrentTime.setDayOfMonth(1);
  CurrentTime.setMonthOfYear(Month::JANUARY);
  CurrentTime.setYear(1970);
  Serial.print(CurrentTime.getUnixTime());
  Serial.println(" / "+ String(CurrentTime));

  Serial.print(wakeup.getUnixTime());
  Serial.println(" / "+ String(wakeup));
  Serial.print(gosleep.getUnixTime());
  Serial.println(" / "+ String(gosleep));
  
  if ((!is_day_set || is_day_up) && (CurrentTime.getUnixTime() < wakeup.getUnixTime() || CurrentTime.getUnixTime() >= gosleep.getUnixTime())) {
    // day -> night transition
    is_day_set = true;
    is_day_up = false;
    Serial.println("It's time to sleep");
    LedMatrix.loadSequence(anim_asleep);
    LedMatrix.play(true);
  }
  else if ((!is_day_set || !is_day_up) && (wakeup.getUnixTime() <= CurrentTime.getUnixTime() && CurrentTime.getUnixTime() < gosleep.getUnixTime())) {
    // night -> day transition
    is_day_set = true;
    is_day_up = true;
    Serial.println("It's time to wake up");
    LedMatrix.loadSequence(anim_awake);
    LedMatrix.play(true);
  }
  // else: continue to play the current animation

  delay(5000); // 5 seconds update rate
}

void rtcUpdateCallback() {
  flag_update = true;
}
