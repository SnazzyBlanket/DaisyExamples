#pragma once
#ifndef OPD_TCLAP_H
#define OPD_TCLAP_H

#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

class TClap
{

private:
	float sample_rate_;

	float freq_;
	float amp_;
	float decay_;
	float res_;
	float drive_;


	WhiteNoise o_whitenoise_;
	Svf o_filter_;
	AdEnv env_a_;


public:
	void Init(float);
	float Process(bool);
	void Trig(void);
	void SetFreq(float);
	void SetRes(float);
	void SetDrive(float);
	void SetAmp(float);
	void SetDecay(float);
};

#endif
