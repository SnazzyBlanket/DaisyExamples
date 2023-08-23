#include "daisy_patch.h"
#include "daisysp.h"
#include <string>

#define NUM_VOICES 2
#define MAX_DELAY ((size_t)(1.0f * 48000.0f))
#define FFT_SIZE 2048 
#define FFT_BUF_SIZE (2*FFT_SIZE) 
float fft_buf[FFT_BUF_SIZE];
float output[FFT_SIZE];

// global, so do_fft() knows when to run
int fft_buf_index = 0;

using namespace daisy;
using namespace daisysp;

// Hardware
DaisyPatch hw;


#include "arm_math.h"
#include "arm_const_structs.h"



// Moved fft_buf and output arrays to SDRAM using DSY_SDRAM_BSS
DSY_SDRAM_BSS float fft_buf[FFT_BUF_SIZE];
DSY_SDRAM_BSS float output[FFT_SIZE];

// Helper function to map a value from one range to another
float map_range(float value, float in_min, float in_max, float out_min, float out_max)
{
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// Synthesis
PolyPluck<NUM_VOICES> synth;
// 10 second delay line on the external SDRAM
DSY_SDRAM_BSS DelayLine<float, MAX_DELAY> delay;
ReverbSc                                  verb;

// Persistent filtered Value for smooth delay time changes.
float smooth_time;

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

// Function to adjust delay time based on frequency
// Function to adjust delay time based on frequency
void AdjustDelayTime(float frequency)
{
    // Map the frequency to a delay time range
    // You can adjust the min and max delay times based on your requirements
    float min_freq = 0.0f;
    float max_freq = hw.AudioSampleRate() * 0.5f;
    float min_deltime = 0.001f; // Minimum delay time in seconds (1 ms)
    float max_deltime = 0.2f;   // Maximum delay time in seconds (200 ms)

    // Map the frequency to the delay time range
    float mapped_deltime = map_range(frequency, min_freq, max_freq, min_deltime, max_deltime);

    // Set the delay time
    smooth_time = mapped_deltime;
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sig, delsig;           // Mono Audio Vars
    float trig, nn, delfb,decay;       // Pluck Vars
    float dry, send, wetl, wetr; // Effects Vars
    float samplerate;

    // Assign Output Buffers
    float *out_left, *out_right;
    out_left = out[0];
    out_right = out[1];

    samplerate = hw.AudioSampleRate();
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    // Handle Triggering the Plucks
    trig = 0.0f;
    if (hw.encoder.RisingEdge() || hw.gate_input[DaisyPatch::GATE_IN_1].Trig())
        trig = 1.0f;

    // Set MIDI Note for new Pluck notes.
    nn = 24.0f + hw.GetKnobValue(DaisyPatch::CTRL_1) * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones

    // Read knobs for decay;
    decay = 0.5f + (hw.GetKnobValue(DaisyPatch::CTRL_2) * 0.5f);
    synth.SetDecay(decay);

    // Synthesis and FFT.
    for (size_t i = 0; i < size; i++)
    {
        // Synthesize Plucks
        sig = synth.Process(trig, nn);

        // Perform FFT
        fft_buf[i] = sig;
        fft_buf[i + 1] = 0.f; // Imaginary part is set to 0 (not used in real FFT)

        // Handle Delay
        delsig = delay.Read();
        delay.Write(sig + (delsig * delfb));

        // Create Reverb Send
        dry = sig + delsig;
        send = dry * 0.6f;
        verb.Process(send, send, &wetl, &wetr);

        // Output
        out_left[i] = dry + wetl;
        out_right[i] = dry + wetr;
    }

    // Check if the buffer is full
    static int fft_buf_index = 0;
    fft_buf_index += size;
    if (fft_buf_index >= FFT_BUF_SIZE)
    {
        // Perform FFT
        do_fft(fft_buf, output);

        // Find the dominant frequency from the FFT output
        float max_magnitude = 0.0f;
        int max_bin_index = -1;
        for (int i = 0; i < FFT_SIZE / 2; i++)
        {
            if (output[i] > max_magnitude)
            {
                max_magnitude = output[i];
                max_bin_index = i;
            }
        }

        // Calculate the frequency corresponding to the max bin
        float bin_freq = static_cast<float>(max_bin_index) * (hw.AudioSampleRate() / FFT_SIZE);

        // Adjust the delay time based on the dominant frequency
        AdjustDelayTime(bin_freq);

        // Reset the buffer index for the next FFT computation
        fft_buf_index = 0;
    }
}



int main(void)
{
    // Init everything.
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();

    //briefly display the module name
    std::string str = "Pluck Echo";
    char *cstr = &str[0];
    hw.display.WriteString(cstr, Font_7x10, true);
    hw.display.Update();
    hw.DelayMs(1000);

    synth.Init(samplerate);

    delay.Init();
    delay.SetDelay(samplerate * 0.8f); // half second delay

    verb.Init(samplerate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);

    // Start the ADC and Audio Peripherals on the Hardware
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    for (;;)
    {
        hw.DisplayControls(false);
    }
}
