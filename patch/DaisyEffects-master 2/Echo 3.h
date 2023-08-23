#ifndef ECHO_H
#define ECHO_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "IEffect.h"
#include "../../include/PedalConfig.h"
#include "../Inputs/Knob.h"
#include "../Inputs/Button.h"
#include "../Inputs/NFNToggle.h"

using namespace daisy;
using namespace daisysp;

/**********************************************
 * Echo Effect
 * 
 * SPST 1 - 
 * SPST 2 - 
 * SPST 3 - 
 * SPST 4 - 
 * 
 * SPDT 1 - Type Switcher
 * SPDT 2 - 
 * 
 * Knob 1 - Mix
 * Knob 2 - Decay
 * Knob 3 - Tempo
 * Knob 4 - 
 * 
 * LED 1 - 
 * LED 2 - 
 * LED 3 - 
 * LED 4 - 
 **********************************************/

// Type constants
enum DelayType
{
    QUARTER = 0,
    DOTTED_EIGHTH = 1,
    TRIPLET = 2,

    DT_UNSET = 99
};

static const size_t delayMaxSize = 192000;
static DelayLine<float, delayMaxSize> DSY_SDRAM_BSS del_line;

class Echo : public IEffect
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
    const char *knobNames[MAX_KNOBS] = {(char *)"MIX", (char *)"DECAY", (char *)"TEMPO", (char *)""};

    void TapTempoInterruptHandler();
    void TypeSwitcherLoopControl();
    float CalculateSampleFromBpm(int bpm);

    DaisySeed *hw;
    float sample_rate;

    // Input handlers
    NFNToggle typeSwitcher;
    Knob mixLevelKnob;
    Knob decayKnob;
    Knob tempoKnob;

    static const size_t initialTempoBpm = 90;

    // Decay constants
    const float minDecayValue = 0.0f;
    const float maxDecayValue = 0.75f;

    // Level constants
    const float mixLevelMin = 0.0f;
    const float mixLevelMax = 1.0f;

    // Tempo constants
    const float tempoMin = 48.0f;
    const float tempoMax = 132.0f;

    // Mutable parameters
    float decayValue = 0.5f;
    float mixLevel = 0.5f;
    float tempo = 0.0f;

    // Tap tempo mutables
    float currentTempoSamples;
    int currentTapTempoBpm = 0;
    int *pedalTapTempoBpm;

    // Type switcher mutables
    DelayType currentDelayType = DT_UNSET;
    float tempoModifier = 1.0f;
};

#endif