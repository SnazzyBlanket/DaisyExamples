/*
  
Project: OscPocketD/Base
Description: Base tool for Daisy
Author: Staffan Melin, staffan.melin@oscillator.se
License: GNU General Public License v3.0
Version: 20210907
Project site: http://www.oscillator.se/opensource

*/

//#define OPD_LOGG // start serial over USB Logger class
//#define OPD_MEASURE // measure MCU utilization
#define OPD_BASE_MIDI // version 2 with MIDI, other LCD pins, no CV1 in, no Gate0/1 out


#include "daisy_seed.h"
#include "daisysp.h"

#include "stm32h7xx_hal.h" // for HAL_NVIC_SystemReset();
//extern "C" void HAL_NVIC_SystemReset();
#include "core_cm7.h"
#include "dev/lcd_hd44780.h"

#include "main.h"
#include "ui.h"
#include "sm.h"
#include "slicer.h"

#include <string.h>

using namespace daisy;
using namespace daisysp;





// globals

DaisySeed hardware;
//MidiHandler midi;
MidiUartHandler midi;


float sysSampleRate;
float sysCallbackRate;

bool uiRedraw = true;



// hardware
LcdHD44780 lcd;
AdcChannelConfig adcConfig[AD_MAX];
dsy_gpio gateOut0, gateOut1;


// FX
OpdFXSettings fxSettings;
OpdModSources modSources;

// osc
Oscillator fxOsc, fxOsc2;

// adsr
Adsr fxAdsr;

// filter
Svf fxFilterL, fxFilterR;

// decimator
Decimator fxDecimatorL;
Decimator fxDecimatorR;

// overdrive
Overdrive fxOverdrive;

// delay
#define DELAY_MAX static_cast<size_t>(48000 * DELAY_MAX_S)
// 7 * 48000 * 2 = 672000 * (size of float)
// https://forum.electro-smith.com/t/daisy-crashes-using-reverbsc-with-96k-sample-rate/311/2
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS fxDelayL;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS fxDelayR;

// chorus
Chorus fxChorus;

// reverb
ReverbSc fxReverb;

// slicer
static Slicer<SLICER_MAX> DSY_SDRAM_BSS fxSlicer;

// mog - special modulators
SM smMod[SM_NUMBER];

// mod - lfo
Oscillator lfoOsc[LFO_NUMBER];

// mod - eg
Adsr egAdsr[EG_NUMBER];

// audio callback

