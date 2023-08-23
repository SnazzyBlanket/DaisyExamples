#include "effect_select_window.h"
#include "src/display.h"
#include "src/command.h"
#include "src/effect_modules/effects_rack.h"

effect_select_window effect_select_window_ins;

void effect_select_window::draw()
{
    // Draw topic and bottom text boxes
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 10);
    u8g2.drawBox(0, 55, 128, 9);

    // Draw effect names
    u8g2.setFont(u8g2_font_6x10_mr);
    for (uint8_t i = cur_page * 4 + 1; i < EFFECTS_AMOUNT && i < cur_page * 4 + 5 ; i++)
    {
        unsigned char pos = (i - 1) % 4;
        if (i == cur_effect_id)
        {
            u8g2.setDrawColor(0);
            u8g2.drawStr(24, 19 + 11 * pos, effects_rack.effects_arr[i]->effect_name);
            u8g2.setDrawColor(1);
            u8g2.drawVLine(23, 11 + 11 * pos, 10);
        }
        else
        {
            u8g2.drawStr(24, 19 + 11 * pos, effects_rack.effects_arr[i]->effect_name);
        }
        if (effects_rack.effect_used[i])
        {
            u8g2.setDrawColor(0);
            u8g2.drawStr(103, 19 + 11 * pos, "USED");
            u8g2.setDrawColor(1);
            u8g2.drawVLine(102, 11 + 11 * pos, 10);
        }
    }

    // Draw page number
    u8g2.setFont(u8g2_font_6x12_mr);
    u8g2.setCursor(1, 35);
    u8g2.print(cur_page + 1);
    u8g2.drawStr(7, 35, "/");
    u8g2.setCursor(13, 35);
    u8g2.print((EFFECTS_AMOUNT-2) / 4 + 1);

    // Draw page arrows
    for (uint8_t i = 0; i <= 3; i++)
    {
        u8g2.drawPixel(10 - i, 16 + i - page_arrow_state);
        u8g2.drawPixel(10 + i, 16 + i - page_arrow_state);
    }
    for (uint8_t i = 0; i <= 3; i++)
    {
        u8g2.drawPixel(10 - i, 48 - i + page_arrow_state);
        u8g2.drawPixel(10 + i, 48 - i + page_arrow_state);
    }

    // Draw topic
    u8g2.setDrawColor(0);
    u8g2.drawStr(1, 8, "Select an effect:");
    // Draw bottom text
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.drawStr(1, 62, " CANCEL/---- | ENTER/----");

    // Draw 8x8 matrix
    u8g2_8x8.setFont(u8g2_font_profont10_mr);
    u8g2_8x8.setCursor(2, 6);
    u8g2_8x8.print(effects_rack.cur_preset);
}

void effect_select_window::update()
{
    // Update animations
    time++;
    if (time > 3)
    {
        page_arrow_state = !page_arrow_state;
        time = 0;
    }

    // Update page
    if (cur_effect_id > 0) // starts from 1 since 0 is empty effect
    {
        cur_page = (cur_effect_id - 1) / 4;
    }
}

all_windows effect_select_window::get_window_id()
{
    return EFFECT_SELECT_WINDOW;
}

void effect_select_window::on_btn_pressed(buttons id)
{
    switch (id)
    {
    case BTN_LEFT:
        insert_command(CMD_UI_EFFECT_PAGE_UP);
        break;

    case BTN_RIGHT:
        insert_command(CMD_UI_EFFECT_PAGE_DOWN);
        break;

    case BTN_UP:
        insert_command(CMD_UI_EFFECT_CUR_UP);
        break;

    case BTN_DOWN:
        insert_command(CMD_UI_EFFECT_CUR_DOWN);
        break;

    case BTN_OK:
        insert_command(CMD_UI_EFFECT_CUR_SELECT);
        break;
    
    case BTN_ENCODER:
        insert_command(CMD_UI_EFFECT_CUR_CANCEL);
        break;

    default:
        break;
    }
}

void effect_select_window::on_btn_holded(buttons id)
{
}

void effect_select_window::on_enc_turned(RotaryEncoder::Direction dir)
{
}