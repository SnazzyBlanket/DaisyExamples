/*
  
Project: OscPocketD/Sampler
Description: Sampler for Daisy
Author: Staffan Melin, staffan.melin@oscillator.se
License: GNU General Public License v3.0
Version: 202104
Project site: http://www.oscillator.se/opensource

*/

//#define OPD_LOGG // start serial over USB Logger class
//#define OPD_MEASURE // measure MCU utilization
#define OPD_BASE_MIDI // version 2 with MIDI, other LCD pins, no CV1 in, no Gate0/1 out

#include "daisysp.h"
#include "daisy_seed.h"

#include "stm32h7xx_hal.h" // for HAL_NVIC_SystemReset();
//extern "C" void HAL_NVIC_SystemReset();
#include "core_cm7.h"

#include "dev/lcd_hd44780.h"
#include "main.h"
#include "ui.h"


using namespace daisysp;
using namespace daisy;



static DaisySeed hardware;
float sysSampleRate;
float sysCallbackRate;

bool uiRedraw = true;


LcdHD44780 lcd;
AdcChannelConfig adcConfig[AD_MAX]; // Lcd button + CV in
uint8_t sysPlay;
bool sysRecord;
uint8_t sysLedMode;
uint32_t sysLedOnTime;
bool sysClip;



// FX
OpdFXSettings fxSettings;
OpdModSources modSources;



// modules

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
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS fxDelayL;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS fxDelayR;

// chorus
Chorus fxChorus;

// reverb
ReverbSc fxReverb;

// lfo
Oscillator lfoOsc[LFO_NUMBER];



// sampler
#define BUFFER_MAX (48000 * 60) // 60 secs; 48k * 2 * 4 = 384k/s 
float DSY_SDRAM_BSS sBufferR[BUFFER_MAX];
float DSY_SDRAM_BSS sBufferL[BUFFER_MAX];

float sIndex; // index into buffer
uint32_t sIndexInt;
float sIndexFraction;
uint32_t sIndexRecord;
float sFactor; // how much to advance index for a new sample
float sFreq; // in Hz
bool sGate, sGatePrev; //
OpdSampleSettings sampleSettings;



// demo
void fillBuffer()
{
	Oscillator osc;

    osc.Init(sysSampleRate);
    osc.SetWaveform(osc.WAVE_TRI);
	osc.SetFreq(440.0f);
	osc.SetAmp(0.1f);

	for (uint32_t i = 0; i < BUFFER_MAX; i++)
	{
		sBufferR[i] = osc.Process();
		sBufferL[i] = sBufferR[i];	
	}
}



