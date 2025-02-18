#include <Arduino.h>
#include <FastLED.h>

#define LED_TYPE WS2812
#define DATA_PIN 11
#define COLOR_ORDER GRB
#define BTN_1_PIN 5  // New Pin: 9, Old Pin: 5
#define BTN_2_PIN 8
#define BTN_3_PIN 12 // New Pin: 7, Old Pin: 12
#define NUM_LEDS 48 // 48 leds is the max number before the frame rate starts to drop

/****
enum TimerEvents {Idle, Expired};
enum MinMax {Min, Max};

constexpr uint8_t FlashLed {9};
constexpr uint32_t FlashTime[] {1000, 2000};

struct TIMER
{
  uint32_t now;
  uint32_t trigger;
  uint8_t control;
  uint8_t expired ( uint32_t currentMillis)
  {
    uint8_t timerState = currentMillis - now >= trigger and control;
    if (timerState == Expired ) now = currentMillis;
    return (timerState);
  }
};
TIMER blinkMe {0, 100, HIGH};
TIMER flashLed {0, 10, HIGH};
***/

uint8_t iter8 = 0;
float onFrames;
float totalFrames;
float freq = 60;  // 60 Hz
float dutyCycle = .1;
float fps = 500; // Some non-zero number. We will calculate the fps after startup
float usPerFrame = 1; // Initialize with non-zero number
float x = fps / freq;

float freqDelta = 1;
float dutyCycleDelta = .01;
int deltaDirection = 1; // -1 or 1

bool waitBtnHoldRelease = false;
bool waitActionPerformed = false;
bool waiting = false;

uint8_t brightness = 200; // Default brightness = medium
uint16_t brightnessChangeDelay = 200; // 200ms between brightness ticks
uint16_t dutyCycleChangeDelay = 100; // 100ms between duty cycle ticks

CRGB color;
CRGB leds[NUM_LEDS];


/**********************************************************
 * Apply gamma correction to a color value
 **********************************************************/
uint8_t gamma(uint8_t val)
{
  uint16_t temp = uint16_t(pow(double(val) / 255.0, 2.7) * 255.0 + 0.5);
  return temp > 255 ? 255 : (uint8_t)temp;  // Cap at 255
}

/*****************************************************************************************
 * Change brightness of LEDs
 * 
 * @param bool increase - True if increasing brightness, False if decreasing brightness
 * @param uint8_t diff - The brightness difference you are changing the brightness by
 *****************************************************************************************/
void changeBrightness(bool increase, uint8_t diff)
{
  byte minBrightness = 65;
  byte maxBrightness = 255; // Need power control before I can go to 255

  if (increase) {
    brightness = ((brightness + diff) > maxBrightness) ? maxBrightness : qadd8(brightness, diff);
  } else {
    brightness = ((brightness - diff) < minBrightness) ? minBrightness : qsub8(brightness, diff);
  }
  
  Serial.print("Brightness: ");
  Serial.print((float)brightness / (float)maxBrightness * 100);
  Serial.println(F("%"));

  FastLED.setBrightness(gamma(brightness));
  FastLED.show();  

  // Display brightness gauge, adjusted for minimum being greater than zero
  /*
  float brightnessPercent = (float)(brightness - minBrightness) / (float)(maxBrightness - minBrightness);
  if (brightnessPercent < .1) {
    brightnessPercent = .1;  // Set minimum percent to 10%
  }
  displayGauge(CRGB::Red, brightnessPercent);
  delay(200); // Slow down the brightness change
  displayAlert = true;
  displayAlertTimer = millis();*/
}


void calculateFrames() {
  // Calculate the onCycles and offCycles based on the frequency and duty cycle
  //totalFrames = (uint8_t)(x / dutyCycle);  // Calculate the total number of frames required to accommdate the duty cycle
  totalFrames = 1 / dutyCycle;  // Calculate the total number of frames required to accommdate the duty cycle
    // totalFrames = 1 / .2 = 5
  onFrames = totalFrames * dutyCycle; // 10 * .1 = 1

  float strobeFrequency = fps / totalFrames;

  Serial.print(F("Frames ON: "));
  Serial.println(onFrames);
  Serial.print(F("TotalFrames: "));
  Serial.println(totalFrames);
  Serial.print(F("Strobe Frequency: "));
  Serial.print(strobeFrequency);
  Serial.println(F(" Hz"));
}

void setup() {
  // Serial.begin(115200); // This baud rate is unavailable in Arduino IDE Serial Monitor for some reason
  Serial.begin(9600); // Setting this baud rate so Ben can use Arduino IDE Serial Monitor
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  FastLED.setBrightness(gamma(brightness)); // Set brightness to default value

  // Initialize buttons
  pinMode(BTN_1_PIN, INPUT_PULLUP);
  pinMode(BTN_2_PIN, INPUT_PULLUP);
  pinMode(BTN_3_PIN, INPUT_PULLUP);

  calculateFrames();
}

