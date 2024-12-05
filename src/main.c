#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "gui.h"
#include "draw.h"

#define MIME_TYPES_LEN 1
static const char *mime_types[MIME_TYPES_LEN] = { "image/png" };

#define TEXTURE_COUNT 10

#define SYMBOL_COUNT 10

static Symbol symbols[SYMBOL_COUNT] = {0};
static size_t symbol_ptr = 0;

#define Err(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

static const void* read_clipboard_data(void *userdata, const char *mime_type, size_t *size)
{
    Context *ctx = userdata;
    void *data = NULL;

    if (ctx->clipboard_data_len == 0) {
        goto out;
    }

    printf("Clipboard data requested: %s\n", mime_type);

    if (strcmp(mime_type, "image/png") == 0) {
        *size = ctx->clipboard_data_len;
        data = ctx->clipboard_data;
    }

out:
    return data;
}

static void clean_clipboard_data(void *userdata)
{
    Context *ctx = userdata;

    /* Only clean it it isn't this app that is providing the data. Eg. We're
     * not actively taking a screenshot */
    if (!ctx->action.take_screenshot) {
        memset(ctx->clipboard_data, 0, sizeof(ctx->clipboard_data));
        ctx->clipboard_data_len = 0;
        symbol_ptr = 0;
        memset(symbols, 0, sizeof(symbols));
        ctx->clipboard_data_provider = false;
    }
}

static bool read_image_from_clipboard(Context *ctx)
{
    bool result = false;
    SDL_IOStream *stream = NULL;
    u8 *data = NULL;
    size_t len = 0;
    float w, h;

    data = SDL_GetClipboardData("image/png", &len);
    if (data == NULL) {
        Err("SDL_GetClipboardData failed: %s", SDL_GetError());
        goto out;
    }

    /* Clean up the current image */
    if (ctx->image != NULL) {
        SDL_DestroyTexture(ctx->image);
        ctx->image = NULL;
    }

    stream = SDL_IOFromMem(data, len);
    if (stream == NULL) {
        Err("SDL_IOFromMem failed: %s", SDL_GetError());
        goto out;
    }

    ctx->image = IMG_LoadTexture_IO(ctx->renderer, stream, true);
    if (ctx->image == NULL) {
        Err("IMG_LoadTexture_IO failed: %s", SDL_GetError());
        goto out;
    }

    if (!SDL_GetTextureSize(ctx->image, &w, &h)) {
        Err("SDL_GetTextureSize failed: %s", SDL_GetError());
        goto out;
    }

    if (!SDL_SetWindowSize(ctx->window, w, h)) {
        Err("SDL_SetWindowSize failed: %s", SDL_GetError());
        goto out;
    }

    result = true;
out:
    if (data != NULL) {
        SDL_free(data);
    }

    return result;
}

static int init_sdl(Context *ctx)
{
    static const int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;

    int result = -1;

    memset(ctx, 0, sizeof(*ctx));

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Err("SDL_Init failed: %s", SDL_GetError());
        goto out;
    }

    if (!SDL_CreateWindowAndRenderer(PROGRAM_NAME " " PROGRAM_VERSION, 640, 480, 0, &ctx->window, &ctx->renderer)) {
        Err("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        goto out;
    }

    result = IMG_Init(img_flags);
    if (result != img_flags) {
        Err("IMG_Init failed: %s", SDL_GetError());
        result = -1;
        goto out;
    }


    result = 0;
out:
    return result;
}

