#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "tcymbal.h"
#include "dcymbal.h"

using namespace daisy;
using namespace daisysp;

// globals
extern DaisySeed hardware;



void DCymbal::Init(float aSampleRate, float aFreq, float aRes, float aDrive, float aAmp, float aDecay, float aMin, float aMix)
{
	settings_.sample_rate = aSampleRate;
	settings_.vol = 1.0f;
		
	// common
	settings_.freq = aFreq;
	settings_.res = aRes;
	settings_.drive = aDrive;
	settings_.amp = aAmp;
	settings_.decay = aDecay;
	settings_.min = aMin;
	settings_.mix = aMix;
	
	// analog
	o_tcymbal_.Init(settings_.sample_rate);

	Setup();
}



void DCymbal::Setup()
{
	o_tcymbal_.SetFreq(settings_.freq);
	o_tcymbal_.SetRes(settings_.res);
	o_tcymbal_.SetDrive(settings_.drive);
	o_tcymbal_.SetAmp(settings_.amp);
	o_tcymbal_.SetDecay(settings_.decay);
	o_tcymbal_.SetMin(settings_.min);
	o_tcymbal_.SetMix(settings_.mix);
}



void DCymbal::Trig(float vol)
{
	settings_.vol = vol;
	
	o_tcymbal_.Trig();
}



float DCymbal::Process()
{
	float voice_out;

	voice_out = o_tcymbal_.Process(false);

	return (voice_out * settings_.vol);
}

void DCymbal::SetFreq()
{
	o_tcymbal_.SetFreq(settings_.freq);
}

void DCymbal::SetDecay()
{
	o_tcymbal_.SetDecay(settings_.decay);
}

void DCymbal::SetRes()
{
	o_tcymbal_.SetRes(settings_.res);
}

void DCymbal::SetDrive()
{
	o_tcymbal_.SetDrive(settings_.drive);
}

void DCymbal::SetAmp()
{
	o_tcymbal_.SetAmp(settings_.amp);
}

void DCymbal::SetMin()
{
	o_tcymbal_.SetMin(settings_.min);
}

void DCymbal::SetMix()
{
	o_tcymbal_.SetMix(settings_.mix);
}


