#pragma once
#include "../game.hpp"

inline int Game::SurfaceOptionCount() const {
    if (surfaceBuilding == 0) return 5;
    if (surfaceBuilding == 1) return 4;
    if (surfaceBuilding == 2) return hasSpouse ? (1 + (int)children.size()) : 4;
    return 2;
}

inline void Game::TriggerSurfaceOption(int tab, int opt) {
    if (tab == 0) {
        if (opt < 4) BuyMeal(opt);
        else AcceptOrClaimBounty();
    } else if (tab == 1) {
        if (opt < 3) BuyHouseUpgrade(opt + 1);
        else ReforgeEquipSlot(0);
    } else if (tab == 2) {
        if (!hasSpouse) TryMarrySpouse(opt);
        else if (opt == 0) TryHaveChild();
        else SuccessionToChild(opt - 1);
    } else if (tab == 3) {
        if (opt == 0) BuyNobleTitleDirect();
        else AcceptRoyalCommission();
    }
}

inline void Game::UpdateSurfaceUi() {
    { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
    if (EscPressed() || IsKeyPressed(KEY_F) || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
        state = PLAYING;
        return;
    }
    if (IsKeyPressed(KEY_Y)) { OpenFamilyTree(SURFACE_UI); return; }
    for (int i = 0; i < 4; ++i) {
        if (IsKeyPressed(KEY_ONE + i)) { surfaceBuilding = i; surfaceSel = 0; PlaySound(snd.sndCoin); }
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { surfaceBuilding = (surfaceBuilding + 1) % 4; surfaceSel = 0; PlaySound(snd.sndCoin); }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) { surfaceBuilding = (surfaceBuilding + 3) % 4; surfaceSel = 0; PlaySound(snd.sndCoin); }

    int cnt = std::max(1, SurfaceOptionCount());
    if (surfaceSel >= cnt) surfaceSel = cnt - 1;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) surfaceSel = (surfaceSel + 1) % cnt;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) surfaceSel = (surfaceSel + cnt - 1) % cnt;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        TriggerSurfaceOption(surfaceBuilding, surfaceSel);
        return;
    }

    Vector2 m = GetMousePosition();
    for (int i = 0; i < 4; ++i) {
        Rectangle tr = { 100.0f + i * 274.0f, 92.0f, 258.0f, 40.0f };
        if (CheckCollisionPointRec(m, tr) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            surfaceBuilding = i;
            surfaceSel = 0;
            PlaySound(snd.sndCoin);
            return;
        }
    }
    if (surfaceBuilding == 0) {
        for (int i = 0; i < 5; ++i) {
            Rectangle r = { 120.0f, 184.0f + i * 84.0f, 1040.0f, 74.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = i; TriggerSurfaceOption(0, i); return; }
            }
        }
    } else if (surfaceBuilding == 1) {
        for (int i = 0; i < 4; ++i) {
            Rectangle r = { 120.0f, 184.0f + i * 102.0f, 1040.0f, 92.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = i; TriggerSurfaceOption(1, i); return; }
            }
        }
    } else if (surfaceBuilding == 2) {
        if (!hasSpouse) {
            for (int i = 0; i < 4; ++i) {
                Rectangle r = { 120.0f, 214.0f + i * 96.0f, 1040.0f, 84.0f };
                if (CheckCollisionPointRec(m, r)) {
                    if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = i; TriggerSurfaceOption(2, i); return; }
                }
            }
        } else {
            Rectangle rChild = { 120.0f, 252.0f, 510.0f, 58.0f };
            Rectangle rTree  = { 650.0f, 252.0f, 510.0f, 58.0f };
            if (CheckCollisionPointRec(m, rChild)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = 0;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = 0; TriggerSurfaceOption(2, 0); return; }
            }
            if (CheckCollisionPointRec(m, rTree) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                OpenFamilyTree(SURFACE_UI);
                return;
            }
            if (IsKeyPressed(KEY_T) && surfaceSel >= 1 && surfaceSel - 1 < (int)children.size()) {
                ToggleChildParty(surfaceSel - 1);
                return;
            }
            for (size_t i = 0; i < children.size(); ++i) {
                Rectangle trB = { 120.0f + 1040.0f - 546.0f, 326.0f + i * 88.0f + 16.0f, 118.0f, 44.0f };
                if (CheckCollisionPointRec(m, trB) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ToggleChildParty((int)i);
                    return;
                }
                Rectangle renB = { 120.0f + 1040.0f - 418.0f, 326.0f + i * 88.0f + 16.0f, 118.0f, 44.0f };
                if (CheckCollisionPointRec(m, renB) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (!ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                        OpenNameModal(2, (int)i, children[i].name);
                    }
                    return;
                }
                Rectangle r = { 120.0f, 326.0f + i * 88.0f, 1040.0f, 76.0f };
                if (CheckCollisionPointRec(m, r)) {
                    if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = (int)i + 1;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = (int)i + 1; TriggerSurfaceOption(2, (int)i + 1); return; }
                }
            }
        }
    } else {
        for (int i = 0; i < 2; ++i) {
            Rectangle r = { 120.0f, 196.0f + i * 190.0f, 1040.0f, 168.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = i; TriggerSurfaceOption(3, i); return; }
            }
        }
    }
}