//void AudioCallback(float* in, float* out, size_t size)
void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
	float sigL, sigR;
	float outTempL, outTempR;
	float outDelayL, outDelayR;
	float sigRevL, sigRevR;
	float sigOutRevL, sigOutRevR;
	float oscFreq1, oscFreq2;
	float sigTemp;

	#ifdef OPD_MEASURE
	// measure - start
	DWT->CYCCNT = 0;
	#endif
	
	// audio

	for (size_t n = 0; n < size; n += 2)
	{

		// sm - special modulators
		for (int8_t i = 0; i < SM_NUMBER; i++)
		{
			modSources.smValue[i] = smMod[i].Process();
			modSources.smGate[i] = smMod[i].Gate();
		}
		
		// lfo
		for (int8_t i = 0; i < LFO_NUMBER; i++)
		{
			modSources.lfoValue[i] = lfoOsc[i].Process();
		}

		// eg
		for (int8_t i = 0; i < EG_NUMBER; i++)
		{
			modSources.egValue[i] = egAdsr[i].Process(ModFactor(fxSettings.egGateMod[i]) > fxSettings.egGateLevel[i]);
		}

		// input
		switch (fxSettings.inputChannel)
		{
		case INPUT_CHANNEL_STEREO:
			sigL = in[n];
			sigR = in[n + 1];
			break;
		case INPUT_CHANNEL_LEFT:
			sigL = in[n];
			sigR = sigL;
			break;
		case INPUT_CHANNEL_RIGHT:
			sigR = in[n + 1];
			sigL = sigR;
			break;
		case INPUT_CHANNEL_MERGE:
			sigR = 0;
			sigL = 0;
			break;
		default:
			sigL = in[n];
			sigR = in[n + 1];
		}
		
		// audio in as mod source
		modSources.audioIn = (sigL + sigR) / 2.0f;
		
		// gain
		sigL *= fxSettings.gainInputL;
		sigR *= fxSettings.gainInputR;
		
		// osc

		if (fxSettings.oscOn)
		{
			// modulation
			if (fxSettings.oscFreqMod != MOD_SOURCE_NONE)
			{
				// modulation exception: MIDI in pitch to frequency
				if (fxSettings.oscFreqMod == MOD_SOURCE_MIDIP)
				{
					oscFreq1 = mtof(modSources.midiPitch) + fxSettings.oscDetune;
				} else {
					oscFreq1 = powf(2.0f, (ModFactor(fxSettings.oscFreqMod) * 3.3f)) * 60 + fxSettings.oscDetune;
				}
			} else {
				oscFreq1 = fxSettings.oscFreq + fxSettings.oscDetune;
			}
			fxOsc.SetFreq(oscFreq1);
			
			if (fxSettings.oscAmpMod != MOD_SOURCE_NONE)
			{
				fxOsc.SetAmp(fxSettings.oscAmp * ModFactor(fxSettings.oscAmpMod));
			}
			
			sigTemp = fxOsc.Process();
			sigL += sigTemp;
			sigR += sigTemp;
			
			// osc2
			if (fxSettings.oscDetune2 > 0)
			{
				// modulation
				if (fxSettings.oscDetune2Mod == MOD_SOURCE_NONE)
				{
					oscFreq2 = oscFreq1 + fxSettings.oscDetune2;
				} else {
					// modulation exception: MIDI in pitch to frequency
					if (fxSettings.oscFreqMod == MOD_SOURCE_MIDIP)
					{
						oscFreq2 = mtof(modSources.midiPitch) + ModFactor(fxSettings.oscDetune2Mod) * fxSettings.oscDetune2;
					} else {
						oscFreq2 = oscFreq1 + ModFactor(fxSettings.oscDetune2Mod) * fxSettings.oscDetune2;
					}
				}
				fxOsc2.SetFreq(oscFreq2);
				
				if (fxSettings.oscAmp2Mod != MOD_SOURCE_NONE)
				{
					fxOsc2.SetAmp(fxSettings.oscAmp2 * ModFactor(fxSettings.oscAmp2Mod));
				}
				
				sigTemp = fxOsc2.Process();
				sigL += sigTemp;
				sigR += sigTemp;
			}
		}
		
		// slicer
		
		if (fxSettings.slicerOn)
		{
			if (fxSettings.slicerRSMMod != MOD_SOURCE_NONE)
			{
				if (ModFactor(fxSettings.slicerRSMMod) > fxSettings.slicerRSMModGateLevel)
				{
					fxSlicer.Trig(true);
				}
			}
			if (fxSettings.slicerPRMMod != MOD_SOURCE_NONE)
			{
				if (ModFactor(fxSettings.slicerPRMMod) > fxSettings.slicerPRMModGateLevel)
				{
					fxSlicer.Trig(false);
				}
			}
			sigL = fxSlicer.Process(sigL);
			sigR = sigL;
		}

		// decimator
		
		if (fxSettings.decimatorOn)
		{
			// modulation
			if (fxSettings.decimatorBitcrushFactorMod != MOD_SOURCE_NONE)
			{
				fxDecimatorL.SetBitcrushFactor(fxSettings.decimatorBitcrushFactor * ModFactor(fxSettings.decimatorBitcrushFactorMod));
				fxDecimatorR.SetBitcrushFactor(fxSettings.decimatorBitcrushFactor * ModFactor(fxSettings.decimatorBitcrushFactorMod));
			}

			sigL = fxDecimatorL.Process(sigL);
			sigR = fxDecimatorR.Process(sigR);
		}

		// overdrive
		
		if (fxSettings.overdriveOn)
		{
			// gain
			sigL *=  fxSettings.overdriveGain * ModFactor(fxSettings.overdriveGainMod);
			sigR *=  fxSettings.overdriveGain * ModFactor(fxSettings.overdriveGainMod);

			// drive - modulation
			if (fxSettings.overdriveDriveMod != MOD_SOURCE_NONE)
			{
				fxOverdrive.SetDrive(fxSettings.overdriveDrive * ModFactor(fxSettings.overdriveDriveMod));
			}

			// overdrive has no state so we can apply it to both L and R
			sigL = fxOverdrive.Process(sigL);
			sigR = fxOverdrive.Process(sigR);
		}
		
		// filter

		if (fxSettings.filterOn)
		{
			// modulation
			if (fxSettings.filterFreqMod != MOD_SOURCE_NONE)
			{
				fxFilterL.SetFreq(fxSettings.filterFreq * ModFactor(fxSettings.filterFreqMod));
				fxFilterR.SetFreq(fxSettings.filterFreq * ModFactor(fxSettings.filterFreqMod));
			}
			if (fxSettings.filterResMod != MOD_SOURCE_NONE)
			{
				fxFilterL.SetRes(fxSettings.filterRes * ModFactor(fxSettings.filterResMod));
				fxFilterR.SetRes(fxSettings.filterRes * ModFactor(fxSettings.filterResMod));
			}

			fxFilterL.Process(sigL);
			fxFilterR.Process(sigR);

			switch (fxSettings.filterType)
			{
			case FILTER_TYPE_LOW:
				sigL = fxFilterL.Low();
				sigR = fxFilterR.Low();
				break;
			case FILTER_TYPE_HIGH:
				sigL = fxFilterL.High();
				sigR = fxFilterR.High();
				break;
			case FILTER_TYPE_BAND:
				sigL = fxFilterL.Band();
				sigR = fxFilterR.Band();
				break;
			}
		}

		// ADSR
		
		if (fxSettings.adsrOn)
		{
			if (fxSettings.adsrSustainMod != MOD_SOURCE_NONE)
			{
				fxAdsr.SetSustainLevel(fxSettings.adsrSustain * ModFactor(fxSettings.adsrSustainMod));
			}
			
			if (fxSettings.adsrGateMod == MOD_SOURCE_MIDIV)
			{
				sigTemp = fxAdsr.Process(modSources.midiVelocity > 0);
			} else {
				// gate
				sigTemp = fxAdsr.Process(ModFactor(fxSettings.adsrGateMod) > fxSettings.adsrGateLevel);
			}
	    	sigL *= sigTemp;
	    	sigR *= sigTemp;
		}
		
		// pan
		
		if (fxSettings.panOn)
		{
			if (fxSettings.panPanMod != MOD_SOURCE_NONE)
			{
				sigTemp = ModFactor(fxSettings.panPanMod);
			} else {
				sigTemp = fxSettings.panPan;
			}
			sigL *= (0.99f - sigTemp);
			sigR *= sigTemp;
		}

		// delay

		if (fxSettings.delayOn)
		{
			outDelayL = fxDelayL.Read();
			outTempL = sigL;
			sigL = outTempL + outDelayL;
			fxDelayL.Write(sigL * fxSettings.delayFeedbackL);

			outDelayR = fxDelayR.Read();
			outTempR = sigR;
			sigR = outTempR + outDelayR;
			fxDelayR.Write(sigR * fxSettings.delayFeedbackR);
		}

		// chorus

		if (fxSettings.chorusOn)
		{
			fxChorus.Process((sigL + sigR) / 2.0f);
			sigL = sigL * fxSettings.chorusDry + fxChorus.GetLeft() * fxSettings.chorusWet;
			sigR = sigR * fxSettings.chorusDry + fxChorus.GetRight() * fxSettings.chorusWet;
		}
		
		// audio merge
		
		if (fxSettings.inputChannel == INPUT_CHANNEL_MERGE)
		{
			sigL += in[n];
			sigR += in[n + 1];
		}

		// reverb send

		if (fxSettings.reverbOn)
		{	
			sigRevL = sigL * fxSettings.reverbWet;
			sigRevR = sigR * fxSettings.reverbWet;
			fxReverb.Process(sigRevL, sigRevR, &sigOutRevL, &sigOutRevR);
			sigL = sigL * fxSettings.reverbDry + sigOutRevL;
			sigR = sigR * fxSettings.reverbDry + sigOutRevR;
		} else {
			sigRevL = 0;
			sigRevR = 0;
		}
		
		// gain
		sigL *= fxSettings.gainOutputL;
		sigR *= fxSettings.gainOutputR;

		out[n] = sigL;
		out[n + 1] = sigR;
	}



	#ifdef OPD_MEASURE
	// measure - stop
	if (DWT->CYCCNT > 390000)
	{
		hardware.SetLed(true);
	}
	#endif
}



