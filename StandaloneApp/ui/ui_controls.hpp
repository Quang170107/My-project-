#pragma once
#include "../game.hpp"

inline std::vector<Game::UiBtn> Game::BuildButtons() {
    std::vector<UiBtn> b;
    Color green = { 36, 104, 62, 255 }, teal = { 36, 92, 118, 255 }, blue = { 36, 60, 120, 255 }, purple = { 70, 50, 120, 255 }, red = { 120, 40, 50, 255 }, grey = { 44, 50, 72, 255 };
    switch (state) {
        case TITLE:
            if (showHelp) {
                b.push_back({ { 540, 650, 200, 44 }, L("CLOSE", "ĐÓNG"), UI_CLOSE_HELP, grey });
            } else {
                b.push_back({ { 470, 392, 340, 56 }, L("NEW GAME", "CHƠI MỚI"), UI_START, green });
                b.push_back({ { 470, 458, 340, 48 }, L("LOAD GAME (4 SLOTS)", "TẢI GAME (4 Ô LƯU)"), UI_SAVE_LOAD, teal });
                b.push_back({ { 470, 516, 166, 44 }, L("SANCTUARY", "THÁNH ĐIỆN"), UI_SANCT, purple });
                b.push_back({ { 644, 516, 166, 44 }, L("SETTINGS", "CÀI ĐẶT"), UI_SETTINGS, blue });
                b.push_back({ { 470, 570, 166, 40 }, L("CONTROLS", "ĐIỀU KHIỂN"), UI_HELP, grey });
#ifndef __ANDROID__
                b.push_back({ { 644, 570, 166, 40 }, L("QUIT", "THOÁT"), UI_QUIT, red });
#endif
            }
            break;
        case PAUSED:
            if (showHelp) {
                b.push_back({ { 540, 650, 200, 44 }, L("CLOSE", "ĐÓNG"), UI_CLOSE_HELP, grey });
            } else {
                float y = 202;
                b.push_back({ { 480, y, 320, 34 }, L("RESUME", "TIẾP TỤC"), UI_RESUME, green }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("SAVE / LOAD GAME", "LƯU / TẢI GAME"), UI_SAVE_LOAD, teal }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("EQUIPMENT & SKILLS", "TRANG BỊ & KỸ NĂNG"), UI_EQUIP, blue }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("WORLD MAP & KINGDOM", "BẢN ĐỒ THẾ GIỚI & LÃNH ĐỊA"), UI_WORLD_MAP, teal }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("FAMILY TREE [Y]", "CÂY GIA PHẢ [Y]"), UI_FAMILY_TREE, purple }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("CHANGE HERO", "ĐỔI TƯỚNG"), UI_CHANGE_CLASS, purple }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("SETTINGS", "CÀI ĐẶT"), UI_SETTINGS, blue }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("RESTART RUN", "CHƠI LẠI"), UI_RESTART, purple }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("CONTROLS", "ĐIỀU KHIỂN"), UI_HELP, grey }); y += 38;
                b.push_back({ { 480, y, 320, 34 }, L("MAIN MENU", "MENU CHÍNH"), UI_MAIN_MENU, red });
            }
            break;
        case GAME_OVER:
            if (!children.empty()) {
                b.push_back({ { 430, 444, 420, 42 }, L("CONTINUE AS HEIR (REST IN PEACE)", "TIẾP TỤC ĐỜI CON (REST IN PEACE)"), UI_CONTINUE_HEIR, green });
            }
            b.push_back({ { 510, 496, 260, 44 }, L("PLAY AGAIN", "CHƠI LẠI"), UI_RESTART, red });
            b.push_back({ { 510, 548, 260, 40 }, L("CHANGE HERO", "ĐỔI TƯỚNG"), UI_CHANGE_CLASS, blue });
            b.push_back({ { 510, 596, 260, 40 }, L("SANCTUARY", "THÁNH ĐIỆN"), UI_SANCT, purple });
            b.push_back({ { 510, 644, 260, 40 }, L("LOAD SAVE", "TẢI BẢN LƯU"), UI_SAVE_LOAD, teal });
            break;
        case INVENTORY:
            b.push_back({ { 590, 622, 256, 40 }, L("WORLD MAP & FIEF [TAB]", "BẢN ĐỒ THẾ GIỚI [TAB]"), UI_WORLD_MAP, teal });
            b.push_back({ { 856, 622, 174, 40 }, L("FAMILY TREE [Y]", "CÂY GIA PHẢ [Y]"), UI_FAMILY_TREE, purple });
            b.push_back({ { 1040, 622, 146, 40 }, L("CLOSE", "ĐÓNG"), UI_CLOSE_INV, grey });
            break;
        case META:
        case SHOP:
        case SAVE_LOAD:
        case SURFACE_UI:
        case FAMILY_TREE:
        case WORLD_MAP:
            b.push_back({ { 30, 28, 150, 44 }, L("< BACK", "< QUAY LẠI"), UI_BACK, grey });
            break;
        default:
            break;
    }
    return b;
}

