// UI

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "ui.h"
#include "vasynth.h"

#include "dev/lcd_hd44780.h"

using namespace daisy;
using namespace daisysp;



// globals
extern DaisySeed hardware;
extern float sysSampleRate;
extern uint8_t sysLedMode;
extern bool uiRedraw;
extern VASynth vasynth;

// FX

char oscWaveNames[WAVEFORMS_MAX][4] = {"SIN", "TRI", "SAW", "RAM", "SQU", "TRP", "SAP", "SQP"};
char lfoTargetName[4][3] = {"--", "PI", "FI", "EG"};
char potTargetName[2][4] = {"---", "FIL"};

// preset
extern uint8_t preset_max;
extern uint8_t preset_number;
extern char preset_name[PRESET_MAX][15];
extern VASynthSetting preset_setting[PRESET_MAX];


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

}

// RIGHT	0
// UP		9
// DOWN		25
// LEFT 	39
// SELECT 	62
// else:	99

void OscUI::Button(float aButtonValue) {

	int aInButtonValue = (int)(aButtonValue*100);

	if (aInButtonValue < 5) {
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
					} else if (button == BUTTON_UP) {
						switch (vasynth.input_channel_)
						{
						case INPUT_CHANNEL_NONE:
							vasynth.input_channel_ = INPUT_CHANNEL_MERGE;
							break;
						case INPUT_CHANNEL_MERGE:
							vasynth.input_channel_ = INPUT_CHANNEL_NONE;
							break;
						}
					} else if (button == BUTTON_DOWN) {
						switch (vasynth.input_channel_)
						{
						case INPUT_CHANNEL_NONE:
							vasynth.input_channel_ = INPUT_CHANNEL_MERGE;
							break;
						case INPUT_CHANNEL_MERGE:
							vasynth.input_channel_ = INPUT_CHANNEL_NONE;
							break;
						}
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 1;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 1)) {

					// OVERVIEW: MIX
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_MIX;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 2;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 2)) {

					// OVERVIEW: UTILITY MENU
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_UTILITY;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 1;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 3;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 3)) {

					// OVERVIEW: LFO
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_LFO;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 4;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 4)) {

					// OVERVIEW: TEST
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_TEST;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 5;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 5)) {

					// OVERVIEW: LOAD
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_LOAD;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 6;
					}
				} else if ((uiMainRow == 0) && (uiMainCol == 6)) {

					// OVERVIEW: SAVE
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_SAVE;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 5;
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
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 6;
						uiMainRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 1;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 1)) {

					// OVERVIEW: OSC EG P
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_OSC_EG;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 2;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 2)) {

					// OVERVIEW: FILTER
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_FILTER;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 1;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 3;
					}

				} else if ((uiMainRow == 1) && (uiMainCol == 3)) {

					// OVERVIEW: FILTER EG F
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_FILTER_EG;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 4;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 4)) {

					// OVERVIEW: ADSR / EG A
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_AMP_EG;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 5;
					}
				} else if ((uiMainRow == 1) && (uiMainCol == 5)) {

					// OVERVIEW: DELAY
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_DELAY;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiMainCol = 6;
					}

				} else if ((uiMainRow == 1) && (uiMainCol == 6)) {

					// OVERVIEW: REVERB
					if (button == BUTTON_SELECT)
					{
						uiMode = UI_MODE_REVERB;
						uiSubRow = 0;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						uiMainCol = 5;
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
						if (vasynth.waveform_ < (WAVEFORMS_MAX - 1))
						{
							vasynth.waveform_ += 1;
							vasynth.SetWaveform();
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.waveform_ > 0)
						{
							vasynth.waveform_ -= 1;
							vasynth.SetWaveform();
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// OSC: DETUNE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.detune_ += 1 * buttonMult;
						vasynth.detune_ = MIN(vasynth.detune_, (sysSampleRate / 2));
					} else if (button == BUTTON_DOWN) {
						vasynth.detune_ -= 1 * buttonMult;;
						vasynth.detune_ = MAX(vasynth.detune_, 1);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 10;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 10)) {

					// OSC: VOICES (NUMBER)
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP)
					{
						if (vasynth.voices_ < VOICES_MAX)
						{
							vasynth.voices_++;
							vasynth.osc_next_ = 0;
							vasynth.SetWaveform();
							vasynth.SetEG();
							vasynth.SetFilter();
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.voices_ > 0)
						{
							vasynth.voices_--;
							vasynth.osc_next_ = 0;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 13;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 13)) {

					// OSC: PORTAMENTO
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.portamento_ += 0.01 * buttonMult;
						vasynth.portamento_ = MIN(vasynth.portamento_, 9.99);
						vasynth.SetWaveform();
					} else if (button == BUTTON_DOWN) {
						vasynth.portamento_ -= 0.01 * buttonMult;;
						vasynth.portamento_ = MAX(vasynth.portamento_, 0);
						vasynth.SetWaveform();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 1;
						uiSubCol = 0;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// OSC2: WAVEFORM
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (vasynth.osc2_waveform_ < (WAVEFORMS_MAX - 1))
						{
							vasynth.osc2_waveform_ += 1;
							vasynth.SetWaveform();
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.osc2_waveform_ > 0)
						{
							vasynth.osc2_waveform_ -= 1;
							vasynth.SetWaveform();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 13;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 4)) {

					// OSC2: DETUNE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.osc2_detune_ += 1 * buttonMult;
						vasynth.osc2_detune_ = MIN(vasynth.osc2_detune_, (sysSampleRate / 2));
					} else if (button == BUTTON_DOWN) {
						vasynth.osc2_detune_ -= 1 * buttonMult;;
						vasynth.osc2_detune_ = MAX(vasynth.osc2_detune_, 0);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 10;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 10)) {

					// OSC2: TRANSPOSE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (vasynth.osc2_transpose_ < 99)
						{
							vasynth.osc2_transpose_++;
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.osc2_transpose_ > -99)
						{
							vasynth.osc2_transpose_--;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 13;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 13)) {

					// OSC2: LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.osc2_level_ += 0.01f * buttonMult;
						vasynth.osc2_level_ = MIN(vasynth.osc2_level_, 0.99);
					} else if (button == BUTTON_DOWN) {
						vasynth.osc2_level_ -= 0.01f * buttonMult;;
						vasynth.osc2_level_ = MAX(vasynth.osc2_level_, 0);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_OSC_EG:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// ADSR: EG_P
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_p_attack_ += 0.01f * buttonMult;
						vasynth.eg_p_attack_ = MIN(vasynth.eg_p_attack_, 9.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_p_attack_ -= 0.01f * buttonMult;
						vasynth.eg_p_attack_ = MAX(vasynth.eg_p_attack_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// ADSR: aDsr
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.eg_p_decay_ += 0.01f * buttonMult;
						vasynth.eg_p_decay_ = MIN(vasynth.eg_p_decay_, 9.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_p_decay_ -= 0.01f * buttonMult;
						vasynth.eg_p_decay_ = MAX(vasynth.eg_p_decay_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// ADSR: adSr - level, not time (from 0-1)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_p_sustain_ += 0.01f * buttonMult;
						vasynth.eg_p_sustain_ = MIN(vasynth.eg_p_sustain_, 1.00f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_p_sustain_ -= 0.01f * buttonMult;
						vasynth.eg_p_sustain_ = MAX(vasynth.eg_p_sustain_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// ADSR: adsR
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_p_release_ += 0.01f * buttonMult;
						vasynth.eg_p_release_ = MIN(vasynth.eg_p_release_, 9.99f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_p_release_ -= 0.01f * buttonMult;
						vasynth.eg_p_release_ = MAX(vasynth.eg_p_release_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_FILTER_EG:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// ADSR: EG_F
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_f_attack_ += 0.01f * buttonMult;
						vasynth.eg_f_attack_ = MIN(vasynth.eg_f_attack_, 9.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_f_attack_ -= 0.01f * buttonMult;
						vasynth.eg_f_attack_ = MAX(vasynth.eg_f_attack_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// ADSR: aDsr
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.eg_f_decay_ += 0.01f * buttonMult;
						vasynth.eg_f_decay_ = MIN(vasynth.eg_f_decay_, 9.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_f_decay_ -= 0.01f * buttonMult;
						vasynth.eg_f_decay_ = MAX(vasynth.eg_f_decay_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// ADSR: adSr - level, not time (from 0-1)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_f_sustain_ += 0.01f * buttonMult;
						vasynth.eg_f_sustain_ = MIN(vasynth.eg_f_sustain_, 1.00f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_f_sustain_ -= 0.01f * buttonMult;
						vasynth.eg_f_sustain_ = MAX(vasynth.eg_f_sustain_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// ADSR: adsR
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_f_release_ += 0.01f * buttonMult;
						vasynth.eg_f_release_ = MIN(vasynth.eg_f_release_, 9.99f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_f_release_ -= 0.01f * buttonMult;
						vasynth.eg_f_release_ = MAX(vasynth.eg_f_release_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_AMP_EG:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// ADSR: EG_A
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_a_attack_ += 0.01f * buttonMult;
						vasynth.eg_a_attack_ = MIN(vasynth.eg_a_attack_, 9.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_a_attack_ -= 0.01f * buttonMult;
						vasynth.eg_a_attack_ = MAX(vasynth.eg_a_attack_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// ADSR: aDsr
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.eg_a_decay_ += 0.01f * buttonMult;
						vasynth.eg_a_decay_ = MIN(vasynth.eg_a_decay_, 9.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_a_decay_ -= 0.01f * buttonMult;
						vasynth.eg_a_decay_ = MAX(vasynth.eg_a_decay_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// ADSR: adSr - level, not time (from 0-1)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_a_sustain_ += 0.01f * buttonMult;
						vasynth.eg_a_sustain_ = MIN(vasynth.eg_a_sustain_, 1.00f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_a_sustain_ -= 0.01f * buttonMult;
						vasynth.eg_a_sustain_ = MAX(vasynth.eg_a_sustain_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// ADSR: adsR
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						vasynth.eg_a_release_ += 0.01f * buttonMult;
						vasynth.eg_a_release_ = MIN(vasynth.eg_a_release_, 9.99f);
						vasynth.SetEG();
					} else if (button == BUTTON_DOWN) {
						vasynth.eg_a_release_ -= 0.01f * buttonMult;
						vasynth.eg_a_release_ = MAX(vasynth.eg_a_release_, 0.0f);
						vasynth.SetEG();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;


			case UI_MODE_FILTER:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// FILTER: TYPE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						switch (vasynth.filter_type_)
						{
						case VASynth::LOW:
							vasynth.filter_type_ = VASynth::HIGH;
							break;
						case VASynth::HIGH:
							vasynth.filter_type_ = VASynth::BAND;
							break;
						case VASynth::BAND:
							vasynth.filter_type_ = VASynth::NOTCH;
							break;
						case VASynth::NOTCH:
							vasynth.filter_type_ = VASynth::PEAK;
							break;
						case VASynth::PEAK:
							vasynth.filter_type_ = VASynth::LOW;
							break;
						}
					} else if (button == BUTTON_DOWN) {
						switch (vasynth.filter_type_)
						{
						case VASynth::LOW:
							vasynth.filter_type_ = VASynth::PEAK;
							break;
						case VASynth::HIGH:
							vasynth.filter_type_ = VASynth::LOW;
							break;
						case VASynth::BAND:
							vasynth.filter_type_ = VASynth::HIGH;
							break;
						case VASynth::NOTCH:
							vasynth.filter_type_ = VASynth::BAND;
							break;
						case VASynth::PEAK:
							vasynth.filter_type_ = VASynth::NOTCH;
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
					} else if (button == BUTTON_UP) {
						vasynth.filter_cutoff_ += 10.0f * buttonMult;
						vasynth.filter_cutoff_ = MIN(vasynth.filter_cutoff_, (sysSampleRate / 2));
						vasynth.SetFilter();
					} else if (button == BUTTON_DOWN) {
						vasynth.filter_cutoff_ -= 10.0f * buttonMult;
						vasynth.filter_cutoff_ = MAX(vasynth.filter_cutoff_, 0.0f);
						vasynth.SetFilter();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// FILTER: RES
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.filter_res_ += 0.01f * buttonMult;
						vasynth.filter_res_ = MIN(vasynth.filter_res_, 0.99f);
						vasynth.SetFilter();
					} else if (button == BUTTON_DOWN) {
						vasynth.filter_res_ -= 0.01f * buttonMult;
						vasynth.filter_res_ = MAX(vasynth.filter_res_, 0.0f);
						vasynth.SetFilter();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_DELAY:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// DELAY: DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (vasynth.delay_delay_ < DELAY_MAX_S)
						{
							vasynth.delay_delay_ += 0.1;
							vasynth.SetDelay();
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.delay_delay_ >= 0.1)
						{
							vasynth.delay_delay_ -= 0.1;
							vasynth.SetDelay();
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// DELAY: FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.delay_feedback_ += 0.01f * buttonMult;
						vasynth.delay_feedback_ = MIN(vasynth.delay_feedback_, 0.99f);
					} else if (button == BUTTON_DOWN) {
						vasynth.delay_feedback_ -= 0.01f * buttonMult;
						vasynth.delay_feedback_ = MAX(vasynth.delay_feedback_, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 4;
					}
				}
				break;


			case UI_MODE_REVERB:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// REVERB: LPF FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.reverb_lpffreq_ += 1.0f * buttonMult;
						vasynth.reverb_lpffreq_ = MIN(vasynth.reverb_lpffreq_, (sysSampleRate / 2));
						vasynth.SetReverb();
					} else if (button == BUTTON_DOWN) {
						vasynth.reverb_lpffreq_ -= 1.0f * buttonMult;
						vasynth.reverb_lpffreq_ = MAX(vasynth.reverb_lpffreq_, 0);
						vasynth.SetReverb();
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
						vasynth.reverb_feedback_ += 0.01f * buttonMult;
						vasynth.reverb_feedback_ = MIN(vasynth.reverb_feedback_, 0.99f);
						vasynth.SetReverb();
					} else if (button == BUTTON_DOWN) {
						vasynth.reverb_feedback_ -= 0.01f * buttonMult;
						vasynth.reverb_feedback_ = MAX(vasynth.reverb_feedback_, 0.0f);
						vasynth.SetReverb();
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
						vasynth.reverb_dry_ += 0.01f * buttonMult;
						vasynth.reverb_dry_ = MIN(vasynth.reverb_dry_, 0.99f);
					} else if (button == BUTTON_DOWN) {
						vasynth.reverb_dry_ -= 0.01f * buttonMult;
						vasynth.reverb_dry_ = MAX(vasynth.reverb_dry_, 0.0f);
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
						vasynth.reverb_wet_ += 0.01 * buttonMult;
						vasynth.reverb_wet_ = MIN(vasynth.reverb_wet_, 0.99f);
					} else if (button == BUTTON_DOWN) {
						vasynth.reverb_wet_ -= 0.01f * buttonMult;
						vasynth.reverb_wet_ = MAX(vasynth.reverb_wet_, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// REVERB: LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.reverb_level_ += 0.01 * buttonMult;
						vasynth.reverb_level_ = MIN(vasynth.reverb_level_, 0.99f);
					} else if (button == BUTTON_DOWN) {
						vasynth.reverb_level_ -= 0.01f * buttonMult;
						vasynth.reverb_level_ = MAX(vasynth.reverb_level_, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 12;
						uiSubRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}
				}
				break;



			case UI_MODE_MIX:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// MIX: LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.level_ += 0.01f * buttonMult;
						vasynth.level_ = MIN(vasynth.level_, 5.00f);
					} else if (button == BUTTON_DOWN) {
						vasynth.level_ -= 0.01 * buttonMult;
						vasynth.level_ = MAX(vasynth.level_, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// MIX: PAN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.pan_ += 0.01f * buttonMult;
						vasynth.pan_ = MIN(vasynth.pan_, 1.00f);
					} else if (button == BUTTON_DOWN) {
						vasynth.pan_ -= 0.01 * buttonMult;
						vasynth.pan_ = MAX(vasynth.pan_, 0.0f);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// MIX: NOISE LEVEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.noise_level_ += 0.01 * buttonMult;
						vasynth.noise_level_ = MIN(vasynth.noise_level_, 0.99);
					} else if (button == BUTTON_DOWN) {
						vasynth.noise_level_ -= 0.01 * buttonMult;
						vasynth.noise_level_ = MAX(vasynth.noise_level_, 0);
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}
				}
				break;


			case UI_MODE_LFO:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// LFO: WAVEFORM
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (vasynth.lfo_waveform_ < (WAVEFORMS_MAX - 1))
						{
							vasynth.lfo_waveform_ += 1;
							vasynth.SetLFO();
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.lfo_waveform_ > 0)
						{
							vasynth.lfo_waveform_ -= 1;
							vasynth.SetLFO();
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
						vasynth.lfo_freq_ += 0.1f * buttonMult;
						vasynth.lfo_freq_ = MIN(vasynth.lfo_freq_, (sysSampleRate / 2) / 10);
						vasynth.SetLFO();
					} else if (button == BUTTON_DOWN) {
						vasynth.lfo_freq_ -= 0.1f * buttonMult;
						vasynth.lfo_freq_ = MAX(vasynth.lfo_freq_, 0.0f);
						vasynth.SetLFO();
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
						vasynth.lfo_amp_ += 0.01f * buttonMult;
						vasynth.lfo_amp_ = MIN(vasynth.lfo_amp_, 9.99f);
						vasynth.SetLFO();
					} else if (button == BUTTON_DOWN) {
						vasynth.lfo_amp_ -= 0.01f * buttonMult;
						vasynth.lfo_amp_ = MAX(vasynth.lfo_amp_, 0.0f);
						vasynth.SetLFO();
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 14;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 14)) {

					// LFO: TARGET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						switch (vasynth.lfo_target_)
						{
						case VASynth::NONE:
							vasynth.lfo_target_ = VASynth::PITCH;
							break;
						case VASynth::PITCH:
							vasynth.lfo_target_ = VASynth::FILTER;
							break;
						case VASynth::FILTER:
							vasynth.lfo_target_ = VASynth::EG;
							break;
						case VASynth::EG:
							vasynth.lfo_target_ = VASynth::NONE;
							break;
						case VASynth::LAST:
							vasynth.lfo_target_ = VASynth::NONE;
							break;
						}
					} else if (button == BUTTON_DOWN) {
						switch (vasynth.lfo_target_)
						{
						case VASynth::NONE:
							vasynth.lfo_target_ = VASynth::EG;
							break;
						case VASynth::PITCH:
							vasynth.lfo_target_ = VASynth::NONE;
							break;
						case VASynth::FILTER:
							vasynth.lfo_target_ = VASynth::PITCH;
							break;
						case VASynth::EG:
							vasynth.lfo_target_ = VASynth::FILTER;
							break;
						case VASynth::LAST:
							vasynth.lfo_target_ = VASynth::NONE;
							break;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_UTILITY:
				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// UTILITY: MIDI CHANNEL
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						vasynth.midi_channel_ += 1;
						vasynth.midi_channel_ = MIN(vasynth.midi_channel_, MIDI_CHANNEL_ALL);
					} else if (button == BUTTON_DOWN) {
						vasynth.midi_channel_ -= 1;
						vasynth.midi_channel_ = MAX(vasynth.midi_channel_, 0);
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// UTILITY: POT0
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (vasynth.pot0_target_ < 1)
						{
							vasynth.pot0_target_ += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.pot0_target_ > 0)
						{
							vasynth.pot0_target_ -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// UTILITY: POT1
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (vasynth.pot1_target_ < 1)
						{
							vasynth.pot1_target_ += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (vasynth.pot1_target_ > 0)
						{
							vasynth.pot1_target_ -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;
				
			case UI_MODE_TEST:
				vasynth.NoteOn(36, 100);
				System::Delay(1000);
				vasynth.NoteOff(36);
				System::Delay(500);
				vasynth.NoteOn(48, 100);
				vasynth.NoteOn(55, 100);
				vasynth.NoteOn(60, 100);
				System::Delay(1000);
				vasynth.NoteOff(48);
				vasynth.NoteOff(55);
				vasynth.NoteOff(60);
				uiMode = UI_MODE_OVERVIEW;
				break;

			case UI_MODE_LOAD:
				if (uiSubRow == 0) {

					// LOAD
					if (button == BUTTON_SELECT)
					{
						vasynth.FlashLoad(uiSubCol);
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
						uiSubRow = 1;
						uiSubCol = 0;
					} else if (button == BUTTON_LEFT) {
						if (uiSubCol == 0)
						{
							uiMode = UI_MODE_OVERVIEW;
						} else {
							uiSubCol--;
						}
					} else if (button == BUTTON_RIGHT) {
						if (uiSubCol == 15)
						{
							uiSubRow = 1;
							uiSubCol = 0;
						} else {
							uiSubCol++;
						}
					}
				} else {
					if (button == BUTTON_SELECT)
					{
						vasynth.SaveToLive(&preset_setting[preset_number]);
					} else if (button == BUTTON_UP) {
						if (preset_number < (preset_max-1))
						{
							preset_number++;
						}
					} else if (button == BUTTON_DOWN) {
						if (preset_number > 0)
						{
							preset_number--;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubRow = 0;
						uiSubCol = 15;
					} else if (button == BUTTON_RIGHT) {
						uiSubRow = 0;
						uiSubCol = 0;
					}
				}
				break;

			case UI_MODE_SAVE:
				if (uiSubRow == 0) {

					// SAVE
					if (button == BUTTON_SELECT)
					{
						vasynth.FlashSave(uiSubCol);
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						if (uiSubCol == 0)
						{
							uiMode = UI_MODE_OVERVIEW;
						} else {
							uiSubCol--;
						}
					} else if (button == BUTTON_RIGHT) {
						if (uiSubCol == 15)
						{
						} else {
							uiSubCol++;
						}
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
			switch (vasynth.input_channel_)
			{
			case INPUT_CHANNEL_NONE:
				buffer0[0] = '-';
				break;
			case INPUT_CHANNEL_MERGE:
				buffer0[0] = 'S';
				break;
			}
			// connect
			buffer0[1] = 'M';
			
			// Utility menu
			buffer0[2] = 'U';

			// LFO
			buffer0[3] = 'L';
			
			// Test
			buffer0[4] = 'T';
			
			// save and load
			buffer0[5] = 'L';
			buffer0[6] = 'S';

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);
			
			// row 1

			// Submenus
			bufferDrawStr(buffer1, 0, "O");
			bufferDrawStr(buffer1, 1, "E");
			bufferDrawStr(buffer1, 2, "F");
			bufferDrawStr(buffer1, 3, "E");
			bufferDrawStr(buffer1, 4, "E");
			bufferDrawStr(buffer1, 5, "Y");
			bufferDrawStr(buffer1, 6, "R");

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiMainRow, uiMainCol);
			
			break;

		case UI_MODE_OSC:
			bufferDrawStr(buffer0, 0, oscWaveNames[vasynth.waveform_]);
			bufferDrawInt(buffer0, 4, 8, (int)vasynth.detune_);
			bufferDrawInt(buffer0, 10, 11, vasynth.voices_);
			bufferDrawInt(buffer0, 13, 15, (int)(vasynth.portamento_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, oscWaveNames[vasynth.osc2_waveform_]);
			bufferDrawInt(buffer1, 4, 8, (int)vasynth.osc2_detune_);
			bufferDrawInt(buffer1, 10, 12, vasynth.osc2_transpose_);
			bufferDrawInt(buffer1, 13, 15, (int)(vasynth.osc2_level_ * 100));


			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_OSC_EG:
			bufferDrawInt(buffer0, 0, 2, (int)(vasynth.eg_p_attack_ * 100));
			bufferDrawInt(buffer0, 4, 6, (int)(vasynth.eg_p_decay_ * 100));
			bufferDrawInt(buffer0, 8, 10, (int)(vasynth.eg_p_sustain_ * 100));
			bufferDrawInt(buffer0, 12, 14, (int)(vasynth.eg_p_release_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_FILTER_EG:
			bufferDrawInt(buffer0, 0, 2, (int)(vasynth.eg_f_attack_ * 100));
			bufferDrawInt(buffer0, 4, 6, (int)(vasynth.eg_f_decay_ * 100));
			bufferDrawInt(buffer0, 8, 10, (int)(vasynth.eg_f_sustain_ * 100));
			bufferDrawInt(buffer0, 12, 14, (int)(vasynth.eg_f_release_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_AMP_EG:
			bufferDrawInt(buffer0, 0, 2, (int)(vasynth.eg_a_attack_ * 100));
			bufferDrawInt(buffer0, 4, 6, (int)(vasynth.eg_a_decay_ * 100));
			bufferDrawInt(buffer0, 8, 10, (int)(vasynth.eg_a_sustain_ * 100));
			bufferDrawInt(buffer0, 12, 14, (int)(vasynth.eg_a_release_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_FILTER:
			switch (vasynth.filter_type_)
			{
			case VASynth::LOW:
				buffer0[0] = 'L';
				break;
			case VASynth::HIGH:
				buffer0[0] = 'H';
				break;
			case VASynth::BAND:
				buffer0[0] = 'B';
				break;
			case VASynth::NOTCH:
				buffer0[0] = 'N';
				break;
			case VASynth::PEAK:
				buffer0[0] = 'P';
				break;
			}

			bufferDrawInt(buffer0, 2, 6, (int)vasynth.filter_cutoff_);
			bufferDrawInt(buffer0, 8, 9, (int)(vasynth.filter_res_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_DELAY:
			bufferDrawInt(buffer0, 0, 2, (int)(vasynth.delay_delay_ * 100));
			bufferDrawInt(buffer0, 4, 5, (int)(vasynth.delay_feedback_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_REVERB:
			bufferDrawInt(buffer0, 0, 4, (int)vasynth.reverb_lpffreq_);
			bufferDrawInt(buffer0, 6, 7, (int)(vasynth.reverb_feedback_ * 100));
			bufferDrawInt(buffer0, 9, 10, (int)(vasynth.reverb_dry_ * 100));
			bufferDrawInt(buffer0, 12, 13, (int)(vasynth.reverb_wet_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawInt(buffer1, 0, 2, (int)vasynth.reverb_level_ * 100);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_MIX:
			bufferDrawInt(buffer0, 0, 2, (int)(vasynth.level_ * 100));
			bufferDrawInt(buffer0, 4, 6, (int)(vasynth.pan_ * 100));
			bufferDrawInt(buffer0, 8, 10, (int)(vasynth.noise_level_ * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_LFO:
			bufferDrawStr(buffer0, 0, oscWaveNames[vasynth.lfo_waveform_]);
			bufferDrawInt(buffer0, 4, 8, vasynth.lfo_freq_);
			bufferDrawInt(buffer0, 10, 12, (int)(vasynth.lfo_amp_ * 100));
			bufferDrawStr(buffer0, 14, lfoTargetName[vasynth.lfo_target_]);

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_UTILITY:
			if (vasynth.midi_channel_ == MIDI_CHANNEL_ALL)
			{
				bufferDrawStr(buffer0, 0, "ALL");
			} else {
				bufferDrawInt(buffer0, 0, 1, vasynth.midi_channel_);
			}

			bufferDrawStr(buffer0, 4, potTargetName[vasynth.pot0_target_]);
			bufferDrawStr(buffer0, 8, potTargetName[vasynth.pot1_target_]);
			
			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			// bufferDrawStr(buffer1, 0, "TEST");

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_LOAD:
			bufferDrawStr(buffer0, 0, "ABCDEFGHIJKLMNOP");

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, preset_name[preset_number]);
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);
			break;

		case UI_MODE_SAVE:
			bufferDrawStr(buffer0, 0, "ABCDEFGHIJKLMNOP");
			
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
