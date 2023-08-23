

# include <math.h>
# include "arm_math.h"
#include <stdio.h>

#define  FFT_SIZE       32
#define  F_PI   3.14



uint16_t i;

  //FFT Variables
  arm_rfft_fast_instance_f32 s;
  enum windowing{hamming=0,hanning,Blackman_Harris,nuttall,Blackman_Nutall } window_type ;
 
float window_multipliers[FFT_SIZE] = {0};  //coefficent for window selecting
float fft_input[FFT_SIZE] ;
float fft_output[FFT_SIZE];//output of FFt that is a complex number but have fft_size/2   point inherent
float fft_mag[FFT_SIZE/2];//magnitude of FFT
 
int main(void)
{
  /* USER CODE BEGIN 1 */

    window_type=hamming;
    switch (window_type){
    case hamming:
        for ( i = 0; i < FFT_SIZE; i++) {
            window_multipliers[i] = 0.54f - 0.46f * arm_cos_f32((2.0f * F_PI * (float32_t)i) / ((float32_t)FFT_SIZE - 1.0f));
        }
    break;

    case Blackman_Harris:
        for ( i = 0; i < FFT_SIZE; i++){
            window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) +0.14128f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) -0.01168f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));  
        }
    break;

    case nuttall:
        for ( i = 0; i < FFT_SIZE; i++){
        window_multipliers[i] = 0.355768f - 0.487396f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) +0.144232f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) -0.012604 * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
        }
    break;
    
    case Blackman_Nutall:
        for ( i = 0; i < FFT_SIZE; i++){
                window_multipliers[i] = 0.3635819f - 0.4891775f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) +0.1365995f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.0106411f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
        }
    break;
    
    case hanning :
        for ( i = 0; i < FFT_SIZE; i++){
        window_multipliers[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * F_PI * (float32_t)i / (float32_t)FFT_SIZE));
        }   
    break;
}//window type switch
/*
****problem maker**    */
  arm_rfft_fast_init_f32(&s,FFT_SIZE);

  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */


  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      for(i=0; i<FFT_SIZE ;i++){ 
          fft_input[i]=arm_sin_f32 ( (2*F_PI*i) / 360 );
       
      }
      
      //do windowing 
      arm_mult_f32(window_multipliers ,fft_input,fft_input,FFT_SIZE);
    
  arm_rfft_fast_f32(&s,fft_input,fft_input,0);       //Q: can  we use same buffer for in& out of FFT 
      
//calculate  magnitude of  the complex output of FFt 
      arm_cmplx_mag_f32(fft_input,fft_mag,FFT_SIZE);
  }
