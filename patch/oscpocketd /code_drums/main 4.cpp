/*
  
Project: OscPocketD/Drums
Description: drum machine for the OscPcoketD/Base (Daisy Seed)
Author: Staffan Melin, staffan.melin@oscillator.se
License: GNU General Public License v3.0
Version: 202111
Project site: http://www.oscillator.se/opensource

*/

//#define OPD_LOGG // start serial over USB Logger class
//#define OPD_MEASURE // start serial over USB Logger class
#define OPD_BASE_MIDI // version 2 with MIDI, other LCD pins, no CV1 in, no Gate0/1 out

#include <stdint.h>

#include "daisy_seed.h"
#include "daisysp.h"

#include "stm32h7xx_hal.h" // for HAL_NVIC_SystemReset();
//extern "C" void HAL_NVIC_SystemReset();
#include "core_cm7.h"
#include "dev/lcd_hd44780.h"

#include "main.h"
#include "ui.h"
#include "dbass.h"
#include "dsnare.h"
#include "dhihat.h"
#include "ddrum.h"
#include "dcymbal.h"
#include "dclap.h"

using namespace daisy;
using namespace daisysp;

// globals
DaisySeed hardware;
float g_sample_rate;
float g_callback_rate;

uint8_t g_input = INPUT_NONE;
uint8_t g_pot0 = POT_NONE;
float g_ad_gate0;
float g_ad_gate1;
float g_ad_cv0;
float g_ad_cv1;
float g_ad_pot0;
float g_ad_pot1;
bool g_ad_gate0_trigged = false;
bool g_ad_gate1_trigged = false;
bool g_ad_cv0_trigged = false;
bool g_ad_cv1_trigged = false;

// UI

// hardware
LcdHD44780 lcd;
AdcChannelConfig adc_config[AD_MAX];

// sound + fx

Settings settings;

#define DELAY_MAX static_cast<size_t>(48000 * DELAY_MAX_S)
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dbass;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dsnare;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dhihat;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dclap;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dcrash;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dride;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dtomlo;
static DelayLine<float, DELAY_MAX> DSY_SDRAM_BSS delay_dtomhi;

// overdrive
Overdrive overdrive_bass;
Overdrive overdrive_snare;

// reverb
ReverbSc reverb;

DBass dbass;
DSnare dsnare;
DHihat dhihat;
DCymbal dcrash;
DCymbal dride;
DClap dclap;
DDrum dtomhi;
DDrum dtomlo;

// seq

uint8_t seq_step; // current step in sequence
uint8_t g_play = PLAY_ON;
uint8_t seq_sync = SYNC_INT;

uint32_t tick_last;
uint32_t tick_len;
bool trig_on;
uint8_t	trig_count;
float trig_gate_level = 0.5;
uint8_t trig_pp16 = 6; // 24ppq



// prototypes / forward decl

void playNotes();



// seq

void tickIntCalc()
{
//	tick_len = 1000000 / (4 * settings.seq_tempo / 60);
	tick_len = 1000 / (4 * settings.seq_tempo / 60);
}



void tickIntStart()
{
	tickIntCalc();
	tick_last = System::GetNow();
	seq_step = 0;
	playNotes();	
}



void tickIntProcess()
{
	uint32_t tick_current = System::GetNow();

	if (tick_last + tick_len < tick_current)
	{
		tick_last = tick_current;
		

		seq_step++;
		if (seq_step == settings.seq_end)
		{
			seq_step = 0;
		}

		playNotes();

		// send trig out
		hardware.dac.WriteValue(DacHandle::Channel::ONE, 3300);
		
	} else {
		// send trig out off
		if ((tick_last + (tick_len/2)) < tick_current)
			hardware.dac.WriteValue(DacHandle::Channel::ONE, 0);
	}
}



void tickExtStart()
{
	trig_on = false;
	trig_count = 0;
}



void tickExtProcess()
{
	// trig detected?
	if (g_ad_gate0 > trig_gate_level)
	{
		if (!trig_on)
		{

			playNotes();
			trig_on = true;

			seq_step++;
			if (seq_step == settings.seq_end)
			{
				seq_step = 0;
			}
		}
	} else {
		trig_count++;
		if (trig_count == trig_pp16)
		{
			trig_count = 0;
			trig_on = false;
		}
	}
}



