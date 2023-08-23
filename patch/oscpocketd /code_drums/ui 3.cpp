// UI

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "ui.h"
#include "dbass.h"
#include "dsnare.h"
#include "dhihat.h"
#include "ddrum.h"
#include "dcymbal.h"
#include "dclap.h"


#include "dev/lcd_hd44780.h"

using namespace daisy;
using namespace daisysp;



// globals
extern DaisySeed hardware;
extern float g_sample_rate;
extern uint8_t g_input;
extern uint8_t g_pot0;
extern uint8_t g_play;
extern uint8_t seq_step;
extern uint8_t seq_sync;
extern float trig_gate_level;
extern uint8_t trig_pp16;

extern Settings settings;

extern DBass dbass;
extern DSnare dsnare;
extern DHihat dhihat;
extern DClap dclap;
extern DCymbal dride;
extern DCymbal dcrash;
extern DDrum dtomhi;
extern DDrum dtomlo;

extern Overdrive overdrive_bass;
extern Overdrive overdrive_snare;



void OscUI::Init(LcdHD44780 *aLcd)
{
	button = BUTTON_NONE;
	button_previous = BUTTON_NONE;
	button_previous_count = 0;
	button_previous_time = System::GetNow();
	button_mult = 1;

	lcd = aLcd;

	ui_mode = UI_MODE_OVERVIEW;

	ui_row = 0;
	ui_col = 0;
	ui_row_sub = 0;
	ui_col_sub = 0;
	ui_row_sub_offset = 0;
	ui_col_sub_offset = 0;
	
	ui_track = TRACK_BASS;
	ui_track_offset = 0;
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
		button_mult = 1;
	} else {
	
		//hardware.PrintLine("Work/button = %d", button);
	
		// handle button pressed down for a duration
		if ((System::GetNow() - button_previous_time) > 50)
		{
			button_previous_time = System::GetNow();

			if (button == button_previous)
			{
				button_previous_count++;
				if (button_previous_count > 10)
				{
					button_mult = button_mult * 10;
					button_previous_count = 0;
				}
			} else {
				button_previous_count = 0;
			}
			button_previous = button;

			// handle menus (states)
			
			switch (ui_mode)
			{
			
			case UI_MODE_OVERVIEW:
				if ((ui_row == 0) && (ui_col == 0)) {

					// OVERVIEW: PLAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (g_play == PLAY_OFF)
						{
							g_play = PLAY_ON;
							if (seq_sync == SYNC_INT)
							{
								tickIntStart();
							} else {
								tickExtStart();
							}
						} else {
							g_play = PLAY_TRIG;
							trigStart();
						}
					} else if (button == BUTTON_DOWN) {
						if (g_play == PLAY_TRIG)
						{
							g_play = PLAY_ON;
							if (seq_sync == SYNC_INT)
							{
								tickIntStart();
							} else {
								tickExtStart();
							}
						} else {
							g_play = PLAY_OFF;
						}
					} else if (button == BUTTON_LEFT) {
					} else if (button == BUTTON_RIGHT) {
						ui_col = 1;
					}
				} else if ((ui_row == 0) && (ui_col == 1)) {

					// OVERVIEW: TEMPO (BPM)
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (settings.seq_tempo + button_mult > 999) {
							settings.seq_tempo = 999;
						} else {
							settings.seq_tempo += button_mult;
						}
						tickIntCalc();
					} else if (button == BUTTON_DOWN) {
						if (settings.seq_tempo - button_mult < 1) {
							settings.seq_tempo = 1;
						} else {
							settings.seq_tempo -= button_mult;
						}
						tickIntCalc();
					} else if (button == BUTTON_LEFT) {
						ui_col = 0;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 4;
					}
				} else if ((ui_row == 0) && (ui_col == 4)) {

					// OVERVIEW: MIXER
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 0;
						ui_mode = UI_MODE_MIXER;
					} else if (button == BUTTON_LEFT) {
						ui_col = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 5;
					}
				} else if ((ui_row == 0) && (ui_col == 5)) {

					// OVERVIEW: UTILITY
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_UTILITY;
					} else if (button == BUTTON_LEFT) {
						ui_col = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 6;
					}

				} else if ((ui_row == 0) && (ui_col == 6)) {

					// OVERVIEW: SAVE
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 0;
						ui_mode = UI_MODE_SAVE;
					} else if (button == BUTTON_LEFT) {
						ui_col = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 7;
					}
					
				} else if ((ui_row == 0) && (ui_col == 7)) {

					// OVERVIEW: LOAD
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 0;
						ui_mode = UI_MODE_LOAD;
					} else if (button == BUTTON_LEFT) {
						ui_col = 6;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 8;
					}

				} else if ((ui_row == 0) && (ui_col == 8)) {
					
					// OVERVIEW: EDIT: BASS
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_BASS1;
					} else if (button == BUTTON_UP) {
						settings.dbass.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dbass.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 7;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 9;
					}
				} else if ((ui_row == 0) && (ui_col == 9)) {
					
					// OVERVIEW: EDIT: SNARE
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_SNARE1;
					} else if (button == BUTTON_UP) {
						settings.dsnare.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dsnare.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 8;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 10;
					}
				} else if ((ui_row == 0) && (ui_col == 10)) {
					
					// OVERVIEW: EDIT: HIHAT
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_HIHAT1;
					} else if (button == BUTTON_UP) {
						settings.dhihat.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dhihat.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 11;
					}
				} else if ((ui_row == 0) && (ui_col == 11)) {
					
					// OVERVIEW: EDIT: CRASH
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_CRASH1;
					} else if (button == BUTTON_UP) {
						settings.dcrash.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dcrash.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 10;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 12;
					}
				} else if ((ui_row == 0) && (ui_col == 12)) {
					
					// OVERVIEW: EDIT: RIDE
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_RIDE1;
					} else if (button == BUTTON_UP) {
						settings.dride.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dride.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 13;
					}
				} else if ((ui_row == 0) && (ui_col == 13)) {
					
					// OVERVIEW: EDIT: CLAP
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_CLAP1;
					} else if (button == BUTTON_UP) {
						settings.dclap.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dclap.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 12;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 14;
					}
				} else if ((ui_row == 0) && (ui_col == 14)) {
					
					// OVERVIEW: EDIT: TOMHI
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_TOMHI1;
					} else if (button == BUTTON_UP) {
						settings.dtomhi.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dtomhi.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 15;
					}
				} else if ((ui_row == 0) && (ui_col == 15)) {
					
					// OVERVIEW: EDIT: TOMLO
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 1;
						ui_mode = UI_MODE_EDIT_TOMLO1;
					} else if (button == BUTTON_UP) {
						settings.dtomhi.on = true;
					} else if (button == BUTTON_DOWN) {
						settings.dtomhi.on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col = 14;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 0;
						ui_row = 1;
					}
				} else if ((ui_row == 1) && (ui_col == 0)) {
					
					// OVERVIEW: SEQ: BASS
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_BASS;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 15;
						ui_row = 0;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 1;
					}
				} else if ((ui_row == 1) && (ui_col == 1)) {
					
					// OVERVIEW: SEQ: SNARE
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_SNARE;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 0;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 2;
					}
				} else if ((ui_row == 1) && (ui_col == 2)) {
					
					// OVERVIEW: SEQ: HIHAT
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_HIHAT;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 3;
					}
				} else if ((ui_row == 1) && (ui_col == 3)) {
					
					// OVERVIEW: SEQ: CRASH
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_CRASH;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 4;
					}
				} else if ((ui_row == 1) && (ui_col == 4)) {
					
					// OVERVIEW: SEQ: RIDE
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_RIDE;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 5;
					}
				} else if ((ui_row == 1) && (ui_col == 5)) {
					
					// OVERVIEW: SEQ: CLAP
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_CLAP;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 6;
					}
				} else if ((ui_row == 1) && (ui_col == 6)) {
					
					// OVERVIEW: SEQ: TOMHI
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_TOMHI;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 7;
					}
				} else if ((ui_row == 1) && (ui_col == 7)) {
					
					// OVERVIEW: SEQ: TOMLO
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 1;
						ui_col_sub = 0;
						ui_track = TRACK_TOMLO;
						ui_mode = UI_MODE_SEQ;
					} else if (button == BUTTON_LEFT) {
						ui_col = 6;
					} else if (button == BUTTON_RIGHT) {
						ui_col = 0;
						ui_row = 0;
					}

				}
				break;

			case UI_MODE_UTILITY:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// UTILITY: SEQ LEN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						uInc(&settings.seq_end, SEQ_MAX);
						seq_step = 0; // TODO
					} else if (button == BUTTON_DOWN) {
						uDec(&settings.seq_end, 1);
						seq_step = 0; // TODO
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 5;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// UTILITY: SYNC EXT/INT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						seq_sync = SYNC_EXT;
					} else if (button == BUTTON_DOWN) {
						seq_sync = SYNC_INT;
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 8;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 8)) {

					// UTILITY: GATE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						trig_gate_level += 0.1f;
						trig_gate_level = MIN(trig_gate_level, 0.9f);
					} else if (button == BUTTON_DOWN) {
						trig_gate_level -= 0.1f;
						trig_gate_level = MAX(trig_gate_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// UTILITY: PP16
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						uInc(&trig_pp16, 99);
					} else if (button == BUTTON_DOWN) {
						uDec(&trig_pp16, 1);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 8;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 15;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 15)) {

					// UTILITY: AUDIO IN (MERGE)
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						g_input = INPUT_MERGE;
					} else if (button == BUTTON_DOWN) {
						g_input = INPUT_NONE;
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 1;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 1)) {

					// UTILITY: GAIN IN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.gain_in, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.gain_in, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 15;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 6;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 6)) {

					// UTILITY: GAIN OUT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.gain_out, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.gain_out, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 12;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 12)) {

					// UTILITY: POT0 TARGET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						g_pot0 = POT_TEMPO;
					} else if (button == BUTTON_DOWN) {
						g_pot0 = POT_NONE;
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 6;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 15;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 15)) {

					// UTILITY: REVERB
					if (button == BUTTON_SELECT)
					{
						ui_row_sub = 0;
						ui_col_sub = 3;
						ui_mode = UI_MODE_REVERB;
					} else if (button == BUTTON_UP) {
						settings.reverb_on = true;
					} else if (button == BUTTON_DOWN) {
						settings.reverb_on = false;
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 6;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}
				break;

			case UI_MODE_REVERB:
				if ((ui_row_sub == 0) && (ui_col_sub == 3)) {

					// REVERB: LPF FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&settings.reverb_lpffreq, g_sample_rate / 2.0f);
						SetReverb();
					} else if (button == BUTTON_DOWN) {
						fDec(&settings.reverb_lpffreq, 0.0f);
						SetReverb();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// REVERB: FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.reverb_feedback, 0.99f);
						SetReverb();
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.reverb_feedback, 0.0f);
						SetReverb();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 3;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 3)) {

					// REVERB: DRY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.reverb_dry, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.reverb_dry, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 9)) {

					// REVERB: WET
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.reverb_wet, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.reverb_wet, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 0;
						ui_col_sub = 3;
					}
				}

				break;
				
			case UI_MODE_LOAD:
				if (ui_row_sub == 0) {

					// LOAD
					if (button == BUTTON_SELECT)
					{
						g_play = PLAY_OFF;
						FlashLoad(ui_col_sub);
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						if (ui_col_sub == 0)
						{
							ui_mode = UI_MODE_OVERVIEW;
						} else {
							ui_col_sub--;
						}
					} else if (button == BUTTON_RIGHT) {
						if (ui_col_sub == 15)
						{
							ui_row_sub = 1;
							ui_col_sub = 0;
						} else {
							ui_col_sub++;
						}
					}
				}
				break;

			case UI_MODE_SAVE:
				if (ui_row_sub == 0) {

					// SAVE
					if (button == BUTTON_SELECT)
					{
						g_play = PLAY_OFF;
						FlashSave(ui_col_sub);
					} else if (button == BUTTON_UP) {
					} else if (button == BUTTON_DOWN) {
					} else if (button == BUTTON_LEFT) {
						if (ui_col_sub == 0)
						{
							ui_mode = UI_MODE_OVERVIEW;
						} else {
							ui_col_sub--;
						}
					} else if (button == BUTTON_RIGHT) {
						if (ui_col_sub == 15)
						{
						} else {
							ui_col_sub++;
						}
					}
				}
				break;				

			case UI_MODE_MIXER:
				if (ui_row_sub == 0) {

					// MIXER: VOLUME
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
					// 0 2 4 6 8
					// 0 1 2 3 4
						MixerLevelInc(ui_col_sub / 2);
					} else if (button == BUTTON_DOWN) {
						MixerLevelDec(ui_col_sub / 2);
					} else if (button == BUTTON_LEFT) {
						if (ui_col_sub == 0)
						{
							ui_mode = UI_MODE_OVERVIEW;
						} else {
							ui_col_sub -= 2;
						}
					} else if (button == BUTTON_RIGHT) {
						if (ui_col_sub == 14)
						{
							ui_col_sub = 0;
							ui_row_sub = 1;
						} else {
							ui_col_sub += 2;
						}
					}

				} else if (ui_row_sub == 1) {

					// MIXER: PAN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						MixerPanInc(ui_col_sub / 2);
					} else if (button == BUTTON_DOWN) {
						MixerPanDec(ui_col_sub / 2);
					} else if (button == BUTTON_LEFT) {
						if (ui_col_sub == 0)
						{
							ui_col_sub = 14;
							ui_row_sub = 0;
						} else {
							ui_col_sub -= 2;
						}
					} else if (button == BUTTON_RIGHT) {
						if (ui_col_sub == 14)
						{
							ui_col_sub = 0;
							ui_row_sub = 0;
						} else {
							ui_col_sub += 2;
						}
					}

				}
				break;
			
			case UI_MODE_SEQ:
				// ui_row always 1
				if (button == BUTTON_SELECT)
				{
					// dont do this here as it is a global setting
					//seq_end = ui_col_sub + ui_track_offset;
					// seq_step = 
				} else if (button == BUTTON_UP) {
					if (settings.seq[ui_track][ui_track_offset + ui_col_sub] == DRUM_NORMAL)
					{
						settings.seq[ui_track][ui_track_offset + ui_col_sub] = DRUM_ACCENT;
					} else {
						settings.seq[ui_track][ui_track_offset + ui_col_sub] = DRUM_NORMAL;
					}
				} else if (button == BUTTON_DOWN) {
					settings.seq[ui_track][ui_track_offset + ui_col_sub] = DRUM_NONE;
				} else if (button == BUTTON_LEFT) {
					if (ui_col_sub > 0)
					{
						ui_col_sub--;
					} else if (ui_track_offset > 0)
					{
						ui_track_offset--;
					} else {
						ui_mode = UI_MODE_OVERVIEW;
					}
				} else if (button == BUTTON_RIGHT) {
					if (ui_col_sub < 15)
					{
						ui_col_sub++;
					} else if (ui_track_offset < (SEQ_MAX - 16))
					{
						ui_track_offset++;
					}
				}
				break;



			case UI_MODE_EDIT_BASS1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: BASS - TYPE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						uInc(&dbass.settings_.type, 3);
					} else if (button == BUTTON_DOWN) {
						uDec(&dbass.settings_.type, 1);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 4;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 4)) {

					// EDIT: BASS - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dbass.settings_.freq, 999.0f);
						dbass.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dbass.settings_.freq, 0.0f);
						dbass.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 9)) {

					// EDIT: BASS - TONE
					if (button == BUTTON_SELECT)
					{
						//ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.tone, 0.99f);
						dbass.SetTone();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.tone, 0.0f);
						dbass.SetTone();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: BASS - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.decay, 0.99f);
						dbass.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.decay, 0.0f);
						dbass.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 4;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 4)) {

					// EDIT: BASS - FM ATTACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.fm_attack, 0.99f);
						dbass.SetFMAttack();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.fm_attack, 0.0f);
						dbass.SetFMAttack();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 9)) {

					// EDIT: BASS - FM SELF
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.fm_self, 0.99f);
						dbass.SetFMAttack();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.fm_self, 0.0f);
						dbass.SetFMAttack();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 0;
						ui_col_sub = 3;
						ui_mode = UI_MODE_EDIT_BASS2;
					}
				}
				break;

			case UI_MODE_EDIT_BASS2:
				if ((ui_row_sub == 0) && (ui_col_sub == 3)) {

					// EDIT: BASS - DIRT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.dirtiness, 0.99f);
						dbass.SetDirtiness();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.dirtiness, 0.0f);
						dbass.SetDirtiness();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_BASS1;
						ui_row_sub = 1;
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 8;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 8)) {

					// EDIT: BASS - FM ENV
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.fm_env_amount, 0.99f);
						dbass.SetFMEnv();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.fm_env_amount, 0.0f);
						dbass.SetFMEnv();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: BASS - FM ENV DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.fm_env_decay, 0.99f);
						dbass.SetFMEnvDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.fm_env_decay, 0.0f);
						dbass.SetFMEnvDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 8;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 3;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 3)) {

					// EDIT: BASS - MIN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dbass.settings_.min, 0.99f);
						dbass.SetMin();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dbass.settings_.min, 0.0f);
						dbass.SetMin();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_BASS3;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}
				break;

			case UI_MODE_EDIT_BASS3:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: BASS - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dbass.delay_delay, 1.99f);
						SetDelayDelay(TRACK_BASS);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dbass.delay_delay, 0.0f);
						SetDelayDelay(TRACK_BASS);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_BASS2;
						ui_row_sub = 1;
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: BASS - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dbass.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dbass.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: BASS - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dbass.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dbass.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 7;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 7)) {

					// EDIT: BASS - OVERDRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dbass.overdrive, 0.99f);
						overdrive_bass.SetDrive(settings.dbass.overdrive);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dbass.overdrive, 0.0f);
						overdrive_bass.SetDrive(settings.dbass.overdrive);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: BASS - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dbass.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dbass.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 7;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_BASS1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}
				break;

			case UI_MODE_EDIT_SNARE1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: SNARE - TYPE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						// uInc(&dsnare.type_, 3);
					} else if (button == BUTTON_DOWN) {
						// uDec(&dsnare.type_, 1);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 4;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 4)) {

					// EDIT: SNARE - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dsnare.settings_.freq, 999.0f);
						dsnare.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dsnare.settings_.freq, 0.0f);
						dsnare.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 9)) {

					// EDIT: SNARE - TONE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.tone, 0.99f);
						dsnare.SetTone();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.tone, 0.0f);
						dsnare.SetTone();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: SNARE - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.decay, 9.99f);
						dsnare.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.decay, 0.0f);
						dsnare.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 4;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 4)) {

					// EDIT: SNARE - SNAPPY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.snappy, 0.99f);
						dsnare.SetSnappy();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.snappy, 0.0f);
						dsnare.SetSnappy();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 12;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 12)) {

					// EDIT: SNARE - FM SELF
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.fm_amount, 0.99f);
						dsnare.SetFM();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.fm_amount, 0.0f);
						dsnare.SetFM();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 0;
						ui_col_sub = 4;
						ui_mode = UI_MODE_EDIT_SNARE2;
					}
				}
				break;

			case UI_MODE_EDIT_SNARE2:
				if ((ui_row_sub == 0) && (ui_col_sub == 4)) {

					// EDIT: SNARE - FREQ NOISE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dsnare.settings_.freq_noise, 9999.0f);
						dsnare.SetFreqNoise();
					} else if (button == BUTTON_DOWN) {
						fDec(&dsnare.settings_.freq_noise, 0.0f);
						dsnare.SetFreqNoise();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_SNARE1;
						ui_row_sub = 1;
						ui_col_sub = 12;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 10;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 10)) {

					// EDIT: SNARE - RES
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.res, 0.99f);
						dsnare.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.res, 0.0f);
						dsnare.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 14;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 14)) {

					// EDIT: SNARE - AMP NOISE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.amp, 0.99f);
						dsnare.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.amp, 0.0f);
						dsnare.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 10;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 3;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 3)) {

					// EDIT: SNARE - MIN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.min, 0.99f);
						dsnare.SetMin();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.min, 0.0f);
						dsnare.SetMin();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 14;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 7;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 7)) {

					// EDIT: SNARE - DRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dsnare.settings_.drive, 0.99f);
						dsnare.SetDrive();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dsnare.settings_.drive, 0.0f);
						dsnare.SetDrive();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_SNARE3;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}
				break;

			case UI_MODE_EDIT_SNARE3:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: SNARE - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dsnare.delay_delay, 1.99f);
						SetDelayDelay(TRACK_SNARE);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dsnare.delay_delay, 0.0f);
						SetDelayDelay(TRACK_SNARE);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_SNARE2;
						ui_row_sub = 1;
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: SNARE - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dsnare.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dsnare.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: SNARE - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dsnare.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dsnare.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 7;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 7)) {

					// EDIT: SNARE - OVERDRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dsnare.overdrive, 0.99f);
						overdrive_snare.SetDrive(settings.dsnare.overdrive);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dsnare.overdrive, 0.0f);
						overdrive_snare.SetDrive(settings.dsnare.overdrive);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: SNARE - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dsnare.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dsnare.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 7;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_SNARE1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}
				break;

			case UI_MODE_EDIT_HIHAT1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: HIHAT - TYPE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						// uInc(&dhihat.type_, 3);
					} else if (button == BUTTON_DOWN) {
						// uDec(&dhihat.type_, 1);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 4;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 4)) {

					// EDIT: HIHAT - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dhihat.settings_.freq, 9999.0f);
						dhihat.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dhihat.settings_.freq, 0.0f);
						dhihat.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 9)) {

					// EDIT: HIHAT - TONE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.tone, 0.99f);
						dhihat.SetTone();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.tone, 0.0f);
						dhihat.SetTone();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: HIHAT - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.decay, 9.99f);
						dhihat.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.decay, 0.0f);
						dhihat.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: HIHAT - 2ND DECAY
					// no need to set it, as it is set by sequencer
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.decay_2nd, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.decay_2nd, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 0;
						ui_col_sub = 4;
						ui_mode = UI_MODE_EDIT_HIHAT2;
					}
				}
				break;

			case UI_MODE_EDIT_HIHAT2:
				if ((ui_row_sub == 0) && (ui_col_sub == 4)) {

					// EDIT: HIHAT - NOISINESS
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.noisiness, 0.99f);
						dhihat.SetNoisiness();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.noisiness, 0.0f);
						dhihat.SetNoisiness();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 13;
						ui_mode = UI_MODE_EDIT_HIHAT1;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 3;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 3)) {

					// EDIT: HIHAT - AMP NOISE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.amp, 0.99f);
						dhihat.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.amp, 0.0f);
						dhihat.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 4;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 7;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 7)) {

					// EDIT: HIHAT - DRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.drive, 0.99f);
						dhihat.SetDrive();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.drive, 0.0f);
						dhihat.SetDrive();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 3;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 11)) {

					// EDIT: HIHAT - RES
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dhihat.settings_.res, 0.99f);
						dhihat.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dhihat.settings_.res, 0.0f);
						dhihat.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 7;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_HIHAT3;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}
				break;

			case UI_MODE_EDIT_HIHAT3:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: HIHAT - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dhihat.delay_delay, 1.99f);
						SetDelayDelay(TRACK_SNARE);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dhihat.delay_delay, 0.0f);
						SetDelayDelay(TRACK_SNARE);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_HIHAT2;
						ui_row_sub = 1;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: HIHAT - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dhihat.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dhihat.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: HIHAT - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dhihat.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dhihat.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: HIHAT - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dhihat.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dhihat.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_HIHAT1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}
				break;
				
			case UI_MODE_EDIT_CRASH1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: CRASH - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dcrash.settings_.freq, 99999.0f);
						dcrash.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dcrash.settings_.freq, 0.0f);
						dcrash.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 7;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 7)) {

					// EDIT: CRASH - MIX
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dcrash.settings_.mix, 0.99f);
						dcrash.SetMix();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dcrash.settings_.mix, 0.0f);
						dcrash.SetMix();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: CRASH - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dcrash.settings_.decay, 9.99f);
						dcrash.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dcrash.settings_.decay, 0.0f);
						dcrash.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 7;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 1;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 1)) {

					// EDIT: CRASH - AMP NOISE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dcrash.settings_.amp, 0.99f);
						dcrash.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dcrash.settings_.amp, 0.0f);
						dcrash.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 5;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 5)) {

					// EDIT: CRASH - DRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dcrash.settings_.drive, 0.99f);
						dcrash.SetDrive();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dcrash.settings_.drive, 0.0f);
						dcrash.SetDrive();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 9)) {

					// EDIT: CRASH - RES
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dcrash.settings_.res, 0.99f);
						dcrash.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dcrash.settings_.res, 0.0f);
						dcrash.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: CRASH - MIN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dcrash.settings_.min, 0.99f);
						dcrash.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dcrash.settings_.min, 0.0f);
						dcrash.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_CRASH2;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}		
				break;

			case UI_MODE_EDIT_CRASH2:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: CRASH - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dcrash.delay_delay, 1.99f);
						SetDelayDelay(TRACK_CRASH);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dcrash.delay_delay, 0.0f);
						SetDelayDelay(TRACK_CRASH);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_CRASH1;
						ui_row_sub = 1;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: CRASH - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dcrash.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dcrash.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: CRASH - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dcrash.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dcrash.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: CRASH - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dcrash.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dcrash.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_CRASH1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}
				break;
				
			case UI_MODE_EDIT_RIDE1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: RIDE - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dride.settings_.freq, 99999.0f);
						dride.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dride.settings_.freq, 0.0f);
						dride.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 7;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 7)) {

					// EDIT: RIDE - MIX
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dride.settings_.mix, 0.99f);
						dride.SetMix();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dride.settings_.mix, 0.0f);
						dride.SetMix();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: RIDE - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dride.settings_.decay, 9.99f);
						dride.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dride.settings_.decay, 0.0f);
						dride.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 7;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 1;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 1)) {

					// EDIT: RIDE - AMP NOISE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dride.settings_.amp, 0.99f);
						dride.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dride.settings_.amp, 0.0f);
						dride.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 5;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 5)) {

					// EDIT: RIDE - DRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dride.settings_.drive, 0.99f);
						dride.SetDrive();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dride.settings_.drive, 0.0f);
						dride.SetDrive();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 9)) {

					// EDIT: RIDE - RES
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dride.settings_.res, 0.99f);
						dride.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dride.settings_.res, 0.0f);
						dride.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: RIDE - MIN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dride.settings_.min, 0.99f);
						dride.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dride.settings_.min, 0.0f);
						dride.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_RIDE2;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}		
				break;

			case UI_MODE_EDIT_RIDE2:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: RIDE - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dride.delay_delay, 1.99f);
						SetDelayDelay(TRACK_RIDE);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dride.delay_delay, 0.0f);
						SetDelayDelay(TRACK_RIDE);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_RIDE1;
						ui_row_sub = 1;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: RIDE - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dride.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dride.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: RIDE - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dride.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dride.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: RIDE - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dride.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dride.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_RIDE1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}		
				break;
				
			case UI_MODE_EDIT_CLAP1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: CLAP - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dclap.settings_.freq, 99999.0f);
						dclap.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dclap.settings_.freq, 0.0f);
						dclap.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: CLAP - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dclap.settings_.decay, 9.99f);
						dclap.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dclap.settings_.decay, 0.0f);
						dclap.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 1;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 1)) {

					// EDIT: CLAP - AMP NOISE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dclap.settings_.amp, 0.99f);
						dclap.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dclap.settings_.amp, 0.0f);
						dclap.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 5;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 5)) {

					// EDIT: CLAP - DRIVE
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dclap.settings_.drive, 0.99f);
						dclap.SetDrive();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dclap.settings_.drive, 0.0f);
						dclap.SetDrive();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 9;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 9)) {

					// EDIT: CLAP - RES
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dclap.settings_.res, 0.99f);
						dclap.SetRes();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dclap.settings_.res, 0.0f);
						dclap.SetRes();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_CLAP2;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}		
				break;

			case UI_MODE_EDIT_CLAP2:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: CLAP - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dclap.delay_delay, 1.99f);
						SetDelayDelay(TRACK_CLAP);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dclap.delay_delay, 0.0f);
						SetDelayDelay(TRACK_CLAP);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_CLAP1;
						ui_row_sub = 1;
						ui_col_sub = 9;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: CLAP - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dclap.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dclap.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: CLAP - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dclap.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dclap.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: CLAP - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dclap.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dclap.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_CLAP1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}				
				break;
				
			case UI_MODE_EDIT_TOMHI1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: TOMHI - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dtomhi.settings_.freq, 99999.0f);
						dtomhi.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dtomhi.settings_.freq, 0.0f);
						dtomhi.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: TOMHI - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dtomhi.settings_.decay, 9.99f);
						dtomhi.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dtomhi.settings_.decay, 0.0f);
						dtomhi.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 1;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 1)) {

					// EDIT: TOMHI - AMP
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dtomhi.settings_.amp, 0.99f);
						dtomhi.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dtomhi.settings_.amp, 0.0f);
						dtomhi.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 5;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 5)) {

					// EDIT: TOMHI - MIN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dtomhi.settings_.min, 0.99f);
						dtomhi.SetMin();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dtomhi.settings_.min, 0.0f);
						dtomhi.SetMin();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_TOMHI2;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}		
				break;

			case UI_MODE_EDIT_TOMHI2:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: TOMHI - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomhi.delay_delay, 1.99f);
						SetDelayDelay(TRACK_TOMHI);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomhi.delay_delay, 0.0f);
						SetDelayDelay(TRACK_TOMHI);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_TOMHI1;
						ui_row_sub = 1;
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: TOMHI - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomhi.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomhi.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: TOMHI - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomhi.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomhi.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: TOMHI - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomhi.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomhi.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_TOMHI1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}			
				break;
				
			case UI_MODE_EDIT_TOMLO1:
				if ((ui_row_sub == 0) && (ui_col_sub == 1)) {

					// EDIT: TOMLO - FREQ
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fInc(&dtomlo.settings_.freq, 99999.0f);
						dtomlo.SetFreq();
					} else if (button == BUTTON_DOWN) {
						fDec(&dtomlo.settings_.freq, 0.0f);
						dtomlo.SetFreq();
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 13)) {

					// EDIT: TOMLO - DECAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dtomlo.settings_.decay, 9.99f);
						dtomlo.SetDecay();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dtomlo.settings_.decay, 0.0f);
						dtomlo.SetDecay();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 1;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 1)) {

					// EDIT: TOMLO - AMP
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dtomlo.settings_.amp, 0.99f);
						dtomlo.SetAmp();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dtomlo.settings_.amp, 0.0f);
						dtomlo.SetAmp();
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 13;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 5;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 5)) {

					// EDIT: TOMLO - MIN
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&dtomlo.settings_.min, 0.99f);
						dtomlo.SetMin();
					} else if (button == BUTTON_DOWN) {
						fDecP(&dtomlo.settings_.min, 0.0f);
						dtomlo.SetMin();
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 1;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_TOMLO2;
						ui_row_sub = 0;
						ui_col_sub = 5;
					}
				}		
				break;

			case UI_MODE_EDIT_TOMLO2:
				if ((ui_row_sub == 0) && (ui_col_sub == 5)) {

					// EDIT: TOMLO - DELAY DELAY
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomlo.delay_delay, 1.99f);
						SetDelayDelay(TRACK_TOMLO);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomlo.delay_delay, 0.0f);
						SetDelayDelay(TRACK_TOMLO);
					} else if (button == BUTTON_LEFT) {
						ui_mode = UI_MODE_EDIT_TOMLO1;
						ui_row_sub = 1;
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 11;
					}
				} else if ((ui_row_sub == 0) && (ui_col_sub == 11)) {

					// EDIT: TOMLO - DELAY FEEDBACK
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomlo.delay_feedback, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomlo.delay_feedback, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 5;
					} else if (button == BUTTON_RIGHT) {
						ui_row_sub = 1;
						ui_col_sub = 2;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 2)) {

					// EDIT: TOMLO - REVERB
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomlo.reverb_level, 0.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomlo.reverb_level, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_row_sub = 0;
						ui_col_sub = 11;
					} else if (button == BUTTON_RIGHT) {
						ui_col_sub = 13;
					}
				} else if ((ui_row_sub == 1) && (ui_col_sub == 13)) {

					// EDIT: TOMLO - ACCENT
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						fIncP(&settings.dtomlo.accent, 9.99f);
					} else if (button == BUTTON_DOWN) {
						fDecP(&settings.dtomlo.accent, 0.0f);
					} else if (button == BUTTON_LEFT) {
						ui_col_sub = 2;
					} else if (button == BUTTON_RIGHT) {
						ui_mode = UI_MODE_EDIT_TOMLO1;
						ui_row_sub = 0;
						ui_col_sub = 1;
					}
				}			
				break;

			}

			ui_redraw = true;

		}
	}
}

