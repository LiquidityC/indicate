#include "gui.h"

extern bool ImGui_ImplSDL3_InitForSDLRenderer(SDL_Window *window, SDL_Renderer *renderer);
extern void ImGui_ImplSDL3_ProcessEvent(const SDL_Event *event);
extern void ImGui_ImplSDL3_NewFrame();
extern void ImGui_ImplSDL3_Shutdown();

extern bool ImGui_ImplSDLRenderer3_Init(SDL_Renderer *renderer);
extern void ImGui_ImplSDLRenderer3_Shutdown();
extern void ImGui_ImplSDLRenderer3_NewFrame();
extern void ImGui_ImplSDLRenderer3_RenderDrawData(ImDrawData* draw_data, SDL_Renderer* renderer);

int gui_init(Context *ctx)
{
    int result = -1;

    /* Setup dear imgui context */
    ctx->imgui.ctx = igCreateContext(NULL);
    ctx->imgui.io = igGetIO();
    ctx->imgui.io->IniFilename = NULL;

    if (!ImGui_ImplSDL3_InitForSDLRenderer(ctx->window, ctx->renderer)) {
        SDL_Log("ImGui_ImplSDL3_InitForSDLRenderer failed");
        goto out;
    }
    if (!ImGui_ImplSDLRenderer3_Init(ctx->renderer)) {
        SDL_Log("ImGui_ImplSDLRenderer3_Init failed");
        goto out;
    }

    result = 0;
out:
    return result;
}

void gui_process_event(SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
}

bool gui_has_keyboard(Context *ctx)
{
    return ctx->imgui.io->WantCaptureKeyboard;
}

bool gui_has_mouse(Context *ctx)
{
    return ctx->imgui.io->WantCaptureMouse;
}

void gui_update(Context *ctx)
{
        /* ImGui frame start */
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        igNewFrame();

        if (ctx->image == NULL) {
            igBegin("Image", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

            int w, h;
            SDL_GetWindowSize(ctx->window, &w, &h);

            igText("No image in clipboard");
            igSetWindowPos_Vec2((ImVec2) { (float) w/2 -  igGetWindowWidth()/2, (float) h/2 - igGetWindowHeight()/2 }, 0);
            igEnd();
            return;
        }

        if (ctx->action.rbutton_press) {
            igBegin("Controls", &ctx->action.rbutton_press, ImGuiWindowFlags_NoMove);

            igColorEdit3("Color", ctx->imgui.fg_color, 0);

            igRadioButton_IntPtr("Box", (int *) &ctx->opt.symbol_type, SYMBOL_BOX);
            igRadioButton_IntPtr("Line", (int*) &ctx->opt.symbol_type, SYMBOL_LINE);

            if (ctx->opt.symbol_type == SYMBOL_BOX) {
                igSliderInt("Stroke", (int*) &ctx->opt.size, MIN_STROKE_WIDTH, MAX_STROKE_WIDTH, "%d px", 0);
            }

            if (igButton("Undo", (ImVec2) {50, 20})) {
                ctx->action.undo = true;
            }

            igSetWindowPos_Vec2((ImVec2) {0, 0}, 0);
            igSetWindowSize_Vec2((ImVec2) { 0, 0 }, 0);
            igEnd();
        }
}

void gui_render(Context *ctx)
{
    /* ImGui frame end */
    igRender();
    ImGui_ImplSDLRenderer3_RenderDrawData(igGetDrawData(), ctx->renderer);
}

void gui_shutdown(Context *ctx)
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    igDestroyContext(ctx->imgui.ctx);
}