inline int Game::HitTestButtons(Vector2 m) {
    if ((state == TITLE || state == PAUSED) && showHelp) return UI_CLOSE_HELP; // chạm vào đâu cũng đóng bảng hướng dẫn
    for (const auto& bt : BuildButtons()) if (CheckCollisionPointRec(m, bt.r)) return bt.action;
    return UI_NONE;
}

inline int Game::HitButton() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return UI_NONE;
    return HitTestButtons(GetMousePosition());
}

inline void Game::HandleUiAction(int a) {
    switch (a) {
        case UI_START: showHelp = false; classSel = selectedClass; state = CLASS_SELECT; PlaySound(snd.sndPotion); break;
        case UI_SAVE_LOAD: showHelp = false; OpenSaveLoad(state); break;
        case UI_CHANGE_CLASS: showHelp = false; classSel = selectedClass; state = CLASS_SELECT; PlaySound(snd.sndPotion); break;
        case UI_SANCT: OpenSanctuary(state == GAME_OVER ? GAME_OVER : TITLE); break;
        case UI_SETTINGS: settingsReturn = (state == PAUSED) ? PAUSED : TITLE; setSel = 0; state = SETTINGS; break;
        case UI_HELP: showHelp = true; break;
        case UI_CLOSE_HELP: showHelp = false; break;
        case UI_QUIT: quitRequested = true; break;
        case UI_MAIN_MENU: showHelp = false; state = TITLE; break;
        case UI_RESUME: showHelp = false; state = PLAYING; break;
        case UI_EQUIP: prevState = PAUSED; state = INVENTORY; break;
        case UI_RESTART: showHelp = false; RestartRun(); break;
        case UI_FAMILY_TREE: showHelp = false; OpenFamilyTree(state); break;
        case UI_WORLD_MAP: showHelp = false; OpenWorldMap(state); break;
        case UI_CONTINUE_HEIR: showHelp = false; if (!children.empty()) SuccessionToChild(OldestChildIdx(), true); break;
        case UI_BACK:
            if (state == META) CloseSanctuary();
            else if (state == SHOP || state == SURFACE_UI) state = PLAYING;
            else if (state == FAMILY_TREE) state = familyTreeReturn;
            else if (state == WORLD_MAP) state = worldMapReturn;
            else if (state == SAVE_LOAD) state = saveLoadReturn;
            break;
        case UI_CLOSE_INV: state = prevState; break;
        default: break;
    }
}

inline void Game::DrawButtons() {
    Vector2 m = GetMousePosition();
    for (const auto& bt : BuildButtons()) {
        bool hover = CheckCollisionPointRec(m, bt.r) && !touchMode;
        DrawRectangleRec(bt.r, hover ? Shade(bt.col, 1.35f) : bt.col);
        DrawRectangleLinesEx(bt.r, 2.0f, hover ? WHITE : Shade(bt.col, 2.1f));
        int fs = bt.r.height >= 50 ? 26 : 20;
        DrawText(bt.label.c_str(), (int)(bt.r.x + bt.r.width / 2) - MeasureText(bt.label.c_str(), fs) / 2, (int)(bt.r.y + bt.r.height / 2) - fs / 2, fs, WHITE);
    }
}

