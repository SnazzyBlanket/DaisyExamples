#pragma once

#define SM_TYPE_NOISE 0
#define SM_TYPE_CRAWL 1
#define SM_TYPE_INTERVAL 2
#define SM_TYPE_CHAOS 3
#define SM_TYPE_SEQ 4
#define SM_TYPE_MAX 5

class SM
{

private:
	uint8_t smNumber;
	float smSampleRate;
	uint8_t smType;
	float smFreq;
	float smAmp;
	float smOffset;
	
	float smVal;
	float smStep;
	float smInc;
	float smGate;
	uint8_t smExtraInt;
	
	void CalcInc(void);

public:
	void Init(uint8_t, float, uint8_t, float, float, float);
	float Process(void);
	float Gate(void);
	void SetType(uint8_t);
	void SetFreq(float);
	void SetAmp(float);
	void SetOffset(float);
};


