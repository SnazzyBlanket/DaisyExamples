#include "daisy_pod.h"
#include "daisysp.h"
#include <arm_math.h>
#include <arm_const_structs.h>
#include <arm_common_tables.h>





#define FFT_SIZE 1024


using namespace daisy;
using namespace daisysp;

DaisyPod   hw;
Oscillator osc;
Parameter  p_freq;

static float freq;
float        sig;
static int   waveform, octave;

// FFT variables
static float              fft_in[FFT_SIZE];
static float              fft_out[FFT_SIZE];
static float              fft_mag[FFT_SIZE / 2];
arm_cfft_radix4_instance_f32 fft_inst;




//static arm_rfft_fast_instance_f32  fft_inst = { FFT_SIZE, (float32_t*)twiddleCoef_4096, 0, 1 };
//static float32_t          twiddleCoef[FFT_SIZE];


static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    hw.ProcessDigitalControls();

    waveform += hw.encoder.Increment();
    waveform = DSY_CLAMP(waveform, 0, 3);
  

    if(hw.button2.RisingEdge())
        octave++;
    if(hw.button1.RisingEdge())
        octave--;
    octave = DSY_CLAMP(octave, 0, 4);

    // convert MIDI to frequency and multiply by octave size
    freq = mtof(p_freq.Process() + (octave * 12));
    osc.SetFreq(freq);

   // Audio Loop
for(size_t i = 0; i < size; i += 2)
{
    // Process
    sig        = osc.Process();
    out[i]     = sig;
    out[i + 1] = sig;

    // add signal to FFT buffer
    fft_in[i/2] = sig;
}

// run FFT

arm_cfft_radix4_f32(&fft_inst, fft_in);

// compute magnitude
arm_cmplx_mag_f32(fft_in, fft_mag, FFT_SIZE / 2);

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

    // init FFT
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
}
int main(void)
{
    float samplerate;

    // Init everything
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    InitSynth(samplerate);

    // start callbacks
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1) {}
}
