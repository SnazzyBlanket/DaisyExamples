// UI



#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "ui.h"

#include "dev/lcd_hd44780.h"

using namespace daisy;
using namespace daisysp;



#define MAX(x,y) ((x > y) ? (x) : (y))



// globals
extern DaisySeed hardware;
extern float sysSampleRate;
extern uint8_t sysLedMode;
extern bool uiRedraw;

extern bool sysPlay;
extern uint8_t sysPlayMode;
extern int sysTickBPM;
extern uint32_t sysTickCount;

// mixer

extern OpdMixSynth mixSynth[TRACK_NUMBER + DRUM_SOUNDS];
extern OpdMixDrum mixDrum;

// sequencer

extern uint8_t seqSongStep;
extern uint8_t seqSongLen;
extern bool seqVoiceOn[8];
extern uint8_t songStep[SONG_STEP_NUMBER][SEQ_VOICES];
extern uint8_t seqSongCurrent[SEQ_VOICES];

extern OpdMidiPitch seqBass[BASS_NUMBER][SEQ_NUMBER][BASS_NOTES];
extern uint32_t seqBassGateTime[BASS_NUMBER];
extern OpdNote seqLead[LEAD_NUMBER][SEQ_NUMBER][LEAD_NOTES];
extern uint8_t seqLeadLen[LEAD_NUMBER][SEQ_NUMBER];
extern uint8_t seqDrum[SEQ_NUMBER][DRUM_NOTES];

// synth sounds

extern OpdSynth synthSettings[SYNTH_NUMBER];

extern char synthOscWaveNames[WAVEFORMS_MAX][4];
extern Oscillator synthOsc[SYNTH_NUMBER];
extern Adsr synthAEnv[SYNTH_NUMBER];
extern Svf synthFilter[SYNTH_NUMBER];

extern Oscillator lfoOsc[SYNTH_NUMBER];
//extern OpdLfo lfo[SYNTH_NUMBER];

// drum sounds

extern OpdDrum drumSettings;

extern Oscillator kickOsc;
extern AdEnv kickAEnv;
extern AdEnv kickPEnv;

extern WhiteNoise snareNoise;
extern Svf snareNoiseLPF;
extern AdEnv snareNAEnv;
extern Oscillator snareOsc;
extern AdEnv snareOAEnv;
extern AdEnv snareOPEnv;

extern WhiteNoise hihatNoise;
extern Svf hihatNoiseHPF;
extern AdEnv hihatNoiseAEnv;

extern WhiteNoise crashNoise;
extern Svf crashNoiseHPF;
extern AdEnv crashNoiseAEnv;

extern WhiteNoise clapNoise;
extern Svf clapNoiseBPF;
extern AdEnv clapNoiseAEnv;

// FX
extern ReverbSc fxReverb;
extern OpdFXSettings fxSettings;



void OscUI::Init(LcdHD44780 *aLcd)
{
	button = BUTTON_NONE;
	buttonPrevious = BUTTON_NONE;
	buttonPreviousTime = System::GetNow();

	lcd = aLcd;

	uiMode = UI_MODE_OVERVIEW;

	uiMainRow = 0;
	uiMainCol = 0;
	uiSubRow = 0;
	uiSubCol = 0;
	uiSubColOffset = 0;
	uiSubRowOffset = 0;

	uiSynth = 0;
	uiSequence = 0;

	uiUtilDirection = UI_UTIL_DIR_UP;
	uiUtilValue = 0;
	uiUtilFromTrackType = UI_UTIL_TARGET_TRACK_B;
	uiUtilFromTrack = 0;
	uiUtilFromSeq = 0;
	uiUtilToTrackType = UI_UTIL_TARGET_TRACK_B;
	uiUtilToTrack = 0;
	uiUtilToSeq = 1;

}

// RIGHT	0
// UP		9
// DOWN		25
// LEFT 	39
// SELECT 	62
// else:	99

void OscUI::Button(float aButtonValue) {

	int aInButtonValue = (int)(aButtonValue*100);
	buttonPrevious = button;

	if (aInButtonValue < 5) {
		button = BUTTON_RIGHT;
	} else if (aInButtonValue < 15) {
		button = BUTTON_UP;
	} else if (aInButtonValue < 30) {
		button = BUTTON_DOWN;
	} else if (aInButtonValue < 50) {
		button = BUTTON_LEFT;
	} else if (aInButtonValue < 70) {
		button = BUTTON_SELECT;
	} else {
		button = BUTTON_NONE;

	}

}




