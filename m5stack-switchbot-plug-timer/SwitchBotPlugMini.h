/* ----------------------------------------------------------------
  SwitchBotPlugMini.h
  - SwitchBot Plug mini (JP) を BLE を通して制御する

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#ifndef SwitchBotPlugMini_h
#define SwitchBotPlugMini_h
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// ---------------------------------------------------------------
// SwitchBotPlugMini クラス
// ---------------------------------------------------------------
class SwitchBotPlugMini {
private:
  // BLE スキャンの時間 (秒)
  const uint8_t _BLE_SCAN_DUR = 3;

  // SwitchBot Plug Mini の BLE の Service と Characteristics の UUID
  const char* _SERVICE_UUID = "cba20d00-224d-11e6-9fb8-0002a5d5c51b";
  const char* _CHAR_RX_UUID = "cba20002-224d-11e6-9fb8-0002a5d5c51b";
  const char* _CHAR_TX_UUID = "cba20003-224d-11e6-9fb8-0002a5d5c51b";

  // BLE MAC アドレス
  char* _address;

  BLEClient* _pClient;
  BLERemoteService* _pService;
  BLERemoteCharacteristic* _pCharRx;
  BLERemoteCharacteristic* _pCharTx;

  bool _connected;
  String _error;

private:
  // Service, Characteristics を準備する
  bool _prepareServiceAndCharacteristics();

  // SwitchBot プラグミニ（JP）にリクエストを送ってレスポンスを得る
  bool _request(uint8_t* reqData, uint8_t len);

  // SwitchBot プラグミニ（JP）からのレスポンスの妥当性をチェック
  bool _checkResponse();

public:
  // コンストラクタ
  SwitchBotPlugMini(char* addr);

  // エラーメッセージを取得
  String getError();

  // 指定の BLE MAC アドレスの SwitchBot プラグミニ（JP）を発見する
  bool find(BLEAdvertisedDevice& foundDevice);

  // SwitchBot プラグミニ（JP）に BLE 接続する
  bool connect();

  // 電源状態を取得する
  bool getPowerStatus(bool& status);

  // 電源状態をセットする
  bool setPowerStatus(bool status);

  // 電源状態を反転する
  bool togglePowerStatus(bool& status);

  // BLE 接続を切断する
  bool disconnect();
};

#endif
