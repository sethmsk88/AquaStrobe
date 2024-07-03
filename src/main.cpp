#include <Arduino.h>
#include <FastLED.h>

#define LED_TYPE WS2812
#define DATA_PIN 11
#define COLOR_ORDER GRB
#define NUM_LEDS 50

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

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
}

void printFrameRate() {
  static int frameRateCycles = 1000;
  static uint32_t timeElapsed = 0;
  static uint32_t frameRateTimerStart = millis();
  int fps;

  if (frameRateCycles > 0) {
    frameRateCycles--;
  }
  else {
    frameRateCycles = 1000;  // Reset
    timeElapsed = millis() - frameRateTimerStart;
    fps = 1000 / (timeElapsed / frameRateCycles);

    Serial.print(fps); Serial.println(" fps");
    frameRateTimerStart = millis();
  }
}

void loop() {

  // Pulse frequency
  // Pulse duration
  // Pulse Intensiry

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

  /*
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
*/
  printFrameRate();
}