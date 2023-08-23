/*
  
Project: OscPocketD
Description: Multitimbral portable music making tool for Daisy
Author: Staffan Melin, staffan.melin@oscillator.se
License: GNU General Public License v3.0
Version: 202102
Project site: http://www.oscillator.se/opensource

*/
#define OPD_BASE_MIDI // version 2 with MIDI

#include "daisy_seed.h"
#include "daisysp.h"

#include "stm32h7xx_hal.h" // for HAL_NVIC_SystemReset();
//extern "C" void HAL_NVIC_SystemReset();
#include "core_cm7.h"

#include "main.h"
#include "ui.h"

#include "dev/lcd_hd44780.h"

#include <string.h>

using namespace daisy;
using namespace daisysp;



#define OPD_LOGG // start serial over USB Logger class
//#define OPD_MEASURE // measure MCU utilization



// globals

DaisySeed hardware;

float sysSampleRate;
float sysCallbackRate;

bool uiRedraw = true;

bool sysPlay;
uint8_t sysPlayMode;



// hardware

LcdHD44780 lcd;
AdcChannelConfig adcLCDButtons; // 	Lcd buttons; ADC configuration
uint8_t sysLedMode;
uint32_t sysLedOnTime;
bool sysClip;



// metronome

Metro tick;
/*
	120 bpm = 2 bps = 2*4 = 8 ticks/s
	but since we wont to be able to change tempo
	let's use 800, and use a counter (to 100) in ProcessTicks()
	ex
	120 bpm = 800
	60 bpm  = 400
	240 bpm = 1600
	gives
	sysTickFactor * 120 = 800
	sysTickFactor = 800 / 120 =approx 7 (6.666...)
	gives
	sysTickFreq = sysTickBPM * 7
	see: CalcTick()
*/

uint32_t sysTickCount;
const uint32_t sysTickFactor = 7;
uint32_t sysTickBPM = 120;
uint32_t sysTickFreq; // set in CalcTick();
#define METRO_FACTOR 100 // see seqBassGateTime[]



// FX

ReverbSc fxReverb;
OpdFXSettings fxSettings;
#define DELAY_MAX static_cast<size_t>(48000 * DELAY_MAX_S)
// 7 * 48000 * 2 = 672000 * (size of float)
// https://forum.electro-smith.com/t/daisy-crashes-using-reverbsc-with-96k-sample-rate/311/2
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS fxDelay[SYNTH_NUMBER];



// synth

OpdSynth synthSettings[SYNTH_NUMBER];
char synthOscWaveNames[WAVEFORMS_MAX][4] = {"SIN", "TRI", "SAW", "RAM", "SQU", "TRP", "SAP", "SQP"};

Oscillator synthOsc[SYNTH_NUMBER];
Adsr synthAEnv[SYNTH_NUMBER];
Svf synthFilter[SYNTH_NUMBER];

OpdDrum drumSettings;

// kick
Oscillator kickOsc;
AdEnv kickAEnv;
AdEnv kickPEnv; // pitch env

// snare
WhiteNoise snareNoise;
Svf snareNoiseLPF;
AdEnv snareNAEnv; // amp env
Oscillator snareOsc;
AdEnv snareOAEnv; // amp env
AdEnv snareOPEnv; // pitch env

// hihat (open and closed)
WhiteNoise hihatNoise;
Svf hihatNoiseHPF;
AdEnv hihatNoiseAEnv; // amp env

// crash
WhiteNoise crashNoise;
Svf crashNoiseHPF;
AdEnv crashNoiseAEnv; // amp env

// clap
WhiteNoise clapNoise;
Svf clapNoiseBPF;
AdEnv clapNoiseAEnv; // amp env



// LFO
Oscillator lfoOsc[SYNTH_NUMBER];
// OpdLfo lfo[SYNTH_NUMBER];



// seq

bool seqSynthGate[SYNTH_NUMBER];

uint8_t seqBassStep;
uint32_t seqBassGateTime[BASS_NUMBER] = {30, 30, 50}; // % of 1/16th

uint8_t seqLeadStep[LEAD_NUMBER];
uint32_t seqLeadTime[LEAD_NUMBER];

uint8_t seqDrumStep;



// song

uint8_t seqSongStep;
uint8_t seqSongLen = 24; // NOTE: see demo.h
bool seqVoiceOn[TRACK_NUMBER];
uint8_t seqSongCurrent[SEQ_VOICES];
int seqSongTicks;



// mixer: vol, pan (, fx)
OpdMixSynth mixSynth[SYNTH_NUMBER + DRUM_SOUNDS];
OpdMixDrum mixDrum;
// translate pan values (0, 1-9) into factors that multiply the volume for each channel
float mixPan[10] = {0, 1, 0.87, 0.75, 0.63, 0.5, 0.38, 0.25, 0.13, 0};



// include demo note data
#include "demo.h"
// NOTE seqSongLen = 4; init'ed above!!!


// prototypes / forward decl

void ProcessTick();



// audio callback

