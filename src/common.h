#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#define CLIPBOARD_SIZE (1024 * 1024)

#include "SDL.h"
#include "SDL_image.h"
#include <stdbool.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

typedef unsigned char u8;

typedef enum SymbolType {
    SYMBOL_BOX,
    SYMBOL_LINE,
} SymbolType;

typedef struct Symbol {
    SymbolType type;
    SDL_FPoint start;
    SDL_FPoint end;
    struct {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    } color;
} Symbol;

typedef struct UserAction {
    bool lbutton_down;
    bool draw_symbol;
    bool take_screenshot;
    bool rbutton_press;
    bool undo;
} UserAction;

typedef struct Context {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *image;
    SymbolType active_symbol_type;
    Symbol active_symbol;
    u8 clipboard_data[CLIPBOARD_SIZE];
    size_t clipboard_data_len;
    UserAction action;
    struct {
        ImGuiContext *ctx;
        ImGuiIO *io;
        float fg_color[3];
    } imgui;
} Context;

#endif  // _CONTEXT_H_

