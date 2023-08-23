#include "daisysp.h"
#include "daisy_patch.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;
Oscillator osc;

Comb       comb;
Oscillator lfo;
CrossFade  fader;
Parameter  freqctrl, wavectrl, ampctrl, finectrl;

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
      


        float  sig, freq, amp;
        float outp;
float a,b,c,d,e,f,g,h,z,j,k,y;
uint32_t junk;

    size_t wave;
    size_t num = 4;

    patch.ProcessAnalogControls();

    for(size_t i = 0; i < size; i++)
    {
        // Read Knobs
        freq = mtof(freqctrl.Process() + finectrl.Process());
        wave = wavectrl.Process();
        amp  = ampctrl.Process();
        // Set osc params
        osc.SetFreq(freq);
        osc.SetWaveform(wave);
        osc.SetAmp(amp);
        //.Process
        a = osc.Process();
          
          
           y = 0.5 * (a+k);
               
              // out = 1+y;
 
       // Assign Synthesized Waveform to all four outputs.
        for(size_t chn = 0; chn < 2; chn++)
        { 
            out[chn][i] = 1+y;
            
             k=j;
             junk =(int)k>>6;
         
 j=z;
 z=h;
h=g;
g=f;
f=e;
e=d;
d=c;
c=b;
b=a;
a=junk;
             //BASED ON THE INTERFACE WITH C BOOK 4.4ms delay
        
 }  
 
   }
}

    


int main(void)
{float samplerate;
    patch.Init(); // Initialize hardware (daisy seed, and patch)
patch.StartAdc();
    int   num_waves = Oscillator::WAVE_LAST - 1;

    samplerate = patch.AudioSampleRate();

    osc.Init(samplerate); // Init oscillator

    freqctrl.Init(
        patch.controls[patch.CTRL_1], 10.0, 110.0f, Parameter::LINEAR);
    finectrl.Init(patch.controls[patch.CTRL_2], 0.f, 7.f, Parameter::LINEAR);
    wavectrl.Init(
        patch.controls[patch.CTRL_3], 0.0, num_waves, Parameter::LINEAR);
    ampctrl.Init(patch.controls[patch.CTRL_4], 0.0, 0.5f, Parameter::LINEAR);
       patch.StartAudio(AudioCallback);



 
 
}