void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
	float sigR, sigL;
	float sigRevL, sigRevR;
	float outTempL, outTempR;
	float outDelayL, outDelayR;

	float sigTemp;
    float a, b;
    //float sigGate;
    
	// measure MCU utilization
	#ifdef OPD_MEASURE
	// measure - start
	DWT->CYCCNT = 0;
	#endif
	
	// audio

    for (size_t i = 0; i < size; i += 2)
    {

		// lfo: update
		for (int8_t j = 0; j < LFO_NUMBER; j++)
		{
			modSources.lfoValue[j] = lfoOsc[j].Process();
		}
		
		// record

		if (sysPlay == PLAY_RECORD)
		{
			// input

			switch (fxSettings.inputChannel)
			{
			case INPUT_CHANNEL_STEREO:
				sigL = in[i];
				sigR = in[i + 1];
				break;
			case INPUT_CHANNEL_LEFT:
				sigL = in[i];
				sigR = sigL;
				break;
			case INPUT_CHANNEL_RIGHT:
				sigR = in[i + 1];
				sigL = sigR;
				break;
			default:
				sigL = in[i];
				sigR = in[i + 1];
			}
		
			if (sysRecord)
			{
				if (sIndexRecord < (BUFFER_MAX - 1))
				{
					sBufferL[sIndexRecord] = sigR;
					sBufferR[sIndexRecord] = sigL;
					sIndexRecord++;

					sampleSettings.sPhaseLoopEnd = sIndexRecord;
					sampleSettings.sPhaseEnd = sIndexRecord;
					sampleSettings.sLength = sIndexRecord;
				}
			}
			
			// pass through
			out[i] = sigR;
			out[i + 1] = sigL;

		} else {

			if ((sysPlay == PLAY_SINGLE) || (sysPlay == PLAY_LOOP))
			{
				// start new note?
				sGate = (ModFactor(fxSettings.oscGateMod) > fxSettings.oscGateLevel);
				if (sGate && (!sGatePrev))
				{
				
					// mod: freq
					if (fxSettings.oscFreqMod != MOD_SOURCE_NONE)
					{
//						sFreq = powf(2.0f, (ModFactor(fxSettings.oscFreqMod) * 3.3f)) * 55;// + fxSettings.oscDetune;
						sFreq = powf(2.0f, (ModFactor(fxSettings.oscFreqMod) * 3.3f)) * 55;
					} else {
						sFreq = fxSettings.oscFreq * (fxSettings.oscDetune / 100.0f);
					}

					sIndex = sampleSettings.sPhaseStart;
					sFactor = (sFreq / 440.0f) * (fxSettings.oscDetune / 100.0f);
					
					//sFactor = expf((sFreq/440.0f) * (gLogmax - gLogmin) + gLogmin) * gScalar;
				}
				sGatePrev = sGate;				

				if (sIndex < sampleSettings.sPhaseEnd)
				{
					sIndexInt = static_cast<int32_t>(sIndex);
					sIndexFraction = sIndex - sIndexInt;

					// get sample
					a = sBufferL[sIndexInt];
					b = sBufferL[sIndexInt + 1];
					sigL = a + (b - a) * sIndexFraction;
					a = sBufferR[sIndexInt];
					b = sBufferR[sIndexInt + 1];
					sigR = a + (b - a) * sIndexFraction;

					sIndex += sFactor;
					if ((sIndex >= sampleSettings.sPhaseLoopEnd) && (sGate || fxSettings.oscGateR) && (sysPlay == PLAY_LOOP))
					{
						sIndex = sampleSettings.sPhaseLoopStart;
					}
					
					// mod: amp
					sigL = sigL * fxSettings.oscAmp * ModFactor(fxSettings.oscAmpMod);
					sigR = sigR * fxSettings.oscAmp * ModFactor(fxSettings.oscAmpMod);

				} else {
					sigL = 0;
					sigR = 0;
				}
				
				
			} else {
				sigL = 0;
				sigR = 0;
			}

			// gain
			
			sigL *= fxSettings.gainInputL;
			sigR *= fxSettings.gainInputR;

			// filter
			
			if (fxSettings.filterOn)
			{

				// mod
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
				// gate
				
				sigTemp = fxAdsr.Process(sGate);

				sigL *= sigTemp;
				sigR *= sigTemp;
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
				// overdrive has no state so we can apply it to both L and R
				sigL = fxOverdrive.Process(sigL);
				sigR = fxOverdrive.Process(sigR);
			}

			// delay

			if (fxSettings.delayOn)
			{
			
				if (fxSettings.delayDelayMod != MOD_SOURCE_NONE)
				{
					fxDelayL.SetDelay(sysSampleRate * fxSettings.delayDelayL * ModFactor(fxSettings.delayDelayMod));
					fxDelayR.SetDelay(sysSampleRate * fxSettings.delayDelayR * ModFactor(fxSettings.delayDelayMod));
				}
			
				outDelayL = fxDelayL.Read();
				outTempL = sigL;
				sigL = outTempL + outDelayL;
				fxDelayL.Write(sigL * fxSettings.delayFeedbackL * ModFactor(fxSettings.delayFeedbackMod));

				outDelayR = fxDelayR.Read();
				outTempR = sigR;
				sigR = outTempR + outDelayR;
				fxDelayR.Write(sigR * fxSettings.delayFeedbackR * ModFactor(fxSettings.delayFeedbackMod));
			}

			// chorus

			if (fxSettings.chorusOn)
			{
				fxChorus.Process((sigL + sigR) / 2.0f);
				sigL = sigL * fxSettings.chorusDry + fxChorus.GetLeft() * fxSettings.chorusWet;
				sigR = sigR * fxSettings.chorusDry + fxChorus.GetRight() * fxSettings.chorusWet;
			}

			// reverb send

			if (fxSettings.reverbOn)
			{
				sigRevL = sigL * fxSettings.reverbWet;
				sigRevR = sigR * fxSettings.reverbWet;
				fxReverb.Process(sigRevL, sigRevR, &outTempL, &outTempR);
				sigL = sigL * fxSettings.reverbDry + outTempL;
				sigR = sigR * fxSettings.reverbDry + outTempR;
			} else {
				sigRevL = 0;
				sigRevR = 0;
			}
			
			// gain
			sigL *= fxSettings.gainOutputL;
			sigR *= fxSettings.gainOutputR;

		    out[i]  = sigL;
		    out[i + 1] = sigR;
		    
		    
			if (sysLedMode == LED_MODE_CLIP)
			{
				if ((out[i] > 1.0f) || (out[i + 1] > 1.0f))
				{
					sysClip = true;
				}
			}
		}
    }

	// measure MCU utilization
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
	default:
		aFactor = 1;
	}

	return aFactor;
}			



