#pragma once

#include "dev/lcd_hd44780.h"

// buttons
#define BUTTON_NONE 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_LEFT 3
#define BUTTON_RIGHT 4
#define BUTTON_SELECT 5

// UI
#define UI_MODE_OVERVIEW 1
#define UI_MODE_SONGEDIT 3
#define UI_MODE_MIXER 4

#define UI_MODE_NOTE 20
#define UI_MODE_NOTE_BASS 21
#define UI_MODE_NOTE_LEAD 22
#define UI_MODE_NOTE_DRUM 24

#define UI_MODE_SOUND 30
#define UI_MODE_SOUND_SYNTH 31
#define UI_MODE_SOUND_DRUM 32

#define UI_MODE_FX 40
#define UI_MODE_FX_REVERB 41
#define UI_MODE_FX_DELAY 42

#define UI_MODE_UTILITY 100
// for uiUtilValue
#define UI_UTIL_DIR_UP 1
#define UI_UTIL_DIR_DOWN 2
// for uiUtilFromTrack and uiUtilToTrack
#define UI_UTIL_TARGET_TRACK_B 1
#define UI_UTIL_TARGET_TRACK_L 2
#define UI_UTIL_TARGET_TRACK_D 3



class OscUI
{

private:
	uint8_t button;
	uint8_t buttonPrevious;
	uint32_t buttonPreviousTime;

	daisy::LcdHD44780 *lcd;

	uint8_t uiMode;
	uint8_t uiMainRow, uiMainCol;
	uint8_t uiSubRow, uiSubCol;
	uint8_t uiSubColOffset, uiSubRowOffset;

	uint8_t uiSynth;
	uint8_t uiSequence;

	uint8_t uiUtilDirection;
	uint8_t uiUtilValue;
	uint8_t uiUtilFromTrackType;
	uint8_t uiUtilFromTrack; // 0-2, 0-2, 0, 0
	uint8_t uiUtilFromSeq;
	uint8_t uiUtilToTrackType;
	uint8_t uiUtilToTrack; // 0-2, 0-2, 0, 0
	uint8_t uiUtilToSeq;
	char uiTrackTypeSymbol[4] = {' ', 'B', 'L', 'D'};

	void bufferDrawInt(char *, uint8_t, uint8_t, int);
	void bufferDrawStr(char *, uint8_t, const char *);

public:
	void Init(daisy::LcdHD44780 *);
	void Button(float);
	void Work();
	void Draw(void);

};


