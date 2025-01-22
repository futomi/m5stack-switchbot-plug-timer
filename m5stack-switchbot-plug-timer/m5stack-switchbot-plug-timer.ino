/* ----------------------------------------------------------------
  switchbot_plug.ino

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#include <M5Core2.h>
#include <vector>

#include "SwitchBotPlugMini.h"
#include "LcdController.h"
#include "TimeManager.h"

// ================================================================
// ユーザー設定
// ----------------------------------------------------------------

// SwitchBot Plug Mini の BLE MAC アドレス
char* BLE_MAC_ADDR = "3c:84:27:ff:ff:ff";

// Wi-FI の SSID とパスワード
char* SSID = "YOUR_SSID";
char* PASS = "YOUR_PASSWORD";

// OFF/ON を実施したい時刻 ("hh:mm:ss")
// - OFF/ON を実施しない場合は空文字列をセット
char* TIMER_TIME = "05:00:00";

// OFF から ON までの待ち時間 (ミリ秒)
uint16_t TIMER_INTERVAL = 5000;

// NTP で時刻同期する時刻 ("hh:mm:ss")
char* NTP_TIME = "03:00:00";

//============================================================== */
// 各種グローバル変数
// ----------------------------------------------------------------

// SwitchBot Plug Mini の表示名
char* NAME = "SwitchBot Plug mini (JP)";

// LCD がスリープするまでの時間 (ミリ秒)
// - 0 を指定するとスリープ無効
uint32_t SLEEP_TIME = 60000;


// SwitchBotPlugMini インスタンスの生成
SwitchBotPlugMini switchBotPlugMini(BLE_MAC_ADDR);

// LcdController インスタンスの生成
LcdController lcdController(BLE_MAC_ADDR, NAME, TIMER_TIME);

// TimeManager インスタンスの生成
TimeManager timeManager(SSID, PASS);

// ボタンモード (0:初期状態, 1:操作待受, 2:確認, 3:処理中, 4:ログ表示)
uint8_t btnmode = 0;

// LCD 省電力モードかどうかのフラグ
bool sleeping = false;

// LCD に表示した最終時刻 ("hh:mm:ss")
String last_lcd_time = "";

// 最後にオフ・オンを実施した日付 ("YYYY/MM/DD")
String last_timer_date = "";

// 最後に NTP 時刻同期した日付 ("YYYY/MM/DD")
String last_ntp_date = "";

// ログの配列
vector<LogRecord> logs;

// ログの保存数
const size_t LOG_LIMIT = 14;

//============================================================== */


// ボタン表示モードを変更
void setButtonMode(uint8_t mode) {
  btnmode = mode;
  lcdController.showButtonMenu(btnmode);
}

// BLE 接続して電源状態を取得して画面表示
void getAndShowPowerStatus() {
  setButtonMode(0);

  // BLE 接続
  lcdController.showMessage("Connecting BLE...");

  if (!switchBotPlugMini.connect()) {
    String err = switchBotPlugMini.getError();
    lcdController.showError(err);
    setButtonMode(1);
    return;
  }

  lcdController.showMessage("Getting power status...");
  bool status;

  if (switchBotPlugMini.getPowerStatus(status)) {
    lcdController.showPowerStatus(status);
  } else {
    lcdController.showPowerStatus(false);
    String err = switchBotPlugMini.getError();
    lcdController.showError(err.c_str());
    setButtonMode(1);
    return;
  }

  lcdController.showMessage("Disonnecting BLE...");
  switchBotPlugMini.disconnect();
  lcdController.clearMessage();
  setButtonMode(1);
}

void pushLog(String text, bool err = false) {
  LogRecord rec = {
    timeManager.getRtcDateAndTime(),
    text,
    err
  };

  logs.push_back(rec);

  if (logs.size() > LOG_LIMIT) {
    logs.erase(logs.begin());
  }
}

void setup() {
  M5.begin();

  // 各種ライブラリの準備
  lcdController.init();
  timeManager.init();

  // Wi-Fi 接続して NTP 時刻同期
  lcdController.showMessage("Syncing time using NTP...");
  while (!timeManager.sync()) {
    delay(5000);
  }

  // 現在時刻を表示
  String time = timeManager.getRtcTime();
  lcdController.showCurrentTime(time);

  // BLE スキャン開始
  lcdController.showMessage("Scaning BLE devices...");
  BLEAdvertisedDevice device;
  bool found = false;

  while (found == false) {
    found = switchBotPlugMini.find(device);
    delay(100);
  }

  // BLE 接続して電源状態を取得して画面表示
  getAndShowPowerStatus();

  pushLog("SYSTEM_STARTED_UP");
}