void calcFrameRate() {
  static int frameRateCycles = 1000;
  static uint32_t timeElapsed = 0;
  static uint32_t frameRateTimerStart = micros();

  if (frameRateCycles > 0) {
    frameRateCycles--;
  }
  else {
    frameRateCycles = 1000;  // Reset
    timeElapsed = micros() - frameRateTimerStart;
    fps = 1000000 / (timeElapsed / frameRateCycles);
    usPerFrame = timeElapsed / frameRateCycles;

    frameRateTimerStart = micros();
  }
}

void printFrameRate() {
  Serial.print(fps); Serial.println(" fps");
  Serial.print(usPerFrame); Serial.println("us/f");
}

void changeDutyCycle(int deltaDirection) {
  float minDutyCycle = 0;
  float maxDutyCycle = 1;
  dutyCycle = dutyCycle + (dutyCycleDelta * deltaDirection);

  if (dutyCycle < minDutyCycle)
    dutyCycle = minDutyCycle;
  else if (dutyCycle > maxDutyCycle)
    dutyCycle = maxDutyCycle;

  Serial.print(F("Duty cycle: "));
  Serial.print(dutyCycle*100);
  Serial.println("%");

  calculateFrames();
}


/********************************************************************************************************************************
 * Perform the action for a single or multi-button press
 * @param hold - A boolean value indicating whether or not the button is being held down
 * @param dblClick - A boolean value indicating whether or not it is a double click
 * Example of btnsVal: Pressing btn2 and btn3 = 110(base2) = 6(base10))
 ********************************************************************************************************************************/
void btnAction(uint8_t btnsVal, bool hold = false, bool dblClick = false, bool btnHoldActionComplete = false)
{
  switch (btnsVal) {
    case 1: // Btn 1
      if (hold && dblClick) {
        if (!btnHoldActionComplete) {
          // First Dbl-click hold
        }
        else {
          // Dbl-click hold
        }
      }
      else if (hold) {
        if (!btnHoldActionComplete) {
          // First Hold
        }
        else {
          // Hold
          EVERY_N_MILLISECONDS(dutyCycleChangeDelay) { changeDutyCycle(1); } // Increase
        }
      }
      else if (dblClick) {
        // Dbl-Click
        // nextMode();
      }
      else {
        // Click
        changeDutyCycle(1); // Increase
      }  
      break;


    case 2: // Btn 2
      break;

    case 3: // Btns 1 + 2
      if (hold) {
        EVERY_N_MILLISECONDS(brightnessChangeDelay) { changeBrightness(true, 10); };
      }
      break;

    case 4: // Btn 3
      if (hold && dblClick) {
        if (!btnHoldActionComplete) {
          // First Dbl-click hold
        }
        else {
          // Dbl-click hold
        }
      }
      else if (hold) {
        if (!btnHoldActionComplete) {
          // First Hold
        }
        else {
          // Hold
          EVERY_N_MILLISECONDS(dutyCycleChangeDelay) { changeDutyCycle(-1); } // Decrease
        }
      }
      else if (dblClick) {
        // Dbl-Click
        // nextMode();
      }
      else {
        // Click
        changeDutyCycle(-1); // Decrease
      }
      break;

    case 6: // Btns 2 + 3
      if (hold) {
        EVERY_N_MILLISECONDS(brightnessChangeDelay) { changeBrightness(false, 10); };
      }
      break;
  }

  

  // delaySaveSettings(true);
}



/********************************************************
 * Get the current state of a button
 * @param btnPin
 * @return true if button is pressed, otherwise false
 ********************************************************/
bool getButtonState(uint16_t btnPin) {
  return digitalRead(btnPin) == LOW ? true : false;
}

/************************************************************
 * Get the total value of the all currently pressed buttons
 * @return uint8_t btnsActionVal
 * 
 * Btn 1 = binary 001 = 1
 * Btn 2 = binary 010 = 2
 * Btn 3 = binary 100 = 4
 ************************************************************/
uint8_t getBtnsActionVal() {
  uint8_t btnsActionVal = 0;
  
  if (getButtonState(BTN_1_PIN)) btnsActionVal += 1;
  if (getButtonState(BTN_2_PIN)) btnsActionVal += 2;
  if (getButtonState(BTN_3_PIN)) btnsActionVal += 4;

  return btnsActionVal;
}



/**************************************************************
 * Check for button presses
 **************************************************************/
