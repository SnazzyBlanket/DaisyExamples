#pragma once
#ifndef OPD_DSNARE_H
#define OPD_DSNARE_H

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "tdrum.h"

using namespace daisy;
using namespace daisysp;

struct Settings_dsnare
{
	float sample_rate;
	uint8_t type;
	float vol;
	
	// common
	float freq;
	float decay;
	float snappy;

	// analog
	float tone;

	// synthetic
	float fm_amount;
	
	// opd
	float amp;
	float min;
	float res;
	float drive;
	float freq_noise;
};

class DSnare
{

	public:

    DSnare() {}
    ~DSnare() {}

	void Init(float, uint8_t);
	void Setup();
	float Process();
	void Trig(float);
	void SetFreq();
	void SetTone();
	void SetDecay();
	void SetSnappy();
	void SetFM();
	void SetFreqNoise();
	void SetRes();
	void SetAmp();
	void SetDrive();
	void SetMin();
	
//private:

	// config

	struct Settings_dsnare settings_;

	// objects
	AnalogSnareDrum o_snare_analog_;
	SyntheticSnareDrum o_snare_synthetic_;

	WhiteNoise o_whitenoise_;
	Svf o_filter_;
	AdEnv o_env_a_;
	TDrum o_tdrum_;	
	
	// runtime	


};

#endif
