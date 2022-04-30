#include "BLEDevice.h"


static BLEUUID      bCoreServiceUUID(               "389caaf0-843f-4d3b-959d-c954cce14655");
static BLEUUID      bCoreCharUUID_GetBatteryVoltage("389CAAF1-843F-4d3b-959D-C954CCE14655");
static BLEUUID      bCoreCharUUID_SetMotorPWM(      "389CAAF2-843F-4d3b-959D-C954CCE14655");
static BLEUUID      bCoreCharUUID_SetPortOut(       "389CAAF3-843F-4d3b-959D-C954CCE14655");
static BLEUUID      bCoreCharUUID_SetServoPosition( "389CAAF4-843F-4d3b-959D-C954CCE14655");
static BLEUUID      bCoreCharUUID_BurstCommand(     "389CAAF5-843F-4d3b-959D-C954CCE14655");
static BLEUUID      bCoreCharUUID_GetFunctions(     "389CAAFF-843F-4d3b-959D-C954CCE14655");


static BLEClient*                   bCoreClient;

static BLERemoteService*            bCoreService;
static BLERemoteCharacteristic*     bCoreCharacteristicGetBatteryVoltage;
static BLERemoteCharacteristic*     bCoreCharacteristicSetMotorPWM;
static BLERemoteCharacteristic*     bCoreCharacteristicSetPortOut;
static BLERemoteCharacteristic*     bCoreCharacteristicSetServoPosition;
static BLERemoteCharacteristic*     bCoreCharacteristicBurstCommand;
static BLERemoteCharacteristic*     bCoreCharacteristicGetFunctions;

static BLEScan* bCoreBLEScan;
static BLEAdvertisedDevice*         bCoreTgtDevAddress;
static String                       bCoreTgtDeviceNameString;


#define     NUM_OF_MAX_DEVICE   5
static int                          numOfbCoreDevice;
static BLEAdvertisedDevice*         bCoreDeviceAddress[NUM_OF_MAX_DEVICE];
static String                       bCoreDeviceNameString[NUM_OF_MAX_DEVICE];
static int                          autoConnectFlag;
static int                          manualConnectFlag;


class bleClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient* pclient) 
  {
    Serial.println("BLEClientCallbacks::onConnect()");
   
    return;
  }

  void onDisconnect(BLEClient* pclient) 
  {
    bCoreBLE_cleanList();       // ScanListの掃除
    selection = 0;
    init_setting();
    
    state = STATE_READY;
    Serial.println("BLEClientCallbacks::onDisconnect()");

    //delete bCoreClient;
    bCoreClient->disconnect();

    // 色々めんどくさいので一回接続切れたらリブートさせる
    Serial.println("ESP.restart()");    // コレをコメントアウトするとリブートしなくなる
    ESP.restart();
    
    return;
  }
};


