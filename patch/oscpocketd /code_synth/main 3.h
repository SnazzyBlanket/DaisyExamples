#pragma once

#include "daisysp.h"

// notes and time

// length of notes in 1/16
#define t1 16
#define t2 8
#define t4 4
#define t8 2
#define t16 1



typedef uint8_t OpdMidiPitch; // MIDI note #

struct OpdNote
{
	OpdMidiPitch notePitch; // MIDI note number
	uint32_t noteTime; // ticks
};

struct OpdMixSynth
{
	uint8_t volume; // 0-99
	uint8_t	pan; // 1-9
	uint8_t reverb; // 0-9
	float factorR; // = volume / 100
	float factorL; // = volume / 100 
};

struct OpdMixDrum
{
	uint8_t kickVolume;
	uint8_t kickPan;
	uint8_t snareVolume;
	uint8_t snarePan;
	uint8_t hihatVolume;
	uint8_t hihatPan;
	uint8_t clapVolume;
	uint8_t clapPan;
	uint8_t crashVolume;
	uint8_t crashPan;
};

struct OpdSynth
{
	uint8_t oscWaveform;
	float oscDetune;
	float filterRes;
	float filterCutoff;
	float adsrAttack;
	float adsrDecay;
	float adsrSustain; // level
	float adsrRelease;

	float lfoFrequency;
	float lfoAmount;
	uint8_t lfoTarget;

	float delayDelay;
	float delayFeedback;
};

struct OpdDrum
{
	float kickFrequency;
	float kickDecay;
	float snareFrequency;
	float snareDecay;
	float snareNoise; // "amount": affects filter freq and Release
	float hhoFilter;
	float hhoDecay;
	float hhcFilter;
	float hhcDecay;
	float crashFilter;
	float crashDecay;
	float clapFilter;
	float clapDecay;
};


// Reboot to bootload
#define PIN_BUTTON_UPLOAD 26

// LFO

#define LFO_TARGET_NONE 0 // none, off
#define LFO_TARGET_PITCH 1 // PITCH
#define LFO_TARGET_FILTER 2 // FILTER
#define LFO_TARGET_AMP 3 // ADSR



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
#define PIN_BUTTONS 17

// bass

#define BASS_NUMBER 3
#define BASS_NOTES 32



// lead

#define LEAD_NUMBER 4
#define LEAD_NOTES 64



#define SYNTH_NUMBER (BASS_NUMBER + LEAD_NUMBER)



// drums

#define DRUM_NUMBER 1
#define DRUM_NOTES 32

// drum sounds in sequence
#define DRUM_KICK		0b00000001
#define DRUM_SNARE		0b00000010
#define DRUM_HHOPEN		0b00000100
#define DRUM_HHCLOSED 	0b00001000
#define DRUM_CRASH		0b00010000
#define DRUM_CLAP		0b00100000

#define DRUM_SOUNDS 5 // kick + snare + hhopen + crash + clap


// song

#define SEQ_NUMBER 10 // number of possible sequences per track
#define SONG_STEP_NUMBER 999
#define SONG_TICKS_PER_STEP 32 // how many "ticks" until songSeqStep advance



// tracks and sequences

#define SEQ_VOICES (BASS_NUMBER + LEAD_NUMBER + DRUM_NUMBER)

#define OFFSET_BASS 0
#define OFFSET_LEAD BASS_NUMBER
#define OFFSET_DRUM (BASS_NUMBER + LEAD_NUMBER)
// it is very important that TRACK_NUMBER stays at 8, because our screen/LCD is 8x2 characters wide
#define TRACK_NUMBER (BASS_NUMBER + LEAD_NUMBER + DRUM_NUMBER)

// play status (sysPlayMode)
#define PLAY_OFF 0
#define PLAY_SEQ 1
#define PLAY_SONG 2



// FX - global
struct OpdFXSettings
{
	float reverbFeedback;
	float reverbLpfFrequency;
};

#define DELAY_MAX_S 2.0f // delay max in seconds



// waveforms
#define WAVEFORMS_MAX 8



// LED modes
#define LED_MODE_TEMPO 0
#define LED_MODE_CLIP 1



// save/load flash

#define FLASH_CONFIG_VERSION 5 // increase if any changes happen to OpdFlashConfig structure

struct OpdFlashConfig
{
	uint8_t configVersion;

	OpdMixSynth mixSynth[SYNTH_NUMBER + DRUM_SOUNDS];
	OpdMixDrum mixDrum;

	OpdSynth synthSettings[SYNTH_NUMBER];
	OpdDrum drumSettings;

	uint8_t songStep[SONG_STEP_NUMBER][SEQ_VOICES];
	uint8_t seqSongLen;
	
	OpdMidiPitch seqBass[BASS_NUMBER][SEQ_NUMBER][BASS_NOTES];
	OpdNote seqLead[LEAD_NUMBER][SEQ_NUMBER][LEAD_NOTES];
	uint8_t seqLeadLen[LEAD_NUMBER][SEQ_NUMBER];
	uint8_t seqDrum[SEQ_NUMBER][DRUM_NOTES];

	OpdFXSettings fxSettings;
	uint32_t seqBassGateTime[BASS_NUMBER];
	bool seqVoiceOn[TRACK_NUMBER];
	uint32_t sysTickBPM;
};



// functions used in other files

void SetupSeq();
void PlaySeq();
void StopSeq();
void CalcTick();
void CalcMix();

void FXSetDelay(int);

void UtilSeqTranspose(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void UtilSeqShift(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void UtilSeqGenerate(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void UtilSeqCopy(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void UtilSeqClear(uint8_t, uint8_t, uint8_t);

void UtilFlashSave();
void UtilFlashLoad();

void UtilMIDIExport();
