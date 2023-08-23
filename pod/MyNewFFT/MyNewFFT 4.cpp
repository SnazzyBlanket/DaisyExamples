#include "daisy_pod.h"
#include "kiss_fft.h"

// Define the size of the FFT transform
#define FFT_SIZE 1024

// Define the frequency range for the 8 bands
float freq_range[8][2] = {{0, 200}, {200, 400}, {400, 800}, {800, 1600},
                          {1600, 3200}, {3200, 6400}, {6400, 12800}, {12800, 22050}};

void loop () {
  // Initialize the Daisy Pod
  daisy::DaisyPod pod;
  pod.Init();

  // Set up the VCOs
  daisy::Svf vco1;
  daisy::Svf vco2;
  // ...

  // Set up the FFT configuration object
  kiss_fft_scalar input[FFT_SIZE];
  kiss_fft_cpx output[FFT_SIZE];
  kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);

  while (1) {
    // Generate the input signal
    // ...

    // Perform the FFT transform
    kiss_fft(cfg, input, output);
    float fft_output[8];
    for (int i = 0; i < 8; i++) {
      fft_output[i] = 0;
      for (int j = freq_range[i][0]; j < freq_range[i][1]; j++) {
        fft_output[i] += sqrt(output[j].r * output[j].r + output[j].i * output[j].i);
      }
      fft_output[i] /= (freq_range[i][1] - freq_range[i][0]);
    }

    // Use the FFT output to control the second VCO
    float vco2_freq = map(fft_output[1], 0, 1, 100, 1000);
    vco2.SetFreq(vco2_freq);

    // ...
  }

  kiss_fft_free(cfg);
  return 0;
}
