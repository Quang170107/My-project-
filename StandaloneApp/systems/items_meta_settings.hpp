#pragma once
#include "../game.hpp"

inline void Game::BuildPillars() {
    pillars.clear();
    switch (layout) {
        case LAY_SCATTERED:
            pillars.push_back({ -350, -100 });
            pillars.push_back({ 350, -100 });
            pillars.push_back({ -200, 120 });
            pillars.push_back({ 200, 120 });
            if (floor % 5 != 0) pillars.push_back({ 0, 0 });
            break;
        case LAY_RING:
            for (int i = 0; i < 8; ++i) {
                float a = i * (2.0f * PI / 8.0f) + PI / 8.0f;
                pillars.push_back({ cosf(a) * 250.0f, -20.0f + sinf(a) * 190.0f });
            }
            break;
        case LAY_BLOCKS:
            for (int i = 0; i < 3; ++i) {
                Vector2 c = (i == 0) ? Vector2{ -330, -110 } : (i == 1 ? Vector2{ 330, -110 } : Vector2{ 0, 40 });
                pillars.push_back({ c.x - 58.0f, c.y });
                pillars.push_back({ c.x + 58.0f, c.y });
                pillars.push_back({ c.x, c.y + 92.0f });
            }
            break;
        case LAY_BASTION:
            for (int i = -3; i <= 3; ++i) if (i != 0) pillars.push_back({ i * 66.0f, -70.0f });
            pillars.push_back({ -200, 60 });
            pillars.push_back({ 200, 60 });
            break;
        default:
            break;
    }
}

inline bool Game::ResolvePillars(Vector2& pos, float radius) {
    bool hit = false;
    for (const auto& pl : pillars) {
        Vector2 d = Vector2Subtract(pos, pl);
        float dist = Vector2Length(d);
        float minD = 27.0f + radius;
        if (dist < minD) {
            if (dist < 0.001f) { d = { 1, 0 }; dist = 1.0f; }
            pos = Vector2Add(pl, Vector2Scale(d, minD / dist));
            hit = true;
        }
    }
    return hit;
}

inline const char* Game::ModLabel() {
    switch (floorMod) {
        case MOD_SWARM: return " - SWARM!";
        case MOD_ELITE: return " - ELITE HUNT!";
        case MOD_SNIPER: return " - SNIPER NEST!";
        case MOD_TREASURE: return " - TREASURE VAULT";
        default: return "";
    }
}

inline void Game::UpdateChests(float dt) {
    for (auto& c : chests) {
        if (c.opened) { if (c.anim < 1.0f) c.anim = std::min(1.0f, c.anim + dt * 4.0f); continue; }
        if (Vector2Distance(c.pos, player.pos) < 42.0f) {
            c.opened = true;
            PlaySound(snd.sndPortal);
            Burst({ c.pos.x, c.pos.y - 10.0f }, { 255, 217, 64, 255 }, 24, 300.0f, 0.7f);
            Shake(0.1f, 3.0f);
            loots.push_back({ 1, { c.pos.x - 22.0f, c.pos.y + 6.0f }, 15, 20.0f });
            loots.push_back({ 1, { c.pos.x + 22.0f, c.pos.y + 6.0f }, 15, 20.0f });
            loots.push_back({ 2, { c.pos.x, c.pos.y - 28.0f }, 26, 20.0f });
            loots.push_back({ 0, { c.pos.x, c.pos.y + 26.0f }, 50, 20.0f });
            SpawnPopup({ c.pos.x, c.pos.y - 30.0f }, "+GOLD", YELLOW);
            if (c.vault) { // kho báu: chắc chắn có trang bị hiếm trở lên (và một relic nếu còn)
                GiveRandomRelic();
                ItemDrop d;
                d.item = GenerateItem(floor + 1, ClassWeaponType(player.heroClass), LifestealScale());
                for (int tries = 0; tries < 30 && d.item.rarity < 1; ++tries) d.item = GenerateItem(floor + 1, ClassWeaponType(player.heroClass), LifestealScale());
                d.pos = { c.pos.x, c.pos.y + 50.0f };
                d.life = 90.0f;
                d.phase = Rand01() * 6.0f;
                itemDrops.push_back(d);
            }
            if (c.cursed) { // Rương Nguyền Rủa: thả đồ hiếm + Linh Thú Hộ Vệ, triệu hồi Tinh Anh hộ vệ
                ItemDrop d;
                d.item = GenerateItem(floor + 2, ClassWeaponType(player.heroClass), LifestealScale());
                for (int tries = 0; tries < 30 && d.item.rarity < 1; ++tries) d.item = GenerateItem(floor + 2, ClassWeaponType(player.heroClass), LifestealScale());
                d.pos = { c.pos.x, c.pos.y + 48.0f };
                d.life = 90.0f;
                d.phase = Rand01() * 6.0f;
                itemDrops.push_back(d);
                GrantOrUpgradeFamiliar();
                Enemy guard = BuildEnemy(BRUTE, { c.pos.x + (c.pos.x >= 0 ? -55.0f : 55.0f), c.pos.y });
                guard.isElite = true;
                guard.eliteAffix = 1 + (rand() % 4);
                guard.maxHp *= 0.85f;
                guard.hp = guard.maxHp;
                enemies.push_back(guard);
                NovaVisual nv; nv.pos = guard.pos; nv.maxRadius = 90.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = RED;
                novas.push_back(nv);
                Burst(c.pos, RED, 28, 300.0f, 0.7f);
            }
        }
    }
}

