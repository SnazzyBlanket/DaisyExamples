// Using FFT size of 2048 needs 16K of sample buffer, and only gets a resolution of
// 47Hz.
#include "daisy_patch.h"
#include "daisysp.h"

#include <string.h>
#include "arm_math.h"
#include "arm_const_structs.h"


using namespace daisy;
using namespace daisysp;

DaisyPatch hw;

// OLED is 64x32

void UpdateOled(float fl)
{
    hw.display.Fill(false);

    std::string str = std::to_string(static_cast<int>(fl));
    char *cstr = &str[0];
    hw.display.SetCursor(0, 0);
		// Font_16x26 4 digits
		// Font_11x18 5 digits BEST for this
		// Font_6x8   10 digits
    hw.display.WriteString(cstr, Font_11x18, true);

    hw.display.Update();
}

#define FFT_SIZE 2048 
#define FFT_BUF_SIZE (2*FFT_SIZE) 
float fft_buf[FFT_BUF_SIZE];
float output[FFT_SIZE];

// global, so do_fft() knows when to run
int fft_buf_index = 0;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
		if (fft_buf_index < FFT_BUF_SIZE) {
			fft_buf[fft_buf_index++] = in[0][i];
			fft_buf[fft_buf_index++] = 0.f;
		}
			
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
    }
}


float do_fft(float *input, float *output)
{

  float32_t maxValue;
  uint32_t index = 0;

  int Fmax = hw.AudioSampleRate() / 2;	// 24000
  uint32_t Nbins = FFT_SIZE / 2;				

  /* Process the data through the CFFT/CIFFT module */
	/* this must match FFT_SIZE, and 2048 is as high as Daisy can fit */
  arm_cfft_f32(&arm_cfft_sR_f32_len2048, input, 0 /*ifftFlag*/, 1/*doBitReverse*/);

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
  arm_cmplx_mag_f32(input, output, FFT_SIZE /*fftSize*/);

  /* Calculates maxValue and returns corresponding BIN value */
  arm_max_f32(output, FFT_SIZE /*fftSize*/, &maxValue, &index);

/*
	to calculate the freq of the selected bin
    N (Bins) = FFT Size/2
    FR = Fmax/N(Bins)
*/

	// error test
	if (index >= Nbins)	
		return -1.f;

	// good result
	return (float) index * ((float)Fmax / Nbins);
}


int main(void)
{
    hw.Init();

	hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while (1)
    {
		float f;
		static float old_f = -1.f;

		if (fft_buf_index == FFT_BUF_SIZE) {
			// do FFT
			f = do_fft(fft_buf, output);
			// start collecting samples again
			fft_buf_index = 0;
		} else {
			continue;
		}

		// if frequency value has changed, update display
		if ((f > 0.f) && (f != old_f)) { 
        	UpdateOled(f);
			old_f = f;
		}
    }
}



