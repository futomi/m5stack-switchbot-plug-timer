/* ----------------------------------------------------------------
  SwitchBotPlugMini.cpp
  - SwitchBot Plug mini (JP) を BLE を通して制御する

  Copyright (c) 2025 Futomi Hatano. All right reserved.
  https://github.com/futomi

  Licensed under the MIT license.
  See LICENSE file in the project root for full license information.
  -------------------------------------------------------------- */
#include "SwitchBotPlugMini.h"


// NOTIFY のコールバック関数と関連のグローバル変数
std::vector<uint8_t> _rdata = {};
bool _received = false;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  for (size_t i = 0; i < length; i++) {
    _rdata.push_back(pData[i]);
  }
  _received = true;
}

// ===============================================================
// SwitchBotPlugMini クラス
// ===============================================================

// ---------------------------------------------------------------
// コンストラクタ
// ---------------------------------------------------------------
SwitchBotPlugMini::SwitchBotPlugMini(char* address) {
  this->_address = address;  // BLE MAC アドレス
  this->_pClient = BLEDevice::createClient();
}

// ---------------------------------------------------------------
// エラーメッセージを取得
// ---------------------------------------------------------------
String SwitchBotPlugMini::getError() {
  return this->_error;
}

// ---------------------------------------------------------------
// 指定の BLE MAC アドレスの SwitchBot プラグミニ（JP）を発見する
// ---------------------------------------------------------------
bool SwitchBotPlugMini::find(BLEAdvertisedDevice& foundDevice) {
  this->_error = "";

  // BLE スキャンの準備
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  // BLE スキャン開始
  BLEScanResults foundDevices = pBLEScan->start(this->_BLE_SCAN_DUR, false);
  int count = foundDevices.getCount();

  if (count == 0) {
    this->_error = "DEVICE_NOT_FOUNDE";
    return false;
  }

  bool found = false;

  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);

    // Manufacturer Data を取得して Company ID をチェック
    if (device.haveManufacturerData() == false) {
      continue;
    }

    std::string mdata = device.getManufacturerData();

    if (mdata.length() < 2 || mdata[0] != 0x69 || mdata[1] != 0x09) {
      continue;
    }

    // Service Data を取得して SwitchBot プラグミニ（JP）かどうかをチェック
    if (device.haveServiceData() == false) {
      continue;
    }

    std::string sdata = device.getServiceData();

    if (sdata.length() == 0 || sdata[0] != 'j') {
      continue;
    }

    // SwitchBot プラグミニ（JP）なら Manufacturer Data は 14 バイトのはず
    if (mdata.length() != 14) {
      continue;
    }

    //Serial.println(device.toString().c_str());

    // BLE アドレスを取得
    BLEAddress addr = device.getAddress();
    if (addr.toString() == this->_address) {
      foundDevice = device;
      found = true;
      break;
    }
  }

  pBLEScan->clearResults();

  if (found == true) {
    this->_error = "";
  } else {
    this->_error = "DEVICE_NOT_FOUNDE";
  }

  return found;
}

// ---------------------------------------------------------------
// SwitchBot プラグミニ（JP）に BLE 接続する
// ---------------------------------------------------------------
bool SwitchBotPlugMini::connect() {
  this->_connected = false;
  this->_error = "";

  // BLE 接続
  //this->_pClient = BLEDevice::createClient();
  BLEAddress bleAddress(this->_address);
  this->_pClient->connect(bleAddress);

  if (!this->_pClient->isConnected()) {
    this->_error = "CONNECT_FAILED";
    return false;
  }

  // Service, Characteristics を準備する
  bool prepared = this->_prepareServiceAndCharacteristics();

  if (prepared == true) {
    this->_connected = true;
    return true;
  } else {
    return false;
  }
}