inline void Game::StockShop() {
    for (int i = 0; i < 4; ++i) {
        shopSlots[i] = ShopSlot();
        shopSlots[i].item = GenerateItem(floor, ClassWeaponType(player.heroClass), LifestealScale());
        shopSlots[i].price = std::max(1, (int)roundf(ItemPrice(shopSlots[i].item) * (1.0f - 0.05f * meta[6])));
    }
    shopSlots[4] = ShopSlot();
    shopSlots[4].potion = true;
    shopSlots[4].heal = 40.0f + floor * 5.0f;
    shopSlots[4].price = std::max(1, (int)roundf((15 + floor * 3) * (1.0f - 0.05f * meta[6])));
    shopSel = 0;
    shopHinted = false;
}

inline void Game::BuyShopSlot(int i) {
    if (i < 0 || i > 4) return;
    ShopSlot& s = shopSlots[i];
    if (s.sold) return;
    if (s.potion && player.hp >= player.maxHp) { ShowBanner(L("HP is already full!", "Máu đã đầy!"), 1.5f); return; }
    if (!s.potion && hasEquip[s.item.slot] && s.item.Score() <= equipped[s.item.slot].Score()) {
        ShowBanner(L("Not an upgrade - you already wear better!", "Bạn đang mặc món mạnh hơn rồi!"), 2.0f);
        return;
    }
    if (player.gold < s.price) { ShowBanner(L("Not enough Gold!", "Không đủ Vàng!"), 1.5f); PlaySound(snd.sndHurt); return; }
    player.gold -= s.price;
    if (s.potion) {
        player.hp = std::min(player.maxHp, player.hp + s.heal);
        ShowBanner(std::string(L("Healed ", "Đã hồi ")) + std::to_string((int)s.heal) + L(" HP!", " Máu!"), 2.0f);
    } else {
        int soldOld = 0;
        if (TryEquipItem(s.item, soldOld)) ShowBanner(std::string(L("EQUIPPED: ", "ĐÃ TRANG BỊ: ")) + equipped[s.item.slot].name, 2.5f);
        else { player.gold += s.item.SellValue(); ShowBanner(std::string(L("Bought & sold ", "Đã mua & bán ")) + s.item.name, 2.0f); }
    }
    PlaySound(snd.sndCoin);
    s.sold = true;
}

inline void Game::UpdateShop() {
    { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
    if (EscPressed() || IsKeyPressed(KEY_F) || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP)) { state = PLAYING; return; }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) shopSel = (shopSel + 1) % 5;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) shopSel = (shopSel + 4) % 5;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) BuyShopSlot(shopSel);
    for (int i = 0; i < 5; ++i) if (IsKeyPressed(KEY_ONE + i)) { shopSel = i; BuyShopSlot(i); }
    Vector2 m = GetMousePosition();
    for (int i = 0; i < 5; ++i) {
        Rectangle r = { 116.0f + i * 212.0f, 170.0f, 200.0f, 340.0f };
        if (CheckCollisionPointRec(m, r)) {
            if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) shopSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { shopSel = i; BuyShopSlot(i); }
        }
    }
}