uint32_t acbCnt = 0;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
	float outSound[SYNTH_NUMBER + DRUM_SOUNDS];

	float outTemp;
	float outDelay;

	float outEnv;
	float sigL, sigR;
	float sigRevL, sigRevR;
	float sigOutRevL, sigOutRevR;

	#ifdef OPD_MEASURE
	// measure - start
	DWT->CYCCNT = 0;
	#endif

	// sequencer

	ProcessTick();

	// audio

    for (size_t n = 0; n < size; n += 2)
    {
		for (int i = 0; i < SYNTH_NUMBER; i++)
		{
			// LFO

			acbCnt++;

			if (acbCnt == 10)
			{
				acbCnt = 0;
				if (synthSettings[i].lfoTarget != LFO_TARGET_NONE)
				{
					outTemp = lfoOsc[i].Process();
					switch (synthSettings[i].lfoTarget)
					{
					case LFO_TARGET_FILTER:
						synthFilter[i].SetFreq(synthSettings[i].filterCutoff * (1 - outTemp));
						break;
					}
				}
			}

			// synth

			outEnv = synthAEnv[i].Process(seqSynthGate[i]);
		    synthOsc[i].SetAmp(outEnv);
		    outTemp = synthOsc[i].Process();
		    synthFilter[i].Process(outTemp);
		    outSound[i] = synthFilter[i].Low();

			if (synthSettings[i].delayFeedback > 0)
			{
				outDelay = fxDelay[i].Read();
				outTemp = outSound[i];
				outSound[i]  = outDelay + outTemp;
				fxDelay[i].Write((outDelay + outTemp) * synthSettings[i].delayFeedback);
			}
		}

		// kick

		outEnv = kickPEnv.Process();
        kickOsc.SetFreq(outEnv);
		outEnv = kickAEnv.Process();
        kickOsc.SetAmp(outEnv);
        outSound[OFFSET_DRUM + 0] = kickOsc.Process();

		// snare - noise + osc

        outEnv = snareNAEnv.Process();
        outTemp = snareNoise.Process();
        snareNoiseLPF.Process(outTemp);
        outTemp = snareNoiseLPF.Low();
        outTemp *= outEnv;

		outEnv = snareOPEnv.Process();
        snareOsc.SetFreq(outEnv);
		outEnv = snareOAEnv.Process();
        snareOsc.SetAmp(outEnv);

		outSound[OFFSET_DRUM + 1] = (outTemp + snareOsc.Process());

		// hihat

		outEnv = hihatNoiseAEnv.Process();
		outTemp = hihatNoise.Process();
		hihatNoiseHPF.Process(outTemp);
		outTemp = hihatNoiseHPF.High();
		outTemp *= outEnv;

        outSound[OFFSET_DRUM + 2] = outTemp;

		// crash

		outEnv = crashNoiseAEnv.Process();
		outTemp = crashNoise.Process();
		crashNoiseHPF.Process(outTemp);
		outTemp = crashNoiseHPF.High();
		outTemp *= outEnv;

        outSound[OFFSET_DRUM + 3] = outTemp;

		// clap

		outEnv = clapNoiseAEnv.Process();
		outTemp = clapNoise.Process();
		clapNoiseBPF.Process(outTemp);
		outTemp = clapNoiseBPF.Band();
		outTemp *= outEnv;

        outSound[OFFSET_DRUM + 4] = outTemp;

		// .factorL and .factorR are precalculated in CalcMix() every time .volume or .pan is changed (in ui.cpp)

		sigL = 0;
		sigRevL = 0;

		for (int i = 0; i < (SYNTH_NUMBER + DRUM_SOUNDS); i++)
		{
			outTemp = outSound[i] * mixSynth[i].factorL;
			sigL += outTemp;
			sigRevL += outTemp * ((float)mixSynth[i].reverb / 10);
		}
		
		sigR = 0;
		sigRevR = 0;

		for (int i = 0; i < (SYNTH_NUMBER + DRUM_SOUNDS); i++)
		{
			outTemp = outSound[i] * mixSynth[i].factorR;
			sigR += outTemp;
			sigRevR += outTemp * ((float)mixSynth[i].reverb / 10);
		}

		// reverb send
		// rev send: add separate sig mix to one common reverb AND add that output to main out
		
        fxReverb.Process(sigRevL, sigRevR, &sigOutRevL, &sigOutRevR);

        out[n] = sigL + sigOutRevL;
        out[n + 1] = sigR + sigOutRevR;

		if (sysLedMode == LED_MODE_CLIP)
		{
			if ((sigR > 1.0f) || (sigL > 1.0f))
			{
				sysClip = true;
			}
		}

    }

	#ifdef OPD_MEASURE
	// measure - stop
	if (DWT->CYCCNT > 390000)
	{
		hardware.SetLed(true);
	}
	#endif
}



// setup synths

void SetupSynth()
{

	// set data to demo settings

	// bass

	synthSettings[0].oscWaveform = Oscillator::WAVE_SAW;
	synthSettings[0].oscDetune = 0;
	synthSettings[0].filterRes = 0;
	synthSettings[0].filterCutoff = 1000;
	synthSettings[0].adsrAttack = 0.01;
	synthSettings[0].adsrDecay = 0.01;
	synthSettings[0].adsrSustain = 1.f;
	synthSettings[0].adsrRelease = 0.1;
	synthSettings[0].lfoFrequency = 0.8f;
	synthSettings[0].lfoAmount = 0.2;
	synthSettings[0].lfoTarget = LFO_TARGET_FILTER;
	synthSettings[0].delayDelay = 0.1f;
	synthSettings[0].delayFeedback = 0.1f;

	synthSettings[1].oscWaveform = Oscillator::WAVE_TRI;
	synthSettings[1].oscDetune = 0;
	synthSettings[1].filterRes = 0;
	synthSettings[1].filterCutoff = 2500;
	synthSettings[1].adsrAttack = 0.01;
	synthSettings[1].adsrDecay = 0.01;
	synthSettings[1].adsrSustain = 1.f;
	synthSettings[1].adsrRelease = 0.1;
	synthSettings[1].lfoFrequency = 0;
	synthSettings[1].lfoAmount = 0;
	synthSettings[1].lfoTarget = LFO_TARGET_NONE;
	synthSettings[1].delayDelay = 0.4f;
	synthSettings[1].delayFeedback = 0.3f;

	synthSettings[2].oscWaveform = Oscillator::WAVE_SAW;
	synthSettings[2].oscDetune = 0;
	synthSettings[2].filterRes = 0;
	synthSettings[2].filterCutoff = 2000;
	synthSettings[2].adsrAttack = 0.01;
	synthSettings[2].adsrDecay = 0.01;
	synthSettings[2].adsrSustain = 1.f;
	synthSettings[2].adsrRelease = 0.1;
	synthSettings[2].lfoFrequency = 1.2;
	synthSettings[2].lfoAmount = 0.5;
	synthSettings[2].lfoTarget = LFO_TARGET_FILTER;
	synthSettings[2].delayDelay = 0.8f;
	synthSettings[2].delayFeedback = 0.4f;

	// lead

	synthSettings[3].oscWaveform = Oscillator::WAVE_TRI;
	synthSettings[3].oscDetune = 0;
	synthSettings[3].filterRes = 0;
	synthSettings[3].filterCutoff = 2000;
	synthSettings[3].adsrAttack = 0.01;
	synthSettings[3].adsrDecay = 0.01;
	synthSettings[3].adsrSustain = 1.f;
	synthSettings[3].adsrRelease = 0.4;
	synthSettings[3].lfoFrequency = 0;
	synthSettings[3].lfoAmount = 0;
	synthSettings[3].lfoTarget = LFO_TARGET_NONE;
	synthSettings[3].delayDelay = 0.1f;
	synthSettings[3].delayFeedback = 0.0f;

	synthSettings[4].oscWaveform = Oscillator::WAVE_SQUARE;
	synthSettings[4].oscDetune = 0;
	synthSettings[4].filterRes = 0;
	synthSettings[4].filterCutoff = 1000;
	synthSettings[4].adsrAttack = 0.01;
	synthSettings[4].adsrDecay = 0.01;
	synthSettings[4].adsrSustain = 1.f;
	synthSettings[4].adsrRelease = 0.6;
	synthSettings[4].lfoFrequency = 0;
	synthSettings[4].lfoAmount = 0;
	synthSettings[4].lfoTarget = LFO_TARGET_NONE;
	synthSettings[4].delayDelay = 0.4f;
	synthSettings[4].delayFeedback = 0.4f;

	synthSettings[5].oscWaveform = Oscillator::WAVE_SAW;
	synthSettings[5].oscDetune = 0;
	synthSettings[5].filterRes = 0;
	synthSettings[5].filterCutoff = 2000;
	synthSettings[5].adsrAttack = 0.01;
	synthSettings[5].adsrDecay = 0.01;
	synthSettings[5].adsrSustain = 1.f;
	synthSettings[5].adsrRelease = 0.4;
	synthSettings[5].lfoFrequency = 1.5f;
	synthSettings[5].lfoAmount = 0.5;
	synthSettings[5].lfoTarget = LFO_TARGET_FILTER;
	synthSettings[5].delayDelay = 0.9f;
	synthSettings[5].delayFeedback = 0.5f;

	synthSettings[6].oscWaveform = Oscillator::WAVE_SQUARE;
	synthSettings[6].oscDetune = 0;
	synthSettings[6].filterRes = 0;
	synthSettings[6].filterCutoff = 1000;
	synthSettings[6].adsrAttack = 0.3;
	synthSettings[6].adsrDecay = 0.01;
	synthSettings[6].adsrSustain = 1.f;
	synthSettings[6].adsrRelease = 0.5;
	synthSettings[6].lfoFrequency = 0;
	synthSettings[6].lfoAmount = 0;
	synthSettings[6].lfoTarget = LFO_TARGET_NONE;
	synthSettings[6].delayDelay = 0.1f;
	synthSettings[6].delayFeedback = 0.0f;



	// init

	for (int i = 0; i < SYNTH_NUMBER; i++)
	{
		// synths

		synthOsc[i].Init(sysSampleRate);
		synthOsc[i].SetWaveform(synthSettings[i].oscWaveform);
		synthOsc[i].SetAmp(0.5f); // default
		synthOsc[i].SetFreq(1000); // default

		synthAEnv[i].Init(sysSampleRate);
		synthAEnv[i].SetTime(ADSR_SEG_ATTACK, synthSettings[i].adsrAttack);
		synthAEnv[i].SetTime(ADSR_SEG_DECAY, synthSettings[i].adsrDecay);
		synthAEnv[i].SetTime(ADSR_SEG_RELEASE, synthSettings[i].adsrRelease);
		synthAEnv[i].SetSustainLevel(synthSettings[i].adsrSustain);

		synthFilter[i].Init(sysSampleRate);
		synthFilter[i].SetRes(synthSettings[i].filterRes);
		synthFilter[i].SetDrive(0.0f); // default
		synthFilter[i].SetFreq(synthSettings[i].filterCutoff);

		// lfo

		lfoOsc[i].Init(sysSampleRate);
		lfoOsc[i].SetAmp(synthSettings[i].lfoAmount);
		lfoOsc[i].SetFreq(synthSettings[i].lfoFrequency);
		lfoOsc[i].SetWaveform(Oscillator::WAVE_SIN);

		// delay

		fxDelay[i].Init();
		fxDelay[i].SetDelay(sysSampleRate * synthSettings[i].delayDelay);
	}

}