void trigStart()
{
	g_ad_gate0_trigged = false;
	g_ad_gate1_trigged = false;
	g_ad_cv0_trigged = false;
	g_ad_cv1_trigged = false;
}



void trigProcess()
{
	// gate0 - dbass
	if (g_ad_gate0 > trig_gate_level)
	{
		if (!g_ad_gate0_trigged)
		{
			dbass.Trig(1.0f);
			g_ad_gate0_trigged = true;
		}
	} else {
		g_ad_gate0_trigged = false;
	}

	// gate1 - snare
	if (g_ad_gate1 > trig_gate_level)
	{
		if (!g_ad_gate1_trigged)
		{
			dsnare.Trig(1.0f);
			g_ad_gate1_trigged = true;
		}
	} else {
		g_ad_gate1_trigged = false;
	}

	// cv0 - hihat
	if (g_ad_cv0 > trig_gate_level)
	{
		if (!g_ad_cv0_trigged)
		{
			dhihat.Trig(1.0f, DRUM_NORMAL);
			g_ad_cv0_trigged = true;
		}
	} else {
		g_ad_cv0_trigged = false;
	}

	// cv1 - crash
	if (g_ad_cv1 > trig_gate_level)
	{
		if (!g_ad_cv1_trigged)
		{
			dcrash.Trig(1.0f);
			g_ad_cv1_trigged = true;
		}
	} else {
		g_ad_cv1_trigged = false;
	}
}



