#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "dhihat.h"

using namespace daisy;
using namespace daisysp;

// globals
extern DaisySeed hardware;



void DHihat::Init(float aSampleRate, uint8_t aType)
{
	settings_.sample_rate = aSampleRate;
	settings_.type = aType;
	settings_.vol = 1.0f;
	
	// common
	settings_.freq = 5000.0f;
	settings_.tone = 0.8f;
	settings_.decay = 0.1f;
	settings_.decay_2nd = 0.3f;

	// analog
	// synthetic
	settings_.noisiness = 0.5f;
	
	// opd
	settings_.amp = 0.3f;
	settings_.res = 0.3f;
	settings_.drive = 0.3f;
	
	// analog (squarenoise)
	o_hihat_analog_.Init(settings_.sample_rate);
	
	// synthetic (ringmodnoise)
	o_hihat_synthetic_.Init(settings_.sample_rate);
	
	// opd
	o_whitenoise_.Init();
	o_filter_.Init(settings_.sample_rate);
    o_env_a_.Init(settings_.sample_rate);

	Setup();
}

void DHihat::Setup()
{
	o_hihat_analog_.SetFreq(settings_.freq);
	o_hihat_analog_.SetTone(settings_.tone);
	o_hihat_analog_.SetDecay(settings_.decay);
	o_hihat_analog_.SetNoisiness(settings_.noisiness);
	
	// synthetic (ringmodnoise)
	o_hihat_synthetic_.SetFreq(settings_.freq);
	o_hihat_synthetic_.SetTone(settings_.tone);
	o_hihat_synthetic_.SetDecay(settings_.decay);
	o_hihat_synthetic_.SetNoisiness(settings_.noisiness);
	
	// opd
	o_whitenoise_.SetAmp(settings_.amp);

	o_filter_.SetFreq(settings_.freq);
	o_filter_.SetRes(settings_.res);
	o_filter_.SetDrive(settings_.drive);

    o_env_a_.SetTime(ADENV_SEG_ATTACK, .01);
    o_env_a_.SetTime(ADENV_SEG_DECAY, settings_.decay);
    o_env_a_.SetMin(0.0);
    o_env_a_.SetMax(1.f);
    o_env_a_.SetCurve(0); // linear
}


void DHihat::Trig(float vol, uint8_t val)
{
	settings_.vol = vol;

	switch (settings_.type)
	{
	case DTYPE_ANALOG:
		if (val == DRUM_ACCENT)
			o_hihat_analog_.SetDecay(settings_.decay_2nd);
		else
			o_hihat_analog_.SetDecay(settings_.decay);
		o_hihat_analog_.Trig();
		break;
	case DTYPE_SYNTHETIC:
		if (val == DRUM_ACCENT)
			o_hihat_analog_.SetDecay(settings_.decay_2nd);
		else
			o_hihat_analog_.SetDecay(settings_.decay);
		o_hihat_synthetic_.Trig();
		break;
	case DTYPE_OPD:
		if (val == DRUM_ACCENT)
    		o_env_a_.SetTime(ADENV_SEG_DECAY, settings_.decay_2nd);
		else
    		o_env_a_.SetTime(ADENV_SEG_DECAY, settings_.decay);
		o_env_a_.Trigger();
		break;
	}		

}

float DHihat::Process()
{
	float voice_out;

	switch (settings_.type)
	{
	case DTYPE_ANALOG:
		voice_out = o_hihat_analog_.Process(false);
		break;
	case DTYPE_SYNTHETIC:
		voice_out = o_hihat_synthetic_.Process(false);
		break;
	case DTYPE_OPD:
		o_filter_.Process(o_whitenoise_.Process());
		voice_out = o_filter_.High() * o_env_a_.Process();
		break;
	default:
		voice_out = 0.0f;
	}		

	return (voice_out * settings_.vol);
}

void DHihat::SetFreq()
{
	o_hihat_analog_.SetFreq(settings_.freq);
	o_hihat_synthetic_.SetFreq(settings_.freq);
	o_filter_.SetFreq(settings_.freq);
}

void DHihat::SetTone()
{
	o_hihat_analog_.SetTone(settings_.tone);
	o_hihat_synthetic_.SetTone(settings_.tone);
}

void DHihat::SetDecay()
{
	o_hihat_analog_.SetDecay(settings_.decay);
	o_hihat_synthetic_.SetDecay(settings_.decay);
    o_env_a_.SetTime(ADENV_SEG_DECAY, settings_.decay);
}

void DHihat::SetNoisiness()
{
	o_hihat_analog_.SetNoisiness(settings_.noisiness);
	o_hihat_synthetic_.SetNoisiness(settings_.noisiness);
}

void DHihat::SetRes()
{
	o_filter_.SetRes(settings_.res);
}

void DHihat::SetAmp()
{
	o_whitenoise_.SetAmp(settings_.amp);
}

void DHihat::SetDrive()
{
	o_filter_.SetDrive(settings_.drive);
}