// setup drums

void SetupDrums()
{

	// set

	drumSettings.kickFrequency = 200;
	drumSettings.kickDecay = 0.2;
	drumSettings.snareFrequency = 300;
	drumSettings.snareDecay = 0.2;
	drumSettings.snareNoise = 50; // 0-99; also affects filter
	drumSettings.hhoFilter = 5000;
	drumSettings.hhoDecay = 0.3;
	drumSettings.hhcFilter = 6000;
	drumSettings.hhcDecay = 0.1;
	drumSettings.crashFilter = 3000;
	drumSettings.crashDecay = 0.9;
	drumSettings.clapFilter = 1400;
	drumSettings.clapDecay = 0.2;


	// init

    kickOsc.Init(sysSampleRate);
    kickOsc.SetWaveform(Oscillator::WAVE_SIN);
    kickOsc.SetAmp(0.5f);
    kickOsc.SetFreq(1000); // unused

    kickAEnv.Init(sysSampleRate);
    kickAEnv.SetCurve(0); // linear
    kickAEnv.SetTime(ADENV_SEG_ATTACK, .01);
    kickAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.kickDecay);
    kickAEnv.SetMin(0.0);
    kickAEnv.SetMax(1.f);

    kickPEnv.Init(sysSampleRate);
    kickPEnv.SetCurve(0); // linear
    kickPEnv.SetTime(ADENV_SEG_ATTACK, .01);
    kickPEnv.SetTime(ADENV_SEG_DECAY, drumSettings.kickDecay);
    kickPEnv.SetMin(drumSettings.kickFrequency / 10.0f);
    kickPEnv.SetMax(drumSettings.kickFrequency);

	snareNoise.Init();
    snareNoiseLPF.Init(sysSampleRate);
    snareNoiseLPF.SetRes(0.0f);
    snareNoiseLPF.SetDrive(0.0f);
    snareNoiseLPF.SetFreq(3000 - (drumSettings.snareNoise * 20));

	snareNAEnv.Init(sysSampleRate);
	snareNAEnv.SetTime(ADENV_SEG_ATTACK, .01);
	snareNAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
	snareNAEnv.SetMin(0);
	snareNAEnv.SetMax(1.0f * ((float)drumSettings.snareNoise / 100.0f));

    snareOsc.Init(sysSampleRate);
    snareOsc.SetWaveform(Oscillator::WAVE_SIN);
    snareOsc.SetAmp(0.5f);
    snareOsc.SetFreq(160); // unused

    snareOAEnv.Init(sysSampleRate);
    snareOAEnv.SetCurve(0); // linear
    snareOAEnv.SetTime(ADENV_SEG_ATTACK, .01);
    snareOAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
    snareOAEnv.SetMin(0.0);
    snareOAEnv.SetMax(1.f);

    snareOPEnv.Init(sysSampleRate);
    snareOPEnv.SetCurve(0); // linear
    snareOPEnv.SetTime(ADENV_SEG_ATTACK, .01);
    snareOPEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
    snareOPEnv.SetMin((float)drumSettings.snareFrequency / 3.0f);
    snareOPEnv.SetMax(drumSettings.snareFrequency);

	// hihat (open and closed)
	// open and closed hihat are exclusive or, ie cannot play at the same time
	// so we can reuse everything for both sounds (see ProcessTicks())
	hihatNoise.Init();
    hihatNoiseHPF.Init(sysSampleRate);
    hihatNoiseHPF.SetRes(0.3f);
    hihatNoiseHPF.SetDrive(0.3f);
    hihatNoiseHPF.SetFreq(drumSettings.hhoFilter);

	hihatNoiseAEnv.Init(sysSampleRate);
	hihatNoiseAEnv.SetTime(ADENV_SEG_ATTACK, .01);
	hihatNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.hhoDecay);
	hihatNoiseAEnv.SetMin(0);
	hihatNoiseAEnv.SetMax(1);

	// crash
	crashNoise.Init();
    crashNoiseHPF.Init(sysSampleRate);
    crashNoiseHPF.SetRes(0.0f);
    crashNoiseHPF.SetDrive(0.0f);
    crashNoiseHPF.SetFreq(drumSettings.crashFilter);

	crashNoiseAEnv.Init(sysSampleRate);
	crashNoiseAEnv.SetTime(ADENV_SEG_ATTACK, .01);
	crashNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.crashDecay);
	crashNoiseAEnv.SetMin(0);
	crashNoiseAEnv.SetMax(1);

	// clap
	clapNoise.Init();
    clapNoiseBPF.Init(sysSampleRate);
    clapNoiseBPF.SetRes(0.0f);
    clapNoiseBPF.SetDrive(0.0f);
    clapNoiseBPF.SetFreq(drumSettings.clapFilter);

	clapNoiseAEnv.Init(sysSampleRate);
	clapNoiseAEnv.SetTime(ADENV_SEG_ATTACK, .01);
	clapNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.clapDecay);
	clapNoiseAEnv.SetMin(0);
	clapNoiseAEnv.SetMax(1);

}



