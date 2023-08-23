#pragma once
#ifndef OPD_TCYMBAL_H
#define OPD_TCYMBAL_H

#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

class TCymbal
{

private:
	float sample_rate_;

	float freq_;
	float amp_;
	float decay_;
	float min_;
	float mix_;
	float res_;
	float drive_;


	WhiteNoise o_whitenoise_;
	RingModNoise o_ringnoise_;
	Svf o_filter_;
	AdEnv env_f_;
	AdEnv env_a_;

	float f0_;


public:
	void Init(float);
	float Process(bool);
	void Trig(void);
	void SetFreq(float);
	void SetRes(float);
	void SetDrive(float);
	void SetAmp(float);
	void SetDecay(float);
	void SetMin(float);
	void SetMix(float);
};

#endif
