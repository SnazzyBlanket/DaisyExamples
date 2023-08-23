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

// DAC output

#define DA_CV0_PIN 22
#define DA_CV1_PIN 23
#define DA_GATE0_PIN 24
#define DA_GATE1_PIN 25





// play status (sysPlayMode)
#define PLAY_OFF 0
#define PLAY_ON 1



// input
#define INPUT_CHANNEL_STEREO 0
#define INPUT_CHANNEL_LEFT 1
#define INPUT_CHANNEL_RIGHT 2
#define INPUT_CHANNEL_MERGE 3



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
#define MOD_SOURCE_MIDIP 10
#define MOD_SOURCE_MIDIV 11
#define MOD_SOURCE_SM0 12
#define MOD_SOURCE_SM1 13
#define MOD_SOURCE_SM2 14
#define MOD_SOURCE_SMT0 15
#define MOD_SOURCE_SMT1 16
#define MOD_SOURCE_SMT2 17
#define MOD_SOURCE_AUDIO 18
#define MOD_SOURCE_EG0 19
#define MOD_SOURCE_EG1 20

#define MOD_SOURCE_MAX 21

// filter
#define FILTER_TYPE_LOW 1
#define FILTER_TYPE_HIGH 2
#define FILTER_TYPE_BAND 3

// delay
#define DELAY_MAX_S 2.0f // delay max in seconds

// special modulators
#define SM_NUMBER 3
#define SEQ_STEPS 16

// lfo
#define LFO_NUMBER 3
#define WAVEFORMS_MAX 8

// eg
#define EG_NUMBER 2

// slicer
#define SLICER_MAX (48000 * 10)

#define SETTINGS_VERSION 3

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
	float oscFreq;
	float oscDetune;
	uint8_t oscFreqMod;
	uint8_t oscWaveform;
	float oscAmp;
	uint8_t oscAmpMod;
	float oscDetune2;
	uint8_t oscDetune2Mod;
	uint8_t oscWaveform2;
	float oscAmp2;
	uint8_t oscAmp2Mod;

	// ADSR
	bool adsrOn;
	float adsrAttack;
	float adsrDecay;
	float adsrSustain;
	float adsrRelease;
	float adsrGateLevel;
	uint8_t adsrSustainMod; // max level
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
	float overdriveGain;
	uint8_t overdriveGainMod;
	float overdriveDrive;
	uint8_t overdriveDriveMod;
	
	// pan
	bool panOn;
	float panPan; // balance: 0.00 = left, 0.50 = center, 0.99 = right
	uint8_t panPanMod;

	// delay
	bool delayOn;
	float delayDelayL;
	float delayDelayR;
	uint8_t delayDelayMod;
	float delayFeedbackL;
	float delayFeedbackR;
	uint8_t delayFeedbackMod;

	// flanger
	bool flangerOn;
	
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
	
	// slicer
	bool slicerOn;
	size_t slicer_record_samples_max;
	uint8_t slicerRSMMod; // trig record
	float slicerRSMModGateLevel;
	size_t slicer_playback_rep_max;
	uint8_t slicerPRMMod; // trig playback
	float slicerPRMModGateLevel;
	bool slicerTrigMode;
	
	// sm - special modulator
	uint8_t smType[SM_NUMBER];
	float smFreq[SM_NUMBER];
	float smAmp[SM_NUMBER];
	float smOffset[SM_NUMBER];
	uint8_t smSeqMidi[SM_NUMBER][SEQ_STEPS];
	float smSeqVal[SM_NUMBER][SEQ_STEPS];
	uint8_t smSeqLen[SM_NUMBER];
	uint8_t smSyncMod[SM_NUMBER];
	float smSyncLevel[SM_NUMBER];
	float smGateLen;

	// lfo
	float lfoFreq[LFO_NUMBER]; // freq Hz
	float lfoAmp[LFO_NUMBER]; // 0-1
	uint8_t lfoWaveform[LFO_NUMBER]; // 0-#define WAVEFORMS_MAX 8
	float lfoOffset[LFO_NUMBER]; // 0-1

	// cv
	float cvAmp[CV_NUMBER]; // 0-2
	float cvOffset[CV_NUMBER]; // 0-1

	// EG
	float egAttack[EG_NUMBER];
	float egDecay[EG_NUMBER];
	float egSustain[EG_NUMBER];
	float egRelease[EG_NUMBER];
	uint8_t egGateMod[EG_NUMBER]; // what mod tells eg if it is on
	float egGateLevel[EG_NUMBER]; // egAdsr is "on" if egGateMod > egGateLevel
	
	// cvgate out
	uint8_t outCV0Mod;
	uint8_t outCV1Mod;
	uint8_t outGate0Mod;
	uint8_t outGate1Mod;
	float outGate0Level; // 0-1
	float outGate1Level; // 0-1
};



struct OpdModSources
{
	float cvValue[CV_NUMBER];
	float lfoValue[LFO_NUMBER];
	float smValue[SM_NUMBER];
	float smGate[SM_NUMBER];
	bool smSync[SM_NUMBER];
	uint8_t midiPitch;
	uint8_t midiVelocity;
	float audioIn;
	float egValue[EG_NUMBER];
};



// save/load flash

#define FLASH_CONFIG_VERSION 1 // increase if any changes happen to OpdFlashConfig structure

struct OpdFlashConfig
{
	uint8_t configVersion;

	OpdFXSettings fxSettings;
};



// functions
float ModFactor(uint8_t);

void FXOscSet();
void FXAdsrSet();
void FXFilterSet();
void FXDecimatorSet();
void FXOverdriveSet();
void FXDelaySet();
void FXChorusSet();
void FXReverbSet();
void FXSlicerSet();
void FXSMSet(uint8_t);
void FXLFOSet(uint8_t);
void FXEGSet(uint8_t);

void UtilFlashSave();
void UtilFlashLoad();

float mtoval(uint8_t);

