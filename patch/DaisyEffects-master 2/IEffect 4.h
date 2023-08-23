#ifndef IEFFECT_H
#define IEFFECT_H

#include "daisy_seed.h"
#include "../../include/PedalConfig.h"
#include "../../include/Storage.h"
#include "../../src/DaisyDisplay.h"

class IEffect
{
public:
    IEffect() {}
    virtual ~IEffect() {}
    virtual void Setup(daisy::DaisySeed *hardware, DaisyDisplay *daisyDisplay, int *newBpm = nullptr) {}
    virtual void Cleanup() {}
    virtual float Process(float in) { return 0; }
    virtual void Loop(bool allowEffectControl, bool isTapPressed = false) {}
    virtual char *GetEffectName() { return (char *)"uh-oh"; }
    virtual char **GetKnobNames() { return (char **)""; }
    virtual EffectSettings GetEffectSettings() { return effectSettings; }
    virtual void SetEffectSettings(EffectSettings effectSettings) {}
    virtual void UpdateToggleDisplay() {}

protected:
    daisy::DaisySeed *hw;
    DaisyDisplay *display;
    EffectSettings effectSettings;
};

#endif