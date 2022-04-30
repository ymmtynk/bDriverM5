bool    crtSw[4];
bool    prvSw[4];

extern bool    port[4];

#define   VOLUME1   0       // J1
#define   VOLUME2   1       // J2
#define   VOLUME3   2       // J3
#define   VOLUME4   3       // J4
#define   VOLUME5   4       // J5
#define   VOLUME6   5       // J6
#define   ATT_ROLL  6
#define   ATT_PITCH 7
#define   ATT_YAW   8
#define   LIMIT_OF_SELECT   VOLUME6

const String   cntSourceStr[] = 
{
  "J1   ", "J2   ",                 // 
  "J3   ", "J4   ",                 // 
  "J5   ", "J6   ",                 // 
  "Roll ", "Pitch", "Yaw  "         // Attitude (6Axis Sensor)
};

const String   cntFlipStr[] = 
{
  "Normal",
  "Flip  "                  // Control Value Flip
};

const String   cntToggleStr[] = 
{
  "Momentary",
  "Toggle   "         // Button push to toggle on/off or momentary on
};

struct    bCoreSetting          // 操作設定のための構造体
{
  int     ptr;                  // 操作設定の捜査対象及び設定モードのオンオフ
  int     source[6];            // Mot[2], Servo[4]の操作量の情報元ID
  bool    flip[6];              // Mot[2], Servo[4]の操作量の反転の有無
  bool    toggle;               // ボタン操作でLEDをトグル切り替えにするとモメンタム操作するか
};


struct bCoreSetting    bStg;    // 操作設定

void    saveSetting(String devName)
{
  // KEYにできるのは１５文字まで
  String    tmp;
  int       len = devName.length();
  if (len > 15) {
    tmp = devName.c_str() + (len-15);
  } else {
    tmp = devName;
  }
  
  Serial.println("> Save Setting Data [KEY: " + tmp + "]");
  preferences.putBytes(tmp.c_str(), &bStg, sizeof(bStg));
  
  return;
}

void    loadSetting(String devName)
{
  // size_t getBytes(const char* key, void * buf, size_t maxLen);
  String    tmp;
  int       len = devName.length();
  if (len > 15) {
    tmp = devName.c_str() + (len-15);
  } else {
    tmp = devName;
  }
  
  Serial.println("> load Setting Data [KEY: " + tmp + "]");
  preferences.getBytes(tmp.c_str(), &bStg, sizeof(bStg));
  
  if (bStg.ptr != -1) {
    // SaveされてるSettingがない場合
    init_setting();
  }
    
  return;
}

void    init_setting()          // 操作設定の初期化
{
  int i;
  
  bStg.ptr = -1;

  bStg.source[0] = 0;   // Mot1 : J1
  bStg.source[1] = 5;   // Mot2 : J6
  bStg.source[2] = 1;   // Srv1 : J2
  bStg.source[3] = 2;   // Srv2 : J3
  bStg.source[4] = 3;   // Srv3 : J4
  bStg.source[5] = 4;   // Srv4 : J5
     
  for (i=0; i<6; i++) {
    bStg.flip[i] = false;
  }
  bStg.toggle = false;
  
  return;
}

void    init_BTN()
{
  init_setting();
  
  return;
}


void    startConnection(int index)
{
  if (bCoreBLE_connect(index)) {
    Serial.println("> Success to Connect.");
            
    // Updte Auto Connect target name string
    // 実際に繋いだときに自動接続が有効なら名前を記憶する
    if (autoConnect == true) {
      setLastConName(bCoreTgtDeviceNameString);
    }
    // 設定を読み出す
    loadSetting(bCoreTgtDeviceNameString);
    
    state = STATE_CONNECTED;
    selection = 0;
  } else {
    Serial.println(">Fail to Connect.");
    state = STATE_READY;
    selection = 0;
  }
  
  return;
}


