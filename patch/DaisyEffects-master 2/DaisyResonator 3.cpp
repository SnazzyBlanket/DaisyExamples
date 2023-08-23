#include "DaisyResonator.h"

void DaisyResonator::Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm)
{
    hw = hardware;
    display = daisyDisplay;

    // Get the sample rate for initializing the resonator
    sample_rate = hw->AudioSampleRate();

    // Initialize the mix toggle
    mixToggle.Init(hw->GetPin(effectSPDT1Pin1), hw->GetPin(effectSPDT1Pin2));

    // Initialize the knobs
    brightnessKnob.Init(hw, KNOB_1_CHN, brightness, brightnessMin, brightnessMax);
    structureKnob.Init(hw, KNOB_2_CHN, structure, structureMin, structureMax);
    dampingKnob.Init(hw, KNOB_3_CHN, damping, dampingMin, dampingMax);
    freqKnob.Init(hw, KNOB_4_CHN, freq, freqMin, freqMax);

    // Initialize the balancer
    balance.Init(sample_rate);

    // Initialize the resonator (check sample rate)
    int ret = -1;
    if (sample_rate > 48000.f)
    {
        resonator.Init(0.5f, 12, hw->AudioSampleRate() / 2);
    }
    else
    {
        resonator.Init(0.5f, 12, hw->AudioSampleRate());
    }
    debugPrintlnF(hw, "Resonator setup %d", ret);
}

float DaisyResonator::Process(float in)
{
    float mix = resonator.Process(in) * currMixValue + in * (1 - currMixValue);

    return balance.Process(mix, in);
}

void DaisyResonator::Cleanup()
{
}

void DaisyResonator::Loop(bool allowEffectControl)
{
    // Only adjust if we are in edit mode
    if (allowEffectControl)
    {
        // Knob 1 controls the brightness level
        if (brightnessKnob.SetNewValue(brightness))
        {
            resonator.SetBrightness(brightness);
            debugPrintlnF(hw, "Updated the brightness level to: %f", brightness);
            updateEditModeKnobValue(display, 0, brightness);

            // Update the effect settings
            effectSettings.knobSettings[0] = brightness;
        }

        // Knob 2 controls the structure level
        if (structureKnob.SetNewValue(structure))
        {
            resonator.SetStructure(structure);
            debugPrintlnF(hw, "Updated the structure level to: %f", structure);
            updateEditModeKnobValue(display, 1, structure);

            // Update the effect settings
            effectSettings.knobSettings[1] = structure;
        }

        // Knob 3 controls the damping level
        if (dampingKnob.SetNewValue(damping))
        {
            resonator.SetDamping(damping);
            debugPrintlnF(hw, "Updated the damping level to: %f", damping);
            updateEditModeKnobValue(display, 2, damping);

            // Update the effect settings
            effectSettings.knobSettings[2] = damping;
        }

        // Knob 4 controls the freq level
        if (freqKnob.SetNewValue(freq))
        {
            resonator.SetFreq(freq);
            debugPrintlnF(hw, "Updated the freq level to: %f", freq);
            updateEditModeKnobValue(display, 3, freq);

            // Update the effect settings
            effectSettings.knobSettings[3] = freq;
        }

        // Update the mix
        SetMix();
    }
}

void DaisyResonator::UpdateToggleDisplay()
{
    switch (currMix)
    {
    case 0:
        updateEditModeToggleValue(display, (char *)"HIGH");
        break;
    case 1:
        updateEditModeToggleValue(display, (char *)"MIDDLE");
        break;
    case 2:
        updateEditModeToggleValue(display, (char *)"LOW");
        break;
    }
}

void DaisyResonator::SetMix()
{
    u_int8_t togg = mixToggle.ReadToggle();
    if (togg != currMix)
    {
        currMix = togg;

        switch (currMix)
        {
        case 0:
            currMixValue = mixHigh;
            debugPrintln(hw, "Mix set to high");
            UpdateToggleDisplay();
            break;
        case 1:
            currMixValue = mixMed;
            debugPrintln(hw, "Mix set to middle");
            UpdateToggleDisplay();
            break;
        case 2:
            currMixValue = mixLow;
            debugPrintln(hw, "Mix set to low");
            UpdateToggleDisplay();
            break;
        }
    }
}

char *DaisyResonator::GetEffectName()
{
    return (char *)"RESONATOR";
}

char **DaisyResonator::GetKnobNames()
{
    return (char **)knobNames;
}

EffectSettings DaisyResonator::GetEffectSettings()
{
    // Update the effect settings
    effectSettings.knobSettings[0] = brightness;
    effectSettings.knobSettings[1] = structure;
    effectSettings.knobSettings[2] = damping;
    effectSettings.knobSettings[3] = freq;

    // Add the mix to the effect settings
    effectSettings.togglePosition = mixToggle.ReadToggle();

    // Return the settings
    return effectSettings;
}

void DaisyResonator::SetEffectSettings(EffectSettings effectSettings)
{
    // Update levels from the effect settings
    brightness = effectSettings.knobSettings[0];
    structure = effectSettings.knobSettings[1];
    damping = effectSettings.knobSettings[2];
    freq = effectSettings.knobSettings[3];

    // Update mix from effect settings
    currMix = effectSettings.togglePosition;
    if (currMix == 0)
    {
        currMixValue = mixHigh;
    }
    else if (currMix == 1)
    {
        currMixValue = mixMed;
    }
    else
    {
        currMixValue = mixLow;
    }
}