// setup fx

void SetupFX()
{
	// reverb

	fxSettings.reverbFeedback = 0.8;
	fxSettings.reverbLpfFrequency = 8000;

	fxReverb.Init(sysSampleRate);
	fxReverb.SetFeedback(fxSettings.reverbFeedback);
	fxReverb.SetLpFreq(fxSettings.reverbLpfFrequency);

}


void FXSetDelay(int aTrack)
{
	fxDelay[aTrack].SetDelay(sysSampleRate * synthSettings[aTrack].delayDelay);
}



// setup mix

void SetupMix()
{
	// bass 1
	mixSynth[0].volume = 25;
	mixSynth[0].pan = 5;
	mixSynth[0].reverb = 1;
	// bass 2
	mixSynth[1].volume = 18;
	mixSynth[1].pan = 3;
	mixSynth[1].reverb = 1;
	// bass 3
	mixSynth[2].volume = 13;
	mixSynth[2].pan = 8;
	mixSynth[2].reverb = 2;
	// lead 1
	mixSynth[3].volume = 20;
	mixSynth[3].pan = 2;
	mixSynth[3].reverb = 2;
	// lead 2
	mixSynth[4].volume = 12;
	mixSynth[4].pan = 3;
	mixSynth[4].reverb = 1;
	// lead 3
	mixSynth[5].volume = 15;
	mixSynth[5].pan = 6;
	mixSynth[5].reverb = 2;
	// lead 4
	mixSynth[6].volume = 15;
	mixSynth[6].pan = 7;
	mixSynth[6].reverb = 2;

	// drum 1 - UNUSED
	//mixSynth[7].volume = 15;
	//mixSynth[7].pan = 5;
	//mixSynth[7].reverb = 0;

	// mixDrum
	mixDrum.kickVolume = 35;
	mixDrum.kickPan = 5;
	mixSynth[OFFSET_DRUM + 0].reverb = 1;
	mixDrum.snareVolume = 30;
	mixDrum.snarePan = 4;
	mixSynth[OFFSET_DRUM + 1].reverb = 3;
	mixDrum.hihatVolume = 7;
	mixDrum.hihatPan = 7;
	mixSynth[OFFSET_DRUM + 2].reverb = 2;
	mixDrum.crashVolume = 12;
	mixDrum.crashPan = 2;
	mixSynth[OFFSET_DRUM + 4].reverb = 4;
	mixDrum.clapVolume = 45;
	mixDrum.clapPan = 6;
	mixSynth[OFFSET_DRUM + 3].reverb = 5;

	for (int i = 0; i < 8; i++)
	{
		seqVoiceOn[i] = true;
	}
}



// calculate values used in audio callback
// call when synth och drum mix changes
void CalcMix()
{
	for (int i = 0; i < SYNTH_NUMBER; i++)
	{
		mixSynth[i].factorL = ((float)mixSynth[i].volume / 100) * mixPan[mixSynth[i].pan];
		mixSynth[i].factorR = ((float)mixSynth[i].volume / 100) * mixPan[10 - mixSynth[i].pan];
	}

	// individual drums

	mixSynth[OFFSET_DRUM + 0].factorL = ((float)mixDrum.kickVolume / 100) * mixPan[mixDrum.kickPan];
	mixSynth[OFFSET_DRUM + 0].factorR = ((float)mixDrum.kickVolume / 100) * mixPan[10 - mixDrum.kickPan];
	mixSynth[OFFSET_DRUM + 1].factorL = ((float)mixDrum.snareVolume / 100) * mixPan[mixDrum.snarePan];
	mixSynth[OFFSET_DRUM + 1].factorR = ((float)mixDrum.snareVolume / 100) * mixPan[10 - mixDrum.snarePan];
	mixSynth[OFFSET_DRUM + 2].factorL = ((float)mixDrum.hihatVolume / 100) * mixPan[mixDrum.hihatPan];
	mixSynth[OFFSET_DRUM + 2].factorR = ((float)mixDrum.hihatVolume / 100) * mixPan[10 - mixDrum.hihatPan];
	mixSynth[OFFSET_DRUM + 3].factorL = ((float)mixDrum.crashVolume / 100) * mixPan[mixDrum.crashPan];
	mixSynth[OFFSET_DRUM + 3].factorR = ((float)mixDrum.crashVolume / 100) * mixPan[10 - mixDrum.crashPan];
	mixSynth[OFFSET_DRUM + 4].factorL = ((float)mixDrum.clapVolume / 100) * mixPan[mixDrum.clapPan];
	mixSynth[OFFSET_DRUM + 4].factorR = ((float)mixDrum.clapVolume / 100) * mixPan[10 - mixDrum.clapPan];

}



// setup note data (sequences)

void SetupSeq()
{
	seqBassStep  = 0;

	for (uint8_t i = 0; i < SYNTH_NUMBER; i++)
	{
		seqSynthGate[i] = false;
	}


	for (uint8_t i = 0; i < LEAD_NUMBER; i++)
	{
		seqLeadTime[i] = 0;
		seqLeadStep[i] = 0;
	}

	seqDrumStep  = 0;

	// init unused song steps

	for (int i = seqSongLen; i < SONG_STEP_NUMBER; i++)
	{
		for (int j = 0; j < SEQ_VOICES; j++)
		{
			songStep[i][j] = 0;
		}
	}

	seqSongTicks = 0;
	for (int j = 0; j < SEQ_VOICES; j++)
	{
		seqSongCurrent[j] = songStep[seqSongStep][j];
	}

}



// sequencer

