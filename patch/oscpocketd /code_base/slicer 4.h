/*
  
Project: Slicer module for Electrosmith's Daisy Seed
Description: Records a random amount of time and plays it back a random amount of times. Can also be triggered.
Author: Staffan Melin, staffan.melin@oscillator.se
License: The MIT License (MIT)
Version: 202112
Project site: http://www.oscillator.se/opensource

Example creation:
static Slicer<480000> DSY_SDRAM_BSS fxSlicer;

Example usage in AudioCallback():
sigOut = fxSlicer.Process(sigIn);

*/

#pragma once
#ifndef OPD_SLICER_H
#define OPD_SLICER_H

#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

template <size_t max_size>
class Slicer
{

private:
	float sample_rate_;

	size_t record_;
	
	size_t record_pos_;
	size_t record_samples_max_; // < max_size
	size_t record_samples_;
	
	size_t playback_pos_;
	size_t playback_end_;
	
	size_t playback_rep_;
	size_t playback_rep_max_;
	size_t playback_rep_nr_;
	
	bool trig_mode_; // if true, record mode and playback mode is started with Trig()

    float buffer_[max_size];

public:
	void Init(float aSampleRate)
	{
		sample_rate_ = aSampleRate;
	
		record_ = true;
		record_pos_ = 0;
		record_samples_max_ = 48000; // 1 sec
		record_samples_ = 1 + ((float)rand() / (float)RAND_MAX) * (float)record_samples_max_;
		
		playback_pos_ = 0;
		playback_end_ = 1;
		playback_rep_ = 0;
		playback_rep_max_ = 1000;
		playback_rep_nr_ = 1 + ((float)rand() / (float)RAND_MAX) * (float)playback_rep_max_;
		
		trig_mode_ = false;
	}
	
	float Process(float in)
	{
		if (record_)
		{
			buffer_[record_pos_] = in;
			record_pos_++;

			if (record_pos_ >= record_samples_ || record_pos_ >= max_size)
			{
				record_ = false;
				playback_end_ = record_pos_ - 1;
				playback_pos_ = 0;
				if (!trig_mode_)
				{
					playback_rep_nr_ = 1 + ((float)rand() / (float)RAND_MAX) * (float)playback_rep_max_;
					playback_rep_ = 0;
				}			
			}
		}

		float return_value = buffer_[playback_pos_];

		playback_pos_++;

		if (playback_pos_ > playback_end_ && !record_)
		{
			playback_pos_ = 0;
			if (!record_ && !trig_mode_)
			{
				playback_rep_++;
				if (playback_rep_ >= playback_rep_nr_)
				{
					playback_rep_nr_ = 1 + ((float)rand() / (float)RAND_MAX) * (float)playback_rep_max_;
					playback_rep_ = 0;
					record_samples_ = 1 + ((float)rand() / (float)RAND_MAX) * (float)record_samples_max_;
					record_pos_ = 0; 
					record_ = true;
				}
			}
		}
		
		return (return_value);
	}
	
	void SetRecordMax(size_t val)
	{
		if (val <= max_size)
			record_samples_max_ = val;
		else
			record_samples_max_ = max_size;
	}

	void SetPlaybackMax(size_t val)
	{
		playback_rep_max_ = val;
	}
	
	void SetTrigMode(bool mode)
	{
		trig_mode_ = mode;
	}
	
	void Trig(bool on)
	{
		if (trig_mode_)
		{
			if (on)
			{
				// start recording
				record_samples_ = max_size;
				record_pos_ = 0; 
				record_ = true;
				playback_pos_ = 0;
			} else {
				// start repeat
				record_ = false;
				playback_end_ = record_pos_ - 1;
				playback_pos_ = 0;

			}
		}
	}

};

#endif
