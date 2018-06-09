#include "FastLED.h"
#define NUM_LEDS 8

CRGB leds[NUM_LEDS];

void setup()
{
	FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
}

int i = 0;
void loop()
{
        for(int dot = 0; dot < NUM_LEDS; dot++) { 
            leds[dot] = CRGB::Red;
            FastLED.show();
            // clear this led for the next time around the loop
            leds[dot] = CRGB::Black;
            delay(130);
        }
}

void hsvTest()
{ 
	static uint8_t hue = 0;
	static uint8_t sat = 0;
	static uint8_t val = 50;
  	FastLED.showColor(CHSV(hue++, 255,255)); 
  	delay(10);
}

void otherTest()
{
	leds[0] = CRGB::Red;
	FastLED.show();
	delay(30);
	leds[0] = CRGB::Black;
	FastLED.show();
	delay(300);

}