#pragma once

#include "daisysp.h"

#include "dbass.h"
#include "dsnare.h"
#include "dhihat.h"
#include "dcymbal.h"
#include "dclap.h"
#include "ddrum.h"

#define MAX(x,y) ((x > y) ? (x) : (y))
#define MIN(x,y) ((x > y) ? (y) : (x))

// boot/upload pin
#define PIN_BUTTON_UPLOAD 26

// settings
#define FILTER_CUTOFF_MAX 12000.0f

// input
#define INPUT_NONE 0
#define INPUT_MERGE 1

// pot target
#define POT_NONE 0
#define POT_TEMPO 1

// play status
#define PLAY_OFF 0
#define PLAY_ON 1
#define PLAY_TRIG 2

// sync status
#define SYNC_INT 0
#define SYNC_EXT 1

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

/*
#define PIN_LCD_RS 15 // LCD: pin 8
#define PIN_LCD_EN 16 // LCD: pin 9
#define PIN_LCD_D4 20 // LCD: D4
#define PIN_LCD_D5 19 // LCD: D5
#define PIN_LCD_D6 18 // LCD: D6
#define PIN_LCD_D7 17 // LCD: D7
*/
//#define AD_LCDBUTTON_PIN 21

// AD input
#define AD_MAX 7
#define AD_LCDBUTTON_INDEX 0
#define AD_CV0_INDEX 1
#define AD_CV1_INDEX 2
#define AD_GATE0_INDEX 3
#define AD_GATE1_INDEX 4
#define AD_POT0_INDEX 5
#define AD_POT1_INDEX 6

#define AD_LCDBUTTON_PIN 17
#define AD_CV0_PIN 18
#define AD_CV1_PIN 19
#define AD_GATE0_PIN 20
#define AD_GATE1_PIN 21
#define AD_POT0_PIN 15
#define AD_POT1_PIN 16

#define CV_NUMBER 6


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

// drums
#define DRUM_NONE 0
#define DRUM_NORMAL 100
#define DRUM_ACCENT 101

#define DTYPE_ANALOG 1
#define DTYPE_SYNTHETIC 2
#define DTYPE_OPD 3

// seq
#define TRACK_MAX 8
#define TRACK_BASS 0
#define TRACK_SNARE 1
#define TRACK_HIHAT 2
#define TRACK_CRASH 3
#define TRACK_RIDE 4
#define TRACK_CLAP 5
#define TRACK_TOMHI 6
#define TRACK_TOMLO 7

#define SEQ_MAX 64


// settings
struct DChannel
{
	bool on;

	float pan;
	float level;

	float delay_delay;
	float delay_feedback;
	
	float reverb_level;
	
	float overdrive;
	
	float accent;
};




struct Settings
{
	struct DChannel dbass;
	struct Settings_dbass settings_dbass;
	struct DChannel dsnare;
	struct Settings_dsnare settings_dsnare;
	struct DChannel dhihat;
	struct Settings_dhihat settings_dhihat;
	struct DChannel dclap;
	struct Settings_dclap settings_dclap;
	struct DChannel dcrash;
	struct Settings_dcymbal settings_dcrash;
	struct DChannel dride;
	struct Settings_dcymbal settings_dride;
	struct DChannel dtomhi;
	struct Settings_ddrum settings_dtomlo;
	struct DChannel dtomlo;
	struct Settings_ddrum settings_dtomhi;
	bool reverb_on;
	float reverb_feedback;
	float reverb_lpffreq;
	float reverb_dry;
	float reverb_wet;
	float gain_in;
	float gain_out;
	uint8_t seq[TRACK_MAX][SEQ_MAX];
	uint8_t seq_end; // number of used steps in sequence (= sequence length)
	uint32_t seq_tempo; // bpm
};



void tickIntCalc();
void tickIntStart();
void tickExtStart();
void trigStart();
void SetDelayDelay(uint8_t);
void SetReverb();
void FlashSave(uint8_t);
void FlashLoad(uint8_t);

