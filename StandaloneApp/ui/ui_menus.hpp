#pragma once
#include "../game.hpp"

inline void Game::DrawShopScreen() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
    const char* title = L("MERCHANT'S SHOP", "CỬA HÀNG THƯƠNG NHÂN");
    OutlinedText(title, 1280 / 2 - MeasureText(title, 44) / 2, 60, 44, GOLD, { 90, 50, 10, 255 });
    std::string g = std::string(L("Gold: ", "Vàng: ")) + std::to_string(player.gold);
    DrawText(g.c_str(), 1280 / 2 - MeasureText(g.c_str(), 26) / 2, 118, 26, YELLOW);
    for (int i = 0; i < 5; ++i) {
        ShopSlot& s = shopSlots[i];
        Rectangle r = { 116.0f + i * 212.0f, 170.0f, 200.0f, 340.0f };
        Color rc = s.potion ? Color{ 255, 100, 130, 255 } : s.item.RarityColor();
        bool sel = (shopSel == i);
        DrawRectangleRec(r, sel ? Color{ 34, 42, 66, 255 } : Color{ 20, 26, 42, 255 });
        DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : rc);
        DrawRectangle((int)r.x, (int)r.y, 200, 26, Fade(rc, 0.25f));
        DrawText(TextFormat("[%d]", i + 1), (int)r.x + 10, (int)r.y + 5, 16, rc);
        Vector2 ic = { r.x + 100, r.y + 76 };
        if (s.potion) {
            DrawCircleV({ ic.x, ic.y + 8 }, 24.0f, { 220, 60, 90, 255 });
            DrawRectangle((int)ic.x - 8, (int)ic.y - 22, 16, 14, { 190, 200, 220, 255 });
            DrawCircleV({ ic.x - 8, ic.y }, 7.0f, Fade(WHITE, 0.5f));
            DrawText(L("Healing Potion", "Bình Hồi Máu"), (int)r.x + 12, (int)r.y + 124, 17, rc);
            DrawText(TextFormat(L("Heals %d HP", "Hồi %d Máu"), (int)s.heal), (int)r.x + 12, (int)r.y + 156, 16, RAYWHITE);
        } else {
            DrawCircleV(ic, 34.0f, Fade(rc, 0.16f));
            DrawItemIcon(s.item.slot, ic, 54.0f, rc, s.item.weaponType);
            // tên có thể dài: tách làm 2 dòng
            std::string nm = s.item.name;
            size_t sp = nm.find(' ');
            std::string l1 = sp == std::string::npos ? nm : nm.substr(0, sp);
            std::string l2 = sp == std::string::npos ? "" : nm.substr(sp + 1);
            DrawText(l1.c_str(), (int)r.x + 12, (int)r.y + 118, 17, rc);
            DrawText(l2.c_str(), (int)r.x + 12, (int)r.y + 138, 17, rc);
            DrawText(s.item.RarityName(), (int)r.x + 12, (int)r.y + 160, 13, Fade(rc, 0.8f));
            auto lines = s.item.StatLines();
            for (size_t k = 0; k < lines.size(); ++k) DrawText(lines[k].c_str(), (int)r.x + 12, (int)r.y + 182 + (int)k * 19, 15, RAYWHITE);
        }
        if (s.sold) {
            DrawRectangleRec(r, Fade(BLACK, 0.55f));
            const char* soldTxt = L("SOLD", "ĐÃ BÁN");
            DrawText(soldTxt, (int)r.x + 100 - MeasureText(soldTxt, 28) / 2, (int)r.y + 150, 28, { 255, 120, 120, 255 });
        } else {
            bool afford = player.gold >= s.price;
            DrawRectangle((int)r.x + 10, (int)r.y + 296, 180, 32, afford ? Color{ 30, 80, 40, 255 } : Color{ 80, 30, 30, 255 });
            DrawRectangleLines((int)r.x + 10, (int)r.y + 296, 180, 32, afford ? Color{ 120, 235, 140, 255 } : Color{ 235, 120, 120, 255 });
            std::string pr = std::to_string(s.price) + (settings.language == 1 ? " Vàng" : " g");
            DrawText(pr.c_str(), (int)r.x + 100 - MeasureText(pr.c_str(), 20) / 2, (int)r.y + 302, 20, afford ? Color{ 255, 230, 120, 255 } : Color{ 255, 160, 160, 255 });
        }
    }
    const char* foot = L("1-5 / click / Enter: buy      A/D: select      Esc or F: leave", "1-5 / nhấp chuột / Enter: mua      A/D: chọn      Esc hoặc F: rời đi");
    DrawText(foot, 1280 / 2 - MeasureText(foot, 18) / 2, 550, 18, { 150, 165, 200, 255 });
    const char* foot2 = L("Gear you buy is equipped automatically if it beats what you wear.", "Trang bị vừa mua sẽ tự động mặc nếu mạnh hơn món hiện tại.");
    DrawText(foot2, 1280 / 2 - MeasureText(foot2, 16) / 2, 580, 16, { 120, 135, 165, 255 });
}

inline void Game::DrawMetaScreen() {
    float t = (float)GetTime();
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.88f));
    const char* title = L("SANCTUARY", "THÁNH ĐIỆN");
    OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 40, 48, { 190, 150, 255, 255 }, { 40, 20, 80, 255 });
    const char* sub = L("Spend Souls on upgrades that last across every run", "Dùng Linh Hồn để nâng cấp chỉ số vĩnh viễn qua mọi lượt chơi");
    DrawText(sub, 1280 / 2 - MeasureText(sub, 18) / 2, 94, 18, { 160, 150, 200, 255 });

    // Linh hồn
    std::string sv = std::string(L("Souls: ", "Linh Hồn: ")) + std::to_string(souls);
    int sw = MeasureText(sv.c_str(), 28);
    float wx = 1280 / 2.0f - sw / 2.0f - 30.0f, wy = 126.0f + sinf(t * 3.0f) * 2.0f;
    BeginBlendMode(BLEND_ADDITIVE);
    CircleGrad(wx, wy, 26.0f, Fade({ 150, 200, 255, 255 }, 0.5f), Fade({ 150, 200, 255, 255 }, 0.0f));
    EndBlendMode();
    DrawCircleV({ wx, wy }, 8.0f, { 190, 225, 255, 255 });
    Tri({ wx - 5, wy + 4 }, { wx + 5, wy + 4 }, { wx, wy + 18 }, Fade({ 190, 225, 255, 255 }, 0.7f));
    DrawText(sv.c_str(), 1280 / 2 - sw / 2 + 4, 112, 28, { 200, 230, 255, 255 });

    for (int i = 0; i < 8; ++i) {
        Rectangle r = { 190.0f, 150.0f + i * 56.0f + 0.0f, 900.0f, 50.0f };
        r.y += 22.0f;
        bool sel = (metaSel == i);
        bool maxed = meta[i] >= kMeta[i].maxLevel;
        bool afford = !maxed && souls >= MetaCost(i);
        DrawRectangleRec(r, sel ? Color{ 40, 34, 70, 255 } : Color{ 22, 20, 40, 255 });
        DrawRectangleLinesEx(r, sel ? 2.0f : 1.0f, sel ? Color{ 190, 150, 255, 255 } : Color{ 60, 54, 100, 255 });
        DrawText(TextFormat("[%d]", i + 1), (int)r.x + 10, (int)r.y + 16, 16, { 140, 130, 190, 255 });
        DrawText(settings.language == 1 ? kMetaVi[i][0] : kMeta[i].name, (int)r.x + 50, (int)r.y + 6, 20, maxed ? Color{ 255, 215, 90, 255 } : WHITE);
        DrawText(settings.language == 1 ? kMetaVi[i][1] : kMeta[i].desc, (int)r.x + 50, (int)r.y + 29, 14, { 170, 180, 210, 255 });
        // Các ô cấp độ
        int pips = kMeta[i].maxLevel;
        for (int k = 0; k < pips; ++k)
            DrawRectangle((int)r.x + 430 + k * 16, (int)r.y + 18, 12, 14, k < meta[i] ? Color{ 190, 150, 255, 255 } : Color{ 48, 44, 80, 255 });
        // Giá
        Rectangle b = { r.x + r.width - 140.0f, r.y + 8.0f, 126.0f, 34.0f };
        if (maxed) {
            DrawRectangleRec(b, { 70, 56, 20, 255 });
            DrawRectangleLinesEx(b, 1.0f, GOLD);
            const char* maxTxt = L("MAX", "TỐI ĐA");
            DrawText(maxTxt, (int)(b.x + b.width / 2) - MeasureText(maxTxt, 18) / 2, (int)b.y + 8, 18, GOLD);
        } else {
            DrawRectangleRec(b, afford ? Color{ 30, 80, 60, 255 } : Color{ 70, 30, 40, 255 });
            DrawRectangleLinesEx(b, 1.0f, afford ? Color{ 120, 235, 170, 255 } : Color{ 235, 120, 130, 255 });
            std::string c = std::to_string(MetaCost(i));
            DrawText(c.c_str(), (int)(b.x + b.width / 2) - MeasureText(c.c_str(), 20) / 2, (int)b.y + 7, 20, afford ? Color{ 200, 255, 220, 255 } : Color{ 255, 170, 170, 255 });
        }
    }
    if (metaMsgTimer > 0) DrawText(metaMsg.c_str(), 1280 / 2 - MeasureText(metaMsg.c_str(), 22) / 2, 640, 22, GOLD);
    const char* foot = L("1-8 / click / Enter: buy      W/S: select      Esc: back", "1-8 / nhấp chuột / Enter: nâng cấp      W/S: chọn      Esc: quay lại");
    DrawText(foot, 1280 / 2 - MeasureText(foot, 16) / 2, 676, 16, { 150, 160, 200, 255 });
}