inline void Game::DrawSurfaceUi() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
    Rectangle panel = { 80.0f, 78.0f, 1120.0f, 594.0f };
    DrawRectangleRec(panel, { 16, 22, 36, 250 });
    DrawRectangleLinesEx(panel, 2.5f, GOLD);

    const char* tabNames[4] = {
        L("[1] TAVERN & FOOD", "[1] QUÁN TRỌ & ĐỒ ĂN"),
        L("[2] HOUSING GUILD", "[2] PHƯỜNG GIA TRANG"),
        L("[3] FAMILY & HEIRS", "[3] TỔ ẤM & ĐỜI CON"),
        L("[4] ROYAL PALACE", "[4] HOÀNG CUNG & VUA")
    };
    Color tabCols[4] = { ORANGE, SKYBLUE, { 255, 130, 175, 255 }, GOLD };
    for (int i = 0; i < 4; ++i) {
        Rectangle tr = { 100.0f + i * 274.0f, 92.0f, 258.0f, 40.0f };
        bool active = (surfaceBuilding == i);
        DrawRectangleRec(tr, active ? Fade(tabCols[i], 0.28f) : Color{ 22, 28, 46, 240 });
        DrawRectangleLinesEx(tr, active ? 2.5f : 1.0f, active ? tabCols[i] : Color{ 70, 82, 115, 255 });
        DrawText(tabNames[i], (int)(tr.x + tr.width / 2) - MeasureText(tabNames[i], 15) / 2, (int)tr.y + 12, 15, active ? WHITE : Color{ 170, 185, 215, 255 });
    }

    // Thanh trạng thái nhanh trên đầu bảng
    DrawRectangle(100, 140, 1080, 32, { 12, 16, 28, 240 });
    std::string info = std::string(L("Gold: ", "Vàng: ")) + std::to_string(player.gold)
                     + "  |  " + L("Essence: ", "Tinh Hoa: ") + std::to_string(bossEssence)
                     + "  |  " + L("Hunger: ", "Độ No: ") + std::to_string((int)ceilf(hunger)) + "/100"
                     + "  |  " + BloodlineTraitName()
                     + "  |  " + TextFormat(L("Day %d %s  Age %d", "Ngày %d %s  Tuổi %d"), dayCount, ClockString().c_str(), heroAge);
    DrawText(info.c_str(), 116, 148, 14, GOLD);

    if (surfaceBuilding == 0) {
        struct MealRow { const char* nameEn; const char* nameVi; const char* descEn; const char* descVi; int baseCost; Color col; };
        static const MealRow meals[4] = {
            { "Warm Village Bread", "Bánh Mì Nóng Hổi", "+35 Hunger & +18 HP immediately", "+35 Độ No & hồi +18 Máu ngay lập tức", 15, { 245, 205, 115, 255 } },
            { "Savory Roast Platter", "Thịt Nướng Đậm Đà", "+70 Hunger & +40% Max HP", "+70 Độ No & hồi +40% Máu Tối Đa", 35, ORANGE },
            { "Royal Palace Banquet", "Yến Tiệc Thịnh Soạn", "Full 100 Hunger, Full 100% HP & +25 Rage", "Đầy 100 Độ No, Đầy 100% Máu & +25 Nộ", 70, GOLD },
            { "Dungeon Ration Pack (x2)", "Túi Lương Khô Hầm Ngục (x2)", "Stores +2 portable Rations. Press [H] anywhere to eat (+45 Hunger & +25% HP)", "Tích +2 Lương Khô mang đi. Nhấn [H] mọi lúc (+45 No & +25% Máu)", 25, { 125, 235, 155, 255 } }
        };
        for (int i = 0; i < 4; ++i) {
            Rectangle r = { 120.0f, 184.0f + i * 84.0f, 1040.0f, 74.0f };
            bool sel = (surfaceSel == i);
            int price = MealPrice(meals[i].baseCost);
            bool afford = player.gold >= price;
            DrawRectangleRec(r, sel ? Color{ 34, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
            DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(meals[i].col, 0.55f));
            DrawText(L(meals[i].nameEn, meals[i].nameVi), (int)r.x + 22, (int)r.y + 10, 20, meals[i].col);
            DrawText(L(meals[i].descEn, meals[i].descVi), (int)r.x + 22, (int)r.y + 40, 14, RAYWHITE);
            Rectangle b = { r.x + r.width - 190.0f, r.y + 15.0f, 168.0f, 44.0f };
            DrawRectangleRec(b, afford ? Color{ 34, 96, 56, 255 } : Color{ 82, 32, 38, 255 });
            DrawRectangleLinesEx(b, 1.5f, afford ? Color{ 120, 240, 150, 255 } : Color{ 240, 120, 130, 255 });
            std::string pTxt = std::to_string(price) + L(" G", " Vàng");
            DrawText(pTxt.c_str(), (int)(b.x + b.width / 2) - MeasureText(pTxt.c_str(), 18) / 2, (int)b.y + 12, 18, afford ? WHITE : Color{ 255, 175, 175, 255 });
        }
        // Bảng Truy Nã Quán Trọ (Option 4)
        {
            Rectangle r = { 120.0f, 184.0f + 4 * 84.0f, 1040.0f, 74.0f };
            bool sel = (surfaceSel == 4);
            bool ready = bountyActive && bountyProgress >= bountyTarget;
            Color bCol = ready ? Color{ 120, 245, 150, 255 } : SKYBLUE;
            DrawRectangleRec(r, sel ? Color{ 34, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
            DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(bCol, 0.6f));
            const char* bTitle = !bountyActive ? L("TAVERN BOUNTY BOARD: Accept Hunt Contract", "BẢNG TRUY NÃ QUÁN TRỌ: Nhận Lệnh Săn Thưởng")
                                               : (ready ? L("BOUNTY COMPLETE! Claim Gold & Boss Essence", "HOÀN THÀNH TRUY NÃ! Nhận Vàng & Tinh Hoa Trùm")
                                                        : TextFormat(L("ACTIVE BOUNTY (%d / %d) - Hunt in Dungeon", "ĐANG TRUY NÃ (%d / %d) - Săn Dưới Hầm Ngục"), bountyProgress, bountyTarget));
            DrawText(bTitle, (int)r.x + 22, (int)r.y + 10, 20, bCol);
            DrawText(L("Earn Gold & Boss Essence to reinforce your weapons at Housing Guild!", "Kiếm Vàng & Tinh Hoa Trùm để Cường Hóa vũ khí tại Phường Gia Trang!"),
                     (int)r.x + 22, (int)r.y + 40, 14, RAYWHITE);
            Rectangle b = { r.x + r.width - 190.0f, r.y + 15.0f, 168.0f, 44.0f };
            DrawRectangleRec(b, ready ? Color{ 34, 110, 62, 255 } : Color{ 38, 58, 96, 255 });
            DrawRectangleLinesEx(b, 1.5f, bCol);
            const char* btnL = !bountyActive ? L("ACCEPT", "NHẬN LỆNH") : (ready ? L("CLAIM!", "NHẬN THƯỞNG") : L("IN PROGRESS", "ĐANG LÀM"));
            DrawText(btnL, (int)(b.x + b.width / 2) - MeasureText(btnL, 16) / 2, (int)b.y + 14, 16, WHITE);
        }
    } else if (surfaceBuilding == 1) {
        for (int t = 1; t <= 3; ++t) {
            Rectangle r = { 120.0f, 184.0f + (t - 1) * 102.0f, 1040.0f, 92.0f };
            bool sel = (surfaceSel == t - 1);
            bool owned = (houseTier >= t);
            bool nextTier = (houseTier + 1 == t);
            int price = HouseUpgradeCost(t);
            bool afford = nextTier && player.gold >= price;
            Color hc = owned ? Color{ 115, 240, 155, 255 } : (nextTier ? GOLD : GRAY);
            DrawRectangleRec(r, sel ? Color{ 32, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
            DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(hc, 0.55f));
            DrawText(HouseName(t), (int)r.x + 24, (int)r.y + 10, 22, hc);
            DrawText(L(kHouses[t].descEn, kHouses[t].descVi), (int)r.x + 24, (int)r.y + 40, 14, RAYWHITE);
            DrawText(TextFormat(L("Total Bonus: +%d Max HP   +%d Damage   Up to %d Children", "Tổng Cộng: +%d Máu Tối Đa   +%d Sát Thương   Tối đa %d Con"),
                (int)kHouses[t].hpBonus, (int)kHouses[t].dmgBonus, kHouses[t].maxKids), (int)r.x + 24, (int)r.y + 64, 13, SKYBLUE);

            Rectangle b = { r.x + r.width - 230.0f, r.y + 22.0f, 206.0f, 46.0f };
            const char* bTxt = owned ? L("OWNED", "ĐÃ SỞ HỮU") : (nextTier ? TextFormat(L("BUY %d G", "MUA %d VÀNG"), price) : L("REQUIRES PREV TIER", "CẦN CẤP TRƯỚC"));
            Color bFill = owned ? Color{ 28, 72, 48, 255 } : (afford ? Color{ 38, 102, 62, 255 } : Color{ 64, 34, 42, 255 });
            DrawRectangleRec(b, bFill);
            DrawRectangleLinesEx(b, 1.5f, hc);
            DrawText(bTxt, (int)(b.x + b.width / 2) - MeasureText(bTxt, 16) / 2, (int)b.y + 14, 16, WHITE);
        }
        // Lò Rèn Cường Hóa Vũ Khí +1..+5 (Option 3)
        {
            Rectangle r = { 120.0f, 184.0f + 3 * 102.0f, 1040.0f, 92.0f };
            bool sel = (surfaceSel == 3);
            int plus = equipPlus[0];
            int goldCost = MealPrice(90 + plus * 70);
            int essCost = (plus >= 2) ? 1 : 0;
            bool canForge = hasEquip[0] && plus < 5 && player.gold >= goldCost && bossEssence >= essCost;
            Color fc = (plus >= 5) ? Color{ 120, 245, 150, 255 } : Color{ 255, 185, 95, 255 };
            DrawRectangleRec(r, sel ? Color{ 32, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
            DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(fc, 0.65f));
            DrawText(TextFormat(L("BLACKSMITH FORGE: Reinforce %s (+%d -> +%d)", "LÒ RÈN THẦN BÍ: Cường Hóa %s (+%d -> +%d)"),
                                hasEquip[0] ? equipped[0].name.c_str() : L("Weapon", "Vũ Khí"), plus, std::min(5, plus + 1)),
                     (int)r.x + 24, (int)r.y + 12, 20, fc);
            DrawText(L("+22% Weapon Stats per level! Requires Gold (+1 Boss Essence for +3 and above).",
                       "+22% Chỉ số Vũ Khí mỗi cấp! Cần Vàng (từ +3 trở lên cần thêm 1 Tinh Hoa Trùm)."),
                     (int)r.x + 24, (int)r.y + 48, 14, RAYWHITE);
            Rectangle b = { r.x + r.width - 230.0f, r.y + 22.0f, 206.0f, 46.0f };
            const char* bTxt = (plus >= 5) ? L("MAX +5", "TỐI ĐA +5")
                                           : TextFormat(essCost > 0 ? L("FORGE %dG + 1E", "RÈN %dV + 1TH") : L("FORGE %d G", "RÈN %d VÀNG"), goldCost);
            DrawRectangleRec(b, canForge ? Color{ 98, 68, 28, 255 } : Color{ 56, 36, 42, 255 });
            DrawRectangleLinesEx(b, 1.5f, fc);
            DrawText(bTxt, (int)(b.x + b.width / 2) - MeasureText(bTxt, 16) / 2, (int)b.y + 14, 16, WHITE);
        }
    } else if (surfaceBuilding == 2) {
        if (!hasSpouse) {
            const char* msg = (houseTier <= 0)
                ? L("Buy a House in Tab [2] first! Spouses CANNOT be bought — unlock via Dungeon Rescues & Quests!",
                    "Hãy mua Nhà trước! Vợ KHÔNG THỂ mua bằng Vàng — chỉ mở khóa qua Giải Cứu Hầm Ngục & Nhiệm Vụ!")
                : L("Spouses cannot be bought with Gold — rescue them in Dungeon events or earn Quest betrothals!",
                    "Vợ KHÔNG bán bằng Vàng — hãy giải cứu trong Hầm Ngục hoặc làm Nhiệm Vụ để được gả!");
            DrawText(msg, 120, 184, 15, houseTier <= 0 ? Color{ 255, 130, 130, 255 } : Color{ 255, 200, 225, 255 });
            for (int i = 0; i < 4; ++i) {
                const SpouseDef& sp = kSpouses[i];
                Rectangle r = { 120.0f, 214.0f + i * 96.0f, 1040.0f, 84.0f };
                bool sel = (surfaceSel == i);
                bool unlocked = spouseUnlocked[i];
                bool can = (houseTier >= 1 && unlocked);
                DrawRectangleRec(r, sel ? Color{ 38, 32, 62, 252 } : Color{ 22, 24, 42, 240 });
                DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(unlocked ? sp.dressCol : GRAY, 0.7f));
                DrawCircle((int)r.x + 42, (int)r.y + 42, 20.0f, Fade(sp.dressCol, unlocked ? 0.28f : 0.10f));
                DrawCircle((int)r.x + 42, (int)r.y + 42, 11.0f, unlocked ? sp.dressCol : GRAY);
                DrawText(SpouseName(i), (int)r.x + 78, (int)r.y + 10, 20, unlocked ? sp.dressCol : LIGHTGRAY);
                DrawText(L(sp.roleEn, sp.roleVi), (int)r.x + 78, (int)r.y + 36, 14, WHITE);
                DrawText(SpouseUnlockConditionText(i), (int)r.x + 78, (int)r.y + 58, 13, unlocked ? Color{ 120, 245, 165, 255 } : ORANGE);
                DrawText(TextFormat(L("+%d HP  +%d DMG  +%d%% CRIT  +%.2f ATK/s", "+%d Máu  +%d ST  +%d%% Chí Mạng  +%.2f Đánh/s"),
                    (int)sp.hpBonus, (int)sp.dmgBonus, (int)roundf(sp.critBonus * 100), sp.atkSpdBonus), (int)r.x + 470, (int)r.y + 30, 15, SKYBLUE);

                Rectangle b = { r.x + r.width - 230.0f, r.y + 20.0f, 208.0f, 44.0f };
                DrawRectangleRec(b, can ? Color{ 115, 42, 82, 255 } : Color{ 44, 40, 56, 255 });
                DrawRectangleLinesEx(b, 1.5f, can ? Color{ 255, 165, 210, 255 } : GRAY);
                const char* pTxt = !unlocked ? L("EVENT / QUEST LOCKED", "CHƯA MỞ (SỰ KIỆN/NV)")
                                             : (houseTier <= 0 ? L("NEED HOUSE FIRST", "CẦN MUA NHÀ") : L("MARRY FREE (UNLOCKED)", "KẾT HÔN (ĐÃ MỞ)"));
                DrawText(pTxt, (int)(b.x + b.width / 2) - MeasureText(pTxt, 14) / 2, (int)b.y + 14, 14, WHITE);
            }
        } else {
            const SpouseDef& sp = kSpouses[std::max(0, std::min(3, spouseIdx))];
            int maxKids = kHouses[std::max(0, std::min(3, houseTier))].maxKids;
            DrawRectangle(120, 184, 1040, 56, { 34, 26, 52, 240 });
            DrawRectangleLines(120, 184, 1040, 56, sp.dressCol);
            std::string fam = std::string(L("Spouse: ", "Bạn Đời: ")) + SpouseName(spouseIdx)
                            + "  (" + L(sp.roleEn, sp.roleVi) + ")   |   "
                            + L("Heirs: ", "Đời Con: ") + std::to_string((int)children.size()) + " / " + std::to_string(maxKids);
            DrawText(fam.c_str(), 142, 202, 18, WHITE);

            Rectangle rChild = { 120.0f, 252.0f, 510.0f, 58.0f };
            Rectangle rTree  = { 650.0f, 252.0f, 510.0f, 58.0f };
            int childCost = MealPrice(100);
            bool canKid = ((int)children.size() < maxKids && player.gold >= childCost);
            DrawRectangleRec(rChild, surfaceSel == 0 ? Color{ 38, 72, 62, 255 } : Color{ 26, 48, 44, 245 });
            DrawRectangleLinesEx(rChild, surfaceSel == 0 ? 2.5f : 1.5f, surfaceSel == 0 ? GOLD : (canKid ? Color{ 120, 240, 170, 255 } : GRAY));
            std::string kcTxt = ((int)children.size() >= maxKids)
                ? L("MAX CHILDREN (Upgrade House)", "ĐỦ SỐ CON (Nâng cấp Nhà)")
                : (std::string(L("HAVE A CHILD (", "SINH CON NỐI DÕI (")) + std::to_string(childCost) + L(" Gold)", " Vàng)"));
            DrawText(kcTxt.c_str(), (int)(rChild.x + rChild.width / 2) - MeasureText(kcTxt.c_str(), 17) / 2, (int)rChild.y + 20, 17, canKid ? WHITE : GRAY);

            DrawRectangleRec(rTree, { 42, 36, 78, 245 });
            DrawRectangleLinesEx(rTree, 1.5f, { 190, 150, 255, 255 });
            const char* ftLbl = L("VIEW FAMILY TREE [Y]", "XEM CÂY GIA PHẢ [Y]");
            DrawText(ftLbl, (int)(rTree.x + rTree.width / 2) - MeasureText(ftLbl, 18) / 2, (int)rTree.y + 19, 18, GOLD);

            if (children.empty()) {
                const char* noKid = L("No children yet. Raise an Heir so your Bloodline lives on via Rest in Peace!",
                                      "Chưa có con nối dõi. Hãy Sinh Con để có thể An Nghỉ (Rest in Peace) & truyền đời!");
                DrawText(noKid, 1280 / 2 - MeasureText(noKid, 17) / 2, 415, 17, { 185, 198, 225, 255 });
            } else {
                for (size_t i = 0; i < children.size(); ++i) {
                    const ChildInfo& ch = children[i];
                    Rectangle r = { 120.0f, 326.0f + i * 88.0f, 1040.0f, 76.0f };
                    bool sel = (surfaceSel == (int)i + 1);
                    Color cc = kClasses[std::max(0, std::min(3, ch.heroClass))].color;
                    DrawRectangleRec(r, sel ? Color{ 34, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
                    DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(cc, 0.65f));
                    DrawText(TextFormat(L("Heir %d: %s  (%s)  Age %d  Lv%d", "Con %d: %s  (%s)  %d tuổi  Lv%d"), (int)i + 1, ch.name.c_str(), ClassDisplayName(ch.heroClass), ChildAgeYears(ch), ch.level), (int)r.x + 22, (int)r.y + 12, 18, cc);
                    DrawText(TextFormat(L("Inherits: +%d HP +%d DMG +%.1f ARM +%d SPD", "Kế thừa: +%d Máu +%d ST +%.1f Giáp +%d Chạy"),
                        (int)ch.bonusHp, (int)ch.bonusDmg, ch.bonusArmor, (int)ch.bonusSpeed), (int)r.x + 22, (int)r.y + 40, 14, RAYWHITE);
                    {
                        std::string st = (ch.injuredTimer > 0.0f) ? std::string(TextFormat(L("Wounded: %ds", "Dưỡng thương: %ds"), (int)ceilf(ch.injuredTimer)))
                                       : (ch.age < (float)kChildTrainAge) ? std::string(TextFormat(L("Too young to train (%d+)", "Chưa đủ tuổi rèn luyện (%d+)"), kChildTrainAge))
                                       : (ch.inParty ? std::string(L("In training party", "Đang theo cha rèn luyện")) : std::string(L("Staying home", "Đang ở nhà")));
                        DrawText(st.c_str(), (int)r.x + 22, (int)r.y + 58, 12, ch.inParty ? Color{ 120, 245, 170, 255 } : Color{ 170, 180, 205, 255 });
                    }

                    Rectangle trB = { r.x + r.width - 546.0f, r.y + 16.0f, 118.0f, 44.0f };
                    bool canTr = ChildCanTrain(ch);
                    DrawRectangleRec(trB, ch.inParty ? Color{ 30, 96, 62, 255 } : (canTr ? Color{ 36, 70, 52, 255 } : Color{ 40, 42, 52, 255 }));
                    DrawRectangleLinesEx(trB, 1.6f, ch.inParty ? Color{ 120, 245, 170, 255 } : (canTr ? Color{ 100, 200, 140, 255 } : GRAY));
                    const char* trLbl = ch.inParty ? L("STAY HOME [T]", "Ở NHÀ [T]") : L("TRAIN [T]", "DẪN THEO [T]");
                    DrawText(trLbl, (int)(trB.x + trB.width / 2) - MeasureText(trLbl, 13) / 2, (int)trB.y + 15, 13, canTr || ch.inParty ? WHITE : GRAY);

                    Rectangle renB = { r.x + r.width - 418.0f, r.y + 16.0f, 118.0f, 44.0f };
                    DrawRectangleRec(renB, { 42, 62, 108, 255 });
                    DrawRectangleLinesEx(renB, 1.6f, SKYBLUE);
                    const char* renLbl = L("RENAME", "ĐỔI TÊN");
                    DrawText(renLbl, (int)(renB.x + renB.width / 2) - MeasureText(renLbl, 14) / 2, (int)renB.y + 14, 14, WHITE);

                    Rectangle rb = { r.x + r.width - 290.0f, r.y + 16.0f, 268.0f, 44.0f };
                    DrawRectangleRec(rb, { 112, 74, 24, 255 });
                    DrawRectangleLinesEx(rb, 1.8f, GOLD);
                    const char* ripLbl = L("REST IN PEACE -> PLAY HEIR", "REST IN PEACE -> CHƠI ĐỜI CON");
                    DrawText(ripLbl, (int)(rb.x + rb.width / 2) - MeasureText(ripLbl, 14) / 2, (int)rb.y + 15, 14, WHITE);
                }
            }
        }
    } else {
        // Hoàng Cung & Vua
        Rectangle r0 = { 120.0f, 196.0f, 1040.0f, 168.0f };
        bool sel0 = (surfaceSel == 0);
        bool affordN = (!isNoble && player.gold >= 1500);
        DrawRectangleRec(r0, sel0 ? Color{ 44, 38, 28, 252 } : Color{ 24, 22, 34, 245 });
        DrawRectangleLinesEx(r0, sel0 ? 2.8f : 1.5f, sel0 ? GOLD : Color{ 195, 155, 65, 255 });
        DrawText(L("DIRECT ROYAL GRANT: BUY NOBLE TITLE (1500 GOLD)", "MUA TƯỚC VỊ QUÝ TỘC TRỰC TIẾP (1500 VÀNG)"), (int)r0.x + 26, (int)r0.y + 18, 22, GOLD);
        DrawText(L("High-gold shortcut to Nobility: +35 HP, +6 DMG, +1.5 Armor, +25% Gold drops, +30G/Floor,",
                   "Dùng lượng Vàng lớn phong tước ngay: +35 Máu, +6 ST, +1.5 Giáp, +25% Vàng rơi, +30V/Tầng,"), (int)r0.x + 26, (int)r0.y + 56, 15, RAYWHITE);
        DrawText(L("30% discount on Food/Marriage/Children & 50% discount on Noble Estate Manor.",
                   "Giảm 30% Đồ Ăn/Cưới/Sinh Con và giảm 50% giá Dinh Thự Quý Tộc."), (int)r0.x + 26, (int)r0.y + 80, 15, SKYBLUE);
        Rectangle b0 = { r0.x + r0.width - 260.0f, r0.y + 106.0f, 236.0f, 44.0f };
        DrawRectangleRec(b0, isNoble ? Color{ 38, 86, 54, 255 } : (affordN ? Color{ 138, 96, 22, 255 } : Color{ 68, 34, 38, 255 }));
        DrawRectangleLinesEx(b0, 1.5f, GOLD);
        const char* nLbl = isNoble ? L("NOBLE TITLE OWNED", "ĐÃ LÀ QUÝ TỘC") : L("BUY NOBILITY - 1500G", "MUA QUÝ TỘC - 1500V");
        DrawText(nLbl, (int)(b0.x + b0.width / 2) - MeasureText(nLbl, 16) / 2, (int)b0.y + 14, 16, WHITE);

        Rectangle r1 = { 120.0f, 386.0f, 1040.0f, 168.0f };
        bool sel1 = (surfaceSel == 1);
        bool readyQ = (royalQuestActive && royalQuestKills >= 35 && royalQuestBosses >= 1);
        DrawRectangleRec(r1, sel1 ? Color{ 32, 42, 68, 252 } : Color{ 20, 26, 44, 245 });
        DrawRectangleLinesEx(r1, sel1 ? 2.8f : 1.5f, sel1 ? GOLD : SKYBLUE);
        DrawText(L("KING'S DUNGEON SUBJUGATION COMMISSION (FREE PATH TO NOBILITY)", "ỦY THÁC THẢO PHẠT HẦM NGỤC CỦA VUA (CON ĐƯỜNG MIỄN PHÍ)"), (int)r1.x + 26, (int)r1.y + 18, 21, SKYBLUE);
        DrawText(L("Descend into the Dungeon, slay 35 Monsters and defeat 1 Boss to earn Nobility + 300G + Relic!",
                   "Xuống Hầm Ngục, tiêu diệt 35 Quái vật và 1 Trùm để Vua phong Tước Quý Tộc + 300 Vàng + Thánh Tích!"), (int)r1.x + 26, (int)r1.y + 54, 15, RAYWHITE);
        DrawText(TextFormat(L("Progress:  Monsters %d / 35    |    Bosses %d / 1", "Tiến Độ:  Quái Đã Hạ %d / 35    |    Trùm Đã Hạ %d / 1"),
            royalQuestKills, royalQuestBosses), (int)r1.x + 26, (int)r1.y + 84, 17, readyQ ? GREEN : GOLD);
        Rectangle b1 = { r1.x + r1.width - 290.0f, r1.y + 106.0f, 266.0f, 44.0f };
        Color qFill = isNoble ? Color{ 38, 86, 54, 255 } : (readyQ ? Color{ 38, 128, 66, 255 } : (royalQuestActive ? Color{ 44, 62, 102, 255 } : Color{ 118, 52, 40, 255 }));
        DrawRectangleRec(b1, qFill);
        DrawRectangleLinesEx(b1, 1.5f, isNoble || readyQ ? GOLD : SKYBLUE);
        const char* qLbl = isNoble ? L("NOBILITY ACHIEVED", "ĐÃ HOÀN THÀNH") : (readyQ ? L("CLAIM NOBILITY!", "NHẬN PHONG QUÝ TỘC!") : (royalQuestActive ? L("IN PROGRESS (ENTER DUNGEON)", "ĐANG LÀM (XUỐNG HẦM NGỤC)") : L("ACCEPT KING'S COMMISSION", "NHẬN ỦY THÁC CỦA VUA")));
        DrawText(qLbl, (int)(b1.x + b1.width / 2) - MeasureText(qLbl, 14) / 2, (int)b1.y + 15, 14, WHITE);
    }

    const char* foot = L("1-4 / Left-Right: Switch Building    Up/Down + Enter or Click: Action    Y: Family Tree    Esc: Back",
                         "1-4 / Trái-Phải: Đổi Khu Vực    Lên/Xuống + Enter hoặc Nhấp: Chọn    Y: Cây Gia Phả    Esc: Thoát");
    DrawText(foot, 1280 / 2 - MeasureText(foot, 15) / 2, 636, 15, { 165, 180, 215, 255 });
}

inline void Game::UpdateFamilyTree() {
    { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
    if (EscPressed() || IsKeyPressed(KEY_Y) || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
        state = familyTreeReturn;
        return;
    }
}

inline void Game::DrawFamilyTreeScreen() {
    DrawRectangle(0, 0, 1280, 720, Fade({ 8, 10, 18, 255 }, 0.94f));
    const char* hdr = L("ROYAL BLOODLINE & FAMILY TREE", "CÂY GIA PHẢ DÒNG TỘC");
    OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 38) / 2, 28, 38, GOLD, { 70, 42, 10, 255 });
    std::string sub = std::string(L("Current Generation: ", "Thế Hệ Hiện Tại: Đời ")) + std::to_string(generation)
                    + "   |   " + (isNoble ? L("NOBLE DYNASTY", "GIA TỘC QUÝ TỘC") : L("Dungeon Lineage", "Dòng Tộc Chiến Binh"))
                    + "   |   " + L("Legacy Bonus: ", "Di Sản Tích Lũy: ")
                    + "+" + std::to_string((int)legacyHp) + "HP +" + std::to_string((int)legacyDmg) + "ST";
    DrawText(sub.c_str(), 1280 / 2 - MeasureText(sub.c_str(), 16) / 2, 74, 16, SKYBLUE);

    // Hàng 1: Tổ tiên đã Rest in Peace (tối đa 4 thẻ mới nhất hiển thị theo chuỗi ngang)
    int ancY = 108;
    if (familyTree.empty()) {
        DrawRectangleLines(220, ancY, 840, 118, { 75, 85, 118, 255 });
        const char* noAnc = L("No Ancestors resting in peace yet. Raise an Heir on the Surface to begin your Dynasty!",
                              "Chưa có Tổ Tiên nào an nghỉ. Hãy Lên Mặt Đất, Sinh Con và chọn Rest in Peace để lập Gia Phả!");
        DrawText(noAnc, 1280 / 2 - MeasureText(noAnc, 16) / 2, ancY + 48, 16, { 165, 180, 210, 255 });
    } else {
        int startIdx = std::max(0, (int)familyTree.size() - 4);
        int showCnt = (int)familyTree.size() - startIdx;
        int cw = 266, gap = 20, x0 = 1280 / 2 - (showCnt * cw + (showCnt - 1) * gap) / 2;
        for (int k = 0; k < showCnt; ++k) {
            const AncestorRecord& a = familyTree[startIdx + k];
            int cx = x0 + k * (cw + gap);
            DrawRectangle(cx, ancY, cw, 128, { 22, 24, 38, 245 });
            DrawRectangleLinesEx({ (float)cx, (float)ancY, (float)cw, 128.0f }, 1.6f, a.wasNoble ? GOLD : Color{ 125, 140, 180, 255 });
            DrawRectangle(cx, ancY, cw, 24, { 40, 32, 46, 255 });
            const char* ripTag = L("REST IN PEACE", "AN NGHỈ");
            DrawText(TextFormat(L("Gen %d", "Đời %d"), a.generation), cx + 10, ancY + 4, 15, GOLD);
            DrawText(ripTag, cx + cw - 10 - MeasureText(ripTag, 13), ancY + 5, 13, { 185, 195, 225, 255 });
            DrawText(a.name.c_str(), cx + 12, ancY + 30, 17, kClasses[std::max(0, std::min(3, a.heroClass))].color);
            DrawText(TextFormat(L("Lv.%d [%s]", "Cấp %d [%s]"), a.levelReached, LevelTitleNameForLevel(a.levelReached)), cx + 12, ancY + 50, 12, LevelTitleColorForLevel(a.levelReached));
            DrawText(a.wasNoble ? L("[Noble]", "[Quý Tộc]") : L("[Commoner]", "[Thường Dân]"), cx + 12, ancY + 67, 12, a.wasNoble ? GOLD : GRAY);
            DrawText((std::string(L("Spouse: ", "Vợ: ")) + a.spouseName).c_str(), cx + 12, ancY + 84, 13, { 245, 175, 210, 255 });
            DrawText(TextFormat(L("Floor %d  |  Slain %d", "Tầng %d  |  Hạ %d quái"), a.floorReached, a.enemiesSlain), cx + 12, ancY + 104, 13, RAYWHITE);
            if (k + 1 < showCnt) {
                DrawLineEx({ (float)(cx + cw), (float)(ancY + 64) }, { (float)(cx + cw + gap), (float)(ancY + 64) }, 2.5f, GOLD);
            }
        }
        DrawLineEx({ 640.0f, (float)(ancY + 128) }, { 640.0f, 272.0f }, 2.5f, GOLD);
    }

    // Hàng 2: Đời hiện tại + Bạn đời
    int curY = 272;
    Rectangle curCard = { 290.0f, (float)curY, 700.0f, 128.0f };
    DrawRectangleRec(curCard, { 24, 34, 58, 250 });
    DrawRectangleLinesEx(curCard, 2.5f, isNoble ? GOLD : HeroColor());
    DrawHeroModel({ 352.0f, (float)curY + 66.0f }, player.heroClass, ClassWeaponType(player.heroClass), { 1, 0 }, false, 0.0f, true, 0.25f, false, false, 0.0f, 0.0f, player.subClass, 0.0f);
    DrawText(TextFormat(L("CURRENT GENERATION (%d): %s", "ĐỜI HIỆN TẠI (ĐỜI %d): %s"), generation, CurrentHeroDisplayName().c_str()),
             408, curY + 16, 20, HeroColor());
    DrawText(TextFormat(L("Class: %s  |  House: %s  |  Status: %s", "Hệ: %s  |  Nhà: %s  |  Thân phận: %s"),
             HeroTitle(), HouseName(houseTier), isNoble ? L("NOBLE", "QUÝ TỘC") : L("Commoner", "Thường Dân")),
             408, curY + 46, 15, WHITE);
    DrawText(TextFormat(L("Spouse: %s   |   HP: %d/%d   DMG: %d   Gold: %d", "Vợ: %s   |   Máu: %d/%d   ST: %d   Vàng: %d"),
             SpouseName(hasSpouse ? spouseIdx : -1), (int)player.hp, (int)player.maxHp, (int)player.baseDmg, player.gold),
             408, curY + 72, 15, { 255, 210, 225, 255 });
    DrawText(TextFormat(L("Floor %d  |  Lv %d/100 (%s)  |  Slain %d", "Tầng %d  |  Cấp %d/100 (%s)  |  Hạ %d quái"),
             floor, player.level, CurrentLevelTitle(), enemiesSlain), 408, curY + 96, 13, CurrentLevelTitleColor());

    // Hàng 3: Thế hệ Con cái (Heirs)
    int kidY = 442;
    if (children.empty()) {
        DrawRectangleLines(290, kidY, 700, 124, { 75, 85, 118, 255 });
        const char* nk = L("No children yet. Build a House & Marry on the Surface to have Heirs!",
                           "Chưa có thế hệ con. Hãy Mua Nhà & Cưới Vợ trên Mặt Đất để Sinh Con nối dõi!");
        DrawText(nk, 1280 / 2 - MeasureText(nk, 16) / 2, kidY + 52, 16, { 165, 180, 210, 255 });
    } else {
        DrawLineEx({ 640.0f, (float)(curY + 128) }, { 640.0f, (float)(kidY - 18) }, 2.5f, GOLD);
        int n = (int)children.size();
        int kw = 310, kgap = 24, x0 = 1280 / 2 - (n * kw + (n - 1) * kgap) / 2;
        if (n > 1) DrawLineEx({ (float)(x0 + kw / 2), (float)(kidY - 18) }, { (float)(x0 + (n - 1) * (kw + kgap) + kw / 2), (float)(kidY - 18) }, 2.5f, GOLD);
        for (int i = 0; i < n; ++i) {
            const ChildInfo& ch = children[i];
            int cx = x0 + i * (kw + kgap);
            DrawLineEx({ (float)(cx + kw / 2), (float)(kidY - 18) }, { (float)(cx + kw / 2), (float)kidY }, 2.5f, GOLD);
            DrawRectangle(cx, kidY, kw, 142, { 22, 30, 48, 245 });
            Color cc = kClasses[std::max(0, std::min(3, ch.heroClass))].color;
            DrawRectangleLinesEx({ (float)cx, (float)kidY, (float)kw, 142.0f }, 1.8f, cc);
            DrawText(TextFormat(L("HEIR %d (Gen %d)", "CON THỨ %d (ĐỜI %d)"), i + 1, generation + 1), cx + 14, kidY + 10, 14, GOLD);
            DrawText(ch.name.c_str(), cx + 14, kidY + 32, 20, cc);
            DrawText(ClassDisplayName(ch.heroClass), cx + 14, kidY + 58, 15, WHITE);
            DrawText(TextFormat(L("+%d HP  +%d DMG", "+%d Máu  +%d ST"), (int)ch.bonusHp, (int)ch.bonusDmg), cx + 14, kidY + 86, 15, { 125, 240, 165, 255 });
            DrawText(TextFormat(L("+%.1f ARM  +%d SPD", "+%.1f Giáp  +%d Chạy"), ch.bonusArmor, (int)ch.bonusSpeed), cx + 14, kidY + 108, 14, SKYBLUE);
        }
    }
    const char* foot = L("Press Y or Esc to return   |   Visit Family Home on Surface to trigger Rest in Peace",
                         "Nhấn Y hoặc Esc để quay lại   |   Vào Tổ Ấm trên Mặt Đất để chọn Rest in Peace kế thừa");
    DrawText(foot, 1280 / 2 - MeasureText(foot, 15) / 2, 654, 15, { 165, 180, 215, 255 });
}