inline void Game::PollTouches() {
    touches.clear();
    if (useFakeTouches) {
        for (size_t i = 0; i < fakeTouches.size(); ++i) touches.push_back({ (int)i, fakeTouches[i] });
    } else {
        int n = GetTouchPointCount();
        for (int i = 0; i < n; ++i) {
            Vector2 p = GetTouchPosition(i);
            touches.push_back({ GetTouchPointId(i), { (p.x - presentDx) / presentScale, (p.y - presentDy) / presentScale } });
        }
    }
    if (!touches.empty()) touchMode = true;
}

inline void Game::PollTouchControls() {
    PollTouches();
    if (!touchMode) return;
    if (state != PLAYING) { joyId = -1; touchDashWas = touchNovaWas = touchSkill2Was = touchUltWas = touchUiWas = false; return; }

    bool atk = false, dashNow = false, novaNow = false, skill2Now = false, ultNow = false, uiNow = false, pauseTap = false, gearTap = false, tradeTap = false;
    bool joyStillDown = false;
    for (const auto& t : touches) if (t.first == joyId) joyStillDown = true;
    if (!joyStillDown) joyId = -1;

    for (const auto& t : touches) {
        Vector2 p = t.second;
        if (CheckCollisionPointCircle(p, AtkC(), kAtkR * 1.15f)) { atk = true; continue; }
        if (CheckCollisionPointCircle(p, DashC(), kDashR * 1.15f)) { dashNow = true; continue; }
        if (CheckCollisionPointCircle(p, NovaC(), kNovaR * 1.15f)) { novaNow = true; continue; }
        if (CheckCollisionPointCircle(p, Skill2C(), kNovaR * 1.15f)) { skill2Now = true; continue; }
        if (CheckCollisionPointCircle(p, UltC(), kNovaR * 1.15f)) { ultNow = true; continue; }
        if (CheckCollisionPointCircle(p, PauseC(), kSmallR * 1.4f)) { uiNow = true; pauseTap = !touchUiWas; continue; }
        if (CheckCollisionPointCircle(p, GearC(), kSmallR * 1.4f)) { uiNow = true; gearTap = !touchUiWas; continue; }
        if (shopFloor && Vector2Distance(player.pos, merchantPos) < 130.0f && CheckCollisionPointRec(p, TradeRect())) { uiNow = true; tradeTap = !touchUiWas; continue; }
        if (t.first == joyId) continue;
        if (joyId == -1 && p.x < 640.0f) { joyId = t.first; joyOrigin = p; }
    }
    touchUiWas = uiNow;

    if (pauseTap) { StopRumble(); state = PAUSED; return; }
    if (gearTap) { prevState = PLAYING; state = INVENTORY; return; }
    if (tradeTap) { state = SHOP; shopSel = 0; PlaySound(snd.sndPotion); return; }

    Vector2 mv = { 0, 0 };
    if (joyId != -1) {
        for (const auto& t : touches) {
            if (t.first != joyId) continue;
            Vector2 d = Vector2Subtract(t.second, joyOrigin);
            float len = Vector2Length(d);
            if (len > 90.0f) { joyOrigin = Vector2Subtract(t.second, Vector2Scale(d, 90.0f / len)); d = Vector2Subtract(t.second, joyOrigin); len = 90.0f; }
            if (len > 12.0f) mv = Vector2Scale(d, 1.0f / 90.0f);
            joyPos = t.second;
        }
    }
    bool dashEdge = dashNow && !touchDashWas, novaEdge = novaNow && !touchNovaWas;
    bool skill2Edge = skill2Now && !touchSkill2Was, ultEdge = ultNow && !touchUltWas;
    touchDashWas = dashNow;
    touchNovaWas = novaNow;
    touchSkill2Was = skill2Now;
    touchUltWas = ultNow;
    if (joyId == -1 && !atk && !dashEdge && !novaEdge && !skill2Edge && !ultEdge) return;

    // tự ngắm vào quái gần nhất
    float best = 420.0f; bool found = false;
    for (const auto& e : enemies) {
        if (e.hp <= 0) continue;
        float d = Vector2Distance(player.pos, e.pos);
        if (d < best) { best = d; padAim = Vector2Normalize(Vector2Subtract(e.pos, player.pos)); found = true; }
    }
    if (!found && Vector2Length(mv) > 0.3f) padAim = Vector2Normalize(mv);
    ctl.active = true;
    if (Vector2Length(mv) > 1.0f) mv = Vector2Normalize(mv);
    ctl.move = mv;
    ctl.aim = padAim;
    ctl.attack = atk;
    ctl.dash = dashEdge;
    ctl.nova = novaEdge;
    ctl.skill2 = skill2Edge;
    ctl.skill3 = false;
    ctl.skill4 = false;
    ctl.ult = ultEdge;
}