float ModFactor(uint8_t aModSource)
{
	float aFactor = 1.0f;

	switch (aModSource)
	{
	case MOD_SOURCE_LFO0:
		aFactor = modSources.lfoValue[0] + fxSettings.lfoOffset[0];
		break;
	case MOD_SOURCE_LFO1:
		aFactor = modSources.lfoValue[1] + fxSettings.lfoOffset[2];
		break;
	case MOD_SOURCE_LFO2:
		aFactor = modSources.lfoValue[2] + fxSettings.lfoOffset[2];
		break;
	case MOD_SOURCE_CV0:
		aFactor = (modSources.cvValue[0] * fxSettings.cvAmp[0]) + fxSettings.cvOffset[0];
		break;
	case MOD_SOURCE_CV1:
		aFactor = (modSources.cvValue[1] * fxSettings.cvAmp[1]) + fxSettings.cvOffset[1];
		break;
	case MOD_SOURCE_GATE0:
		aFactor = (modSources.cvValue[2] * fxSettings.cvAmp[2]) + fxSettings.cvOffset[2];
		break;
	case MOD_SOURCE_GATE1:
		aFactor = (modSources.cvValue[3] * fxSettings.cvAmp[3]) + fxSettings.cvOffset[3];
		break;
	case MOD_SOURCE_POT0:
		aFactor = (modSources.cvValue[4] * fxSettings.cvAmp[4]) + fxSettings.cvOffset[4];
		break;
	case MOD_SOURCE_POT1:
		aFactor = (modSources.cvValue[5] * fxSettings.cvAmp[5]) + fxSettings.cvOffset[5];
		break;
	case MOD_SOURCE_MIDIP:
		aFactor = (modSources.midiPitch / 127.0f);
		break;
	case MOD_SOURCE_MIDIV:
		aFactor = (modSources.midiVelocity / 127.0f);
		break;
	case MOD_SOURCE_SM0:
		aFactor = modSources.smValue[0];
		break;
	case MOD_SOURCE_SM1:
		aFactor = modSources.smValue[1];
		break;
	case MOD_SOURCE_SM2:
		aFactor = modSources.smValue[2];
		break;
	case MOD_SOURCE_SMT0:
		aFactor = modSources.smGate[0];
		break;
	case MOD_SOURCE_SMT1:
		aFactor = modSources.smGate[1];
		break;
	case MOD_SOURCE_SMT2:
		aFactor = modSources.smGate[2];
		break;
	case MOD_SOURCE_AUDIO:
		aFactor = modSources.audioIn;
		break;
	case MOD_SOURCE_EG0:
		aFactor = modSources.egValue[0];
		break;
	case MOD_SOURCE_EG1:
		aFactor = modSources.egValue[1];
		break;
	default:
		aFactor = 1;
	}

	return aFactor;
}			



