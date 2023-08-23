#pragma once
#ifndef OPD_DDRUM_H
#define OPD_DDRUM_H

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "tdrum.h"

using namespace daisy;
using namespace daisysp;



struct Settings_ddrum
{
	float sample_rate;
	uint8_t type;
	float vol;

	// common
	float freq;
	float amp;
	float decay;
	float min;	
};



class DDrum
{

	public:

    DDrum() {}
    ~DDrum() {}

	void Init(float, float, float, float, float);
	void Setup();
	float Process();
	void Trig(float);
	
	void SetFreq();
	void SetDecay();
	void SetAmp();
	void SetMin();
	
//private:

	// config
	Settings_ddrum settings_;

	// objects
	TDrum o_tdrum_;
	
	// runtime	

};

#endif