inline void Game::DrawTouchControls() {
    auto ring = [&](Vector2 c, float r, Color col, float frac) {
        if (frac >= 0.999f) DrawRing(c, r - 7.0f, r - 2.0f, 0.0f, 360.0f, 40, Fade(col, 0.85f));
        else DrawRing(c, r - 7.0f, r - 2.0f, -90.0f, -90.0f + 360.0f * std::max(0.0f, frac), 40, Fade(col, 0.85f));
    };
    // cần ảo
    Vector2 jo = (joyId != -1) ? joyOrigin : Vector2{ 150, 590 };
    DrawCircleV(jo, 90.0f, Fade(WHITE, joyId != -1 ? 0.10f : 0.06f));
    DrawCircleLines((int)jo.x, (int)jo.y, 90.0f, Fade(WHITE, 0.35f));
    Vector2 knob = (joyId != -1) ? joyPos : jo;
    if (joyId != -1) { Vector2 dd = Vector2Subtract(knob, jo); if (Vector2Length(dd) > 90.0f) knob = Vector2Add(jo, Vector2Scale(Vector2Normalize(dd), 90.0f)); }
    DrawCircleV(knob, 38.0f, Fade(WHITE, joyId != -1 ? 0.35f : 0.18f));

    // nút chém
    Vector2 a = AtkC();
    DrawCircleV(a, kAtkR, Fade({ 220, 70, 70, 255 }, 0.30f));
    DrawCircleLines((int)a.x, (int)a.y, kAtkR, Fade({ 255, 150, 150, 255 }, 0.7f));
    DrawItemIcon(SLOT_WEAPON, a, 62.0f, { 255, 225, 225, 255 }, hasEquip[SLOT_WEAPON] ? equipped[SLOT_WEAPON].weaponType : 0);
    // lướt
    Vector2 d = DashC();
    float dashFrac = player.dashCooldown > 0 ? 1.0f - std::max(0.0f, player.dashTimer) / player.dashCooldown : 1.0f;
    DrawCircleV(d, kDashR, Fade({ 240, 220, 80, 255 }, 0.25f));
    ring(d, kDashR, { 255, 230, 90, 255 }, dashFrac);
    DrawText("DASH", (int)d.x - MeasureText("DASH", 18) / 2, (int)d.y - 9, 18, Fade(WHITE, 0.95f));
    // Skill 1 (Q)
    Vector2 n = NovaC();
    float qFrac = player.specialCooldown > 0 ? 1.0f - std::max(0.0f, player.specialTimer) / player.specialCooldown : 1.0f;
    DrawCircleV(n, kNovaR, Fade({ 80, 190, 255, 255 }, 0.25f));
    ring(n, kNovaR, { 110, 210, 255, 255 }, qFrac);
    DrawText("SK1", (int)n.x - MeasureText("SK1", 18) / 2, (int)n.y - 9, 18, Fade(WHITE, 0.95f));
    // Skill 2 (E)
    Vector2 s2 = Skill2C();
    float eFrac = player.skill2Cooldown > 0 ? 1.0f - std::max(0.0f, player.skill2Timer) / player.skill2Cooldown : 1.0f;
    DrawCircleV(s2, kNovaR, Fade({ 90, 220, 150, 255 }, 0.25f));
    ring(s2, kNovaR, { 110, 235, 175, 255 }, eFrac);
    DrawText("SK2", (int)s2.x - MeasureText("SK2", 18) / 2, (int)s2.y - 9, 18, Fade(WHITE, 0.95f));
    // Ultimate (R)
    Vector2 u = UltC();
    float rFrac = std::min(1.0f, player.rage / player.maxRage);
    DrawCircleV(u, kNovaR, Fade(rFrac >= 1.0f ? GOLD : ORANGE, rFrac >= 1.0f ? 0.38f : 0.22f));
    ring(u, kNovaR, rFrac >= 1.0f ? GOLD : ORANGE, rFrac);
    DrawText("ULT", (int)u.x - MeasureText("ULT", 18) / 2, (int)u.y - 9, 18, Fade(WHITE, 0.95f));
    // tạm dừng / trang bị
    Vector2 pc = PauseC(), gc = GearC();
    DrawCircleV(pc, kSmallR, Fade(BLACK, 0.45f));
    DrawCircleLines((int)pc.x, (int)pc.y, kSmallR, Fade(WHITE, 0.6f));
    DrawRectangle((int)pc.x - 8, (int)pc.y - 9, 5, 18, Fade(WHITE, 0.9f));
    DrawRectangle((int)pc.x + 3, (int)pc.y - 9, 5, 18, Fade(WHITE, 0.9f));
    DrawCircleV(gc, kSmallR, Fade(BLACK, 0.45f));
    DrawCircleLines((int)gc.x, (int)gc.y, kSmallR, Fade(WHITE, 0.6f));
    DrawItemIcon(SLOT_ARMOR, gc, 26.0f, { 220, 230, 255, 255 });
    // nút thương nhân
    if (shopFloor && Vector2Distance(player.pos, merchantPos) < 130.0f) {
        Rectangle tr = TradeRect();
        DrawRectangleRec(tr, Fade({ 60, 40, 100, 255 }, 0.9f));
        DrawRectangleLinesEx(tr, 3.0f, GOLD);
        DrawText("TRADE", (int)(tr.x + tr.width / 2) - MeasureText("TRADE", 28) / 2, (int)tr.y + 14, 28, GOLD);
    }
}