void OscUI::uInc(uint8_t *value, uint8_t value_max)
{
	if (*value + 1 * button_mult > value_max)
		*value = value_max;
	else
		*value += 1 * button_mult;
}

void OscUI::uDec(uint8_t *value, uint8_t value_min)
{
	if (*value - 1 * button_mult < value_min)
		*value = value_min;
	else
		*value -= 1 * button_mult;
}

void OscUI::fIncP(float *value, float max)
{
	*value += 0.01f * button_mult;
	*value = MIN(*value, max);
}

void OscUI::fDecP(float *value, float min)
{
	*value -= 0.01f * button_mult;
	*value = MAX(*value, min);
}

void OscUI::fInc(float *value, float max)
{
	*value += 1.0f * button_mult;
	*value = MIN(*value, max);
}

void OscUI::fDec(float *value, float min)
{
	*value -= 1.0f * button_mult;
	*value = MAX(*value, min);
}


void OscUI::MixerLevelInc(uint8_t track)
{
	switch (track)
	{
	case TRACK_BASS:
		fIncP(&settings.dbass.level, 0.99f);
		break;
	case TRACK_SNARE:
		fIncP(&settings.dsnare.level, 0.99f);
		break;
	case TRACK_HIHAT:
		fIncP(&settings.dhihat.level, 0.99f);
		break;
	case TRACK_CRASH:
		fIncP(&settings.dcrash.level, 0.99f);
		break;
	case TRACK_RIDE:
		fIncP(&settings.dride.level, 0.99f);
		break;
	case TRACK_CLAP:
		fIncP(&settings.dclap.level, 0.99f);
		break;
	case TRACK_TOMHI:
		fIncP(&settings.dtomhi.level, 0.99f);
		break;
	case TRACK_TOMLO:
		fIncP(&settings.dtomlo.level, 0.99f);
		break;
	}
}