inline void Game::AwardSouls() {
    lastRunSouls = (int)roundf((floor * 10 + enemiesSlain / 2 + bossesKilled * 25 + (floor > bestFloor ? 20 : 0)) * DiffSouls());
    souls += lastRunSouls;
    runs++;
    SaveSettings();
}

inline void Game::BuyMeta(int i) {
    if (i < 0 || i >= 8) return;
    if (meta[i] >= kMeta[i].maxLevel) { metaMsg = "Already at max level"; metaMsgTimer = 2.0f; return; }
    int cost = MetaCost(i);
    if (souls < cost) { metaMsg = "Not enough Souls"; metaMsgTimer = 2.0f; PlaySound(snd.sndHurt); return; }
    souls -= cost;
    meta[i]++;
    SaveSettings();
    PlaySound(snd.sndLevelUp);
    metaMsg = std::string(kMeta[i].name) + " upgraded to level " + std::to_string(meta[i]) + "!";
    metaMsgTimer = 2.5f;
}

inline void Game::OpenSanctuary(State from) {
    metaReturn = from;
    metaSel = 0;
    metaMsg.clear();
    state = META;
}

inline void Game::CloseSanctuary() {
    if (metaReturn == TITLE) { RestartRun(); state = TITLE; } // áp dụng nâng cấp mới cho lượt chơi sắp bắt đầu
    else state = metaReturn;
}

inline void Game::UpdateMeta(float dt) {
    { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
    if (metaMsgTimer > 0) metaMsgTimer -= dt;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) metaSel = (metaSel + 1) % 8;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) metaSel = (metaSel + 7) % 8;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) BuyMeta(metaSel);
    for (int i = 0; i < 8; ++i) if (IsKeyPressed(KEY_ONE + i)) { metaSel = i; BuyMeta(i); }
    Vector2 m = GetMousePosition();
    for (int i = 0; i < 8; ++i) {
        Rectangle r = { 190.0f, 150.0f + i * 56.0f, 900.0f, 50.0f };
        if (CheckCollisionPointRec(m, r)) {
            if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) metaSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { metaSel = i; BuyMeta(i); }
        }
    }
    if (EscPressed() || IsKeyPressed(KEY_BACKSPACE)) CloseSanctuary();
}

