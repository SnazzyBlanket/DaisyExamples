#ifndef METRONOME_H
#define METRONOME_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "IEffect.h"
#include "../../../include/PedalConfig.h"
#include "../Inputs/Knob.h"
#include "../Inputs/NFNToggle.h"

using namespace daisy;
using namespace daisysp;

/**********************************************
 * Drum Machine Effect
 * 
 * SPST 1 - 
 * SPST 2 - 
 * SPST 3 - 
 * SPST 4 - 
 * 
 * SPDT 1 - Time Signature (3/4, 4/4, 6/8)
 * SPDT 2 - 
 * 
 * Knob 1 - Volume
 * Knob 2 - Tempo
 * Knob 3 - 
 * Knob 4 - 
 * 
 * LED 1 - 
 * LED 2 - 
 * LED 3 - 
 * LED 4 - 
 **********************************************/

enum TimeSignature
{
    ThreeFour = 3,
    FourFour = 4,
    SixEight = 6
};

class Metronome : public IEffect
{
public:
    void Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm);
    void Cleanup();
    float Process(float in);
    void Loop(bool allowEffectControl);
    char *GetEffectName();
    char **GetKnobNames();
    EffectSettings GetEffectSettings();
    void SetEffectSettings(EffectSettings effectSettings);
    void UpdateToggleDisplay();

private:
    const char *knobNames[MAX_KNOBS] = {(char *)"VOLUME", (char *)"TEMPO", (char *)"", (char *)""};

    float GetTempoFromBpm(int bpm);

    // Beat parameters
    const float bFrequency = 550.0f;
    const float dbFrequency = 1000.0f;
    const float dirtiness = 0.0f;
    const float fmEnvelopeAmount = 0.0f;
    const float fmEnvelopeDecay = 0.0f;
    const float decay = 0.0f;
    const float tone = 1.0f;
    const float accent = 0.0f;

    // Adjustable parameters
    const float volumeLevelMin = 0.0f;
    const float volumeLevelMax = 2.0f;
    const float tempoMin = 48.0f;
    const float tempoMax = 132.0f;

    float volume = 1.0f;
    float tempo = 90.0f;
    TimeSignature timeSignature = TimeSignature::FourFour;

    // Tap tempo mutables
    int currentTapTempoBpm = 0;
    int *pedalTapTempoBpm;

    int count = 0;

    Knob volumeKnob;
    Knob tempoKnob;
    NFNToggle timeSigToggle;

    Metro tick;
    SyntheticBassDrum beat;
};

#endif