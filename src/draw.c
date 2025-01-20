#include <SDL3/SDL.h>
#include "draw.h"

static void draw_line(Context *ctx, SDL_FPoint start, SDL_FPoint end)
{
    SDL_FPoint points[] = {
        {start.x, start.y},
        {end.x, end.y},
    };
    SDL_RenderLines(ctx->renderer, points, 2);
}

static void draw_box(Context *ctx, SDL_FPoint start, SDL_FPoint end, u8 width)
{
    SDL_FRect rects[MAX_STROKE_WIDTH] = {0};

    SDL_assert(width <= MAX_STROKE_WIDTH);

    for (size_t i = 0; i < width; ++i) {
        rects[i] = (SDL_FRect) {
            .x = start.x + i,
            .y = start.y + i,
            .w = end.x - start.x - i*2,
            .h = end.y - start.y - i*2,
        };
    }
    SDL_RenderRects(ctx->renderer, rects, width);
}

static void draw_box_fill(Context *ctx, SDL_FPoint start, SDL_FPoint end, u8 width)
{

    SDL_assert(width <= MAX_STROKE_WIDTH);

    const SDL_FRect rect = {
        .x = start.x,
        .y = start.y,
        .w = end.x - start.x,
        .h = end.y - start.y
    };

    SDL_RenderFillRect(ctx->renderer, &rect);
}

void draw_symbol(Context *ctx, Symbol *symbol)
{
    SDL_SetRenderDrawColor(ctx->renderer, symbol->color.r, symbol->color.g, symbol->color.b, symbol->color.a);
    switch (symbol->type) {
        case SYMBOL_BOX:
            draw_box(ctx, symbol->start, symbol->end, symbol->size);
            break;
        case SYMBOL_BOX_FILL:
            draw_box_fill(ctx, symbol->start, symbol->end, symbol->size);
            break;
        case SYMBOL_LINE:
            draw_line(ctx, symbol->start, symbol->end);
            break;
        default:
            SDL_Log("Unknown symbol type");
            break;
    }
}