inline void Game::LoadSettings() {
    char* txt = LoadFileText("settings.cfg"); // trên Android raylib tự đọc từ bộ nhớ trong của ứng dụng
    if (!txt) return;
    std::istringstream in(txt);
    UnloadFileText(txt);
    std::string line;
    while (std::getline(in, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        int v = atoi(line.substr(eq + 1).c_str());
        if (k == "resW") settings.resW = v;
        else if (k == "resH") settings.resH = v;
        else if (k == "windowMode") settings.windowMode = std::max(0, std::min(2, v));
        else if (k == "fullscreen") settings.windowMode = (v == 1 ? 2 : (v == 2 ? 1 : 0));
        else if (k == "vsync") settings.vsync = v != 0;
        else if (k == "fpsIndex") settings.fpsIndex = std::max(0, std::min(4, v));
        else if (k == "showFps") settings.showFps = v != 0;
        else if (k == "vibration") settings.vibration = v != 0;
        else if (k == "lighting") settings.lighting = v != 0;
        else if (k == "particles") settings.particles = std::max(0, std::min(2, v));
        else if (k == "vignette") settings.vignette = v != 0;
        else if (k == "volume") settings.volume = std::max(0, std::min(10, v));
        else if (k == "difficulty") settings.difficulty = std::max(0, std::min(2, v));
        else if (k == "language") settings.language = std::max(0, std::min(1, v));
        else if (k == "best") bestFloor = std::max(1, v);
        else if (k == "souls") souls = std::max(0, v);
        else if (k == "runs") runs = std::max(0, v);
        else if (k == "heroClass") selectedClass = std::max(0, std::min(kNumClasses - 1, v));
        else if (k.size() == 5 && k.compare(0, 4, "meta") == 0) {
            int mi = k[4] - '0';
            if (mi >= 0 && mi < 8) meta[mi] = std::max(0, std::min(kMeta[mi].maxLevel, v));
        }
    }
}

inline void Game::SaveSettings() {
    std::ostringstream o;
    o << "resW=" << settings.resW << "\nresH=" << settings.resH << "\nwindowMode=" << settings.windowMode
      << "\nvsync=" << settings.vsync << "\nfpsIndex=" << settings.fpsIndex << "\nshowFps=" << settings.showFps
      << "\nvibration=" << settings.vibration << "\nlighting=" << settings.lighting << "\nparticles=" << settings.particles
      << "\nvignette=" << settings.vignette << "\nvolume=" << settings.volume << "\ndifficulty=" << settings.difficulty
      << "\nlanguage=" << settings.language
      << "\nbest=" << bestFloor << "\nsouls=" << souls << "\nruns=" << runs << "\nheroClass=" << selectedClass;
    for (int i = 0; i < 8; ++i) o << "\nmeta" << i << "=" << meta[i];
    o << "\n";
    std::string s = o.str();
    SaveFileText("settings.cfg", (char*)s.c_str());
}

inline void Game::BuildResList() {
    resList.clear();
#ifdef __ANDROID__
    resList.push_back(0);
    return;
#endif
    int mw = GetMonitorWidth(GetCurrentMonitor()), mh = GetMonitorHeight(GetCurrentMonitor());
    for (int i = 0; i < 5; ++i)
        if (kResOptions[i][0] <= mw && kResOptions[i][1] <= mh) resList.push_back(i);
    if (resList.empty()) resList.push_back(0);
    // Nếu độ phân giải đã lưu không còn khả dụng thì quay về lựa chọn lớn nhất hiện có
    bool ok = false;
    for (int idx : resList) if (kResOptions[idx][0] == settings.resW && kResOptions[idx][1] == settings.resH) ok = true;
    if (!ok) { settings.resW = kResOptions[resList.back()][0]; settings.resH = kResOptions[resList.back()][1]; }
}

inline void Game::ApplySettings() {
#ifndef __ANDROID__
    if (lastAppliedWinMode != settings.windowMode || lastAppliedW != settings.resW || lastAppliedH != settings.resH) {
        if (IsWindowFullscreen()) ToggleFullscreen();
        if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();

        int mon = GetCurrentMonitor();
        int mw = GetMonitorWidth(mon), mh = GetMonitorHeight(mon);
        Vector2 mpos = GetMonitorPosition(mon);

        if (settings.windowMode == 1) {
            // Windowed Fullscreen thực sự: không viền và cao hơn màn hình 1px để driver OpenGL không tự chiếm Exclusive Fullscreen (Alt+Tab tức thì!)
            SetWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowPosition((int)mpos.x, (int)mpos.y);
            SetWindowSize(mw, mh + 1);
        } else if (settings.windowMode == 2) {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowSize(mw, mh);
            if (!IsWindowFullscreen()) ToggleFullscreen();
        } else {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowSize(settings.resW, settings.resH);
            SetWindowPosition((int)mpos.x + std::max(0, (mw - settings.resW) / 2), (int)mpos.y + std::max(0, (mh - settings.resH) / 2));
        }
        lastAppliedWinMode = settings.windowMode;
        lastAppliedW = settings.resW;
        lastAppliedH = settings.resH;
    }
    if (settings.vsync) SetWindowState(FLAG_VSYNC_HINT); else ClearWindowState(FLAG_VSYNC_HINT);
#endif
    SetTargetFPS(kFpsOptions[settings.fpsIndex]);
    SetMasterVolume(settings.volume / 10.0f);
}

inline void Game::ChangeSetting(int row, int d) {
#ifdef __ANDROID__
    if (row <= 2) return; // độ phân giải, toàn màn hình, VSync do hệ điều hành quản lý
#endif
    switch (row) {
        case 0: {
            int cur = 0;
            for (size_t i = 0; i < resList.size(); ++i)
                if (kResOptions[resList[i]][0] == settings.resW && kResOptions[resList[i]][1] == settings.resH) cur = (int)i;
            cur = (cur + d + (int)resList.size()) % (int)resList.size();
            settings.resW = kResOptions[resList[cur]][0];
            settings.resH = kResOptions[resList[cur]][1];
            break;
        }
        case 1: settings.windowMode = (settings.windowMode + d + 3) % 3; break;
        case 2: settings.vsync = !settings.vsync; break;
        case 3: settings.fpsIndex = (settings.fpsIndex + d + 5) % 5; break;
        case 4: settings.showFps = !settings.showFps; break;
        case 5: settings.vibration = !settings.vibration; break;
        case 6: settings.lighting = !settings.lighting; break;
        case 7: settings.particles = (settings.particles + d + 3) % 3; break;
        case 8: settings.vignette = !settings.vignette; break;
        case 9: settings.volume = std::max(0, std::min(10, settings.volume + d)); break;
        case 10: settings.difficulty = (settings.difficulty + d + 3) % 3; break;
        case 11: settings.language = (settings.language + d + 2) % 2; break;
    }
}

inline std::string Game::SettingValue(int row) {
#ifdef __ANDROID__
    if (row <= 2) return L("Auto", "Tự động");
#endif
    switch (row) {
        case 0: return settings.windowMode == 1 ? L("Desktop (Borderless)", "Màn hình (Không viền)") : (settings.windowMode == 2 ? L("Native (Fullscreen)", "Gốc (Toàn màn hình)") : std::to_string(settings.resW) + " x " + std::to_string(settings.resH));
        case 1: return settings.windowMode == 0 ? L("Windowed", "Cửa sổ") : (settings.windowMode == 1 ? L("Windowed Fullscreen", "Toàn màn hình không viền") : L("Fullscreen", "Toàn màn hình"));
        case 2: return settings.vsync ? L("ON", "BẬT") : L("OFF", "TẮT");
        case 3: return kFpsOptions[settings.fpsIndex] == 0 ? L("Unlimited", "Không giới hạn") : std::to_string(kFpsOptions[settings.fpsIndex]);
        case 4: return settings.showFps ? L("ON", "BẬT") : L("OFF", "TẮT");
        case 5: return settings.vibration ? L("ON", "BẬT") : L("OFF", "TẮT");
        case 6: return settings.lighting ? L("ON", "BẬT") : L("OFF", "TẮT");
        case 7: return settings.particles == 0 ? L("Low", "Thấp") : (settings.particles == 1 ? L("Medium", "Trung bình") : L("High", "Cao"));
        case 8: return settings.vignette ? L("ON", "BẬT") : L("OFF", "TẮT");
        case 9: return std::to_string(settings.volume * 10) + "%";
        case 10: return settings.difficulty == 0 ? L("Easy", "Dễ") : (settings.difficulty == 1 ? L("Normal", "Thường") : L("Hard", "Khó"));
        case 11: return settings.language == 1 ? "Tiếng Việt" : "English";
    }
    return "";
}

inline void Game::UpdateSettingsMenu() {
    const int rows = 13; // 12 mục + Quay lại
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) setSel = (setSel + 1) % rows;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) setSel = (setSel + rows - 1) % rows;
    int delta = 0;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) delta = 1;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) delta = -1;

    Vector2 m = GetMousePosition();
    for (int i = 0; i < rows; ++i) {
        Rectangle r = { 330.0f, 120.0f + i * 34.0f, 620.0f, 30.0f };
        if (CheckCollisionPointRec(m, r)) {
            if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) setSel = i;
            if (setSel == i) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) delta = 1;
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) delta = -1;
            }
        }
    }
    if (EscPressed() || IsKeyPressed(KEY_BACKSPACE)) { state = settingsReturn; return; }
    if (delta != 0) {
        if (setSel == 12) { state = settingsReturn; return; }
        ChangeSetting(setSel, delta);
        SaveSettings();
        ApplySettings();
    }
}

