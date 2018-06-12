#include "FastLED.h"

FASTLED_USING_NAMESPACE

// Shamelessly derived from the DemoReel100 FastLED example

// things i want to do
// have a bunch of different lightning patterns that i can cycle throught with the SimplePatternList object below
// patterns should
// 	vary brightness
//	vary number of flashes
// 	how flashes travel through cloud (random?  nearest neighbors favored?)
//  fade flashes (what's the fade command here)
// 	trigger sound effects eventally.
//  	simulate distance to storm by varying volume, flash->sound delay, brightness of flashes (maybe "bottom-only" or "top-only" might look cool as a "distance" cue)? )
//		sound effects volume based on brightness?  maybe that would give a distance effect
// create some sort of mapping function so we can define "left", "right", "top" and "bottom" of the cloud.  since strip
//   will be wound up inside in some random fashion, it would be nice to be able to somehow discover which LEDs ended up
//   on the left, right, top, and bottom of the cloud so we can use that spatial knowledge in other lightning patterns (for traveling light, etc..)

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN 6
//#define CLK_PIN   4
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 60
#define BRIGHTNESS 100
#define FRAMES_PER_SECOND 100

// temporary variables for use during prototyping
#define CLOUD_LED_START 38 // start number of pixels inside polyfill. use this when strip only partially inside polyfill during prototyping

CRGB leds[NUM_LEDS];

void setup()
{
	delay(1500); // sanity delay
	Serial.begin(9600);

	// tell FastLED about the LED strip configuration
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	//FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

	// set master brightness control
	FastLED.setBrightness(BRIGHTNESS);

	// set max volts and milliamps that project can draw (this fixes flickering strip issue i've seen when all LEDs are being lit at high brightness)
	set_max_power_in_volts_and_milliamps(5, 500);
}

// List of storm effects to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
SimplePatternList gPatterns = {rollingFlashes};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;				   // rotating "base color" used by many of the patterns

void loop()
{
	// Add entropy to random number generator; we use a lot of it.
	random16_add_entropy(random());

	// Call the current pattern function once, updating the 'leds' array
	gPatterns[gCurrentPatternNumber]();

	//delay(100);

	// send the 'leds' array out to the actual LED strip
	FastLED.show();

	// insert a delay to keep the framerate modest
	delay(1000 / FRAMES_PER_SECOND);

	// do some periodic updates
	EVERY_N_MILLISECONDS(20) { gHue++; }   // slowly cycle the "base color" through the rainbow
	EVERY_N_SECONDS(10) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
	// add one to the current pattern number, and wrap around at the end
	gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void rollingFlashes()
{
	// configure storm intensity
	uint8_t minInitFlashes = 5;
	uint8_t maxInitFlashes = 35;

	FastLED.clear(true); // clear the entire strip

	// initial lightning stroke(s)
	int i = 0;
	while (i < random8(minInitFlashes, maxInitFlashes))
	{
		uint8_t randomDot = random8(CLOUD_LED_START, NUM_LEDS);

		// a single lightning bolt pixel
		leds[randomDot] = CRGB::GhostWhite;
		FastLED.show();

		// wait a random bit, then apply fade function to any currently lit lightning pixels
		delay(random8(20, 70));
		fadeToBlackBy(leds, NUM_LEDS, random8(20, 70)); // fade to black at random-ish rate

		// then, add a bit of arcing inside the cloud for effect and tinge of color
		addCloudArcing(80, CRGB::LightCyan);
		FastLED.show();
		i++;
	}

	// lastly, complete the fadeToBlack for any remaining pixels still on their way to black
	for (int i = 0; i <= 10; i++)
	{
		fadeToBlackBy(leds, NUM_LEDS, 25);
		addCloudArcing(80, CRGB::LightCyan);
		FastLED.show();
		delay(random8(10, 50));
	}

	FastLED.clear(true); // clear the entire strip

	// now wait for next set of bolts
	delay(random16(4000, 12000));
}

void addCloudArcing(fract8 chanceOfGlitter, CRGB arcColor)
{
	if (random8() < chanceOfGlitter)
	{
		leds[random16(CLOUD_LED_START, NUM_LEDS)] += arcColor;
	}
}

void rainbow()
{
	// FastLED's built-in rainbow generator
	fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
	// built-in FastLED rainbow, plus some random sparkly glitter
	rainbow();
	addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
	if (random8() < chanceOfGlitter)
	{
		leds[random16(NUM_LEDS)] += CRGB::White;
	}
}

void confetti()
{
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy(leds, NUM_LEDS, 10);
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
	// a colored dot sweeping back and forth, with fading trails
	fadeToBlackBy(leds, NUM_LEDS, 20);
	int pos = beatsin16(13, 0, NUM_LEDS - 1);
	leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
	uint8_t BeatsPerMinute = 62;
	CRGBPalette16 palette = PartyColors_p;
	uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++)
	{ //9948
		leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
	}
}

void juggle()
{
	// eight colored dots, weaving in and out of sync with each other
	fadeToBlackBy(leds, NUM_LEDS, 20);
	byte dothue = 0;
	for (int i = 0; i < 8; i++)
	{
		leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
		dothue += 32;
	}
}