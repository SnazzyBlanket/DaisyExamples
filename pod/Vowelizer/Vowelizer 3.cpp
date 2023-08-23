//Kludged together by Asa Patterson from existing ElectroSmith examples.
//
//Two parallel state variable bandpass filters, each with different frequency
//ranges to simulate human vowel sounds. Best results are found by inputting a
//140Hz sawtooth wave.

#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
static DaisyPod pod;


void UpdateControls();
Svf filt1;
Svf filt2;
Parameter p_formant1, p_formant2, lfo1freq, lfoAMP;
Oscillator osc,lfo1,lfo2;

float formant1, formant2;

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)

{ float sig;

    float band1, band2, notch1, notch2, high1, high2, low1, low2,peak1, peak2;
      //add switch case for buttons
      //if button1 = rising edge, mod1= such and such 
      //if button 2 = rising edge then mod 2 =
      //if button 1 and button 2 are both held together then do ...


      //also add encoder states
      //so divide encoder into 6 slices and assign a diff set of outputs and mods 

    formant1 = p_formant1.Process();
    formant2 = p_formant2.Process();
  uint16_t m1= lfo1.Process();
uint16_t lBit = m1 >> 4;
 
  uint16_t m2= lfo2.Process();
  uint16_t jbit = m2 << 2;
  uint16_t nbit = jbit >> (4);

  
 // int randomValue = rand() % 50;
            uint16_t newBit = lBit >> (4);
           
    float mod1 = formant1+lBit;
     float mod2 = formant2+newBit;
   band1 = filt1.Band();
        band2 = filt2.Band();
        notch2 = filt2.Notch();
        low1=filt1.Low();
        peak2=filt2.Peak();
     
   
    
    // sig = channel(getleft);
    for(size_t i = 0; i < size; i++)//OUTER LOOP
    {   

               filt1.SetFreq(mod1);
    filt2.SetFreq(mod2);
   
    {     for(int chn = 0; chn < 2; chn++)
        {
            float input = in[chn][i];
             sig =input;
                filt1.Process(sig);
        filt2.Process(sig);
        filt2.SetRes(low1);
  
       
     out[chn][i] = (band1+band2);
        
    
        
      
       
          

    }
                          }
    }
}

int main(void){   
     float sample_rate;
    float rando= (rand() % 4);
    // initialize pod hardware and oscillator daisysp module
    pod.Init();
    sample_rate = pod.AudioSampleRate();
        //Assign formants to knobs.
    //Formant 1 has range 200Hz - 1000Hz
    //Formant 2 has range 500Hz - 2500Hz
    
    p_formant1.Init(pod.knob1, 200, 1000, Parameter::LOGARITHMIC);
    p_formant2.Init(pod.knob2, 500, 2500, Parameter::LOGARITHMIC);
 // lfo1freq.Init(pod.knob2, 500, 2500, Parameter::CUBE);
   //  lfoAMP.Init(pod.knob2, 500, 2500, Parameter::EXPONENTIAL);

    // Initialize Filters, and set parameters.
    filt1.Init(sample_rate);
    lfo1.Init(sample_rate);
     lfo2.Init(sample_rate);
    filt1.SetRes(0.85);
    filt1.SetDrive(0.94);
    lfo1.SetFreq(0.25f);
        lfo1.SetAmp(.81);
          lfo2.SetFreq(0.05f);
        lfo2.SetAmp(.85);
    
    
    filt2.Init(sample_rate);
    filt2.SetRes(0.85);
    filt2.SetDrive(0.92);

    

    
    // start callback
    pod.StartAdc();
    pod.StartAudio(AudioCallback);
    while(1) {}
}


void Controls()
{
    pod.ProcessAnalogControls();
    pod.ProcessDigitalControls();

}