void OscUI::Work()
{

	if ((button != BUTTON_NONE) && ((System::GetNow() - buttonPreviousTime) > 50)) {
		buttonPreviousTime = System::GetNow();

		switch (uiMode)
		{
		case UI_MODE_OVERVIEW:
			if ((uiMainRow == 0) && (uiMainCol == 0)) {

				// OVERVIEW: START/STOP
				if (button == BUTTON_SELECT)
				{
					if (sysPlay)
					{
						sysPlay = false;
						StopSeq();
					} else {

						if (sysPlayMode == PLAY_SEQ)
						{
							sysPlay = true;
							SetupSeq();
							PlaySeq();

							sysTickCount = 0;
						} else if (sysPlayMode == PLAY_SONG)
						{
							sysPlay = true;
							seqSongStep = 0;
							SetupSeq();
							PlaySeq();

							sysTickCount = 0;
						}
					}
				} else if (button == BUTTON_UP)
				{
					if (!sysPlay)
					{
						if (sysPlayMode == PLAY_OFF)
						{
							sysPlayMode = PLAY_SEQ;
						} else if (sysPlayMode == PLAY_SEQ) {
							sysPlayMode = PLAY_SONG;
						} else if (sysPlayMode == PLAY_SONG) {
							sysPlayMode = PLAY_OFF;
						} 
					}
				} else if (button == BUTTON_DOWN) {
					uiMainRow = 1;
				} else if (button == BUTTON_LEFT) {
					// jump to end of second line
					uiMainCol = 15;
					uiMainRow = 1;
				} else if (button == BUTTON_RIGHT) {
					uiMainCol = 1;
				}
			} else if ((uiMainRow == 0) && (uiMainCol == 1)) {

				// OVERVIEW: BPM
				if (button == BUTTON_UP)
				{
					if (sysTickBPM < 998)
					{
						sysTickBPM++;
						CalcTick();
					}
				} else if (button == BUTTON_DOWN) {
					if (sysTickBPM > 10)
					{
						sysTickBPM--;
						CalcTick();
					}
				} else if (button == BUTTON_LEFT) {
					uiMainCol = 0;
				} else if (button == BUTTON_RIGHT) {
					uiMainCol = 4;
				}
			} else if ((uiMainRow == 0) && (uiMainCol == 4)) {

				// OVERVIEW: SONG step
				if (button == BUTTON_SELECT)
				{
					// stop playing before editing song (problem when changing last step etc
					sysPlay = false;
					StopSeq();
					uiSubRow = 0;
					uiSubCol = 0;
					uiSubColOffset = 0;
					uiSubRowOffset = 0;
					uiMode = UI_MODE_SONGEDIT;
				} else if (button == BUTTON_UP)
				{
					if (seqSongStep < (seqSongLen - 1))
					{
						seqSongStep++;
					}
				} else if (button == BUTTON_DOWN) {
					if (seqSongStep > 0)
					{
						seqSongStep--;
					}
				} else if (button == BUTTON_LEFT) {
					uiMainCol = 1;
				} else if (button == BUTTON_RIGHT) {
					uiMainCol = 7;
				}
			} else if ((uiMainRow == 0) && (uiMainCol == 7)) {

				// OVERVIEW: UTILITY menu
				if (button == BUTTON_SELECT)
				{
					uiSubRow = 0;
					uiSubCol = 0;
					uiSubColOffset = 0;
					uiSubRowOffset = 0;
					uiMode = UI_MODE_UTILITY;
				} else if (button == BUTTON_DOWN) {
					uiMainRow = 1;
				} else if (button == BUTTON_LEFT) {
					uiMainCol = 4;
				} else if (button == BUTTON_RIGHT) {
					uiMainCol = 8;
				}
			} else if ((uiMainRow == 0) && (uiMainCol > 7)) {

				// OVERVIEW: MIXER
				if (button == BUTTON_SELECT)
				{
					uiSynth = uiMainCol - 8;
					uiSubRow = (uiSynth < 4 ? 0 : 1);
					uiSubCol = (uiSynth < 4 ? (uiSynth * 4) : ((uiSynth - 4) * 4));
					uiSubColOffset = 0;
					uiSubRowOffset = 0;
					sysLedMode = LED_MODE_CLIP;
					uiMode = UI_MODE_MIXER;
				} else if (button == BUTTON_LEFT) {
					uiMainCol--;
				} else if (button == BUTTON_RIGHT) {
					uiMainCol++;
					if (uiMainCol > 15)
					{
						// jump to next line
						uiMainCol = 0;
						uiMainRow = 1;
					}
				} else if (button == BUTTON_UP) {
					int aVoice = uiMainCol - 8;
					if (mixSynth[aVoice].volume < 99)
					{
						if (mixSynth[aVoice].volume > 89)
						{
							mixSynth[aVoice].volume = 99;
						} else {
							mixSynth[aVoice].volume += 10;
						}
						CalcMix();
					}
				} else if (button == BUTTON_DOWN) {
					int aVoice = uiMainCol - 8;
					if (mixSynth[aVoice].volume > 0)
					{
						if (mixSynth[aVoice].volume  < 10)
						{
							mixSynth[aVoice].volume = 0;
						} else {
							mixSynth[aVoice].volume -= 10;
						}
						CalcMix();
					}
				}

			} else if ((uiMainRow == 1) && (uiMainCol < 8)) {

				// OVERVIEW: NOTES

				if (button == BUTTON_SELECT)
				{
					// lead track: turn off sequencer, as we can change the length of the seq
					if (uiSynth == OFFSET_LEAD)
					{
						sysPlay = false;
						StopSeq();
					}
					uiSynth = uiMainCol;
					uiSequence = songStep[seqSongStep][uiSynth];
					uiSubRow = 0;
					uiSubCol = 0;
					uiSubColOffset = 0;
					uiSubRowOffset = 0;
					uiMode = UI_MODE_NOTE;
				} else if (button == BUTTON_UP) {
					// change current seq at next change
					if (songStep[seqSongStep][uiMainCol] < SEQ_NUMBER - 1)
					{
						songStep[seqSongStep][uiMainCol]++;
					}
				} else if (button == BUTTON_DOWN) {
					// change current seq at next change
					if (songStep[seqSongStep][uiMainCol] > 0)
					{
						songStep[seqSongStep][uiMainCol]--;
					}
				} else if (button == BUTTON_LEFT) {
					if (uiMainCol == 0)
					{
						uiMainCol = 15;
						uiMainRow = 0;
					} else {
						uiMainCol--;
					}
				} else if (button == BUTTON_RIGHT) {
					uiMainCol++;
				}
			} else if ((uiMainRow == 1) && (uiMainCol > 7)) {

				// OVERVIEW: SOUNDS

				if (button == BUTTON_SELECT)
				{
					uiSynth = uiMainCol - 8; // 0-7
					uiSubRow = 0;
					uiSubCol = 0;
					uiSubColOffset = 0;
					uiSubRowOffset = 0;
					uiMode = UI_MODE_SOUND;
				} else if (button == BUTTON_UP) {
					seqVoiceOn[uiMainCol - 8] = true;
				} else if (button == BUTTON_DOWN) {
					seqVoiceOn[uiMainCol - 8] = false;
				} else if (button == BUTTON_LEFT) {
					uiMainCol--;
				} else if (button == BUTTON_RIGHT) {
					if (uiMainCol == 15)
					{
						uiMainCol = 0;
						uiMainRow = 0;
					} else {
						uiMainCol++;
					}
				}
			}
			break; 		
		
		case UI_MODE_SONGEDIT:
			// 1234567812345678
			// St L         / Step = nn (0-999), Last - marker for last s
			// 1122334455667788

			if ((uiSubRow == 0) && (uiSubCol == 0)) {

				// OVERVIEW: SONGEDIT step
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP)
				{
					if (seqSongStep < SONG_STEP_NUMBER)
					{
						seqSongStep++;
					}
				} else if (button == BUTTON_DOWN) {
					if (seqSongStep > 0)
					{
						seqSongStep--;
					}
				} else if (button == BUTTON_LEFT) {
					seqSongStep = 0; // reset so current seq < last
					uiMode = UI_MODE_OVERVIEW;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 3;
				}
			} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

				// OVERVIEW: SONGEDIT last
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					seqSongLen = seqSongStep;
				} else if (button == BUTTON_DOWN) {
					seqSongLen = 0;
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 0;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 0;
					uiSubRow = 1;
				}
			} else if (uiSubRow == 1) {

				// OVERVIEW: SONGEDIT last
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (songStep[seqSongStep][uiSubCol / 2] < (SEQ_NUMBER - 1))
					{
						songStep[seqSongStep][uiSubCol / 2]++;
					}
				} else if (button == BUTTON_DOWN) {
					if (songStep[seqSongStep][uiSubCol / 2] > 0)
					{
						songStep[seqSongStep][uiSubCol / 2]--;
					}
				} else if (button == BUTTON_LEFT) {
					if ((uiSubCol) > 0)
					{
						uiSubCol -= 2;
					} else {
						uiSubCol = 3;
						uiSubRow = 0;
					}
				} else if (button == BUTTON_RIGHT) {
					if (uiSubCol < 14)
					{
						uiSubCol += 2;
					} else {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				}
			}
			break;



		case UI_MODE_UTILITY:
			// 1234567812345678
			// SvLdRvDySMg1g2g3
			// TrShCpGdnnffnttn

			if ((uiSubRow == 0) && (uiSubCol == 0)) {

				// UTILITY: Save
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					sysPlay = false;
					StopSeq();
					UtilFlashSave();
				} else if (button == BUTTON_DOWN) {
					uiSubRow = 1;
					uiSubCol = 0;
				} else if (button == BUTTON_LEFT) {
					uiMode = UI_MODE_OVERVIEW;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 2;
				}
			} else if ((uiSubRow == 0) && (uiSubCol == 2)) {

				// UTILITY: Load
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					sysPlay = false;
					StopSeq();
					UtilFlashLoad();
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 0;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 4;
				}
			} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

				// UTILITY: FX Reverb
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					uiSubRow = 0;
					uiSubCol = 0;
					uiMode = UI_MODE_FX_REVERB;
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 2;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 6;
				}

			} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

				// UTILITY: FX Delay
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					uiSubRow = 0;
					uiSubCol = 0;
					uiMode = UI_MODE_FX_DELAY;
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 4;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 9;
				}

			} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

				// UTILITY: MIDI export
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					UtilMIDIExport();
					uiMode = UI_MODE_OVERVIEW;
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 6;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 10;
				}

			} else if ((uiSubRow == 0) && (uiSubCol == 10)) {

				// UTILITY: bass gate time 0
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (seqBassGateTime[0] < 99)
					{
						seqBassGateTime[0]++;
					}
				} else if (button == BUTTON_DOWN) {
					if (seqBassGateTime[0] > 1)
					{
						seqBassGateTime[0]--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 6;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 12;
				}
			} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

				// UTILITY: bass gate time 1
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (seqBassGateTime[1] < 99)
					{
						seqBassGateTime[1]++;
					}
				} else if (button == BUTTON_DOWN) {
					if (seqBassGateTime[1] > 1)
					{
						seqBassGateTime[1]--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 10;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 14;
				}
			} else if ((uiSubRow == 0) && (uiSubCol == 14)) {

				// UTILITY: bass gate time 2
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (seqBassGateTime[2] < 99)
					{
						seqBassGateTime[2]++;
					}
				} else if (button == BUTTON_DOWN) {
					if (seqBassGateTime[2] > 1)
					{
						seqBassGateTime[2]--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 12;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 0;
					uiSubRow = 1;
				}

			} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

				// UTILITY: Transpose
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					UtilSeqTranspose(uiUtilDirection, uiUtilValue, uiUtilFromTrackType, uiUtilFromTrack, uiUtilFromSeq);
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 14;
					uiSubRow = 0;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 1;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 1)) {

				// UTILITY: Shift
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					UtilSeqShift(uiUtilDirection, uiUtilValue, uiUtilFromTrackType, uiUtilFromTrack, uiUtilFromSeq);
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 0;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 2;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 2)) {

				// UTILITY: Copy
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					sysPlay = false;
					StopSeq();
					UtilSeqCopy(uiUtilFromTrackType, uiUtilFromTrack, uiUtilFromSeq, uiUtilToTrackType, uiUtilToTrack, uiUtilToSeq);
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 1;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 3;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 3)) {

				// UTILITY: X - Clear
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					sysPlay = false;
					StopSeq();
					UtilSeqClear(uiUtilFromTrackType, uiUtilFromTrack, uiUtilFromSeq);
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 2;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 4;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 4)) {

				// UTILITY: Generate
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					sysPlay = false;
					StopSeq();
					UtilSeqGenerate(uiUtilDirection, uiUtilValue, uiUtilFromTrackType, uiUtilFromTrack, uiUtilFromSeq);
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 3;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 8;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 7)) {

				// UTILITY: direction
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					uiUtilDirection = UI_UTIL_DIR_UP;
				} else if (button == BUTTON_DOWN) {
					uiUtilDirection = UI_UTIL_DIR_DOWN;
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 4;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 8;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 8)) {

				// UTILITY: value
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilValue < 99)
					{
						uiUtilValue++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilValue > 1)
					{
						uiUtilValue--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 7;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 10;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 10)) {

				// UTILITY: from track type
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilFromTrackType < 3)
					{
						uiUtilFromTrackType++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilFromTrackType > 1)
					{
						uiUtilFromTrackType--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 8;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 11;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 11)) {

				// UTILITY: from track
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilFromTrack < MAX(BASS_NUMBER, LEAD_NUMBER)) // assume CHORD_NUMBER is less
					{
						uiUtilFromTrack++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilFromTrack > 0)
					{
						uiUtilFromTrack--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 10;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 12;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 12)) {

				// UTILITY: from seq
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilFromSeq < (SEQ_NUMBER - 1))
					{
						uiUtilFromSeq++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilFromSeq > 0)
					{
						uiUtilFromSeq--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 11;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 13;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 13)) {

				// UTILITY: to track type
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilToTrackType < 3)
					{
						uiUtilToTrackType++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilToTrackType > 1)
					{
						uiUtilToTrackType--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 12;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 14;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 14)) {

				// UTILITY: to track
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilToTrack < MAX(BASS_NUMBER, LEAD_NUMBER))
					{
						uiUtilToTrack++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilToTrack > 0)
					{
						uiUtilToTrack--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 13;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 15;
				}
			} else if ((uiSubRow == 1) && (uiSubCol == 15)) {

				// UTILITY: to seq
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (uiUtilToSeq < (SEQ_NUMBER - 1))
					{
						uiUtilToSeq++;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiUtilToSeq > 0)
					{
						uiUtilToSeq--;
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 14;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 0;
					uiSubRow = 0;
				}
			}
			break;


		case UI_MODE_FX_REVERB:
			// 1234567812345678
			// FFrq

			if ((uiSubRow == 0) && (uiSubCol == 0)) {

				// FX: reverb feedback
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (fxSettings.reverbFeedback < 0.9)
					{
						fxSettings.reverbFeedback += 0.1;
						fxReverb.SetFeedback(fxSettings.reverbFeedback);
					}
				} else if (button == BUTTON_DOWN) {
					if (fxSettings.reverbFeedback > 0)
					{
						fxSettings.reverbFeedback -= 0.1;
						fxReverb.SetFeedback(fxSettings.reverbFeedback);
					}
				} else if (button == BUTTON_LEFT) {
					uiMode = UI_MODE_OVERVIEW;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 1;
				}
			} else if ((uiSubRow == 0) && (uiSubCol == 1)) {

				// FX: reverb frequency (/100)
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (fxSettings.reverbLpfFrequency < (sysSampleRate / 2))
					{
						fxSettings.reverbLpfFrequency += 100;
						fxReverb.SetLpFreq(fxSettings.reverbLpfFrequency);
					}
				} else if (button == BUTTON_DOWN) {
					if (fxSettings.reverbLpfFrequency >= 100)
					{
						fxSettings.reverbLpfFrequency -= 100;
						fxReverb.SetLpFreq(fxSettings.reverbLpfFrequency);
					}
				} else if (button == BUTTON_LEFT) {
					uiSubCol = 0;
				} else if (button == BUTTON_RIGHT) {
					uiSubCol = 0;
				}

			}
			break;

		case UI_MODE_FX_DELAY:

			uiSynth = (uiSubCol / 4) + ((uiSubRow == 0) ? 0 : 4);

			if (button == BUTTON_SELECT)
			{
			} else if (button == BUTTON_UP) {
				if (uiSubCol % 4 == 0) {
					if (synthSettings[uiSynth].delayDelay < DELAY_MAX_S)
					{
						synthSettings[uiSynth].delayDelay += 0.1;
						FXSetDelay(uiSynth);
					}
				} else if (uiSubCol % 4 == 2) {
					if (synthSettings[uiSynth].delayFeedback < 0.99)
					{
						synthSettings[uiSynth].delayFeedback += 0.01;
					}
				}
			} else if (button == BUTTON_DOWN) {
				if (uiSubCol % 4 == 0) {
					if (synthSettings[uiSynth].delayDelay >= 0.1)
					{
						synthSettings[uiSynth].delayDelay -= 0.1;
						FXSetDelay(uiSynth);
					}
				} else if (uiSubCol % 4 == 2) {
					if (synthSettings[uiSynth].delayFeedback >= 0.01)
					{
						synthSettings[uiSynth].delayFeedback -= 0.01;
						if (synthSettings[uiSynth].delayFeedback < 0.01)
						{
							synthSettings[uiSynth].delayFeedback = 0;
						}
					}
				}
			} else if (button == BUTTON_LEFT) {
				if (uiSubCol == 0)
				{
					if (uiSubRow == 0)
					{
						uiMode = UI_MODE_OVERVIEW;
					} else {
						uiSubCol = 14;
						uiSubRow = 0;
					}
				} else {
					uiSubCol -= 2;
				}
			} else if (button == BUTTON_RIGHT) {
				if ((uiSubCol == 14) && (uiSubRow == 0))
				{
					uiSubCol = 0;
					uiSubRow = 1;
				} else if ((uiSubCol == 10) && (uiSubRow == 1)) {
					uiSubCol = 0;
					uiSubRow = 0;
				} else {
					uiSubCol += 2;
				}

			}
			break;

		case UI_MODE_MIXER:

			if (uiSynth < 7)
			{
				// synth mixer

				uint8_t theSynth = (uiSubCol / 4) + ((uiSubRow == 0) ? 0 : 4);

				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP)
				{
					if (uiSubCol % 4 == 0) {
						// level (2)
						if (mixSynth[theSynth].volume < 99)
						{
							mixSynth[theSynth].volume += 1;
							CalcMix();
						}
					} else if (uiSubCol % 4 == 2) {
						// pan (1)
						if (mixSynth[theSynth].pan < 9)
						{
							mixSynth[theSynth].pan += 1;
							CalcMix();
						}
					} else if (uiSubCol % 4 == 3) {
						// reverb (1)
						if (mixSynth[theSynth].reverb < 9)
						{
							mixSynth[theSynth].reverb += 1;
							CalcMix();
						}
					}
				} else if (button == BUTTON_DOWN) {
					if (uiSubCol % 4 == 0) {
						// level (2)
						if (mixSynth[theSynth].volume > 0)
						{
							mixSynth[theSynth].volume -= 1;
							CalcMix();
						}
					} else if (uiSubCol % 4 == 2) {
						// pan (1)
						if (mixSynth[theSynth].pan > 0)
						{
							mixSynth[theSynth].pan -= 1;
							CalcMix();
						}
					} else if (uiSubCol % 4 == 3) {
						// reverb (1)
						if (mixSynth[theSynth].reverb > 0)
						{
							mixSynth[theSynth].reverb -= 1;
							CalcMix();
						}
					}
				} else if (button == BUTTON_LEFT) {
					if (uiSubCol == 0)
					{
						if (uiSubRow == 0)
						{
							uiMode = UI_MODE_OVERVIEW;
							sysLedMode = LED_MODE_TEMPO;
						} else if (uiSubRow == 1) {
							uiSubCol = 15;
							uiSubRow = 0;
						}
					} else if (uiSubCol == 4 || uiSubCol == 8 || uiSubCol == 12) {
						uiSubCol -= 1;
					} else if (uiSubCol == 3 || uiSubCol == 7 || uiSubCol == 11 || uiSubCol == 15) {
						uiSubCol -= 1;
					} else if (uiSubCol == 2 || uiSubCol == 6 || uiSubCol == 10 || uiSubCol == 14) {
						uiSubCol -= 2;
					}

				} else if (button == BUTTON_RIGHT) {
					if (uiSubRow == 0)
					{
						if (uiSubCol == 0 || uiSubCol == 4 || uiSubCol == 8 || uiSubCol == 12)
						{
							uiSubCol += 2; // move to pan
						} else if (uiSubCol == 2 || uiSubCol == 6 || uiSubCol == 10 || uiSubCol == 14) {
							uiSubCol += 1; // move to rev/fx;
						} else if (uiSubCol == 3 || uiSubCol == 7 || uiSubCol == 11) {
							uiSubCol += 1; // move to next voice
						} else if (uiSubCol == 15) {
							uiSubCol = 0;
							uiSubRow = 1;
						}
					} else if (uiSubRow == 1) {
						if (uiSubCol == 0 || uiSubCol == 4 || uiSubCol == 8)
						{
							uiSubCol += 2; // move to pan
						} else if (uiSubCol == 2 || uiSubCol == 6 || uiSubCol == 10) {
							uiSubCol += 1; // move to rev/fx;
						} else if (uiSubCol == 3 || uiSubCol == 7) {
							uiSubCol += 1; // move to next voice
						} else if (uiSubCol == 11) {
							uiSubCol = 0;
							uiSubRow = 0;
						}
					}
				}

			} else {
				// drum mixer

				// kick vol (2), kick pan(1), kick rev (1), snare vol, snare pan, snare rev

				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// MIXER: kick vol
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.kickVolume < 99)
						{
							mixDrum.kickVolume += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.kickVolume > 0)
						{
							mixDrum.kickVolume -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
						sysLedMode = LED_MODE_TEMPO;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 2;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 2)) { 

					// MIXER: kick pan
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.kickPan < 9)
						{
							mixDrum.kickPan += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.kickPan > 0)
						{
							mixDrum.kickPan -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) { 

					// MIXER: kick reverb
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixSynth[OFFSET_DRUM + 0].reverb < 9)
						{
							mixSynth[OFFSET_DRUM + 0].reverb += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (mixSynth[OFFSET_DRUM + 0].reverb > 0)
						{
							mixSynth[OFFSET_DRUM + 0].reverb -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 4;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 4)) {

					// MIXER: snare vol
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.snareVolume < 99)
						{
							mixDrum.snareVolume += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.snareVolume > 0)
						{
							mixDrum.snareVolume -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) { 

					// MIXER: snare pan
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.snarePan < 9)
						{
							mixDrum.snarePan += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.snarePan > 0)
						{
							mixDrum.snarePan -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 4;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 7;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 7)) { 

					// MIXER: snare reverb
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixSynth[OFFSET_DRUM + 1].reverb < 9)
						{
							mixSynth[OFFSET_DRUM + 1].reverb += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (mixSynth[OFFSET_DRUM + 1].reverb > 0)
						{
							mixSynth[OFFSET_DRUM + 1].reverb -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 8;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 8)) {

					// MIXER: hihat vol
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.hihatVolume < 99)
						{
							mixDrum.hihatVolume += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.hihatVolume > 0)
						{
							mixDrum.hihatVolume -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 7;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 10;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 10)) { 

					// MIXER: hihat pan
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.hihatPan < 9)
						{
							mixDrum.hihatPan += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.hihatPan > 0)
						{
							mixDrum.hihatPan -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 8;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 11;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 11)) { 

					// MIXER: hihat reverb
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixSynth[OFFSET_DRUM + 2].reverb < 9)
						{
							mixSynth[OFFSET_DRUM + 2].reverb += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (mixSynth[OFFSET_DRUM + 2].reverb > 0)
						{
							mixSynth[OFFSET_DRUM + 2].reverb -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 10;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// MIXER: crash vol
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.crashVolume < 99)
						{
							mixDrum.crashVolume += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.crashVolume > 0)
						{
							mixDrum.crashVolume -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 11;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 14;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 14)) { 

					// MIXER: crash pan
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.crashPan < 9)
						{
							mixDrum.crashPan += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.crashPan > 0)
						{
							mixDrum.crashPan -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 12;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 15;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 15)) { 

					// MIXER: crash reverb
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixSynth[OFFSET_DRUM + 4].reverb < 9)
						{
							mixSynth[OFFSET_DRUM + 4].reverb += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (mixSynth[OFFSET_DRUM + 4].reverb > 0)
						{
							mixSynth[OFFSET_DRUM + 4].reverb -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 14;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// MIXER: clap vol
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.clapVolume < 99)
						{
							mixDrum.clapVolume += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.clapVolume > 0)
						{
							mixDrum.clapVolume -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 15;
						uiSubRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 2;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 2)) { 

					// MIXER: clap pan
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixDrum.clapPan < 9)
						{
							mixDrum.clapPan += 1;
							CalcMix();
						}
					} else if (button == BUTTON_DOWN) {
						if (mixDrum.clapPan > 0)
						{
							mixDrum.clapPan -= 1;
							CalcMix();
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 3)) { 

					// MIXER: clap reverb
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (mixSynth[OFFSET_DRUM + 3].reverb < 9)
						{
							mixSynth[OFFSET_DRUM + 3].reverb += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (mixSynth[OFFSET_DRUM + 3].reverb > 0)
						{
							mixSynth[OFFSET_DRUM + 3].reverb -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 2;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}
				}
			}
			break;



		case UI_MODE_NOTE:
			
			switch (uiSynth)
			{
			case 0:
			case 1:
			case 2:
				// bass
				if (button == BUTTON_SELECT)
				{
				} else if (button == BUTTON_UP) {
					if (seqBass[uiSynth][uiSequence][uiSubCol + uiSubColOffset] < 98)
					{
						seqBass[uiSynth][uiSequence][uiSubCol + uiSubColOffset]++;
					}
				} else if (button == BUTTON_DOWN) {
					if (seqBass[uiSynth][uiSequence][uiSubCol + uiSubColOffset] > 0)
					{
						seqBass[uiSynth][uiSequence][uiSubCol + uiSubColOffset]--;
					}
				} else if (button == BUTTON_LEFT) {
					if (uiSubCol == 0)
					{
						if (uiSubColOffset == 16)
						{
							uiSubColOffset = 0;
							uiSubCol = 15;
						} else if (uiSubColOffset == 0)
						{
							uiMode = UI_MODE_OVERVIEW;
						}
					} else {
						uiSubCol--;
					}
				} else if (button == BUTTON_RIGHT) {
					if (uiSubCol == 15)
					{
						if (uiSubColOffset == 0)
						{
							uiSubColOffset = 16;
							uiSubCol = 0;
						}
					} else {
						uiSubCol++;
					}
				}


				break;

			case 3:
			case 4:
			case 5:
			case 6:
				// lead
				{
					uint8_t theVoice = uiSynth - OFFSET_LEAD;
					uint8_t theNote = (uiSubCol + uiSubColOffset) / 2;

					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if ((uiSubCol % 2) == 0)
						{
							// notePitch
							if (seqLead[theVoice][uiSequence][theNote].notePitch < 98)
							{
								seqLead[theVoice][uiSequence][theNote].notePitch++;
							}
						} else {
							// noteTime
							if (seqLead[theVoice][uiSequence][theNote].noteTime < 98)
							{
								if (seqLead[theVoice][uiSequence][theNote].noteTime == 0)
								{
									seqLeadLen[theVoice][uiSequence] = theNote + 1;
								}
								seqLead[theVoice][uiSequence][theNote].noteTime++;
							}
						}
					} else if (button == BUTTON_DOWN) {
						if ((uiSubCol % 2) == 0)
						{
							if (seqLead[theVoice][uiSequence][theNote].notePitch > 0)
							{
								seqLead[theVoice][uiSequence][theNote].notePitch--;
							}
						} else {
							// noteTime
							if (seqLead[theVoice][uiSequence][theNote].noteTime > 0)
							{
								seqLead[theVoice][uiSequence][theNote].noteTime--;
								// set noteTime == 0 marks end of sequence/variation
								if (seqLead[theVoice][uiSequence][theNote].noteTime == 0)
								{
									seqLeadLen[theVoice][uiSequence] = theNote + 1;
								}
							}
						}
					} else if (button == BUTTON_LEFT) {
						if (uiSubCol == 0)
						{
							if (uiSubColOffset > 0)
							{
								uiSubColOffset -= 16;
								uiSubCol = 15;
							} else if (uiSubColOffset == 0)
							{
								uiMode = UI_MODE_OVERVIEW;
							}
						} else {
							uiSubCol--;
						}
					} else if (button == BUTTON_RIGHT) {
						if (uiSubCol == 15)
						{
							if ((uiSubColOffset / 2) < (LEAD_NOTES - 8))
							{
								uiSubColOffset += 16;
								uiSubCol = 0;
							}
						} else {
							uiSubCol++;
						}
					}
				}
				break;

			case 7:
				// drums

				if (button == BUTTON_SELECT)
				{
					if (uiSubRow + uiSubRowOffset == 0)
					{
						// kick
						if (seqDrum[uiSequence][uiSubCol + uiSubColOffset] & DRUM_KICK)
						{
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] - DRUM_KICK;
						} else {
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] + DRUM_KICK;
						}	
					} else if (uiSubRow + uiSubRowOffset == 1) {
						// snare
						if (seqDrum[uiSequence][uiSubCol + uiSubColOffset] & DRUM_SNARE)
						{
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] - DRUM_SNARE;
						} else {
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] + DRUM_SNARE;
						}
					} else if (uiSubRow + uiSubRowOffset == 2) {
						// hihat open, hihat closed
						if (seqDrum[uiSequence][uiSubCol + uiSubColOffset] & DRUM_HHCLOSED)
						{
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] - DRUM_HHCLOSED;
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] + DRUM_HHOPEN;
						} else if (seqDrum[uiSequence][uiSubCol + uiSubColOffset] & DRUM_HHOPEN) {
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] - DRUM_HHOPEN;
						} else {
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] + DRUM_HHCLOSED;
						}
					} else if (uiSubRow + uiSubRowOffset == 3) {
						// crash
						if (seqDrum[uiSequence][uiSubCol + uiSubColOffset] & DRUM_CRASH)
						{
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] - DRUM_CRASH;
						} else {
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] + DRUM_CRASH;
						}
					} else if (uiSubRow + uiSubRowOffset == 4) {
						// clap
						if (seqDrum[uiSequence][uiSubCol + uiSubColOffset] & DRUM_CLAP)
						{
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] - DRUM_CLAP;
						} else {
							seqDrum[uiSequence][uiSubCol + uiSubColOffset] = seqDrum[uiSequence][uiSubCol + uiSubColOffset] + DRUM_CLAP;
						}
					}
				} else if (button == BUTTON_UP) {
					if (uiSubRow == 0)
					{
						if (uiSubRowOffset == 0)
						{
						} else {
							uiSubRowOffset--;
						}
					} else {
						uiSubRow--;
					}
				} else if (button == BUTTON_DOWN) {
					if (uiSubRow == 1)
					{
						if (uiSubRowOffset < 3)
						{
							uiSubRowOffset += 1;
						} else {
							// do nothing
						}
					} else {
						uiSubRow++;
					}
				} else if (button == BUTTON_LEFT) {
					if (uiSubCol == 0)
					{
						if (uiSubColOffset == 16)
						{
							uiSubColOffset = 0;
							uiSubCol = 15;
						} else if (uiSubColOffset == 0)
						{
							uiMode = UI_MODE_OVERVIEW;
						}
					} else {
						uiSubCol--;
					}
				} else if (button == BUTTON_RIGHT) {
					if (uiSubCol == 15)
					{
						if (uiSubColOffset == 0)
						{
							uiSubColOffset = 16;
							uiSubCol = 0;
						}
					} else {
						uiSubCol++;
					}
				}

				break;

			}

			break;



		case UI_MODE_SOUND:
			// current sound is in uiSynth

			if (uiSynth < 7)
			{
				// synths

				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// SOUND: WAVEFORM
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].oscWaveform < 7)
						{
							synthSettings[uiSynth].oscWaveform++;
							synthOsc[uiSynth].SetWaveform(synthSettings[uiSynth].oscWaveform);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].oscWaveform > 0)
						{
							synthSettings[uiSynth].oscWaveform--;
							synthOsc[uiSynth].SetWaveform(synthSettings[uiSynth].oscWaveform);
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// SOUND: detune
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].oscDetune < 999)
						{
							synthSettings[uiSynth].oscDetune += 1;
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].oscDetune > 0)
						{
							synthSettings[uiSynth].oscDetune -= 1;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// SOUND: cutoff
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].filterCutoff < (sysSampleRate / 2))
						{
							synthSettings[uiSynth].filterCutoff += 100;
							synthFilter[uiSynth].SetFreq(synthSettings[uiSynth].filterCutoff);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].filterCutoff >= 100)
						{
							synthSettings[uiSynth].filterCutoff -= 100;
							synthFilter[uiSynth].SetFreq(synthSettings[uiSynth].filterCutoff);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// SOUND: res; level 0-1
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].filterRes < 1)
						{
							synthSettings[uiSynth].filterRes += 0.05;
							synthFilter[uiSynth].SetRes(synthSettings[uiSynth].filterRes);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].filterRes > 0)
						{
							synthSettings[uiSynth].filterRes -= 0.05;
							synthFilter[uiSynth].SetRes(synthSettings[uiSynth].filterRes);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 13;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 13)) {

					// SOUND: LFO freq
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].lfoFrequency < 99.9)
						{
							synthSettings[uiSynth].lfoFrequency += 0.1;
							lfoOsc[uiSynth].SetFreq(synthSettings[uiSynth].lfoFrequency);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].lfoFrequency > 0)
						{
							synthSettings[uiSynth].lfoFrequency -= 0.1;
							lfoOsc[uiSynth].SetFreq(synthSettings[uiSynth].lfoFrequency);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// SOUND: Adsr
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].adsrAttack < 9.0)
						{
							synthSettings[uiSynth].adsrAttack += 0.05;
							synthAEnv[uiSynth].SetTime(ADSR_SEG_ATTACK, MAX(0.01, synthSettings[uiSynth].adsrAttack));
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].adsrAttack >= 0.05)
						{
							synthSettings[uiSynth].adsrAttack -= 0.05;
							synthAEnv[uiSynth].SetTime(ADSR_SEG_ATTACK, MAX(0.01, synthSettings[uiSynth].adsrAttack));
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 13;
						uiSubRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 3)) {

					// SOUND: aDsr
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].adsrDecay < 9.0)
						{
							synthSettings[uiSynth].adsrDecay += 0.05;
							synthAEnv[uiSynth].SetTime(ADSR_SEG_DECAY, MAX(0.01, synthSettings[uiSynth].adsrDecay));
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].adsrDecay > 0.0)
						{
							synthSettings[uiSynth].adsrDecay -= 0.05;
							synthAEnv[uiSynth].SetTime(ADSR_SEG_DECAY, MAX(0.01, synthSettings[uiSynth].adsrDecay));
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 6)) {

					// SOUND: adSr - level, not time (from 0-1)
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].adsrSustain <= 0.94)
						{
							synthSettings[uiSynth].adsrSustain += 0.05;
							synthAEnv[uiSynth].SetSustainLevel(synthSettings[uiSynth].adsrSustain);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].adsrSustain >= 0.05)
						{
							synthSettings[uiSynth].adsrSustain -= 0.05;
							synthAEnv[uiSynth].SetSustainLevel(synthSettings[uiSynth].adsrSustain);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 9)) {

					// SOUND: adsR
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].adsrRelease < 9.0)
						{
							synthSettings[uiSynth].adsrRelease += 0.05;
							synthAEnv[uiSynth].SetTime(ADSR_SEG_RELEASE, synthSettings[uiSynth].adsrRelease);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].adsrRelease >= 0.05)
						{
							synthSettings[uiSynth].adsrRelease -= 0.05;
							synthAEnv[uiSynth].SetTime(ADSR_SEG_RELEASE, synthSettings[uiSynth].adsrRelease);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 13;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 13)) {

					// SOUND: LFO amount 0-99/100
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						if (synthSettings[uiSynth].lfoAmount < 0.99)
						{
							synthSettings[uiSynth].lfoAmount += 0.01;
							lfoOsc[uiSynth].SetAmp(synthSettings[uiSynth].lfoAmount);
						}
					} else if (button == BUTTON_DOWN) {
						if (synthSettings[uiSynth].lfoAmount >= 0.01)
						{
							synthSettings[uiSynth].lfoAmount -= 0.01;
							lfoOsc[uiSynth].SetAmp(synthSettings[uiSynth].lfoAmount);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 15;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 15)) {

					// SOUND: LFO target
					if (button == BUTTON_SELECT) {
					} else if (button == BUTTON_UP) {
						synthSettings[uiSynth].lfoTarget = LFO_TARGET_FILTER;
					} else if (button == BUTTON_DOWN) {
						synthSettings[uiSynth].lfoTarget = LFO_TARGET_NONE;
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 13;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}
				}
			} else {
				// drum
				// kick: pitch, len
				// snare: pitch, len, noise
				// hihat: open: filter, len, closed: filter, len
				// 1234567812345678
				// KP KL SP SL SN X
				// OF OL CF CL CRCL

				if ((uiSubRow == 0) && (uiSubCol == 0)) {

					// SOUND: kick pitch
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.kickFrequency < 900)
						{
							drumSettings.kickFrequency += 10;
							kickPEnv.SetMin(drumSettings.kickFrequency / 10.0f);
    						kickPEnv.SetMax(drumSettings.kickFrequency);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.kickFrequency > 50)
						{
							drumSettings.kickFrequency -= 10;
							kickPEnv.SetMin(drumSettings.kickFrequency / 10.0f);
    						kickPEnv.SetMax(drumSettings.kickFrequency);
						}
					} else if (button == BUTTON_LEFT) {
						uiMode = UI_MODE_OVERVIEW;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 3)) {

					// SOUND: kick len
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.kickDecay < 1)
						{
							drumSettings.kickDecay += 0.05;
    						kickPEnv.SetTime(ADENV_SEG_DECAY, drumSettings.kickDecay);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.kickDecay > 0)
						{
							drumSettings.kickDecay -= 0.05;
    						kickPEnv.SetTime(ADENV_SEG_DECAY, drumSettings.kickDecay);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 6)) {

					// SOUND: snare pitch
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.snareFrequency < 900)
						{
							drumSettings.snareFrequency += 10;
							snareOPEnv.SetMin(drumSettings.snareFrequency / 3.0f);
							snareOPEnv.SetMax(drumSettings.snareFrequency);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.snareFrequency > 50)
						{
							drumSettings.snareFrequency -= 10;
							snareOPEnv.SetMin(drumSettings.snareFrequency / 3.0f);
							snareOPEnv.SetMax(drumSettings.snareFrequency);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 9)) {

					// SOUND: snare len
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.snareDecay < 1)
						{
							drumSettings.snareDecay += 0.05;
							snareNAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
							snareOAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
							snareOPEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.snareDecay > 0)
						{
							drumSettings.snareDecay -= 0.05;
							snareNAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
							snareOAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
							snareOPEnv.SetTime(ADENV_SEG_DECAY, drumSettings.snareDecay);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}
				} else if ((uiSubRow == 0) && (uiSubCol == 12)) {

					// SOUND: snare noise (0-99%)
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.snareNoise < 94)
						{
							drumSettings.snareNoise += 5;
							snareNoiseLPF.SetFreq(3000 - (drumSettings.snareNoise * 20));
							snareNAEnv.SetMax(1.0f * (drumSettings.snareNoise / 100.0f));
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.snareNoise > 0)
						{
							drumSettings.snareNoise -= 5;
							snareNoiseLPF.SetFreq(3000 - (drumSettings.snareNoise * 20));
							snareNAEnv.SetMax(1.0f * (drumSettings.snareNoise / 100.0f));
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 1;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 0)) {

					// SOUND: hho filter
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.hhoFilter < 7000)
						{
							drumSettings.hhoFilter += 100;
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.hhoFilter > 300)
						{
							drumSettings.hhoFilter -= 100;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 12;
						uiSubRow = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 3;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 3)) {

					// SOUND: hho len
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.hhoDecay < 5)
						{
							drumSettings.hhoDecay += 0.05;
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.hhoDecay > 0)
						{
							drumSettings.hhoDecay -= 0.05;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 0;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 6;
					}
				} else if ((uiSubRow == 1) && (uiSubCol == 6)) {

					// SOUND: hhc filter
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.hhcFilter < 7000)
						{
							drumSettings.hhcFilter += 100;
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.hhcFilter > 300)
						{
							drumSettings.hhcFilter -= 100;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 3;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 9;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 9)) {

					// SOUND: hhc len
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.hhcDecay < 5)
						{
							drumSettings.hhcDecay += 0.05;
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.hhcDecay > 0)
						{
							drumSettings.hhcDecay -= 0.05;
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 6;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 12;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 12)) {

					// SOUND: crash filter
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.crashFilter <= 4000)
						{
							drumSettings.crashFilter += 500;
						    crashNoiseHPF.SetFreq(drumSettings.crashFilter);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.crashFilter >= 500)
						{
							drumSettings.crashFilter -= 500;
						    crashNoiseHPF.SetFreq(drumSettings.crashFilter);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 9;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 13;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 13)) {

					// SOUND: crash decay
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.crashDecay <= 2.7)
						{
							drumSettings.crashDecay += 0.3;
							crashNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.crashDecay);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.crashDecay >= 0.3)
						{
							drumSettings.crashDecay -= 0.3;
							crashNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.crashDecay);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 12;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 14;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 14)) {

					// SOUND: clap filter
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.clapFilter <= 1700)
						{
							drumSettings.clapFilter += 100;
						    clapNoiseBPF.SetFreq(drumSettings.clapFilter);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.clapFilter >= 800)
						{
							drumSettings.clapFilter -= 100;
						    clapNoiseBPF.SetFreq(drumSettings.clapFilter);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 13;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 15;
					}

				} else if ((uiSubRow == 1) && (uiSubCol == 15)) {

					// SOUND: clap decay
					if (button == BUTTON_SELECT)
					{
					} else if (button == BUTTON_UP) {
						if (drumSettings.clapDecay <= 0.8)
						{
							drumSettings.clapDecay += 0.1;
							clapNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.clapDecay);
						}
					} else if (button == BUTTON_DOWN) {
						if (drumSettings.clapDecay >= 0.1)
						{
							drumSettings.clapDecay -= 0.1;
							clapNoiseAEnv.SetTime(ADENV_SEG_DECAY, drumSettings.clapDecay);
						}
					} else if (button == BUTTON_LEFT) {
						uiSubCol = 14;
					} else if (button == BUTTON_RIGHT) {
						uiSubCol = 0;
						uiSubRow = 0;
					}


				} //

			}
			break;
		}


		uiRedraw = true;

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

		// lcd->clear();

		switch (uiMode)
		{
		case UI_MODE_OVERVIEW:

			// row 0

			// Play/off
			switch (sysPlayMode)
			{
			case PLAY_OFF:
				buffer0[0] = 'X';
				break;
			case PLAY_SEQ:
				buffer0[0] = sysPlay ? 'P' : 'p';
				break;
			case PLAY_SONG:
				buffer0[0] = sysPlay ? 'S' : 's';
				break;
			}

			// BPM (max 3 digits)
			bufferDrawInt(buffer0, 1, 3, sysTickBPM);

			// Song step (max 3 digits)
			bufferDrawInt(buffer0, 4, 6, seqSongStep);

			// utility menu
			buffer0[7] = 'U';

			// Mixer levels / 10
			for (int i = 0; i < (TRACK_NUMBER - 1); i++)
			{
				buffer0[8 + i] = (mixSynth[i].volume / 10) + 48;
			}
			buffer0[15] = 'D';

			lcd->SetCursor(0, 0);
			lcd->Print(buffer0);
			
			// row 1

			// notes: current seq
			for (int i = 0; i < 8; i++)
			{
				buffer1[i] = 48 + songStep[seqSongStep][i];
			}
			// sounds (# = on, o=off)
			for (int i = 0; i < 8; i++)
			{
				buffer1[i+8] = seqVoiceOn[i] ? (48 + i + 1) : 'X';
			}

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiMainRow, uiMainCol);
			
			break;

		case UI_MODE_SOUND:

			if (uiSynth < 7)
			{
				// row 0 

				// waveform
				bufferDrawStr(buffer0, 0, synthOscWaveNames[synthSettings[uiSynth].oscWaveform]);

				// detune
				bufferDrawInt(buffer0, 3, 5, synthSettings[uiSynth].oscDetune);

				// cutoff
				bufferDrawInt(buffer0, 6, 8, (int)(synthSettings[uiSynth].filterCutoff / 100));

				// res
				bufferDrawInt(buffer0, 9, 11, (int)(synthSettings[uiSynth].filterRes * 100));

				// LFO freq
				bufferDrawInt(buffer0, 13, 15, (int)(synthSettings[uiSynth].lfoFrequency * 10));

				lcd->SetCursor(0, 0);
				lcd->Print(buffer0);

				// row 1

				// Adsr
				bufferDrawInt(buffer1, 0, 2, (int)(synthSettings[uiSynth].adsrAttack * 100));

				// aDsr
				bufferDrawInt(buffer1, 3, 5, (int)(synthSettings[uiSynth].adsrDecay * 100));

				// adSr
				bufferDrawInt(buffer1, 6, 8, (int)(synthSettings[uiSynth].adsrSustain * 100));

				// adsR
				bufferDrawInt(buffer1, 9, 11, (int)(synthSettings[uiSynth].adsrRelease * 100));

				// LFO amount
				bufferDrawInt(buffer1, 13, 14, (int)(synthSettings[uiSynth].lfoAmount * 100));

				// LFO target
				switch (synthSettings[uiSynth].lfoTarget)
				{
				case LFO_TARGET_NONE:
					buffer1[15] = 'X';
					break;
				case LFO_TARGET_PITCH:
					buffer1[15] = 'P';
					break;
				case LFO_TARGET_FILTER:
					buffer1[15] = 'F';
					break;
				case LFO_TARGET_AMP:
					buffer1[15] = 'A';
					break;
				}

				lcd->SetCursor(1, 0),
				lcd->Print(buffer1);

				lcd->SetCursor(uiSubRow, uiSubCol);

			} else {

				// drum
				// 1234567812345678
				// KP KL SP SL SN _
				// OF OL CF CL XxPp

				// row 0

				// kick pitch
				bufferDrawInt(buffer0, 0, 1, (int)(drumSettings.kickFrequency / 10));

				// kick len
				bufferDrawInt(buffer0, 3, 4, (int)(drumSettings.kickDecay * 100));

				// snare pitch
				bufferDrawInt(buffer0, 6, 7, (int)(drumSettings.snareFrequency / 10));

				// snare len
				bufferDrawInt(buffer0, 9, 10, (int)(drumSettings.snareDecay * 100));

				// snare len
				bufferDrawInt(buffer0, 12, 13, (int)(drumSettings.snareNoise));

				// row 1

				// hho filter freq
				bufferDrawInt(buffer1, 0, 1, (int)(drumSettings.hhoFilter / 100));

				// hho decay
				bufferDrawInt(buffer1, 3, 4, (int)(drumSettings.hhoDecay * 100));

				// hhc filter freq
				bufferDrawInt(buffer1, 6, 7, (int)(drumSettings.hhcFilter / 100));

				// hhc decay
				bufferDrawInt(buffer1, 9, 10, (int)(drumSettings.hhcDecay * 100));

				// crash filter freq
				buffer1[12] = 48 + (int)(drumSettings.crashFilter / 500);

				// crash decay
				buffer1[13] = 48 + (int)(drumSettings.crashDecay * 3.0f);

				// clap filter freq
				buffer1[14] = 48 + (int)((drumSettings.clapFilter - 800)/ 100);

				// clap decay
				buffer1[15] = 48 + (int)(drumSettings.clapDecay * 10.0f);

				lcd->SetCursor(0, 0);
				lcd->Print(buffer0);
				lcd->SetCursor(1, 0),
				lcd->Print(buffer1);

				lcd->SetCursor(uiSubRow, uiSubCol);

			}

			break;

		case UI_MODE_SONGEDIT:

			// row 0

			// song step
			bufferDrawInt(buffer0, 0, 2, seqSongStep);

			// Last?
			buffer0[3] = (seqSongLen == seqSongStep) ? '*' : ' ';

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			// row 1

			for (int i = 0; i < SEQ_VOICES; i++)
			{
				bufferDrawInt(buffer1, i * 2, i * 2 + 1, songStep[seqSongStep][i]);
			}
			lcd->SetCursor(1, 0);
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_MIXER:
			// Level (2), Pan (1), Fx (1)

			if (uiSynth < 7)
			{

				// row 0

				for (int i = 0; i < 4; i++)
				{
					bufferDrawInt(buffer0, i * 4, i * 4 + 1, mixSynth[i].volume);
					buffer0[i * 4 + 2] = 48 + mixSynth[i].pan;
					buffer0[i * 4 + 3] = 48 + mixSynth[i].reverb;
				}

				lcd->SetCursor(0, 0),
				lcd->Print(buffer0);

				// row 1

				for (int i = 0; i < 3; i++)
				{
					bufferDrawInt(buffer1, i * 4, i * 4 + 1, mixSynth[i + 4].volume);
					buffer1[i * 4 + 2] = 48 + mixSynth[i + 4].pan;
					buffer1[i * 4 + 3] = 48 + mixSynth[i + 4].reverb;
				}

				lcd->SetCursor(1, 0),
				lcd->Print(buffer1);

			} else {

				// drums

				// kick
				bufferDrawInt(buffer0, 0, 1, mixDrum.kickVolume);
				buffer0[2] = 48 + mixDrum.kickPan;
				buffer0[3] = 48 + mixSynth[OFFSET_DRUM + 0].reverb;

				// snare
				bufferDrawInt(buffer0, 4, 5, mixDrum.snareVolume);
				buffer0[6] = 48 + mixDrum.snarePan;
				buffer0[7] = 48 + mixSynth[OFFSET_DRUM + 1].reverb;

				// hihat
				bufferDrawInt(buffer0, 8, 9, mixDrum.hihatVolume);
				buffer0[10] = 48 + mixDrum.hihatPan;
				buffer0[11] = 48 + mixSynth[OFFSET_DRUM + 2].reverb;

				// crash
				bufferDrawInt(buffer0, 12, 13, mixDrum.crashVolume);
				buffer0[14] = 48 + mixDrum.crashPan;
				buffer0[15] = 48 + mixSynth[OFFSET_DRUM + 4].reverb;

				// clap
				bufferDrawInt(buffer1, 0, 1, mixDrum.clapVolume);
				buffer1[2] = 48 + mixDrum.clapPan;
				buffer1[3] = 48 + mixSynth[OFFSET_DRUM + 3].reverb;

				lcd->SetCursor(0, 0),
				lcd->Print(buffer0);

				lcd->SetCursor(1, 0),
				lcd->Print(buffer1);
			}

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_NOTE:
			switch (uiSynth)
			{
			case 0:
			case 1:
			case 2:
				// bass

				for (int i = 0; i < 16; i++)
				{
					buffer0[i] = 48 + seqBass[uiSynth][uiSequence][i + uiSubColOffset] / 10;
					buffer1[i] = 48 + seqBass[uiSynth][uiSequence][i + uiSubColOffset] % 10;
				}
				break;

			case 3:
			case 4:
			case 5:
			case 6:
				// lead

				for (int i = 0; i < 8; i++)
				{
					uint8_t theVoice = uiSynth - BASS_NUMBER;
					uint8_t theNote = i + (uiSubColOffset / 2);

					if (theNote == seqLeadLen[uiSynth - BASS_NUMBER][uiSequence])
					{
						buffer0[i * 2 + 1] = '*';
						buffer1[i * 2 + 1] = '*';
					} else {
						buffer0[i * 2] = 48 + seqLead[theVoice][uiSequence][theNote].notePitch / 10;
						buffer1[i * 2] = 48 + seqLead[theVoice][uiSequence][theNote].notePitch % 10;
						buffer0[i * 2 + 1] = 48 + seqLead[theVoice][uiSequence][theNote].noteTime / 10;
						buffer1[i * 2 + 1] = 48 + seqLead[theVoice][uiSequence][theNote].noteTime % 10;
					}
				}
				break;

			case 7:
				// drum

				for (int i = 0; i < 16; i++)
				{
					if (uiSubRowOffset == 0)
					{
						buffer0[i] = (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_KICK ? 'K' : ' ');
						buffer1[i] = (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_SNARE ? 'S' : ' ');
					} else if (uiSubRowOffset == 1) {
						buffer0[i] = (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_SNARE ? 'S' : ' ');
						if (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_HHOPEN)
						{
							buffer1[i] = 'O';
						} else if (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_HHCLOSED)
						{
							buffer1[i] = 'C';
						} 
					} else if (uiSubRowOffset == 2) {
						if (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_HHOPEN)
						{
							buffer0[i] = 'O';
						} else if (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_HHCLOSED)
						{
							buffer0[i] = 'C';
						} 
						buffer1[i] = (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_CRASH ? 'X' : ' ');
					} else if (uiSubRowOffset == 3) {
						buffer0[i] = (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_CRASH ? 'X' : ' ');
						buffer1[i] = (seqDrum[uiSequence][i + uiSubColOffset] & DRUM_CLAP ? 'P' : ' ');
					}
				}

				break;
			}

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_UTILITY:

			bufferDrawStr(buffer0, 0, "SvLdRvDySM");

			bufferDrawInt(buffer0, 10, 11, seqBassGateTime[0]);
			bufferDrawInt(buffer0, 12, 13, seqBassGateTime[1]);
			bufferDrawInt(buffer0, 14, 15, seqBassGateTime[2]);

			bufferDrawStr(buffer1, 0, "TSCXG");

			buffer1[7] = ((uiUtilDirection == UI_UTIL_DIR_UP) ? '+' : '-');
			bufferDrawInt(buffer1, 8, 9, uiUtilValue);
			buffer1[10] = uiTrackTypeSymbol[uiUtilFromTrackType];
			buffer1[11] = 48 + uiUtilFromTrack;
			buffer1[12] = 48 + uiUtilFromSeq;
			buffer1[13] = uiTrackTypeSymbol[uiUtilToTrackType];
			buffer1[14] = 48 + uiUtilToTrack;
			buffer1[15] = 48 + uiUtilToSeq;

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_FX_REVERB:
			// 1234567812345678
			// FFrq
			buffer0[0] = 48 + (int)(fxSettings.reverbFeedback * 10);
			bufferDrawInt(buffer0, 1, 3, fxSettings.reverbLpfFrequency / 100);

			lcd->SetCursor(0, 0),
			lcd->Print(buffer0);

			lcd->SetCursor(1, 0),
			lcd->Print(buffer1);

			lcd->SetCursor(uiSubRow, uiSubCol);

			break;

		case UI_MODE_FX_DELAY:
			for (int i = 0; i < 4; i++)
			{
				bufferDrawInt(buffer0, i * 4, i * 4 + 1, (int)(synthSettings[i].delayDelay * 10));
				bufferDrawInt(buffer0, i * 4 + 2, i * 4 + 3, (int)(synthSettings[i].delayFeedback * 100));
			}

			for (int i = 0; i < 3; i++)
			{
				bufferDrawInt(buffer1, i * 4, i * 4 + 1, (int)(synthSettings[i + 4].delayDelay * 10));
				bufferDrawInt(buffer1, i * 4 + 2, i * 4 + 3, (int)(synthSettings[i + 4].delayFeedback * 100));
			}

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