inline void Game::Present() {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
#ifndef __ANDROID__
    if (settings.windowMode == 1) {
        int mon = GetCurrentMonitor();
        sw = GetMonitorWidth(mon);
        sh = GetMonitorHeight(mon);
    }
#endif
    float scale = std::min(sw / 1280.0f, sh / 720.0f);
    float dw = 1280.0f * scale, dh = 720.0f * scale;
    float dx = (sw - dw) * 0.5f, dy = (sh - dh) * 0.5f;
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(screenRT.texture, { 0, 0, (float)screenRT.texture.width, -(float)screenRT.texture.height }, { dx, dy, dw, dh }, { 0, 0 }, 0.0f, WHITE);
    EndDrawing();
    SetMouseOffset(-(int)dx, -(int)dy);
    SetMouseScale(1.0f / scale, 1.0f / scale);
    presentDx = dx;
    presentDy = dy;
    presentScale = scale;
}

inline void Game::ApplyItem(const EquipItem& it, float sign) {
    player.baseDmg += sign * it.damage;
    player.maxHp += sign * it.maxHp;
    if (sign > 0) player.hp += it.maxHp;
    else player.hp = std::max(1.0f, std::min(player.hp - it.maxHp, player.maxHp));
    player.armor += sign * it.armor;
    player.speed += sign * it.moveSpeed * 50.0f;
    player.attackRate += sign * it.attackRate;
    player.lifesteal += sign * it.lifesteal;
    player.critChance += sign * it.crit;
}