void OscUI::MixerLevelDec(uint8_t track)
{
	switch (track)
	{
	case TRACK_BASS:
		fDecP(&settings.dbass.level, 0.0f);
		break;
	case TRACK_SNARE:
		fDecP(&settings.dsnare.level, 0.0f);
		break;
	case TRACK_HIHAT:
		fDecP(&settings.dhihat.level, 0.0f);
		break;
	case TRACK_CRASH:
		fDecP(&settings.dcrash.level, 0.0f);
		break;
	case TRACK_RIDE:
		fDecP(&settings.dride.level, 0.0f);
		break;
	case TRACK_CLAP:
		fDecP(&settings.dclap.level, 0.0f);
		break;
	case TRACK_TOMHI:
		fDecP(&settings.dtomhi.level, 0.0f);
		break;
	case TRACK_TOMLO:
		fDecP(&settings.dtomlo.level, 0.0f);
		break;
	}
}



void OscUI::MixerPanInc(uint8_t track)
{
	switch (track)
	{
	case TRACK_BASS:
		fIncP(&settings.dbass.pan, 0.99f);
		break;
	case TRACK_SNARE:
		fIncP(&settings.dsnare.pan, 0.99f);
		break;
	case TRACK_HIHAT:
		fIncP(&settings.dhihat.pan, 0.99f);
		break;
	case TRACK_CRASH:
		fIncP(&settings.dcrash.pan, 0.99f);
		break;
	case TRACK_RIDE:
		fIncP(&settings.dride.pan, 0.99f);
		break;
	case TRACK_CLAP:
		fIncP(&settings.dclap.pan, 0.99f);
		break;
	case TRACK_TOMHI:
		fIncP(&settings.dtomhi.pan, 0.99f);
		break;
	case TRACK_TOMLO:
		fIncP(&settings.dtomlo.pan, 0.99f);
		break;
	}
}



