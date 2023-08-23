#ifndef EFFECT_PARAM_WINDOW_H
#define EFFECT_PARAM_WINDOW_H

#include "windows.h"
#include "src/effect_modules/effects.h"

// Effect Parameters Window class
class effect_param_window : public windows
{
public:
    virtual void draw() override;
    virtual void update() override;
    virtual all_windows get_window_id() override;
    virtual void on_btn_pressed(buttons id) override;
    virtual void on_btn_holded(buttons id) override;
    virtual void on_enc_turned(RotaryEncoder::Direction dir) override;
    uint8_t cur_param = 0;
    uint8_t cur_step = 1;

private:
    int time = 0;
    int step_arrows_state = false;
    void print_param(char mode, param_true_val true_val, char prec, const char* unit); // print out the parameter with units
};

// create a single instance
extern effect_param_window effect_param_window_ins;

#endif