bool bCoreBLE_connect(int index) 
{
  bCoreTgtDevAddress = bCoreDeviceAddress[index];
  bCoreTgtDeviceNameString = bCoreDeviceNameString[index];
    
  Serial.print("Forming a connection to ");
  Serial.println(bCoreTgtDevAddress->getAddress().toString().c_str());
  Serial.print("Device Name : ");
  Serial.println(bCoreTgtDeviceNameString);

  bCoreClient  = BLEDevice::createClient();
  bCoreClient->setClientCallbacks(new bleClientCallback());

  // Connect to the remove BLE Server.
  bCoreClient->connect(bCoreTgtDevAddress);
  
  // Obtain a reference to the service we are after in the remote BLE server.
  bCoreService = bCoreClient->getService(bCoreServiceUUID);
  if (bCoreService == NULL) {
    Serial.print("Failed to find bCore Service UUID: ");
    Serial.println(bCoreServiceUUID.toString().c_str());
    bCoreClient->disconnect();
  
    return false;
  }
  
  Serial.println(" - bCore service found.");

  // Obtain a reference to the characteristic in the bCore service.
  bCoreCharacteristicGetBatteryVoltage = bCoreService->getCharacteristic(bCoreCharUUID_GetBatteryVoltage);
  Serial.printf(" - bCoreCharacteristicGetBatteryVoltage: 0x%x\r\n", (unsigned int)bCoreCharacteristicGetBatteryVoltage);
  if (bCoreCharacteristicGetBatteryVoltage == NULL) {
    Serial.print(" - Failed to find GetBatteryVoltage UUID: ");
    Serial.println(bCoreCharUUID_GetBatteryVoltage.toString().c_str());
    bCoreClient->disconnect();

    return false;
  }
  bCoreCharacteristicSetMotorPWM = bCoreService->getCharacteristic(bCoreCharUUID_SetMotorPWM);
  Serial.printf(" - bCoreCharacteristicSetMotorPWM: 0x%x\r\n", (unsigned int)bCoreCharacteristicSetMotorPWM);
  if (bCoreCharacteristicSetMotorPWM == NULL) {
    Serial.print("- Failed to find SetMotorPWM UUID: ");
    Serial.println(bCoreCharUUID_SetMotorPWM.toString().c_str());
    bCoreClient->disconnect();

    return false;
  }
  bCoreCharacteristicSetPortOut = bCoreService->getCharacteristic(bCoreCharUUID_SetPortOut);
  Serial.printf(" - bCoreCharacteristicSetPortOut: 0x%x\r\n", (unsigned int)bCoreCharacteristicSetPortOut);
  if (bCoreCharacteristicSetPortOut == NULL) {
    Serial.print(" - Failed to find SetPortOut UUID: ");
    Serial.println(bCoreCharUUID_SetPortOut.toString().c_str());
    bCoreClient->disconnect();

    return false;
  }
  bCoreCharacteristicSetServoPosition = bCoreService->getCharacteristic(bCoreCharUUID_SetServoPosition);
  Serial.printf(" - bCoreCharacteristicSetServoPosition: 0x%x\r\n", (unsigned int)bCoreCharacteristicSetServoPosition);
  if (bCoreCharacteristicSetServoPosition == NULL) {
    Serial.print("- Failed to find SetServoPosition UUID: ");
    Serial.println(bCoreCharUUID_SetServoPosition.toString().c_str());
    bCoreClient->disconnect();

    return false;
  }
  bCoreCharacteristicBurstCommand = bCoreService->getCharacteristic(bCoreCharUUID_BurstCommand);
  Serial.printf(" - bCoreCharacteristicBurstCommand: 0x%x\r\n", (unsigned int)bCoreCharacteristicBurstCommand);
  if (bCoreCharacteristicBurstCommand == NULL) {
    Serial.print(" - Failed to find BurstCommand UUID: ");
    Serial.println(bCoreCharUUID_BurstCommand.toString().c_str());
    bCoreClient->disconnect();

    return false;
  }
  bCoreCharacteristicGetFunctions = bCoreService->getCharacteristic(bCoreCharUUID_GetFunctions);
  Serial.printf(" - bCoreCharacteristicGetFunctions: 0x%x\r\n", (unsigned int)bCoreCharacteristicGetFunctions);
  if (bCoreCharacteristicGetFunctions == NULL) {
    Serial.print(" - Failed to find GetFunctions UUID: ");
    Serial.println(bCoreCharUUID_GetFunctions.toString().c_str());
    bCoreClient->disconnect();

    return false;
  }

  // Read the value of the characteristic.
  if (bCoreCharacteristicGetFunctions->canRead()) {
    std::string value = bCoreCharacteristicGetFunctions->readValue();
    word tmp = ((word)value[1]) * 256 + ((word)value[0]);
    Serial.printf(" - GetFunctions value was: 0x%x\r\n", tmp);            // bCore1:0x1f03, bCore2,3:0xff03
  }
  
  return    true;
}


