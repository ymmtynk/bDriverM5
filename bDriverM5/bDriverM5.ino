#include <M5StickC.h>
#include <Preferences.h>

Preferences preferences;

// Preferenceで覚えておくもの
const char  PREF_KEY_AUTOCONNECT[]    = "AutoConnect";        // AutoConnect On/Off           bool
const char  PREF_KEY_LASTCONNAME[]    = "LastConName";        // LastConnected bCore Name     String


#define  STATE_NOT_READY    0               // 起動処理前
#define  STATE_READY        1               // 起動完了、Scan、接続待ち
#define  STATE_SCANNING     2               // Scan中
#define  STATE_CONNECTED    3               // 接続中

int     state;                              // 状態管理
bool    autoConnect;                        // 自動接続オンオフ管理フラグ
String  lastConName;                        // 自動接続する場合の接続相手名称

#define NUM_OF_SCAN_COUNT   100             // Scan時間のカウントダウン x 1/10[sec]
int     scancounter;                        // 

int     selection;                          // UI操作の上下移動の位置

float   batteryVoltage = -1.0f;             // 電源電圧


void    getLastConName()
{
  if (autoConnect == true) {
    lastConName = preferences.getString(PREF_KEY_LASTCONNAME);
    Serial.println(" Load Auto Connect Target [" + lastConName + "]\r\n");   
  }
  
  return;
}

void    setLastConName(String nameStr)
{
  if (autoConnect == true) {
    lastConName = nameStr;
    preferences.putString(PREF_KEY_LASTCONNAME, lastConName);
    Serial.println(" Save Auto Connect Target [" + lastConName + "]\r\n");       
  }
   
  return;
}

void  switch5Vout(bool onoff)
{
  if (onoff == false) {
    Serial.println("5V Out Off");
    Wire1.beginTransmission(0x34);
    Wire1.write(0x12);
    Wire1.endTransmission();
    Wire1.requestFrom(0x34, 1);
    uint8_t state = Wire1.read() & ~(1 << 6);
    Wire1.beginTransmission(0x34);
    Wire1.write(0x12);
    Wire1.write(state);
    Wire1.endTransmission();
  } else{
    Serial.println("5V Out On");
    Wire1.beginTransmission(0x34);
    Wire1.write(0x12);
    Wire1.endTransmission();
    Wire1.requestFrom(0x34, 1);
    uint8_t state = Wire1.read() | (1 << 6);
    Wire1.beginTransmission(0x34);
    Wire1.write(0x12);
    Wire1.write(state);
    Wire1.endTransmission();
  }

  return;
}

void  switchRTC(bool onoff)
{
  if (onoff == false) {
    Serial.println("RTC Off");
    Wire1.beginTransmission(0x34);
    Wire1.write(0x35);
    Wire1.endTransmission();
    Wire1.requestFrom(0x34, 1);
    uint8_t state = Wire1.read() & ~(1 << 7);
    Wire1.beginTransmission(0x34);
    Wire1.write(0x35);
    Wire1.write(state);
    Wire1.endTransmission();
  } else {
    Serial.println("RTC_On");
    Wire1.beginTransmission(0x34);
    Wire1.write(0x35);
    Wire1.endTransmission();
    Wire1.requestFrom(0x34, 1);
    uint8_t state = Wire1.read() | (1 << 7);
    Wire1.beginTransmission(0x34);
    Wire1.write(0x35);
    Wire1.write(state);
    Wire1.endTransmission();  
  }
  
  return;
}


void setup() 
{
  state = STATE_NOT_READY;
  
  M5.begin();

  // Low Power 
  setCpuFrequencyMhz(80);       // 240MHz -> 80MHz
  M5.Axp.ScreenBreath(8);       // LCD Backlight
  switchRTC(false);             // RTC Off
  switch5Vout(false);           // 5V output Off
  
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(1);
  
  Serial.begin(115200);

  Serial.println();
  Serial.println("> bCore BLE Controller");
  
  preferences.begin("bDriver", false);
  autoConnect = preferences.getBool(PREF_KEY_AUTOCONNECT);
  getLastConName();
  
  selection = 0;

  init_BTN();           // ボタン周りの初期化
  update_LCD();         // LCD周りの初期化
  init_LED();           // LED周りの初期化
  init_multiADC();      // 外部接続のADCの初期化

  Serial.println("> Ready");
  state = STATE_READY;
  
  // BLE Scanは処理をブロックするのでボタン操作、表示更新の処理は別コアで行う
  Serial.println("> Start SubTask");
  xTaskCreatePinnedToCore(updateSubTask, "updateSubTask", 4096, NULL, 1, NULL, 0);

  Serial.println("> Init BLE");  
  init_bCoreBLE();  // BLE初期化
  
  return;
}


void loop() 
{
  //Serial.printf("loop(%d)\r\n", counter);
  if (state == STATE_READY) {
    checkAutoConnect();
    checkManualConnect();
    delay(100);    
  } else 
  if (state == STATE_SCANNING) {
    Serial.println("> bCoreBLE_startScan(10)");
    bCoreBLE_startScan(10);         // この命令は処理をブロックする！
    Serial.println("> end of scan.");
    state = STATE_READY;

    checkAutoConnect();
    checkManualConnect();
    delay(100);
  } else 
  if (state == STATE_CONNECTED) {
    byte    burstCommandBuffer[7];    // Mot1, Mot2, LED, Servo1, Servo2, Servo3, Servo4
    burstCommandBuffer[0] = get_pwmValue(0);
    burstCommandBuffer[1] = get_pwmValue(1);
    burstCommandBuffer[2] = get_portValue();
    burstCommandBuffer[3] = get_servoValue(0);
    burstCommandBuffer[4] = get_servoValue(1);
    burstCommandBuffer[5] = get_servoValue(2);
    burstCommandBuffer[6] = get_servoValue(3);    
    bCoreBLE_sendBurstCommand(burstCommandBuffer);
    delay(50);
    
    // Update Batteru Voltage Info
    batteryVoltage = bCoreBLE_getBatteryVoltage();
    delay(50);
  } else {
    delay(100);
  }

  //Serial.printf("%d[sec]\r\n", millis()/1000);
  
  return;
}


void updateSubTask(void * pvParameters) 
{
  while(1) {
    update_multiADC();
    
    update_BTN();          // ボタンの状態更新
    update_LED();          // Status LEDの点滅更新
    update_LCD();          // LCD表示後進
    
    delay(100);
  }
  
  return;
}
