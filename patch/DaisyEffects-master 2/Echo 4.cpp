#include "Echo.h"

void Echo::Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm)
{
    hw = hardware;
    display = daisyDisplay;
    sample_rate = hw->AudioSampleRate();

    // Init Delay Line
    del_line.Init();

    // Set Delay Time in Samples
    currentTempoSamples = CalculateSampleFromBpm(initialTempoBpm);
    del_line.SetDelay(currentTempoSamples);

    // Initialize the tap tempo parameters
    pedalTapTempoBpm = newBpm;

    // Initialize the knobs
    mixLevelKnob.Init(hw, KNOB_1_CHN, mixLevel, mixLevelMin, mixLevelMax);
    decayKnob.Init(hw, KNOB_2_CHN, decayValue, minDecayValue, maxDecayValue);
    tempoKnob.Init(hw, KNOB_3_CHN, tempo, tempoMin, tempoMax);

    // Initialize the type
    typeSwitcher.Init(hw->GetPin(effectSPDT1Pin1), hw->GetPin(effectSPDT1Pin2));
    TypeSwitcherLoopControl();
    UpdateToggleDisplay();
}

float Echo::Process(float in)
{
    float dry, wet;

    // Read Dry from I/O
    dry = in;

    // Read Wet from Delay Line
    wet = del_line.Read();

    // Write to Delay with a controlled decay time
    del_line.Write((wet * decayValue) + dry);

    // Mix Dry and Wet and send to I/O
    return wet * mixLevel + dry * (1 - mixLevel);
}

void Echo::Cleanup()
{
    del_line.Reset();
}

void Echo::Loop(bool allowEffectControl)
{
    // Only adjust if we are in edit mode
    if (allowEffectControl)
    {
        // Update the effect level if the knob has been moved
        if (mixLevelKnob.SetNewValue(mixLevel))
        {
            debugPrintlnF(hw, "Updated the mix level to: %f", mixLevel);
            updateEditModeKnobValue(display, 0, mixLevel);
        }

        // Update the decay if the knob has been moved
        if (decayKnob.SetNewValue(decayValue))
        {
            debugPrintlnF(hw, "Updated the decay level to: %f", decayValue);
            updateEditModeKnobValue(display, 1, decayValue);
        }

        // Update the tempo if the knob has been moved
        if (tempoKnob.SetNewValue(tempo))
        {
            // Round tempo to nearest 2 and set the new delay
            int bpm = round((int)tempo / 2) * 2;
            currentTempoSamples = CalculateSampleFromBpm(bpm);
            del_line.SetDelay(currentTempoSamples);

            debugPrintlnF(hw, "Updated the tempo to: %d bpm", bpm);
            updateEditModeKnobValue(display, 2, bpm);
        }

        // Handle type
        TypeSwitcherLoopControl();
    }

    // Check for an updated tap tempo
    if (currentTapTempoBpm != *pedalTapTempoBpm)
    {
        currentTapTempoBpm = *pedalTapTempoBpm;

        // Set the new delay based on the calculated duration
        currentTempoSamples = CalculateSampleFromBpm(currentTapTempoBpm);
        del_line.SetDelay(currentTempoSamples);
    }
}

// Handle reading the SPDT switch and setting the delay type
void Echo::TypeSwitcherLoopControl()
{
    // Determine which type is selected
    if (typeSwitcher.ReadToggle() == 0)
    {
        // Only set the type if we have a new one
        if (currentDelayType != QUARTER)
        {
            debugPrintln(hw, "Changing to quarter note delay");

            // Set the delay type and tempo modifier
            currentDelayType = QUARTER;
            tempoModifier = 1.0f;

            // Update the delay tempo
            del_line.SetDelay(currentTempoSamples * tempoModifier);

            UpdateToggleDisplay();
        }
    }
    else if (typeSwitcher.ReadToggle() == 2)
    {
        // Only set the type if we have a new one
        if (currentDelayType != TRIPLET)
        {
            debugPrintln(hw, "Changing to triplet note delay");

            // Set the delay type and tempo modifier
            currentDelayType = TRIPLET;
            tempoModifier = 0.333f;

            // Update the delay tempo
            del_line.SetDelay(currentTempoSamples * tempoModifier);

            UpdateToggleDisplay();
        }
    }
    else
    {
        // Only set the type if we have a new one
        if (currentDelayType != DOTTED_EIGHTH)
        {
            debugPrintln(hw, "Changing to dotted eighth note delay");

            // Set the delay type and tempo modifier
            currentDelayType = DOTTED_EIGHTH;
            tempoModifier = 0.75f;

            // Update the delay tempo
            del_line.SetDelay(currentTempoSamples * tempoModifier);

            UpdateToggleDisplay();
        }
    }
}

float Echo::CalculateSampleFromBpm(int bpm)
{
    return (sample_rate) * (60.f / (float)bpm);
}

char *Echo::GetEffectName()
{
    return (char *)"ECHO";
}

char **Echo::GetKnobNames()
{
    return (char **)knobNames;
}

void Echo::UpdateToggleDisplay()
{
    switch (currentDelayType)
    {
    case QUARTER:
        updateEditModeToggleValue(display, (char *)"QUARTER");
        break;
    case DOTTED_EIGHTH:
        updateEditModeToggleValue(display, (char *)"DOT EIGHT");
        break;
    case TRIPLET:
        updateEditModeToggleValue(display, (char *)"TRIPLET");
        break;
    case DT_UNSET:
        break;
    }
}

EffectSettings Echo::GetEffectSettings()
{
    // Add levels to the effect settings
    effectSettings.knobSettings[0] = mixLevel;
    effectSettings.knobSettings[1] = decayValue;
    effectSettings.knobSettings[2] = tempo;

    // Add the wave shape to the effect settings
    effectSettings.togglePosition = typeSwitcher.ReadToggle();

    // Return the settings
    return effectSettings;
}

void Echo::SetEffectSettings(EffectSettings effectSettings)
{
    // Update delay type from effect settings
    if (effectSettings.togglePosition == 0)
    {
        // Set the delay type and tempo modifier
        currentDelayType = QUARTER;
        tempoModifier = 1.0f;

        // Update the delay tempo
        del_line.SetDelay(currentTempoSamples * tempoModifier);
    }
    else if (effectSettings.togglePosition == 2)
    {
        // Set the delay type and tempo modifier
        currentDelayType = TRIPLET;
        tempoModifier = 0.333f;

        // Update the delay tempo
        del_line.SetDelay(currentTempoSamples * tempoModifier);
    }
    else
    {
        // Set the delay type and tempo modifier
        currentDelayType = DOTTED_EIGHTH;
        tempoModifier = 0.75f;

        // Update the delay tempo
        del_line.SetDelay(currentTempoSamples * tempoModifier);
    }

    // Update levels from effect settings
    mixLevel = effectSettings.knobSettings[0];
    decayValue = effectSettings.knobSettings[1];
    tempo = effectSettings.knobSettings[2];

    // Update tempo from speed knob
    currentTempoSamples = ((delayMaxSize / tempo) * 30) * tempoModifier;
    del_line.SetDelay(currentTempoSamples);
}