void checkBtnPress()
{
  static uint8_t btnActionState = 0;  // initialize to starting state
  static uint8_t btnActionVal = 0;
  static uint32_t btnPressStartTime = 0;  // initialize to small meaningless value, just in case
  static bool btnHoldActionComplete = false;  // whether or not btn hold action has been completed
  const uint16_t btnHoldTimeout = 1000; // Number of ms a button must be held before it is considered a button hold
  const uint16_t btnDblClickHoldTimeout = 1500;
  const uint16_t dblClickTimeout = 375;  // Time(ms) to wait on double click before performing single click action
  // uint16_t tripleClickTimeout = 400;  // Time(ms) to wait on triple click before performing double click action
  // uint16_t multiBtnDblClickTimeout = 500; // Time(ms) to wait on a multi-button double click before performing single click action

  uint8_t currentBtnsVal = getBtnsActionVal();
  bool btnPressed = currentBtnsVal > 0 ? true : false;
  if (currentBtnsVal > btnActionVal) {
    btnActionState = 0;
    btnActionVal = currentBtnsVal;
  }
  

  // If waiting for a btn hold to be released, don't evaluate button press
  if (waitBtnHoldRelease) {
    if (btnPressed) {
      return; // Still waiting for btn hold release
    } else {
      waitBtnHoldRelease = false;
    }
  }

  uint32_t now = millis();

  if (btnActionState == 0) {  // waiting for a button to be pressed
    waitActionPerformed = false;
    waiting = false;  // reset waiting flag

    if (btnPressed) {
      btnActionState = 1;  // Serial.println(F("S1")); // DEBUGGING
      btnPressStartTime = now;
    }
  }
  // Button has been pressed
  else if (btnActionState == 1)
  {
    // Check if button has been released
    if (!btnPressed) {
      btnActionState = 2;  // Serial.println(F("S2")); // DEBUGGING
    }
    // Check if btn hold timeout has passed
    else if (now > btnPressStartTime + btnHoldTimeout) {
      // ACTION: Button Hold
      btnAction(btnActionVal, true);  // btn hold action
      btnActionState = 4;  // Serial.println(F("S4")); // DEBUGGING
      btnHoldActionComplete = true;
    }
  }
  // Waiting for the button to be pressed a second time, or a timeout to occur
  else if (btnActionState == 2) {
    // If a single button double click has timed out
    if (now > btnPressStartTime + dblClickTimeout) {
      // ACTION: Click
      btnAction(btnActionVal);
      btnActionState = 0;   // Serial.println(F("S0")); // DEBUGGING
      btnActionVal = 0; // reset btn action value
    }
    else if (btnPressed) {
      btnActionState = 3;   // Serial.println(F("S3")); // DEBUGGING
      btnPressStartTime = now;
    }   
  }
  else if (btnActionState == 3) {
    // Waiting for dbl click hold timeout
    if (btnPressed && (now > btnPressStartTime + btnDblClickHoldTimeout)) {
      // ACTION: Double-click Hold
      btnAction(btnActionVal, true, true);
      btnActionState = 5;   // Serial.println(F("S5")); // DEBUGGING
      btnHoldActionComplete = true;
    }
    else if (!btnPressed) {
      // ACTION: Double-click
      btnAction(btnActionVal, false, true);
      btnActionState = 0;   // Serial.println(F("S0")); // DEBUGGING
      btnActionVal = 0; // reset btn action value
    }
  }
  // Waiting for button hold to be released
  else if (btnActionState == 4) {
    if (!btnPressed) {
      btnActionState = 0;   // Serial.println(F("S0")); // DEBUGGING
      btnActionVal = 0; // reset btn action value

      // reset flag indicating that a button hold action has been performed
      btnHoldActionComplete = false;
      // patternComplete = false; // trying to fix the changeBrightness not changing direction
    }
    else {
      // Btn hold action
      btnAction(btnActionVal, true, false, btnHoldActionComplete);  // btn hold action
      btnHoldActionComplete = true;
    }
  }
  // Waiting for the dbl-click btn hold to be released
  else if (btnActionState == 5) {
    if (!btnPressed) {
      btnActionState = 0;   // Serial.println(F("S0")); // DEBUGGING
      btnActionVal = 0; // reset btn action value

      // reset flag indicating that a btn hold action has been performed
      btnHoldActionComplete = false;
    } else {
      // ACTION: Double-click button hold
      btnAction(btnActionVal, true, true, btnHoldActionComplete);
      btnHoldActionComplete = true;
    }
  }
}

void loop() {

  // Pulse frequency
  // Pulse duration
  // Pulse Intensiry
/*
  uint32_t currentMillis = millis();
  if (blinkMe.expired(currentMillis) == Expired)
  {
    //blinkMe.trigger = ((uint32_t) map(analogRead(PotPin), 0, 1023, FlashTime[Min], FlashTime[Max]));

    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    flashLed.now = currentMillis;
  }
  if (flashLed.expired(currentMillis) == Expired)
  {
    fill_solid(leds, NUM_LEDS, 0);
    FastLED.show();
  }
  */

  if (iter8 < onFrames) {
    color = CRGB::Blue;
  }
  else {
    color = 0;
  }

  iter8++;

  if (iter8 >= totalFrames) {
    iter8 = 0;
  }

  // fill_solid(leds, NUM_LEDS, color);
  // FastLED.show();

  calcFrameRate();
  checkBtnPress();

  EVERY_N_SECONDS(5) {
    printFrameRate();
  }
}