void loop() {
  M5.update();

  if (sleeping == true) {
    // ボタン操作があったら LCD 省電力モードから復帰
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
      lcdController.wakeup();
      if (btnmode == 1) {
        getAndShowPowerStatus();
      }
      sleeping = false;
    }

  } else if (sleeping == false) {
    if (btnmode == 1) {  // 操作待受モード
      // ボタン A (LOG) が押されたときの処理
      if (M5.BtnA.wasPressed()) {
        setButtonMode(4);
        lcdController.showLogs(logs);
      }

      // ボタン B (SWITCH) が押されたときの処理
      if (M5.BtnB.wasPressed()) {
        setButtonMode(2);  // ボタン確認モード表示
      }

    } else if (btnmode == 2) {  // ボタン確認モード
      // ボタン A (CANCEL) が押されたときの処理
      if (M5.BtnA.wasPressed()) {
        setButtonMode(1);
      }

      // ボタン C (OK) が押されたときの処理
      if (M5.BtnC.wasPressed()) {
        setButtonMode(3);  // ボタン処理中 (PROCESSING..) モード表示

        // ON/OFF を切り替え
        bool status;
        if (switchBotPlugMini.togglePowerStatus(status)) {
          lcdController.showPowerStatus(status);
        } else {
          String msg = switchBotPlugMini.getError();
          lcdController.showError(msg);
        }

        setButtonMode(1);  // ボタン待受モード表示
      }

    } else if (btnmode == 4) {  // ログ表示モード
      // ボタン A (BACK) が押されたときの処理
      if (M5.BtnA.wasPressed()) {
        // BLE 接続して電源状態を取得して画面表示
        lcdController.init();
        getAndShowPowerStatus();

        setButtonMode(1);  // ボタン待受モード表示
      }
    }

    // 所定時間 (SLEEP_TIME) 以上ボタン操作がないなら LCD 省電力モードに移行
    if (SLEEP_TIME > 0) {
      uint32_t now = millis();
      uint32_t ca = M5.BtnA.lastChange();
      uint32_t cb = M5.BtnB.lastChange();
      uint32_t cc = M5.BtnC.lastChange();
      if (now - ca > SLEEP_TIME && now - cb > SLEEP_TIME && now - cc > SLEEP_TIME) {
        lcdController.sleep();
        sleeping = true;
      }
    }
  }

  // 現在日時を取得
  String date = timeManager.getRtcDate();
  String time = timeManager.getRtcTime();

  // 現在日時を表示
  if (sleeping == false && btnmode != 4) {
    if (time != last_lcd_time) {
      lcdController.showCurrentTime(time);
      last_lcd_time = time;
    }
  }

  // タイマーによる OFF/ON 実施
  if (date != last_timer_date) {
    if (time == String(TIMER_TIME)) {
      if (sleeping == true) {
        lcdController.wakeup();
      }

      setButtonMode(0);

      // デバイスを OFF する
      last_timer_date = date;
      lcdController.showMessage("TIMER: Turning off...");
      if (switchBotPlugMini.setPowerStatus(false)) {
        pushLog("TIMER_TURNED_OFF");
      } else {
        pushLog(switchBotPlugMini.getError(), true);
      }

      // 少し待つ
      lcdController.showMessage("TIMER: Waiting...");
      delay(TIMER_INTERVAL);

      // デバイスを ON する
      lcdController.showMessage("TIMER: Turning on...");
      if (switchBotPlugMini.setPowerStatus(true)) {
        pushLog("TIMER_TURNED_ON");
      } else {
        pushLog(switchBotPlugMini.getError(), true);
      }

      setButtonMode(1);

      if (sleeping == true) {
        lcdController.sleep();
      }
    }
  }

  // NTP 時刻同期
  if (date != last_ntp_date) {
    if (time == String(NTP_TIME)) {
      if (sleeping == true) {
        lcdController.wakeup();
      }

      setButtonMode(0);

      // Wi-Fi 接続して NTP 時刻同期
      lcdController.showMessage("Syncing time using NTP...");
      if (timeManager.sync()) {
        pushLog("NTP_TIME_SYNCHRONIZED");
      } else {
        pushLog(timeManager.getError(), true);
      }

      setButtonMode(1);

      if (sleeping == true) {
        lcdController.sleep();
      }
    }
  }

  delay(20);
}
