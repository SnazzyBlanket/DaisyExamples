#pragma once

#include "daisysp.h"

#define MAX(x,y) ((x > y) ? (x) : (y))
#define MIN(x,y) ((x > y) ? (y) : (x))

// boot/upload pin
#define PIN_BUTTON_UPLOAD 26

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

// play status (sysPlay)
#define PLAY_OFF 0
#define PLAY_SINGLE 1
#define PLAY_LOOP 2
#define PLAY_RECORD 3



// input
#define INPUT_CHANNEL_STEREO 0
#define INPUT_CHANNEL_LEFT 1
#define INPUT_CHANNEL_RIGHT 2



// LED modes
#define LED_MODE_TEMPO 0
#define LED_MODE_CLIP 1





// fx
#define MOD_SOURCE_NONE 0
#define MOD_SOURCE_LFO0 1
#define MOD_SOURCE_LFO1 2
#define MOD_SOURCE_LFO2 3
#define MOD_SOURCE_CV0 4
#define MOD_SOURCE_CV1 5
#define MOD_SOURCE_GATE0 6
#define MOD_SOURCE_GATE1 7
#define MOD_SOURCE_POT0 8
#define MOD_SOURCE_POT1 9
#define MOD_SOURCE_MAX 10

// filter
#define FILTER_TYPE_LOW 1
#define FILTER_TYPE_HIGH 2
#define FILTER_TYPE_BAND 3

// delay
#define DELAY_MAX_S 2.0f // delay max in seconds

// lfo
#define LFO_NUMBER 3
#define WAVEFORMS_MAX 8


#define SETTINGS_VERSION 1 // increase if any changes happen to OpdFlashConfig structure

struct OpdFXSettings
{
	uint8_t settingsVersion;

	// global
	uint8_t inputChannel;
	float gainInputL; // 0-5
	float gainInputR; // 0-5
	float gainOutputL; // 0-5
	float gainOutputR; // 0-5

	// oscillator
	bool oscOn;
	uint8_t oscGateMod;
	float oscGateLevel;
	bool oscGateR; // continue to loop, let ADSR handle Release stage (ie don't continue into PhaseEnd)
	float oscFreq;
	float oscDetune;
	uint8_t oscFreqMod;
	float oscAmp;
	uint8_t oscAmpMod;

	// ADSR
	bool adsrOn;
	float adsrAttack;
	float adsrDecay;
	float adsrSustain;
	float adsrRelease;
	float adsrGateLevel;
	uint8_t adsrGateMod;

	// filter
	bool filterOn;
	float filterFreq;
	uint8_t filterFreqMod;
	float filterRes;
	uint8_t filterResMod;
	uint8_t filterType;

	// decimator
	bool decimatorOn;
	float decimatorDownsampleFactor;
	float decimatorBitcrushFactor;
	uint8_t decimatorBitcrushFactorMod;
	int decimatorBitsToCrush;

	// overdrive
	bool overdriveOn;
	float overdriveDrive;

	// delay
	bool delayOn;
	float delayDelayL;
	float delayDelayR;
	uint8_t delayDelayMod;
	float delayFeedbackL;
	float delayFeedbackR;
	uint8_t delayFeedbackMod;

	// chorus
	bool chorusOn;
	float chorusDelay;
	float chorusFeedback;
	float chorusLfoDepth;
	float chorusLfoFreq;
	float chorusPan;
	float chorusDry;
	float chorusWet;

	// reverb
	bool reverbOn;
	float reverbFeedback;
	float reverbLPFFreq;
	float reverbDry;
	float reverbWet;

	// lfo
	float lfoFreq[LFO_NUMBER]; // freq Hz
	float lfoAmp[LFO_NUMBER]; // 0-1
	uint8_t lfoWaveform[LFO_NUMBER]; // 0-#define WAVEFORMS_MAX 8
	float lfoOffset[LFO_NUMBER]; // 0-1

	// cv
	float cvAmp[CV_NUMBER]; // 0-2
	float cvOffset[CV_NUMBER]; // 0-1

};



struct OpdSampleSettings
{
	// sample
	// sssssssssssssssssssss
	// start lstart lend end
	// gate--<--------->
	uint32_t sLength; // length of sample, < BUFFER_MAX
	uint32_t sPhaseStart;
	uint32_t sPhaseLoopStart;
	uint32_t sPhaseLoopEnd;
	uint32_t sPhaseEnd;
};



struct OpdModSources
{
	float cvValue[CV_NUMBER];
	float lfoValue[LFO_NUMBER];
};



// functions
float ModFactor(uint8_t);

void FXAdsrSet();
void FXFilterSet();
void FXDecimatorSet();
void FXOverdriveSet();
void FXDelaySet();
void FXChorusSet();
void FXReverbSet();
void FXLFOSet(uint8_t);

void RecordPrepare();

void UtilFlashSave();
void UtilFlashLoad();
