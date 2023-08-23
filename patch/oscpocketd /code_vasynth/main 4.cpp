/*
  
Project: OscPocketD/VASynth
Description: polyphonic MIDI Virtual Analog synthsizer for OscPcoketD/Base2 (Daisy Seed)
Author: Staffan Melin, staffan.melin@oscillator.se
License: GNU General Public License v3.0
Version: 202109
Project site: http://www.oscillator.se/opensource

*/

//#define OPD_LOGG // start serial over USB Logger class
//#define OPD_MEASURE // start serial over USB Logger class
#define OPD_BASE_MIDI // version 2 with MIDI, other LCD pins, no CV1 in, no Gate0/1 out

#include "daisy_seed.h"
#include "daisysp.h"

#include "stm32h7xx_hal.h" // for HAL_NVIC_SystemReset();
//extern "C" void HAL_NVIC_SystemReset();
#include "core_cm7.h"
#include "dev/lcd_hd44780.h"

#include "main.h"
#include "ui.h"
#include "vasynth.h"

using namespace daisy;
using namespace daisysp;

// globals
DaisySeed hardware;
MidiUartHandler midi;
float sysSampleRate;
float sysCallbackRate;

// UI
bool uiRedraw = true;

// hardware
LcdHD44780 lcd;
AdcChannelConfig adcConfig[AD_MAX];

// sound + fx
VASynth vasynth;
OpdModSources modSources;
uint8_t gPlay = PLAY_ON;



// audio callback

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
	float voice_left, voice_right;

	#ifdef OPD_MEASURE
	// measure - start
	DWT->CYCCNT = 0;
	#endif
	
	// audio

	for (size_t n = 0; n < size; n += 2)
	{	
		if (gPlay == PLAY_ON)
		{
			// voices
			
			vasynth.Process(&voice_left, &voice_right);
			
			if (vasynth.input_channel_ == INPUT_CHANNEL_NONE)
			{
				out[n] = voice_left;
				out[n + 1] = voice_right;
			} else {
				out[n] = voice_left + in[n];
				out[n + 1] = voice_right + in[n + 1];		
			}
		} else {
			out[n] = 0;
			out[n + 1] = 0;		
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


#ifdef OPD_BASE_MIDI
// midi handler
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
        	hardware.SetLed(true);
            NoteOnEvent p = m.AsNoteOn();
            if ((vasynth.midi_channel_ == MIDI_CHANNEL_ALL) || (p.channel == vasynth.midi_channel_))
			{
				vasynth.NoteOn(p.note, p.velocity);				
			}
	        break;
        }
        case NoteOff:
        {
        	hardware.SetLed(false);
            NoteOnEvent p = m.AsNoteOn();
            if ((vasynth.midi_channel_ == MIDI_CHANNEL_ALL) || (p.channel == vasynth.midi_channel_))
			{
				vasynth.NoteOff(p.note);
			}			
	        break;
        }        
        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            switch(p.control_number)
            {
                case 74: // cutoff, 0-127 -> frequency
                	vasynth.filter_cutoff_ = ((float)p.value / 127.0f) * FILTER_CUTOFF_MAX;
                    vasynth.SetFilter();
                    break;
                case 71: // res, 0-127
                	vasynth.filter_res_ = ((float)p.value / 127.0f);
                    vasynth.SetFilter();
                    break;
                default: break;
            }
            break;
        }
        default: break;
    }
}
#endif



uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



// Convert MIDI note number to CV
// DAC outputs from 0 to 3v3 (3300 mV's)
// We have 1V/Oct, so 3+ octaves on a 3v3 system
// The DAC wants values from 0 to 4095
uint16_t mtocv(uint8_t midiPitch)
{
	float voltsPerNote = 0.0833f; // 1/12 V
	float mV; // from 0 to 0xFFF (4096);

	mV = 1000 * (midiPitch * voltsPerNote);

	return (map(mV, 0, 3300, 0, 4095));
}



float mtoval(uint8_t midiPitch)
{
	float voltsPerNote = 0.0833f; // 1/12 V

	return ((midiPitch * voltsPerNote) / 3.3f);
}



int main(void)
{
	// init hardware
	hardware.Configure();
	hardware.Init(true); // true = boost to 480MHz
	sysSampleRate = hardware.AudioSampleRate();
	sysCallbackRate = hardware.AudioCallbackRate();

	// setup incl default values
	vasynth.First();

	// init AD inputs
	adcConfig[AD_LCDBUTTON_INDEX].InitSingle(hardware.GetPin(AD_LCDBUTTON_PIN));
	adcConfig[AD_POT0_INDEX].InitSingle(hardware.GetPin(AD_POT0_PIN));
	adcConfig[AD_POT1_INDEX].InitSingle(hardware.GetPin(AD_POT1_PIN));
	hardware.adc.Init(adcConfig, AD_MAX);
	hardware.adc.Start();

	// init boot/upload button
	Switch buttonUpload;
	buttonUpload.Init(hardware.GetPin(PIN_BUTTON_UPLOAD), 10);
	
	#ifdef OPD_BASE_MIDI
	MidiUartHandler::Config midi_config;
	midi.Init(midi_config);
	#endif

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

		#ifdef OPD_BASE_MIDI
        // handle MIDI Events
        midi.Listen();
        while(midi.HasEvents())
        {
            HandleMidiMessage(midi.PopEvent());
        }
		#endif

		// read analog inputs
		for (uint8_t i = AD_POT0_INDEX; i <= CV_NUMBER; i++)
		{
			modSources.cvValue[i - AD_POT0_INDEX] = hardware.adc.GetFloat(i);
		}
		if (vasynth.pot0_target_ == POT_TARGET_FILTER)
		{
			vasynth.filter_cutoff_ = modSources.cvValue[0] * FILTER_CUTOFF_MAX;
		}

		if (vasynth.pot1_target_ == POT_TARGET_FILTER)
		{
			vasynth.filter_res_ = modSources.cvValue[1];
			if ((vasynth.filter_res_ > 0.0f) && (vasynth.filter_res_ < 1.0f))
			{
				for (uint8_t j = 0; j < vasynth.voices_; j++)
				{
					vasynth.svf_[j].SetRes(vasynth.filter_res_);
				}
			}
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
