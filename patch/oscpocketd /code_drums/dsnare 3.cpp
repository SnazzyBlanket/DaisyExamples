#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "dsnare.h"
#include "tdrum.h"

using namespace daisy;
using namespace daisysp;

// globals
extern DaisySeed hardware;



void DSnare::Init(float aSampleRate, uint8_t aType)
{
	settings_.sample_rate = aSampleRate;
	settings_.type = aType;
	settings_.vol = 1.0f;
	
	// common
	if (settings_.type == DTYPE_OPD)
		settings_.freq = 200.0f;
	else
		settings_.freq = 60.0f;
	settings_.tone = 0.5f;
	settings_.decay = 0.3f;

	// analog
	settings_.snappy = 0.3f;
	
	// synthetic
	settings_.fm_amount = 0.5f;
	
	// opd
	settings_.freq_noise = 1000.0f; // highpass
	settings_.amp = 0.5f;
	settings_.res = 0.3f;
	settings_.drive = 0.3f;
	settings_.min = 0.3f;

	
	// analog
	o_snare_analog_.Init(settings_.sample_rate);
	
	// synthetic
	o_snare_synthetic_.Init(settings_.sample_rate);
	
	// opd
	o_whitenoise_.Init();
	o_filter_.Init(settings_.sample_rate);
    o_env_a_.Init(settings_.sample_rate);
	o_tdrum_.Init(settings_.sample_rate);
	
	Setup();
}



void DSnare::Setup()
{
	// analog
	o_snare_analog_.SetFreq(settings_.freq);
	o_snare_analog_.SetTone(settings_.tone);
	o_snare_analog_.SetDecay(settings_.decay);
	o_snare_analog_.SetSnappy(settings_.snappy);
	
	// synthetic
	o_snare_synthetic_.SetFreq(settings_.freq);
	o_snare_synthetic_.SetDecay(settings_.decay);
	o_snare_synthetic_.SetSnappy(settings_.snappy);
	o_snare_synthetic_.SetFmAmount(settings_.fm_amount);
	
	// opd
	o_whitenoise_.SetAmp(settings_.amp);
	
	o_filter_.SetFreq(settings_.freq_noise);
	o_filter_.SetRes(settings_.res);
	o_filter_.SetDrive(settings_.drive);
	
    o_env_a_.SetTime(ADENV_SEG_ATTACK, .01);
    o_env_a_.SetTime(ADENV_SEG_DECAY, settings_.decay);
    o_env_a_.SetMin(0.0);
    o_env_a_.SetMax(1.f);
    o_env_a_.SetCurve(0); // linear

	o_tdrum_.SetFreq(settings_.freq);
	o_tdrum_.SetAmp(settings_.amp);
	o_tdrum_.SetDecay(settings_.decay);
	o_tdrum_.SetMin(settings_.min);
}


void DSnare::Trig(float vol)
{
	settings_.vol = vol;

	switch (settings_.type)
	{
	case DTYPE_ANALOG:
		o_snare_analog_.Trig();
		break;
	case DTYPE_SYNTHETIC:
		o_snare_synthetic_.Trig();
		break;
	case DTYPE_OPD:
		o_tdrum_.Trig();
		o_env_a_.Trigger();
		break;
	}		

}

float DSnare::Process()
{
	float voice_out;

	switch (settings_.type)
	{
	case DTYPE_ANALOG:
		voice_out = o_snare_analog_.Process(false);
		break;
	case DTYPE_SYNTHETIC:
		voice_out = o_snare_synthetic_.Process(false);
		break;
	case DTYPE_OPD:
		o_filter_.Process(o_whitenoise_.Process());
		voice_out = o_filter_.High() * o_env_a_.Process() * settings_.snappy;
		voice_out += o_tdrum_.Process(false) * (1.0f - settings_.snappy);
		//voice_out /= 2;
		break;
	default:
		voice_out = 0.0f;
	}		

	return (voice_out * settings_.vol);
}


void DSnare::SetFreq()
{
	o_snare_analog_.SetFreq(settings_.freq);
	o_snare_synthetic_.SetFreq(settings_.freq);
	o_tdrum_.SetFreq(settings_.freq);
}

void DSnare::SetTone()
{
	o_snare_analog_.SetTone(settings_.tone);
}

void DSnare::SetDecay()
{
	o_snare_analog_.SetDecay(settings_.decay);
	o_snare_synthetic_.SetDecay(settings_.decay);
	o_env_a_.SetTime(ADENV_SEG_DECAY, settings_.decay);
	o_tdrum_.SetDecay(settings_.decay);
}

void DSnare::SetSnappy()
{
	o_snare_analog_.SetSnappy(settings_.snappy);
	o_snare_synthetic_.SetSnappy(settings_.snappy);
}

void DSnare::SetFM()
{
	o_snare_synthetic_.SetFmAmount(settings_.fm_amount);
}

void DSnare::SetFreqNoise()
{
	o_filter_.SetFreq(settings_.freq_noise);
}

void DSnare::SetRes()
{
	o_filter_.SetRes(settings_.res);
}

void DSnare::SetAmp()
{
	o_tdrum_.SetAmp(settings_.amp);
}

void DSnare::SetDrive()
{
	o_filter_.SetDrive(settings_.drive);
}

void DSnare::SetMin()
{
	o_tdrum_.SetMin(settings_.min);
}


