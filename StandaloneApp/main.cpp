#include "core/fonts_audio.hpp"
#include "core/game_data.hpp"
#include "game.hpp"

#include "systems/dynasty_surface.hpp"
#include "systems/dungeon_loop.hpp"
#include "systems/combat_skills.hpp"
#include "systems/items_meta_settings.hpp"
#include "systems/save_load.hpp"
#include "render/entity_sprites.hpp"
#include "render/world_render.hpp"
#include "ui/ui_controls.hpp"
#include "ui/ui_menus.hpp"
#include "ui/ui_surface_dynasty.hpp"

int main() {
#ifndef __ANDROID__
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
#endif
    InitWindow(1280, 720, "Geometric Dungeon - Standalone Roguelike RPG");
#ifndef __ANDROID__
    SetWindowMinSize(640, 360);
#endif
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // Esc dùng để tạm dừng, không thoát ngay

    Game game;
    game.Init();

    while (!WindowShouldClose() && !game.quitRequested) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;
        game.Update(dt);
        game.Draw();
    }

    game.StopRumble();
    game.snd.Close();
    CloseGameFont();
    CloseWindow();
    return 0;
}