inline void Game::DrawSettingsScreen() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.86f));
    const char* title = L("SETTINGS", "CÀI ĐẶT");
    OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 54, 48, GOLD, { 90, 50, 10, 255 });
    static const char* labelsEn[12] = { "Resolution", "Display Mode", "VSync", "FPS Limit", "Show FPS", "Vibration (shake + controller)", "Lighting", "Particles", "Vignette", "Volume", "Difficulty (next floors)", "Language" };
    static const char* labelsVi[12] = { "Độ Phân Giải", "Chế Độ Màn Hình", "Đồng Bộ Dọc (VSync)", "Giới Hạn FPS", "Hiện FPS", "Rung (màn hình + tay cầm)", "Ánh Sáng", "Hiệu Ứng Hạt", "Viền Tối Màn Hình", "Âm Lượng", "Độ Khó (tầng tiếp theo)", "Ngôn Ngữ" };
    for (int i = 0; i < 13; ++i) {
        Rectangle r = { 330.0f, 120.0f + i * 34.0f, 620.0f, 30.0f };
        bool sel = (setSel == i);
        DrawRectangleRec(r, sel ? Color{ 40, 52, 84, 255 } : Color{ 20, 26, 42, 255 });
        if (sel) DrawRectangleLinesEx(r, 2.0f, GOLD);
        if (i < 12) {
            DrawText(settings.language == 1 ? labelsVi[i] : labelsEn[i], (int)r.x + 16, (int)r.y + 5, 19, sel ? WHITE : Color{ 190, 205, 235, 255 });
            std::string v = SettingValue(i);
            bool isOn = (v == "ON" || v == "BẬT");
            bool isOff = (v == "OFF" || v == "TẮT");
            Color vc = isOn ? Color{ 120, 235, 140, 255 } : (isOff ? Color{ 255, 130, 130, 255 } : Color{ 255, 215, 90, 255 });
            if (i == 1) vc = (settings.windowMode == 1) ? Color{ 110, 235, 195, 255 } : (settings.windowMode == 2 ? Color{ 120, 235, 140, 255 } : SKYBLUE);
            int vw = MeasureText(v.c_str(), 19);
            if (i == 10) vc = settings.difficulty == 0 ? Color{ 120, 235, 140, 255 } : (settings.difficulty == 2 ? Color{ 255, 120, 110, 255 } : Color{ 255, 215, 90, 255 });
            if (i == 11) vc = Color{ 130, 220, 255, 255 };
            DrawText(v.c_str(), (int)(r.x + r.width) - 40 - vw, (int)r.y + 5, 19, vc);
            if (sel) {
                DrawText("<", (int)(r.x + r.width) - 40 - vw - 22, (int)r.y + 5, 19, GOLD);
                DrawText(">", (int)(r.x + r.width) - 28, (int)r.y + 5, 19, GOLD);
            }
            if (i == 9) { // thanh âm lượng
                for (int b = 0; b < 10; ++b)
                    DrawRectangle((int)r.x + 220 + b * 12, (int)r.y + 8, 9, 14, b < settings.volume ? Color{ 255, 215, 90, 255 } : Color{ 50, 58, 84, 255 });
            }
        } else {
            const char* back = L("Back", "Quay Lại");
            DrawText(back, (int)(r.x + r.width / 2) - MeasureText(back, 21) / 2, (int)r.y + 5, 21, sel ? WHITE : Color{ 190, 205, 235, 255 });
        }
    }
    const char* f1 = L("W/S or mouse: select     A/D, Enter or click: change     Esc: back", "W/S hoặc chuột: chọn     A/D, Enter hoặc nhấp: đổi     Esc: quay lại");
    const char* f2 = L("Resolution applies in windowed mode; fullscreen uses your desktop resolution. Settings are saved automatically.", "Độ phân giải áp dụng ở chế độ Cửa sổ; Toàn màn hình dùng độ phân giải màn hình chính. Cài đặt tự lưu.");
    DrawText(f1, 1280 / 2 - MeasureText(f1, 16) / 2, 582, 16, { 150, 165, 200, 255 });
    DrawText(f2, 1280 / 2 - MeasureText(f2, 14) / 2, 610, 14, { 120, 135, 165, 255 });
}

