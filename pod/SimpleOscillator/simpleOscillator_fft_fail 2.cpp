
#include "daisy_pod.h"
#include "daisysp.h"

#include "arm_math.h"
#include "arm_const_structs.h"
#define NUM_WAVEFORMS 4


using namespace daisy;
using namespace daisysp;

DaisyPod   hw;
Oscillator osc;
Parameter  p_freq;




#define FFT_SIZE 2048 
#define FFT_BUF_SIZE (2*FFT_SIZE) 
float fft_buf[FFT_BUF_SIZE];
float output[FFT_SIZE];

// global, so do_fft() knows when to run
int fft_buf_index = 0;

uint8_t waveforms[NUM_WAVEFORMS] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_TRI,
    Oscillator::WAVE_POLYBLEP_SAW,
    Oscillator::WAVE_POLYBLEP_SQUARE,
};

static float freq;
float        sig,fftFreq;
static int   waveform, octave;


float do_fft(float *input, float *output)
{

  float32_t maxValue;
  uint32_t index = 0;

  int Fmax = hw.AudioSampleRate() / 2;  // 24000
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


static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    hw.ProcessDigitalControls();

    waveform += hw.encoder.Increment();
    waveform = DSY_CLAMP(waveform, 0, NUM_WAVEFORMS);
    osc.SetWaveform(waveforms[waveform]);

    if(hw.button2.RisingEdge())
        octave++;
    if(hw.button1.RisingEdge())
        octave--;
    octave = DSY_CLAMP(octave, 0, 4);

    // convert MIDI to frequency and multiply by octave size
    freq = mtof(p_freq.Process() + (octave * 12));
    osc.SetFreq(freq);

    // Audio Loop
       // Audio Loop
    for(size_t i = 0; i < size; i += 2)
    {
          // Process
        sig        = osc.Process();
        out[i]     = sig;
        out[i + 1] = sig;}

     // Store in fft_buf for FFT analysis
     /*
        if (fft_buf_index < FFT_BUF_SIZE) {
            fft_buf[fft_buf_index++] = sig;
            fft_buf[fft_buf_index++] = 0.f;
        }
    

    // Perform FFT when fft_buf is full
    if (fft_buf_index == FFT_BUF_SIZE) {
        float f = do_fft(fft_buf, output);
        fftFreq = f;
        fft_buf_index = 0;
    }*/

        
    }





void InitSynth(float samplerate)
{
    // Init freq Parameter to knob1 using MIDI note numbers
    // min 10, max 127, curve linear
    p_freq.Init(hw.knob1, 0, 127, Parameter::LINEAR);

    osc.Init(samplerate);
    osc.SetAmp(1.f);

    waveform = 0;
    octave   = 0;
}

int main(void)
{
float samplerate;


  

    // Init everything
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    InitSynth(samplerate);
 


 

    // Initialization code...
    
    // Test the do_fft function
    const float sample_rate = hw.AudioSampleRate();
    const float frequency = 22.0f; // A4 note frequency (440 Hz)
    const float amplitude = 0.5f;   // Set a suitable amplitude for the sine wave

    // Generate the known input signal (sine wave at 440 Hz)
    for (size_t i = 0; i < FFT_SIZE; i++)
    {
        float t = static_cast<float>(i) / sample_rate;
        float input_signal = amplitude * sinf(2.0f * M_PI * frequency * t);
        fft_buf[i * 2] = input_signal; // Real part of the complex input
        fft_buf[i * 2 + 1] = 0.0f;     // Imaginary part is set to 0 (not used in real FFT)
    }

    // Call the do_fft function to analyze the input signal
    float analyzed_frequency = do_fft(fft_buf, output);

    // Display the analyzed frequency using LEDs
    const float expected_frequency = 22.0f;
    const float tolerance = 5.0f; // You can adjust the tolerance as needed

    // Check if the analyzed frequency is close to the expected frequency
    if (fabs(analyzed_frequency - expected_frequency) <= tolerance)
    {
        // Turn on a specific LED to indicate success
        
        hw.led2.Set(34, 10, 33);

   
    }
    else
    {
        // Turn on a different LED to indicate failure
        hw.led1.Set(10,33,22);
   
    }
  
  
       hw.StartAdc();
    hw.StartAudio(AudioCallback);
    // Rest 
    while (1)

    {
      hw.UpdateLeds();    /* code */
    }
    


}