// setup fx

void SetupFX()
{
	// osc
	fxSettings.oscOn = false;
	fxSettings.oscFreq = 440.0f;
	fxSettings.oscFreqMod = MOD_SOURCE_NONE;
	fxSettings.oscDetune = 0.0f;
	fxSettings.oscWaveform = 0;
	fxSettings.oscAmp = 0.2f;
	fxSettings.oscAmpMod = MOD_SOURCE_NONE;
	
	fxSettings.oscDetune2 = 0;
	fxSettings.oscDetune2Mod = MOD_SOURCE_NONE;
	fxSettings.oscWaveform2 = 0;
	fxSettings.oscAmp2 = 0.2f;
	fxSettings.oscAmp2Mod = MOD_SOURCE_NONE;
	
	fxOsc.Init(sysSampleRate);
	fxOsc2.Init(sysSampleRate);
	FXOscSet();

	// ADSR
	fxSettings.adsrOn = false;
	fxSettings.adsrAttack = 0.01f;
	fxSettings.adsrDecay = 0.01f;
	fxSettings.adsrSustain = 1.0f;
	fxSettings.adsrRelease = 0.1f;
	fxSettings.adsrGateLevel = 0.5f;
	fxSettings.adsrSustainMod = MOD_SOURCE_NONE;
	fxSettings.adsrGateMod = MOD_SOURCE_NONE;
	fxAdsr.Init(sysSampleRate);
	FXAdsrSet();

	// filter
	fxSettings.filterOn = false;
	fxSettings.filterFreq = 1000.0f;
	fxSettings.filterFreqMod = MOD_SOURCE_NONE;
	fxSettings.filterRes = 0.0f;
	fxSettings.filterType = FILTER_TYPE_LOW;

	fxFilterL.Init(sysSampleRate);
	fxFilterL.SetDrive(0.0f); // default
	fxFilterR.Init(sysSampleRate);
	fxFilterR.SetDrive(0.0f); // default
	FXFilterSet();

	// decimator
	fxSettings.decimatorOn = false;
	fxSettings.decimatorDownsampleFactor = 0.5f;
	fxSettings.decimatorBitcrushFactor = 0.5f;
	fxSettings.decimatorBitcrushFactorMod = MOD_SOURCE_NONE;
	fxSettings.decimatorBitsToCrush = 8;

	fxDecimatorL.Init();
	fxDecimatorR.Init();
	FXDecimatorSet();
	
	// overdrive
	fxSettings.overdriveOn = false;
	fxSettings.overdriveGain = 1.0f;
	fxSettings.overdriveGainMod = MOD_SOURCE_NONE;
	fxSettings.overdriveDrive = 0.5f;
	fxSettings.overdriveDriveMod = MOD_SOURCE_NONE;
	
	fxOverdrive.Init();
	FXOverdriveSet();
	
	// pan
	fxSettings.panOn = false;
	fxSettings.panPan = 0.50f;
	fxSettings.panPanMod = MOD_SOURCE_NONE;

	// delay
	fxSettings.delayOn = false;
	fxSettings.delayDelayL = 0.3f;
	fxSettings.delayFeedbackL = 0.3f;
	fxSettings.delayDelayR = 0.4f;
	fxSettings.delayFeedbackR = 0.3f;

	fxDelayL.Init();
	fxDelayR.Init();
	FXDelaySet();

	// flanger
	fxSettings.flangerOn = false;
	
	// chorus
	fxSettings.chorusOn = false;
	fxSettings.chorusDelay = 0.5f; // 0-1
	fxSettings.chorusFeedback = 0.5f; // 0-1
	fxSettings.chorusLfoDepth = 0.5; // 0-1
	fxSettings.chorusLfoFreq = 0.5f; // Hz
	fxSettings.chorusPan = 0.5f; // 0-1
	fxSettings.chorusDry = 0.50f;
	fxSettings.chorusWet = 0.50f;
	
	fxChorus.Init(sysSampleRate);
	FXChorusSet();

	// reverb
	fxSettings.reverbOn = false;
	fxSettings.reverbFeedback = 0.3f;
	fxSettings.reverbLPFFreq = 8000;
	fxSettings.reverbDry = 0.50f;
	fxSettings.reverbWet = 0.50f;

	fxReverb.Init(sysSampleRate);
	FXReverbSet();
	
	// slicer
	fxSettings.slicerOn = false;
	fxSettings.slicer_record_samples_max = 24000;
	fxSettings.slicerRSMMod = MOD_SOURCE_NONE;
	fxSettings.slicerRSMModGateLevel = 0.50f;
	fxSettings.slicer_playback_rep_max = 20;
	fxSettings.slicerPRMMod = MOD_SOURCE_NONE;
	fxSettings.slicerPRMModGateLevel = 0.50f;
	fxSettings.slicerTrigMode = false;
	fxSlicer.Init(sysSampleRate);
	FXSlicerSet();

	// mod - special modulator
	for (int i = 0; i < SM_NUMBER; i++)
	{
		fxSettings.smType[i] = SM_TYPE_NOISE;
		fxSettings.smFreq[i] = 100.0f;
		fxSettings.smAmp[i] = 1.0f;
		fxSettings.smOffset[i] = 0.0f;
		fxSettings.smSyncMod[i] = MOD_SOURCE_NONE;
		fxSettings.smSyncLevel[i] = 0.5f;
		smMod[i].Init(i, sysSampleRate, fxSettings.smType[i], fxSettings.smFreq[i], fxSettings.smAmp[i], fxSettings.smOffset[i]);
		FXSMSet(i);
		
		// seq
		fxSettings.smSeqLen[i] = 1;
		for (int j = 0; j < SEQ_STEPS; j++)
		{
			fxSettings.smSeqMidi[i][j] = 36;
			fxSettings.smSeqVal[i][j] = mtoval(fxSettings.smSeqMidi[i][j]);
		}
	}
	fxSettings.smGateLen = 0.5f;
	
	
	// mog - lfo
	for (int i = 0; i < LFO_NUMBER; i++)
	{
		fxSettings.lfoWaveform[i] = 0;
		fxSettings.lfoFreq[i] = 10.0f;
		fxSettings.lfoAmp[i] = 1.0f;
		fxSettings.lfoOffset[i] = 0.5f;
		lfoOsc[i].Init(sysSampleRate);
		FXLFOSet(i);
	}

	// mod - eg
	for (int i = 0; i < EG_NUMBER; i++)
	{
		fxSettings.egAttack[i] = 0.01f;
		fxSettings.egDecay[i] = 0.01f;
		fxSettings.egSustain[i] = 1.0f;
		fxSettings.egRelease[i] = 0.1f;
		fxSettings.egGateMod[i] = MOD_SOURCE_NONE;
		fxSettings.egGateLevel[i] = 0.50f;

		egAdsr[i].Init(sysSampleRate);
		FXEGSet(i);
	}
	
	// mod - CV
	for (int i = 0; i < CV_NUMBER; i++)
	{
		fxSettings.cvAmp[i] = 1.0f;
		fxSettings.cvOffset[i] = 0.0f;
	}

	// output
	fxSettings.outCV0Mod = MOD_SOURCE_NONE;
	fxSettings.outCV1Mod = MOD_SOURCE_NONE;
	fxSettings.outGate0Mod = MOD_SOURCE_NONE;
	fxSettings.outGate1Mod = MOD_SOURCE_NONE;
}



