/* ----------------------------------------------------------------
  TimeManager.h
  - Wi-Fi に接続して NTP サーバーを使って時刻同期する

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#ifndef TimeManager_h
#define TimeManager_h
#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <time.h>

// ---------------------------------------------------------------
// TimeManager クラス
// ---------------------------------------------------------------
class TimeManager {
private:
  // Wi-Fi の SSID とパスワード
  char* _ssid;
  const char* _pass;

  const long _TZ_OFFSET = 9 * 3600; // タイムゾーンオフセット (秒)
  //const char* _NTP_SERVER = "ntp.nict.jp";  // NTP サーバー
  const char* _NTP_SERVER = "ntp.jst.mfeed.ad.jp";  // NTP サーバー
  //const char* _NTP_SERVER = "time.google.com";  // NTP サーバー

  const uint16_t _WIFI_TIMEOUT = 10000; // Wi-Fi 接続タイムアウト (ミリ秒)
  const uint16_t _NTP_TIMEOUT = 5000; // NTP 時刻同期タイムアウト (ミリ秒)

  String _error; // 最終のエラーメッセージ

public:
  // コンストラクタ
  TimeManager(char* ssid, char* pass);

  // エラーメッセージを取得
  String getError();

  // 初期化
  void init();
  
  // Wi-Fi 接続および時刻同期
  bool sync();

  // 現在日時を取得 (RTC を使わない)
  //tm now();

  // 今日の日付を RTC から取得
  String getRtcDate();

  // 現在時刻を RTC から取得
  String getRtcTime();

  // 現在日時を RTC から取得
  String getRtcDateAndTime();
};

#endif