static void handle_events(Context *ctx, bool *quit)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            *quit = true;
            break;
        }

        gui_process_event(&event);

        if (!gui_has_keyboard(ctx)){
            switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        *quit = true;
                        break;
                    } else if (event.key.key == SDLK_Z && event.key.mod & SDL_KMOD_CTRL) {
                        ctx->action.undo = true;
                    } else if (event.key.key == SDLK_1) {
                        ctx->opt.symbol_type = SYMBOL_BOX;
                    } else if (event.key.key == SDLK_2) {
                        ctx->opt.symbol_type = SYMBOL_BOX_FILL;
                    } else if (event.key.key == SDLK_3) {
                        ctx->opt.symbol_type = SYMBOL_LINE;
                    } else if (event.key.key == SDLK_SPACE) {
                        size_t size;
                        const uint8_t *data;
                        data = SDL_GetClipboardData("image/bmp", &size);
                        if (size > 0) {
                            SDL_Log("Clipboard has image/bmp data: %zu bytes", size);
                        } else {
                            SDL_Log("Clipboard does not have image/bmp data");
                        }

                        if (data) {
                            printf("Data received\n");
                        }
                    }
                    break;

            }
        }

        if (!gui_has_mouse(ctx)) {
            switch (event.type) {
                case SDL_EVENT_CLIPBOARD_UPDATE:
                    if (!ctx->clipboard_data_provider && SDL_HasClipboardData("image/png")) {
                        if (!read_image_from_clipboard(ctx)) {
                            Err("read_image_from_clipboard failed");
                        }
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        ctx->active_symbol.color.r = 255 * ctx->imgui.fg_color[0];
                        ctx->active_symbol.color.g = 255 * ctx->imgui.fg_color[1];
                        ctx->active_symbol.color.b = 255 * ctx->imgui.fg_color[2];
                        ctx->active_symbol.color.a = 255;
                        ctx->active_symbol.start.x = event.button.x;
                        ctx->active_symbol.start.y = event.button.y;
                        ctx->active_symbol.type = ctx->opt.symbol_type;
                        ctx->active_symbol.size = ctx->opt.size;
                        ctx->action.lbutton_down = true;
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        ctx->action.rbutton_press ^= true;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if (ctx->action.lbutton_down) {
                        ctx->active_symbol.end.x = event.button.x;
                        ctx->active_symbol.end.y = event.button.y;
                        ctx->action.draw_symbol = true;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        ctx->action.lbutton_down = false;
                        ctx->action.draw_symbol = false;
                        ctx->action.take_screenshot = true;
                        if (symbol_ptr < SYMBOL_COUNT) {
                            memcpy(&symbols[symbol_ptr], &ctx->active_symbol, sizeof(ctx->active_symbol));
                            symbol_ptr++;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

static void run(Context *ctx)
{
    bool quit = false;

    /* We don't get a clipboard update event when the program starts, so we need to check manually */
    if (SDL_HasClipboardData("image/png")) {
        read_image_from_clipboard(ctx);
    }

    while (!quit) {
        handle_events(ctx, &quit);

        gui_update(ctx);

        if (ctx->action.undo) {
            if (symbol_ptr > 0) {
                symbol_ptr--;
            }
            ctx->action.undo = false;
            ctx->action.take_screenshot = true;
        }

        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx->renderer);

        if (ctx->image != NULL) {
            SDL_RenderTexture(ctx->renderer, ctx->image, NULL, NULL);
            for (size_t i = 0; i < symbol_ptr; i++) {
                draw_symbol(ctx, &symbols[i]);
            }
            if (ctx->action.draw_symbol) {
                draw_symbol(ctx, &ctx->active_symbol);
            }
        }

        if (ctx->action.take_screenshot) {
            SDL_Surface *surf = SDL_RenderReadPixels(ctx->renderer, NULL);
            if (surf != NULL) {
                SDL_IOStream *stream = SDL_IOFromMem(ctx->clipboard_data, CLIPBOARD_SIZE);
                IMG_SavePNG_IO(surf, stream, false);
                ctx->clipboard_data_len = SDL_TellIO(stream);
                SDL_CloseIO(stream);
                SDL_DestroySurface(surf);

                ctx->clipboard_data_provider = true;
                SDL_SetClipboardData(read_clipboard_data, clean_clipboard_data, ctx, mime_types, MIME_TYPES_LEN);
            }

            /* It's importante that the flag is reset after the clipboard has
             * been updated.
             *
             * @see clean_clipboard_data */
            ctx->action.take_screenshot = false;
        }

        gui_render(ctx);

        SDL_RenderPresent(ctx->renderer);
    }
}

static void set_context_defaults(Context *ctx)
{
    ctx->imgui.fg_color[0] = 1.0f; // Red
    ctx->opt.size = DEFAULT_STROKE_WIDTH;
    ctx->opt.symbol_type = SYMBOL_BOX;
}

int main(int argc, char *argv[]) {

    Context ctx;

    int result;

    result = init_sdl(&ctx);
    if (result != 0) {
        goto out;
    }

    result = gui_init(&ctx);
    if (result != 0) {
        goto out;
    }

    set_context_defaults(&ctx);

    run(&ctx);

out:

    if (ctx.image != NULL) {
        SDL_DestroyTexture(ctx.image);
    }
    if (ctx.renderer != NULL) {
        SDL_DestroyRenderer(ctx.renderer);
    }
    if (ctx.window != NULL) {
        SDL_DestroyWindow(ctx.window);
    }

    gui_shutdown(&ctx);

    SDL_Quit();

    return result;
}
