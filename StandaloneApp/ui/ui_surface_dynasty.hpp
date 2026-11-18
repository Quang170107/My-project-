#pragma once
#include "../game.hpp"

inline int Game::SurfaceOptionCount() const {
    if (surfaceBuilding == 0) return 5;
    if (surfaceBuilding == 1) return 4;
    if (surfaceBuilding == 2) return hasSpouse ? (1 + (int)children.size()) : 4;
    return 4;
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
        else if (opt == 1) AcceptRoyalCommission();
        else if (opt == 2) OpenWorldMap(SURFACE_UI);
        else StartThroneRebellion();
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
        static const Rectangle cards[4] = {
            { 120.0f, 192.0f, 1040.0f, 118.0f },
            { 120.0f, 320.0f, 1040.0f, 118.0f },
            { 120.0f, 448.0f,  510.0f, 128.0f },
            { 650.0f, 448.0f,  510.0f, 128.0f }
        };
        for (int i = 0; i < 4; ++i) {
            if (CheckCollisionPointRec(m, cards[i])) {
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
                     + "  |  " + PeerageTitleName()
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
                    Color cc = kClasses[std::max(0, std::min(kNumClasses - 1, ch.heroClass))].color;
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
        // Hoàng Cung & Hệ thống Tước Vị Vương Quốc Anh + Lãnh Địa + Nổi Dậy
        int nextRank = std::min(8, peerageRank + 1);
        const PeerageDef& nxtP = kPeerageDefs[nextRank];
        int promoCost = (peerageRank == 0) ? 1500 : std::max(100, nxtP.goldCost - warMerit * 3);
        bool maxRank = (peerageRank >= 7);

        Rectangle r0 = { 120.0f, 192.0f, 1040.0f, 118.0f };
        bool sel0 = (surfaceSel == 0);
        bool affordN = (!maxRank && player.gold >= promoCost);
        DrawRectangleRec(r0, sel0 ? Color{ 44, 38, 28, 252 } : Color{ 24, 22, 34, 245 });
        DrawRectangleLinesEx(r0, sel0 ? 2.8f : 1.5f, sel0 ? GOLD : PeerageColor());
        DrawText(TextFormat(L("BRITISH PEERAGE (%s): Elevate to %s", "TƯỚC VỊ HOÀNG GIA (%s): Thăng %s"),
                            PeerageStyleName(), maxRank ? PeerageTitleName() : (settings.language == 1 ? nxtP.nameVi : nxtP.nameEn)),
                 (int)r0.x + 24, (int)r0.y + 14, 21, GOLD);
        DrawText(TextFormat(L("Knight->Baronet->Baron->Viscount->Earl->Marquess->Duke  |  Daily Tax: +%dG  |  Merit: %d",
                              "Hiệp Sĩ->Chuẩn Nam Tước->Nam Tước->Tử Tước->Bá Tước->Hầu Tước->Công Tước  |  Bổng Lộc: +%dV  |  Công: %d"),
                            kPeerageDefs[peerageRank].dailyGold, warMerit),
                 (int)r0.x + 24, (int)r0.y + 46, 14, RAYWHITE);
        DrawText(maxRank ? L("Duke reached! Open World Map or launch Throne Rebellion to seize the Crown!",
                             "Đã đạt bậc Công Tước! Mở Bản Đồ Thế Giới hoặc Nổi Dậy Đoạt Ngai Vàng!")
                         : TextFormat(L("Next Bonus: +%d HP, +%d DMG, +%.1f ARM, +%dG/day  (War Merit discounts cost!)",
                                        "Thưởng Bậc Tới: +%d Máu, +%d ST, +%.1f Giáp, +%dV/ngày  (Chiến Công giảm giá!)"),
                                      (int)nxtP.hpBonus, (int)nxtP.dmgBonus, nxtP.armorBonus, nxtP.dailyGold),
                 (int)r0.x + 24, (int)r0.y + 72, 14, SKYBLUE);
        Rectangle b0 = { r0.x + r0.width - 270.0f, r0.y + 36.0f, 246.0f, 46.0f };
        DrawRectangleRec(b0, maxRank ? Color{ 38, 86, 54, 255 } : (affordN ? Color{ 138, 96, 22, 255 } : Color{ 68, 34, 38, 255 }));
        DrawRectangleLinesEx(b0, 1.5f, GOLD);
        const char* nLbl = maxRank ? L("MAX PEERAGE: DUKE+", "ĐÃ ĐẠT BẬC TỐI ĐA")
                                   : TextFormat(L("PROMOTE - %dG", "THĂNG TƯỚC - %dV"), promoCost);
        DrawText(nLbl, (int)(b0.x + b0.width / 2) - MeasureText(nLbl, 15) / 2, (int)b0.y + 15, 15, WHITE);

        Rectangle r1 = { 120.0f, 320.0f, 1040.0f, 118.0f };
        bool sel1 = (surfaceSel == 1);
        bool readyQ = (royalQuestActive && royalQuestKills >= 35 && royalQuestBosses >= 1);
        DrawRectangleRec(r1, sel1 ? Color{ 32, 42, 68, 252 } : Color{ 20, 26, 44, 245 });
        DrawRectangleLinesEx(r1, sel1 ? 2.8f : 1.5f, sel1 ? GOLD : SKYBLUE);
        DrawText(L("KING'S DUNGEON SUBJUGATION COMMISSION (FREE NOBILITY PATH)", "ỦY THÁC THẢO PHẠT HẦM NGỤC CỦA VUA (CON ĐƯỜNG MIỄN PHÍ)"),
                 (int)r1.x + 24, (int)r1.y + 14, 20, SKYBLUE);
        DrawText(L("Slay 35 Monsters & 1 Boss in the Dungeon for Knighthood + 300 Gold + Relic (+War Merit!).",
                   "Diệt 35 Quái vật & 1 Trùm dưới Hầm Ngục để nhận Phong Tước + 300 Vàng + Thánh Tích!"),
                 (int)r1.x + 24, (int)r1.y + 44, 14, RAYWHITE);
        DrawText(TextFormat(L("Progress:  Monsters %d / 35    |    Bosses %d / 1", "Tiến Độ:  Quái Đã Hạ %d / 35    |    Trùm Đã Hạ %d / 1"),
                            royalQuestKills, royalQuestBosses),
                 (int)r1.x + 24, (int)r1.y + 72, 16, readyQ ? GREEN : GOLD);
        Rectangle b1 = { r1.x + r1.width - 270.0f, r1.y + 36.0f, 246.0f, 46.0f };
        Color qFill = isNoble ? Color{ 38, 86, 54, 255 } : (readyQ ? Color{ 38, 128, 66, 255 } : (royalQuestActive ? Color{ 44, 62, 102, 255 } : Color{ 118, 52, 40, 255 }));
        DrawRectangleRec(b1, qFill);
        DrawRectangleLinesEx(b1, 1.5f, isNoble || readyQ ? GOLD : SKYBLUE);
        const char* qLbl = isNoble ? L("NOBILITY ACHIEVED", "ĐÃ ĐẠT QUÝ TỘC")
                                   : (readyQ ? L("CLAIM NOBILITY!", "NHẬN PHONG TƯỚC!")
                                             : (royalQuestActive ? L("IN PROGRESS", "ĐANG THỰC HIỆN")
                                                                 : L("ACCEPT COMMISSION", "NHẬN ỦY THÁC")));
        DrawText(qLbl, (int)(b1.x + b1.width / 2) - MeasureText(qLbl, 14) / 2, (int)b1.y + 15, 14, WHITE);

        Rectangle r2 = { 120.0f, 448.0f, 510.0f, 128.0f };
        bool sel2 = (surfaceSel == 2);
        DrawRectangleRec(r2, sel2 ? Color{ 28, 48, 64, 252 } : Color{ 18, 28, 44, 245 });
        DrawRectangleLinesEx(r2, sel2 ? 2.8f : 1.5f, sel2 ? GOLD : Color{ 110, 230, 175, 255 });
        DrawText(L("WORLD MAP, FIEFDOM & WARS", "BẢN ĐỒ THẾ GIỚI, LÃNH ĐỊA & CHIẾN TRANH"),
                 (int)r2.x + 20, (int)r2.y + 14, 18, { 120, 240, 185, 255 });
        DrawText(fiefEstablished ? TextFormat(L("Fief: Lv.%d  |  Troops: %d  |  Tax: +%dG", "Lãnh Địa: Cấp %d | Quân: %d | Thuế: +%dV"),
                                              fiefLevel, fiefSoldiers, FiefDailyGoldIncome())
                                 : L("Found your Fief, manage 5 Nations, Wage War & Annex!",
                                     "Lập Lãnh Địa Riêng, Ngoại Giao & Tuyên Chiến 5 Quốc Gia!"),
                 (int)r2.x + 20, (int)r2.y + 44, 14, RAYWHITE);
        Rectangle b2 = { r2.x + 20.0f, r2.y + 72.0f, r2.width - 40.0f, 40.0f };
        DrawRectangleRec(b2, { 26, 78, 96, 255 });
        DrawRectangleLinesEx(b2, 1.5f, { 120, 240, 185, 255 });
        const char* mLbl = L("OPEN WORLD MAP [K / TAB]", "MỞ BẢN ĐỒ THẾ GIỚI [K / TAB]");
        DrawText(mLbl, (int)(b2.x + b2.width / 2) - MeasureText(mLbl, 15) / 2, (int)b2.y + 12, 15, WHITE);

        Rectangle r3 = { 650.0f, 448.0f, 510.0f, 128.0f };
        bool sel3 = (surfaceSel == 3);
        float pwr = ComputePlayerPowerScore();
        int estKingHp = (int)roundf((1800.0f + player.maxHp * 16.0f + player.baseDmg * 58.0f + pwr * 2.8f) * DiffHp());
        DrawRectangleRec(r3, sel3 ? Color{ 62, 26, 34, 252 } : Color{ 38, 18, 26, 245 });
        DrawRectangleLinesEx(r3, sel3 ? 2.8f : 1.5f, sel3 ? GOLD : Color{ 255, 110, 110, 255 });
        DrawText(L("THRONE REBELLION (USURP CROWN)", "NỔI DẬY CHIẾM NGAI VÀNG"),
                 (int)r3.x + 20, (int)r3.y + 14, 18, { 255, 125, 125, 255 });
        DrawText(TextFormat(L("Power: %d -> Scaled King ~%d HP (Random Class)",
                              "Lực Chiến: %d -> Vua ~%d Máu (Class Ngẫu Nhiên)"), (int)roundf(pwr), estKingHp),
                 (int)r3.x + 20, (int)r3.y + 44, 14, GOLD);
        Rectangle b3 = { r3.x + 20.0f, r3.y + 72.0f, r3.width - 40.0f, 40.0f };
        DrawRectangleRec(b3, { 135, 32, 42, 255 });
        DrawRectangleLinesEx(b3, 1.5f, GOLD);
        const char* kLbl = isKing ? L("DEFEND THRONE VS ROYAL GUARD!", "THỬ THÁCH VỆ BINH HOÀNG GIA!")
                                  : L("STORM THE PALACE & CLAIM CROWN!", "TIẾN VÀO ĐẠI ĐIỆN SOÁN NGÔI VUA!");
        DrawText(kLbl, (int)(b3.x + b3.width / 2) - MeasureText(kLbl, 15) / 2, (int)b3.y + 12, 15, WHITE);
    }

    const char* foot = L("1-4 / Left-Right: Building   Arrows+Enter/Click: Action   K: World Map   Y: Family Tree   Esc: Back",
                         "1-4 / Trái-Phải: Khu Vực   Mũi Tên+Enter/Nhấp: Chọn   K: Bản Đồ   Y: Gia Phả   Esc: Thoát");
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
            DrawText(a.name.c_str(), cx + 12, ancY + 30, 17, kClasses[std::max(0, std::min(kNumClasses - 1, a.heroClass))].color);
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
            Color cc = kClasses[std::max(0, std::min(kNumClasses - 1, ch.heroClass))].color;
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

// ── Bố cục nút bấm dùng chung giữa UpdateWorldMap và DrawWorldMapScreen ──
struct WorldMapUiLayout {
    Rectangle rPromo   = { 1070.0f, 156.0f, 156.0f, 44.0f };
    Rectangle rFound   = {  814.0f, 304.0f, 410.0f, 52.0f };
    Rectangle rBldg[4] = {
        {  812.0f, 254.0f, 202.0f, 34.0f },
        { 1024.0f, 254.0f, 202.0f, 34.0f },
        {  812.0f, 294.0f, 202.0f, 34.0f },
        { 1024.0f, 294.0f, 202.0f, 34.0f }
    };
    Rectangle rCollect = {  812.0f, 336.0f, 202.0f, 34.0f };
    Rectangle rRecruit = { 1024.0f, 336.0f, 202.0f, 34.0f };
    Rectangle rWar     = {  812.0f, 488.0f, 134.0f, 40.0f };
    Rectangle rArmy    = {  952.0f, 488.0f, 134.0f, 40.0f };
    Rectangle rBattle  = { 1092.0f, 488.0f, 134.0f, 40.0f };
    Rectangle rRebel   = {  812.0f, 602.0f, 414.0f, 38.0f };
};

inline void Game::OpenWorldMap(State from) {
    if (nations[0].nameEn == nullptr) InitNations();
    worldMapReturn = from;
    worldMapSel = std::max(0, std::min(4, worldMapSel));
    if (worldMapSel == 0) worldMapSel = 2; // Mặc định chọn một quốc gia lân bang để ngoại giao / giao chiến
    state = WORLD_MAP;
    PlaySound(snd.sndPortal);
}

inline void Game::UpdateWorldMap(float dt) {
    if (bannerTimer > 0.0f) bannerTimer = std::max(0.0f, bannerTimer - dt);
    { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_F3)) {
        state = worldMapReturn;
        return;
    }

    if (IsKeyPressed(KEY_ONE))   worldMapSel = 0;
    if (IsKeyPressed(KEY_TWO))   worldMapSel = 1;
    if (IsKeyPressed(KEY_THREE)) worldMapSel = 2;
    if (IsKeyPressed(KEY_FOUR))  worldMapSel = 3;
    if (IsKeyPressed(KEY_FIVE))  worldMapSel = 4;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP))    worldMapSel = (worldMapSel + 4) % 5;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN)) worldMapSel = (worldMapSel + 1) % 5;

    if (IsKeyPressed(KEY_P)) PromotePeerage();
    if (IsKeyPressed(KEY_E)) EstablishFief();
    if (IsKeyPressed(KEY_C)) CollectFiefIncome();
    if (IsKeyPressed(KEY_T)) RecruitFiefSoldiers();
    if (IsKeyPressed(KEY_W)) DeclareWarOrAlliance(worldMapSel);
    if (IsKeyPressed(KEY_A)) DispatchArmyCampaign(worldMapSel);
    if (IsKeyPressed(KEY_F)) { StartNationalBattlefield(worldMapSel); return; }
    if (IsKeyPressed(KEY_R)) { StartThroneRebellion(); return; }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 5; ++i) {
            if (CheckCollisionPointCircle(m, nations[i].mapPos, 36.0f)) {
                worldMapSel = i;
                PlaySound(snd.sndCoin);
                return;
            }
        }
        const WorldMapUiLayout ui;
        if (CheckCollisionPointRec(m, ui.rPromo)) { PromotePeerage(); return; }
        if (!fiefEstablished) {
            if (CheckCollisionPointRec(m, ui.rFound)) { EstablishFief(); return; }
        } else {
            for (int b = 0; b < 4; ++b) {
                if (CheckCollisionPointRec(m, ui.rBldg[b])) { UpgradeFiefBuilding(b); return; }
            }
            if (CheckCollisionPointRec(m, ui.rCollect)) { CollectFiefIncome(); return; }
            if (CheckCollisionPointRec(m, ui.rRecruit)) { RecruitFiefSoldiers(); return; }
        }
        if (worldMapSel >= 1) {
            if (CheckCollisionPointRec(m, ui.rWar))    { DeclareWarOrAlliance(worldMapSel); return; }
            if (CheckCollisionPointRec(m, ui.rArmy))   { DispatchArmyCampaign(worldMapSel); return; }
            if (CheckCollisionPointRec(m, ui.rBattle)) { StartNationalBattlefield(worldMapSel); return; }
        }
        if (CheckCollisionPointRec(m, ui.rRebel)) { StartThroneRebellion(); return; }
    }
}