inline void Game::Rumble(float strong, float weak, float duration) {
    if (!settings.vibration || !IsGamepadAvailable(0)) return;
    RumbleRaw(strong, weak);
    rumbleTimer = duration;
}

inline void Game::PollGamepad() {
    if (ctl.external) return;
    ctl.active = false;
    ctl.attack = ctl.dash = ctl.nova = ctl.skill2 = ctl.skill3 = ctl.skill4 = ctl.ult = false;
    ctl.move = { 0, 0 };
    if (!IsGamepadAvailable(0)) return;
    auto dz = [](float v) { return fabsf(v) < 0.2f ? 0.0f : v; };
    Vector2 mv = { dz(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X)), dz(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)) };
    Vector2 stick = { GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X), GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) };
    bool aimStick = Vector2Length(stick) > 0.35f;
    float rt = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER);
    float lt = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER);
    bool atk = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || rt > 0.3f;
    bool ltDown = lt > 0.5f;
    bool dashBtn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || (ltDown && !padLtWas);
    padLtWas = ltDown;
    bool novaBtn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
    bool skill2Btn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
    bool ultBtn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
    if (Vector2Length(mv) < 0.01f && !aimStick && !atk && !dashBtn && !novaBtn && !skill2Btn && !ultBtn) return; // tay cầm đang rảnh: dùng bàn phím/chuột

    if (aimStick) {
        padAim = Vector2Normalize(stick);
    } else {
        // tự ngắm quái gần nhất trong tầm
        float best = 380.0f; bool found = false;
        for (const auto& e : enemies) {
            if (e.hp <= 0) continue;
            float d = Vector2Distance(player.pos, e.pos);
            if (d < best) { best = d; padAim = Vector2Normalize(Vector2Subtract(e.pos, player.pos)); found = true; }
        }
        if (!found && Vector2Length(mv) > 0.3f) padAim = Vector2Normalize(mv);
    }
    ctl.active = true;
    if (Vector2Length(mv) > 1.0f) mv = Vector2Normalize(mv);
    ctl.move = mv;
    ctl.aim = padAim;
    ctl.attack = atk;
    ctl.dash = dashBtn;
    ctl.nova = novaBtn;
    ctl.skill2 = skill2Btn;
    ctl.ult = ultBtn;
}