void bassPlay(int j)
{
	uint8_t aSeq = seqSongCurrent[OFFSET_BASS + j];

	OpdMidiPitch aNote = seqBass[j][aSeq][seqBassStep]; 
	if (aNote != 0)
	{
		synthOsc[OFFSET_BASS + j].SetFreq(mtof(aNote) + synthSettings[j].oscDetune);
		seqSynthGate[j] = true;
	}
}



void leadPlay(int j)
{
	uint8_t aSeq = seqSongCurrent[OFFSET_LEAD + j];

	OpdMidiPitch aNote = seqLead[j][aSeq][seqLeadStep[j]].notePitch;

	if (aNote != 0)
	{
		synthOsc[OFFSET_LEAD + j].SetFreq(mtof(aNote) + synthSettings[OFFSET_LEAD + j].oscDetune);
		seqSynthGate[OFFSET_LEAD + j] = true;
	}

}



void drumPlay()
{
	uint8_t aVar = seqSongCurrent[OFFSET_DRUM];

	uint8_t aNote = seqDrum[aVar][seqDrumStep];

	if (aNote & DRUM_KICK)
	{
		kickAEnv.Trigger();
		kickPEnv.Trigger();
	}

	if (aNote & DRUM_SNARE)
	{
		snareNAEnv.Trigger();
		snareOAEnv.Trigger();
		snareOPEnv.Trigger();
	}

	if (aNote & DRUM_HHOPEN)
	{
		hihatNoiseHPF.SetFreq(drumSettings.hhoFilter);
		hihatNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.hhoDecay);
		hihatNoiseAEnv.Trigger();
	}

	if (aNote & DRUM_HHCLOSED)
	{
		hihatNoiseHPF.SetFreq(drumSettings.hhcFilter);
		hihatNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.hhcDecay);
		hihatNoiseAEnv.Trigger();
	}

	if (aNote & DRUM_CRASH)
	{
		crashNoiseHPF.SetFreq(drumSettings.crashFilter);
		crashNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.crashDecay);
		crashNoiseAEnv.Trigger();
	}

	if (aNote & DRUM_CLAP)
	{
		clapNoiseBPF.SetFreq(drumSettings.clapFilter);
		clapNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.clapDecay);
		clapNoiseAEnv.Trigger();
	}
}



void ProcessTick()
{
	if (sysPlay)
	{
		if (tick.Process())
		{
			sysTickCount++;

			// gate bass synth
			for (int i = 0; i < BASS_NUMBER; i++)
			{
				if (sysTickCount == seqBassGateTime[i])
				{
					seqSynthGate[i] = false;
				}
			}

			// has one 1/16 passed?
			if (sysTickCount == METRO_FACTOR)
			{
				sysTickCount = 0;	

				// blink every beat

				#ifndef OPD_MEASURE
				if (sysLedMode == LED_MODE_TEMPO)
				{
					hardware.SetLed(!(seqSongTicks % 4));
				}
				#endif

				// advance song step
				seqSongTicks++;
				if (seqSongTicks == SONG_TICKS_PER_STEP)
				{
					seqSongTicks = 0;
					if (sysPlayMode == PLAY_SONG)
					{
						seqSongStep++;
						if (seqSongStep == seqSongLen)
						{
							seqSongStep = 0;
						}
						uiRedraw = true;
					}
				}


				// bass

				seqBassStep++;
				if (seqBassStep == BASS_NOTES)
				{
					seqBassStep = 0;
					for (int j = 0; j < BASS_NUMBER; j++)
					{
						seqSongCurrent[OFFSET_BASS + j] = songStep[seqSongStep][OFFSET_BASS + j];
					}
				}

				for (int j = 0; j < BASS_NUMBER; j++)
				{
					if (seqVoiceOn[OFFSET_BASS + j])
					{
						bassPlay(j);
					}
				}


				// lead

				for (int j = 0; j < LEAD_NUMBER; j++)
				{
					seqLeadTime[j]++;

					uint8_t aSeq = seqSongCurrent[OFFSET_LEAD + j];

					// current lead seq is of len == 0 and bass is changing seq then read new seq
					if ((seqLeadLen[j][aSeq] == 0) && (seqSongTicks == 0))
					{
						seqSongCurrent[OFFSET_LEAD + j] = songStep[seqSongStep][OFFSET_LEAD + j];			
						aSeq = seqSongCurrent[OFFSET_LEAD + j];

						// TODO? start new note and zero seqLeadTime[]?
						// same for chord
						if (seqLeadLen[j][aSeq] > 0)
						{
							seqLeadTime[j] = 0;
							seqLeadStep[j] = 0;
							if (seqVoiceOn[j + OFFSET_LEAD])
							{
								leadPlay(j);
							}
						}

					} 			

					if (seqLeadLen[j][aSeq] > 0)
					{

						// is note finished?
						if (seqLeadTime[j] >= seqLead[j][aSeq][seqLeadStep[j]].noteTime)
						{
							seqLeadTime[j] = 0;
							// trigger release
							seqSynthGate[OFFSET_LEAD + j] = false;

							// TODO: env gate false!!! if we get another note directly after, restart env
							synthAEnv[OFFSET_LEAD + j].Process(seqSynthGate[OFFSET_LEAD + j]);

							// start next note
							seqLeadStep[j]++;
							if (seqLeadStep[j] == seqLeadLen[j][aSeq])
							{
								seqLeadStep[j] = 0;
								seqSongCurrent[OFFSET_LEAD + j] = songStep[seqSongStep][OFFSET_LEAD + j];
							}

							if (seqVoiceOn[j + OFFSET_LEAD])
							{
								leadPlay(j);
							}
						}
					} 
				}


				// drum

				seqDrumStep++;
				if (seqDrumStep == DRUM_NOTES)
				{
					seqDrumStep = 0;
					seqSongCurrent[OFFSET_DRUM] = songStep[seqSongStep][OFFSET_DRUM];
				}

				if (seqVoiceOn[OFFSET_DRUM])
				{
					drumPlay();
				}

			}
		}
	}
}



// used to play first note when starting
void PlaySeq()
{
	for (int j = 0; j < BASS_NUMBER; j++)
	{
		if (seqVoiceOn[j + OFFSET_BASS])
		{
			bassPlay(j);
		}
	}

	for (int j = 0; j < LEAD_NUMBER; j++)
	{
		if (seqVoiceOn[j + OFFSET_LEAD])
		{
			leadPlay(j);
		}
	}

	drumPlay();
}



void StopSeq()
{
	for (int j = 0; j < SYNTH_NUMBER; j++)
	{
		seqSynthGate[j] = false;
	}
}



// call when changing BPM
void CalcTick()
{
	sysTickFreq = sysTickFactor * sysTickBPM;
	tick.SetFreq(sysTickFreq);
}



