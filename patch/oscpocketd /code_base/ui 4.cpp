// UI



#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "ui.h"
#include "sm.h"

#include "dev/lcd_hd44780.h"

using namespace daisy;
using namespace daisysp;



// globals
extern DaisySeed hardware;
extern float sysSampleRate;
extern uint8_t sysLedMode;
extern bool uiRedraw;

extern bool sysPlay;

extern OpdFXSettings fxSettings;
extern OpdModSources modSources;

// FX

char smTypeNames[SM_TYPE_MAX][4] = {"NOI", "CRA", "INT", "CHA", "SEQ"};
char oscWaveNames[WAVEFORMS_MAX][4] = {"SIN", "TRI", "SAW", "RAM", "SQU", "TRP", "SAP", "SQP"};
char modSourceNames[MOD_SOURCE_MAX][3] = {"--", "L0", "L1", "L2", "C0", "C1", "G0", "G1", "P0", "P1", "MP", "MV", "S0", "S1", "S2", "T0", "T1", "T2", "AU", "E0", "E1"};

void OscUI::Init(LcdHD44780 *aLcd)
{
	button = BUTTON_NONE;
	buttonPrevious = BUTTON_NONE;
	buttonPreviousCount = 0;
	buttonPreviousTime = System::GetNow();
	buttonMult = 1;

	lcd = aLcd;

	uiMode = UI_MODE_OVERVIEW;

	uiMainRow = 0;
	uiMainCol = 0;
	uiSubRow = 0;
	uiSubCol = 0;
	uiSubColOffset = 0;
	uiSubRowOffset = 0;
	
	uiLFO = 0;
	uiEG = 0;

}

// RIGHT	0
// UP		9
// DOWN		25
// LEFT 	39
// SELECT 	62
// else:	99

void OscUI::Button(float aButtonValue) {

	int aInButtonValue = (int)(aButtonValue*100);

	if (aInButtonValue < 9) {
		button = BUTTON_RIGHT; // 0
	} else if (aInButtonValue < 20) {
		button = BUTTON_UP; // 10
	} else if (aInButtonValue < 30) {
		button = BUTTON_DOWN; // 25
	} else if (aInButtonValue < 50) {
		button = BUTTON_LEFT; // 40
	} else if (aInButtonValue < 70) {
		button = BUTTON_SELECT; // 62
	} else {
		button = BUTTON_NONE;
	}

}