//
// Scan for BLE servers and find the first one that advertises the service we are looking for.
//
class bleAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
  //Called for each advertising BLE server.
  
  void onResult(BLEAdvertisedDevice advertisedDevice) 
  {
    Serial.print("BLE Adv-Dev found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (   advertisedDevice.haveServiceUUID()
        && advertisedDevice.isAdvertisingService(bCoreServiceUUID)
        ) {
      
      Serial.print("> bCore Device found. (");
      bCoreDeviceAddress[numOfbCoreDevice] = new BLEAdvertisedDevice(advertisedDevice);
      bCoreDeviceNameString[numOfbCoreDevice] = String(advertisedDevice.getName().c_str());
      Serial.printf(" %d: ", numOfbCoreDevice);
      Serial.print(bCoreDeviceNameString[numOfbCoreDevice]);
      Serial.println(")");
      
      // 自動接続の処理
      if (autoConnect == true) {
        if (lastConName == bCoreDeviceNameString[numOfbCoreDevice]) {
          // bCoreに接続するフラグを立てる
          Serial.println("> Connection Start by Auto Connect.");
          autoConnectFlag = numOfbCoreDevice;
          bCoreBLE_stopScan();
        }
      }
      
      // たくさん見つかっても表示できないので上限値で止める
      numOfbCoreDevice++;
      if (numOfbCoreDevice >= NUM_OF_MAX_DEVICE) {
        Serial.println("> Scan Stop by Limit.");
        bCoreBLE_stopScan();
      }
    }
    
    return;
  }
};




void    init_bCoreBLE()
{
  bCoreBLE_cleanList();
  
  BLEDevice::init("");
    
  bCoreBLEScan = BLEDevice::getScan();
  bCoreBLEScan->setAdvertisedDeviceCallbacks(new bleAdvertisedDeviceCallbacks());
//  bCoreBLEScan->setInterval(1349);          // 1349: void  setInterval(uint16_t intervalMSecs);
//  bCoreBLEScan->setWindow(449);             //  449: void  setWindow(uint16_t windowMSecs);
  bCoreBLEScan->setActiveScan(true);

  return;
}



void    bCoreBLE_sendBurstCommand(byte* command)
{
  if (bCoreCharacteristicBurstCommand != NULL) {
    bCoreCharacteristicBurstCommand->writeValue(command, 7);
  }
  
  return;
}

float   bCoreBLE_getBatteryVoltage()
{
  float  result = -1.0f;
  
  if (bCoreCharacteristicGetBatteryVoltage->canRead()) {
    std::string value = bCoreCharacteristicGetBatteryVoltage->readValue();
    result = ((float)((word)value[1] * 256 + (word)value[0]))/1000.0;
  }
  
  return  result;
}

void    bCoreBLE_cleanList()
{
  int i;
  
  numOfbCoreDevice = 0;
  autoConnectFlag = -1;
  manualConnectFlag = -1;
  
  for (i=0; i<NUM_OF_MAX_DEVICE; i++) {
    if (bCoreDeviceAddress[i] != NULL) {
        delete bCoreDeviceAddress[i];
    }
    bCoreDeviceAddress[i] = NULL;
    bCoreDeviceNameString[i] = "";
  }

  bCoreTgtDevAddress = NULL;
  bCoreTgtDeviceNameString = "";
  
  return;
}

void    bCoreBLE_startScan(uint32_t scantime)
{
  bCoreBLE_cleanList();
  bCoreBLEScan->start(scantime, false);   // 0, false : BLEScanResults start(uint32_t duration, bool is_continue = false);
    
  return;
}

void    bCoreBLE_stopScan()
{
  bCoreBLEScan->stop();
  
  return;
}

void    bCoreBLE_disconnect()
{
  //
  bCoreClient->disconnect();
  
  return;
}


void    checkAutoConnect()
{
  // 自動接続
  if (autoConnectFlag != -1) {
    // 選択されたbCoreに接続する
    Serial.println("> Connection Start by Auto.");
    startConnection(autoConnectFlag);
    autoConnectFlag = -1;
  }
  
  return;
}

void    checkManualConnect()
{
  // 手動接続
  if (manualConnectFlag != -1) {
    // 選択されたbCoreに接続する
    Serial.println("> Connection Start by Manual.");
    startConnection(manualConnectFlag);
    manualConnectFlag = -1;
  }
  
  return;
}
