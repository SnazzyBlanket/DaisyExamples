#ifndef STOMPBOX_H
#define STOMPBOX_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "IEffect.h"
#include "../../../include/PedalConfig.h"
#include "../Inputs/Knob.h"

using namespace daisy;
using namespace daisysp;

/**********************************************
 * Stomp Box Effect
 *
 * SPST 1 -
 * SPST 2 -
 * SPST 3 -
 * SPST 4 -
 *
 * SPDT 1 -
 * SPDT 2 -
 *
 * Knob 1 - Level
 * Knob 2 - Decay
 * Knob 3 - Tone
 * Knob 4 - Accent
 *
 * LED 1 -
 * LED 2 -
 * LED 3 -
 * LED 4 -
 **********************************************/

class StompBox : public IEffect
{
public:
    void Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm);
    void Cleanup();
    float Process(float in);
    void Loop(bool allowEffectControl, bool isTapPressed = false);
    char *GetEffectName();
    char **GetKnobNames();
    EffectSettings GetEffectSettings();
    void SetEffectSettings(EffectSettings effectSettings);

private:
    const char *knobNames[MAX_KNOBS] = {(char *)"LEVEL", (char *)"DECAY", (char *)"TONE", (char *)"ACCENT"};

    // Beat parameters
    const float bFrequency = 3.0f;
    float decay = 0.0f;
    float tone = 0.0f;
    float accent = 0.0f;

    const float levelMin = 1.0f;
    const float levelMax = 50.0f;
    float level = 25.0f;

    volatile bool doStomp = false;

    Knob levelKnob;
    Knob decayKnob;
    Knob toneKnob;
    Knob accentKnob;

    AnalogBassDrum stomp;
};

#endif