void OscUI::MixerPanDec(uint8_t track)
{
	switch (track)
	{
	case TRACK_BASS:
		fDecP(&settings.dbass.pan, 0.0f);
		break;
	case TRACK_SNARE:
		fDecP(&settings.dsnare.pan, 0.0f);
		break;
	case TRACK_HIHAT:
		fDecP(&settings.dhihat.pan, 0.0f);
		break;
	case TRACK_CRASH:
		fDecP(&settings.dcrash.pan, 0.0f);
		break;
	case TRACK_RIDE:
		fDecP(&settings.dride.pan, 0.0f);
		break;
	case TRACK_CLAP:
		fDecP(&settings.dclap.pan, 0.0f);
		break;
	case TRACK_TOMHI:
		fDecP(&settings.dtomhi.pan, 0.0f);
		break;
	case TRACK_TOMLO:
		fDecP(&settings.dtomlo.pan, 0.0f);
		break;
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

	if (ui_redraw)
	{
		char buffer0[17] = "                "; // 16 blanks
		char buffer1[17] = "                "; // 16 blanks
		buffer0[16] = '\0'; // necessary?
		buffer1[16] = '\0'; // necessary?

		// clear is unneccessary since we write all characters every time; this produces better results on LCD
		//lcd->Clear();

		switch (ui_mode)
		{
		case UI_MODE_OVERVIEW:

			// row 0

			// play
			switch (g_play)
			{
			case PLAY_OFF:
				buffer0[0] = '-';
				break;
			case PLAY_ON:
				buffer0[0] = 'P';
				break;
			case PLAY_TRIG:
				buffer0[0] = 'T';
				break;
			}
			// tempo
			bufferDrawInt(buffer0, 1, 3, settings.seq_tempo);

			// mixer
			buffer0[4] = 'M';
			// utility
			buffer0[5] = 'U';
			// save/load
			buffer0[6] = 'S';
			buffer0[7] = 'L';

			// sound settings
			if (settings.dbass.on)
				buffer0[8] = 'K';
			else
				buffer0[8] = 'k';
			if (settings.dsnare.on)
				buffer0[9] = 'S';
			else
				buffer0[9] = 's';
			if (settings.dhihat.on)
				buffer0[10] = 'H';
			else
				buffer0[10] = 'h';
			if (settings.dcrash.on)
				buffer0[11] = 'C';
			else
				buffer0[11] = 'c';
			if (settings.dride.on)
				buffer0[12] = 'R';
			else
				buffer0[12] = 'r';
			if (settings.dclap.on)
				buffer0[13] = 'C';
			else
				buffer0[13] = 'c';
			if (settings.dtomhi.on)
				buffer0[14] = 'T';
			else
				buffer0[14] = 't';
			if (settings.dtomlo.on)
				buffer0[15] = 'L';
			else
				buffer0[15] = 'l';

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);
			
			// row 1

			// seq
			buffer1[0] = 'K';
			buffer1[1] = 'S';
			buffer1[2] = 'H';
			buffer1[3] = 'C';
			buffer1[4] = 'R';
			buffer1[5] = 'C';
			buffer1[6] = 'T';
			buffer1[7] = 'L';

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row, ui_col);
			
			break;

		case UI_MODE_MIXER:

			// row 0

			// vol
			bufferDrawInt(buffer0, 0, 1, (int)(settings.dbass.level * 100));
			bufferDrawInt(buffer0, 2, 3, (int)(settings.dsnare.level * 100));
			bufferDrawInt(buffer0, 4, 5, (int)(settings.dhihat.level * 100));
			bufferDrawInt(buffer0, 6, 7, (int)(settings.dcrash.level * 100));
			bufferDrawInt(buffer0, 8, 9, (int)(settings.dride.level * 100));
			bufferDrawInt(buffer0, 10, 11, (int)(settings.dclap.level * 100));
			bufferDrawInt(buffer0, 12, 13, (int)(settings.dtomhi.level * 100));
			bufferDrawInt(buffer0, 14, 15, (int)(settings.dtomlo.level * 100));
			
			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);
			
			// row 1

			// pan
			bufferDrawInt(buffer1, 0, 1, (int)(settings.dbass.pan * 100));
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dsnare.pan * 100));
			bufferDrawInt(buffer1, 4, 5, (int)(settings.dhihat.pan * 100));
			bufferDrawInt(buffer1, 6, 7, (int)(settings.dcrash.pan * 100));
			bufferDrawInt(buffer1, 8, 9, (int)(settings.dride.pan * 100));
			bufferDrawInt(buffer1, 10, 11, (int)(settings.dclap.pan * 100));
			bufferDrawInt(buffer1, 12, 13, (int)(settings.dtomhi.pan * 100));
			bufferDrawInt(buffer1, 14, 15, (int)(settings.dtomlo.pan * 100));
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			
			break;

		case UI_MODE_UTILITY:
		
			// row 1
			
			// seq max len
			buffer0[0] = 'L';
			bufferDrawInt(buffer0, 1, 3, settings.seq_end);
			
			// ext sync
			buffer0[4] = 'S';
			switch (seq_sync)
			{
			case SYNC_INT:
				buffer0[5] = 'I';
				break;
			case SYNC_EXT:
				buffer0[5] = 'E';
				break;
			}

			// trig gate level
			buffer0[7] = 'G';
			bufferDrawInt(buffer0, 8, 8, (int)(trig_gate_level * 10));

			// pp16 (trig signals per 1/16th note)
			buffer0[10] = 'P';
			bufferDrawInt(buffer0, 11, 12, trig_pp16);

			// input handling
			buffer0[14] = 'I';
			switch (g_input)
			{
			case INPUT_NONE:
				buffer0[15] = '-';
				break;
			case INPUT_MERGE:
				buffer0[15] = 'M';
				break;
			}
	
			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			// row 1
			
			// gain in and out
			buffer1[0] = 'I';
			bufferDrawInt(buffer1, 1, 3, (int)(settings.gain_in * 100));
			buffer1[5] = 'O';
			bufferDrawInt(buffer1, 6, 8, (int)(settings.gain_out * 100));

			// pot 0 function (tempo)
			buffer1[10] = 'P';
			buffer1[11] = '0';
			switch (g_pot0)
			{
			case POT_NONE:
				buffer1[12] = '-';
				break;
			case POT_TEMPO:
				buffer1[12] = 'T';
				break;
			}

			if (settings.reverb_on)
			{
				buffer1[15] = 'R';
			} else {
				buffer1[15] = 'r';
			}
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);

			break;

		case UI_MODE_REVERB:
			buffer0[0] = 'L';
			buffer0[1] = 'P';
			buffer0[2] = 'F';
			bufferDrawInt(buffer0, 3, 7, (int)settings.reverb_lpffreq);
			buffer0[9] = 'F';
			buffer0[10] = 'B';
			bufferDrawInt(buffer0, 11, 12, (int)(settings.reverb_feedback * 100));

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			buffer1[0] = 'D';
			buffer1[1] = 'R';
			buffer1[2] = 'Y';
			bufferDrawInt(buffer1, 3, 4, (int)(settings.reverb_dry * 100));
			buffer1[6] = 'W';
			buffer1[7] = 'E';
			buffer1[8] = 'T';
			bufferDrawInt(buffer1, 9, 10, (int)(settings.reverb_wet * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_LOAD:
			bufferDrawStr(buffer0, 0, "ABCDEFGHIJKLMNOP");

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_SAVE:
			bufferDrawStr(buffer0, 0, "ABCDEFGHIJKLMNOP");
			
			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_SEQ:
			// row 0
			switch (ui_track)
			{
			case TRACK_BASS:
				bufferDrawStr(buffer0, 0, "KICK");
				break;
			case TRACK_SNARE:
				bufferDrawStr(buffer0, 0, "SNARE");
				break;
			case TRACK_HIHAT:
				bufferDrawStr(buffer0, 0, "HIHAT");
				break;
			case TRACK_CRASH:
				bufferDrawStr(buffer0, 0, "CRASH");
				break;
			case TRACK_RIDE:
				bufferDrawStr(buffer0, 0, "RIDE");
				break;
			case TRACK_CLAP:
				bufferDrawStr(buffer0, 0, "CLAP");
				break;
			case TRACK_TOMHI:
				bufferDrawStr(buffer0, 0, "TOMHI");
				break;
			case TRACK_TOMLO:
				bufferDrawStr(buffer0, 0, "TOMLO");
				break;
			}
			bufferDrawInt(buffer0, 6, 7, ui_track_offset);
			bufferDrawInt(buffer0, 9, 10, settings.seq_end);

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);
		
			// row 1
			for (uint8_t i = 0; i < 16; i++)
			{
				switch (settings.seq[ui_track][ui_track_offset + i])
				{
				case DRUM_NORMAL:
					buffer1[i] = 'x';
					break;
				case DRUM_ACCENT:
					buffer1[i] = 'X';
				}
			}
			
			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(1, ui_col_sub);
			break;

		case UI_MODE_EDIT_BASS1:
			buffer0[0] = 'T';
			bufferDrawInt(buffer0, 1, 1, (int)dbass.settings_.type);
			buffer0[3] = 'F';
			bufferDrawInt(buffer0, 4, 6, (int)dbass.settings_.freq);
			buffer0[8] = 'T';
			bufferDrawInt(buffer0, 9, 10, (int)(dbass.settings_.tone * 100));
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dbass.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawStr(buffer1, 2, "FA");
			bufferDrawInt(buffer1, 4, 5, (int)(dbass.settings_.fm_attack * 100));
			bufferDrawStr(buffer1, 7, "FS");
			bufferDrawInt(buffer1, 9, 10, (int)(dbass.settings_.fm_self * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_BASS2:
			buffer0[0] = 'S';
			buffer0[2] = 'D';
			bufferDrawInt(buffer0, 3, 4, (int)(dbass.settings_.dirtiness * 100));
			bufferDrawStr(buffer0, 6, "FA");
			bufferDrawInt(buffer0, 8, 9, (int)(dbass.settings_.fm_env_amount * 100));
			bufferDrawStr(buffer0, 11, "FD");
			bufferDrawInt(buffer0, 13, 14, (int)(dbass.settings_.fm_env_decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'O';
			buffer1[2] = 'M';
			bufferDrawInt(buffer1, 3, 4, (int)(dbass.settings_.min * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_BASS3:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dbass.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dbass.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dbass.reverb_level * 100));
			bufferDrawStr(buffer1, 5, "OD");
			bufferDrawInt(buffer1, 7, 8, (int)(settings.dbass.overdrive * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dbass.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_SNARE1:
			buffer0[0] = 'T';
			bufferDrawInt(buffer0, 1, 1, (int)dsnare.settings_.type);
			buffer0[3] = 'F';
			bufferDrawInt(buffer0, 4, 6, (int)dsnare.settings_.freq);
			buffer0[8] = 'T';
			bufferDrawInt(buffer0, 9, 10, (int)(dsnare.settings_.tone * 100));
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dsnare.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawStr(buffer1, 2, "SN");
			bufferDrawInt(buffer1, 4, 5, (int)(dsnare.settings_.snappy * 100));
			buffer1[7] = 'S';
			bufferDrawStr(buffer1, 10, "FS");
			bufferDrawInt(buffer1, 12, 13, (int)(dsnare.settings_.fm_amount * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_SNARE2:
			buffer0[0] = 'O';
			bufferDrawStr(buffer0, 2, "NF");
			bufferDrawInt(buffer0, 4, 7, (int)dsnare.settings_.freq_noise);
			buffer0[9] = 'R';
			bufferDrawInt(buffer0, 10, 11, (int)(dsnare.settings_.res * 100));
			buffer0[13] = 'A';
			bufferDrawInt(buffer0, 14, 15, (int)(dsnare.settings_.amp * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[2] = 'M';
			bufferDrawInt(buffer1, 3, 4, (int)(dsnare.settings_.min * 100));
			buffer1[6] = 'D';
			bufferDrawInt(buffer1, 7, 8, (int)(dsnare.settings_.drive * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_SNARE3:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dsnare.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dsnare.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dsnare.reverb_level * 100));
			bufferDrawStr(buffer1, 5, "OD");
			bufferDrawInt(buffer1, 7, 8, (int)(settings.dsnare.overdrive * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dsnare.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;
			
		case UI_MODE_EDIT_HIHAT1:
			buffer0[0] = 'T';
			bufferDrawInt(buffer0, 1, 1, (int)dhihat.settings_.type);
			buffer0[3] = 'F';
			bufferDrawInt(buffer0, 4, 7, (int)dhihat.settings_.freq);
			buffer0[8] = 'T';
			bufferDrawInt(buffer0, 9, 10, (int)(dhihat.settings_.tone * 100));
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dhihat.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 11, "2D");
			bufferDrawInt(buffer1, 13, 14, (int)(dhihat.settings_.decay_2nd * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_HIHAT2:
			bufferDrawStr(buffer0, 0, "AS");
			buffer0[3] = 'N';
			bufferDrawInt(buffer0, 4, 5, (int)(dhihat.settings_.noisiness * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'O';
			buffer1[2] = 'A';
			bufferDrawInt(buffer1, 3, 4, (int)(dhihat.settings_.amp * 100));
			buffer1[6] = 'D';
			bufferDrawInt(buffer1, 7, 8, (int)(dhihat.settings_.drive * 100));
			buffer0[10] = 'R';
			bufferDrawInt(buffer1, 11, 12, (int)(dhihat.settings_.res * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_HIHAT3:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dhihat.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dhihat.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dhihat.reverb_level * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dhihat.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_CRASH1:
			buffer0[0] = 'F';
			bufferDrawInt(buffer0, 1, 4, (int)dcrash.settings_.freq);
			buffer0[6] = 'M';
			bufferDrawInt(buffer0, 7, 8, (int)(dcrash.settings_.mix * 100));
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 15, (int)(dcrash.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawInt(buffer1, 1, 2, (int)(dcrash.settings_.amp * 100));
			buffer1[4] = 'D';
			bufferDrawInt(buffer1, 5, 6, (int)(dcrash.settings_.drive * 100));
			buffer1[8] = 'R';
			bufferDrawInt(buffer1, 9, 10, (int)(dcrash.settings_.res * 100));
			buffer1[12] = 'M';
			bufferDrawInt(buffer1, 13, 14, (int)(dcrash.settings_.min * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_CRASH2:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dcrash.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dcrash.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dcrash.reverb_level * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dcrash.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_RIDE1:
			buffer0[0] = 'F';
			bufferDrawInt(buffer0, 1, 4, (int)dride.settings_.freq);
			buffer0[6] = 'M';
			bufferDrawInt(buffer0, 7, 8, (int)(dride.settings_.mix * 100));
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dride.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawInt(buffer1, 1, 2, (int)(dride.settings_.amp * 100));
			buffer1[4] = 'D';
			bufferDrawInt(buffer1, 5, 6, (int)(dride.settings_.drive * 100));
			buffer1[8] = 'R';
			bufferDrawInt(buffer1, 9, 10, (int)(dride.settings_.res * 100));
			buffer1[12] = 'M';
			bufferDrawInt(buffer1, 13, 14, (int)(dride.settings_.min * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_RIDE2:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dride.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dride.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dride.reverb_level * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dride.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_CLAP1:
			buffer0[0] = 'F';
			bufferDrawInt(buffer0, 1, 4, (int)dclap.settings_.freq);
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dclap.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawInt(buffer1, 1, 2, (int)(dclap.settings_.amp * 100));
			buffer1[4] = 'D';
			bufferDrawInt(buffer1, 5, 6, (int)(dclap.settings_.drive * 100));
			buffer1[8] = 'R';
			bufferDrawInt(buffer1, 9, 10, (int)(dclap.settings_.res * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_CLAP2:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dclap.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dclap.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dclap.reverb_level * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dclap.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_TOMHI1:
			buffer0[0] = 'F';
			bufferDrawInt(buffer0, 1, 4, (int)dtomhi.settings_.freq);
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dtomhi.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawInt(buffer1, 1, 2, (int)(dtomhi.settings_.amp * 100));
			buffer1[4] = 'M';
			bufferDrawInt(buffer1, 5, 6, (int)(dtomhi.settings_.min * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_TOMHI2:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dtomhi.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dtomhi.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dtomhi.reverb_level * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dtomhi.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_TOMLO1:
			buffer0[0] = 'F';
			bufferDrawInt(buffer0, 1, 4, (int)dtomlo.settings_.freq);
			buffer0[12] = 'D';
			bufferDrawInt(buffer0, 13, 14, (int)(dtomlo.settings_.decay * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			buffer1[0] = 'A';
			bufferDrawInt(buffer1, 1, 2, (int)(dtomlo.settings_.amp * 100));
			buffer1[4] = 'M';
			bufferDrawInt(buffer1, 5, 6, (int)(dtomlo.settings_.min * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;

		case UI_MODE_EDIT_TOMLO2:
			bufferDrawStr(buffer0, 0, "FX");
			bufferDrawStr(buffer0, 3, "DD");
			bufferDrawInt(buffer0, 5, 7, (int)(settings.dtomlo.delay_delay * 100));
			bufferDrawStr(buffer0, 9, "DF");
			bufferDrawInt(buffer0, 11, 12, (int)(settings.dtomlo.delay_feedback * 100));

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);

			bufferDrawStr(buffer1, 0, "RL");
			bufferDrawInt(buffer1, 2, 3, (int)(settings.dtomlo.reverb_level * 100));
			bufferDrawStr(buffer1, 11, "AC");
			bufferDrawInt(buffer1, 13, 15, (int)(settings.dtomlo.accent * 100));

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(ui_row_sub, ui_col_sub);
			break;
		}

		ui_redraw = false;
	}

}
