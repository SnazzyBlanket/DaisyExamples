#pragma once
#ifndef OPD_DHIHAT_H
#define OPD_DHIHAT_H

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"

using namespace daisy;
using namespace daisysp;

struct Settings_dhihat
{
	float sample_rate;
	uint8_t type;
	float vol;

	// common
	float freq;
	float tone;
	float decay;
	float decay_2nd;

	// analog
	// synthetic
	float noisiness;
	
	// opd
	float amp;
	float res; // tone?
	float drive; // noisiness_?

};

class DHihat
{

	public:

    DHihat() {}
    ~DHihat() {}

	void Init(float, uint8_t);
	void Setup();
	float Process();
	void Trig(float, uint8_t);

	void SetFreq();
	void SetTone();
	void SetDecay();
	void SetNoisiness();
	void SetRes();
	void SetAmp();
	void SetDrive();
	
//private:

	// config
	Settings_dhihat settings_;

	// objects
	HiHat<SquareNoise> o_hihat_analog_;
	HiHat<RingModNoise> o_hihat_synthetic_;
	WhiteNoise o_whitenoise_;
	Svf o_filter_;
	AdEnv o_env_a_;

	// runtime	
};

#endif