void    update_BTN()
{
  M5.update();
  
  // ボタン操作
  // メインボタンは状態によって機能が異なる
  if (M5.BtnA.wasReleased()) {
    switch (state) {
      case STATE_READY :
        // 待機状態
        if (selection == 0) {
          // Scanコマンドの実行
          scancounter = NUM_OF_SCAN_COUNT;
          Serial.println("> Scan Start by Btn.");
          state = STATE_SCANNING;
        } else {
          // 選択されたbCoreに接続する
          Serial.println("> Connection Start by Btn.");
          //startConnection(selection-1);
          manualConnectFlag = selection-1;
        }
        break;
      case STATE_SCANNING :
        // Scan中
        if (selection == 0) {
          // Scanの中止
          Serial.println("> Scan Stop by Btn.");
          bCoreBLE_stopScan();
          state = STATE_READY;
        } else {
          // 選択されたbCoreに接続する
          Serial.println("> Connection Start by Btn.");
          bCoreBLE_stopScan();
          //state = STATE_READY;
          //startConnection(selection-1);
          manualConnectFlag = selection-1;
        }
        break;
      case STATE_CONNECTED :
        // 接続中
        if (bStg.ptr != -1) {
          // 操作設定モードから抜ける
          Serial.println(">Setting Mode Off.");
          bStg.ptr = -1;
          // 設定を保存する（ptrを-1にしてから保存するのが大事！）
          saveSetting(bCoreTgtDeviceNameString);
        } else {
          // 操作設定モードにはいる
          Serial.println(">Setting Mode On.");
          bStg.ptr = 0;
        }
        break;
      case STATE_NOT_READY:
      default:
        break;
    }
  }

  // サブボタンは自動接続のOn／Offにのみ使う
  if (M5.BtnB.wasReleased()) {
    if (autoConnect == true) {
      autoConnect = false;
    } else {
      autoConnect = true;

      if (state != STATE_CONNECTED) {
        // 接続してないときは接続対象の名前を取るだけ
        getLastConName();
      } else {
        // 接続中のときは今つないでいる名前を記憶する
        setLastConName(bCoreTgtDeviceNameString);
      }
    }
    // 保存する
    preferences.putBool(PREF_KEY_AUTOCONNECT, autoConnect);
  }
    
  // 4つの外部接続のボタンの処理
  byte  tmp = update_swValue();
  
  if (tmp & 0x01) {
    crtSw[0] = true;
  } else {
    crtSw[0] = false;
  }
  if (tmp & 0x02) {
    crtSw[1] = true;
  } else {
    crtSw[1] = false;
  }
  if (tmp & 0x04) {
    crtSw[2] = true;
  } else {
    crtSw[2] = false;
  }
  if (tmp & 0x08) {
    crtSw[3] = true;
  } else {
    crtSw[3] = false;
  }
  
  // スイッチ１はメニューの上移動に使用する
  if ((prvSw[0] != crtSw[0]) && (crtSw[0] == false)) {
    // SW1 Released
    Serial.println("> SW1 Released.");
    if (state != STATE_CONNECTED) {
      selection--;
      if (selection < 0) {
        selection = numOfbCoreDevice;
      }
    } else {
      if (bStg.ptr != -1) {
        // 設定モードに入ってるとき
        bStg.ptr--;
        if (bStg.ptr < 0) {
          bStg.ptr = 6;
        }
      } else {
        // 設定モードに入ってないとき
        if (bStg.toggle == true) {
          if (port[0] == true) {
            port[0] = false;
          } else {
            port[0] = true;
          }
        //} else {
        //  port[0] = crtSw[0];
        }
      }
    }
  }
  
  // スイッチ2は設定モードのときに操作量の切り替えとLEDのトグル操作切り替えに使用する
  if ((prvSw[1] != crtSw[1]) && (crtSw[1] == false)) {
    // SW2 Released
    Serial.println("> SW2 Released.");
    if (state != STATE_CONNECTED) {
      ;
    } else {
      if (bStg.ptr != -1) {
        // 設定モードに入ってるとき
        if (bStg.ptr == 6) {
          // LEDのトグル操作の切り替え
          if (bStg.toggle == true) {
            bStg.toggle = false;
          } else {
            bStg.toggle = true;
          }
        } else {
          // 操作情報の切り替え（順送りのみ）
          bStg.source[bStg.ptr]++;
          if (bStg.source[bStg.ptr] > LIMIT_OF_SELECT) {
            bStg.source[bStg.ptr] = 0;
          }
        }
      } else {
        // 設定モードに入ってないとき
        if (bStg.toggle == true) {
          if (port[1] == true) {
            port[1] = false;
          } else {
            port[1] = true;
          }
        //} else {
        //  port[1] = crtSw[1];
        }
      }
    }
  }

  // スイッチ3は設定モードのときに操作量のFlipオンオフとLEDのトグル操作切り替えに使用する  
  if ((prvSw[2] != crtSw[2]) && (crtSw[2] == false)) {
    // SW3 Released
    Serial.println("> SW3 Released.");
    if (state != STATE_CONNECTED) {
      ;
    } else {
      if (bStg.ptr != -1) {
        // 設定モードに入ってるとき
        if (bStg.ptr == 6) {
          // LEDのトグル操作の切り替え
          if (bStg.toggle == true) {
            bStg.toggle = false;
          } else {
            bStg.toggle = true;
          }
        } else {
          if (bStg.flip[bStg.ptr] == true) {
            bStg.flip[bStg.ptr] = false;
          } else {
            bStg.flip[bStg.ptr] = true;
          }
        }
      } else {
        // 設定モードに入ってないとき
        if (bStg.toggle == true) {
          if (port[2] == true) {
            port[2] = false;
          } else {
            port[2] = true;
          }
        //} else {
        //  port[2] = crtSw[2];
        }
      }
    }
  }
  
  // スイッチ４はメニューの下移動に使用する
  if ((prvSw[3] != crtSw[3]) && (crtSw[3] == false)) {
    // SW4 Released
    Serial.println("> SW4 Released.");
    if (state != STATE_CONNECTED) {
      selection++;
      if (selection > numOfbCoreDevice) {
        selection = 0;
      }
    } else {
      if (bStg.ptr != -1) {
        // 設定モードに入ってるとき
        bStg.ptr++;
        if (bStg.ptr > 6) {
          bStg.ptr = 0;
        }
      } else {
        // 設定モードに入ってないとき
        if (bStg.toggle == true) {
          if (port[3] == true) {
            port[3] = false;
          } else {
            port[3] = true;
          }
        //} else {
        //  port[3] = crtSw[3];
        }
      }
    }
  }

  if (bStg.toggle != true) {
    port[0] = crtSw[0];
    port[1] = crtSw[1];
    port[2] = crtSw[2];
    port[3] = crtSw[3];
  }
  
  prvSw[0] = crtSw[0];
  prvSw[1] = crtSw[1];
  prvSw[2] = crtSw[2];
  prvSw[3] = crtSw[3];
  
  return;
}