inline void Game::DrawInventory() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
    int px = 60, py = 24, pw = 1160, ph = 674;
    DrawRectangle(px, py, pw, ph, { 14, 18, 30, 248 });
    DrawRectangleLines(px, py, pw, ph, GOLD);
    std::string title = std::string(L("EQUIPMENT & SKILLS  -  ", "TRANG BỊ & KỸ NĂNG  -  ")) + HeroTitle() + "  [" + CurrentLevelTitle() + "]";
    DrawText(title.c_str(), 1280 / 2 - MeasureText(title.c_str(), 26) / 2, py + 12, 26, CurrentLevelTitleColor());
    std::string lvSub = TextFormat(L("Level %d / %d%s   |   Bloodline: %s (%s)",
                                     "Cấp %d / %d%s   |   Huyết Mạch: %s (%s)"),
                                   player.level, kMaxPlayerLevel, player.level >= kMaxPlayerLevel ? L(" (MAX)", " (TỐI ĐA)") : "",
                                   BloodlineTraitName(), BloodlineTraitDesc());
    DrawText(lvSub.c_str(), 1280 / 2 - MeasureText(lvSub.c_str(), 13) / 2, py + 41, 13, player.level >= kMaxPlayerLevel ? GOLD : SKYBLUE);

    const char* slotNames[4] = { L("WEAPON", "VŨ KHÍ"), L("ARMOR", "ÁO GIÁP"), L("BOOTS", "GIÀY"), L("RING", "NHẪN") };
    int cw = 266, gap = 14, cx0 = px + (pw - (cw * 4 + gap * 3)) / 2, cy = py + 60, ch = 196;
    for (int i = 0; i < 4; ++i) {
        int cx = cx0 + i * (cw + gap);
        Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 70, 80, 105, 255 };
        DrawRectangle(cx, cy, cw, ch, { 22, 28, 44, 255 });
        DrawRectangleLines(cx, cy, cw, ch, rc);
        DrawRectangle(cx, cy, cw, 24, Fade(rc, 0.25f));
        DrawText(slotNames[i], cx + 12, cy + 4, 16, rc);
        if (hasEquip[i]) {
            DrawCircleV({ (float)cx + cw / 2.0f, (float)cy + 56 }, 28.0f, Fade(rc, 0.18f));
            DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 56 }, 44.0f, rc, equipped[i].weaponType);
            std::string eqTitle = equipped[i].name + (equipPlus[i] > 0 ? (" +" + std::to_string(equipPlus[i])) : "");
            DrawText(eqTitle.c_str(), cx + 12, cy + 92, 16, rc);
            DrawText(equipped[i].RarityName(), cx + 12, cy + 111, 12, Fade(rc, 0.8f));
            auto lines = equipped[i].StatLines();
            for (size_t k = 0; k < lines.size(); ++k) DrawText(lines[k].c_str(), cx + 12, cy + 129 + (int)k * 16, 14, RAYWHITE);
        } else {
            DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 56 }, 44.0f, { 60, 68, 90, 255 }, 0);
            const char* empTxt = L("- Empty -", "- Trống -");
            DrawText(empTxt, cx + cw / 2 - MeasureText(empTxt, 17) / 2, cy + 110, 17, { 110, 120, 145, 255 });
        }
    }

    // Chỉ số nhân vật
    int sy = cy + ch + 10;
    DrawText(L("CHARACTER", "CHỈ SỐ NHÂN VẬT"), px + 32, sy, 18, SKYBLUE);
    char buf[96];
    auto stat = [&](int col, int row, const char* label, const std::string& val) {
        DrawText(label, px + 32 + col * 355, sy + 24 + row * 22, 15, { 170, 185, 215, 255 });
        DrawText(val.c_str(), px + 32 + col * 355 + 155, sy + 24 + row * 22, 15, WHITE);
    };
    snprintf(buf, sizeof(buf), "%d / %d", (int)player.hp, (int)player.maxHp); stat(0, 0, L("Health", "Máu"), buf);
    snprintf(buf, sizeof(buf), "%.0f", player.baseDmg); stat(0, 1, L("Damage", "Sát Thương"), buf);
    snprintf(buf, sizeof(buf), "%.1f", EffectiveArmor()); stat(0, 2, L("Armor", "Giáp"), buf);
    snprintf(buf, sizeof(buf), "%.0f", EffectiveMoveSpeed()); stat(1, 0, L("Move Speed", "Tốc Chạy"), buf);
    snprintf(buf, sizeof(buf), "%.2f /s", EffectiveAttackRate()); stat(1, 1, L("Attack Speed", "Tốc Đánh"), buf);
    snprintf(buf, sizeof(buf), "%d%%", (int)roundf(std::min(0.75f, player.critChance) * 100)); stat(1, 2, L("Crit Chance", "Chí Mạng"), buf);
    stat(2, 0, L("Lifesteal", "Hút Máu"), FmtPct(std::min(LifestealCap() + 0.05f, player.lifesteal)));
    snprintf(buf, sizeof(buf), "%.1fs", player.dashCooldown); stat(2, 1, L("Dash Cooldown", "Hồi Lướt"), buf);
    if (player.subClass == 0) {
        snprintf(buf, sizeof(buf), "%.1fs / %.1fs", Skill1MaxCd(), Skill2MaxCd()); stat(2, 2, L("Q / E Cooldown", "Hồi Chiêu Q/E"), buf);
    } else {
        snprintf(buf, sizeof(buf), "%.0f / %.0f / %.0f / %.0fs", Skill1MaxCd(), Skill2MaxCd(), Skill3MaxCd(), Skill4MaxCd()); stat(2, 2, L("Q/E/C/V CD", "Hồi Q/E/C/V"), buf);
    }
    std::string extra = std::string(L("Essence: ", "Tinh Hoa: ")) + std::to_string(bossEssence) + "   "
                      + L("Reactions: ", "Phản Ứng: ") + std::to_string(elementalReactionsCount) + "   "
                      + L("Ember ", "Lửa ") + std::to_string(player.igniteLevel) + "  "
                      + L("Frost ", "Băng ") + std::to_string(player.frostLevel);
    DrawText(extra.c_str(), px + pw - 32 - MeasureText(extra.c_str(), 13), sy + 2, 13, { 165, 195, 235, 255 });
    std::string duoStr = "";
    if (HasDuoTempestFrost()) duoStr += L(" [Tempest Frost]", " [Bão Băng Sét]");
    if (HasDuoPlaguefire()) duoStr += L(" [Plaguefire]", " [Độc Hỏa]");
    if (HasDuoImmortalBastion()) duoStr += L(" [Immortal Bastion]", " [Pháo Đài Bất Tử]");
    std::string dynInfo = std::string(L("Gen ", "Đời ")) + std::to_string(generation)
                        + " | " + (isNoble ? L("NOBLE", "QUÝ TỘC") : L("Commoner", "Thường Dân"))
                        + " | " + HouseName(houseTier)
                        + " | " + L("Spouse: ", "Vợ: ") + SpouseName(hasSpouse ? spouseIdx : -1)
                        + duoStr;
    DrawText(dynInfo.c_str(), px + 230, sy + 2, 13, isNoble ? GOLD : Color{ 195, 225, 195, 255 });

    // Bộ kỹ năng của Class (4) hoặc Nhánh Phụ (6)
    int ky = sy + 94;
    DrawText(player.subClass == 0 ? L("SKILLS (4)", "BỘ KỸ NĂNG (4)") : L("SUBCLASS SKILLS (6)", "BỘ KỸ NĂNG NHÁNH PHỤ (6)"), px + 32, ky, 17, HeroAccent());
    if (player.subClass == 0) {
        const char* tags[4] = { L("[PASSIVE]", "[NỘI TẠI]"), "[Q]", "[E]", L("[R] RAGE", "[R] NỘ") };
        const char* sNames[4] = { PassiveName(), QName(), EName(), RName() };
        const char* sDescs[4] = { PassiveDesc(), QDesc(), EDesc(), RDesc() };
        for (int s = 0; s < 4; ++s) {
            int rowY = ky + 22 + s * 22;
            DrawText(tags[s], px + 32, rowY, 14, s == 3 ? ORANGE : (s == 0 ? HeroColor() : SKYBLUE));
            DrawText(sNames[s], px + 126, rowY, 14, WHITE);
            DrawText(sDescs[s], px + 318, rowY, 13, { 190, 205, 230, 255 });
        }
    } else {
        const char* tags[6] = { L("[PASSIVE]", "[NỘI TẠI]"), "[Q]", "[E]", "[C]", "[V]", L("[R] RAGE", "[R] NỘ") };
        const char* sNames[6] = { PassiveName(), QName(), EName(), CName(), VName(), RName() };
        const char* sDescs[6] = { PassiveDesc(), QDesc(), EDesc(), CDesc(), VDesc(), RDesc() };
        Color sCols[6] = { HeroColor(), SKYBLUE, { 110, 235, 175, 255 }, { 245, 200, 95, 255 }, { 225, 115, 255, 255 }, ORANGE };
        for (int s = 0; s < 6; ++s) {
            int colX = (s < 3) ? (px + 32) : (px + 588);
            int rowY = ky + 20 + (s % 3) * 32;
            DrawText(tags[s], colX, rowY, 13, sCols[s]);
            DrawText(sNames[s], colX + 82, rowY, 13, WHITE);
            DrawText(sDescs[s], colX + 8, rowY + 14, 11, { 190, 205, 230, 255 });
        }
    }

    // Lộ trình Danh Hiệu theo Mốc Cấp Độ: 20 / 40 / 60 / 80 / 100 (MAX)
    int ty = ky + 118;
    DrawText(L("LEVEL MILESTONE TITLES (LV.100 MAX)", "DANH HIỆU THEO CẤP ĐỘ (CẤP 100 TỐI ĐA)"), px + 32, ty, 15, GOLD);
    int mw = 212, mgap = 8, mx0 = px + 32;
    for (int ti = 1; ti <= 5; ++ti) {
        const LevelTitleDef& td = kLevelTitles[ti];
        bool unlocked = (player.level >= td.minLevel);
        int mx = mx0 + (ti - 1) * (mw + mgap);
        int my = ty + 20;
        DrawRectangle(mx, my, mw, 50, unlocked ? Fade(td.color, 0.16f) : Color{ 18, 22, 34, 220 });
        DrawRectangleLinesEx({ (float)mx, (float)my, (float)mw, 50.0f }, unlocked ? 1.8f : 1.0f, unlocked ? td.color : Color{ 55, 64, 88, 255 });
        std::string lvTag = TextFormat(L("LV.%d%s", "CẤP %d%s"), td.minLevel, td.minLevel == 100 ? " MAX" : "");
        DrawText(lvTag.c_str(), mx + 8, my + 5, 11, unlocked ? td.color : GRAY);
        DrawText(unlocked ? L("ACTIVE", "ĐÃ MỞ") : L("LOCKED", "CHƯA MỞ"), mx + mw - 54, my + 5, 11, unlocked ? Color{ 120, 240, 155, 255 } : DARKGRAY);
        DrawText(settings.language == 1 ? td.nameVi : td.nameEn, mx + 8, my + 19, 12, unlocked ? WHITE : GRAY);
        DrawText(settings.language == 1 ? td.bonusVi : td.bonusEn, mx + 8, my + 34, 10, unlocked ? td.color : Color{ 110, 120, 145, 255 });
    }

    if (!player.relics.empty()) {
        int ry = ty + 78;
        DrawText(L("RELICS", "THÁNH TÍCH"), px + 32, ry, 14, SKYBLUE);
        int rx = px + 128;
        for (const auto& r : player.relics) {
            std::string label = r.name + " (" + RelicDesc(r.type) + ")";
            int w = MeasureText(label.c_str(), 12) + 22;
            if (rx + w > px + pw - 20) { rx = px + 128; ry += 15; }
            DrawCircle(rx + 5, ry + 6, 4.0f, r.color);
            DrawText(label.c_str(), rx + 13, ry + 1, 12, r.color);
            rx += w + 10;
        }
    }
    DrawText(L("Reach Lv.20 / 40 / 60 / 80 / 100 for permanent Titles & Stat Bonuses!   [I] or [Esc] Close",
               "Đạt Cấp 20 / 40 / 60 / 80 / 100 để mở Danh Hiệu & Cộng Chỉ Số Vĩnh Viễn!   [I] hoặc [Esc] Đóng"),
             px + 32, py + ph - 22, 13, { 150, 170, 205, 255 });
}

