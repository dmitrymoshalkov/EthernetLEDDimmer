#ifndef LightControl_h
#define LightControl_h

// Arduino pin attached to driver pins
#define WHITE_PIN 3
#define WHITE2_PIN 5
#define WHITE3_PIN 6


#define NOCONTROLLER_MODE_PIN 17

// Smooth stepping between the values
#define STEP 1
#define INTERVAL 3 // 10
const int pwmIntervals = 255; //255;
float R; // equation for dimming curve





Bounce debouncer = Bounce();

byte oldButtonVal;

boolean whiteColor=false;


long lastcolorencoderValue=0;
unsigned long previousStatMillis = 0;
byte bNoControllerMode = HIGH;
// Stores the current color settings
byte channels[3] = {WHITE_PIN,WHITE2_PIN,WHITE3_PIN};




// stores dimming level
byte dimming = 100;
byte target_dimming = 100;



// time tracking for updates
unsigned long lastupdate = millis();


// update knobs changed
unsigned long lastKnobsChanged;
boolean bNeedToSendUpdate = false;
Bounce debouncernocontroller = Bounce();


void initLights()
{


//D5-6
//TCCR0B = TCCR0B & B11111000 | B00000001;

//D3
//TCCR2B = TCCR2B & B11111000 | B00000001;
//D9
//TCCR1B = TCCR1B & B11111000 | B00000001;


  pinMode(KNOB_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(KNOB_BUTTON_PIN, HIGH);
  debouncer.attach(KNOB_BUTTON_PIN);
  debouncer.interval(5);
  oldButtonVal = debouncer.read();

    pinMode(NOCONTROLLER_MODE_PIN,INPUT_PULLUP);
    digitalWrite(NOCONTROLLER_MODE_PIN,HIGH);
    debouncernocontroller.attach(NOCONTROLLER_MODE_PIN);
    debouncernocontroller.interval(5);
    bNoControllerMode = debouncernocontroller.read();


        pinMode(WHITE_PIN, OUTPUT);
        pinMode(WHITE2_PIN, OUTPUT);
        pinMode(WHITE3_PIN, OUTPUT);

// set up dimming
  R = (pwmIntervals * log10(2))/(log10(255));




//EEPROM_readAnything(0, values);
//EEPROM_readAnything(4, target_values);

bCurrentBrightness = target_values[0];

lastencoderValue = target_values[0];

}



// this gets called every INTERVAL milliseconds and updates the current pwm levels for all colors
void updateLights() {


  // for each color
  for (int v = 0; v < NUM_CHANNELS; v++) {

    if (values[v] < target_values[v]) {
      values[v] += STEP;
      if (values[v] > target_values[v]) {
        values[v] = target_values[v];
      }
    }

    if (values[v] > target_values[v]) {
      values[v] -= STEP;
      if (values[v] < target_values[v]) {
        values[v] = target_values[v];
      }
    }
  }

  // dimming
  if (dimming < target_dimming) {
    dimming += STEP;
    if (dimming > target_dimming) {
      dimming = target_dimming;
    }
  }
  if (dimming > target_dimming) {
    dimming -= STEP;
    if (dimming < target_dimming) {
      dimming = target_dimming;
    }
  }




		    if (isOnTable) {
		      analogWrite(channels[2], pow (2, (values[2] / R)) - 1);
		    } else {
		      analogWrite(channels[2], 0);
		    }

  // set actual pin values
  for (int i = 0; i < NUM_CHANNELS-1; i++) {
    if (isOn) {
      analogWrite(channels[i], pow (2, (values[i] / R)) - 1);
    } else {
      analogWrite(channels[i], 0);
      	    	//EEPROM_writeAnything(0, values);
      	    	//EEPROM_writeAnything(4, target_values);
    }
  }




}


void checkButtonClick()
{
  debouncer.update();
  byte buttonVal = debouncer.read();
  byte newLevel = 0;
  if (buttonVal != oldButtonVal && buttonVal == LOW) {

  	if (isOn)
  	{
  		isOn = false;
  		isOnTable = false;
  	}
  	else
  	{
      values[0]=map(0,0,100,0,255);
      values[1]=map(0,0,100,0,255);
      values[2]=map(0,0,100,0,255);
  		isOn = true;
  		isOnTable = true;
  	}

        sendDataToMQTT();

      }


  oldButtonVal = buttonVal;
}


void checkRotaryEncoder()
{
  long encoderValue = knob.read();

  if (encoderValue != lastencoderValue)
  {
        #ifdef NDEBUG
  			Serial.print("Encoder: ");
  			Serial.println(encoderValue);
        #endif

  			lastencoderValue=encoderValue;


 for (int i = 0; i < NUM_CHANNELS-1; i++)
{
    target_values[i] = map(lastencoderValue,0,100,0,255);
}


	lastKnobsChanged = millis();
	bNeedToSendUpdate = true;


	//isOn = true;


  }

  if (encoderValue > 100) {
    encoderValue = 100;
    knob.write(100);
  } else if (encoderValue < 13) {
    encoderValue = 13;
    knob.write(13);
  }

  //bCurrentBrightness = encoderValue;

}


void updateBrightness()
{

 unsigned long currentTempMillis = millis();
    if(((currentTempMillis - lastKnobsChanged ) > KNOBUPDATE_TIME) && bNeedToSendUpdate)

	{

			bNeedToSendUpdate = false;

      bCurrentBrightness = lastencoderValue;

        sendDataToMQTT();


	}

}




#endif