// Service, Characteristics を準備する
bool SwitchBotPlugMini::_prepareServiceAndCharacteristics() {
  // Service を取得
  this->_pService = this->_pClient->getService(this->_SERVICE_UUID);

  if (this->_pService == nullptr) {
    this->_error = "SERVICE_NOT_FOUND";
    this->disconnect();
    return false;
  }

  // データ受信用の Characteristic を取得
  this->_pCharRx = this->_pService->getCharacteristic(this->_CHAR_RX_UUID);

  if (this->_pCharRx == nullptr) {
    this->_error = "CHAR_RX_NOT_FOUND";
    this->disconnect();
    return false;
  }

  // データ送信用の Characteristic を取得
  this->_pCharTx = this->_pService->getCharacteristic(this->_CHAR_TX_UUID);

  if (this->_pCharTx == nullptr) {
    this->_error = "CHAR_TX_NOT_FOUND";
    this->disconnect();
    return false;
  }

  // データ送信用の Characteristic が NOTIFY をサポートしているかをチェック
  if (this->_pCharTx->canNotify() == false) {
    this->_error = "CHAR_TX_NOT_SUPPORT_NOTIFY";
    this->disconnect();
    return false;
  }

  // NOTIFY のコールバックをセット
  this->_pCharTx->registerForNotify(notifyCallback);

  return true;
}

// ---------------------------------------------------------------
// 電源状態を取得する
// ---------------------------------------------------------------
bool SwitchBotPlugMini::getPowerStatus(bool& status) {
  uint8_t reqData[] = { 0x57, 0x0f, 0x51, 0x01 };

  if (!this->_request(reqData, 4)) {
    return false;
  }

  if (!this->_checkResponse()) {
    return false;
  }

  if (_rdata[1] == 0x00) {
    status = false;
  } else if (_rdata[1] == 0x80) {
    status = true;
  } else {
    this->_error = "INVALID_RESPONSE";
    return false;
  }

  return true;
}

// SwitchBot プラグミニ（JP）からのレスポンスの妥当性をチェック
bool SwitchBotPlugMini::_checkResponse() {
  std::size_t len = _rdata.size();
  if (len != 2) {
    this->_error = "INVALID_RESPONSE";
    return false;
  }

  if (_rdata[0] != 0x01) {
    this->_error = "INVALID_RESPONSE";
    return false;
  }

  return true;
}

// SwitchBot プラグミニ（JP）にリクエストを送ってレスポンスを得る
bool SwitchBotPlugMini::_request(uint8_t* reqData, uint8_t len) {
  this->_error = "";

  // BLE 接続がなければ接続する
  bool cstatus = this->_connected;
  if (!cstatus) {
    if (!this->connect()) {
      return false;
    }
  }

  _received = false;
  _rdata.clear();

  this->_pCharRx->writeValue(reqData, len, false);

  while (_received == false) {
    delay(50);
  }

  // もともと BLE 接続していなかったなら切断する
  if (!cstatus) {
    this->disconnect();
  }

  return true;
}

// ---------------------------------------------------------------
// 電源状態をセットする
// ---------------------------------------------------------------
bool SwitchBotPlugMini::setPowerStatus(bool status) {
  uint8_t reqData[] = { 0x57, 0x0f, 0x50, 0x01, 0x01, 0x00 };

  if (status == true) {
    reqData[5] = 0x80;
  }

  if (!this->_request(reqData, 6)) {
    return false;
  }

  if (!this->_checkResponse()) {
    return false;
  }

  bool rstatus;
  if (_rdata[1] == 0x00) {
    rstatus = false;
  } else if (_rdata[1] == 0x80) {
    rstatus = true;
  } else {
    this->_error = "INVALID_RESPONSE";
    return false;
  }

  if (rstatus != status) {
    this->_error = "OPERATION_FAILED";
    return false;
  }

  return true;
}

// ---------------------------------------------------------------
// 電源状態を反転する
// ---------------------------------------------------------------
bool SwitchBotPlugMini::togglePowerStatus(bool& status) {
  uint8_t reqData[] = { 0x57, 0x0f, 0x50, 0x01, 0x02, 0x80 };

  if (!this->_request(reqData, 6)) {
    return false;
  }

  if (!this->_checkResponse()) {
    return false;
  }

  if (_rdata[1] == 0x00) {
    status = false;
  } else if (_rdata[1] == 0x80) {
    status = true;
  } else {
    this->_error = "INVALID_RESPONSE";
    return false;
  }

  return true;
}

// ---------------------------------------------------------------
// BLE 接続を切断する
// ---------------------------------------------------------------
bool SwitchBotPlugMini::disconnect() {
  this->_error = "";
  this->_pClient->disconnect();
  this->_connected = false;
  return true;
}