void FXOscSet()
{
	fxOsc.SetWaveform(fxSettings.oscWaveform);
	fxOsc.SetAmp(fxSettings.oscAmp);
	fxOsc.SetFreq(fxSettings.oscFreq + fxSettings.oscDetune);
	
	fxOsc2.SetWaveform(fxSettings.oscWaveform2);
	fxOsc2.SetAmp(fxSettings.oscAmp2);
	fxOsc2.SetFreq(fxSettings.oscFreq + fxSettings.oscDetune2);
}



void FXAdsrSet()
{
	fxAdsr.SetTime(ADSR_SEG_ATTACK, MAX(0.01, fxSettings.adsrAttack));
	fxAdsr.SetTime(ADSR_SEG_DECAY, MAX(0.01, fxSettings.adsrDecay));
	fxAdsr.SetSustainLevel(fxSettings.adsrSustain);
	fxAdsr.SetTime(ADSR_SEG_RELEASE, fxSettings.adsrRelease);
}



void FXFilterSet()
{
	fxFilterR.SetFreq(fxSettings.filterFreq);
	fxFilterL.SetFreq(fxSettings.filterFreq);
	fxFilterR.SetRes(fxSettings.filterRes);
	fxFilterL.SetRes(fxSettings.filterRes);
}



void FXDecimatorSet()
{
	fxDecimatorL.SetBitsToCrush(fxSettings.decimatorBitsToCrush);
	fxDecimatorL.SetDownsampleFactor(fxSettings.decimatorDownsampleFactor);
	fxDecimatorL.SetBitcrushFactor(fxSettings.decimatorBitcrushFactor);

	fxDecimatorR.SetBitsToCrush(fxSettings.decimatorBitsToCrush);
	fxDecimatorR.SetDownsampleFactor(fxSettings.decimatorDownsampleFactor);
	fxDecimatorR.SetBitcrushFactor(fxSettings.decimatorBitcrushFactor);
}



