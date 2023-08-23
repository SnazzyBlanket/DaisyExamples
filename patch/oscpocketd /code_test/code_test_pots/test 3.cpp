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
#define PIN_POT_0 15
#define PIN_POT_1 16


#define PIN_BUTTON_UPLOAD 26


using namespace daisy;


DaisySeed hardware;



// hardware

LcdHD44780 lcd;

AdcChannelConfig adcButtons[2]; // pots


void AudioCallback(float* in, float* out, size_t size)
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

	// Configure pin as an ADC input
	adcButtons[0].InitSingle(hardware.GetPin(PIN_POT_0));
	adcButtons[1].InitSingle(hardware.GetPin(PIN_POT_1));
	hardware.adc.Init(adcButtons, 2);
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

	// Loop forever
	for(;;)
	{
		lcd.Clear();

		float adcButton = hardware.adc.GetFloat(0);
		lcd.SetCursor(0, 0);
		lcd.PrintInt((int)(adcButton*100));

		adcButton = hardware.adc.GetFloat(1);
		lcd.SetCursor(1, 0);
		lcd.PrintInt((int)(adcButton*100));

		// reset to upload
		buttonUpload.Debounce();
		if (buttonUpload.Pressed())
		{
			RebootToBootloader();
		}

		System::Delay(100);
	}
}