inline int Game::DrawWrappedText(const char* txt, int x, int y, int maxW, int fontSize, int lineH, Color col) {
    std::istringstream iss(txt);
    std::string word, line;
    int curY = y;
    while (iss >> word) {
        std::string test = line.empty() ? word : (line + " " + word);
        if (MeasureText(test.c_str(), fontSize) > maxW && !line.empty()) {
            DrawText(line.c_str(), x, curY, fontSize, col);
            curY += lineH;
            line = word;
        } else {
            line = test;
        }
    }
    if (!line.empty()) {
        DrawText(line.c_str(), x, curY, fontSize, col);
        curY += lineH;
    }
    return curY;
}

inline void Game::DrawSkillIcon(int heroClass, int slotKind, Vector2 c, float s, Color col) {
    if (slotKind == -1) { // Dash wing/boot
        DrawLineEx({ c.x - s * 0.35f, c.y + s * 0.15f }, { c.x + s * 0.25f, c.y - s * 0.25f }, 3.0f, col);
        DrawLineEx({ c.x - s * 0.25f, c.y + s * 0.30f }, { c.x + s * 0.35f, c.y - s * 0.05f }, 2.5f, Fade(col, 0.75f));
        Tri({ c.x + s * 0.38f, c.y - s * 0.28f }, { c.x + s * 0.08f, c.y - s * 0.28f }, { c.x + s * 0.28f, c.y - s * 0.02f }, col);
        return;
    }
    if (heroClass == CLASS_KNIGHT) {
        if (slotKind == 0) { // Passive: Shield + Cross
            DrawCircleV(c, s * 0.36f, Fade(col, 0.22f));
            DrawCircleLines((int)c.x, (int)c.y, s * 0.36f, col);
            DrawLineEx({ c.x, c.y - s * 0.22f }, { c.x, c.y + s * 0.22f }, 3.0f, GOLD);
            DrawLineEx({ c.x - s * 0.20f, c.y }, { c.x + s * 0.20f, c.y }, 3.0f, GOLD);
        } else if (slotKind == 1) { // Q: Shield Bash
            DrawRectangle((int)(c.x - s * 0.24f), (int)(c.y - s * 0.26f), (int)(s * 0.42f), (int)(s * 0.50f), col);
            DrawLineEx({ c.x + s * 0.18f, c.y }, { c.x + s * 0.42f, c.y }, 3.5f, GOLD);
            Tri({ c.x + s * 0.46f, c.y }, { c.x + s * 0.26f, c.y - s * 0.18f }, { c.x + s * 0.26f, c.y + s * 0.18f }, GOLD);
        } else if (slotKind == 2) { // E: Holy Nova
            DrawCircleLines((int)c.x, (int)c.y, s * 0.34f, col);
            DrawCircleV(c, s * 0.16f, GOLD);
            for (int i = 0; i < 4; ++i) {
                float a = i * (PI / 2.0f);
                DrawLineEx({ c.x + cosf(a) * s * 0.18f, c.y + sinf(a) * s * 0.18f }, { c.x + cosf(a) * s * 0.42f, c.y + sinf(a) * s * 0.42f }, 2.5f, GOLD);
            }
        } else if (slotKind == 4) { // C: Consecrated Hammer / Crimson Eruption
            DrawRectangle((int)(c.x - s * 0.28f), (int)(c.y - s * 0.30f), (int)(s * 0.56f), (int)(s * 0.24f), col);
            DrawLineEx({ c.x, c.y - s * 0.06f }, { c.x, c.y + s * 0.38f }, 4.0f, WHITE);
        } else if (slotKind == 5) { // V: Heavenly Chains / Gore Whirlwind
            DrawCircleLines((int)c.x, (int)c.y, s * 0.34f, GOLD);
            DrawRing(c, s * 0.16f, s * 0.26f, 30.0f, 310.0f, 12, col);
        } else { // R: Heaven's Judgment Sword
            DrawLineEx({ c.x, c.y - s * 0.42f }, { c.x, c.y + s * 0.38f }, 4.5f, GOLD);
            DrawLineEx({ c.x - s * 0.24f, c.y - s * 0.16f }, { c.x + s * 0.24f, c.y - s * 0.16f }, 3.5f, WHITE);
            DrawCircleLines((int)c.x, (int)(c.y + s * 0.20f), s * 0.28f, Fade(GOLD, 0.8f));
        }
    } else if (heroClass == CLASS_ASSASSIN) {
        if (slotKind == 0) { // Passive: Crit Star
            Tri({ c.x, c.y - s * 0.36f }, { c.x - s * 0.22f, c.y + s * 0.24f }, { c.x + s * 0.22f, c.y + s * 0.24f }, col);
            Tri({ c.x, c.y + s * 0.36f }, { c.x - s * 0.22f, c.y - s * 0.18f }, { c.x + s * 0.22f, c.y - s * 0.18f }, Fade(GOLD, 0.85f));
        } else if (slotKind == 1) { // Q: Fan of Knives
            for (int i = -1; i <= 1; ++i) {
                float a = -PI / 2.0f + i * 0.45f;
                DrawLineEx(c, { c.x + cosf(a) * s * 0.42f, c.y + sinf(a) * s * 0.42f }, 3.0f, col);
            }
        } else if (slotKind == 2) { // E: Shadow Step
            DrawCircleLines((int)(c.x - s * 0.14f), (int)c.y, s * 0.20f, Fade(col, 0.5f));
            DrawLineEx({ c.x - s * 0.22f, c.y + s * 0.22f }, { c.x + s * 0.34f, c.y - s * 0.30f }, 3.5f, col);
            DrawCircleV({ c.x + s * 0.26f, c.y - s * 0.22f }, 4.0f, WHITE);
        } else if (slotKind == 4) { // C: Wraith Swarm / Caltrop Storm
            for (int i = 0; i < 3; ++i) {
                float a = -PI / 2.0f + i * (2.0f * PI / 3.0f);
                DrawCircleV({ c.x + cosf(a) * s * 0.22f, c.y + sinf(a) * s * 0.22f }, 4.5f, col);
            }
        } else if (slotKind == 5) { // V: Graveyard Scythe / Thunder Slash
            DrawRing(c, s * 0.20f, s * 0.36f, -45.0f, 135.0f, 12, col);
            DrawLineEx({ c.x - s * 0.28f, c.y + s * 0.30f }, { c.x + s * 0.28f, c.y - s * 0.28f }, 3.0f, WHITE);
        } else { // R: Blade Storm
            DrawRing(c, s * 0.22f, s * 0.36f, 0.0f, 140.0f, 10, col);
            DrawRing(c, s * 0.22f, s * 0.36f, 180.0f, 320.0f, 10, { 255, 90, 175, 255 });
            DrawCircleV(c, s * 0.10f, WHITE);
        }
    } else if (heroClass == CLASS_MAGE) {
        if (slotKind == 0) { // Passive: Arcane Crystal
            Tri({ c.x, c.y - s * 0.38f }, { c.x - s * 0.22f, c.y }, { c.x + s * 0.22f, c.y }, col);
            Tri({ c.x, c.y + s * 0.38f }, { c.x - s * 0.22f, c.y }, { c.x + s * 0.22f, c.y }, Shade(col, 0.7f));
        } else if (slotKind == 1) { // Q: Fireball
            DrawCircleV(c, s * 0.28f, ORANGE);
            DrawCircleV({ c.x + s * 0.04f, c.y - s * 0.04f }, s * 0.15f, YELLOW);
        } else if (slotKind == 2) { // E: Frost Nova
            for (int i = 0; i < 3; ++i) {
                float a = i * (PI / 3.0f);
                DrawLineEx({ c.x - cosf(a) * s * 0.36f, c.y - sinf(a) * s * 0.36f }, { c.x + cosf(a) * s * 0.36f, c.y + sinf(a) * s * 0.36f }, 2.5f, SKYBLUE);
            }
        } else if (slotKind == 4) { // C: Moonlight Cross / Chain Tempest
            DrawLineEx({ c.x - s * 0.28f, c.y - s * 0.28f }, { c.x + s * 0.28f, c.y + s * 0.28f }, 3.2f, col);
            DrawLineEx({ c.x - s * 0.28f, c.y + s * 0.28f }, { c.x + s * 0.28f, c.y - s * 0.28f }, 3.2f, WHITE);
        } else if (slotKind == 5) { // V: Astral Sword Array / Cataclysm Pillar
            DrawRectangle((int)(c.x - s * 0.12f), (int)(c.y - s * 0.36f), (int)(s * 0.24f), (int)(s * 0.72f), Fade(col, 0.75f));
            DrawCircleLines((int)c.x, (int)c.y, s * 0.36f, GOLD);
        } else { // R: Void Meteor
            DrawCircleV(c, s * 0.26f, { 165, 85, 255, 255 });
            DrawCircleLines((int)c.x, (int)c.y, s * 0.38f, SKYBLUE);
            DrawCircleV({ c.x - s * 0.08f, c.y - s * 0.08f }, s * 0.10f, WHITE);
        }
    } else {
        if (slotKind == 0) { // Passive: Eagle Mark
            DrawRing(c, s * 0.26f, s * 0.34f, 0.0f, 360.0f, 16, col);
            DrawCircleV(c, s * 0.12f, GOLD);
        } else if (slotKind == 1) { // Q: Multishot
            for (int i = -1; i <= 1; ++i) {
                float a = -PI / 2.0f + i * 0.38f;
                DrawLineEx({ c.x - cosf(a) * s * 0.22f, c.y - sinf(a) * s * 0.22f }, { c.x + cosf(a) * s * 0.34f, c.y + sinf(a) * s * 0.34f }, 2.5f, GOLD);
            }
        } else if (slotKind == 2) { // E: Explosive Trap
            DrawCircleLines((int)c.x, (int)c.y, s * 0.34f, col);
            DrawCircleV(c, s * 0.16f, ORANGE);
        } else if (slotKind == 4) { // C: Railshot / Valkyrie Spear
            DrawLineEx({ c.x - s * 0.38f, c.y + s * 0.18f }, { c.x + s * 0.34f, c.y - s * 0.18f }, 3.8f, col);
            Tri({ c.x + s * 0.40f, c.y - s * 0.22f }, { c.x + s * 0.16f, c.y - s * 0.24f }, { c.x + s * 0.26f, c.y - s * 0.02f }, WHITE);
        } else if (slotKind == 5) { // V: Flare & Camo / Eye of Hurricane
            DrawCircleLines((int)c.x, (int)c.y, s * 0.34f, col);
            DrawCircleV(c, s * 0.20f, Fade(WHITE, 0.65f));
            DrawCircleV(c, s * 0.10f, GOLD);
        } else { // R: Phoenix Arrow Rain
            for (int i = -1; i <= 1; ++i) {
                DrawLineEx({ c.x + i * s * 0.18f - s * 0.12f, c.y - s * 0.34f }, { c.x + i * s * 0.18f + s * 0.12f, c.y + s * 0.34f }, 2.5f, { 255, 165, 55, 255 });
            }
        }
    }
}