inline void Game::DrawWorldMapScreen() {
    if (nations[0].nameEn == nullptr) InitNations();
    const WorldMapUiLayout ui;
    const Vector2 m = GetMousePosition();
    const float tm = (float)GetTime();

    DrawRectangle(0, 0, 1280, 720, { 8, 12, 22, 255 });
    DrawRectangleGradientV(0, 0, 1280, 720, { 12, 18, 34, 240 }, { 7, 10, 18, 255 });

    const char* title = L("WORLD MAP, FIEFDOM & THRONE REBELLION", "BẢN ĐỒ THẾ GIỚI, LÃNH ĐỊA & ĐOẠT NGAI VÀNG");
    DrawText(title, 640 - MeasureText(title, 26) / 2, 22, 26, GOLD);
    const char* topInfo = TextFormat(L("%s (%s)  |  War Merit: %d  |  Gold: %d  |  Fief Troops: %d  |  Day %d",
                                       "%s (%s)  |  Chiến Công: %d  |  Vàng: %d  |  Quân Lãnh Địa: %d  |  Ngày %d"),
                                     PeerageTitleName(), PeerageStyleName(), warMerit, player.gold, fiefSoldiers, dayCount);
    DrawText(topInfo, 640 - MeasureText(topInfo, 15) / 2, 56, 15, PeerageColor());

    // ── Khung Trái: Bản Đồ Lục Địa 5 Quốc Gia ───────────────────────────────
    Rectangle rMap = { 30.0f, 98.0f, 740.0f, 562.0f };
    DrawRectangleRec(rMap, { 13, 21, 36, 248 });
    DrawRectangleLinesEx(rMap, 2.0f, { 75, 110, 165, 255 });
    for (int gx = 90; gx <= 730; gx += 80) DrawLine(gx, 104, gx, 654, { 26, 38, 62, 120 });
    for (int gy = 150; gy <= 630; gy += 80) DrawLine(36, gy, 764, gy, { 26, 38, 62, 120 });
    DrawEllipse(400, 365, 310, 230, { 24, 38, 56, 190 });
    DrawEllipseLines(400, 365, 310, 230, { 55, 88, 125, 180 });
    DrawText(L("CONTINENTAL REALM CHART", "LỤC ĐỊA NGŨ QUỐC"), 48, 110, 15, { 145, 175, 225, 220 });

    // Đường nối ngoại giao / chiến tranh từ kinh đô Albion tới 4 nước còn lại
    for (int i = 1; i < 5; ++i) {
        const NationState& nt = nations[i];
        Color lc = { 80, 96, 125, 130 };
        float thick = 2.0f;
        if (nt.status == 1)      { lc = { 90, 225, 140, 200 }; thick = 3.0f; }
        else if (nt.status == 2) { lc = { 245, 75, 75, (unsigned char)(175 + 75 * sinf(tm * 5.0f + i)) }; thick = 3.4f; }
        else if (nt.status == 3) { lc = Fade(GOLD, 0.85f); thick = 3.2f; }
        DrawLineEx(nations[0].mapPos, nt.mapPos, thick, lc);
    }

    // Biểu tượng Lãnh Địa Riêng trên bản đồ
    Vector2 fiefMapPos = { 415.0f, 250.0f };
    if (fiefEstablished) {
        DrawLineEx(nations[0].mapPos, fiefMapPos, 2.2f, Fade(GOLD, 0.7f));
        DrawCircleV(fiefMapPos, 17.0f, { 42, 34, 18, 245 });
        DrawCircleLines((int)fiefMapPos.x, (int)fiefMapPos.y, 17.0f, GOLD);
        DrawRectangle((int)fiefMapPos.x - 7, (int)fiefMapPos.y - 6, 14, 12, GOLD);
        const char* fTag = TextFormat(L("FIEF Lv.%d", "LÃNH ĐỊA %d"), fiefLevel);
        DrawText(fTag, (int)fiefMapPos.x - MeasureText(fTag, 12) / 2, (int)fiefMapPos.y + 20, 12, GOLD);
    } else {
        DrawCircleLines((int)fiefMapPos.x, (int)fiefMapPos.y, 14.0f, { 110, 125, 150, 140 });
        const char* fTag = L("Empty Fief [E]", "Chưa Lập [E]");
        DrawText(fTag, (int)fiefMapPos.x - MeasureText(fTag, 11) / 2, (int)fiefMapPos.y + 18, 11, { 145, 160, 190, 190 });
    }

    // Nút 5 Quốc Gia trên Bản Đồ
    for (int i = 0; i < 5; ++i) {
        const NationState& nt = nations[i];
        bool sel = (worldMapSel == i);
        bool hov = CheckCollisionPointCircle(m, nt.mapPos, 32.0f);
        if (sel) DrawCircleLines((int)nt.mapPos.x, (int)nt.mapPos.y, 31.0f + sinf(tm * 4.0f) * 2.5f, GOLD);
        DrawCircleV(nt.mapPos, 24.0f, { (unsigned char)(nt.color.r / 3), (unsigned char)(nt.color.g / 3), (unsigned char)(nt.color.b / 3), 245 });
        DrawCircleLines((int)nt.mapPos.x, (int)nt.mapPos.y, 24.0f, (sel || hov) ? GOLD : nt.color);
        // Tháp thành nổi
        DrawRectangle((int)nt.mapPos.x - 9, (int)nt.mapPos.y - 7, 18, 15, nt.color);
        DrawTriangle({ nt.mapPos.x - 11, nt.mapPos.y - 7 }, { nt.mapPos.x + 11, nt.mapPos.y - 7 }, { nt.mapPos.x, nt.mapPos.y - 16 }, WHITE);
        DrawText(TextFormat("%d", i + 1), (int)nt.mapPos.x - 4, (int)nt.mapPos.y - 4, 12, BLACK);

        const char* nName = L(nt.nameEn, nt.nameVi);
        DrawText(nName, (int)nt.mapPos.x - MeasureText(nName, 14) / 2, (int)nt.mapPos.y + 28, 14, sel ? GOLD : RAYWHITE);

        const char* stLbl = L("PEACE", "HÒA BÌNH");
        Color stCol = SKYBLUE;
        if (nt.status == 1)      { stLbl = L("ALLIED", "ĐỒNG MINH"); stCol = GREEN; }
        else if (nt.status == 2) { stLbl = TextFormat(L("AT WAR (%d%%)", "CHIẾN TRANH (%d%%)"), (int)roundf(nt.warProgress)); stCol = RED; }
        else if (nt.status == 3) { stLbl = L("ANNEXED", "ĐÃ CHINH PHẠT"); stCol = GOLD; }
        DrawText(stLbl, (int)nt.mapPos.x - MeasureText(stLbl, 12) / 2, (int)nt.mapPos.y + 46, 12, stCol);
    }

    if (bannerTimer > 0.0f && !bannerText.empty()) {
        DrawRectangle(44, 610, 712, 38, { 18, 26, 44, 245 });
        DrawRectangleLines(44, 610, 712, 38, GOLD);
        DrawText(bannerText.c_str(), 400 - MeasureText(bannerText.c_str(), 15) / 2, 621, 15, GOLD);
    }

    // ── Khung Phải: Quý Tộc Anh, Lãnh Địa, Ngoại Giao, Nổi Dậy ──────────────
    DrawRectangle(788, 98, 462, 562, { 15, 20, 34, 248 });
    DrawRectangleLines(788, 98, 462, 562, { 75, 110, 165, 255 });

    // 1. Khối Tước Vị Chuẩn Vương Quốc Anh
    DrawRectangle(800, 110, 438, 102, { 24, 26, 40, 245 });
    DrawRectangleLines(800, 110, 438, 102, PeerageColor());
    DrawText(L("BRITISH PEERAGE (UK STANDARD)", "TƯỚC VỊ QUÝ TỘC CHUẨN VƯƠNG QUỐC ANH"), 812, 118, 14, GOLD);
    DrawText(TextFormat(L("Current: %s (%d/8)  |  +%dG/day", "Hiện Tại: %s (%d/8)  |  +%dV/ngày"),
                        PeerageTitleName(), peerageRank, kPeerageDefs[peerageRank].dailyGold),
             812, 138, 14, PeerageColor());
    int nxtR = std::min(8, peerageRank + 1);
    int pCost = (peerageRank == 0) ? 1500 : std::max(100, kPeerageDefs[nxtR].goldCost - warMerit * 3);
    if (peerageRank >= 7) {
        DrawText(peerageRank >= 8 ? L("Supreme Monarch! (+15% DMG Aura)", "Quốc Vương Tối Cao! (+15% ST)")
                                  : L("Duke (Max Peerage). Rebel for King!", "Công Tước (Tối Đa). Nổi dậy đoạt Vương!"),
                 812, 168, 13, SKYBLUE);
    } else {
        DrawText(TextFormat(L("Next: %s", "Tiếp: %s"), settings.language == 1 ? kPeerageDefs[nxtR].nameVi : kPeerageDefs[nxtR].nameEn),
                 812, 162, 13, RAYWHITE);
        DrawText(TextFormat(L("Cost: %dG  (+%d HP +%d DMG)", "Giá: %dV  (+%d Máu +%d ST)"),
                            pCost, (int)kPeerageDefs[nxtR].hpBonus, (int)kPeerageDefs[nxtR].dmgBonus),
                 812, 182, 12, { 150, 235, 180, 255 });
    }
    bool hovP = CheckCollisionPointRec(m, ui.rPromo);
    DrawRectangleRec(ui.rPromo, (peerageRank >= 7) ? Color{ 44, 72, 54, 255 } : (player.gold >= pCost ? Color{ 138, 96, 24, 255 } : Color{ 68, 34, 40, 255 }));
    DrawRectangleLinesEx(ui.rPromo, hovP ? 2.2f : 1.3f, GOLD);
    const char* pBtn = (peerageRank >= 7) ? L("MAX PEERAGE", "TỐI ĐA") : L("[P] PROMOTE", "[P] THĂNG TƯỚC");
    DrawText(pBtn, (int)(ui.rPromo.x + ui.rPromo.width / 2) - MeasureText(pBtn, 14) / 2, (int)ui.rPromo.y + 15, 14, WHITE);

    // 2. Khối Lãnh Địa Riêng
    DrawRectangle(800, 220, 438, 158, { 20, 28, 44, 245 });
    DrawRectangleLines(800, 220, 438, 158, { 110, 215, 175, 255 });
    if (!fiefEstablished) {
        int fCost = (peerageRank >= 3) ? 0 : (isNoble ? 250 : 500);
        DrawText(L("PERSONAL FIEFDOM", "LÃNH ĐỊA RIÊNG"), 812, 228, 15, { 120, 240, 185, 255 });
        DrawText(L("Found your own domain with Keep, Barracks, Market & Academy.",
                   "Khởi tạo Thành Trì riêng với Thành Quách, Doanh Trại, Thương Điếm & Học Viện."), 812, 250, 12, RAYWHITE);
        DrawText(L("Grants +40 HP, +8 DMG, +1.5 ARM, +50 Troops & Daily Taxes (Free at Baron).",
                   "Nhận +40 Máu, +8 ST, +1.5 Giáp, +50 Quân & Thuế ngày (Miễn phí từ Nam Tước)."), 812, 270, 12, SKYBLUE);
        bool hovF = CheckCollisionPointRec(m, ui.rFound);
        DrawRectangleRec(ui.rFound, (player.gold >= fCost) ? Color{ 32, 108, 72, 255 } : Color{ 74, 36, 42, 255 });
        DrawRectangleLinesEx(ui.rFound, hovF ? 2.4f : 1.5f, GOLD);
        const char* fBtn = fCost == 0 ? L("[E] FOUND FIEFDOM (FREE)", "[E] LẬP LÃNH ĐỊA (MIỄN PHÍ)")
                                      : TextFormat(L("[E] FOUND FIEFDOM (%dG)", "[E] LẬP LÃNH ĐỊA (%dV)"), fCost);
        DrawText(fBtn, (int)(ui.rFound.x + ui.rFound.width / 2) - MeasureText(fBtn, 16) / 2, (int)ui.rFound.y + 18, 16, WHITE);
    } else {
        DrawText(TextFormat(L("%s (Lv.%d)", "%s (Cấp %d)"), fiefName.c_str(), fiefLevel), 812, 228, 14, GOLD);
        const char* rStat = TextFormat(L("Troops: %d  |  +%dG/d", "Quân: %d  |  +%dV/ng"), fiefSoldiers, FiefDailyGoldIncome());
        DrawText(rStat, 1228 - MeasureText(rStat, 12), 230, 12, { 130, 240, 175, 255 });
        static const char* bEn[4] = { "Keep", "Barracks", "Market", "Academy" };
        static const char* bVi[4] = { "Thành", "Doanh Trại", "Chợ", "Học Viện" };
        for (int b = 0; b < 4; ++b) {
            bool hov = CheckCollisionPointRec(m, ui.rBldg[b]);
            int bCost = 180 + fiefBuildings[b] * 140;
            bool maxB = fiefBuildings[b] >= 5;
            DrawRectangleRec(ui.rBldg[b], maxB ? Color{ 36, 72, 54, 255 } : (player.gold >= bCost ? Color{ 30, 52, 86, 255 } : Color{ 44, 34, 48, 255 }));
            DrawRectangleLinesEx(ui.rBldg[b], hov ? 2.0f : 1.2f, hov ? GOLD : Color{ 100, 155, 215, 255 });
            const char* bTxt = maxB ? TextFormat("%s Lv.5 MAX", settings.language == 1 ? bVi[b] : bEn[b])
                                    : TextFormat("%s Lv.%d (%d%s)", settings.language == 1 ? bVi[b] : bEn[b], fiefBuildings[b], bCost, settings.language == 1 ? "V" : "G");
            DrawText(bTxt, (int)(ui.rBldg[b].x + ui.rBldg[b].width / 2) - MeasureText(bTxt, 12) / 2, (int)ui.rBldg[b].y + 11, 12, WHITE);
        }
        bool hovC = CheckCollisionPointRec(m, ui.rCollect);
        DrawRectangleRec(ui.rCollect, { 108, 78, 22, 255 });
        DrawRectangleLinesEx(ui.rCollect, hovC ? 2.0f : 1.3f, GOLD);
        const char* cTxt = TextFormat(L("[C] Tax (+%dG)", "[C] Thu Thuế (+%dV)"), std::max(FiefDailyGoldIncome(), fiefTreasury));
        DrawText(cTxt, (int)(ui.rCollect.x + ui.rCollect.width / 2) - MeasureText(cTxt, 13) / 2, (int)ui.rCollect.y + 10, 13, WHITE);

        bool hovT = CheckCollisionPointRec(m, ui.rRecruit);
        DrawRectangleRec(ui.rRecruit, (player.gold >= 100) ? Color{ 34, 92, 62, 255 } : Color{ 68, 34, 40, 255 });
        DrawRectangleLinesEx(ui.rRecruit, hovT ? 2.0f : 1.3f, { 125, 240, 165, 255 });
        const char* tTxt = TextFormat(L("[T] +%d Troops (100G)", "[T] +%d Quân (100V)"), 35 + fiefBuildings[1] * 10);
        DrawText(tTxt, (int)(ui.rRecruit.x + ui.rRecruit.width / 2) - MeasureText(tTxt, 13) / 2, (int)ui.rRecruit.y + 10, 13, WHITE);
    }

    // 3. Khối Quốc Gia & Chiến Tranh
    int sel = std::max(0, std::min(4, worldMapSel));
    const NationState& nt = nations[sel];
    DrawRectangle(800, 386, 438, 154, { 22, 24, 38, 245 });
    DrawRectangleLines(800, 386, 438, 154, nt.color);
    DrawText(TextFormat("[%d] %s", sel + 1, L(nt.nameEn, nt.nameVi)), 812, 394, 16, nt.color);
    DrawText(TextFormat(L("Capital: %s  |  Army: %d  |  Provinces: %d", "Kinh Đô: %s  |  Binh Lực: %d  |  Lãnh Thổ: %d"),
                        L(nt.capitalEn, nt.capitalVi), nt.militaryPower, nt.territories),
             812, 418, 12, RAYWHITE);
    // Thanh Tiến Độ Chiến Tranh
    DrawRectangle(812, 442, 414, 18, { 14, 16, 26, 255 });
    DrawRectangle(812, 442, (int)(414 * Clamp(nt.warProgress / 100.0f, 0.0f, 1.0f)), 18, nt.status == 3 ? GOLD : ORANGE);
    DrawRectangleLines(812, 442, 414, 18, { 95, 110, 145, 255 });
    const char* wpLbl = TextFormat(L("Conquest Progress: %d%%", "Tiến Độ Chinh Phạt: %d%%"), (int)roundf(nt.warProgress));
    DrawText(wpLbl, 1019 - MeasureText(wpLbl, 12) / 2, 445, 12, WHITE);

    if (sel == 0) {
        DrawWrappedText(L("Homeland of Albion Crown. Select Foreign Nations [2-5] to forge Alliances, send Fief Armies, or lead Battlefield Sieges!",
                          "Mẫu quốc Hoàng Triều Albion. Chọn 4 quốc gia [2-5] để Ký Đồng Minh, Điều Binh hoặc Đích Thân Thân Chinh!"),
                        812, 472, 414, 13, 18, SKYBLUE);
    } else {
        bool hovW = CheckCollisionPointRec(m, ui.rWar);
        DrawRectangleRec(ui.rWar, nt.status == 2 ? Color{ 38, 86, 56, 255 } : Color{ 118, 42, 42, 255 });
        DrawRectangleLinesEx(ui.rWar, hovW ? 2.0f : 1.2f, WHITE);
        const char* wTxt = (nt.status == 2) ? L("[W] Peace 250G", "[W] Hòa 250V") : L("[W] Declare War", "[W] Tuyên Chiến");
        DrawText(wTxt, (int)(ui.rWar.x + ui.rWar.width / 2) - MeasureText(wTxt, 12) / 2, (int)ui.rWar.y + 14, 12, WHITE);

        bool hovA = CheckCollisionPointRec(m, ui.rArmy);
        DrawRectangleRec(ui.rArmy, { 126, 78, 28, 255 });
        DrawRectangleLinesEx(ui.rArmy, hovA ? 2.0f : 1.2f, GOLD);
        const char* aTxt = L("[A] Army (-20)", "[A] Điều Binh");
        DrawText(aTxt, (int)(ui.rArmy.x + ui.rArmy.width / 2) - MeasureText(aTxt, 12) / 2, (int)ui.rArmy.y + 14, 12, WHITE);

        bool hovB = CheckCollisionPointRec(m, ui.rBattle);
        DrawRectangleRec(ui.rBattle, { 142, 32, 44, 255 });
        DrawRectangleLinesEx(ui.rBattle, hovB ? 2.0f : 1.2f, GOLD);
        const char* bTxt = L("[F] Lead Battle!", "[F] Thân Chinh!");
        DrawText(bTxt, (int)(ui.rBattle.x + ui.rBattle.width / 2) - MeasureText(bTxt, 12) / 2, (int)ui.rBattle.y + 14, 12, WHITE);
    }

    // 4. Khối Nổi Dậy Chiếm Ngai Vàng (Vua scale chỉ số + random class)
    DrawRectangle(800, 548, 438, 102, { 42, 16, 24, 248 });
    DrawRectangleLines(800, 548, 438, 102, GOLD);
    float pwr = ComputePlayerPowerScore();
    int estKingHp = (int)roundf((1800.0f + player.maxHp * 16.0f + player.baseDmg * 58.0f + pwr * 2.8f) * DiffHp());
    DrawText(L("THRONE REBELLION (RANDOM-CLASS SCALED KING)", "NỔI DẬY ĐOẠT NGAI VÀNG (VUA SCALE CHỈ SỐ + RANDOM CLASS)"), 812, 556, 13, GOLD);
    DrawText(TextFormat(L("Your Power: %d   |   King Scaled HP: ~%d   |   Hero Class: RANDOM",
                          "Lực Chiến: %d   |   Vua Scale Máu: ~%d   |   Class: NGẪU NHIÊN"),
                        (int)roundf(pwr), estKingHp), 812, 578, 12, RAYWHITE);
    bool hovR = CheckCollisionPointRec(m, ui.rRebel);
    DrawRectangleRec(ui.rRebel, { 155, 28, 38, 255 });
    DrawRectangleLinesEx(ui.rRebel, hovR ? 2.4f : 1.5f, GOLD);
    const char* rTxt = isKing ? L("[R] ROYAL TRIAL: SPAR THRONE GUARDIAN", "[R] THỬ THÁCH ĐỆ TỬ HỘ GIÁ NGAI VÀNG")
                              : L("[R] STORM THE PALACE & USURP THRONE!", "[R] NỔI DẬY TIẾN ĐẠI ĐIỆN CHIẾM NGAI VÀNG!");
    DrawText(rTxt, (int)(ui.rRebel.x + ui.rRebel.width / 2) - MeasureText(rTxt, 14) / 2, (int)ui.rRebel.y + 12, 14, WHITE);

    const char* foot = L("1-5/Arrows: Nation  |  P: Peerage  |  E/C/T: Fief  |  W/A/F: War & Battle  |  R: Rebel  |  Esc: Back",
                         "1-5/Mũi Tên: Quốc Gia  |  P: Tước Vị  |  E/C/T: Lãnh Địa  |  W/A/F: Chiến Tranh  |  R: Nổi Dậy  |  Esc: Thoát");
    DrawText(foot, 640 - MeasureText(foot, 14) / 2, 678, 14, { 165, 185, 220, 255 });
}