void OscUI::Work()
{

	if (button == BUTTON_NONE) 
	{
		buttonMult = 1;
	} else {
		if ((System::GetNow() - buttonPreviousTime) > 50)
		{
			buttonPreviousTime = System::GetNow();

			if (button == buttonPrevious)
			{
				buttonPreviousCount++;
				if (buttonPreviousCount > 10)
				{
					buttonMult = buttonMult * 10;
					buttonPreviousCount = 0;
				}
			} else {
				buttonPreviousCount = 0;
			}
			buttonPrevious = button;

			
			switch (uiMode)
			{
			case UI_MODE_OVERVIEW:
				if ((uiMainRow == 0) && (uiMainCol == 0)) {

					// OVERVIEW: INPUT
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_GAIN;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						switch (fxSettings.inputChannel)
						{
						case INPUT_CHANNEL_STEREO:
							fxSettings.inputChannel = INPUT_CHANNEL_LEFT;
							break;
						case INPUT_CHANNEL_LEFT:
							fxSettings.inputChannel = INPUT_CHANNEL_RIGHT;
							break;
						case INPUT_CHANNEL_RIGHT:
							fxSettings.inputChannel = INPUT_CHANNEL_MERGE;
							break;
						case INPUT_CHANNEL_MERGE:
							fxSettings.inputChannel = INPUT_CHANNEL_STEREO;
							break;
						}
					} else if (button == BUTTON_DOWN) {
						switch (fxSettings.inputChannel)
						{
						case INPUT_CHANNEL_STEREO:
							fxSettings.inputChannel = INPUT_CHANNEL_MERGE;
							break;
						case INPUT_CHANNEL_LEFT:
							fxSettings.inputChannel = INPUT_CHANNEL_STEREO;
							break;
						case INPUT_CHANNEL_RIGHT:
							fxSettings.inputChannel = INPUT_CHANNEL_LEFT;
							break;
						case INPUT_CHANNEL_MERGE:
							fxSettings.inputChannel = INPUT_CHANNEL_RIGHT;
							break;
						}
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 1;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 1)) {

					// OVERVIEW: UTILITY MENU
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_UTILITY;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_DOWN) {
						uiMainRow = 1;
						uiMainCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 2;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 2)) {
				
					// OVERVIEW: EG0
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_EG;
						uiEG = 0;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 1;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 3;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 3)) {

					// OVERVIEW: EG1
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_EG;
						uiEG = 1;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 4;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 4)) {

					// OVERVIEW: SM0
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_SM;
						uiSM = 0;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 5;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 5)) {

					// OVERVIEW: SM1
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_SM;
						uiSM = 1;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 6;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 6)) {

					// OVERVIEW: SM2
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_SM;
						uiSM = 2;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 5;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 7;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 7)) {

					// OVERVIEW: LFO0
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_LFO;
						uiLFO = 0;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 8;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 8)) {

					// OVERVIEW: LFO1
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_LFO;
						uiLFO = 1;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 7;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 9;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 9)) {

					// OVERVIEW: LFO2
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_LFO;
						uiLFO = 2;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 10;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 10)) {

					// OVERVIEW: CV0
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CV;
						uiCV = 0;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 11;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 11)) {

					// OVERVIEW: CV1
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CV;
						uiCV = 1;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 12;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 12)) {

					// OVERVIEW: GATE0
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CV;
						uiCV = 2;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 11;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 13;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 13)) {

					// OVERVIEW: GATE1
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CV;
						uiCV = 3;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 12;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 14;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 14)) {

					// OVERVIEW: POT0
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CV;
						uiCV = 4;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 13;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 15;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 15)) {

					// OVERVIEW: POT1
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CV;
						uiCV = 5;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 14;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 0;
						uiMainRow = 1;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 0)) {

					// OVERVIEW: OSC
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_OSC;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.oscOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.oscOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 15;
						uiMainRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 1;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 1)) {

					// OVERVIEW: SLICER
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_SLICER;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.slicerOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.slicerOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 2;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 2)) {

					// OVERVIEW: DECIMATOR
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_DECIMATOR;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.decimatorOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.decimatorOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 1;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 3;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 3)) {

					// OVERVIEW: OVERDRIVE
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_OVERDRIVE;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.overdriveOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.overdriveOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 4;
					}

				} else if ((uiMainRow == 1) && (uiMainCol == 4)) {

					// OVERVIEW: FILTER
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_FILTER;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.filterOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.filterOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 5;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 5)) {

					// OVERVIEW: ADSR
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_ADSR;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.adsrOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.adsrOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 6;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 6)) {

					// OVERVIEW: PAN
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_PAN;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.panOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.panOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 5;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 7;
					}

				} else if ((uiMainRow == 1) && (uiMainCol == 7)) {

					// OVERVIEW: DELAY
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_DELAY;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.delayOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.delayOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 8;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 8)) {

					// OVERVIEW: CHORUS
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CHORUS;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.chorusOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 7;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 9;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 9)) {

					// OVERVIEW: REVERB
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_REVERB;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
						fxSettings.reverbOn = true;
					} else if (button == BUTTON_DOWN) {
						fxSettings.reverbOn = false;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 0;
						uiMainRow = 0;
					}
				}
				break;

			case UI_MODE_OSC:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// OSC: WAVEFORM
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.oscWaveform < (WAVEFORMS_MAX - 1))
						{
							fxSettings.oscWaveform += 1;
							FXOscSet();
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.oscWaveform > 0)
						{
							fxSettings.oscWaveform -= 1;
							FXOscSet();
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// OSC: FREQ
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.oscFreqMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.oscFreqMod++;
						} else {
							fxSettings.oscFreqMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.oscFreq += 1 * buttonMult;
						fxSettings.oscFreq = MIN(fxSettings.oscFreq, (sysSampleRate / 2));
						FXOscSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.oscFreq -= 1 * buttonMult;;
						fxSettings.oscFreq = MAX(fxSettings.oscFreq, 1);
						FXOscSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// OSC: AMP
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.oscAmpMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.oscAmpMod++;
						} else {
							fxSettings.oscAmpMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.oscAmp += 0.01 * buttonMult;
						fxSettings.oscAmp = MIN(fxSettings.oscAmp, 0.99);
						FXOscSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.oscAmp -= 0.01 * buttonMult;
						fxSettings.oscAmp = MAX(fxSettings.oscAmp, 0);
						FXOscSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 11;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 11)) {

					// OSC: DETUNE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.oscDetune += 1 * buttonMult;
						fxSettings.oscDetune = MIN(fxSettings.oscDetune, (sysSampleRate / 2));
						FXOscSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.oscDetune -= 1 * buttonMult;;
						fxSettings.oscDetune = MAX(fxSettings.oscDetune, 0);
						FXOscSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 0;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// OSC2: WAVEFORM
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.oscWaveform2 < (WAVEFORMS_MAX - 1))
						{
							fxSettings.oscWaveform2 += 1;
							FXOscSet();
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.oscWaveform2 > 0)
						{
							fxSettings.oscWaveform2 -= 1;
							FXOscSet();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 11;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 9)) {

					// OSC2: AMP
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.oscAmp2Mod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.oscAmp2Mod++;
						} else {
							fxSettings.oscAmp2Mod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.oscAmp2 += 0.01 * buttonMult;
						fxSettings.oscAmp2 = MIN(fxSettings.oscAmp2, 0.99);
						FXOscSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.oscAmp2 -= 0.01 * buttonMult;
						fxSettings.oscAmp2 = MAX(fxSettings.oscAmp2, 0);
						FXOscSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 11;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 11)) {

					// OSC2: DETUNE
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.oscDetune2Mod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.oscDetune2Mod++;
						} else {
							fxSettings.oscDetune2Mod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.oscDetune2 += 1 * buttonMult;
						fxSettings.oscDetune2 = MIN(fxSettings.oscDetune2, (sysSampleRate / 2));
						FXOscSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.oscDetune2 -= 1 * buttonMult;;
						fxSettings.oscDetune2 = MAX(fxSettings.oscDetune2, 0);
						FXOscSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}
				}
				break;

			case UI_MODE_ADSR:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// ADSR: Adsr
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.adsrAttack += 0.01f * buttonMult;
						fxSettings.adsrAttack = MIN(fxSettings.adsrAttack, 9.0f);
						FXAdsrSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.adsrAttack -= 0.01f * buttonMult;
						fxSettings.adsrAttack = MAX(fxSettings.adsrAttack, 0.0f);
						FXAdsrSet();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// ADSR: aDsr
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.adsrDecay += 0.01f * buttonMult;
						fxSettings.adsrDecay = MIN(fxSettings.adsrDecay, 9.0f);
						FXAdsrSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.adsrDecay -= 0.01f * buttonMult;
						fxSettings.adsrDecay = MAX(fxSettings.adsrDecay, 0.0f);
						FXAdsrSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// ADSR: adSr - level, not time (from 0-1)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.adsrSustain += 0.01f * buttonMult;
						fxSettings.adsrSustain = MIN(fxSettings.adsrSustain, 0.99f);
						FXAdsrSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.adsrSustain -= 0.01f * buttonMult;
						fxSettings.adsrSustain = MAX(fxSettings.adsrSustain, 0.0f);
						FXAdsrSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// ADSR: adsR
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.adsrRelease += 0.01f * buttonMult;
						fxSettings.adsrRelease = MIN(fxSettings.adsrRelease, 9.99f);
						FXAdsrSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.adsrRelease -= 0.01f * buttonMult;
						fxSettings.adsrRelease = MAX(fxSettings.adsrRelease, 0.0f);
						FXAdsrSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// ADSR: GATE LEVEL (0-0.99)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.adsrGateLevel += 0.01f * buttonMult;
						fxSettings.adsrGateLevel = MIN(fxSettings.adsrGateLevel, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.adsrGateLevel -= 0.01f * buttonMult;
						fxSettings.adsrGateLevel = MAX(fxSettings.adsrGateLevel, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
						uiSubRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 3)) {

					// ADSR: GATE MOD
					if (button == BUTTON_SELECT) {
						if (fxSettings.adsrGateMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.adsrGateMod++;
						} else {
							fxSettings.adsrGateMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 6)) {

					// ADSR: SUSTAIN MOD
					if (button == BUTTON_SELECT) {
						if (fxSettings.adsrSustainMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.adsrSustainMod++;
						} else {
							fxSettings.adsrSustainMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}
				}
				break;

			case UI_MODE_FILTER:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// FILTER: TYPE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						switch (fxSettings.filterType)
						{
						case FILTER_TYPE_LOW:
							fxSettings.filterType = FILTER_TYPE_HIGH;
							break;
						case FILTER_TYPE_HIGH:
							fxSettings.filterType = FILTER_TYPE_BAND;
							break;
						case FILTER_TYPE_BAND:
							fxSettings.filterType = FILTER_TYPE_LOW;
							break;
						}
					} else if (button == BUTTON_DOWN) {
						switch (fxSettings.filterType)
						{
						case FILTER_TYPE_LOW:
							fxSettings.filterType = FILTER_TYPE_BAND;
							break;
						case FILTER_TYPE_HIGH:
							fxSettings.filterType = FILTER_TYPE_LOW;
							break;
						case FILTER_TYPE_BAND:
							fxSettings.filterType = FILTER_TYPE_HIGH;
							break;
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 2;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 2)) {

					// FILTER: FREQ
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.filterFreqMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.filterFreqMod++;
						} else {
							fxSettings.filterFreqMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.filterFreq += 10.0f * buttonMult;
						fxSettings.filterFreq = MIN(fxSettings.filterFreq, (sysSampleRate / 2));
						FXFilterSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.filterFreq -= 10.0f * buttonMult;
						fxSettings.filterFreq = MAX(fxSettings.filterFreq, 0.0f);
						FXFilterSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// FILTER: RES
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.filterResMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.filterResMod++;
						} else {
							fxSettings.filterResMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.filterRes += 0.01f * buttonMult;
						fxSettings.filterRes = MIN(fxSettings.filterRes, 0.99f);
						FXFilterSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.filterRes -= 0.01f * buttonMult;
						fxSettings.filterRes = MAX(fxSettings.filterRes, 0.0f);
						FXFilterSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_DECIMATOR:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// DECIMATOR: DOWNSAMPLE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.decimatorDownsampleFactor += 0.1f * buttonMult;
						fxSettings.decimatorDownsampleFactor = MIN(fxSettings.decimatorDownsampleFactor, 0.99f);
						FXDecimatorSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.decimatorDownsampleFactor -= 0.1f * buttonMult;
						fxSettings.decimatorDownsampleFactor = MAX(fxSettings.decimatorDownsampleFactor, 0.0f);
						FXDecimatorSet();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// DECIMATOR: BITCRUSH
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.decimatorBitcrushFactorMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.decimatorBitcrushFactorMod++;
						} else {
							fxSettings.decimatorBitcrushFactorMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.decimatorBitcrushFactor += 0.01f * buttonMult;
						fxSettings.decimatorBitcrushFactor = MIN(fxSettings.decimatorBitcrushFactor, 0.99f);
						FXDecimatorSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.decimatorBitcrushFactor -= 0.01f * buttonMult;
						fxSettings.decimatorBitcrushFactor = MAX(fxSettings.decimatorBitcrushFactor, 0.0f);
						FXDecimatorSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// DECIMATOR: BITS
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.decimatorBitsToCrush < 16)
						{
							fxSettings.decimatorBitsToCrush += 1;
							FXDecimatorSet();
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.decimatorBitsToCrush > 0)
						{
							fxSettings.decimatorBitsToCrush -= 1;
							FXDecimatorSet();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_OVERDRIVE:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// OVERDRIVE: GAIN
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.overdriveGainMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.overdriveGainMod++;
						} else {
							fxSettings.overdriveGainMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.overdriveGain += 0.01f * buttonMult;
						fxSettings.overdriveGain = MIN(fxSettings.overdriveGain, 2.0f);
						FXOverdriveSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.overdriveGain -= 0.01 * buttonMult;
						fxSettings.overdriveGain = MAX(fxSettings.overdriveGain, 0.0f);
						FXOverdriveSet();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// OVERDRIVE: DRIVE
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.overdriveDriveMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.overdriveDriveMod++;
						} else {
							fxSettings.overdriveDriveMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.overdriveDrive += 0.01f * buttonMult;
						fxSettings.overdriveDrive = MIN(fxSettings.overdriveDrive, 1.0f);
						FXOverdriveSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.overdriveDrive -= 0.01 * buttonMult;
						fxSettings.overdriveDrive = MAX(fxSettings.overdriveDrive, 0.0f);
						FXOverdriveSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
					}
				}
				break;

			case UI_MODE_PAN:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// PAN: PAN
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.panPanMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.panPanMod++;
						} else {
							fxSettings.panPanMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.panPan += 0.01f * buttonMult;
						fxSettings.panPan = MIN(fxSettings.panPan, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.panPan -= 0.01 * buttonMult;
						fxSettings.panPan = MAX(fxSettings.panPan, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
					}
				}
				break;

			case UI_MODE_DELAY:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// DELAY: L DELAY
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.delayDelayMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.delayDelayMod++;
						} else {
							fxSettings.delayDelayMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						if (fxSettings.delayDelayL < DELAY_MAX_S)
						{
							fxSettings.delayDelayL += 0.1;
							FXDelaySet();
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.delayDelayL >= 0.1)
						{
							fxSettings.delayDelayL -= 0.1;
							FXDelaySet();
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// DELAY: L FEEDBACK
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.delayFeedbackMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.delayFeedbackMod++;
						} else {
							fxSettings.delayFeedbackMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.delayFeedbackL += 0.01f * buttonMult;
						fxSettings.delayFeedbackL = MIN(fxSettings.delayFeedbackL, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.delayFeedbackL -= 0.01f * buttonMult;
						fxSettings.delayFeedbackL = MAX(fxSettings.delayFeedbackL, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 0;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// DELAY: R DELAY
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.delayDelayMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.delayDelayMod++;
						} else {
							fxSettings.delayDelayMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						if (fxSettings.delayDelayR < DELAY_MAX_S)
						{
							fxSettings.delayDelayR += 0.1;
							FXDelaySet();
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.delayDelayR >= 0.1)
						{
							fxSettings.delayDelayR -= 0.1;
							FXDelaySet();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 4)) {

					// DELAY: R FEEDBACK
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.delayFeedbackMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.delayFeedbackMod++;
						} else {
							fxSettings.delayFeedbackMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
						fxSettings.delayFeedbackR += 0.01f * buttonMult;
						fxSettings.delayFeedbackR = MIN(fxSettings.delayFeedbackR, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.delayFeedbackR -= 0.01f * buttonMult;
						fxSettings.delayFeedbackR = MAX(fxSettings.delayFeedbackR, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_CHORUS:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// CHORUS: DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.chorusDelay += 0.01f * buttonMult;
						fxSettings.chorusDelay = MIN(fxSettings.chorusDelay, 0.99f);
						FXChorusSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusDelay -= 0.01f * buttonMult;
						fxSettings.chorusDelay = MAX(fxSettings.chorusDelay, 0.0f);
						FXChorusSet();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// CHORUS: FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.chorusFeedback += 0.01f * buttonMult;
						fxSettings.chorusFeedback = MIN(fxSettings.chorusFeedback, 0.99f);
						FXChorusSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusFeedback -= 0.01f * buttonMult;
						fxSettings.chorusFeedback = MAX(fxSettings.chorusFeedback, 0.0f);
						FXChorusSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// CHORUS: LFO DEPTH
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.chorusLfoDepth += 0.01f * buttonMult;
						fxSettings.chorusLfoDepth = MIN(fxSettings.chorusLfoDepth, 0.99f);
						FXChorusSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusLfoDepth -= 0.01 * buttonMult;
						fxSettings.chorusLfoDepth = MAX(fxSettings.chorusLfoDepth, 0.0f);
						FXChorusSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else 
				if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// CHORUS: LFO FREQ
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_UP) {
						fxSettings.chorusLfoFreq += 1.0f * buttonMult;
						fxSettings.chorusLfoFreq = MIN(fxSettings.chorusLfoFreq, 10000.0f);
						FXReverbSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusLfoFreq -= 1.0f * buttonMult;
						fxSettings.chorusLfoFreq = MAX(fxSettings.chorusLfoFreq, 0.0f);
						FXReverbSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 0;
					}
	 			} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// CHORUS: DRY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.chorusDry += 0.01f * buttonMult;
						fxSettings.chorusDry = MIN(fxSettings.chorusDry, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusDry -= 0.01f * buttonMult;
						fxSettings.chorusDry = MAX(fxSettings.chorusDry, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 3)) {

					// CHORUS: WET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.chorusWet += 0.01f * buttonMult;
						fxSettings.chorusWet = MIN(fxSettings.chorusWet, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.chorusWet -= 0.01f * buttonMult;
						fxSettings.chorusWet = MAX(fxSettings.chorusWet, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_REVERB:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// REVERB: LPF FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.reverbLPFFreq += 1.0f * buttonMult;
						fxSettings.reverbLPFFreq = MIN(fxSettings.reverbLPFFreq, (sysSampleRate / 2));
						FXReverbSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.reverbLPFFreq -= 1.0f * buttonMult;
						fxSettings.reverbLPFFreq = MAX(fxSettings.reverbLPFFreq, 0);
						FXReverbSet();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// REVERB: FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.reverbFeedback += 0.01f * buttonMult;
						fxSettings.reverbFeedback = MIN(fxSettings.reverbFeedback, 0.99f);
						FXReverbSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.reverbFeedback -= 0.01f * buttonMult;
						fxSettings.reverbFeedback = MAX(fxSettings.reverbFeedback, 0.0f);
						FXReverbSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// REVERB: DRY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.reverbDry += 0.01f * buttonMult;
						fxSettings.reverbDry = MIN(fxSettings.reverbDry, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.reverbDry -= 0.01f * buttonMult;
						fxSettings.reverbDry = MAX(fxSettings.reverbDry, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// REVERB: WET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.reverbWet += 0.01 * buttonMult;
						fxSettings.reverbWet = MIN(fxSettings.reverbWet, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.reverbWet -= 0.01f * buttonMult;
						fxSettings.reverbWet = MAX(fxSettings.reverbWet, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;
			
			case UI_MODE_SLICER:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// SLICER: MAX RECORD
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.slicer_record_samples_max + 1000 * buttonMult < SLICER_MAX)
						{
							fxSettings.slicer_record_samples_max += 1000 * buttonMult;
						}
						FXSlicerSet();
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.slicer_record_samples_max > 1000 * buttonMult)
						{
							fxSettings.slicer_record_samples_max -= 1000 * buttonMult;
						}
						FXSlicerSet();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// SLICER: MAX RECORD MOD
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.slicerRSMMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.slicerRSMMod++;
						} else {
							fxSettings.slicerRSMMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 7;
					}

				} else if ((uiSubRow == 0) && (uiSubCol == 7)) {

					// SLICER: MAX RECORD MOD GATE LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.slicerRSMModGateLevel += 0.01f * buttonMult;
						fxSettings.slicerRSMModGateLevel = MIN(fxSettings.slicerRSMModGateLevel, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.slicerRSMModGateLevel -= 0.01f * buttonMult;
						fxSettings.slicerRSMModGateLevel = MAX(fxSettings.slicerRSMModGateLevel, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 10;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 10)) {

					// SLICER: TRIG MODE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.slicerTrigMode = true;
						FXSlicerSet();
					} else if (button == BUTTON_DOWN) {
						fxSettings.slicerTrigMode = false;
						FXSlicerSet();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 7;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 0;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {
				
					// SLICER: PLAYBACK REPEAT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.slicer_playback_rep_max + 1 * buttonMult < 999)
						{
							fxSettings.slicer_playback_rep_max += 1 * buttonMult;
						}
						FXSlicerSet();
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.slicer_playback_rep_max > 1 * buttonMult)
						{
							fxSettings.slicer_playback_rep_max -= 1 * buttonMult;
						}
						FXSlicerSet();
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 4)) {
				
					// SLICER: PLAYBACK REPEAT MOD
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.slicerPRMMod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.slicerPRMMod++;
						} else {
							fxSettings.slicerPRMMod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 7;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 7)) {

					// SLICER: PLAYBACK REPEAT MOD GATE LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.slicerPRMModGateLevel += 0.01f * buttonMult;
						fxSettings.slicerPRMModGateLevel = MIN(fxSettings.slicerPRMModGateLevel, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.slicerPRMModGateLevel -= 0.01f * buttonMult;
						fxSettings.slicerPRMModGateLevel = MAX(fxSettings.slicerPRMModGateLevel, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_GAIN:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// GAIN: L INPUT
					if (button == BUTTON_SELECT)
					{
						fxSettings.gainInputR = fxSettings.gainInputL;
					} else if (button == BUTTON_UP) {
						if (fxSettings.gainInputL < 5.0f)
						{
							fxSettings.gainInputL += 0.1;
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.gainInputL > 0)
						{
							fxSettings.gainInputL -= 0.1;
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 2;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 2)) {

					// GAIN: R INPUT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.gainInputR < 5.0f)
						{
							fxSettings.gainInputR += 0.1;
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.gainInputR > 0)
						{
							fxSettings.gainInputR -= 0.1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// GAIN: L OUTPUT
					if (button == BUTTON_SELECT)
					{
						fxSettings.gainOutputR = fxSettings.gainOutputL;
					} else if (button == BUTTON_UP) {
						if (fxSettings.gainOutputL < 5.0f)
						{
							fxSettings.gainOutputL += 0.1;
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.gainOutputL > 0)
						{
							fxSettings.gainOutputL -= 0.1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// GAIN: R OUTPUT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.gainOutputR < 5.0f)
						{
							fxSettings.gainOutputR += 0.1;
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.gainOutputR > 0)
						{
							fxSettings.gainOutputR -= 0.1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_SM:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// SM: WAVEFORM
					if (button == BUTTON_SELECT)
					{
						// edit seq
						uiMode = UI_MODE_SEQ;
						uiSubRow = 0;
						uiSubCol = 0;
						uiSubRowOffset = 0;
					} else if (button == BUTTON_UP) {
						if (fxSettings.smType[uiSM] < (SM_TYPE_MAX - 1))
						{
							fxSettings.smType[uiSM] += 1;
							FXSMSet(uiSM);
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.smType[uiSM] > 0)
						{
							fxSettings.smType[uiSM] -= 1;
							FXSMSet(uiSM);
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// SM: FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.smFreq[uiSM] += 0.1f * buttonMult;
						fxSettings.smFreq[uiSM] = MIN(fxSettings.smFreq[uiSM], (sysSampleRate / 2) / 10);
						FXSMSet(uiSM);
					} else if (button == BUTTON_DOWN) {
						fxSettings.smFreq[uiSM] -= 0.1f * buttonMult;
						fxSettings.smFreq[uiSM] = MAX(fxSettings.smFreq[uiSM], 0.0f);
						FXSMSet(uiSM);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 10;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 10)) {

					// SM: AMP
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.smAmp[uiSM] += 0.01f * buttonMult;
						fxSettings.smAmp[uiSM] = MIN(fxSettings.smAmp[uiSM], 9.99f);
						FXSMSet(uiSM);
					} else if (button == BUTTON_DOWN) {
						fxSettings.smAmp[uiSM] -= 0.01f * buttonMult;
						fxSettings.smAmp[uiSM] = MAX(fxSettings.smAmp[uiSM], 0.0f);
						FXSMSet(uiSM);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 14;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 14)) {

					// SM: OFFSET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.smOffset[uiSM] += 0.01f * buttonMult;
						fxSettings.smOffset[uiSM] = MIN(fxSettings.smOffset[uiSM], 0.99f);
						FXSMSet(uiSM);
					} else if (button == BUTTON_DOWN) {
						fxSettings.smOffset[uiSM] -= 0.01f * buttonMult;
						fxSettings.smOffset[uiSM] = MAX(fxSettings.smOffset[uiSM], 0.0f);
						FXSMSet(uiSM);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 4)) {
				
					// SM: SYNC MOD
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.smSyncMod[uiSM] < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.smSyncMod[uiSM]++;
						} else {
							fxSettings.smSyncMod[uiSM] = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 14;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 6)) {

					// SM: SYNC LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.smSyncLevel[uiSM] += 0.01f * buttonMult;
						fxSettings.smSyncLevel[uiSM] = MIN(fxSettings.smSyncLevel[uiSM], 1.0f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.smSyncLevel[uiSM] -= 0.01f * buttonMult;
						fxSettings.smSyncLevel[uiSM] = MAX(fxSettings.smSyncLevel[uiSM], 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_SEQ:
				// left/right: switch between Step, Midi och Freq
				// up/down: move up/down if on Step, else change value
				// select: toggle seq end/len
				// move up when on step 0: exit
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// SEQ: STEP
					if (button == BUTTON_SELECT)
					{
						fxSettings.smSeqLen[uiSM] = uiSubRowOffset + 1;
					} else if (button == BUTTON_UP) {
						if (uiSubRowOffset > 0)
						{
							uiSubRowOffset--;
						}
					} else if (button == BUTTON_DOWN) {
						if (uiSubRowOffset < (SEQ_STEPS - 1))
						{
							uiSubRowOffset++;
						}
					} else if (button == BUTTON_LEFT) {
						if (uiSubRowOffset == 0)
						{
							uiMode = UI_MODE_SM;
							uiSubCol = 0;
							uiSubRow = 0;
						}
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// SEQ: MIDI
					if (button == BUTTON_SELECT)
					{
						fxSettings.smSeqMidi[uiSM][uiSubRowOffset] = 36;
						fxSettings.smSeqVal[uiSM][uiSubRowOffset] = mtoval(fxSettings.smSeqMidi[uiSM][uiSubRowOffset]);
					} else if (button == BUTTON_UP) {
						if (fxSettings.smSeqMidi[uiSM][uiSubRowOffset] < 127)
						{
							if (buttonMult + fxSettings.smSeqMidi[uiSM][uiSubRowOffset] > 127)
							{
								fxSettings.smSeqMidi[uiSM][uiSubRowOffset] = 127;
							} else {
								fxSettings.smSeqMidi[uiSM][uiSubRowOffset] += buttonMult;
							}
							fxSettings.smSeqVal[uiSM][uiSubRowOffset] = mtoval(fxSettings.smSeqMidi[uiSM][uiSubRowOffset]);
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.smSeqMidi[uiSM][uiSubRowOffset] > 0)
						{
							if (buttonMult > fxSettings.smSeqMidi[uiSM][uiSubRowOffset])
							{
								fxSettings.smSeqMidi[uiSM][uiSubRowOffset] = 0;
							} else {
								fxSettings.smSeqMidi[uiSM][uiSubRowOffset] -= buttonMult;
							}
							fxSettings.smSeqVal[uiSM][uiSubRowOffset] = mtoval(fxSettings.smSeqMidi[uiSM][uiSubRowOffset]);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 7;
					}
					
				} else if ((uiSubRow == 0) && (uiSubCol == 7)) {

					// SEQ: VAL
					if (button == BUTTON_SELECT)
					{
						fxSettings.smSeqVal[uiSM][uiSubRowOffset] = 0.5;
					} else if (button == BUTTON_UP) {
						fxSettings.smSeqVal[uiSM][uiSubRowOffset] += 0.001f * buttonMult;
						fxSettings.smSeqVal[uiSM][uiSubRowOffset] = MIN(fxSettings.smSeqVal[uiSM][uiSubRowOffset], 4);
					} else if (button == BUTTON_DOWN) {
						fxSettings.smSeqVal[uiSM][uiSubRowOffset] -= 0.001f * buttonMult;
						fxSettings.smSeqVal[uiSM][uiSubRowOffset] = MAX(fxSettings.smSeqVal[uiSM][uiSubRowOffset], 0);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_LFO:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// LFO: WAVEFORM
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (fxSettings.lfoWaveform[uiLFO] < (WAVEFORMS_MAX - 1))
						{
							fxSettings.lfoWaveform[uiLFO] += 1;
							FXLFOSet(uiLFO);
						}
					} else if (button == BUTTON_DOWN) {
						if (fxSettings.lfoWaveform[uiLFO] > 0)
						{
							fxSettings.lfoWaveform[uiLFO] -= 1;
							FXLFOSet(uiLFO);
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// LFO: FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.lfoFreq[uiLFO] += 0.1f * buttonMult;
						fxSettings.lfoFreq[uiLFO] = MIN(fxSettings.lfoFreq[uiLFO], (sysSampleRate / 2) / 10);
						FXLFOSet(uiLFO);
					} else if (button == BUTTON_DOWN) {
						fxSettings.lfoFreq[uiLFO] -= 0.1f * buttonMult;
						fxSettings.lfoFreq[uiLFO] = MAX(fxSettings.lfoFreq[uiLFO], 0.0f);
						FXLFOSet(uiLFO);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 10;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 10)) {

					// LFO: AMP
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.lfoAmp[uiLFO] += 0.01f * buttonMult;
						fxSettings.lfoAmp[uiLFO] = MIN(fxSettings.lfoAmp[uiLFO], 9.99f);
						FXLFOSet(uiLFO);
					} else if (button == BUTTON_DOWN) {
						fxSettings.lfoAmp[uiLFO] -= 0.01f * buttonMult;
						fxSettings.lfoAmp[uiLFO] = MAX(fxSettings.lfoAmp[uiLFO], 0.0f);
						FXLFOSet(uiLFO);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 14;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 14)) {

					// LFO: OFFSET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.lfoOffset[uiLFO] += 0.01f * buttonMult;
						fxSettings.lfoOffset[uiLFO] = MIN(fxSettings.lfoOffset[uiLFO], 0.99f);
						FXLFOSet(uiLFO);
					} else if (button == BUTTON_DOWN) {
						fxSettings.lfoOffset[uiLFO] -= 0.01f * buttonMult;
						fxSettings.lfoOffset[uiLFO] = MAX(fxSettings.lfoOffset[uiLFO], 0.0f);
						FXLFOSet(uiLFO);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;
				
			case UI_MODE_EG:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// EG: Adsr
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.egAttack[uiEG] += 0.01f * buttonMult;
						fxSettings.egAttack[uiEG] = MIN(fxSettings.egAttack[uiEG], 9.0f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_DOWN) {
						fxSettings.egAttack[uiEG] -= 0.01f * buttonMult;
						fxSettings.egAttack[uiEG] = MAX(fxSettings.egAttack[uiEG], 0.0f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// EG: aDsr
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.egDecay[uiEG] += 0.01f * buttonMult;
						fxSettings.egDecay[uiEG] = MIN(fxSettings.egDecay[uiEG], 9.0f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_DOWN) {
						fxSettings.egDecay[uiEG] -= 0.01f * buttonMult;
						fxSettings.egDecay[uiEG] = MAX(fxSettings.egDecay[uiEG], 0.0f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// EG: adSr - level, not time (from 0-1)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.egSustain[uiEG] += 0.01f * buttonMult;
						fxSettings.egSustain[uiEG] = MIN(fxSettings.egSustain[uiEG], 0.99f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_DOWN) {
						fxSettings.egSustain[uiEG] -= 0.01f * buttonMult;
						fxSettings.egSustain[uiEG] = MAX(fxSettings.egSustain[uiEG], 0.0f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// EG: adsR
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.egRelease[uiEG] += 0.01f * buttonMult;
						fxSettings.egRelease[uiEG] = MIN(fxSettings.egRelease[uiEG], 9.99f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_DOWN) {
						fxSettings.egRelease[uiEG] -= 0.01f * buttonMult;
						fxSettings.egRelease[uiEG] = MAX(fxSettings.egRelease[uiEG], 0.0f);
						FXEGSet(uiEG);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// EG: GATE LEVEL (0-0.99)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.egGateLevel[uiEG] += 0.01f * buttonMult;
						fxSettings.egGateLevel[uiEG] = MIN(fxSettings.egGateLevel[uiEG], 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.egGateLevel[uiEG] -= 0.01f * buttonMult;
						fxSettings.egGateLevel[uiEG] = MAX(fxSettings.egGateLevel[uiEG], 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
						uiSubRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 3)) {

					// EG: GATE MOD
					if (button == BUTTON_SELECT) {
						if (fxSettings.egGateMod[uiEG] < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.egGateMod[uiEG]++;
						} else {
							fxSettings.egGateMod[uiEG] = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;



			case UI_MODE_CV:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// CV: AMP
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.cvAmp[uiCV] += 0.01f * buttonMult;
						fxSettings.cvAmp[uiCV] = MIN(fxSettings.cvAmp[uiCV], 9.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.cvAmp[uiCV] -= 0.01f * buttonMult;
						fxSettings.cvAmp[uiCV] = MAX(fxSettings.cvAmp[uiCV], 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// CV: OFFSET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.cvOffset[uiCV] += 0.01f * buttonMult;
						fxSettings.cvOffset[uiCV] = MIN(fxSettings.cvOffset[uiCV], 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.cvOffset[uiCV] -= 0.01f * buttonMult;
						fxSettings.cvOffset[uiCV] = MAX(fxSettings.cvOffset[uiCV], -0.99f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_UTILITY:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// UTILITY: ADC
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_UTILITY_ADC;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 1;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 1)) {

					// UTILITY: SAVE TO FLASH
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						UtilFlashSave();
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 2;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 2)) {

					// UTILITY: LOAD FROM FLASH
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						UtilFlashLoad();
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 1;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// UTILITY: GATE LEN
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_GATE;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// OVERVIEW: CVGATE OUT MENU
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_CVGATE_OUT;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 0;
					}

				}
				break;

			case UI_MODE_UTILITY_ADC:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// UTILITY: ADC
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_UTILITY;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
					}
				}
				break;

			case UI_MODE_GATE:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// GATE: GATE LEN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fxSettings.smGateLen += 0.01f * buttonMult;
						fxSettings.smGateLen = MIN(fxSettings.smGateLen, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.smGateLen -= 0.01 * buttonMult;
						fxSettings.smGateLen = MAX(fxSettings.smGateLen, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
					}
				}
				break;

			case UI_MODE_CVGATE_OUT:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// CVGATE OUT: CV0
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.outCV0Mod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.outCV0Mod++;
						} else {
							fxSettings.outCV0Mod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// CVGATE OUT: CV1
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.outCV1Mod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.outCV1Mod++;
						} else {
							fxSettings.outCV1Mod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// CVGATE OUT: GATE0
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.outGate0Mod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.outGate0Mod++;
						} else {
							fxSettings.outGate0Mod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// CVGATE OUT: GATE1
					if (button == BUTTON_SELECT)
					{
						if (fxSettings.outGate1Mod < (MOD_SOURCE_MAX - 1))				
						{
							fxSettings.outGate1Mod++;
						} else {
							fxSettings.outGate1Mod = MOD_SOURCE_NONE;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 8;
					}
					
				} else if ((uiSubRow == 1) && (uiSubCol == 8)) {
				
					// CVGATE OUT: GATE0 LEVEL (0-0.99)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.outGate0Level += 0.01f * buttonMult;
						fxSettings.outGate0Level = MIN(fxSettings.outGate0Level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.outGate0Level -= 0.01f * buttonMult;
						fxSettings.outGate0Level = MAX(fxSettings.outGate0Level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 12;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 12;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 12)) {
				
					// CVGATE OUT: GATE0 LEVEL (0-0.99)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						fxSettings.outGate1Level += 0.01f * buttonMult;
						fxSettings.outGate1Level = MIN(fxSettings.outGate1Level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fxSettings.outGate1Level -= 0.01f * buttonMult;
						fxSettings.outGate1Level = MAX(fxSettings.outGate1Level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 1;
						uiSubCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
					
				}
				break;
			}

			uiRedraw = true;

		}
	}
}



void OscUI::bufferDrawInt(char * aBufferTo, uint8_t aFrom, uint8_t aTo, int aInt)
{
	char aBufferFrom[11];
	sprintf(aBufferFrom, "%d", aInt);

	int i = 0;
	while (aBufferFrom[i] != '\0' && i < (aTo - aFrom + 1))
	{
		aBufferTo[aFrom + i] = aBufferFrom[i];
		i++;
	}
}



void OscUI::bufferDrawStr(char * aBufferTo, uint8_t aTo, const char * aBufferFrom)
{
	int i = 0;
	while ((aBufferFrom[i] != '\0') && (i < 16))
	{
		aBufferTo[aTo + i] = aBufferFrom[i];
		i++;
	}
}


void OscUI::Draw(void) {

	if (uiRedraw)
	{
		char buffer0[17] = "                "; // 16 blanks
		char buffer1[17] = "                "; // 16 blanks
		buffer0[16] = '\0'; // necessary?
		buffer1[16] = '\0'; // necessary?

		// clear is unneccessary since we write all characters every time; this produces better results on LCD
		//lcd->Clear();

		switch (uiMode)
		{
		case UI_MODE_OVERVIEW:

			// row 0

			// input
			switch (fxSettings.inputChannel)
			{
			case INPUT_CHANNEL_STEREO:
				buffer0[0] = 'S';
				break;
			case INPUT_CHANNEL_LEFT:
				buffer0[0] = 'L';
				break;
			case INPUT_CHANNEL_RIGHT:
				buffer0[0] = 'R';
				break;
			case INPUT_CHANNEL_MERGE:
				buffer0[0] = 'M';
				break;
			}
			
			// Utility menu
			buffer0[1] = 'U';

			// modulators - EG
			buffer0[2] = 'E';
			buffer0[3] = 'E';
			
			// modulators - special
			buffer0[4] = 'S';
			buffer0[5] = 'S';
			buffer0[6] = 'S';
			// LFO
			buffer0[7] = 'L';
			buffer0[8] = 'L';
			buffer0[9] = 'L';
			// CV/Gate
			buffer0[10] = 'C';
			buffer0[11] = 'C';
			buffer0[12] = 'G';
			buffer0[13] = 'G';
			// pot
			buffer0[14] = 'P';
			buffer0[15] = 'P';

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);
			
			// row 1

			// Submenus
			if (fxSettings.oscOn)
			{
				bufferDrawStr(buffer1, 0, "O");
			} else {
				bufferDrawStr(buffer1, 0, "-");
			}
			if (fxSettings.slicerOn)
			{
				bufferDrawStr(buffer1, 1, "S");
			} else {
				bufferDrawStr(buffer1, 1, "-");
			}
			if (fxSettings.decimatorOn)
			{
				bufferDrawStr(buffer1, 2, "D");
			} else {
				bufferDrawStr(buffer1, 2, "-");
			}
			if (fxSettings.overdriveOn)
			{
				bufferDrawStr(buffer1, 3, "O");
			} else {
				bufferDrawStr(buffer1, 3, "-");
			}
			if (fxSettings.filterOn)
			{
				bufferDrawStr(buffer1, 4, "F");
			} else {
				bufferDrawStr(buffer1, 4, "-");
			}
			if (fxSettings.adsrOn)
			{
				bufferDrawStr(buffer1, 5, "A");
			} else {
				bufferDrawStr(buffer1, 5, "-");
			}
			if (fxSettings.panOn)
			{
				bufferDrawStr(buffer1, 6, "P");
			} else {
				bufferDrawStr(buffer1, 6, "-");
			}
			if (fxSettings.delayOn)
			{
				bufferDrawStr(buffer1, 7, "Y");
			} else {
				bufferDrawStr(buffer1, 7, "-");
			}
			if (fxSettings.chorusOn)
			{
				bufferDrawStr(buffer1, 8, "C");
			} else {
				bufferDrawStr(buffer1, 8, "-");
			}
			if (fxSettings.reverbOn)
			{
				bufferDrawStr(buffer1, 9, "R");
			} else {
				bufferDrawStr(buffer1, 9, "-");
			}

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiMainRow, uiMainCol);
			
			break;

		case UI_MODE_OSC:
			bufferDrawStr(buffer0, 0, oscWaveNames[fxSettings.oscWaveform]);
			if (fxSettings.oscFreqMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 3, 7, (int)fxSettings.oscFreq);
			} else {
				bufferDrawStr(buffer0, 3, modSourceNames[fxSettings.oscFreqMod]);
			}
			if (fxSettings.oscAmpMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 9, 10, (int)(fxSettings.oscAmp * 100));
			} else {
				bufferDrawStr(buffer0, 9, modSourceNames[fxSettings.oscAmpMod]);
			}
			bufferDrawInt(buffer0, 11, 15, (int)fxSettings.oscDetune);

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, oscWaveNames[fxSettings.oscWaveform2]);
			if (fxSettings.oscAmp2Mod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer1, 9, 10, (int)(fxSettings.oscAmp2 * 100));
			} else {
				bufferDrawStr(buffer1, 9, modSourceNames[fxSettings.oscAmp2Mod]);
			}
			if (fxSettings.oscDetune2Mod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer1, 11, 15, (int)fxSettings.oscDetune2);
			} else {
				bufferDrawStr(buffer1, 11, modSourceNames[fxSettings.oscDetune2Mod]);
			}

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_ADSR:
			bufferDrawInt(buffer0, 0, 2, (int)(fxSettings.adsrAttack * 100));
			bufferDrawInt(buffer0, 3, 5, (int)(fxSettings.adsrDecay * 100));
			bufferDrawInt(buffer0, 6, 8, (int)(fxSettings.adsrSustain * 100));
			bufferDrawInt(buffer0, 9, 11, (int)(fxSettings.adsrRelease * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawInt(buffer1, 0, 1, (int)(fxSettings.adsrGateLevel * 100));
			bufferDrawStr(buffer1, 3, modSourceNames[fxSettings.adsrGateMod]);
			bufferDrawStr(buffer1, 6, modSourceNames[fxSettings.adsrSustainMod]);
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_FILTER:
			switch (fxSettings.filterType)
			{
			case FILTER_TYPE_LOW:
				buffer0[0] = 'L';
				break;
			case FILTER_TYPE_HIGH:
				buffer0[0] = 'H';
				break;
			case FILTER_TYPE_BAND:
				buffer0[0] = 'B';
				break;
			}

			if (fxSettings.filterFreqMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 2, 6, (int)fxSettings.filterFreq);
			} else {
				bufferDrawStr(buffer0, 2, modSourceNames[fxSettings.filterFreqMod]);
			}
			if (fxSettings.filterResMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 8, 9, (int)(fxSettings.filterRes * 100));
			} else {
				bufferDrawStr(buffer0, 8, modSourceNames[fxSettings.filterResMod]);
			}

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_DECIMATOR:
			bufferDrawInt(buffer0, 0, 1, (int)(fxSettings.decimatorDownsampleFactor * 100));
			if (fxSettings.decimatorBitcrushFactorMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 3, 4, (int)(fxSettings.decimatorBitcrushFactor * 100));
			} else {
				bufferDrawStr(buffer0, 3, modSourceNames[fxSettings.decimatorBitcrushFactorMod]);
			}
			bufferDrawInt(buffer0, 6, 7, (int)(fxSettings.decimatorBitsToCrush));
			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_OVERDRIVE:
			if (fxSettings.overdriveGainMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 0, 2, (int)(fxSettings.overdriveGain * 100));
			} else {
				bufferDrawStr(buffer0, 0, modSourceNames[fxSettings.overdriveGainMod]);
			}
			if (fxSettings.overdriveDriveMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 4, 6, (int)(fxSettings.overdriveDrive * 100));
			} else {
				bufferDrawStr(buffer0, 4, modSourceNames[fxSettings.overdriveDriveMod]);
			}

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;
		
		case UI_MODE_PAN:;
			if (fxSettings.panPanMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 0, 2, (int)(fxSettings.panPan * 100));
			} else {
				bufferDrawStr(buffer0, 0, modSourceNames[fxSettings.panPanMod]);
			}

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);


			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_DELAY:
			if (fxSettings.delayDelayMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 0, 2, (int)(fxSettings.delayDelayL * 100));
				bufferDrawInt(buffer1, 0, 2, (int)(fxSettings.delayDelayR * 100));
			} else {
				bufferDrawStr(buffer0, 0, modSourceNames[fxSettings.delayDelayMod]);
				bufferDrawStr(buffer1, 0, modSourceNames[fxSettings.delayDelayMod]);
			}

			if (fxSettings.delayFeedbackMod == MOD_SOURCE_NONE)
			{
				bufferDrawInt(buffer0, 4, 5, (int)(fxSettings.delayFeedbackL * 100));
				bufferDrawInt(buffer1, 4, 5, (int)(fxSettings.delayFeedbackR * 100));
			} else {
				bufferDrawStr(buffer0, 4, modSourceNames[fxSettings.delayFeedbackMod]);
				bufferDrawStr(buffer1, 4, modSourceNames[fxSettings.delayFeedbackMod]);
			}

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);


			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_CHORUS:
			bufferDrawInt(buffer0, 0, 1, (int)(fxSettings.chorusDelay * 100));
			bufferDrawInt(buffer0, 3, 4, (int)(fxSettings.chorusFeedback * 100));
			bufferDrawInt(buffer0, 6, 7, (int)(fxSettings.chorusLfoDepth * 100));
			bufferDrawInt(buffer0, 9, 13, (int)(fxSettings.chorusLfoFreq));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawInt(buffer1, 0, 1, (int)(fxSettings.chorusDry * 100));
			bufferDrawInt(buffer1, 3, 4, (int)(fxSettings.chorusWet * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_REVERB:
			bufferDrawInt(buffer0, 0, 4, (int)fxSettings.reverbLPFFreq);
			bufferDrawInt(buffer0, 6, 7, (int)(fxSettings.reverbFeedback * 100));
			bufferDrawInt(buffer0, 9, 10, (int)(fxSettings.reverbDry * 100));
			bufferDrawInt(buffer0, 12, 13, (int)(fxSettings.reverbWet * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_SLICER:
			bufferDrawInt(buffer0, 0, 2, (int)fxSettings.slicer_record_samples_max / 1000);
			bufferDrawStr(buffer0, 4, modSourceNames[fxSettings.slicerRSMMod]);
			bufferDrawInt(buffer0, 7, 8, (int)(fxSettings.slicerRSMModGateLevel * 100));
			if (fxSettings.slicerTrigMode)
				buffer0[10] = 'T';
			else
				buffer0[10] = '-';

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawInt(buffer1, 0, 6, (int)fxSettings.slicer_playback_rep_max);
			bufferDrawStr(buffer1, 4, modSourceNames[fxSettings.slicerPRMMod]);
			bufferDrawInt(buffer1, 7, 8, (int)(fxSettings.slicerPRMModGateLevel * 100));
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_GAIN:
			bufferDrawInt(buffer0, 0, 1, (int)(fxSettings.gainInputL * 10));
			bufferDrawInt(buffer0, 2, 3, (int)(fxSettings.gainInputR * 10));
			bufferDrawInt(buffer0, 4, 5, (int)(fxSettings.gainOutputL * 10));
			bufferDrawInt(buffer0, 6, 7, (int)(fxSettings.gainOutputR * 10));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_SM:
			bufferDrawStr(buffer0, 0, smTypeNames[fxSettings.smType[uiSM]]);
			bufferDrawInt(buffer0, 4, 8, (int)(fxSettings.smFreq[uiSM] * 10));
			bufferDrawInt(buffer0, 10, 12, (int)(fxSettings.smAmp[uiSM] * 100));
			bufferDrawInt(buffer0, 14, 15, (int)(fxSettings.smOffset[uiSM] * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 4, modSourceNames[fxSettings.smSyncMod[uiSM]]);
			bufferDrawInt(buffer1, 6, 8, (int)(fxSettings.smSyncLevel[uiSM] * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_EG:
			bufferDrawInt(buffer0, 0, 2, (int)(fxSettings.egAttack[uiEG] * 100));
			bufferDrawInt(buffer0, 3, 5, (int)(fxSettings.egDecay[uiEG] * 100));
			bufferDrawInt(buffer0, 6, 8, (int)(fxSettings.egSustain[uiEG] * 100));
			bufferDrawInt(buffer0, 9, 11, (int)(fxSettings.egRelease[uiEG] * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawInt(buffer1, 0, 1, (int)(fxSettings.egGateLevel[uiEG] * 100));
			bufferDrawStr(buffer1, 3, modSourceNames[fxSettings.egGateMod[uiEG]]);
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_LFO:
			bufferDrawStr(buffer0, 0, oscWaveNames[fxSettings.lfoWaveform[uiLFO]]);
			bufferDrawInt(buffer0, 4, 8, (int)(fxSettings.lfoFreq[uiLFO] * 10));
			bufferDrawInt(buffer0, 10, 12, (int)(fxSettings.lfoAmp[uiLFO] * 100));
			bufferDrawInt(buffer0, 14, 15, (int)(fxSettings.lfoOffset[uiLFO] * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_CV:
			bufferDrawInt(buffer0, 0, 2, (int)(fxSettings.cvAmp[uiCV] * 100));
			bufferDrawInt(buffer0, 4, 6, (int)(fxSettings.cvOffset[uiCV] * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_UTILITY:
			buffer0[0] = 'A'; // show AD values
			buffer0[1] = 'S'; // save to Flash
			buffer0[2] = 'L'; // save to Flash
			buffer0[3] = 'G'; // set sm gate len
			buffer0[4] = '>'; // CV/Gate out

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_UTILITY_ADC:
			for (uint8_t i = 0; i < CV_NUMBER; i++)
			{
				bufferDrawInt(buffer0, 0 + i*3, 3 + i*3, (int)(modSources.cvValue[i] * 1000));
			}

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_CVGATE_OUT:
			bufferDrawStr(buffer0, 0, modSourceNames[fxSettings.outCV0Mod]);
			bufferDrawStr(buffer0, 4, modSourceNames[fxSettings.outCV1Mod]);
			bufferDrawStr(buffer0, 8, modSourceNames[fxSettings.outGate0Mod]);
			bufferDrawStr(buffer0, 12, modSourceNames[fxSettings.outGate1Mod]);

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);
			
			bufferDrawInt(buffer1, 8, 10, (int)(fxSettings.outGate0Level * 100));
			bufferDrawInt(buffer1, 12, 14, (int)(fxSettings.outGate1Level * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_SEQ:
			bufferDrawInt(buffer0, 0, 1, (int)uiSubRowOffset);
			bufferDrawInt(buffer0, 3, 5, (int)fxSettings.smSeqMidi[uiSM][uiSubRowOffset]);
			bufferDrawInt(buffer0, 7, 10, (int)(fxSettings.smSeqVal[uiSM][uiSubRowOffset] * 1000));
			if (uiSubRowOffset == (fxSettings.smSeqLen[uiSM] - 1))
			{
				buffer0[13] = '*';
			}
			
			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;
		
		case UI_MODE_GATE:
			bufferDrawInt(buffer0, 0, 1, (int)(fxSettings.smGateLen * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;
		}

		uiRedraw = false;
	}

}
