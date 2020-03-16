#include <Arduino.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

// Shamelessly derived from the DemoReel100 FastLED example

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

// Neopixel stuff
#define NEOPIXEL_DATA_PIN 6
//#define CLK_PIN   4
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 60
#define BRIGHTNESS 100
#define FRAMES_PER_SECOND 100

// sfx setup
#define SFX_PIN 4

// temporary variables for use during prototyping
#define CLOUD_LED_START 0 // start number of pixels inside polyfill. use this when strip only partially inside polyfill during prototyping

int COUNTER = 1;
CRGB leds[NUM_LEDS];

uint8_t gCurrentPatternNumber = 0;  // Index number of which pattern is current
uint8_t gHue = 0;				            // rotating "base color" used by many of the patterns

// declare function prototypes
void nextPattern();
void rainbow();
void rainbowWithGlitter();
void addGlitter(fract8 chanceOfGlitter);
void confetti();
void sinelon();
void juggle();
void bpm();
void activateSound(int sfx_pin);
void addCloudArcing(fract8 chanceOfGlitter, CRGB arcColor);
void lightningRollingFlashes();
void lila();

void setup() {
    delay(1500); // sanity delay
    Serial.begin(9600);

    // neopixel setup
    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE, NEOPIXEL_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    //FastLED.addLeds<LED_TYPE,NEOPIXEL_DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    // set max volts and milliamps that project can draw (this fixes flickering strip issue i've seen when all LEDs are being lit at high brightness)
    set_max_power_in_volts_and_milliamps(5, 500);

    // adafruit soundboard fx setup
    // setupSoundFx(SFX_PIN);
}



// List of storm effects to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
// SimplePatternList gPatterns = { juggle, bpm };
// SimplePatternList gPatterns = { lightningRollingFlashes, juggle };

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
	// add one to the current pattern number, and wrap around at the end
	gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);

}

void loop() {
  Serial.println(COUNTER);
  COUNTER++;

  	// Add entropy to random number generator; we use a lot of it.
	random16_add_entropy(random());

	// Call the current pattern function once, updating the 'leds' array
	gPatterns[gCurrentPatternNumber]();

 	// send the 'leds' array out to the actual LED strip
	FastLED.show();

	// insert a delay to keep the framerate modest
	delay(1000 / FRAMES_PER_SECOND);

	// do some periodic updates
	EVERY_N_MILLISECONDS(20) { gHue++; }   // slowly cycle the "base color" through the rainbow
	EVERY_N_SECONDS(10) { nextPattern(); } // change patterns periodically
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

void activateSound(int sfx_pin)
{
	Serial.println("begin: activating sound");
	digitalWrite(sfx_pin, LOW); // bring the pin low to begin the activation
	/*
  According to the documentation, the Audio FX board needs 50ms to trigger. However,
  I've found that coming from my 3.3v Arduino Pro, it needs 100ms to get the trigger
  the going
  */
	delay(200);					 // hold the pin low long enough to trigger the board; may need to be longer for consistent triggering
	digitalWrite(sfx_pin, HIGH); // bring the pin high again to end the activation
	Serial.println("end: activating sound");
}

void addCloudArcing(fract8 chanceOfGlitter, CRGB arcColor)
{
	if (random8() < chanceOfGlitter)
	{
		leds[random16(CLOUD_LED_START, NUM_LEDS)] += arcColor;
	}
}

void lightningRollingFlashes()
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

	// activate sound
	// activateSound(SFX_PIN);

	FastLED.clear(true); // clear the entire strip

	// now wait for next set of bolts
	delay(random16(8000, 12000));
}

void lila()
{
	for (int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = CRGB::DeepSkyBlue;
		FastLED.show();
		delay(10);
		fadeToBlackBy(leds, NUM_LEDS, random8(20, 70)); // fade to black at random-ish rate
	}

	for (int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = CRGB::Red;
		FastLED.show();
		delay(5);
		fadeToBlackBy(leds, NUM_LEDS, random8(100, 600)); // fade to black at random-ish rate
	}
}