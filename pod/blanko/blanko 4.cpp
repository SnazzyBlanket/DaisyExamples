#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

#include <arm_math.h>
#include <arm_const_structs.h>

#define TEST_LENGTH_SAMPLES 2048

/* -------------------------------------------------------------------
* External Input and Output buffer Declarations for FFT Bin Example
* ------------------------------------------------------------------- */
extern float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES];
static float32_t testOutput[TEST_LENGTH_SAMPLES/2];

/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */
uint32_t fftSize = 1024;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

/* Reference index at which max energy of bin ocuurs */
uint32_t refIndex = 213, testIndex = 0;

/* ----------------------------------------------------------------------
* Max magnitude FFT Bin test
* ------------------------------------------------------------------- */
DaisyPod hw; 


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}

int main(void)
{

  arm_status status;
  float32_t maxValue;

  status = ARM_MATH_SUCCESS;

  /* Process the data through the CFFT/CIFFT module */
  arm_cfft_f32(&arm_cfft_sR_f32_len1024, testInput_f32_10khz, ifftFlag, doBitReverse);

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
  arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);

  /* Calculates maxValue and returns corresponding BIN value */
  arm_max_f32(testOutput, fftSize, &maxValue, &testIndex);

  if (testIndex !=  refIndex)
  {
    status = ARM_MATH_TEST_FAILURE;
  }

  /* ----------------------------------------------------------------------
  ** Loop here if the signals fail the PASS check.
  ** This denotes a test failure
  ** ------------------------------------------------------------------- */

  if ( status != ARM_MATH_SUCCESS)
  {
    while (1);
  }
hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {}            
	
	           /* main function does not return */
}


 /** \endlink */




