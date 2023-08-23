#ifndef LOOPER_WINDOW_H
#define LOOPER_WINDOW_H

#include "windows.h"

// Looper Window class
class looper_window : public windows
{
public:
    virtual void draw() override;
    virtual void update() override;
    virtual all_windows get_window_id() override;
    virtual void on_btn_pressed(buttons id) override;
    virtual void on_btn_holded(buttons id) override;
    virtual void on_enc_turned(RotaryEncoder::Direction dir) override;
};

// create a single instance
extern looper_window looper_window_ins;

#endif