
int     counter;
bool    ledControl;

void    init_LED()
{
  counter = 0;
  ledControl = true;
  
  pinMode(GPIO_NUM_10, OUTPUT);
  digitalWrite(GPIO_NUM_10, ledControl);

  return;
}

void    toggle_LED()
{
  if (ledControl) {
    ledControl = false;
  } else {
    ledControl = true;
  }
  
  digitalWrite(GPIO_NUM_10, ledControl);
    
  return;
}

void    control_LED(bool onoff)
{
  ledControl = onoff;
  
  digitalWrite(GPIO_NUM_10, ledControl);
    
  return;
}

void    update_LED()
{
  counter++;
  if (counter >= 10) {
    counter = 0;
  }

  // STATE_NOT_READY    Off
  // STATE_READY        Blink Slow
  // STATE_SCANNING     Blink 2times
  // STATE_CONNECTED    Blink Fast

  switch (state) {
    case STATE_READY:
      if ((counter == 0) || (counter == 5)) {
        toggle_LED();
      }
      break;
    case STATE_SCANNING:
      if (counter < 4) {
        toggle_LED();
      }
      break;
    case STATE_CONNECTED:
      toggle_LED();
      break;
    case STATE_NOT_READY:
    default:
      control_LED(false);
      break;
  }
  
  return;
}



