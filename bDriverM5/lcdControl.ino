
void    update_LCD()
{
  // STATE_NOT_READY    Title
  // STATE_READY        Autoconnect On/Off
  // STATE_SCANNING     Scanning Info
  // STATE_SCANED       List
  // STATE_CONNECTED    Sending Data and Battery Info

  // No Connection/Connected
  // Scaning(x.x)/ Battery
  // info
  static int laststate;

  if (laststate != state) {
    M5.Lcd.fillScreen(BLACK);
  }
  M5.Lcd.setCursor(0, 0);
  
  if (bCoreTgtDeviceNameString == NULL) {
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println(" No Connection         ");  
  } else {
    if (state == STATE_CONNECTED) {
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.println(" " + bCoreTgtDeviceNameString);
    } else {
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.println(" " + bCoreTgtDeviceNameString);
    }
  }

  float vbat = M5.Axp.GetBatVoltage();
  int   leve = M5.Axp.GetWarningLeve();       // 1: Low

  M5.Lcd.setCursor(118, 0);
  if (leve != 1) {
    M5.Lcd.setTextColor(WHITE, BLACK);
  } else {
    M5.Lcd.setTextColor(RED, BLACK);
  }
  M5.Lcd.printf("%1.2f[V]", vbat); 
  

  M5.Lcd.setCursor(124, 12);
  if (autoConnect == true) {
    M5.Lcd.setTextColor(BLACK, YELLOW);
    M5.Lcd.println("AT On "); 
  } else {
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println("AT Off"); 
  }

  M5.Lcd.setCursor(0, 12);

  int i;
  
  switch (state) {
    case STATE_READY:
      if (selection == 0) {
        M5.Lcd.setTextColor(BLACK, WHITE);
      } else {
        M5.Lcd.setTextColor(WHITE, BLACK);
      }
      //M5.Lcd.println("              ");
      M5.Lcd.println(" Start Scan!  ");
      for (i=0; i<numOfbCoreDevice; i++) {
        if (selection == i+1) {
          M5.Lcd.setTextColor(BLACK, WHITE);
        } else {
          M5.Lcd.setTextColor(WHITE, BLACK);
        }
        M5.Lcd.printf(" %d:", i);
        M5.Lcd.println(bCoreDeviceNameString[i]);
      }
      for (i=numOfbCoreDevice; i<NUM_OF_MAX_DEVICE; i++) {
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.println("                     ");
      }
      break;
    case STATE_SCANNING:
      scancounter--;
      if (selection == 0) {
        M5.Lcd.setTextColor(BLACK, RED);
      } else {
        M5.Lcd.setTextColor(RED, BLACK);
      }
      M5.Lcd.printf(" Scanning(%1.1f)\r\n", scancounter/10.0f);
      for (i=0; i<numOfbCoreDevice; i++) {
        if (selection == i+1) {
          M5.Lcd.setTextColor(BLACK, WHITE);
        } else {
          M5.Lcd.setTextColor(WHITE, BLACK);
        }
        M5.Lcd.printf(" %d:", i);
        M5.Lcd.println(bCoreDeviceNameString[i]);
      }
      for (i=numOfbCoreDevice; i<NUM_OF_MAX_DEVICE; i++) {
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.println("                     ");
      }
      break;
    case STATE_CONNECTED:
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.printf(" Battery : %2.2f[V]  \r\n", batteryVoltage);
      if (bStg.ptr == 0) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" Mt1:%3d  %s %s \r\n", get_pwmValue(0)  , cntSourceStr[bStg.source[0]].c_str(), cntFlipStr[bStg.flip[0]].c_str());
      if (bStg.ptr == 1) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" Mt2:%3d  %s %s \r\n", get_pwmValue(1)  , cntSourceStr[bStg.source[1]].c_str(), cntFlipStr[bStg.flip[1]].c_str());
      if (bStg.ptr == 2) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" Sv1:%3d  %s %s \r\n", get_servoValue(0), cntSourceStr[bStg.source[2]].c_str(), cntFlipStr[bStg.flip[2]].c_str());
      if (bStg.ptr == 3) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" Sv2:%3d  %s %s \r\n", get_servoValue(1), cntSourceStr[bStg.source[3]].c_str(), cntFlipStr[bStg.flip[3]].c_str());
      if (bStg.ptr == 4) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" Sv3:%3d  %s %s \r\n", get_servoValue(2), cntSourceStr[bStg.source[4]].c_str(), cntFlipStr[bStg.flip[4]].c_str());
      if (bStg.ptr == 5) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" Sv4:%3d  %s %s \r\n", get_servoValue(3), cntSourceStr[bStg.source[5]].c_str(), cntFlipStr[bStg.flip[5]].c_str());
      if (bStg.ptr == 6) { M5.Lcd.setTextColor(BLACK, WHITE); } else { M5.Lcd.setTextColor(WHITE, BLACK); }
      M5.Lcd.printf(" SW:%d%d%d%d  %s", crtSw[0], crtSw[1], crtSw[2], crtSw[3], cntToggleStr[bStg.toggle].c_str());
      break;
    case STATE_NOT_READY:
    default:
      break;
  }

  laststate = state;
  
  return;
}