inline bool Game::TryEquipItem(const EquipItem& it, int& soldGold) {
    soldGold = 0;
    EquipItem item = it;
    const SubclassDef* sc = ActiveSubclass();
    if (item.slot == SLOT_WEAPON) {
        int cw = ClassWeaponType(player.heroClass);
        if (!CanUseWeapon(item.weaponType)) {
            item.weaponType = cw;
            static const char* weaponNames[8] = { "Sword", "Dagger", "Axe", "Spear", "Hammer", "Scythe", "Bow", "Staff" };
            item.name = std::string(item.RarityName()) + " " + weaponNames[cw];
        }
        if (sc != nullptr) {
            item.name = std::string(item.RarityName()) + " " + sc->weaponTitle;
        }
    }
    int s = item.slot;
    if (hasEquip[s]) {
        if (item.Score() <= equipped[s].Score()) return false;
        ApplyItem(equipped[s], -1.0f);
        soldGold = equipped[s].SellValue();
        player.gold += soldGold;
    }
    equipped[s] = item;
    hasEquip[s] = true;
    ApplyItem(item, 1.0f);
    return true;
}

inline void Game::SpawnItemDrop(Vector2 pos) {
    ItemDrop d;
    d.pos = Vector2Add(pos, { (Rand01() - 0.5f) * 30.0f, (Rand01() - 0.5f) * 30.0f });
    d.item = GenerateItem(floor, ClassWeaponType(player.heroClass), LifestealScale());
    d.life = 90.0f;
    d.phase = Rand01() * 6.0f;
    itemDrops.push_back(d);
}

inline void Game::UpdateItemDrops(float dt) {
    for (size_t i = 0; i < itemDrops.size();) {
        ItemDrop& d = itemDrops[i];
        d.life -= dt;
        bool remove = d.life <= 0;
        if (!remove && Vector2Distance(d.pos, player.pos) < 40.0f) {
            int sold = 0;
            Color rc = d.item.RarityColor();
            if (TryEquipItem(d.item, sold)) {
                ShowBanner("EQUIPPED: " + equipped[d.item.slot].name + (sold > 0 ? "  (old gear sold +" + std::to_string(sold) + "g)" : ""), 3.0f);
                PlaySound(snd.sndLevelUp);
                Burst(player.pos, rc, 24, 260.0f, 0.7f);
            } else {
                int v = d.item.SellValue();
                player.gold += v;
                SpawnPopup(player.pos, "Sold +" + std::to_string(v) + "g", GOLD);
                PlaySound(snd.sndCoin);
                Burst(player.pos, GOLD, 5, 140.0f, 0.4f);
            }
            remove = true;
        }
        if (remove) { itemDrops[i] = itemDrops.back(); itemDrops.pop_back(); continue; }
        ++i;
    }
}
