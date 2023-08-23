#pragma once
#ifndef OPD_DBASS_H
#define OPD_DBASS_H

#include "daisy_seed.h"
#include "daisysp.h"

#include "main.h"
#include "tdrum.h"

using namespace daisy;
using namespace daisysp;


struct Settings_dbass
{
	float sample_rate;
	uint8_t type;
	float vol;

	// common
	float freq;
	float tone;
	float decay;
	
	// analog
	float fm_attack;
	float fm_self;
	
	// synthetic
	float dirtiness;
	float fm_env_amount;
	float fm_env_decay;
	
	// opd
	float min;
};



class DBass
{

	public:

    DBass() {}
    ~DBass() {}

	void Init(float, uint8_t);
	void Setup();
	float Process();
	void Trig(float);
	
	void SetFreq();
	void SetTone();
	void SetDecay();
	void SetFMAttack();
	void SetFMSelf();
	void SetDirtiness();
	void SetFMEnv();
	void SetFMEnvDecay();
	void SetMin();
	
//private:

	// config
	Settings_dbass settings_;
	
	// objects
	AnalogBassDrum o_bass_analog_;
	SyntheticBassDrum o_bass_synthetic_;
	TDrum o_tdrum_;
	
	// runtime	


};

#endif