// utility

void RebootToBootloader()
{
    // Initialize Boot Pin
    dsy_gpio_pin bootpin = {DSY_GPIOG, 3};
    dsy_gpio pin;
    pin.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin.pin = bootpin;
    dsy_gpio_init(&pin);

    // Pull Pin HIGH
    dsy_gpio_write(&pin, 1);

	// wait a few ms for cap to charge
	hardware.DelayMs(10);

    // Software Reset
	HAL_NVIC_SystemReset();
}



// transpose sequence
void UtilSeqTranspose(uint8_t aDirection, uint8_t aValue, uint8_t aTrackType, uint8_t aTrack, uint8_t aSeq)
{
	OpdMidiPitch aNote;
	int8_t aDirFact = (aDirection == UI_UTIL_DIR_UP ? +1 : -1);

	switch (aTrackType)
	{
	case UI_UTIL_TARGET_TRACK_B:
		if (aTrack < BASS_NUMBER)
		{
			for (int i = 0; i < BASS_NOTES; i++)
			{
				aNote = seqBass[aTrack][aSeq][i] + (aValue * aDirFact);
				if (aNote > 0 && aNote < 100)
				{
					seqBass[aTrack][aSeq][i] = aNote;
				}
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_L:
		if (aTrack < LEAD_NUMBER)
		{
			for (int i = 0; i < seqLeadLen[aTrack][aSeq]; i++)
			{
				aNote = seqLead[aTrack][aSeq][i].notePitch + (aValue * aDirFact);
				if (aNote > 0 && aNote < 100)
				{
					seqLead[aTrack][aSeq][i].notePitch = aNote;
				}
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_D:
		// do not transpose drums
		break;
	}

}



// shift sequence left/right
void UtilSeqShift(uint8_t aDirection, uint8_t aValue, uint8_t aTrackType, uint8_t aTrack, uint8_t aSeq)
{
	switch (aTrackType)
	{
	case UI_UTIL_TARGET_TRACK_B:
		if (aTrack < BASS_NUMBER)
		{
			OpdMidiPitch aNoteSaved;

			while (aValue)  
			{  
				if (aDirection == UI_UTIL_DIR_DOWN)  
				{  
				    aNoteSaved = seqBass[aTrack][aSeq][0];  
				    for(int i = 0; i < BASS_NOTES - 1; i++)
					{
						seqBass[aTrack][aSeq][i] = seqBass[aTrack][aSeq][i + 1];  
					}
				    seqBass[aTrack][aSeq][BASS_NOTES - 1] = aNoteSaved;  
				} else {  
				    aNoteSaved = seqBass[aTrack][aSeq][BASS_NOTES - 1];  
				    for(int i = BASS_NOTES - 1; i > 0; i--)
					{
				        seqBass[aTrack][aSeq][i] = seqBass[aTrack][aSeq][i - 1];  
		  			}
				    seqBass[aTrack][aSeq][0] = aNoteSaved;  
				}  		  
				aValue--;  
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_L:
		if (aTrack < LEAD_NUMBER)
		{
			OpdNote aNoteSaved;

			while (aValue)  
			{  
				if (aDirection == UI_UTIL_DIR_DOWN)  
				{  
				    aNoteSaved = seqLead[aTrack][aSeq][0];  
				    for(int i = 0; i < seqLeadLen[aTrack][aSeq] - 1; i++)
					{
						seqLead[aTrack][aSeq][i] = seqLead[aTrack][aSeq][i + 1];  
					}
				    seqLead[aTrack][aSeq][seqLeadLen[aTrack][aSeq] - 1] = aNoteSaved;  
				} else {  
				    aNoteSaved = seqLead[aTrack][aSeq][seqLeadLen[aTrack][aSeq] - 1];  
				    for(int i = seqLeadLen[aTrack][aSeq] - 1; i > 0; i--)
					{
				        seqLead[aTrack][aSeq][i] = seqLead[aTrack][aSeq][i - 1];  
		  			}
				    seqLead[aTrack][aSeq][0] = aNoteSaved;  
				}  		  
				aValue--;  
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_D:

		OpdMidiPitch aNoteSaved;

		while (aValue)  
		{  
			if (aDirection == UI_UTIL_DIR_DOWN)  
			{  
			    aNoteSaved = seqDrum[aSeq][0];  
			    for(int i = 0; i < DRUM_NOTES - 1; i++)
				{
					seqDrum[aSeq][i] = seqDrum[aSeq][i + 1];  
				}
			    seqDrum[aSeq][DRUM_NOTES - 1] = aNoteSaved;  
			} else {  
			    aNoteSaved = seqDrum[aSeq][DRUM_NOTES - 1];  
			    for(int i = DRUM_NOTES - 1; i > 0; i--)
				{
			        seqDrum[aSeq][i] = seqDrum[aSeq][i - 1];  
	  			}
			    seqDrum[aSeq][0] = aNoteSaved;  
			}  		  
			aValue--;  
		}

		break;
	}
}



// generate random sequence (based on aDirection and aValue)
void UtilSeqGenerate(uint8_t aDirection, uint8_t aValue, uint8_t aTrackType, uint8_t aTrack, uint8_t aSeq)
{
	OpdMidiPitch aNote;

	switch (aTrackType)
	{
	case UI_UTIL_TARGET_TRACK_B:
		if (aTrack < BASS_NUMBER)
		{
		    for (int i = 0; i < BASS_NOTES; i++)
			{
				if ((myrand() % 100) < aValue)
				{
					aNote = 36 + (myrand() % 12);
				} else {
					aNote = 36;
				}
				seqBass[aTrack][aSeq][i] = aNote + (aTrack * 12);
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_L:
		if (aTrack < LEAD_NUMBER)
		{
			// assume bass has same number of tracks and seqs as lead
			aNote = seqBass[aTrack][aSeq][0];
			if (aNote == 0)
			{
				aNote = 48;
			}
			// totalt length will be 2 bars
			int aLenLeft = t1 * 2;
			int aNumberOfNotes = 2 + (myrand() % 12);
			int aNoteLen;

			for (int i = 0; i < aNumberOfNotes; i++)
			{
				aNoteLen = t16 * (myrand() % 8);
				if (aNoteLen > aLenLeft)
				{
					aNoteLen = aLenLeft;
				}
				if ((myrand() % 100) < 75)
				{
					// note
					seqLead[aTrack][aSeq][i].notePitch = aNote + ((myrand() % 24) * ((float) aValue / 100));
					seqLead[aTrack][aSeq][i].noteTime = aNoteLen; 
				} else {
					// rest
					seqLead[aTrack][aSeq][i].notePitch = 0;
					seqLead[aTrack][aSeq][i].noteTime = aNoteLen;
				}
				aLenLeft -= aNoteLen;
				if (aLenLeft == 0)
				{
					break;
				}
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_D:
	    for (int i = 0; i < DRUM_NOTES; i++)
		{
			if ((i % 8) == 0)
			{
				seqDrum[aSeq][i] = DRUM_KICK;
			}
			if (((i  + 4) % 8) == 0)
			{
				seqDrum[aSeq][i] += DRUM_SNARE;
			}
			if ((i % 4) == 0)
			{
				seqDrum[aSeq][i] += DRUM_HHCLOSED;
			}
			if (((i + 2) % 4) == 0)
			{
				seqDrum[aSeq][i] += DRUM_HHOPEN;
			}
		}
		break;
	}
}



// copy sequence
void UtilSeqCopy(uint8_t aFromTrackType, uint8_t aFromTrack, uint8_t aFromSeq, uint8_t aToTrackType, uint8_t aToTrack, uint8_t aToSeq)
{
	// only copy between tracks of same type (for now)
	if (aFromTrackType == aToTrackType)
	{
		switch (aToTrackType)
		{
		case UI_UTIL_TARGET_TRACK_B:
			if ((aFromTrack < BASS_NUMBER) && (aToTrack < BASS_NUMBER))
			{
				for (int i = 0; i < BASS_NOTES; i++)
				{
					seqBass[aToTrack][aToSeq][i] = seqBass[aFromTrack][aFromSeq][i];
				}
			}
			break;
		case UI_UTIL_TARGET_TRACK_L:
			if ((aFromTrack < LEAD_NUMBER) && (aToTrack < LEAD_NUMBER))
			{
				for (int i = 0; i < seqLeadLen[aFromTrack][aFromSeq]; i++)
				{
					seqLead[aToTrack][aToSeq][i] =  seqLead[aFromTrack][aFromSeq][i];
				}
				seqLeadLen[aToTrack][aToSeq] = seqLeadLen[aFromTrack][aFromSeq];
			}
			break;
		case UI_UTIL_TARGET_TRACK_D:
			if ((aFromSeq < SEQ_NUMBER) && (aToSeq < SEQ_NUMBER) && (aFromSeq != aToSeq))
			{
				for (int i = 0; i < DRUM_NOTES; i++)
				{
					seqDrum[aToSeq][i] = seqDrum[aFromSeq][i];
				}
			}
			break;
		}
	}
}



// clear sequence
void UtilSeqClear(uint8_t aTrackType, uint8_t aTrack, uint8_t aSeq)
{
	switch (aTrackType)
	{
	case UI_UTIL_TARGET_TRACK_B:
		if (aTrack < BASS_NUMBER)
		{
		    for (int i = 0; i < BASS_NOTES; i++)
			{
				seqBass[aTrack][aSeq][i] = 0;
			}
		}
		break;
	case UI_UTIL_TARGET_TRACK_L:
		if (aTrack < LEAD_NUMBER)
		{
			seqLeadLen[aTrack][aSeq] = 0;
		}
		break;
	case UI_UTIL_TARGET_TRACK_D:
	    for (int i = 0; i < DRUM_NOTES; i++)
		{
			seqDrum[aSeq][i] = 0;
		}
		break;
	}
}




static OpdFlashConfig DSY_QSPI_BSS flashConfigLoad;

void UtilFlashSave()
{
	// uint32_t base = 0x90000000;
	OpdFlashConfig flashConfigSave;

	// copy data to flashConfigSave struct

	flashConfigSave.configVersion = FLASH_CONFIG_VERSION;
	memcpy(&flashConfigSave.mixSynth, &mixSynth, sizeof(mixSynth));
	flashConfigSave.mixDrum = mixDrum;

	memcpy(&flashConfigSave.synthSettings, &synthSettings, sizeof(synthSettings));
	flashConfigSave.drumSettings = drumSettings;

	memcpy(&flashConfigSave.songStep, &songStep, sizeof(songStep));
	flashConfigSave.seqSongLen = seqSongLen;

	memcpy(&flashConfigSave.seqBass, &seqBass, sizeof(seqBass));
	memcpy(&flashConfigSave.seqLead, &seqLead, sizeof(seqLead));
	memcpy(&flashConfigSave.seqLeadLen, &seqLeadLen, sizeof(seqLeadLen));
	memcpy(&flashConfigSave.seqDrum, &seqDrum, sizeof(seqDrum));

	//memcpy(&flashConfigSave.lfo, &lfo, sizeof(lfo));
	flashConfigSave.fxSettings = fxSettings;
	memcpy(&flashConfigSave.seqBassGateTime, &seqBassGateTime, sizeof(seqBassGateTime));
	memcpy(&flashConfigSave.seqVoiceOn, &seqVoiceOn, sizeof(seqVoiceOn));
	flashConfigSave.sysTickBPM = sysTickBPM;

	// init and set mode

	// hardware.qspi_handle.mode = DSY_QSPI_MODE_INDIRECT_POLLING;
	// dsy_qspi_init(&hardware.qspi_handle);

	size_t start_address = (size_t)&flashConfigLoad;
    size_t size = sizeof(flashConfigSave);
	size_t slot_address = (uint8_t)start_address;

	// erase

    // dsy_qspi_erase(base, base + sizeof(flashConfigSave));
    hardware.qspi.Erase(slot_address, slot_address + size);
    
	// write

    //dsy_qspi_write(base, sizeof(flashConfigSave), (uint8_t*)&flashConfigSave);
    hardware.qspi.Write(slot_address, size, (uint8_t*)&flashConfigSave);
    
	// de-init

	// dsy_qspi_deinit();

}



void UtilFlashLoad()
{

	// init and set mode

	// hardware.qspi_handle.mode = DSY_QSPI_MODE_DSY_MEMORY_MAPPED;
	// dsy_qspi_init(&hardware.qspi_handle);

	// Flash is memory mapped so no need to read

	// copy data from opdLoadConfig struct to data

	if (flashConfigLoad.configVersion == FLASH_CONFIG_VERSION)
	{
		memcpy(&mixSynth, &flashConfigLoad.mixSynth, sizeof(mixSynth));
		mixDrum = flashConfigLoad.mixDrum;

		memcpy(&synthSettings, &flashConfigLoad.synthSettings, sizeof(synthSettings));
		drumSettings = flashConfigLoad.drumSettings;

		memcpy(&songStep, &flashConfigLoad.songStep, sizeof(songStep));
		seqSongLen = flashConfigLoad.seqSongLen;

		memcpy(&seqBass, &flashConfigLoad.seqBass, sizeof(seqBass));
		memcpy(&seqLead, &flashConfigLoad.seqLead, sizeof(seqLead));
		memcpy(&seqLeadLen, &flashConfigLoad.seqLeadLen, sizeof(seqLeadLen));
		memcpy(&seqDrum, &flashConfigLoad.seqDrum, sizeof(seqDrum));

		//memcpy(&lfo, &flashConfigLoad.lfo, sizeof(lfo));
		fxSettings = flashConfigLoad.fxSettings;
		memcpy(&seqBassGateTime, &flashConfigLoad.seqBassGateTime, sizeof(seqBassGateTime));
		memcpy(&seqVoiceOn, &flashConfigLoad.seqVoiceOn, sizeof(seqVoiceOn));
		sysTickBPM = flashConfigLoad.sysTickBPM;
	}

	// de-init

	// dsy_qspi_deinit();

}



bool UtilMIDIExportInSong(int aTrack, int aSequence)
{
	bool found = false;

	for (int i = 0; i < seqSongLen; i++)
	{
		if (songStep[i][aTrack] == aSequence)
		{
			found = true;
			break;
		}
	}

	return (found);
}

#define MIDI_UPB 96 // MIDI Units Per Beat, 4 * 1/16th


void UtilMIDIExport()
{
	#ifdef OPD_LOGG

	// write XML header
	hardware.PrintLine("<?xml version='1.0' encoding='UTF-8' ?>");

	hardware.PrintLine("<opd upb='96'>");

	// only export the sequences we use in the song

	// write bass sequences

	for (int i = 0; i < BASS_NUMBER; i++)
	{
		hardware.PrintLine("<track number='%d'>", i);
		for (int j = 0; j < SEQ_NUMBER; j++)
		{
			if (UtilMIDIExportInSong(OFFSET_BASS + i, j))
			{
				// export this sequence
				hardware.PrintLine("<seq number='%d'>", j);
				for (int k = 0; k < BASS_NOTES; k++)
				{
					hardware.PrintLine("<note>");
					hardware.PrintLine("<pitch>%d</pitch>", seqBass[i][j][k]);
					hardware.PrintLine("<length>%d</length>", MIDI_UPB / 4);
					hardware.PrintLine("</note>");
				}
				hardware.PrintLine("</seq>");
			}
		}
		hardware.PrintLine("</track>");
	}

	// write lead sequences

	for (int i = 0; i < LEAD_NUMBER; i++)
	{
		hardware.PrintLine("<track number='%d'>", i + OFFSET_LEAD);
		for (int j = 0; j < SEQ_NUMBER; j++)
		{
			if (UtilMIDIExportInSong(OFFSET_LEAD + i, j))
			{
				// export this sequence
				hardware.PrintLine("<seq number='%d'>", j);
				for (int k = 0; k < seqLeadLen[i][j]; k++)
				{
					hardware.PrintLine("<note>");
					hardware.PrintLine("<pitch>%d</pitch>", seqLead[i][j][k]);
					hardware.PrintLine("<length>%d</length>", MIDI_UPB / 4);
					hardware.PrintLine("</note>");
				}
				hardware.PrintLine("</seq>");
			}
		}
		hardware.PrintLine("</track>");
	}

	// write drum sequences

	hardware.PrintLine("<track number='%d'>", 10);
	for (int j = 0; j < SEQ_NUMBER; j++)
	{
		if (UtilMIDIExportInSong(OFFSET_DRUM, j))
		{
			// export this sequence
			hardware.PrintLine("<seq number='%d'>", j);
			for (int k = 0; k < DRUM_NOTES; k++)
			{
				hardware.PrintLine("<note>");
				hardware.PrintLine("<pitch>%d</pitch>", seqDrum[j][k]);
				hardware.PrintLine("<length>%d</length>", MIDI_UPB / 4);
				hardware.PrintLine("</note>");
			}
			hardware.PrintLine("</seq>");
		}
	}
	hardware.PrintLine("</track>");

	hardware.PrintLine("</opd>");
	#endif
}



int main(void)
{
	// init hardware
	hardware.Configure();
	hardware.Init();
	sysSampleRate = hardware.AudioSampleRate();
	sysCallbackRate = hardware.AudioCallbackRate();


	// Set up note data
	SetupSeq();


	// Set up synths and drums
	SetupSynth();
	SetupDrums();


	// set up fx
	SetupFX();


	// set up mix
	SetupMix();
	CalcMix();


    // Configure and initialize button
    Switch button1;
    button1.Init(hardware.GetPin(PIN_BUTTON_UPLOAD), 10);


	// LCD buttons
	// Configure pin as an ADC input
	adcLCDButtons.InitSingle(hardware.GetPin(PIN_BUTTONS));
	hardware.adc.Init(&adcLCDButtons, 1);
	// Start reading values
	hardware.adc.Start();


	// LCD
	LcdHD44780::Config lcd_config;
	lcd_config.cursor_on = true;
	lcd_config.cursor_blink = false;
	lcd_config.rs = hardware.GetPin(PIN_LCD_RS);
	lcd_config.en = hardware.GetPin(PIN_LCD_EN);
	lcd_config.d4 = hardware.GetPin(PIN_LCD_D4);
	lcd_config.d5 = hardware.GetPin(PIN_LCD_D5);
	lcd_config.d6 = hardware.GetPin(PIN_LCD_D6);
	lcd_config.d7 = hardware.GetPin(PIN_LCD_D7);
	lcd.Init(lcd_config);


	// UI
	OscUI ui;
	ui.Init(&lcd);

	// start all voices
	//PlaySeq();


	// global setup
	sysPlay = false;
	sysPlayMode = PLAY_OFF;
	sysLedMode = LED_MODE_TEMPO;
	sysClip = false;


	// logging and MIDI export over serial USB
	#ifdef OPD_LOGG
	hardware.StartLog(false); // start log but don't wait for PC - we can be connected to a battery
	#endif

	// let everything settle (esp the LCD)
	System::Delay(100);


	// Start metronome
	CalcTick();
	tick.Init(sysTickFreq, sysCallbackRate);
	sysTickCount = 0;


	#ifdef OPD_MEASURE
	// setup measurement
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	#endif

	// Start calling the audio callback
	hardware.StartAudio(AudioCallback);


	// Loop forever
	for(;;)
	{
		// handle UI

		float adcButton = hardware.adc.GetFloat(0);
		ui.Button(adcButton);
		ui.Work();
		ui.Draw();


		// LED is working as clip notifier in OVERVIEW: MIXER

		if (sysLedMode == LED_MODE_CLIP)
		{
			if (sysClip)
			{
				hardware.SetLed(true);
				sysLedOnTime = System::GetNow();
				sysClip = false;

			} else if ((System::GetNow() - sysLedOnTime) > 200)
			{
				hardware.SetLed(false);
				sysClip = false;
			}
		}


		// Reset to upload?
		button1.Debounce();
		if (button1.Pressed())
		{
			RebootToBootloader();
		}


		// wait
		System::Delay(25);
	}
}
