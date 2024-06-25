#include "SDL.h"
#include "draw.h"

static void draw_line(Context *ctx, SDL_FPoint start, SDL_FPoint end)
{
    SDL_FPoint points[] = {
        {start.x, start.y},
        {end.x, end.y},
    };
    SDL_RenderLines(ctx->renderer, points, 2);
}

static void draw_box(Context *ctx, SDL_FPoint start, SDL_FPoint end)
{
    SDL_FRect rect = {
        .x = start.x,
        .y = start.y,
        .w = end.x - start.x,
        .h = end.y - start.y,
    };
    SDL_RenderRect(ctx->renderer, &rect);
}

void draw_symbol(Context *ctx, Symbol *symbol)
{
    SDL_SetRenderDrawColor(ctx->renderer, symbol->color.r, symbol->color.g, symbol->color.b, symbol->color.a);
    switch (symbol->type) {
        case SYMBOL_BOX:
            draw_box(ctx, symbol->start, symbol->end);
            break;
        case SYMBOL_LINE:
            draw_line(ctx, symbol->start, symbol->end);
            break;
        default:
            SDL_Log("Unknown symbol type");
            break;
    }
}