inline void Game::UpdateClassSelect() {
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) classSel = (classSel + 1) % 4;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) classSel = (classSel + 3) % 4;
    for (int i = 0; i < 4; ++i) if (IsKeyPressed(KEY_ONE + i)) classSel = i;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) { state = TITLE; return; }

    Vector2 m = GetMousePosition();
    bool startClick = false;
    const int cardW = 285, gap = 16, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 84, cardH = 532;
    for (int i = 0; i < 4; ++i) {
        Rectangle r = { (float)(x0 + i * (cardW + gap)), (float)cy, (float)cardW, (float)cardH };
        if (CheckCollisionPointRec(m, r)) {
            if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) classSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (classSel == i) startClick = true;
                else { classSel = i; PlaySound(snd.sndCoin); }
            }
        }
    }
    Rectangle startBtn = { 1280 / 2.0f - 180.0f, 634.0f, 360.0f, 46.0f };
    if (CheckCollisionPointRec(m, startBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) startClick = true;

    if (startClick || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        selectedClass = classSel;
        SaveSettings();
        RestartRun();
        PlaySound(snd.sndPotion);
        if (!ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
            OpenNameModal(1, -1, kHeirNames[(selectedClass * 2) % 10]);
        }
    }
}

inline void Game::DrawClassSelectScreen() {
    float t = (float)GetTime();
    DrawRectangleGradientV(0, 0, 1280, 720, Fade({ 8, 10, 18, 255 }, 0.92f), Fade({ 12, 16, 28, 255 }, 0.88f));
    const char* hdr = L("CHOOSE YOUR HERO", "CHỌN NHÂN VẬT");
    OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 42) / 2, 22, 42, GOLD, { 80, 40, 10, 255 });

    const int cardW = 285, gap = 16, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 84, cardH = 532;
    static const char* wpNamesEn[8] = { "Sword", "Dagger", "Axe", "Spear", "Hammer", "Scythe", "Bow", "Staff" };
    static const char* wpNamesVi[8] = { "Kiếm", "Dao Găm", "Rìu", "Thương", "Búa", "Lưỡi Hái", "Cung", "Quyền Trượng" };
    for (int i = 0; i < 4; ++i) {
        const ClassDef& cd = kClasses[i];
        const ClassTextVi& vi = kClassesVi[i];
        const char* cname = settings.language == 1 ? vi.name : cd.name;
        const char* crole = settings.language == 1 ? vi.role : cd.role;
        int cx = x0 + i * (cardW + gap);
        bool sel = (classSel == i);
        Rectangle r = { (float)cx, (float)cy, (float)cardW, (float)cardH };
        DrawRectangleRec(r, sel ? Color{ 26, 34, 56, 252 } : Color{ 16, 20, 34, 240 });
        DrawRectangle(cx, cy, cardW, 44, Fade(cd.color, sel ? 0.35f : 0.16f));
        DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : Fade(cd.color, 0.55f));

        DrawText(TextFormat("[%d]", i + 1), cx + 10, cy + 12, 16, GOLD);
        DrawText(cname, cx + 40, cy + 8, 20, sel ? WHITE : cd.color);
        DrawText(crole, cx + 40, cy + 28, 12, { 185, 198, 225, 255 });

        // Xem trước nhân vật sống động
        Vector2 prevPos = { cx + cardW / 2.0f, (float)cy + 102.0f };
        DrawCircleV(prevPos, 38.0f, Fade(cd.color, sel ? 0.18f : 0.08f));
        float aimA = sel ? sinf(t * 1.8f) * 0.45f : 0.25f;
        DrawHeroModel(prevPos, i, cd.startWeapon, { cosf(aimA), sinf(aimA) }, false, t * (sel ? 6.0f : 0.0f), sel, sel ? sinf(t * 5.0f) * 0.3f : 0.35f, false, false, 0.0f, 0.0f);

        // Chỉ số cơ bản + nhánh Chuyển Chức Lv.5
        int stY = cy + 146;
        DrawRectangle(cx + 10, stY, cardW - 20, 68, { 12, 15, 26, 220 });
        DrawText(TextFormat(L("HP: %d   DMG: %d   ARM: %.1f", "Máu: %d  ST: %d  Giáp: %.1f"), (int)cd.baseHp, (int)cd.baseDmg, cd.baseArmor), cx + 18, stY + 6, 13, RAYWHITE);
        DrawText(TextFormat(L("SPD: %d   ATK: %.1f/s   CRIT: %d%%", "Chạy: %d  Đánh: %.1f/s  CM: %d%%"), (int)cd.baseSpeed, cd.baseAtkRate, (int)roundf(cd.baseCrit * 100)), cx + 18, stY + 21, 13, SKYBLUE);
        DrawText(TextFormat(L("Weapon: %s", "Vũ Khí: %s"), settings.language == 1 ? wpNamesVi[cd.startWeapon] : wpNamesEn[cd.startWeapon]), cx + 18, stY + 36, 13, GOLD);
        std::string brTxt = std::string(L("Lv.5: ", "Cấp 5: ")) + SubDisplayName(i, 1) + " / " + SubDisplayName(i, 2);
        if (MeasureText(brTxt.c_str(), 11) > cardW - 28) brTxt = std::string(L("Lv.5: ", "Cấp 5: ")) + SubDisplayName(i, 1) + " | ...";
        DrawText(brTxt.c_str(), cx + 18, stY + 51, 11, cd.accent);

        // 4 kỹ năng: 1 Passive, 2 thường (Q, E), 1 Nộ (R)
        int sy = stY + 74;
        const char* tags[4] = { L("PASSIVE", "NỘI TẠI"), L("SKILL [Q]", "CHIÊU [Q]"), L("SKILL [E]", "CHIÊU [E]"), L("ULTIMATE [R]", "TUYỆT KỸ [R]") };
        const char* sn[4] = {
            settings.language == 1 ? vi.passiveName : cd.passiveName,
            settings.language == 1 ? vi.qName       : cd.qName,
            settings.language == 1 ? vi.eName       : cd.eName,
            settings.language == 1 ? vi.rName       : cd.rName
        };
        const char* sd[4] = {
            settings.language == 1 ? vi.passiveDesc : cd.passiveDesc,
            settings.language == 1 ? vi.qDesc       : cd.qDesc,
            settings.language == 1 ? vi.eDesc       : cd.eDesc,
            settings.language == 1 ? vi.rDesc       : cd.rDesc
        };
        Color sCol[4] = { cd.color, SKYBLUE, { 110, 235, 175, 255 }, ORANGE };
        for (int k = 0; k < 4; ++k) {
            int boxY = sy + k * 76;
            DrawRectangle(cx + 10, boxY, cardW - 20, 70, { 12, 16, 28, 220 });
            DrawRectangleLines(cx + 10, boxY, cardW - 20, 70, Fade(sCol[k], 0.45f));
            DrawSkillIcon(i, k, { (float)cx + 28, (float)boxY + 20 }, 24.0f, sCol[k]);
            DrawText(tags[k], cx + 48, boxY + 4, 11, sCol[k]);
            DrawText(sn[k], cx + 48, boxY + 17, 14, WHITE);
            DrawWrappedText(sd[k], cx + 16, boxY + 34, cardW - 32, 11, 13, { 188, 200, 224, 255 });
        }
    }

    Rectangle startBtn = { 1280 / 2.0f - 180.0f, 634.0f, 360.0f, 46.0f };
    DrawRectangleRec(startBtn, { 34, 88, 56, 255 });
    DrawRectangleLinesEx(startBtn, 2.0f, GOLD);
    std::string bTxt = std::string(L("PLAY AS ", "CHỌN ")) + ClassDisplayName(classSel) + " [ENTER]";
    DrawText(bTxt.c_str(), 1280 / 2 - MeasureText(bTxt.c_str(), 18) / 2, 648, 18, WHITE);
    const char* foot = L("1-4 / A-D / Click: select hero     Enter / Double-Click: start     Esc: back", "1-4 / A-D / Nhấp: chọn tướng     Enter / Nhấp đúp: bắt đầu     Esc: quay lại");
    DrawText(foot, 1280 / 2 - MeasureText(foot, 15) / 2, 692, 15, { 150, 165, 200, 255 });
}

