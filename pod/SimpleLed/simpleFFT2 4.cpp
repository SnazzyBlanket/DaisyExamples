#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

const size_t FFT_SIZE = 2048;
const size_t NUM_CHANNELS = 64;

DaisyPod pod;
AnalogMic mic;
Fft fft(FFT_SIZE);
Svf filterbank[NUM_CHANNELS];
Oscillator modulator;
float mod_scale = 0.5f;

int main()
{
    pod.Init();
    mic.Init();
    fft.Init();
    for (int i = 0; i < NUM_CHANNELS; i++) {
        filterbank[i].Init(44100);
    }
    modulator.Init(44100);

    while(1)
    {
        float in = mic.Process();
        float mod = modulator.Process();

        fft.Process(in);
        for (int i = 0; i < NUM_CHANNELS; i++) {
            filterbank[i].Process(fft.GetBin(i), 0, 0);
            float scaled_mod = mod_scale * mod * filterbank[i].Process(mod, 0);
            fft.SetBin(i, scaled_mod);
        }
        fft.ProcessInv();
        float out = fft.Get(0);

        pod.led1.Write(in);
        pod.led2.Write(mod);
        pod.led3.Write(out);
    }
}
