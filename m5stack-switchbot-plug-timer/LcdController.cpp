/* ----------------------------------------------------------------
  LcdController.cpp
  - LCD へのコンテンツ表示処理を行う

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#include "LcdController.h"

// ===============================================================
// LcdController クラス
// ===============================================================

// ---------------------------------------------------------------
// コンストラクタ
// ---------------------------------------------------------------
LcdController::LcdController(char* address, char* name, char* time) {
  this->_address = address;
  this->_name = name;
  this->_time = time;
}

// ---------------------------------------------------------------
// 初期化
// ---------------------------------------------------------------
void LcdController::init() {
  M5.Lcd.begin();
  M5.Lcd.clear();
  M5.Lcd.setBrightness(150);
  M5.Lcd.setTextWrap(false, false);

  // タイトルを表示
  this->_showTitle();

  // 電源状態表示
  this->showPowerStatus(false);

  // OFF/ON タイマー時刻を表示
  if (String(this->_time) != String("")) {
    this->_showTimerTime();
  }

  // ボタンメニュー表示
  this->showButtonMenu(0);
}

void LcdController::_showTitle() {
  M5.Lcd.setTextColor(WHITE, BLACK);

  M5.Lcd.setTextSize(2);
  int16_t x1 = this->_getXaxisForTextCentering(this->_name);
  M5.Lcd.setCursor(x1, 4);
  M5.Lcd.printf(this->_name);

  M5.Lcd.setTextSize(2);
  int16_t x2 = this->_getXaxisForTextCentering(this->_address);
  M5.Lcd.setCursor(x2, 28);
  M5.Lcd.printf(this->_address);
}

// テキストをセンタリングした際の x 座標の値を取得
int16_t LcdController::_getXaxisForTextCentering(const char* text) {
  int16_t dwidth = M5.Lcd.width();
  int16_t twidth = M5.Lcd.textWidth(text);
  int16_t x = (dwidth - twidth) / 2;
  return x;
}

// ---------------------------------------------------------------
// 電源状態表示
// ---------------------------------------------------------------
void LcdController::showPowerStatus(bool status) {
  int16_t w = M5.Lcd.width();
  int16_t h = M5.Lcd.height();
  uint32_t color = LIGHTGREY;
  String text = "OFF";

  if (status == true) {
    color = GREEN;
    text = "ON";
  }

  int32_t cx = w / 2;
  int32_t cy = h / 2;

  M5.Lcd.fillCircle(cx, cy, h / 4, color);
  M5.Lcd.fillCircle(cx, cy, h / 5, WHITE);

  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(DARKGREY, WHITE);

  int16_t x = this->_getXaxisForTextCentering(text.c_str());
  M5.Lcd.setCursor(x, cy - 8);
  M5.Lcd.printf(text.c_str());
}

// ---------------------------------------------------------------
// ボタンメニュー表示
// ---------------------------------------------------------------
void LcdController::showButtonMenu(uint8_t mode) {
  this->clearMessage();

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 225);
  M5.Lcd.setTextColor(WHITE, BLACK);

  if (mode == 0) {
    M5.Lcd.printf("                          ");
  } else if (mode == 1) {
    M5.Lcd.printf("   LOG    ON/OFF          ");
  } else if (mode == 2) {
    M5.Lcd.printf(" CANCEL              OK   ");
  } else if (mode == 3) {
    M5.Lcd.printf("       PROCESSING...      ");
  } else if (mode == 4) {
    M5.Lcd.printf("  BACK                    ");
  } else {
    M5.Lcd.printf("                          ");
  }
}


// ---------------------------------------------------------------
// メッセージ表示
// ---------------------------------------------------------------
void LcdController::showMessage(String msg) {
  this->clearMessage();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  int16_t x = this->_getXaxisForTextCentering(msg.c_str());
  M5.Lcd.setCursor(x, 194);
  M5.Lcd.printf(msg.c_str());
}

// ---------------------------------------------------------------
// エラー表示
// ---------------------------------------------------------------
void LcdController::showError(String msg) {
  this->clearMessage();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(RED, BLACK);
  int16_t x = this->_getXaxisForTextCentering(msg.c_str());
  M5.Lcd.setCursor(x, 194);
  M5.Lcd.printf(msg.c_str());
}

// ---------------------------------------------------------------
// メッセージ消去
// ---------------------------------------------------------------
void LcdController::clearMessage() {
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 194);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf("                           ");
}


// ---------------------------------------------------------------
// 現在時刻を表示
// ---------------------------------------------------------------
void LcdController::showCurrentTime(String time) {
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);

  M5.Lcd.setCursor(230, 126);
  M5.Lcd.printf("Current");

  M5.Lcd.setCursor(266, 146);
  M5.Lcd.printf("Time");

  M5.Lcd.setCursor(218, 166);
  M5.Lcd.printf(time.c_str());
}


// OFF/ON タイマー時刻を表示
void LcdController::_showTimerTime() {
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);

  M5.Lcd.setCursor(10, 126);
  M5.Lcd.printf("OFF/ON");

  M5.Lcd.setCursor(10, 146);
  M5.Lcd.printf("Timer");

  M5.Lcd.setCursor(10, 166);
  M5.Lcd.printf(this->_time);
}

// ---------------------------------------------------------------
// LCD 省電力モードへ移行
// ---------------------------------------------------------------
void LcdController::sleep() {
  M5.Lcd.sleep();
}

// ---------------------------------------------------------------
// LCD 省電力モードから復帰
// ---------------------------------------------------------------
void LcdController::wakeup() {
  M5.Lcd.wakeup();
}

// ---------------------------------------------------------------
// ログ表示
// ---------------------------------------------------------------
void LcdController::showLogs(vector<LogRecord> const& logs) {
  M5.Lcd.clear();
  this->showButtonMenu(4);
  M5.Lcd.setTextSize(1);

  int16_t y = 3;

  for (LogRecord log : logs) {
    if (log.err) {
      M5.Lcd.setTextColor(RED, BLACK);

    } else {
      M5.Lcd.setTextColor(WHITE, BLACK);
    }

    M5.Lcd.setCursor(10, y);
    String line = String(log.timestamp + " " + log.text);
    M5.Lcd.printf(line.c_str());

    y = y + 15;
  }
}