inline void Game::DrawSaveLoadScreen() {
    float t = (float)GetTime();
    DrawRectangleGradientV(0, 0, 1280, 720, Fade({ 8, 12, 22, 255 }, 0.94f), Fade({ 14, 18, 32, 255 }, 0.92f));
    const char* hdr = (saveLoadReturn == PAUSED) ? L("SAVE & LOAD GAME (4 SLOTS)", "LƯU & TẢI GAME (4 Ô LƯU)") : L("LOAD SAVED GAME (4 SLOTS)", "TẢI GAME ĐÃ LƯU (4 Ô LƯU)");
    OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 38) / 2, 26, 38, GOLD, { 70, 40, 10, 255 });

    const int cardW = 280, gap = 18, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 98, cardH = 505;
    Vector2 m = GetMousePosition();
    for (int i = 0; i < 4; ++i) {
        const SaveSlotInfo& info = saveSlots[i];
        int cx = x0 + i * (cardW + gap);
        bool sel = (saveSlotSel == i);
        Rectangle r = { (float)cx, (float)cy, (float)cardW, (float)cardH };
        int hc = std::max(0, std::min(3, info.heroClass));
        const ClassDef& cd = kClasses[hc];
        const SubclassDef* sc = (info.subClass == 1 || info.subClass == 2) ? &kSubclasses[hc][info.subClass - 1] : nullptr;
        const char* slotClsName = sc ? SubDisplayName(hc, info.subClass) : ClassDisplayName(hc);
        Color accent = info.exists ? (sc ? sc->color : cd.color) : Color{ 95, 110, 140, 255 };

        DrawRectangleRec(r, sel ? Color{ 26, 34, 56, 252 } : Color{ 16, 20, 34, 240 });
        DrawRectangle(cx, cy, cardW, 42, Fade(accent, sel ? 0.35f : 0.18f));
        DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : Fade(accent, 0.6f));
        DrawText(TextFormat(L("[%d] SLOT %d", "[%d] Ô LƯU %d"), i + 1, i + 1), cx + 14, cy + 11, 20, sel ? WHITE : accent);
        const char* tag = info.exists ? L("SAVED", "ĐÃ LƯU") : L("EMPTY", "TRỐNG");
        DrawText(tag, cx + cardW - 14 - MeasureText(tag, 14), cy + 14, 14, info.exists ? Color{ 120, 235, 150, 255 } : GRAY);

        if (info.exists) {
            Vector2 prev = { cx + cardW / 2.0f, (float)cy + 122.0f };
            DrawCircleV(prev, 44.0f, Fade(accent, sel ? 0.18f : 0.09f));
            DrawHeroModel(prev, info.heroClass, info.weaponType, { 0.9f, sinf(t * 2.0f + i) * 0.2f }, false, t * (sel ? 5.0f : 0.0f), sel, 0.35f, false, false, 0.0f, 0.0f, info.subClass, 0.0f);

            int iy = cy + 190;
            std::string cardTitle = (!info.heroName.empty() ? (info.heroName + " - ") : std::string("")) + slotClsName;
            DrawText(cardTitle.c_str(), cx + cardW / 2 - MeasureText(cardTitle.c_str(), 17) / 2, iy, 17, accent);
            DrawRectangle(cx + 16, iy + 32, cardW - 32, 138, { 12, 16, 28, 225 });
            DrawRectangleLines(cx + 16, iy + 32, cardW - 32, 138, Fade(accent, 0.35f));
            DrawText(TextFormat(L("Floor %d  -  Lv %d/100", "Tầng %d  -  Cấp %d/100"), info.floor, info.level), cx + 26, iy + 38, 16, GOLD);
            DrawText(LevelTitleNameForLevel(info.level), cx + 26, iy + 58, 13, LevelTitleColorForLevel(info.level));
            DrawText(TextFormat(L("HP: %d / %d", "Máu: %d / %d"), info.hp, info.maxHp), cx + 26, iy + 78, 15, { 245, 110, 120, 255 });
            DrawText(TextFormat(L("Gold: %d   Slain: %d", "Vàng: %d   Đã hạ: %d"), info.gold, info.enemiesSlain), cx + 26, iy + 99, 15, RAYWHITE);
            std::string wn = info.weaponName;
            if ((int)wn.size() > 18) wn = wn.substr(0, 16) + "..";
            std::string wTxt = std::string(L("Weapon: ", "Vũ khí: ")) + wn;
            DrawText(wTxt.c_str(), cx + 26, iy + 120, 14, SKYBLUE);
            DrawText(info.timestamp.c_str(), cx + 26, iy + 144, 13, { 150, 165, 195, 255 });
        } else {
            const char* empSlot = L("EMPTY SLOT", "Ô LƯU TRỐNG");
            DrawText(empSlot, cx + cardW / 2 - MeasureText(empSlot, 20) / 2, cy + 196, 20, { 110, 122, 150, 255 });
            const char* sub = (saveLoadReturn == PAUSED) ? L("Click SAVE HERE below", "Nhấn LƯU TẠI ĐÂY bên dưới") : L("No saved run yet", "Chưa có dữ liệu lưu");
            DrawText(sub, cx + cardW / 2 - MeasureText(sub, 15) / 2, cy + 230, 15, { 130, 145, 175, 255 });
        }

        if (saveLoadReturn == PAUSED) {
            Rectangle saveBtn = { r.x + 16.0f, r.y + r.height - 96.0f, r.width - 32.0f, 38.0f };
            bool hovS = CheckCollisionPointRec(m, saveBtn);
            Color sCol = info.exists ? Color{ 160, 102, 24, 255 } : Color{ 34, 106, 62, 255 };
            DrawRectangleRec(saveBtn, hovS ? Shade(sCol, 1.3f) : sCol);
            DrawRectangleLinesEx(saveBtn, 2.0f, hovS ? WHITE : (info.exists ? GOLD : Color{ 110, 235, 145, 255 }));
            const char* sLbl = info.exists ? L("OVERWRITE SAVE", "GHI ĐÈ BẢN LƯU") : L("SAVE HERE", "LƯU TẠI ĐÂY");
            DrawText(sLbl, (int)(saveBtn.x + saveBtn.width / 2) - MeasureText(sLbl, 17) / 2, (int)saveBtn.y + 10, 17, WHITE);

            if (info.exists) {
                Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 94.0f, 36.0f };
                Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 50.0f, 54.0f, 36.0f };
                bool hovL = CheckCollisionPointRec(m, loadBtn);
                bool hovD = CheckCollisionPointRec(m, delBtn);
                DrawRectangleRec(loadBtn, hovL ? Color{ 48, 104, 165, 255 } : Color{ 34, 76, 124, 255 });
                DrawRectangleLinesEx(loadBtn, 1.6f, hovL ? WHITE : SKYBLUE);
                const char* lLbl = L("LOAD", "TẢI");
                const char* dLbl = L("DEL", "XÓA");
                DrawText(lLbl, (int)(loadBtn.x + loadBtn.width / 2) - MeasureText(lLbl, 17) / 2, (int)loadBtn.y + 9, 17, WHITE);
                DrawRectangleRec(delBtn, hovD ? Color{ 155, 50, 62, 255 } : Color{ 110, 36, 46, 255 });
                DrawRectangleLinesEx(delBtn, 1.6f, hovD ? WHITE : Color{ 240, 120, 130, 255 });
                DrawText(dLbl, (int)(delBtn.x + delBtn.width / 2) - MeasureText(dLbl, 15) / 2, (int)delBtn.y + 10, 15, WHITE);
            } else {
                Rectangle noBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 32.0f, 36.0f };
                DrawRectangleRec(noBtn, { 28, 34, 50, 220 });
                DrawRectangleLinesEx(noBtn, 1.2f, { 60, 70, 95, 255 });
                const char* noLbl = L("NO SAVE TO LOAD", "CHƯA CÓ BẢN LƯU");
                DrawText(noLbl, (int)(noBtn.x + noBtn.width / 2) - MeasureText(noLbl, 14) / 2, (int)noBtn.y + 11, 14, { 110, 120, 145, 255 });
            }
        } else {
            if (info.exists) {
                Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 94.0f, 40.0f };
                Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 54.0f, 54.0f, 40.0f };
                bool hovL = CheckCollisionPointRec(m, loadBtn);
                bool hovD = CheckCollisionPointRec(m, delBtn);
                DrawRectangleRec(loadBtn, hovL ? Color{ 44, 130, 82, 255 } : Color{ 32, 98, 60, 255 });
                DrawRectangleLinesEx(loadBtn, 2.0f, hovL ? WHITE : Color{ 120, 240, 160, 255 });
                const char* lgLbl = L("LOAD GAME", "TẢI GAME");
                const char* dLbl = L("DEL", "XÓA");
                DrawText(lgLbl, (int)(loadBtn.x + loadBtn.width / 2) - MeasureText(lgLbl, 18) / 2, (int)loadBtn.y + 11, 18, WHITE);
                DrawRectangleRec(delBtn, hovD ? Color{ 155, 50, 62, 255 } : Color{ 110, 36, 46, 255 });
                DrawRectangleLinesEx(delBtn, 1.6f, hovD ? WHITE : Color{ 240, 120, 130, 255 });
                DrawText(dLbl, (int)(delBtn.x + delBtn.width / 2) - MeasureText(dLbl, 15) / 2, (int)delBtn.y + 12, 15, WHITE);
            } else {
                Rectangle noBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 32.0f, 40.0f };
                bool hovN = CheckCollisionPointRec(m, noBtn);
                DrawRectangleRec(noBtn, hovN ? Color{ 44, 124, 78, 255 } : Color{ 28, 72, 52, 240 });
                DrawRectangleLinesEx(noBtn, 1.6f, hovN ? WHITE : Color{ 110, 235, 160, 255 });
                const char* eLbl = L("CREATE NEW SAVE [N]", "TẠO BẢN LƯU MỚI [N]");
                DrawText(eLbl, (int)(noBtn.x + noBtn.width / 2) - MeasureText(eLbl, 15) / 2, (int)noBtn.y + 12, 15, WHITE);
            }
        }
    }

    if (saveStatusTimer > 0.0f && !saveStatusMsg.empty()) {
        DrawText(saveStatusMsg.c_str(), 1280 / 2 - MeasureText(saveStatusMsg.c_str(), 24) / 2, 626, 24, GOLD);
    }
    const char* foot = (saveLoadReturn == PAUSED)
        ? L("1-4 / A-D: slot   Enter: save   N: name save   L: load   Del/X: delete   Esc: back", "1-4 / A-D: chọn ô   Enter: lưu   N: đặt tên save   L: tải   Del/X: xóa   Esc: quay lại")
        : L("1-4 / A-D: slot   Enter / Click: load / create named save   N: new name   Esc: back", "1-4 / A-D: chọn ô   Enter / Nhấp: tải hoặc tạo save có tên   Esc: quay lại");
    DrawText(foot, 1280 / 2 - MeasureText(foot, 16) / 2, 678, 16, { 150, 165, 200, 255 });
}

