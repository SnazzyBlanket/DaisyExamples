#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "ddrum.h"
#include "tdrum.h"

using namespace daisy;
using namespace daisysp;

// globals
extern DaisySeed hardware;



void DDrum::Init(float aSampleRate, float aFreq, float aAmp, float aDecay, float aMin)
{
	settings_.sample_rate = aSampleRate;
	settings_.vol = 1.0f;
	
	// common
	settings_.freq = aFreq;
	settings_.amp = aAmp;
	settings_.decay = aDecay;
	settings_.min = aMin;
	
	
	// analog
	o_tdrum_.Init(settings_.sample_rate);

	Setup();
}



void DDrum::Setup()
{
	o_tdrum_.SetFreq(settings_.freq);
	o_tdrum_.SetAmp(settings_.amp);
	o_tdrum_.SetDecay(settings_.decay);
	o_tdrum_.SetMin(settings_.min);
}



void DDrum::Trig(float vol)
{
	settings_.vol = vol;
	
	o_tdrum_.Trig();
}



float DDrum::Process()
{
	float voice_out;

	voice_out = o_tdrum_.Process(false);

	return (voice_out * settings_.vol);
}


void DDrum::SetFreq()
{
	o_tdrum_.SetFreq(settings_.freq);
}

void DDrum::SetDecay()
{
	o_tdrum_.SetDecay(settings_.decay);
}

void DDrum::SetAmp()
{
	o_tdrum_.SetAmp(settings_.amp);
}

void DDrum::SetMin()
{
	o_tdrum_.SetMin(settings_.min);
}

