#include "Metronome.h"

void Metronome::Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm)
{
    hw = hardware;
    display = daisyDisplay;

    // Initialize the knobs
    volumeKnob.Init(hw, KNOB_1_CHN, volume, volumeLevelMin, volumeLevelMax);
    tempoKnob.Init(hw, KNOB_2_CHN, tempo, tempoMin, tempoMax);

    // Initialize the toggle
    timeSigToggle.Init(hw->GetPin(effectSPDT1Pin1), hw->GetPin(effectSPDT1Pin2));
    UpdateToggleDisplay();

    // Initialize the tap tempo parameters
    pedalTapTempoBpm = newBpm;

    // Initialize metronome and beat
    float sample_rate = hw->AudioSampleRate();
    tick.Init(GetTempoFromBpm((int)tempo), sample_rate);
    beat.Init(sample_rate);

    // Setup beat
    beat.SetFreq(bFrequency);
    beat.SetDirtiness(dirtiness);
    beat.SetFmEnvelopeAmount(fmEnvelopeAmount);
    beat.SetFmEnvelopeDecay(fmEnvelopeDecay);
    beat.SetDecay(decay);
    beat.SetTone(tone);
    beat.SetAccent(accent);
}

float Metronome::Process(float in)
{
    bool t = tick.Process();

    if (t)
    {
        // Play the down beat if we are on the 1
        if (count == 0)
        {
            beat.SetFreq(dbFrequency);
        }
        else
        {
            beat.SetFreq(bFrequency);
        }

        // Increment and loop the count
        count++;
        if (count >= (int)timeSignature)
        {
            count = 0;
        }
    }

    return in + (beat.Process(t) * volume);
}

void Metronome::Cleanup()
{
}

void Metronome::Loop(bool allowEffectControl)
{
    // Only adjust if we are in edit mode
    if (allowEffectControl)
    {
        // Knob 1 controls the volume level
        if (volumeKnob.SetNewValue(volume))
        {
            debugPrintlnF(hw, "Updated the volume level to: %f", volume);
            updateEditModeKnobValue(display, 0, volume);
        }

        // Knob 2 controls the tempo
        if (tempoKnob.SetNewValue(tempo))
        {
            // Convert tempo float to an int
            //int t = round((int)tempo / 2) * 2;
            int t = round((int)tempo);
            tick.SetFreq(GetTempoFromBpm(t));

            debugPrintlnF(hw, "Updated the tempo to: %d", t);
            updateEditModeKnobValue(display, 1, t);
        }

        // Read the toggle to set the time signature
        if (timeSigToggle.ReadToggle() == 0)
        {
            if (timeSignature != TimeSignature::ThreeFour)
            {
                timeSignature = TimeSignature::ThreeFour;
                count = 0;
                debugPrintln(hw, "Setting time signature to 3/4");
                UpdateToggleDisplay();
            }
        }
        else if (timeSigToggle.ReadToggle() == 1)
        {
            if (timeSignature != TimeSignature::FourFour)
            {
                timeSignature = TimeSignature::FourFour;
                count = 0;
                debugPrintln(hw, "Setting time signature to 4/4");
                UpdateToggleDisplay();
            }
        }
        else
        {
            if (timeSignature != TimeSignature::SixEight)
            {
                timeSignature = TimeSignature::SixEight;
                count = 0;
                debugPrintln(hw, "Setting time signature to 6/8");
                UpdateToggleDisplay();
            }
        }
    }

    // Check for an updated tap tempo
    if (currentTapTempoBpm != *pedalTapTempoBpm)
    {
        currentTapTempoBpm = *pedalTapTempoBpm;

        // Update the tick frequency
        tick.SetFreq(GetTempoFromBpm(currentTapTempoBpm));

        debugPrintlnF(hw, "New Tempo: %d bpm", currentTapTempoBpm);
    }
}

float Metronome::GetTempoFromBpm(int bpm)
{
    return bpm / 60.0f;
}

void Metronome::UpdateToggleDisplay()
{
    switch (timeSignature)
    {
    case TimeSignature::ThreeFour:
        updateEditModeToggleValue(display, (char *)"3/4");
        break;
    case TimeSignature::FourFour:
        updateEditModeToggleValue(display, (char *)"4/4");
        break;
    case TimeSignature::SixEight:
        updateEditModeToggleValue(display, (char *)"6/8");
        break;
    }
}

char *Metronome::GetEffectName()
{
    return (char *)"METRONOME";
}

char **Metronome::GetKnobNames()
{
    return (char **)knobNames;
}

EffectSettings Metronome::GetEffectSettings()
{
    // Add to the effect settings
    effectSettings.knobSettings[0] = volume;
    effectSettings.knobSettings[1] = tempo;

    // Add the time signature to the effect settings
    effectSettings.togglePosition = timeSigToggle.ReadToggle();

    // Return the settings
    return effectSettings;
}

void Metronome::SetEffectSettings(EffectSettings effectSettings)
{
    // Update the effect settings
    volume = effectSettings.knobSettings[0];
    tempo = effectSettings.knobSettings[1];

    // Read the toggle to set the time signature
    if (effectSettings.togglePosition == 0)
    {
        timeSignature = TimeSignature::ThreeFour;
    }
    else if (effectSettings.togglePosition == 1)
    {
        timeSignature = TimeSignature::FourFour;
    }
    else
    {
        timeSignature = TimeSignature::SixEight;
    }
}