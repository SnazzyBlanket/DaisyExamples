#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "dclap.h"

using namespace daisy;
using namespace daisysp;

// globals
extern DaisySeed hardware;

void DClap::Init(float aSampleRate, float aFreq, float aRes, float aDrive, float aAmp, float aDecay)
{
	settings_.sample_rate = aSampleRate;
	settings_.vol = 1.0f;
		
	// common
	settings_.freq = aFreq;
	settings_.res = aRes;
	settings_.drive = aDrive;
	settings_.amp = aAmp;
	settings_.decay = aDecay;
	
	
	// OPD
	o_tclap_.Init(settings_.sample_rate);

	Setup();
}



void DClap::Setup()
{	o_tclap_.SetFreq(settings_.freq);
	o_tclap_.SetRes(settings_.res);
	o_tclap_.SetDrive(settings_.drive);
	o_tclap_.SetAmp(settings_.amp);
	o_tclap_.SetDecay(settings_.decay);
}



void DClap::Trig(float vol)
{
	settings_.vol = vol;

	o_tclap_.Trig();
}



float DClap::Process()
{
	float voice_out;

	voice_out = o_tclap_.Process(false);

	return (voice_out * settings_.vol);
}


void DClap::SetFreq()
{
	o_tclap_.SetFreq(settings_.freq);
}

void DClap::SetDecay()
{
	o_tclap_.SetDecay(settings_.decay);
}

void DClap::SetRes()
{
	o_tclap_.SetRes(settings_.res);
}

void DClap::SetDrive()
{
	o_tclap_.SetDrive(settings_.drive);
}

void DClap::SetAmp()
{
	o_tclap_.SetAmp(settings_.amp);
}

