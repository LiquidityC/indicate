#ifndef _GUI_H_
#define _GUI_H_

#include "common.h"

int gui_init(Context *ctx);

void gui_process_event(SDL_Event *event);

bool gui_has_keyboard(Context *ctx);

bool gui_has_mouse(Context *ctx);

void gui_update(Context *ctx);

void gui_render(Context *ctx);

void gui_shutdown(Context *ctx);

#endif  // _GUI_H_