// setup fx

void SetupFX()
{
	// osc
	fxSettings.oscOn = true;
	fxSettings.oscGateMod = MOD_SOURCE_CV0;
	fxSettings.oscGateLevel = 0.3f;
	fxSettings.oscGateR = false;
	fxSettings.oscFreq = 440.0f;
	fxSettings.oscFreqMod = MOD_SOURCE_CV1;
	fxSettings.oscDetune = 100.0f; // %
	fxSettings.oscAmp = 0.2f;
	fxSettings.oscAmpMod = MOD_SOURCE_NONE;	

	// ADSR
	fxSettings.adsrOn = true;
	fxSettings.adsrAttack = 0.01f;
	fxSettings.adsrDecay = 0.01f;
	fxSettings.adsrSustain = 1.0f;
	fxSettings.adsrRelease = 0.3f;
	fxSettings.adsrGateLevel = 0.5f;
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
	fxSettings.overdriveDrive = 0.5f;
	FXOverdriveSet();

	// delay
	fxSettings.delayOn = true;
	fxSettings.delayDelayL = 0.3f;
	fxSettings.delayFeedbackL = 0.3f;
	fxSettings.delayDelayR = 0.4f;
	fxSettings.delayFeedbackR = 0.3f;
	fxDelayL.Init();
	fxDelayR.Init();
	FXDelaySet();

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
	fxSettings.reverbOn = true;
	fxSettings.reverbFeedback = 0.3f;
	fxSettings.reverbLPFFreq = 8000;
	fxSettings.reverbDry = 0.50f;
	fxSettings.reverbWet = 0.50f;
	fxReverb.Init(sysSampleRate);
	FXReverbSet();
	
	// lfo
	for (int i = 0; i < LFO_NUMBER; i++)
	{
		fxSettings.lfoWaveform[i] = 0;
		fxSettings.lfoFreq[i] = 10.0f;
		fxSettings.lfoAmp[i] = 1.0f;
		fxSettings.lfoOffset[i] = 0.5f;
		lfoOsc[i].Init(sysSampleRate);
		FXLFOSet(i);
	}
	
	// CV
	for (int i = 0; i < CV_NUMBER; i++)
	{
		fxSettings.cvAmp[i] = 1.0f;
		fxSettings.cvOffset[i] = 0.0f;
	}



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



void FXLFOSet(uint8_t aLFO)
{
	lfoOsc[aLFO].SetWaveform(fxSettings.lfoWaveform[aLFO]);
	lfoOsc[aLFO].SetAmp(fxSettings.lfoAmp[aLFO]);
	lfoOsc[aLFO].SetFreq(fxSettings.lfoFreq[aLFO]);
}



void RecordPrepare()
{
	sIndexRecord = 0;
	sampleSettings.sPhaseStart = 0;
	sampleSettings.sPhaseLoopStart = 0;
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
	// uint32_t base = 0x90000000;

	// init and set mode

	// hardware.qspi_handle.mode = DSY_QSPI_MODE_INDIRECT_POLLING;
	// dsy_qspi_init(&hardware.qspi_handle);

	size_t start_address = (size_t)&fxSettingsFlash;
    size_t size = sizeof(fxSettings);
	size_t slot_address = (uint8_t)start_address;

	// erase

	// dsy_qspi_erase(base, base + sizeof(OpdFXSettings));
    hardware.qspi.Erase(slot_address, slot_address + size);
    
	// write

	// dsy_qspi_write(base, sizeof(fxSettings), (uint8_t*)&fxSettings);
    hardware.qspi.Write(slot_address, size, (uint8_t*)&fxSettings);
    
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

	if (fxSettingsFlash.settingsVersion == SETTINGS_VERSION)
	{
		fxSettings = fxSettingsFlash;
	}

	// de-init

	// dsy_qspi_deinit();

	FXAdsrSet();
	FXFilterSet();
	FXDecimatorSet();
	FXOverdriveSet();
	FXDelaySet();
	FXChorusSet();
	FXReverbSet();
	for (uint8_t i = 0; i < LFO_NUMBER; i++)
	{
		FXLFOSet(i);
	}

}


int main(void)
{

/*
gLogmax = log(pow(2, 5));
gLogmin = log(1);
gScalar = 1/pow(2.f, 3.f);
https://en.wikipedia.org/wiki/Twelfth_root_of_two
https://www.reddit.com/r/audioengineering/comments/5wwd4r/how_to_calculate_pitch_change_from_speed_change/
*/

    // initialize seed hardware and daisysp modules
    hardware.Configure();
    hardware.Init();
	sysSampleRate = hardware.AudioSampleRate();
	sysCallbackRate = hardware.AudioCallbackRate();

	// Configure and initialize button
	Switch button1;
	button1.Init(hardware.GetPin(PIN_BUTTON_UPLOAD), 10);
	
	// AD inputs
	adcConfig[AD_LCDBUTTON_INDEX].InitSingle(hardware.GetPin(AD_LCDBUTTON_PIN));
	adcConfig[AD_CV0_INDEX].InitSingle(hardware.GetPin(AD_CV0_PIN));
	adcConfig[AD_CV1_INDEX].InitSingle(hardware.GetPin(AD_CV1_PIN));
	adcConfig[AD_GATE0_INDEX].InitSingle(hardware.GetPin(AD_GATE0_PIN));
	adcConfig[AD_GATE1_INDEX].InitSingle(hardware.GetPin(AD_GATE1_PIN));
	adcConfig[AD_POT0_INDEX].InitSingle(hardware.GetPin(AD_POT0_PIN));
	adcConfig[AD_POT1_INDEX].InitSingle(hardware.GetPin(AD_POT1_PIN));
	hardware.adc.Init(adcConfig, AD_MAX);
	hardware.adc.Start();

	// global setup
	sysPlay = PLAY_OFF;
	sysRecord = false;
	sysLedMode = LED_MODE_TEMPO;
	sysClip = false;

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

	// sampler - setup
	fillBuffer();
	sIndex = 0.0f;
	sFreq = 440.0f;
	sFactor = (sFreq / 440.0f);
	sGate = false;
	sGatePrev = false;
	sampleSettings.sPhaseStart = 48000.0f * 0.0f;
	sampleSettings.sPhaseLoopStart = 48000.0f * 0.1f;
	sampleSettings.sPhaseLoopEnd = 48000.0f * 0.5f;
	sampleSettings.sPhaseEnd = 48000.0f * 1.0f;
	sampleSettings.sLength = sampleSettings.sPhaseEnd;


	#ifdef OPD_MEASURE
	// setup measurement
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	#endif



    // start callback
    hardware.StartAudio(AudioCallback);

	// loop forever
	for(;;)
	{
		// handle UI

		float adcButton = hardware.adc.GetFloat(AD_LCDBUTTON_INDEX);
		ui.Button(adcButton);
		ui.Work();
		ui.Draw();

		// LED is working as clip notifier

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



		// read CV inputs
/*
		for (uint8_t i = 0; i < CV_NUMBER; i++)
		{
			modSources.cvValue[i] = hardware.adc.GetFloat(AD_CV0_INDEX + i);
		}
*/
		for (uint8_t i = AD_CV0_INDEX; i <= CV_NUMBER; i++)
		{
			modSources.cvValue[i - AD_CV0_INDEX] = hardware.adc.GetFloat(i);
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
