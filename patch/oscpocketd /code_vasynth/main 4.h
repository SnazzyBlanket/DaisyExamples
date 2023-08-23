#pragma once

#include "daisysp.h"

#define MAX(x,y) ((x > y) ? (x) : (y))
#define MIN(x,y) ((x > y) ? (y) : (x))

// boot/upload pin
#define PIN_BUTTON_UPLOAD 26

// settings
#define WAVEFORMS_MAX 8
#define VOICES_MAX 8
#define VOICES_OK 5
#define FILTER_CUTOFF_MAX 12000.0f
#define POT_TARGET_NONE 0
#define POT_TARGET_FILTER 1

// input
#define INPUT_CHANNEL_NONE 0
#define INPUT_CHANNEL_MERGE 1

// play status
#define PLAY_OFF 0
#define PLAY_ON 1

// Lcd
#ifdef OPD_BASE_MIDI
	#define PIN_LCD_RS 8 // LCD: pin 8
	#define PIN_LCD_EN 7 // LCD: pin 9
	#define PIN_LCD_D4 9 // LCD: D4
	#define PIN_LCD_D5 10 // LCD: D5
	#define PIN_LCD_D6 11 // LCD: D6
	#define PIN_LCD_D7 12 // LCD: D7
#else
	#define PIN_LCD_RS 10 // LCD: pin 8
	#define PIN_LCD_EN 9 // LCD: pin 9
	#define PIN_LCD_D4 11 // LCD: D4
	#define PIN_LCD_D5 12 // LCD: D5
	#define PIN_LCD_D6 13 // LCD: D6
	#define PIN_LCD_D7 14 // LCD: D7
#endif

// AD input
#define AD_MAX 3

#define AD_LCDBUTTON_INDEX 0
#define AD_POT0_INDEX 1
#define AD_POT1_INDEX 2

#define AD_LCDBUTTON_PIN 17
#define AD_POT0_PIN 15
#define AD_POT1_PIN 16

#define CV_NUMBER 2

// LED modes
#define LED_MODE_TEMPO 0
#define LED_MODE_CLIP 1

// filter
#define FILTER_TYPE_LOW 1
#define FILTER_TYPE_HIGH 2
#define FILTER_TYPE_BAND 3

// delay
#define DELAY_MAX static_cast<size_t>(48000 * DELAY_MAX_S)
#define DELAY_MAX_S 2.0f // delay max in seconds

// MIDI
#define MIDI_CHANNEL_ALL 17
#define MIDI_VELOCITY_MAX 127


struct OpdModSources
{
	float cvValue[CV_NUMBER];
	uint8_t midiPitch;
	uint8_t midiVelocity;
};