// audio callback

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
	float voice, voice_left, voice_right;
	float delay_read;
	float mix_left, mix_right;
	float rev_send_left, rev_send_right;
	float rev_out_left, rev_out_right;

	#ifdef OPD_MEASURE
	// measure - start
	DWT->CYCCNT = 0;
	#endif
	
	// sequencer

	switch (g_play)
	{
	case PLAY_ON:
		switch (seq_sync)
		{
		case SYNC_INT:
			tickIntProcess();
			break;
		case SYNC_EXT:
			tickExtProcess();
			break;
		}
		break;
	case PLAY_TRIG:
		trigProcess();
		break;
	}

    for (size_t n = 0; n < size; n += 2)
    {

		// bass

		voice = dbass.Process() * settings.dbass.level;
		// overdrive
		if (settings.dbass.overdrive > 0.0f)
		{
			voice = overdrive_bass.Process(voice);
		}
		// delay
		if (settings.dbass.delay_feedback > 0.0f)
		{
			delay_read = delay_dbass.Read();
			delay_dbass.Write((voice + delay_read) * settings.dbass.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dbass.pan);
		voice_right = voice * (settings.dbass.pan);
		// mix
		mix_left = voice_left;
		mix_right = voice_right;
		rev_send_left = voice_left * settings.dbass.reverb_level;
		rev_send_right = voice_right * settings.dbass.reverb_level;
		
		// snare
		
		voice = dsnare.Process() * settings.dsnare.level;
		// overdrive
		if (settings.dsnare.overdrive > 0.0f)
		{
			voice = overdrive_snare.Process(voice);
		}
		// delay
		if (settings.dsnare.delay_feedback > 0.0f)
		{
			delay_read = delay_dsnare.Read();
			delay_dsnare.Write((voice + delay_read) * settings.dsnare.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dsnare.pan);
		voice_right = voice * (settings.dsnare.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dsnare.reverb_level;
		rev_send_right += voice_right * settings.dsnare.reverb_level;

		// hihat

		voice = dhihat.Process() * settings.dhihat.level;
		// delay
		if (settings.dhihat.delay_feedback > 0.0f)
		{
			delay_read = delay_dhihat.Read();
			delay_dhihat.Write((voice + delay_read) * settings.dhihat.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dhihat.pan);
		voice_right = voice * (settings.dhihat.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dhihat.reverb_level;
		rev_send_right += voice_right * settings.dhihat.reverb_level;

		// ride

		voice = dride.Process() * settings.dride.level;
		// delay
		if (settings.dride.delay_feedback > 0.0f)
		{
			delay_read = delay_dride.Read();
			delay_dride.Write((voice + delay_read) * settings.dride.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dride.pan);
		voice_right = voice * (settings.dride.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dride.reverb_level;
		rev_send_right += voice_right * settings.dride.reverb_level;

		// crash
		
		voice = dcrash.Process() * settings.dcrash.level;
		// delay
		if (settings.dcrash.delay_feedback > 0.0f)
		{
			delay_read = delay_dcrash.Read();
			delay_dcrash.Write((voice + delay_read) * settings.dcrash.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dcrash.pan);
		voice_right = voice * (settings.dcrash.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dcrash.reverb_level;
		rev_send_right += voice_right * settings.dcrash.reverb_level;

		// clap

		voice = dclap.Process() * settings.dclap.level;
		// delay
		if (settings.dclap.delay_feedback > 0.0f)
		{
			delay_read = delay_dclap.Read();
			delay_dclap.Write((voice + delay_read) * settings.dclap.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dclap.pan);
		voice_right = voice * (settings.dclap.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dclap.reverb_level;
		rev_send_right += voice_right * settings.dclap.reverb_level;

		// tom hi
					
		voice = dtomhi.Process();
		// delay
		if (settings.dtomhi.delay_feedback > 0.0f)
		{
			delay_read = delay_dtomhi.Read();
			delay_dtomhi.Write((voice + delay_read) * settings.dtomhi.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dtomhi.pan);
		voice_right = voice * (settings.dtomhi.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dtomhi.reverb_level;
		rev_send_right += voice_right * settings.dtomhi.reverb_level;
		
		// tom lo
		
		voice = dtomlo.Process();
		// delay
		if (settings.dtomlo.delay_feedback > 0.0f)
		{
			delay_read = delay_dtomlo.Read();
			delay_dtomlo.Write((voice + delay_read) * settings.dtomlo.delay_feedback);
			voice += delay_read;
		}
		// pan
		voice_left = voice * (1.0f - settings.dtomlo.pan);
		voice_right = voice * (settings.dtomlo.pan);
		// mix
		mix_left += voice_left;
		mix_right += voice_right;
		rev_send_left += voice_left * settings.dtomlo.reverb_level;
		rev_send_right += voice_right * settings.dtomlo.reverb_level;
		
		// reverb send
		
		if (settings.reverb_on)
		{	
			reverb.Process(rev_send_left, rev_send_right, &rev_out_left, &rev_out_right);
			mix_left = mix_left * settings.reverb_dry + rev_out_left * settings.reverb_wet;
			mix_right = mix_right * settings.reverb_dry + rev_out_right * settings.reverb_wet;
		}
		
		// compressor
		// TODO
		
		if (g_input == INPUT_NONE)
		{
			out[n] = mix_left * settings.gain_out;
			out[n + 1] = mix_right * settings.gain_out;
		} else {
			out[n] = mix_left * settings.gain_out + in[n] * settings.gain_in;
			out[n + 1] = mix_right * settings.gain_out + in[n + 1] * settings.gain_in;
		}
		
	}

	#ifdef OPD_MEASURE
	// measure - stop
	if (DWT->CYCCNT > 390000)
	{
		hardware.SetLed(true);
	}
	#endif
}



void playNotes()
{
	uint8_t step_vol;
	
	if (settings.dbass.on && ((step_vol = settings.seq[TRACK_BASS][seq_step]) > 0))
	{
		dbass.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dbass.accent);
	}
	if (settings.dsnare.on && ((step_vol = settings.seq[TRACK_SNARE][seq_step]) > 0))
	{
		dsnare.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dsnare.accent);
	}
	if (settings.dhihat.on && ((step_vol = settings.seq[TRACK_HIHAT][seq_step]) > 0))
	{
		dhihat.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dhihat.accent, step_vol);
	}
	if (settings.dride.on && ((step_vol = settings.seq[TRACK_RIDE][seq_step]) > 0))
	{
		dride.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dride.accent);
	}
	if (settings.dcrash.on && ((step_vol = settings.seq[TRACK_CRASH][seq_step]) > 0))
	{
		dcrash.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dcrash.accent);
	}
	if (settings.dclap.on && ((step_vol = settings.seq[TRACK_CLAP][seq_step]) > 0))
	{
		dclap.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dclap.accent);
	}
	if (settings.dtomhi.on && ((step_vol = settings.seq[TRACK_TOMHI][seq_step]) > 0))
	{
		dtomhi.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dtomhi.accent);
	}
	if (settings.dtomlo.on && ((step_vol = settings.seq[TRACK_TOMLO][seq_step]) > 0))
	{
		dtomlo.Trig(step_vol == DRUM_NORMAL ? 1.0f : settings.dtomlo.accent);
	}
}



// demo sequence
void SetupSeq()
{
	settings.seq_end = 32;
	settings.seq_tempo = 120;

	settings.seq[TRACK_BASS][0] = DRUM_NORMAL;
	settings.seq[TRACK_BASS][8] = DRUM_NORMAL;
	settings.seq[TRACK_BASS][10] = DRUM_NORMAL;
	settings.seq[TRACK_BASS][16] = DRUM_NORMAL;
	settings.seq[TRACK_BASS][24] = DRUM_NORMAL;
	settings.seq[TRACK_BASS][30] = DRUM_NORMAL;

	settings.seq[TRACK_SNARE][4] = DRUM_NORMAL;
	settings.seq[TRACK_CLAP][12] = DRUM_NORMAL;
	settings.seq[TRACK_SNARE][20] = DRUM_NORMAL;
	settings.seq[TRACK_CLAP][28] = DRUM_NORMAL;
	
	settings.seq[TRACK_HIHAT][0] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][2] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][4] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][6] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][8] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][10] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][12] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][14] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][16] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][18] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][22] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][22] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][24] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][26] = DRUM_ACCENT;
	settings.seq[TRACK_HIHAT][28] = DRUM_NORMAL;
	settings.seq[TRACK_HIHAT][30] = DRUM_ACCENT;

	settings.seq[TRACK_CRASH][0] = DRUM_NORMAL;
	
	settings.seq[TRACK_TOMHI][28] = DRUM_NORMAL;
	settings.seq[TRACK_TOMLO][30] = DRUM_NORMAL;

}


