#include "StompBox.h"

void StompBox::Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm)
{
    hw = hardware;
    display = daisyDisplay;

    // Initialize the knobs
    levelKnob.Init(hw, KNOB_1_CHN, level, levelMin, levelMax);
    decayKnob.Init(hw, KNOB_2_CHN, decay);
    toneKnob.Init(hw, KNOB_3_CHN, tone);
    accentKnob.Init(hw, KNOB_4_CHN, accent);

    // Initialize the stomp
    float sample_rate = hw->AudioSampleRate();
    stomp.Init(sample_rate);
    stomp.SetFreq(bFrequency);
}

float StompBox::Process(float in)
{
    // Process the stomp then reset the trigger
    float stompProc = stomp.Process(doStomp);
    doStomp = false;

    return in + (stompProc * level);
}

void StompBox::Cleanup()
{
}

void StompBox::Loop(bool allowEffectControl, bool isTapPressed)
{
    if (isTapPressed)
    {
        // Trigger the stomp to happen
        doStomp = true;
    }

    // Only adjust if we are in edit mode
    if (allowEffectControl)
    {
        // Knob 1 controls the level
        if (levelKnob.SetNewValue(level))
        {
            debugPrintlnF(hw, "Updated the level to: %f", level);
            updateEditModeKnobValue(display, 0, level);

            // Update the effect settings
            effectSettings.knobSettings[0] = level;
        }

        // Knob 1 controls the decay
        if (decayKnob.SetNewValue(decay))
        {
            debugPrintlnF(hw, "Updated the decay to: %f", decay);
            updateEditModeKnobValue(display, 1, decay);
            stomp.SetDecay(decay);

            // Update the effect settings
            effectSettings.knobSettings[1] = decay;
        }

        // Knob 1 controls the tone
        if (toneKnob.SetNewValue(tone))
        {
            debugPrintlnF(hw, "Updated the tone to: %f", tone);
            updateEditModeKnobValue(display, 2, tone);
            stomp.SetTone(tone);

            // Update the effect settings
            effectSettings.knobSettings[2] = tone;
        }

        // Knob 1 controls the accent
        if (accentKnob.SetNewValue(accent))
        {
            debugPrintlnF(hw, "Updated the accent to: %f", accent);
            updateEditModeKnobValue(display, 3, accent);
            stomp.SetAccent(accent);

            // Update the effect settings
            effectSettings.knobSettings[3] = accent;
        }
    }
}

char *StompBox::GetEffectName()
{
    return (char *)"STOMP BOX";
}

char **StompBox::GetKnobNames()
{
    return (char **)knobNames;
}

EffectSettings StompBox::GetEffectSettings()
{
    // Get settings from the effect settings
    effectSettings.knobSettings[0] = level;
    effectSettings.knobSettings[1] = decay;
    effectSettings.knobSettings[2] = tone;
    effectSettings.knobSettings[3] = accent;

    // Return the settings
    return effectSettings;
}

void StompBox::SetEffectSettings(EffectSettings effectSettings)
{
    // Update the effect settings
    level = effectSettings.knobSettings[0];
    decay = effectSettings.knobSettings[1];
    tone = effectSettings.knobSettings[1];
    accent = effectSettings.knobSettings[1];
}