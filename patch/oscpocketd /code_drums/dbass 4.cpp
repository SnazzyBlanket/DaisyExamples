#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "dbass.h"

using namespace daisy;
using namespace daisysp;

// globals
extern DaisySeed hardware;
extern Settings settings;



void DBass::Init(float aSampleRate, uint8_t aType)
{
	settings_.sample_rate = aSampleRate;
	settings_.type = aType;
	
	// default values
	
	settings_.vol = 1.0f;
	
	// common
	settings_.freq = 50.0f;
	settings_.tone = 0.4f;
	settings_.decay = 0.3f;
	
	// analog
	settings_.fm_attack = 0.8f;
	settings_.fm_self = 0.8f;
	
	// synthetic
	settings_.dirtiness = 0.5f;
	settings_.fm_env_amount = 0.5f;
	settings_.fm_env_decay = 0.5f;
	
	// opd
	settings_.min = 0.5;
		
	// analog
	o_bass_analog_.Init(settings_.sample_rate);
	
	// synthetic
	o_bass_synthetic_.Init(settings_.sample_rate);
	
	// opd
	o_tdrum_.Init(settings_.sample_rate);

	Setup();	
}

void DBass::Setup()
{
	o_bass_analog_.SetFreq(settings_.freq);
	o_bass_analog_.SetTone(settings_.tone);
	o_bass_analog_.SetDecay(settings_.decay);
	o_bass_analog_.SetAttackFmAmount(settings_.fm_attack);
	o_bass_analog_.SetSelfFmAmount(settings_.fm_self);
	
	// synthetic
	o_bass_synthetic_.SetFreq(settings_.freq);
	o_bass_synthetic_.SetTone(settings_.tone);
//	o_bass_synthetic_.SetAccent(0.8f);
	o_bass_synthetic_.SetDecay(settings_.decay);
	o_bass_synthetic_.SetDirtiness(settings_.dirtiness);
	o_bass_synthetic_.SetFmEnvelopeAmount(settings_.fm_env_amount);
	o_bass_synthetic_.SetFmEnvelopeDecay(settings_.fm_env_decay);
	
	// opd
	o_tdrum_.SetFreq(settings_.freq);
	//o_tdrum_.SetAmp(amp_);
	o_tdrum_.SetDecay(settings_.decay);
	o_tdrum_.SetMin(settings_.min);
}



void DBass::Trig(float vol)
{
	settings_.vol = vol;

	switch (settings_.type)
	{
	case DTYPE_ANALOG:
		o_bass_analog_.Trig();
		break;
	case DTYPE_SYNTHETIC:
		o_bass_synthetic_.Trig();
		break;
	case DTYPE_OPD:
		o_tdrum_.Trig();
		break;
	}		
}



float DBass::Process()
{
	float voice_out;

	switch (settings_.type)
	{
	case DTYPE_ANALOG:
		voice_out = o_bass_analog_.Process(false);
		break;
	case DTYPE_SYNTHETIC:
		voice_out = o_bass_synthetic_.Process(false);
		break;
	case DTYPE_OPD:
		voice_out = o_tdrum_.Process(false);
		break;
	default:
		voice_out = 0.0f;
	}		

	// out

	return (voice_out * settings_.vol);
}



void DBass::SetFreq()
{
	o_bass_analog_.SetFreq(settings_.freq);
	o_bass_synthetic_.SetFreq(settings_.freq);
	o_tdrum_.SetFreq(settings_.freq);
}

void DBass::SetTone()
{
	o_bass_analog_.SetTone(settings_.tone);
	o_bass_synthetic_.SetTone(settings_.tone);
}

void DBass::SetDecay()
{
	o_bass_analog_.SetDecay(settings_.decay);
	o_bass_synthetic_.SetDecay(settings_.decay);
	o_tdrum_.SetDecay(settings_.decay);
}

void DBass::SetFMAttack()
{
	o_bass_analog_.SetAttackFmAmount(settings_.fm_attack);
}

void DBass::SetFMSelf()
{
	o_bass_analog_.SetSelfFmAmount(settings_.fm_self);
}

void DBass::SetDirtiness()
{
	o_bass_synthetic_.SetDirtiness(settings_.dirtiness);
}

void DBass::SetFMEnv()
{
	o_bass_synthetic_.SetFmEnvelopeAmount(settings_.fm_env_amount);
}

void DBass::SetFMEnvDecay()
{
	o_bass_synthetic_.SetFmEnvelopeDecay(settings_.fm_env_decay);
}

void DBass::SetMin()
{
	o_tdrum_.SetMin(settings_.min);
}