void FXOverdriveSet()
{
	fxOverdrive.SetDrive(fxSettings.overdriveDrive);
}



void FXChorusSet()
{
	fxChorus.SetDelay(fxSettings.chorusDelay);
	fxChorus.SetFeedback(fxSettings.chorusFeedback);
	fxChorus.SetLfoDepth(fxSettings.chorusLfoDepth);
	fxChorus.SetLfoFreq(fxSettings.chorusLfoFreq);
	fxChorus.SetPan(fxSettings.chorusPan);
}



void FXDelaySet()
{
	fxDelayL.SetDelay(sysSampleRate * fxSettings.delayDelayL);
	fxDelayR.SetDelay(sysSampleRate * fxSettings.delayDelayR);
}



void FXReverbSet()
{
	fxReverb.SetFeedback(fxSettings.reverbFeedback);
	fxReverb.SetLpFreq(fxSettings.reverbLPFFreq);
}



void FXSlicerSet()
{
	fxSlicer.SetRecordMax(fxSettings.slicer_record_samples_max);
	fxSlicer.SetPlaybackMax(fxSettings.slicer_playback_rep_max);
	fxSlicer.SetTrigMode(fxSettings.slicerTrigMode);
}



void FXSMSet(uint8_t aSM)
{
	smMod[aSM].SetType(fxSettings.smType[aSM]);
	smMod[aSM].SetFreq(fxSettings.smFreq[aSM]);
	smMod[aSM].SetAmp(fxSettings.smAmp[aSM]);
	smMod[aSM].SetOffset(fxSettings.smOffset[aSM]);
}



void FXLFOSet(uint8_t aLFO)
{
	lfoOsc[aLFO].SetWaveform(fxSettings.lfoWaveform[aLFO]);
	lfoOsc[aLFO].SetAmp(fxSettings.lfoAmp[aLFO]);
	lfoOsc[aLFO].SetFreq(fxSettings.lfoFreq[aLFO]);
}