void SetupTracks()
{

	// bass
	dbass.Init(g_sample_rate, DTYPE_SYNTHETIC);
	settings.dbass.on = true;
	settings.dbass.pan = 0.5f;
	settings.dbass.level = 0.8f;
	settings.dbass.accent = 2.0f;
	settings.dbass.delay_delay = 0.0f;
	settings.dbass.delay_feedback = 0.0f;
	settings.dbass.reverb_level = 0.1f;
	settings.dbass.overdrive = 0.0f;
	delay_dbass.Init();
	delay_dbass.SetDelay(g_sample_rate * settings.dbass.delay_delay);
	overdrive_bass.Init();
	overdrive_bass.SetDrive(settings.dbass.overdrive);

	// snare
	dsnare.Init(g_sample_rate, DTYPE_OPD);
	settings.dsnare.on = true;
	settings.dsnare.pan = 0.5f;
	settings.dsnare.level = 0.7f;
	settings.dsnare.accent = 2.0f;
	settings.dsnare.delay_delay = 0.0f;
	settings.dsnare.delay_feedback = 0.0f;
	settings.dsnare.reverb_level = 0.1f;
	settings.dsnare.overdrive = 0.0f;
	delay_dsnare.Init();
	delay_dsnare.SetDelay(g_sample_rate * settings.dsnare.delay_delay);
	overdrive_snare.Init();
	overdrive_snare.SetDrive(settings.dsnare.overdrive);

	// hihat
	dhihat.Init(g_sample_rate, DTYPE_OPD);
	settings.dhihat.on = true;
	settings.dhihat.pan = 0.3f;
	settings.dhihat.level = 0.3f;
	settings.dhihat.accent = 2.0f;
	settings.dhihat.delay_delay = 0.0f;
	settings.dhihat.delay_feedback = 0.0f;
	settings.dhihat.reverb_level = 0.3f;
	delay_dhihat.Init();
	delay_dhihat.SetDelay(g_sample_rate * settings.dhihat.delay_delay);
	
	// ride
	dride.Init(g_sample_rate, 3600.0f, 0.3f, 0.3f, 0.4f, 0.4f, 0.6f, 0.1f);
	settings.dride.on = true;
	settings.dride.pan = 0.8f;
	settings.dride.level = 0.2f;
	settings.dride.accent = 2.0f;
	settings.dride.delay_delay = 0.0f;
	settings.dride.delay_feedback = 0.0f;
	settings.dride.reverb_level = 0.3f;
	delay_dride.Init();
	delay_dride.SetDelay(g_sample_rate * settings.dride.delay_delay);
	
	// crash
	dcrash.Init(g_sample_rate, 1000.0f, 0.3f, 0.3f, 0.4f, 1.2f, 0.3f, 0.3f);
	settings.dcrash.on = true;
	settings.dcrash.pan = 0.8f;
	settings.dcrash.level = 0.3f;
	settings.dcrash.accent = 2.0f;
	settings.dcrash.delay_delay = 0.0f;
	settings.dcrash.delay_feedback = 0.0f;
	settings.dcrash.reverb_level = 0.3f;
	delay_dcrash.Init();
	delay_dcrash.SetDelay(g_sample_rate * settings.dcrash.delay_delay);

	// clap
	dclap.Init(g_sample_rate, 1200.0f, 0.5f, 0.1f, 0.8f, 0.14f);
	settings.dclap.on = true;
	settings.dclap.pan = 0.5f;
	settings.dclap.level = 0.6f;
	settings.dclap.accent = 2.0f;
	settings.dclap.delay_delay = 0.3f;
	settings.dclap.delay_feedback = 0.1f;
	settings.dclap.reverb_level = 0.4f;
	delay_dclap.Init();
	delay_dclap.SetDelay(g_sample_rate * settings.dclap.delay_delay);

	// tom hi
	dtomhi.Init(g_sample_rate, 300.0f, 0.5f, 0.4f, 0.1f);
	settings.dtomhi.on = true;
	settings.dtomhi.pan = 0.7f;
	settings.dtomhi.level = 0.5f;
	settings.dtomhi.accent = 2.0f;
	settings.dtomhi.delay_delay = 0.3f;
	settings.dtomhi.delay_feedback = 0.0f;
	settings.dtomhi.reverb_level = 0.2f;
	delay_dtomhi.Init();
	delay_dtomhi.SetDelay(g_sample_rate * settings.dtomhi.delay_delay);
	
	// tom lo
	dtomlo.Init(g_sample_rate, 150.0f, 0.5f, 0.4f, 0.1f);
	settings.dtomlo.on = true;
	settings.dtomlo.pan = 0.85f;
	settings.dtomlo.level = 0.5f;
	settings.dtomlo.accent = 2.0f;
	settings.dtomlo.delay_delay = 0.3f;
	settings.dtomlo.delay_feedback = 0.0f;
	settings.dtomlo.reverb_level = 0.2f;
	delay_dtomlo.Init();
	delay_dtomlo.SetDelay(g_sample_rate * settings.dtomlo.delay_delay);

	// reverb
	settings.reverb_on = true;
	settings.reverb_feedback = 0.6f;
	settings.reverb_lpffreq = 8000.0f;
	settings.reverb_dry = 0.50f;
	settings.reverb_wet = 0.50f;

	reverb.Init(g_sample_rate);
	reverb.SetFeedback(settings.reverb_feedback);
	reverb.SetLpFreq(settings.reverb_lpffreq);
	
	// gain
	settings.gain_in = 1.0f;
	settings.gain_out = 1.0f;
}




