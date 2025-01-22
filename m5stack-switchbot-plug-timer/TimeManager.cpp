/* ----------------------------------------------------------------
  TimeManager.cpp
  - Wi-Fi に接続して NTP サーバーを使って時刻同期する

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#include "TimeManager.h"

// ===============================================================
// TimeManager クラス
// ===============================================================

// ---------------------------------------------------------------
// コンストラクタ
// ---------------------------------------------------------------
TimeManager::TimeManager(char* ssid, char* pass) {
  this->_ssid = ssid;
  this->_pass = pass;
}

// ---------------------------------------------------------------
// エラーメッセージを取得
// ---------------------------------------------------------------
String TimeManager::getError() {
  return this->_error;
}

// ---------------------------------------------------------------
//  初期化
// ---------------------------------------------------------------
void TimeManager::init() {
  M5.Rtc.begin();
}

// ---------------------------------------------------------------
//  Wi-Fi 接続および時刻同期
// ---------------------------------------------------------------
bool TimeManager::sync() {
  // Wi-Fi 接続
  uint32_t wifi_stime = millis();
  WiFi.begin(this->_ssid, this->_pass);
  bool wifi_success = true;

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() > wifi_stime + this->_WIFI_TIMEOUT) {
      wifi_success = false;
      break;
    }
    delay(100);
  }

  if (wifi_success == false) {
    WiFi.disconnect(true);
    this->_error = "WIFI_TIMEOUT";
    return false;
  }

  delay(1000);

  // NTP サーバーと同期
  uint32_t ntp_stime = millis();
  bool ntp_success = true;
  configTime(this->_TZ_OFFSET, 0, this->_NTP_SERVER);

  struct tm dt;
  while (!getLocalTime(&dt)) {
    if (millis() > ntp_stime + this->_NTP_TIMEOUT) {
      ntp_success = false;
      break;
    }
    delay(100);
  }

  if (ntp_success == false) {
    WiFi.disconnect(true);
    this->_error = "NTP_TIMEOUT";
    return false;
  }

  // RTC に日時をセット
  RTC_TimeTypeDef rtctime;
  rtctime.Hours = dt.tm_hour;
  rtctime.Minutes = dt.tm_min;
  rtctime.Seconds = dt.tm_sec;
  M5.Rtc.SetTime(&rtctime);

  RTC_DateTypeDef rtcdate;
  rtcdate.WeekDay = dt.tm_wday;
  rtcdate.Month = dt.tm_mon + 1;
  rtcdate.Date = dt.tm_mday;
  rtcdate.Year = dt.tm_year + 1900;
  M5.Rtc.SetDate(&rtcdate);

  // Wi-Fi 切断
  WiFi.disconnect(true);

  return true;
}

// ---------------------------------------------------------------
//  現在日時を取得 (RTC を使わない)
// ---------------------------------------------------------------
/*
tm TimeManager::now() {
  struct tm dt;
  getLocalTime(&dt);
  return dt;
}
*/

// ---------------------------------------------------------------
// 今日の日付を RTC から取得
// ---------------------------------------------------------------
String TimeManager::getRtcDate() {
  RTC_DateTypeDef rtcdate;
  M5.Rtc.GetDate(&rtcdate);

  char date_str[11];
  sprintf(date_str, "%04d/%02d/%02d", rtcdate.Year, rtcdate.Month, rtcdate.Date);
  String date = String(date_str);

  return date;
}

// ---------------------------------------------------------------
//  現在時刻を RTC から取得
// ---------------------------------------------------------------
String TimeManager::getRtcTime() {
  RTC_TimeTypeDef rtctime;
  M5.Rtc.GetTime(&rtctime);

  char time_str[9];
  sprintf(time_str, "%02d:%02d:%02d", rtctime.Hours, rtctime.Minutes, rtctime.Seconds);
  String time = String(time_str);

  return time;
}

// ---------------------------------------------------------------
//  現在日時を RTC から取得
// ---------------------------------------------------------------
String TimeManager::getRtcDateAndTime() {
  String date = this->getRtcDate();
  String time = this->getRtcTime();
  return String(date + " " + time);
}
