/*
DAC
https://forum.electro-smith.com/t/dac-output-on-seed/612/2
https://github.com/electro-smith/libDaisy/search?q=DacHandle
https://forum.electro-smith.com/t/cant-get-value-from-patch-controls-x-process/1545
*/

#include "daisy_seed.h"

#include "dev/lcd_hd44780.h"

#include "stm32h7xx_hal.h" // for HAL_NVIC_SystemReset();
//extern "C" void HAL_NVIC_SystemReset();
#include "core_cm7.h"



// Lcd

#define PIN_LCD_RS 10 // LCD: pin 8
#define PIN_LCD_EN 9 // LCD: pin 9
#define PIN_LCD_D4 11 // LCD: D4
#define PIN_LCD_D5 12 // LCD: D5
#define PIN_LCD_D6 13 // LCD: D6
#define PIN_LCD_D7 14 // LCD: D7	

#define PIN_LCD_BUTTON 17

#define PIN_CV_0_IN 18
#define PIN_CV_1_IN 19
#define PIN_GATE_0_IN 20
#define PIN_GATE_1_IN 21
#define PIN_CV_0_OUT 22
#define PIN_CV_1_OUT 23
#define PIN_GATE_0_OUT 24
#define PIN_GATE_1_OUT 25


#define PIN_BUTTON_UPLOAD 26


using namespace daisy;


DaisySeed hardware;



// hardware

LcdHD44780 lcd;

// in
AdcChannelConfig adcInputs[4]; // inputs
float adcValue[4]; // read values: CV0, CV1, Gate0, Gate1
// out
dsy_gpio gateOut0, gateOut1;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    for (size_t i = 0; i < size; i += 2)
    {
		out[i] = in[i];
        out[i + 1] = in[i + 1];
	}
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


int main()
{
	// Configure and Initialize the Daisy Seed
	hardware.Configure();
	hardware.Init();

	// Configure and initialize button
	Switch buttonUpload;
	buttonUpload.Init(hardware.GetPin(PIN_BUTTON_UPLOAD), 10);

	// init DACs
	DacHandle::Config cfg;
	cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
	cfg.buff_state = DacHandle::BufferState::ENABLED;
	cfg.mode       = DacHandle::Mode::POLLING;
	cfg.chn        = DacHandle::Channel::BOTH;
    hardware.dac.Init(cfg);
    hardware.dac.WriteValue(DacHandle::Channel::BOTH, 0);


    // define
	gateOut0.pin  = hardware.GetPin(PIN_GATE_0_OUT);
	gateOut0.mode = DSY_GPIO_MODE_OUTPUT_PP;
	gateOut0.pull = DSY_GPIO_NOPULL;
	dsy_gpio_init(&gateOut0);
	gateOut1.pin  = hardware.GetPin(PIN_GATE_1_OUT);
	gateOut1.mode = DSY_GPIO_MODE_OUTPUT_PP;
	gateOut1.pull = DSY_GPIO_NOPULL;
	dsy_gpio_init(&gateOut1);
	// write/set
	// dsy_gpio_write(&gateOut0, true);  // set high/low true/false 1/0


    
	// configure ADCs
	adcInputs[0].InitSingle(hardware.GetPin(PIN_CV_0_IN));
	adcInputs[1].InitSingle(hardware.GetPin(PIN_CV_1_IN));
	adcInputs[2].InitSingle(hardware.GetPin(PIN_GATE_0_IN));
	adcInputs[3].InitSingle(hardware.GetPin(PIN_GATE_1_IN));
	hardware.adc.Init(adcInputs, 4);
	// Start reading values
	hardware.adc.Start();

	// Lcd
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


	hardware.StartAudio(AudioCallback);

	int outCnt = 0;

	// Loop forever
	for(;;)
	{
		lcd.Clear();

		for (int i = 0; i < 4; i++)
		{
			adcValue[i] = hardware.adc.GetFloat(i);
			lcd.SetCursor(0, i * 4);
			lcd.PrintInt((int)(adcValue[i] * 100));			
		}

		if (outCnt > 3500)
		{
			outCnt = 0;
		} else {
			outCnt += 250;
		}
		//dsy_gpio_write(&gateOut0, outCnt);
		//dsy_gpio_write(&gateOut1, outCnt);
		hardware.dac.WriteValue(DacHandle::Channel::ONE, outCnt); // CV0
		// hardware.dac.WriteValue(DacHandle::Channel::ONE, 2000); // CV1


		// reset to upload
		buttonUpload.Debounce();
		if (buttonUpload.Pressed())
		{
			RebootToBootloader();
		}

		System::Delay(500);
	}
}
