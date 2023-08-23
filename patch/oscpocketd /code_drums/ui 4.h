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
#define UI_MODE_MIXER 2
#define UI_MODE_UTILITY 3
#define UI_MODE_REVERB 5
#define UI_MODE_SAVE 6
#define UI_MODE_LOAD 7
#define UI_MODE_SEQ 8

#define UI_MODE_EDIT_BASS1 10
#define UI_MODE_EDIT_BASS2 11
#define UI_MODE_EDIT_BASS3 12

#define UI_MODE_EDIT_SNARE1 15
#define UI_MODE_EDIT_SNARE2 16
#define UI_MODE_EDIT_SNARE3 17

#define UI_MODE_EDIT_HIHAT1 20
#define UI_MODE_EDIT_HIHAT2 21
#define UI_MODE_EDIT_HIHAT3 22

#define UI_MODE_EDIT_CRASH1 25
#define UI_MODE_EDIT_CRASH2 26
//#define UI_MODE_EDIT_CRASH3 27

#define UI_MODE_EDIT_RIDE1 30
#define UI_MODE_EDIT_RIDE2 31
//#define UI_MODE_EDIT_RIDE3 32

#define UI_MODE_EDIT_CLAP1 35
#define UI_MODE_EDIT_CLAP2 36
//#define UI_MODE_EDIT_CLAP3 37

#define UI_MODE_EDIT_TOMHI1 40
#define UI_MODE_EDIT_TOMHI2 41
#define UI_MODE_EDIT_TOMHI3 42

#define UI_MODE_EDIT_TOMLO1 45
#define UI_MODE_EDIT_TOMLO2 46
#define UI_MODE_EDIT_TOMLO3 47

//           111111
// 0123456789012345
// IGUL
// OAFAADR



class OscUI
{

private:
	uint8_t button;
	uint8_t button_previous;
	uint32_t button_previous_time;
	uint32_t button_previous_count;
	uint32_t button_mult;

	daisy::LcdHD44780 *lcd;

	bool ui_redraw;
	uint8_t ui_mode;
	uint8_t ui_row, ui_col;
	uint8_t ui_row_sub, ui_col_sub;
	uint8_t ui_row_sub_offset, ui_col_sub_offset;
	uint8_t ui_track, ui_track_offset;

	void bufferDrawInt(char *, uint8_t, uint8_t, int);
	void bufferDrawStr(char *, uint8_t, const char *);
	
	void uInc(uint8_t *, uint8_t);
	void uDec(uint8_t *, uint8_t);
	void fIncP(float *, float);
	void fDecP(float *, float);
	void fInc(float *, float);
	void fDec(float *, float);
	
	void MixerLevelInc(uint8_t);
	void MixerLevelDec(uint8_t);
	void MixerPanInc(uint8_t);
	void MixerPanDec(uint8_t);

public:
	void Init(daisy::LcdHD44780 *);
	void Button(float);
	void Work();
	void Draw(void);

};


