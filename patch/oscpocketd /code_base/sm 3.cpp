#include "daisy_seed.h"
#include "sm.h"
#include "main.h"

using namespace daisy;

extern DaisySeed hardware;
extern OpdFXSettings fxSettings;
extern OpdModSources modSources;

void SM::Init(uint8_t aNumber,float aSampleRate, uint8_t aType, float aFreq, float aAmp, float aOffset)
{
	smNumber = aNumber;
	smSampleRate = aSampleRate;
	smType = aType;
	smFreq = aFreq;
	smAmp = aAmp;
	smOffset = aOffset;

	smVal = 0;
	smStep = 0;
	smGate = 0.0f;
	
	smExtraInt = 0;

	CalcInc();
}

void SM::CalcInc()
{
	smInc = (smFreq / smSampleRate);
}

float SM::Process()
{
	/*
		c += (1/sr) * f
		
		f = 1000
		sr = 48 000
		c = 1/48
	*/
	bool nextNote = false;
	
	if (fxSettings.smSyncMod[smNumber] == MOD_SOURCE_NONE)
	{
		nextNote = (smStep == 0);
	} else {
		if (modSources.smSync[smNumber])
		{
			if (ModFactor(fxSettings.smSyncMod[smNumber]) < fxSettings.smSyncLevel[smNumber])
			{
				modSources.smSync[smNumber] = 0;
			}
		} else {
			if (ModFactor(fxSettings.smSyncMod[smNumber]) >= fxSettings.smSyncLevel[smNumber])
			{
				modSources.smSync[smNumber] = 1;
				nextNote = true;
			}
		}
	}
	
	if (nextNote)
	{
		switch (smType)
		{
		case SM_TYPE_NOISE:
			// int theDice = std::rand() % 100;
			// return ((float)rand() / (float)RAND_MAX) <= prob ? true : false;
			smVal = ((float)rand() / (float)RAND_MAX) * smAmp + smOffset;
			smVal = MIN(smVal, 1);
			smGate = 1.0f;
			break;
		case SM_TYPE_CRAWL:
			// smExtra = direction (0 = down, 1 = up)
			// smAmp = how much to change / 10, ie 50 = 5 steps
			// smOffset = probabibilty to NOT CHANGE
			if (smExtraInt == 0)
			{
				smVal += (smAmp / 10);
				smVal = MIN(smVal, 1);
			} else {
				smVal -= (smAmp / 10);
				smVal = MAX(smVal, 0);
			}
			if (((float)rand() / (float)RAND_MAX) > smOffset)
			{
				if (smExtraInt == 0)
				{
					smExtraInt = 1;
				} else {
					smExtraInt = 0;
				}
			}
			smGate = 1.0f;	
			break;
		case SM_TYPE_INTERVAL:
			// interval can be made longer by NOT CHANGING smVal based on probability
			// smAmp = value that should be output
			// smOffset = probabibilty to NOT CHANGE
			if (((float)rand() / (float)RAND_MAX) > smOffset)
			{
				if (smVal == smAmp)
				{
					smVal = 0;
				} else {
					smVal = smAmp;
					smGate = 1.0f;
				}
			}		
			break;
		case SM_TYPE_CHAOS:
			// smOffset = probabibilty to NOT CHANGE
			if (((float)rand() / (float)RAND_MAX) > smOffset)
			{
				smVal = ((float)rand() / (float)RAND_MAX) * smAmp;
				smVal = MIN(smVal, 1);
				smGate = 1.0f;
			}		
			break;
		case SM_TYPE_SEQ:
			smExtraInt++;
			// a bit ugly to be accessing fxSettings/global data in an object...
			if (smExtraInt >= fxSettings.smSeqLen[smNumber])
			{
				smExtraInt = 0;
			}
			smVal = fxSettings.smSeqVal[smNumber][smExtraInt];
			smGate = 1.0f;
			break;
		}
	}

	smStep += smInc;
	if (smStep >= 1)
	{
		smStep = 0;
	}
	
	return (smVal);
}


float SM::Gate()
{

	if (fxSettings.smSyncMod[smNumber] == MOD_SOURCE_NONE)
	{
		if (smStep > fxSettings.smGateLen)
		{
			smGate = 0.0f;
		}
	} else {
		smGate = (modSources.smSync[smNumber] ? 1.0f : 0.0f);
	}

	return (smGate);
}


void SM::SetType(uint8_t aType)
{
	smType = aType;
	smExtraInt = 0;
	CalcInc();
}

void SM::SetFreq(float aFreq)
{
	smFreq = aFreq;
	CalcInc();
}

void SM::SetAmp(float aAmp)
{
	smAmp = aAmp;
	CalcInc();
}

void SM::SetOffset(float aOffset)
{
	smOffset = aOffset;
	CalcInc();
}