void SetDelayDelay(uint8_t track)
{
	switch (track)
	{
	case TRACK_BASS:
		delay_dbass.SetDelay(g_sample_rate * settings.dbass.delay_delay);
		break;
	case TRACK_SNARE:
		delay_dsnare.SetDelay(g_sample_rate * settings.dsnare.delay_delay);
		break;
	case TRACK_HIHAT:
		delay_dhihat.SetDelay(g_sample_rate * settings.dhihat.delay_delay);
		break;
	case TRACK_CRASH:
		delay_dcrash.SetDelay(g_sample_rate * settings.dcrash.delay_delay);
		break;
	case TRACK_RIDE:
		delay_dride.SetDelay(g_sample_rate * settings.dride.delay_delay);
		break;
	case TRACK_CLAP:
		delay_dclap.SetDelay(g_sample_rate * settings.dclap.delay_delay);
		break;
	case TRACK_TOMHI:
		delay_dtomhi.SetDelay(g_sample_rate * settings.dtomhi.delay_delay);
		break;
	case TRACK_TOMLO:
		delay_dtomlo.SetDelay(g_sample_rate * settings.dtomlo.delay_delay);
		break;
	}
}

void SetReverb()
{
	reverb.SetLpFreq(settings.reverb_lpffreq);
	reverb.SetFeedback(settings.reverb_feedback);
}


