#pragma once
#ifndef OPD_DCLAP_H
#define OPD_DCLAP_H

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "tclap.h"

using namespace daisy;
using namespace daisysp;



struct Settings_dclap
{
	float sample_rate;
	float vol;

	// common
	float freq;
	float res;
	float drive;
	float amp;
	float decay;	
};


class DClap
{

	public:

    DClap() {}
    ~DClap() {}

	void Init(float, float, float, float, float, float);
	void Setup();
	float Process();
	void Trig(float);

	void SetFreq();
	void SetDecay();
	void SetRes();
	void SetDrive();
	void SetAmp();

//private:

	// config
	Settings_dclap settings_;

	// objects
	TClap o_tclap_;
	
	// runtime
};

#endif
