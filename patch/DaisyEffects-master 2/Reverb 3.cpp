#include "Reverb.h"

void Reverb::Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm)
{
    hw = hardware;
    display = daisyDisplay;

    // Get the sample rate for initializing the reverb and calculating the max LP Frequency
    sample_rate = hw->AudioSampleRate();

    // Initialize the knobs
    mixKnob.Init(hw, KNOB_1_CHN, mixLevel);
    decayKnob.Init(hw, KNOB_2_CHN, decay, decayMin, decayMax);
    toneKnob.Init(hw, KNOB_3_CHN, tone, toneMin, sample_rate / 2.0f);

    // Initialize the reverb (check sample rate)
    int ret = -1;
    if (sample_rate > 48000.f)
    {
        ret = verb.Init(sample_rate / 2);
    }
    else
    {
        ret = verb.Init(sample_rate);
    }
    debugPrintlnF(hw, "Verb setup %d", ret);
}

float Reverb::Process(float in)
{
    float out1;
    float out2;
    verb.Process(in, in, &out1, &out2);
    return out1 * mixLevel + in * (1 - mixLevel);
}

void Reverb::Cleanup()
{
}

void Reverb::Loop(bool allowEffectControl)
{
    // Only adjust if we are in edit mode
    if (allowEffectControl)
    {
        // Update the mix level
        if (mixKnob.SetNewValue(mixLevel))
        {
            debugPrintlnF(hw, "Updated the mix level to: %f", mixLevel);
            updateEditModeKnobValue(display, 0, mixLevel);
        }

        // Knob 1 controls the ratio
        if (decayKnob.SetNewValue(decay))
        {
            verb.SetFeedback(decay);
            debugPrintlnF(hw, "Updated the decay to: %f", decay);
            updateEditModeKnobValue(display, 1, decay);

            // Update the effect settings
            effectSettings.knobSettings[0] = decay;
        }

        // Knob 2 controls the threshold
        if (toneKnob.SetNewValue(tone))
        {
            verb.SetLpFreq(tone);
            debugPrintlnF(hw, "Updated the tone to: %f", tone);
            updateEditModeKnobValue(display, 2, tone);

            // Update the effect settings
            effectSettings.knobSettings[1] = tone;
        }
    }
}

char *Reverb::GetEffectName()
{
    return (char *)"REVERB";
}

char **Reverb::GetKnobNames()
{
    return (char **)knobNames;
}

EffectSettings Reverb::GetEffectSettings()
{
    // Update the effect settings
    effectSettings.knobSettings[0] = decay;
    effectSettings.knobSettings[1] = tone;
    effectSettings.knobSettings[2] = mixLevel;

    // Return the settings
    return effectSettings;
}

void Reverb::SetEffectSettings(EffectSettings effectSettings)
{
    // Update boost level from the effect settings
    decay = effectSettings.knobSettings[0];
    tone = effectSettings.knobSettings[1];
    mixLevel = effectSettings.knobSettings[2];

    verb.SetFeedback(decay);
    verb.SetLpFreq(tone);
}