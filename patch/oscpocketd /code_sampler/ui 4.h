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
#define UI_MODE_MIXER 4
#define UI_MODE_GAIN 5

#define UI_MODE_FILTER 10
#define UI_MODE_OVERDRIVE 11
#define UI_MODE_DECIMATOR 12
#define UI_MODE_ADSR 13
#define UI_MODE_DELAY 14
#define UI_MODE_REVERB 15
#define UI_MODE_CHORUS 16
#define UI_MODE_FLANGER 17
#define UI_MODE_OSC 18
#define UI_MODE_SAMPLE 19

#define UI_MODE_UTILITY 20
#define UI_MODE_UTILITY_ADC 21
#define UI_MODE_LFO 30
#define UI_MODE_CV 31


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
	uint8_t uiLFO;
	uint8_t uiCV;

	void bufferDrawInt(char *, uint8_t, uint8_t, int);
	void bufferDrawStr(char *, uint8_t, const char *);

public:
	void Init(daisy::LcdHD44780 *);
	void Button(float);
	void Work();
	void Draw(void);

};


