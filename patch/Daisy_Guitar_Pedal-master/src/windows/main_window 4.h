#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "windows.h"

// Main Window class
class main_window : public windows
{
public:
    virtual void draw() override;
    virtual void update() override;
    virtual all_windows get_window_id() override;
    virtual void on_btn_pressed(buttons id) override;
    virtual void on_btn_holded(buttons id) override;
    virtual void on_enc_turned(RotaryEncoder::Direction dir) override;
    uint8_t cur_effect = 0;
    uint8_t cur_page = 0;

private:
    int time = 0;
};

// create a single instance
extern main_window main_window_ins;

#endif