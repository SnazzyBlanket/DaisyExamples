#pragma once
#ifndef OPD_DCYMBAL_H
#define OPD_DCYMBAL_H

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "tcymbal.h"

using namespace daisy;
using namespace daisysp;

struct Settings_dcymbal
{
	float sample_rate;
	uint8_t type;
	float vol;

	// common
	float freq;
	float res;
	float drive;
	float amp;
	float decay;
	float min;
	float mix;
};

class DCymbal
{
	public:

    DCymbal() {}
    ~DCymbal() {}

	void Init(float, float, float, float, float, float, float, float);
	void Setup();
	float Process();
	void Trig(float);

	void SetFreq();
	void SetDecay();
	void SetRes();
	void SetDrive();
	void SetAmp();
	void SetMin();
	void SetMix();

//private:

	// config

	Settings_dcymbal settings_;
	
	// objects
	TCymbal o_tcymbal_;
	
	// runtime	

};

#endif
