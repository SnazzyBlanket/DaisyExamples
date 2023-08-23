#include "daisy_seed.h"

#include "lcd_hd44780.h"

#define PIN_LCD_RS 10 // LCD: pin 8
#define PIN_LCD_EN 9 // LCD: pin 9
#define PIN_LCD_D4 11 // LCD: D4
#define PIN_LCD_D5 12 // LCD: D5
#define PIN_LCD_D6 13 // LCD: D6
#define PIN_LCD_D7 14 // LCD: D7	


#define PIN_CV0 22
#define PIN_CV1 23
#define PIN_CV2 24
#define PIN_CV3 25
#define PIN_GATE 28


using namespace daisy;


DaisySeed hardware;



// hardware
Lcd lcd;
AdcChannelConfig adcLCDButtons[5]; // 	Lcd buttons; ADC configuration


void AudioCallback(float* in, float* out, size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
		out[i] = in[i];
        out[i + 1] = in[i + 1];
	}
}

int main()
{
	// Configure and Initialize the Daisy Seed
	hardware.Configure();
	hardware.Init();

	// Configure pin as an ADC input
	adcLCDButtons[0].InitSingle(hardware.GetPin(PIN_CV0));
	adcLCDButtons[1].InitSingle(hardware.GetPin(PIN_CV1));
	adcLCDButtons[2].InitSingle(hardware.GetPin(PIN_CV2));
	adcLCDButtons[3].InitSingle(hardware.GetPin(PIN_CV3));
	adcLCDButtons[4].InitSingle(hardware.GetPin(PIN_GATE));
	hardware.adc.Init(adcLCDButtons, 5);
	// Start reading values
	hardware.adc.Start();

	// Lcd
	lcd.init(&hardware, false, true, PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

	hardware.StartAudio(AudioCallback);

	// Loop forever
	for(;;)
	{
		float adcCV0 = hardware.adc.GetFloat(0);
		float adcCV1 = hardware.adc.GetFloat(1);
		float adcCV2 = hardware.adc.GetFloat(2);
		float adcCV3 = hardware.adc.GetFloat(3);
		float adcGate = hardware.adc.GetFloat(4);

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.printInt((int)(adcCV0*100));

		lcd.setCursor(0, 4);
		lcd.printInt((int)(adcCV1*100));

		lcd.setCursor(0, 8);
		lcd.printInt((int)(adcCV2*100));

		lcd.setCursor(0, 12);
		lcd.printInt((int)(adcCV3*100));

		lcd.setCursor(1, 0);
		lcd.printInt((int)(adcGate*100));

		System::Delay(1000);
	}
}
