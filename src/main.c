#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_image.h"

typedef unsigned char u8;

#define CLIPBOARD_SIZE (1024 * 1024)

typedef struct Context {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *image;
    SDL_FRect box;
    u8 clipboard_data[CLIPBOARD_SIZE];
    size_t clipboard_data_len;
    bool button_down;
    bool draw_box;
    bool take_screenshot;
} Context;

#define MIME_TYPES_LEN 1
static const char *mime_types[MIME_TYPES_LEN] = { "image/png" };

static SDL_FRect boxes[10];
static size_t box_ptr = 0;

static const void* read_clipboard_data(void *userdata, const char *mime_type, size_t *size)
{
    SDL_Log("%s", __func__);

    Context *ctx = userdata;
    void *data = NULL;

    if (ctx->clipboard_data_len == 0) {
        goto out;
    }

    if (strcmp(mime_type, "image/png") == 0) {
        *size = ctx->clipboard_data_len;
        data = ctx->clipboard_data;
    }

out:
    return data;
}

static void clean_clipboard_data(void *userdata)
{
    // Pass
}

static int read_image_from_clipboard(Context *ctx)
{
    int result = -1;
    SDL_IOStream *stream = NULL;
    u8 *data = NULL;
    size_t len = 0;
    float w, h;

    SDL_Log("Clipboard has image/png data");
    data = SDL_GetClipboardData("image/png", &len);
    if (data == NULL) {
        SDL_Log("SDL_GetClipboardData failed: %s", SDL_GetError());
        goto out;
    }

    stream = SDL_IOFromMem(data, len);
    if (stream == NULL) {
        SDL_Log("SDL_IOFromMem failed: %s", SDL_GetError());
        goto out;
    }

    ctx->image = IMG_LoadTexture_IO(ctx->renderer, stream, SDL_TRUE);
    if (ctx->image == NULL) {
        SDL_Log("IMG_LoadTexture_IO failed: %s", IMG_GetError());
        goto out;
    }

    result = SDL_GetTextureSize(ctx->image, &w, &h);
    if (result != 0) {
        SDL_Log("SDL_GetTextureSize failed: %s", SDL_GetError());
        goto out;
    }

    result = SDL_SetWindowSize(ctx->window, w, h);
    if (result != 0) {
        SDL_Log("SDL_SetWindowSize failed: %s", SDL_GetError());
        goto out;
    }

    result = 0;
out:
    if (data != NULL) {
        SDL_free(data);
    }

    return result;
}

static int init_sdl(Context *ctx)
{
    static const int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;

    int result = 0;

    memset(ctx, 0, sizeof(*ctx));

    result = SDL_Init(SDL_INIT_VIDEO);
    if (result != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        goto out;
    }

    result = SDL_CreateWindowAndRenderer("cbdraw", 640, 480, 0, &ctx->window, &ctx->renderer);
    if (result != 0) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        goto out;
    }

    result = IMG_Init(img_flags);
    if (result != img_flags) {
        SDL_Log("IMG_Init failed: %s", IMG_GetError());
        goto out;
    }

    const char *renderer_name = SDL_GetRendererName(ctx->renderer);
    SDL_Log("Renderer: %s\n", renderer_name);

    result = 0;
out:
    return result;
}

static void handle_events(Context *ctx, bool *quit)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                *quit = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    *quit = true;
                }
                break;
            case SDL_EVENT_CLIPBOARD_UPDATE:
                if (ctx->image == NULL && SDL_HasClipboardData("image/png")) {
                    if (read_image_from_clipboard(ctx) != 0) {
                        SDL_Log("read_image_from_clipboard failed");
                    }
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    ctx->box.x = event.button.x;
                    ctx->box.y = event.button.y;
                    ctx->button_down = true;
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (ctx->button_down) {
                    ctx->box.w = event.motion.x - ctx->box.x;
                    ctx->box.h = event.motion.y - ctx->box.y;
                    ctx->draw_box = true;
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                ctx->button_down = false;
                ctx->draw_box = false;
                if (box_ptr < 10) {
                    boxes[box_ptr++] = ctx->box;
                }
                ctx->take_screenshot = true;
                break;
            default:
                break;
        }
    }
}


static void run(Context *ctx)
{
    bool quit = false;

    while (!quit) {
        handle_events(ctx, &quit);

        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx->renderer);

        if (ctx->image != NULL) {
            SDL_RenderTexture(ctx->renderer, ctx->image, NULL, NULL);
        }

        SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255);
        if (ctx->draw_box) {
            SDL_RenderRect(ctx->renderer, &ctx->box);
        }

        if (box_ptr > 0) {
            SDL_RenderRects(ctx->renderer, boxes, box_ptr);
        }

        if (ctx->take_screenshot) {

            /* TODO:
             * SDL_RenderReadPixels doesn't seem to be working on wayland/opengl.
             * Once that's figured out this should write to clipboard and not to a file.
             */
            SDL_Log("Rendering to png");
            SDL_Surface *surf = SDL_RenderReadPixels(ctx->renderer, NULL);
            if (surf != NULL) {
                SDL_Log("Saving screenshot.png");
                IMG_SavePNG(surf, "/tmp/screenshot.png");

                SDL_IOStream *stream = SDL_IOFromMem(ctx->clipboard_data, CLIPBOARD_SIZE);
                IMG_SavePNG_IO(surf, stream, SDL_FALSE);
                ctx->clipboard_data_len = SDL_TellIO(stream);
                SDL_CloseIO(stream);
                SDL_DestroySurface(surf);

                SDL_SetClipboardData(read_clipboard_data, clean_clipboard_data, ctx, mime_types, MIME_TYPES_LEN);
            }

            ctx->take_screenshot = false;
        }

        SDL_RenderPresent(ctx->renderer);
    }
}

int main(int argc, char *argv[]) {

    Context ctx;
    int result;

    result = init_sdl(&ctx);
    if (result != 0) {
        goto out;
    }

    run(&ctx);

out:
    if (ctx.renderer != NULL) {
        SDL_DestroyRenderer(ctx.renderer);
    }
    if (ctx.window != NULL) {
        SDL_DestroyWindow(ctx.window);
    }
    SDL_Quit();

    return result;
}