inline void Game::OutlinedText(const char* txt, int x, int y, int size, Color col, Color outline) {
    gDrawingOutlinePass = true;
    for (int ox = -2; ox <= 2; ox += 2)
        for (int oy = -2; oy <= 2; oy += 2)
            if (ox != 0 || oy != 0) DrawText(txt, x + ox, y + oy, size, outline);
    gDrawingOutlinePass = false;
    DrawText(txt, x, y, size, col);
}

inline void Game::DrawHelpPanel() {
    int w = 860, h = 612, x = 1280 / 2 - w / 2, y = 54;
    DrawRectangle(x, y, w, h, { 14, 18, 30, 245 });
    DrawRectangleLines(x, y, w, h, GOLD);
    const char* hdr = L("CONTROLS", "HƯỚNG DẪN ĐIỀU KHIỂN");
    DrawText(hdr, x + w / 2 - MeasureText(hdr, 30) / 2, y + 16, 30, GOLD);
    static const char* rowsEn[15][2] = {
        { "W A S D / Arrows", "Move" },
        { "Mouse", "Aim weapon & skills" },
        { "Left Click / Space / J", "Basic Attack (Class-Exclusive Weapon)" },
        { "Right Click / Shift / K", "Dash (brief invulnerability)" },
        { "Q / U  |  E / L", "Class Skills 1 & 2" },
        { "C / Z  |  V / X", "Subclass Skills 3 & 4 (unlocked at Lv.5)" },
        { "R / Middle Click", "Ultimate Rage Skill (at 100 Rage)" },
        { "1 / 2 / 3", "Pick an upgrade on level-up" },
        { "I / Y / H", "Equipment & Skills / Family Tree / Eat Ration" },
        { "F / G", "Trade / Surface Landmarks / Ascend at Floor 10+" },
        { "Gamepad", "Sticks, RT/A atk, B dash, X Skill 1, Y Skill 2, LB Ult" },
        { "O / F2 / F11", "Settings / Windowed Fullscreen" },
        { "S", "Sanctuary: permanent upgrades (title / game over)" },
        { "F5 / L", "Save & Load Game (4 slots with overwrite)" },
        { "M / Esc / F1", "Radar / Pause & Back / Controls" },
    };
    static const char* rowsVi[15][2] = {
        { "W A S D / Phím Mũi Tên", "Di chuyển nhân vật" },
        { "Chuột", "Ngắm hướng đánh & kỹ năng" },
        { "Chuột Trái / Space / J", "Tấn công cơ bản (Vũ khí đặc trưng hệ)" },
        { "Chuột Phải / Shift / K", "Lướt né đòn (miễn sát thương ngắn)" },
        { "Q / U  |  E / L", "Kỹ năng 1 & Kỹ năng 2" },
        { "C / Z  |  V / X", "Kỹ năng Nhánh Phụ 3 & 4 (mở ở Cấp 5)" },
        { "R / Chuột Giữa", "Tuyệt Kỹ Nộ Khí (khi đủ 100 Nộ)" },
        { "1 / 2 / 3", "Chọn nâng cấp hoặc nhánh thức tỉnh" },
        { "I / Y / H", "Trang bị & Kỹ năng / Cây Gia Phả / Ăn Lương Khô" },
        { "F / G", "Thương Nhân / Công Trình / Lên Mặt Đất (Tầng 10+)" },
        { "Tay Cầm", "Cần gạt, RT/A đánh, B lướt, X/Y chiêu, LB Tuyệt Kỹ" },
        { "O / F2 / F11", "Cài Đặt / Toàn Màn Hình Không Viền" },
        { "S", "Thánh Điện: nâng cấp vĩnh viễn" },
        { "F5 / L", "Lưu & Tải Game (tối đa 4 ô ghi đè)" },
        { "M / Esc / F1", "Bản đồ nhỏ / Tạm dừng & Quay lại / Hướng dẫn" },
    };
    for (int i = 0; i < 15; ++i) {
        DrawText(settings.language == 1 ? rowsVi[i][0] : rowsEn[i][0], x + 36, y + 70 + i * 32, 18, SKYBLUE);
        DrawText(settings.language == 1 ? rowsVi[i][1] : rowsEn[i][1], x + 340, y + 70 + i * 32, 18, RAYWHITE);
    }
    DrawText(L("Clear a room, enter the portal. Hit enemies to build 100 Rage for [R] Ultimate!", "Dọn sạch quái trong phòng rồi bước vào cổng. Đánh địch tích 100 Nộ để tung Tuyệt Kỹ [R]!"), x + 36, y + h - 42, 16, { 190, 200, 220, 255 });
}