void FXEGSet(uint8_t aEG)
{
	egAdsr[aEG].SetTime(ADSR_SEG_ATTACK, MAX(0.01, fxSettings.egAttack[aEG]));
	egAdsr[aEG].SetTime(ADSR_SEG_DECAY, MAX(0.01, fxSettings.egDecay[aEG]));
	egAdsr[aEG].SetSustainLevel(fxSettings.egSustain[aEG]);
	egAdsr[aEG].SetTime(ADSR_SEG_RELEASE, fxSettings.egRelease[aEG]);
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



static OpdFXSettings DSY_QSPI_BSS fxSettingsFlash;

void UtilFlashSave()
{
//	uint32_t base = 0x90000000;

	// init and set mode

	// hardware.qspi_handle.mode = DSY_QSPI_MODE_INDIRECT_POLLING;
	// dsy_qspi_init(&hardware.qspi_handle);

	size_t start_address = (size_t)&fxSettingsFlash;
    size_t size = sizeof(fxSettings);
	size_t slot_address = start_address;

	// erase
	
	// size_t base = (size_t)&fxSettingsFlash;

	// dsy_qspi_erase(base, base + sizeof(OpdFXSettings));
    hardware.qspi.Erase(slot_address, slot_address + size);

	// write

	//dsy_qspi_write(base, sizeof(fxSettings), (uint8_t*)&fxSettings);
    hardware.qspi.Write(slot_address, size, (uint8_t*)&fxSettings);

	// de-init

	//dsy_qspi_deinit();

}



void UtilFlashLoad()
{

	// init and set mode
	//hardware.qspi_handle.mode = DSY_QSPI_MODE_DSY_MEMORY_MAPPED;
	//dsy_qspi_init(&hardware.qspi_handle);

	// Flash is memory mapped so no need to read

	// copy data from opdLoadConfig struct to data

	if (fxSettingsFlash.settingsVersion == SETTINGS_VERSION)
	{
		fxSettings = fxSettingsFlash;
	}

	// de-init
	// dsy_qspi_deinit();

	FXOscSet();
	FXAdsrSet();
	FXFilterSet();
	FXDecimatorSet();
	FXOverdriveSet();
	FXChorusSet();
	FXDelaySet();
	FXReverbSet();
	for (uint8_t i = 0; i < SM_NUMBER; i++)
	{
		FXSMSet(i);
	}
	for (uint8_t i = 0; i < LFO_NUMBER; i++)
	{
		FXLFOSet(i);
	}

}



#ifdef OPD_BASE_MIDI
// midi handler
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
			modSources.midiPitch = p.note;
			modSources.midiVelocity = p.velocity;				
	        break;
        }
        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
			modSources.midiPitch = p.note;
			modSources.midiVelocity = 0; // p.velocity;				
	        break;
        }        
        case ControlChange:
        {
		/*
            ControlChangeEvent p = m.AsControlChange();
            switch(p.control_number)
            {
                case 1:
                    // CC 1 for cutoff.
                    filt.SetFreq(mtof((float)p.value));
                    break;
                case 2:
                    // CC 2 for res.
                    filt.SetRes(((float)p.value / 127.0f));
                    break;
                default: break;
            }
			*/
            break;
        }
        default: break;
    }
}
#endif



uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



// Convert MIDI note number to CV
// DAC outputs from 0 to 3v3 (3300 mV's)
// We have 1V/Oct, so 3+ octaves on a 3v3 system
// The DAC wants values from 0 to 4095
uint16_t mtocv(uint8_t midiPitch)
{
	float voltsPerNote = 0.0833f; // 1/12 V
	float mV; // from 0 to 0xFFF (4096);

	mV = 1000 * (midiPitch * voltsPerNote);

	return (map(mV, 0, 3300, 0, 4095));
}



float mtoval(uint8_t midiPitch)
{
	float voltsPerNote = 0.0833f; // 1/12 V

	return ((midiPitch * voltsPerNote) / 3.3f);
}



