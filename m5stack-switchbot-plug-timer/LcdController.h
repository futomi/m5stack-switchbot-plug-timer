/* ----------------------------------------------------------------
  LcdController.h
  - LCD へのコンテンツ表示処理を行う

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#ifndef LcdController_h
#define LcdController_h
#include <Arduino.h>
#include <M5Core2.h>
#include <vector>
using std::vector;

// ログのレコードの構造体
struct LogRecord {
  String timestamp;  // タイムスタンプ (YYYY-MM-DD hh:mm:ss)
  String text;       // ログの内容
  bool err;          // エラーかどうか
};

// ---------------------------------------------------------------
// LcdController クラス
// ---------------------------------------------------------------
class LcdController {
private:
  // SwitchBot Plug Mini の BLE MAC アドレス
  char* _address;

  // SwitchBot Plug Mini の表示名
  const char* _name;

  // OFF/ON タイマーの時刻
  const char* _time;

private:
  // タイトルを表示
  void _showTitle();

  // OFF/ON タイマー時刻を表示
  void _showTimerTime();

  // テキストをセンタリングした際の x 座標の値を取得
  int16_t _getXaxisForTextCentering(const char* text);


public:
  // コンストラクタ
  LcdController(char* address, char* name, char* time);

  // 初期化
  void init();

  // 電源状態表示
  void showPowerStatus(bool status);

  // ボタンメニュー表示
  void showButtonMenu(uint8_t mode);

  // メッセージ表示
  void showMessage(String msg);

  // エラー表示
  void showError(String msg);

  // メッセージ消去
  void clearMessage();

  // 現在時刻を表示
  void showCurrentTime(String time);

  // LCD 省電力モードへ移行
  void sleep();

  // LCD 省電力モードから復帰
  void wakeup();

  // ログ表示
  void showLogs(vector<LogRecord> const& logs);
};

#endif
