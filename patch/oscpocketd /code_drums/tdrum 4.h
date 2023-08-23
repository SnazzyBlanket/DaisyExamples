#pragma once
#ifndef OPD_TDRUM_H
#define OPD_TDRUM_H

#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

class TDrum
{

private:
	float sample_rate_;

	float freq_;
	float amp_;
	float decay_;
	float min_;

	Oscillator osc_;
	AdEnv env_p_;
	AdEnv env_a_;

public:
	void Init(float);
	float Process(bool);
	void Trig(void);
	void SetFreq(float);
	void SetAmp(float);
	void SetDecay(float);
	void SetMin(float);
};

#endif