inline void Game::DrawTitleScreen() {
    float t = (float)GetTime();
    DrawRectangleGradientV(0, 0, 1280, 720, Fade(BLACK, 0.86f), Fade(BLACK, 0.70f));
    if (showHelp) { DrawHelpPanel(); return; }
    const char* title = "GEOMETRIC DUNGEON";
    int tw = MeasureText(title, 72);
    int ty = 100 + (int)(sinf(t * 1.6f) * 5.0f);
    DrawText(title, 1280 / 2 - tw / 2 + 4, ty + 5, 72, Fade(BLACK, 0.8f));
    OutlinedText(title, 1280 / 2 - tw / 2, ty, 72, GOLD, { 120, 60, 10, 255 });
    const char* sub = L("4 Classes  -  8 Weapons  -  Rage Ultimates", "4 Hệ Tướng  -  8 Vũ Khí  -  Tuyệt Kỹ Nộ Khí");
    DrawText(sub, 1280 / 2 - MeasureText(sub, 22) / 2, ty + 88, 22, { 170, 190, 230, 255 });

    std::string rec = std::string(L("Best Record: Floor ", "Kỷ Lục Cao Nhất: Tầng ")) + std::to_string(bestFloor);
    DrawText(rec.c_str(), 1280 / 2 - MeasureText(rec.c_str(), 22) / 2, 292, 22, SKYBLUE);

    int selCls = std::max(0, std::min(3, selectedClass));
    const ClassDef& cd = kClasses[selCls];
    std::string heroTxt = std::string(L("Hero: ", "Tướng Đang Chọn: ")) + ClassDisplayName(selCls);
    DrawText(heroTxt.c_str(), 1280 / 2 - MeasureText(heroTxt.c_str(), 20) / 2, 324, 20, cd.color);

    const char* diffName = settings.difficulty == 0 ? L("Easy", "Dễ") : (settings.difficulty == 1 ? L("Normal", "Thường") : L("Hard", "Khó"));
    std::string prog = std::string(diffName) + L("     Souls: ", "     Linh Hồn: ") + std::to_string(souls) + L("     Runs: ", "     Lượt Chơi: ") + std::to_string(runs);
    DrawText(prog.c_str(), 1280 / 2 - MeasureText(prog.c_str(), 20) / 2, 354, 20, { 190, 150, 255, 255 });
#ifndef __ANDROID__
    const char* hint = L("Enter: New Game    L: Load Save    S: Sanctuary    O: Settings    F1: Controls", "Enter: Chơi Mới    L: Tải Game    S: Thánh Điện    O: Cài Đặt    F1: Điều Khiển");
    DrawText(hint, 1280 / 2 - MeasureText(hint, 18) / 2, 628, 18, { 130, 145, 180, 255 });
#endif
}

inline void Game::DrawPauseScreen() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.72f));
    if (showHelp) { DrawHelpPanel(); return; }
    const char* t1 = L("PAUSED", "TẠM DỪNG");
    OutlinedText(t1, 1280 / 2 - MeasureText(t1, 56) / 2, 108, 56, WHITE, { 20, 24, 40, 255 });
    std::string info = std::string(HeroTitle()) + L("   Floor ", "   Tầng ") + std::to_string(floor) + L("   Level ", "   Cấp ") + std::to_string(player.level) + L("   Slain ", "   Đã Hạ ") + std::to_string(enemiesSlain);
    DrawText(info.c_str(), 1280 / 2 - MeasureText(info.c_str(), 20) / 2, 170, 20, HeroColor());
}