// utility

// Flash handling - load and save
// 8MB of flash
// 4kB blocks
// assume our settings < 4kB, so put one patch per block
#define FLASH_BLOCK 4096

uint8_t DSY_QSPI_BSS qspi_buffer[FLASH_BLOCK * 16];

void FlashLoad(uint8_t aSlot)
{
    size_t size = sizeof(Settings);
    
	memcpy(&settings, &qspi_buffer[aSlot * FLASH_BLOCK], size);

	dbass.settings_ = settings.settings_dbass;
	dbass.Setup();
	dsnare.settings_ = settings.settings_dsnare;
	dsnare.Setup();
	dhihat.settings_ = settings.settings_dhihat;
	dhihat.Setup();
	dcrash.settings_ = settings.settings_dcrash;
	dcrash.Setup();
	dride.settings_ = settings.settings_dride;
	dride.Setup();
	dclap.settings_ = settings.settings_dclap;
	dclap.Setup();
	dtomhi.settings_ = settings.settings_dtomhi;
	dtomhi.Setup();
	dtomlo.settings_ = settings.settings_dtomlo;
	dtomlo.Setup();
}



void FlashSave(uint8_t aSlot)
{
	settings.settings_dbass = dbass.settings_;
	settings.settings_dsnare = dsnare.settings_;
	settings.settings_dhihat = dhihat.settings_;
	settings.settings_dcrash = dcrash.settings_;
	settings.settings_dride = dride.settings_;
	settings.settings_dclap = dclap.settings_;
	settings.settings_dtomhi = dtomhi.settings_;
	settings.settings_dtomlo = dtomlo.settings_;

	size_t start_address = (size_t)qspi_buffer;
    size_t size = sizeof(Settings);
	size_t slot_address = start_address + (aSlot * FLASH_BLOCK);

    hardware.qspi.Erase(slot_address, slot_address + size);
    hardware.qspi.Write(slot_address, size, (uint8_t *)&settings);

}



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





int main(void)
{
	// init hardware
	hardware.Configure();
	hardware.Init(true); // true = boost to 480MHz
	g_sample_rate = hardware.AudioSampleRate();
	g_callback_rate = hardware.AudioCallbackRate();

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
	
	// init AD inputs
	adc_config[AD_LCDBUTTON_INDEX].InitSingle(hardware.GetPin(AD_LCDBUTTON_PIN));
	adc_config[AD_CV0_INDEX].InitSingle(hardware.GetPin(AD_CV0_PIN));
	adc_config[AD_CV1_INDEX].InitSingle(hardware.GetPin(AD_CV1_PIN));
	adc_config[AD_GATE0_INDEX].InitSingle(hardware.GetPin(AD_GATE0_PIN));
	adc_config[AD_GATE1_INDEX].InitSingle(hardware.GetPin(AD_GATE1_PIN));
	adc_config[AD_POT0_INDEX].InitSingle(hardware.GetPin(AD_POT0_PIN));
	adc_config[AD_POT1_INDEX].InitSingle(hardware.GetPin(AD_POT1_PIN));
	hardware.adc.Init(adc_config, AD_MAX);
	hardware.adc.Start();

	// init boot/upload button
	Switch buttonUpload;
	buttonUpload.Init(hardware.GetPin(PIN_BUTTON_UPLOAD), 10);
	
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
	
	// tracks
	SetupTracks();

	// seq
	SetupSeq();
	tickIntStart();

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
		
		// read AD inputs
		g_ad_gate0 = hardware.adc.GetFloat(AD_GATE0_INDEX);
		g_ad_gate1 = hardware.adc.GetFloat(AD_GATE1_INDEX);
		g_ad_cv0 = hardware.adc.GetFloat(AD_CV0_INDEX);
		g_ad_cv1 = hardware.adc.GetFloat(AD_CV1_INDEX);
		g_ad_pot0 = hardware.adc.GetFloat(AD_POT0_INDEX);
		g_ad_pot1 = hardware.adc.GetFloat(AD_POT1_INDEX);

		// pot0 adjust tempo		
		if (g_pot0 == POT_TEMPO)
		{
			settings.seq_tempo = (int)(g_ad_pot0 * 999);
			tickIntCalc();
		}

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