int main(void)
{
	// init hardware
	hardware.Configure();
	hardware.Init();
	sysSampleRate = hardware.AudioSampleRate();
	sysCallbackRate = hardware.AudioCallbackRate();

	#ifdef OPD_BASE_MIDI
    MidiUartHandler::Config midi_config;
	midi.Init(midi_config);
    //midi.Init(MidiHandler::INPUT_MODE_UART1, MidiHandler::OUTPUT_MODE_NONE);
	#endif

	// init boot/upload button
	Switch buttonUpload;
	buttonUpload.Init(hardware.GetPin(PIN_BUTTON_UPLOAD), 10);
	
	
	// init DAC outputs
	DacHandle::Config cfg;
	cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
	cfg.buff_state = DacHandle::BufferState::ENABLED;
	cfg.mode       = DacHandle::Mode::POLLING;
	cfg.chn        = DacHandle::Channel::BOTH;
    hardware.dac.Init(cfg);
    hardware.dac.WriteValue(DacHandle::Channel::BOTH, 0);
	hardware.dac.WriteValue(DacHandle::Channel::ONE, 0); // CV0
	hardware.dac.WriteValue(DacHandle::Channel::TWO, 0); // CV1


    // init GATE outputs
	gateOut0.pin  = hardware.GetPin(DA_GATE0_PIN);
	gateOut0.mode = DSY_GPIO_MODE_OUTPUT_PP;
	gateOut0.pull = DSY_GPIO_NOPULL;
	dsy_gpio_init(&gateOut0);
	gateOut1.pin  = hardware.GetPin(DA_GATE1_PIN);
	gateOut1.mode = DSY_GPIO_MODE_OUTPUT_PP;
	gateOut1.pull = DSY_GPIO_NOPULL;
	dsy_gpio_init(&gateOut1);
	// write/set
	// dsy_gpio_write(&gateOut0, true);  // set high/low true/false 1/0


	// init AD inputs
	adcConfig[AD_LCDBUTTON_INDEX].InitSingle(hardware.GetPin(AD_LCDBUTTON_PIN));
	adcConfig[AD_CV0_INDEX].InitSingle(hardware.GetPin(AD_CV0_PIN));
	adcConfig[AD_CV1_INDEX].InitSingle(hardware.GetPin(AD_CV1_PIN));
	adcConfig[AD_GATE0_INDEX].InitSingle(hardware.GetPin(AD_GATE0_PIN));
	adcConfig[AD_GATE1_INDEX].InitSingle(hardware.GetPin(AD_GATE1_PIN));
	adcConfig[AD_POT0_INDEX].InitSingle(hardware.GetPin(AD_POT0_PIN));
	adcConfig[AD_POT1_INDEX].InitSingle(hardware.GetPin(AD_POT1_PIN));
	hardware.adc.Init(adcConfig, AD_MAX);
	hardware.adc.Start();
	
	
	// init modsources
	modSources.midiPitch = 0;
	modSources.midiVelocity = 0;
	modSources.audioIn = 0;


	// set up fx
	SetupFX();
	fxSettings.settingsVersion = SETTINGS_VERSION;
	fxSettings.inputChannel = INPUT_CHANNEL_STEREO;
	fxSettings.gainInputL = 1.0f;
	fxSettings.gainInputR = 1.0f;
	fxSettings.gainOutputL = 1.0f;
	fxSettings.gainOutputR = 1.0f;


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


	// logging over serial USB
	#ifdef OPD_LOGG
	hardware.StartLog(false); // start log but don't wait for PC - we can be connected to a battery
	#endif


	// let everything settle (esp the LCD)
	System::Delay(100);


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

		float adcButton = hardware.adc.GetFloat(AD_LCDBUTTON_INDEX);
		ui.Button(adcButton);
		ui.Work();
		ui.Draw();

		#ifdef OPD_BASE_MIDI
        // handle MIDI Events
        midi.Listen();
        while(midi.HasEvents())
        {
            HandleMidiMessage(midi.PopEvent());
        }
		#endif
		
		// read CV inputs
		for (uint8_t i = AD_CV0_INDEX; i <= CV_NUMBER; i++)
		{
			modSources.cvValue[i - AD_CV0_INDEX] = hardware.adc.GetFloat(i);
		}

		// write CV/Gate outputs
		if (fxSettings.outCV0Mod != MOD_SOURCE_NONE)
		{
			if (fxSettings.outCV0Mod == MOD_SOURCE_MIDIP)
			{
				hardware.dac.WriteValue(DacHandle::Channel::ONE, mtocv(modSources.midiPitch));	
			} else {
				hardware.dac.WriteValue(DacHandle::Channel::ONE, 4096.0f * ModFactor(fxSettings.outCV0Mod) - 1);
			}
		}
		if (fxSettings.outCV1Mod != MOD_SOURCE_NONE)
		{
			if (fxSettings.outCV1Mod == MOD_SOURCE_MIDIP)
			{
				hardware.dac.WriteValue(DacHandle::Channel::TWO, mtocv(modSources.midiPitch));	
			} else {
				hardware.dac.WriteValue(DacHandle::Channel::TWO, 4096.0f * ModFactor(fxSettings.outCV1Mod) - 1);
			}
		}
		#ifndef OPD_BASE_MIDI
		if (fxSettings.outGate0Mod != MOD_SOURCE_NONE)
		{
			if (ModFactor(fxSettings.outGate0Mod) > fxSettings.outGate0Level)
			{
				dsy_gpio_write(&gateOut0, 1);
			} else {
				dsy_gpio_write(&gateOut0, 0);
			}
		}
		if (fxSettings.outGate1Mod != MOD_SOURCE_NONE)
		{
			if (ModFactor(fxSettings.outGate1Mod) > fxSettings.outGate1Level)
			{
				dsy_gpio_write(&gateOut1, 1);
			} else {
				dsy_gpio_write(&gateOut1, 0);
			}
		}
		#endif

		// Reset to upload?
		buttonUpload.Debounce();
		if (buttonUpload.Pressed())
		{
			RebootToBootloader();
		}


		// wait
		System::Delay(25);
	}
}
