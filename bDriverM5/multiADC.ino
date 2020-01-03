// NCD9830 https://www.onsemi.com/pub/Collateral/NCD9830-D.PDF
#include <Wire.h>

#define     I2C_SCK     26
#define     I2C_SDA      0
#define     NCD9830_ADDRESS     0x48

byte    i2cAdcBuff[8];


bool    port[4];
float   rollAtt, pitchAtt, yawAtt; 

extern struct bCoreSetting    bStg;


void  init_multiADC()
{
  Wire.begin(I2C_SDA, I2C_SCK);
  
  return;
}

bool  get_I2C_ADCResult(int ch, byte* adcdata)
{
  bool result = false;
  byte command;

  int num;
  
  switch (ch) {
    case 0 :  command = 0x84; break;
    case 1 :  command = 0xc4; break;
    case 2 :  command = 0x94; break;
    case 3 :  command = 0xd4; break;
    case 4 :  command = 0xa4; break;
    case 5 :  command = 0xe4; break;
    case 6 :  command = 0xb4; break;
    case 7 :  command = 0xf4; break;
  }
  Wire.beginTransmission(NCD9830_ADDRESS);
  Wire.write(&command, 1);
    
  if (Wire.endTransmission(false) != 0) {
    return  result;
  }

  if (Wire.requestFrom(NCD9830_ADDRESS, 1) != 1) {
    return  result;
  }
    
  num = 0;
  while (Wire.available()) {
    *adcdata = Wire.read();
    num++;
  }

  if (num == 1) {
    ;
  } else {
        return  result;        
  }
  
  result = true;
  
  return  result;
}

void    update_multiADC()
{
  int i;

  for (i = 0; i<8; i++) {
    get_I2C_ADCResult(i, &(i2cAdcBuff[i]));
  }
  
  return;
}

byte     get_adc_value(int ch)
//byte     get_I2C_adc_value(int ch)
{
  return  i2cAdcBuff[ch];
}


byte    get_control_value(int sourceid)
{
  byte  result;

  switch (sourceid) {
    case 0:
      result = get_adc_value(0);
      break;
    case 1:
      result = get_adc_value(1);
      break;
    case 2:
      result = get_adc_value(2);
      break;
    case 3:
      result = get_adc_value(5);
      break;
    case 4:
      result = get_adc_value(6);
      break;
    case 5:     
      result = get_adc_value(7);
      break;
    case 6:     // Roll
      //result = (byte)(128.0 - rollAtt*(128.0f/180.0f));
      result = 128;
      break;
    case 7:     // Pitch
      //result = (byte)(128.0 - pitchAtt*(128.0f/180.0f));;
      result = 128;
      break;
    case 8:     // Yaw
      //result = (byte)(128.0 - yawAtt*(128.0f/180.0f));;
      result = 128;
      break;
    default:
      result = 128;
      break;
  }
  
  return  result;
}


byte    get_servoValue(int ch)
{
  byte result = 0;
  if (ch == 0) {
    result = get_control_value(bStg.source[2]);
  } else 
  if (ch == 1) {
    result = get_control_value(bStg.source[3]);
  } else 
  if (ch == 2) {
    result = get_control_value(bStg.source[4]);
  } else 
  if (ch == 3) {
    result = get_control_value(bStg.source[5]);
  }

  if (bStg.flip[ch+2]) {
    result = 255 - result;
  }
  
  return  result;
}

byte    get_pwmValue(int ch)
{
  byte result = 0;
  if (ch == 0) {
    result = get_control_value(bStg.source[0]);
  } else 
  if (ch == 1) {
    result = get_control_value(bStg.source[1]);
  }
  
  if (bStg.flip[ch]) {
    result = 255 - result;
  }
  
  return  result;
}

byte    update_swValue()
{
    byte result = 0x00;
    byte adc3 = get_adc_value(3);
    byte adc4 = get_adc_value(4);

    if (adc3 > 213) {
    } else 
    if (adc3 > 149) {
      result += 0x02;
    } else 
    if (adc3 > 115) {
      result += 0x01;
    } else {
      result += 0x03;
    }
    if (adc4 > 213) {
    } else 
    if (adc4 > 149) {
      result += 0x08;
    } else 
    if (adc4 > 115) {
      result += 0x04;
    } else {
      result += 0x0C;
    }
    
    return  result;
}

byte    get_portValue()
{
  byte result = 0x00;
  
  if (port[0] == true) {
    result += 0x01;
  }
  if (port[1] == true) {
    result += 0x02;
  }
  if (port[2] == true) {
    result += 0x04;
  }
  if (port[3] == true) {
    result += 0x08;
  }
  
  return    result;
}

