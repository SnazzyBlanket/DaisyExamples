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
//           111111
// 0123456789012345
// IGUL
// OAFAADR
#define UI_MODE_OSC 18 // wave, (de)tune, osc2 wave, osc2 detune, o2 lvl, noise lvl, portamento
//           111111
// 0123456789012345
// Wa1 tune1 vo por
// Wa2 detu2 l2 nl
#define UI_MODE_OSC_EG 40 // EG_P
#define UI_MODE_FILTER_EG 41 // EG_F
#define UI_MODE_AMP_EG 42 // EG_A
#define UI_MODE_FILTER 10 // type, cutoff, res
#define UI_MODE_DELAY 14
#define UI_MODE_REVERB 15
#define UI_MODE_MIX 5
#define UI_MODE_LFO 30

#define UI_MODE_UTILITY 20 // save, load, midi channel
#define UI_MODE_LOAD 51
#define UI_MODE_SAVE 50
#define UI_MODE_TEST 60



class OscUI
{

private:
	uint8_t button;
	uint8_t buttonPrevious;
	uint32_t buttonPreviousTime;
	uint32_t buttonPreviousCount;
	uint32_t buttonMult;

	daisy::LcdHD44780 *lcd;

	uint8_t uiMode;
	uint8_t uiMainRow, uiMainCol;
	uint8_t uiSubRow, uiSubCol;
	uint8_t uiSubColOffset, uiSubRowOffset;
	uint8_t uiSM; // Special Modulator

	void bufferDrawInt(char *, uint8_t, uint8_t, int);
	void bufferDrawStr(char *, uint8_t, const char *);

public:
	void Init(daisy::LcdHD44780 *);
	void Button(float);
	void Work();
	void Draw(void);

};


