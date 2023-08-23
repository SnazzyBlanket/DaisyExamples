#ifndef DAISY_RESONATOR_H
#define DAISY_RESONATOR_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "IEffect.h"
#include "../../../include/PedalConfig.h"
#include "../Inputs/Knob.h"
#include "../Inputs/NFNToggle.h"

using namespace daisy;
using namespace daisysp;

/**********************************************
 * Daisy Resonator Effect
 *
 * SPST 1 -
 * SPST 2 -
 * SPST 3 -
 * SPST 4 -
 *
 * SPDT 1 - Mix Level
 * SPDT 2 -
 *
 * Knob 1 - Brightness
 * Knob 2 - Structure
 * Knob 3 - Damping
 * Knob 4 - Frequency
 *
 * LED 1 -
 * LED 2 -
 * LED 3 -
 * LED 4 -
 **********************************************/

class DaisyResonator : public IEffect
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
    const char *knobNames[MAX_KNOBS] = {(char *)"BRIGHT", (char *)"STRUCT", (char *)"DAMP", (char *)"FREQ"};

    void SetMix();
    float sample_rate;

    // Brightness
    const float brightnessMin = 0.0f;
    const float brightnessMax = 1.0f;
    float brightness = 0.5f;

    // Structure
    const float structureMin = 0.0f;
    const float structureMax = 1.0f;
    float structure = 0.5f;

    // Damping
    const float dampingMin = 0.0f;
    const float dampingMax = 1.0f;
    float damping = 0.5f;

    // Frequency
    const float freqMin = 110.0f;
    const float freqMax = 880.0f;
    float freq = 110.f;

    // Mix
    const float mixLow = 0.33f;
    const float mixMed = 0.66f;
    const float mixHigh = 1.0f;
    float currMixValue = 1.0f;
    u_int8_t currMix = -1;

    Knob brightnessKnob;
    Knob structureKnob;
    Knob dampingKnob;
    Knob freqKnob;

    NFNToggle mixToggle;

    Resonator resonator;

    Balance balance;
};

#endif