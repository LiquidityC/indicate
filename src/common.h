#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#define CLIPBOARD_SIZE (1024 * 1024)

#include "SDL.h"
#include "SDL_image.h"
#include <stdbool.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

typedef unsigned char u8;

typedef struct Context {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *image;
    SDL_FRect box;
    u8 clipboard_data[CLIPBOARD_SIZE];
    size_t clipboard_data_len;
    struct {
        bool lbutton_down;
        bool draw_box;
        bool take_screenshot;
        bool rbutton_press;
    } action;
    struct {
        ImGuiContext *ctx;
        ImGuiIO *io;
        float box_color[3];
    } imgui;
} Context;

#endif  // _CONTEXT_H_

