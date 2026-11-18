#pragma once
#include "../game.hpp"

inline void Game::DrawEnemyBody(const Enemy& e) {
    const float t = (float)GetTime();
    const Vector2 c = e.pos;
    float r = e.radius;
    Vector2 look = Vector2Subtract(player.pos, e.pos);
    look = (Vector2Length(look) > 0.01f) ? Vector2Normalize(look) : Vector2{ 0, 1 };
    const float face = (look.x < -0.10f) ? -1.0f : (look.x > 0.10f ? 1.0f : (e.facingLeft ? -1.0f : 1.0f));
    const Color ink = { 18, 16, 26, 255 };

    if (e.isKingBoss || e.isNationalGeneral) {
        int cls = std::max(0, std::min(kNumClasses - 1, e.isKingBoss ? e.kingClass : (int)CLASS_TEMPLAR));
        Color rCol = e.isKingBoss ? GOLD : nations[std::max(1, std::min(4, e.generalNation))].color;
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(c.x, c.y, 64.0f, Fade(e.enraged ? RED : rCol, 0.35f), Fade(rCol, 0.0f));
        EndBlendMode();
        DrawPolyLinesEx(c, 8, 44.0f, t * 28.0f, 2.2f, Fade(rCol, 0.72f));
        rlPushMatrix();
        rlTranslatef(c.x, c.y, 0.0f);
        rlScalef(1.42f, 1.42f, 1.0f);
        DrawHeroModel({ 0.0f, 0.0f }, cls, ClassWeaponType(cls), look, face < 0.0f, e.bounceTimer, true,
                      sinf(t * 4.5f) * 0.35f, e.hitFlash > 0.0f, true, e.enraged ? 1.0f : 0.0f, 0.0f, e.enraged ? 2 : 1, 0.0f);
        rlPopMatrix();
        if (e.isKingBoss) {
            float cy = c.y - 45.0f + sinf(t * 3.5f) * 2.0f;
            Tri({ c.x - 13.0f, cy + 5.0f }, { c.x - 15.0f, cy - 10.0f }, { c.x - 5.0f, cy + 5.0f }, GOLD);
            Tri({ c.x - 6.0f,  cy + 5.0f }, { c.x,         cy - 14.0f }, { c.x + 6.0f, cy + 5.0f }, GOLD);
            Tri({ c.x + 5.0f,  cy + 5.0f }, { c.x + 15.0f, cy - 10.0f }, { c.x + 13.0f, cy + 5.0f }, GOLD);
            DrawRectangle((int)c.x - 14, (int)cy + 3, 28, 6, { 215, 165, 35, 255 });
            DrawRectangleLines((int)c.x - 14, (int)cy + 3, 28, 6, ink);
            Diamond({ c.x, cy + 1.0f }, 3.5f, 4.0f, RED, WHITE);
        }
        return;
    }

    Color base = { 90, 210, 90, 255 };
    if (e.type == SPLITTER) base = { 70, 190, 205, 255 };
    else if (e.type == SUMMONER) base = { 96, 60, 168, 255 };
    else if (e.type == RANGER) base = { 228, 72, 88, 255 };
    else if (e.type == BRUTE) base = e.isWarden ? Color{ 214, 156, 62, 255 } : Color{ 146, 92, 206, 255 };
    else if (e.type == BOSS) base = (e.bossKind == 1) ? (e.enraged ? Color{ 205, 62, 30, 255 } : Color{ 165, 78, 44, 255 }) : (e.bossKind == 2 ? (e.enraged ? Color{ 45, 165, 235, 255 } : Color{ 58, 132, 205, 255 }) : (e.enraged ? Color{ 150, 40, 70, 255 } : Color{ 88, 46, 142, 255 }));
    else if (e.type == BOMBER) base = { 255, 140, 38, 255 };
    else if (e.type == CHARGER) base = { 205, 120, 66, 255 };
    if (e.isGoldenGoblin) base = { 255, 210, 55, 255 };

    if (e.isGoldenGoblin) {
        float bob = sinf(t * 14.0f) * 3.0f;
        DrawCircle((int)c.x, (int)c.y, r + 10.0f, Fade(GOLD, 0.22f + 0.12f * sinf(t * 8.0f)));
        DrawEllipse((int)c.x, (int)c.y + r * 0.9f, r * 0.9f, r * 0.35f, { 0, 0, 0, 110 });
        // Túi vàng trên lưng
        DrawCircle((int)(c.x - face * r * 0.55f), (int)(c.y - 2.0f + bob), r * 0.65f, { 215, 155, 35, 255 });
        DrawCircleLines((int)(c.x - face * r * 0.55f), (int)(c.y - 2.0f + bob), r * 0.65f, GOLD);
        // Thân & mũ chóp Yêu Tinh Vàng
        DrawCircle((int)c.x, (int)(c.y + bob), r * 0.78f, base);
        DrawCircleLines((int)c.x, (int)(c.y + bob), r * 0.78f, WHITE);
        Tri({ c.x - r * 0.6f, c.y - r * 0.4f + bob }, { c.x + r * 0.6f, c.y - r * 0.4f + bob }, { c.x, c.y - r * 1.45f + bob }, { 255, 235, 110, 255 });
        Eye({ c.x + face * 4.5f, c.y - 1.5f + bob }, 2.6f, 2.6f, look, { 60, 35, 10, 255 });
        const char* gTag = TextFormat("%.0fs", std::max(0.0f, e.escapeTimer));
        DrawText(gTag, (int)c.x - MeasureText(gTag, 13) / 2, (int)(c.y - r * 2.1f), 13, GOLD);
        return;
    }

    bool bomberBlink = false;
    // Bomber cháy ngòi: rung lắc, phồng lên, nháy đỏ, vòng cảnh báo bán kính nổ
    if (e.type == BOMBER && e.fusing) {
        float f = e.fuseTimer / 0.75f;
        r = e.radius * (1.0f + 0.30f * f);
        bomberBlink = sinf(f * f * 40.0f) > 0.0f;
        DrawCircle((int)c.x, (int)c.y, 120.0f, Fade({ 255, 60, 30, 255 }, bomberBlink ? 0.22f : 0.10f));
        DrawCircleLines((int)c.x, (int)c.y, 120.0f, Fade({ 255, 90, 40, 255 }, bomberBlink ? 1.0f : 0.55f));
        if (bomberBlink) base = { 255, 60, 50, 255 };
    }
    base = TintCol(base, e);

    // Additive 2.5D depth glow behind enemy
    BeginBlendMode(BLEND_ADDITIVE);
    CircleGrad(c.x, c.y, r * 1.35f, Fade(base, e.type == BOSS ? 0.32f : 0.16f), Fade(base, 0.0f));
    EndBlendMode();

    // Hào quang elite / boss
    if (e.isElite) {
        float k = 0.5f + 0.5f * sinf(t * 5.0f + c.x);
        DrawRing(c, r + 8.0f, r + 11.5f, t * 45.0f, t * 45.0f + 300.0f, 20, Fade(GOLD, 0.50f + 0.4f * k));
    }
    if (e.type == BOSS) {
        float k = 0.5f + 0.5f * sinf(t * 3.0f);
        Color aura = (e.bossKind == 1) ? (e.enraged ? Color{ 255, 90, 30, 255 } : Color{ 255, 150, 50, 255 }) : (e.bossKind == 2 ? (e.enraged ? Color{ 120, 245, 255, 255 } : Color{ 80, 205, 255, 255 }) : (e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 170, 80, 255, 255 }));
        DrawPolyLinesEx(c, 8, r * 1.38f, t * 22.0f, 2.0f, Fade(aura, 0.25f + 0.15f * k));
        DrawPolyLinesEx(c, 8, r * 1.18f, -t * 18.0f, 1.5f, Fade(aura, 0.35f + 0.15f * k));
        for (int i = 0; i < 4; ++i) {
            float a = t * (e.enraged ? 2.4f : 1.5f) + i * (PI / 2.0f);
            Vector2 o = { c.x + cosf(a) * r * 1.45f, c.y + sinf(a) * r * 1.45f * 0.78f };
            Diamond(o, 5.5f, 8.0f, { 255, 220, 120, 255 }, aura);
        }
    }

    if (e.type == CHASER) {
        // Goblin Raider: ủng chạy bước dài, giáp da, dao găm sắc, đầu góc cạnh + tai nhọn
        float step = sinf(e.bounceTimer * 1.4f) * 4.5f;
        float hop = fabsf(sinf(e.bounceTimer * 1.4f)) * 2.5f;
        Vector2 bc = { c.x, c.y - hop };
        Color skin = base;
        Color leather = TintCol({ 104, 72, 48, 255 }, e);
        // Chân & ủng
        BoxRot({ c.x - r * 0.34f, c.y + r * 0.68f + step * 0.45f }, r * 0.32f, r * 0.42f, -step * 2.5f, Shade(leather, 0.65f), ink);
        BoxRot({ c.x + r * 0.34f, c.y + r * 0.68f - step * 0.45f }, r * 0.32f, r * 0.42f, step * 2.5f, Shade(leather, 0.65f), ink);
        // Thân hình thang mặc giáp da + đai lưng
        QuadOut({ bc.x - r * 0.56f, bc.y - r * 0.18f }, { bc.x + r * 0.56f, bc.y - r * 0.18f },
                { bc.x + r * 0.40f, bc.y + r * 0.55f }, { bc.x - r * 0.40f, bc.y + r * 0.55f }, leather, ink);
        BoxRot({ bc.x, bc.y + r * 0.28f }, r * 0.88f, r * 0.18f, 0.0f, Shade(leather, 0.55f), ink, 1.2f);
        BoxRot({ bc.x, bc.y + r * 0.28f }, r * 0.22f, r * 0.20f, 0.0f, TintCol(GOLD, e), ink, 1.0f);
        // Tay cầm dao găm hướng về người chơi
        Vector2 wHand = { bc.x + face * r * 0.64f, bc.y + r * 0.06f };
        BoxRot({ bc.x - face * r * 0.58f, bc.y + r * 0.08f }, r * 0.24f, r * 0.36f, face * 20.0f, skin, ink);
        BoxRot(wHand, r * 0.26f, r * 0.32f, -face * 25.0f, skin, ink);
        Vector2 perp = { -look.y, look.x };
        Vector2 bladeTip = { wHand.x + look.x * r * 0.95f, wHand.y + look.y * r * 0.95f };
        Tri({ wHand.x + perp.x * 3.0f, wHand.y + perp.y * 3.0f }, { wHand.x - perp.x * 3.0f, wHand.y - perp.y * 3.0f }, bladeTip, TintCol({ 205, 218, 236, 255 }, e));
        DrawLineEx(wHand, bladeTip, 1.5f, WHITE);
        // Đầu Goblin góc cạnh + tai nhọn dài
        Vector2 hc = { bc.x + face * r * 0.06f, bc.y - r * 0.46f };
        Tri({ hc.x - r * 0.42f, hc.y - r * 0.10f }, { hc.x - r * 0.28f, hc.y + r * 0.18f }, { hc.x - r * 1.08f, hc.y - r * 0.42f }, Shade(skin, 0.8f));
        Tri({ hc.x + r * 0.42f, hc.y - r * 0.10f }, { hc.x + r * 0.28f, hc.y + r * 0.18f }, { hc.x + r * 1.08f, hc.y - r * 0.42f }, Shade(skin, 0.8f));
        QuadOut({ hc.x - r * 0.48f, hc.y - r * 0.36f }, { hc.x + r * 0.48f, hc.y - r * 0.36f },
                { hc.x + r * 0.38f, hc.y + r * 0.28f }, { hc.x - r * 0.38f, hc.y + r * 0.28f }, skin, ink);
        DrawLineEx({ hc.x - r * 0.42f, hc.y - r * 0.31f }, { hc.x + r * 0.36f, hc.y - r * 0.31f }, 2.0f, Fade(WHITE, 0.28f));
        Tri({ hc.x - r * 0.38f, hc.y + r * 0.28f }, { hc.x + r * 0.38f, hc.y + r * 0.28f }, { hc.x, hc.y + r * 0.46f }, Shade(skin, 0.85f));
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            Vector2 ec = { hc.x + s2 * r * 0.22f, hc.y - r * 0.04f };
            Eye(ec, r * 0.16f, r * 0.15f, look, { 255, 195, 45, 255 });
            DrawLineEx({ ec.x - s2 * r * 0.20f, ec.y - r * 0.22f }, { ec.x + s2 * r * 0.12f, ec.y - r * 0.10f }, 2.4f, Shade(skin, 0.35f));
        }
        Tri({ hc.x - r * 0.20f, hc.y + r * 0.25f }, { hc.x - r * 0.10f, hc.y + r * 0.25f }, { hc.x - r * 0.15f, hc.y + r * 0.12f }, WHITE);
        Tri({ hc.x + r * 0.20f, hc.y + r * 0.25f }, { hc.x + r * 0.10f, hc.y + r * 0.25f }, { hc.x + r * 0.15f, hc.y + r * 0.12f }, WHITE);
    } else if (e.type == SPLITTER) {
        // Prismatic Crystal Golem: thân pha lê lục giác sắc cạnh + gai vai + 2 lõi tách đôi
        float bob = sinf(e.bounceTimer * 0.9f) * 3.0f;
        Vector2 bc = { c.x, c.y - bob * 0.8f };
        Color cryDark = Shade(base, 0.58f);
        Color cryLight = Shade(base, 1.32f);
        // Chân pha lê lơ lửng
        Diamond({ c.x - r * 0.42f, c.y + r * 0.74f }, r * 0.20f, r * 0.30f, cryDark, ink);
        Diamond({ c.x + r * 0.42f, c.y + r * 0.74f }, r * 0.20f, r * 0.30f, cryDark, ink);
        // Gai tinh thể hai bên vai
        Tri({ bc.x - r * 0.65f, bc.y - r * 0.25f }, { bc.x - r * 0.40f, bc.y + r * 0.25f }, { bc.x - r * 1.18f, bc.y - r * 0.65f }, cryLight);
        Tri({ bc.x + r * 0.65f, bc.y - r * 0.25f }, { bc.x + r * 0.40f, bc.y + r * 0.25f }, { bc.x + r * 1.18f, bc.y - r * 0.65f }, cryLight);
        // Thân lục giác vát mặt sáng tối
        DrawPoly(bc, 6, r * 0.94f, 30.0f, base);
        Tri(bc, { bc.x - r * 0.82f, bc.y - r * 0.42f }, { bc.x, bc.y - r * 0.94f }, cryLight);
        Tri(bc, { bc.x + r * 0.82f, bc.y + r * 0.42f }, { bc.x, bc.y + r * 0.94f }, cryDark);
        DrawPolyLinesEx(bc, 6, r * 0.94f, 30.0f, 2.2f, ink);
        // Nếu chưa tách: 2 lõi pha lê bên trong báo hiệu sẽ phân đôi
        if (e.splitLevel == 0) {
            Diamond({ bc.x - r * 0.28f, bc.y + r * 0.26f }, r * 0.16f, r * 0.24f, { 185, 255, 255, 255 }, cryDark);
            Diamond({ bc.x + r * 0.28f, bc.y + r * 0.26f }, r * 0.16f, r * 0.24f, { 185, 255, 255, 255 }, cryDark);
        }
        // Mão đầu tinh thể + khe mắt phát sáng
        Diamond({ bc.x, bc.y - r * 0.56f }, r * 0.34f, r * 0.34f, cryLight, ink);
        BoxRot({ bc.x - r * 0.24f, bc.y - r * 0.20f }, r * 0.28f, r * 0.12f, -15.0f, { 220, 255, 255, 255 }, ink, 1.0f);
        BoxRot({ bc.x + r * 0.24f, bc.y - r * 0.20f }, r * 0.28f, r * 0.12f, 15.0f, { 220, 255, 255, 255 }, ink, 1.0f);
    } else if (e.type == RANGER) {
        // Cultist Crossbowman: áo choàng đỏ thẫm, mũ trùm nhọn, nỏ nặng ngắm theo look
        float charge = std::min(1.0f, e.attackTimer / 2.2f);
        Color robe = base;
        Color cloak = Shade(base, 0.50f);
        Color iron = TintCol({ 124, 134, 156, 255 }, e);
        // Giày & áo choàng xẻ tà
        BoxRot({ c.x - r * 0.30f, c.y + r * 0.78f }, r * 0.28f, r * 0.36f, 0.0f, TintCol({ 42, 32, 38, 255 }, e), ink);
        BoxRot({ c.x + r * 0.30f, c.y + r * 0.78f }, r * 0.28f, r * 0.36f, 0.0f, TintCol({ 42, 32, 38, 255 }, e), ink);
        QuadOut({ c.x - r * 0.52f, c.y - r * 0.20f }, { c.x + r * 0.52f, c.y - r * 0.20f },
                { c.x + r * 0.80f, c.y + r * 0.78f }, { c.x - r * 0.80f, c.y + r * 0.78f }, cloak, ink);
        QuadOut({ c.x - r * 0.42f, c.y - r * 0.18f }, { c.x + r * 0.42f, c.y - r * 0.18f },
                { c.x + r * 0.54f, c.y + r * 0.66f }, { c.x - r * 0.54f, c.y + r * 0.66f }, robe, ink);
        // Giáp vai + đai
        BoxRot({ c.x - r * 0.60f, c.y - r * 0.14f }, r * 0.38f, r * 0.28f, -18.0f, iron, ink);
        BoxRot({ c.x + r * 0.60f, c.y - r * 0.14f }, r * 0.38f, r * 0.28f, 18.0f, iron, ink);
        // Mũ trùm nhọn có sừng nhỏ + khuôn mặt tối + mắt bắn tỉa
        Vector2 hc = { c.x, c.y - r * 0.46f };
        Tri({ hc.x - r * 0.54f, hc.y + r * 0.22f }, { hc.x + r * 0.54f, hc.y + r * 0.22f }, { hc.x - face * r * 0.20f, hc.y - r * 0.88f }, cloak);
        DrawLineEx({ hc.x - r * 0.54f, hc.y + r * 0.22f }, { hc.x - face * r * 0.20f, hc.y - r * 0.88f }, 1.6f, ink);
        DrawLineEx({ hc.x + r * 0.54f, hc.y + r * 0.22f }, { hc.x - face * r * 0.20f, hc.y - r * 0.88f }, 1.6f, ink);
        QuadOut({ hc.x - r * 0.38f, hc.y - r * 0.18f }, { hc.x + r * 0.38f, hc.y - r * 0.18f },
                { hc.x + r * 0.32f, hc.y + r * 0.28f }, { hc.x - r * 0.32f, hc.y + r * 0.28f }, { 24, 14, 22, 255 }, ink);
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            Vector2 ec = { hc.x + s2 * r * 0.18f, hc.y + r * 0.04f };
            Eye(ec, r * 0.15f, r * 0.15f, look, { 255, 200, 50, 255 });
        }
        // Nỏ nặng ngắm theo hướng look
        Vector2 perp = { -look.y, look.x };
        Vector2 xRoot = { c.x + look.x * r * 0.35f, c.y + look.y * r * 0.35f };
        Vector2 xTip = { c.x + look.x * r * 1.18f, c.y + look.y * r * 1.18f };
        DrawLineEx(xRoot, xTip, 4.5f, TintCol({ 115, 72, 44, 255 }, e));
        Vector2 pL = Vector2Add(xTip, Vector2Add(Vector2Scale(look, -r * 0.25f), Vector2Scale(perp, r * 0.65f)));
        Vector2 pR = Vector2Add(xTip, Vector2Add(Vector2Scale(look, -r * 0.25f), Vector2Scale(perp, -r * 0.65f)));
        DrawLineEx(xTip, pL, 3.2f, iron);
        DrawLineEx(xTip, pR, 3.2f, iron);
        Vector2 nock = Vector2Add(xTip, Vector2Scale(look, -r * (0.16f + 0.38f * charge)));
        DrawLineEx(pL, nock, 1.4f, WHITE);
        DrawLineEx(pR, nock, 1.4f, WHITE);
        Color boltCol = charge > 0.6f ? Color{ 255, 195, 65, 255 } : Color{ 255, 95, 95, 255 };
        DrawLineEx(nock, Vector2Add(xTip, Vector2Scale(look, 4.0f)), 2.8f, boltCol);
        if (charge > 0.45f) Diamond(Vector2Add(xTip, Vector2Scale(look, 5.0f)), 4.0f * charge + 1.5f, 4.0f * charge + 1.5f, YELLOW, ORANGE);
    } else if (e.type == BRUTE) {
        // Iron Juggernaut / Minotaur Knight (hoặc Golden Warden): giáp tấm nặng, khiên lớn, búa/rìu chiến
        Color armor = base;
        Color steel = TintCol(e.isWarden ? Color{ 242, 198, 86, 255 } : Color{ 154, 164, 188, 255 }, e);
        Color horn = TintCol(e.isWarden ? Color{ 255, 240, 190, 255 } : Color{ 232, 226, 210, 255 }, e);
        float hs = e.isWarden ? 1.32f : 1.0f;
        float walk = sinf(e.bounceTimer * 0.8f) * 3.0f;
        // Chân giáp sắt to bản
        BoxRot({ c.x - r * 0.40f, c.y + r * 0.72f + walk * 0.4f }, r * 0.38f, r * 0.44f, 0.0f, Shade(steel, 0.62f), ink);
        BoxRot({ c.x + r * 0.40f, c.y + r * 0.72f - walk * 0.4f }, r * 0.38f, r * 0.44f, 0.0f, Shade(steel, 0.62f), ink);
        // Thân giáp hình thang + tà giáp hông
        QuadOut({ c.x - r * 0.76f, c.y - r * 0.32f }, { c.x + r * 0.76f, c.y - r * 0.32f },
                { c.x + r * 0.56f, c.y + r * 0.54f }, { c.x - r * 0.56f, c.y + r * 0.54f }, armor, ink, 2.0f);
        QuadOut({ c.x - r * 0.50f, c.y - r * 0.24f }, { c.x + r * 0.50f, c.y - r * 0.24f },
                { c.x + r * 0.36f, c.y + r * 0.28f }, { c.x - r * 0.36f, c.y + r * 0.28f }, steel, ink, 1.5f);
        DrawLineEx({ c.x - r * 0.44f, c.y - r * 0.20f }, { c.x + r * 0.44f, c.y - r * 0.20f }, 2.2f, Fade(WHITE, 0.34f));
        BoxRot({ c.x, c.y + r * 0.42f }, r * 1.05f, r * 0.22f, 0.0f, Shade(steel, 0.58f), ink);
        // Cầu vai có gai
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ c.x + s2 * r * 0.82f, c.y - r * 0.22f }, r * 0.48f, r * 0.42f, s2 * 18.0f, steel, ink);
            Tri({ c.x + s2 * r * 0.74f, c.y - r * 0.40f }, { c.x + s2 * r * 0.98f, c.y - r * 0.24f }, { c.x + s2 * r * 1.18f, c.y - r * 0.68f }, horn);
        }
        // Mũ giáp sừng bò + khe mắt rực đỏ
        Vector2 hc = { c.x, c.y - r * 0.54f };
        Tri({ hc.x - r * 0.48f, hc.y - r * 0.08f }, { hc.x - r * 0.30f, hc.y - r * 0.34f }, { hc.x - r * 1.02f * hs, hc.y - r * 0.90f * hs }, horn);
        Tri({ hc.x + r * 0.48f, hc.y - r * 0.08f }, { hc.x + r * 0.30f, hc.y - r * 0.34f }, { hc.x + r * 1.02f * hs, hc.y - r * 0.90f * hs }, horn);
        QuadOut({ hc.x - r * 0.48f, hc.y - r * 0.36f }, { hc.x + r * 0.48f, hc.y - r * 0.36f },
                { hc.x + r * 0.40f, hc.y + r * 0.30f }, { hc.x - r * 0.40f, hc.y + r * 0.30f }, steel, ink, 1.8f);
        BoxRot({ hc.x, hc.y + r * 0.04f }, r * 0.68f, r * 0.20f, 0.0f, { 20, 14, 24, 255 }, ink, 1.0f);
        DrawRectangle((int)(hc.x - r * 0.24f), (int)(hc.y - r * 0.02f), (int)(r * 0.18f), (int)(r * 0.10f), { 255, 90, 60, 255 });
        DrawRectangle((int)(hc.x + r * 0.06f), (int)(hc.y - r * 0.02f), (int)(r * 0.18f), (int)(r * 0.10f), { 255, 90, 60, 255 });
        // Tay trái cầm Khiên Tháp, tay phải vác Chùy/Rìu
        Vector2 shC = { c.x - face * r * 0.84f, c.y + r * 0.16f };
        QuadOut({ shC.x - r * 0.26f, shC.y - r * 0.46f }, { shC.x + r * 0.26f, shC.y - r * 0.46f },
                { shC.x + r * 0.20f, shC.y + r * 0.44f }, { shC.x - r * 0.20f, shC.y + r * 0.44f }, Shade(steel, 0.72f), ink, 1.8f);
        Diamond(shC, r * 0.12f, r * 0.18f, e.isWarden ? WHITE : GOLD, ink);
        Vector2 wHand = { c.x + face * r * 0.86f, c.y + r * 0.10f };
        float slamLift = (e.attackTimer > 2.8f) ? -r * 0.35f : 0.0f;
        DrawLineEx({ wHand.x, wHand.y + r * 0.35f }, { wHand.x + face * r * 0.10f, wHand.y - r * 0.65f + slamLift }, 4.2f, TintCol({ 110, 74, 42, 255 }, e));
        BoxRot({ wHand.x + face * r * 0.10f, wHand.y - r * 0.62f + slamLift }, r * 0.56f, r * 0.32f, face * 12.0f, steel, ink, 1.8f);
        // Báo hiệu đập đất
        if (e.attackTimer > 2.8f && Vector2Distance(c, player.pos) < 150.0f) {
            float k = (e.attackTimer - 2.8f) / 0.7f;
            DrawCircleLines((int)c.x, (int)c.y, 140.0f * std::min(1.0f, k), Fade({ 200, 120, 255, 255 }, 0.8f));
        }
    } else if (e.type == BOSS && e.bossKind == 1) {
        // THE MOLTEN COLOSSUS: Titan dung nham thân đá góc cạnh, nắm đấm khổng lồ, khe nứt lửa
        if (e.chargeState == 1) {
            float k = 1.0f - e.chargeTimer / 0.9f;
            DrawCircle((int)c.x, (int)c.y, 190.0f, Fade({ 255, 70, 30, 255 }, 0.06f + 0.12f * k));
            DrawCircleLines((int)c.x, (int)c.y, 190.0f * std::max(0.1f, k), Fade({ 255, 160, 60, 255 }, 0.9f));
            DrawCircleLines((int)c.x, (int)c.y, 190.0f, Fade({ 255, 90, 40, 255 }, 0.8f));
        }
        if (e.chargeState == 3) {
            float k = 0.5f + 0.5f * sinf(t * 30.0f);
            Vector2 end = { c.x + e.chargeDir.x * 560.0f, c.y + e.chargeDir.y * 560.0f };
            DrawLineEx(c, end, r * 1.6f, Fade({ 255, 60, 40, 255 }, 0.10f + 0.10f * k));
            DrawLineEx(c, end, 4.0f, Fade({ 255, 120, 60, 255 }, 0.6f + 0.4f * k));
        }
        float sq = (e.chargeState == 4) ? 1.10f : 1.0f;
        float smashY = (e.chargeState == 4) ? r * 0.25f : sinf(t * 3.0f) * 3.5f;
        Color rockDark = Shade(base, 0.48f);
        Color rockMid = base;
        Color rockHi = Shade(base, 1.25f);
        Color lava = { 255, 185, 55, 255 };
        // Hai cột chân đá núi lửa
        BoxRot({ c.x - r * 0.42f, c.y + r * 0.72f }, r * 0.42f, r * 0.50f, 0.0f, rockDark, ink, 2.2f);
        BoxRot({ c.x + r * 0.42f, c.y + r * 0.72f }, r * 0.42f, r * 0.50f, 0.0f, rockDark, ink, 2.2f);
        // Gai đá sau lưng
        for (int i = -2; i <= 2; ++i) {
            float bx2 = c.x + i * r * 0.30f;
            Tri({ bx2 - r * 0.16f, c.y - r * 0.42f }, { bx2 + r * 0.16f, c.y - r * 0.42f },
                { bx2 + i * 4.0f, c.y - r * (1.06f + 0.12f * (2 - abs(i))) }, rockDark);
        }
        // Thân đá V-shape khổng lồ
        QuadOut({ c.x - r * 0.92f * sq, c.y - r * 0.45f }, { c.x + r * 0.92f * sq, c.y - r * 0.45f },
                { c.x + r * 0.56f * sq, c.y + r * 0.58f }, { c.x - r * 0.56f * sq, c.y + r * 0.58f }, rockMid, ink, 2.5f);
        // Vết nứt dung nham phát sáng + lõi lò ngực (additive pulse)
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(c.x, c.y + r * 0.14f, r * 0.68f, Fade(lava, 0.42f + 0.16f * sinf(t * 5.0f)), Fade(lava, 0.0f));
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            CircleGrad(c.x + s2 * r * 1.10f, c.y + r * 0.18f + smashY, r * 0.42f, Fade(ORANGE, 0.32f), Fade(ORANGE, 0.0f));
        }
        EndBlendMode();
        for (int i = -1; i <= 1; ++i) {
            float gl = 0.72f + 0.28f * sinf(t * 4.0f + i);
            DrawLineEx({ c.x + i * r * 0.34f, c.y - r * 0.15f }, { c.x + i * r * 0.26f, c.y + r * 0.50f }, 3.5f, Fade(lava, gl));
        }
        Diamond({ c.x, c.y + r * 0.14f }, r * 0.24f, r * 0.30f, lava, { 255, 80, 20, 255 });
        // Hai khối vai nham thạch + hai nắm đấm đá khổng lồ
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ c.x + s2 * r * 0.92f, c.y - r * 0.34f }, r * 0.52f, r * 0.44f, s2 * 22.0f, rockHi, ink, 2.2f);
            BoxRot({ c.x + s2 * r * 1.10f, c.y + r * 0.18f + smashY }, r * 0.46f, r * 0.54f, s2 * 10.0f, rockDark, ink, 2.2f);
            DrawLineEx({ c.x + s2 * r * 0.98f, c.y + r * 0.18f + smashY }, { c.x + s2 * r * 1.22f, c.y + r * 0.18f + smashY }, 3.0f, lava);
        }
        // Đầu Titan góc cạnh + mắt khe dung nham + 5 ngọn lửa vương miện
        Vector2 hc = { c.x, c.y - r * 0.56f };
        QuadOut({ hc.x - r * 0.44f, hc.y - r * 0.34f }, { hc.x + r * 0.44f, hc.y - r * 0.34f },
                { hc.x + r * 0.32f, hc.y + r * 0.32f }, { hc.x - r * 0.32f, hc.y + r * 0.32f }, rockDark, ink, 2.2f);
        Color eyeC = e.enraged ? Color{ 255, 245, 130, 255 } : lava;
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ hc.x + s2 * r * 0.19f, hc.y - r * 0.04f }, r * 0.22f, r * 0.10f, -s2 * 14.0f, eyeC, ink, 1.0f);
        }
        for (int i = -2; i <= 2; ++i) {
            float fl = 0.7f + 0.3f * sinf(t * 14.0f + i * 1.7f);
            float fx = hc.x + i * r * 0.16f, fy = hc.y - r * 0.34f;
            float fh = r * (0.44f - 0.04f * abs(i)) * fl + 6.0f;
            Tri({ fx - 5.5f, fy }, { fx + 5.5f, fy }, { fx, fy - fh }, { 255, 130, 35, 255 });
            Tri({ fx - 2.6f, fy }, { fx + 2.6f, fy }, { fx, fy - fh * 0.62f }, { 255, 240, 140, 255 });
        }
    } else if (e.type == BOSS && e.bossKind == 2) {
        // BĂNG LONG ĐẾ QUÂN (Frost Dragon Archon): Cánh rồng băng vỗ 3 khớp, sừng pha lê, vòng tinh thể tuyết
        float flap = sinf(t * 3.2f) * r * 0.28f;
        float hover = sinf(t * 2.4f) * 3.5f;
        Vector2 bc = { c.x, c.y + hover };
        Color iceDark = Shade(base, 0.52f);
        Color iceHi = { 165, 240, 255, 255 };
        Color iceCore = { 235, 252, 255, 255 };
        // Đôi cánh rồng băng khổng lồ vỗ động
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            Vector2 root = { bc.x + s2 * r * 0.42f, bc.y - r * 0.18f };
            Vector2 mid = { bc.x + s2 * r * 1.15f, bc.y - r * 0.85f + flap * 0.6f };
            Vector2 tip = { bc.x + s2 * r * 1.72f, bc.y - r * 0.35f + flap };
            Vector2 low1 = { bc.x + s2 * r * 1.32f, bc.y + r * 0.38f + flap * 0.5f };
            Vector2 low2 = { bc.x + s2 * r * 0.72f, bc.y + r * 0.42f };
            Tri(root, mid, low2, Fade(base, 0.85f));
            Tri(mid, tip, low1, Fade(iceDark, 0.88f));
            Tri(root, mid, low1, Fade(iceHi, 0.42f));
            DrawLineEx(root, mid, 3.8f, iceHi);
            DrawLineEx(mid, tip, 3.2f, WHITE);
        }
        // Đuôi rồng băng cong uốn lượn
        for (int ti = 0; ti < 3; ++ti) {
            float tx = bc.x - face * r * (0.35f + ti * 0.32f);
            float ty = bc.y + r * (0.45f + ti * 0.22f) + sinf(t * 4.0f + ti) * 4.0f;
            Diamond({ tx, ty }, r * (0.28f - ti * 0.05f), r * (0.20f - ti * 0.03f), iceDark, iceHi);
        }
        // Thân giáp vảy rồng + lõi băng tim
        QuadOut({ bc.x - r * 0.68f, bc.y - r * 0.38f }, { bc.x + r * 0.68f, bc.y - r * 0.38f },
                { bc.x + r * 0.44f, bc.y + r * 0.54f }, { bc.x - r * 0.44f, bc.y + r * 0.54f }, base, ink, 2.2f);
        QuadOut({ bc.x - r * 0.42f, bc.y - r * 0.26f }, { bc.x + r * 0.42f, bc.y - r * 0.26f },
                { bc.x + r * 0.26f, bc.y + r * 0.38f }, { bc.x - r * 0.26f, bc.y + r * 0.38f }, iceDark, iceHi, 1.4f);
        Diamond({ bc.x, bc.y + r * 0.05f }, r * 0.22f, r * 0.32f, iceCore, iceHi);
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ bc.x + s2 * r * 0.72f, bc.y - r * 0.30f }, r * 0.44f, r * 0.36f, s2 * 22.0f, iceHi, ink, 1.8f);
        }
        // Đầu Băng Long + 2 sừng pha lê + vương miện băng
        Vector2 hc = { bc.x, bc.y - r * 0.62f };
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            Tri({ hc.x + s2 * r * 0.22f, hc.y - r * 0.18f }, { hc.x + s2 * r * 0.42f, hc.y - r * 0.08f }, { hc.x + s2 * r * 0.96f, hc.y - r * 0.92f }, iceCore);
            DrawLineEx({ hc.x + s2 * r * 0.32f, hc.y - r * 0.14f }, { hc.x + s2 * r * 0.96f, hc.y - r * 0.92f }, 2.0f, WHITE);
        }
        QuadOut({ hc.x - r * 0.42f, hc.y - r * 0.32f }, { hc.x + r * 0.42f, hc.y - r * 0.32f },
                { hc.x + r * 0.30f, hc.y + r * 0.28f }, { hc.x - r * 0.30f, hc.y + r * 0.28f }, iceDark, ink, 2.0f);
        Diamond({ hc.x, hc.y - r * 0.45f }, r * 0.18f, r * 0.28f, WHITE, iceHi);
        Color eyeC = e.enraged ? Color{ 255, 110, 130, 255 } : iceCore;
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ hc.x + s2 * r * 0.18f, hc.y + r * 0.02f }, r * 0.20f, r * 0.09f, -s2 * 16.0f, eyeC, ink, 1.0f);
        }
        // 6 tinh thể tuyết xoay quanh
        for (int bi = 0; bi < 6; ++bi) {
            float a = t * 2.2f + bi * (PI / 3.0f);
            Diamond({ bc.x + cosf(a) * r * 1.15f, bc.y + sinf(a) * r * 0.82f }, 4.5f, 6.5f, WHITE, iceHi);
        }
    } else if (e.type == BOSS) {
        // THE VOID SOVEREIGN: Hoàng đế Hư Không - 3 cặp cánh kiếm nổi, hào quang sau đầu, áo choàng, vương miện
        float hover = sinf(t * 2.5f) * 4.0f;
        Vector2 bc = { c.x, c.y + hover * 0.5f };
        Color darkVoid = Shade(base, 0.42f);
        Color accent = e.enraged ? Color{ 255, 80, 90, 255 } : Color{ 195, 110, 255, 255 };
        // Cánh kiếm obsidian nổi 3 tầng phía sau 2 bên
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            for (int w = 0; w < 3; ++w) {
                float wy = bc.y - r * 0.38f + w * r * 0.26f + sinf(t * 3.0f + w) * 3.0f;
                Tri({ bc.x + s2 * r * 0.40f, wy },
                    { bc.x + s2 * r * (1.36f - w * 0.16f), wy - r * 0.52f },
                    { bc.x + s2 * r * (0.98f - w * 0.10f), wy + r * 0.18f }, darkVoid);
                DrawLineEx({ bc.x + s2 * r * 0.40f, wy }, { bc.x + s2 * r * (1.36f - w * 0.16f), wy - r * 0.52f }, 2.2f, accent);
            }
        }
        // Áo choàng hoàng gia rộng
        QuadOut({ bc.x - r * 0.56f, bc.y - r * 0.30f }, { bc.x + r * 0.56f, bc.y - r * 0.30f },
                { bc.x + r * 0.88f, bc.y + r * 0.96f }, { bc.x - r * 0.88f, bc.y + r * 0.96f }, darkVoid, ink, 2.2f);
        // Thân giáp Hư Không + lõi pha lê ngực
        QuadOut({ bc.x - r * 0.58f, bc.y - r * 0.38f }, { bc.x + r * 0.58f, bc.y - r * 0.38f },
                { bc.x + r * 0.40f, bc.y + r * 0.54f }, { bc.x - r * 0.40f, bc.y + r * 0.54f }, base, ink, 2.2f);
        DrawLineEx({ bc.x - r * 0.50f, bc.y - r * 0.33f }, { bc.x + r * 0.50f, bc.y - r * 0.33f }, 2.2f, Fade(WHITE, 0.30f));
        Diamond({ bc.x, bc.y + r * 0.06f }, r * 0.20f, r * 0.28f, accent, GOLD);
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ bc.x + s2 * r * 0.68f, bc.y - r * 0.28f }, r * 0.44f, r * 0.34f, s2 * 22.0f, Shade(base, 1.25f), GOLD, 1.8f);
        }
        // Hào quang xoay & đầu giáp sừng + vương miện 3 ngọn
        Vector2 hc = { bc.x, bc.y - r * 0.64f };
        DrawRing({ hc.x, hc.y - 4.0f }, r * 0.55f, r * 0.62f, t * 50.0f, t * 50.0f + 280.0f, 20, accent);
        QuadOut({ hc.x - r * 0.36f, hc.y - r * 0.30f }, { hc.x + r * 0.36f, hc.y - r * 0.30f },
                { hc.x + r * 0.26f, hc.y + r * 0.30f }, { hc.x - r * 0.26f, hc.y + r * 0.30f }, darkVoid, ink, 2.0f);
        Color eyeC = e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 255, 220, 90, 255 };
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            BoxRot({ hc.x + s2 * r * 0.16f, hc.y + r * 0.02f }, r * 0.18f, r * 0.10f, -s2 * 16.0f, eyeC, ink, 1.0f);
        }
        float cx = hc.x, cy = hc.y - r * 0.32f;
        DrawRectangle((int)cx - 16, (int)cy - 7, 32, 8, GOLD);
        Tri({ cx - 16, cy - 7 }, { cx - 10, cy - 7 }, { cx - 17, cy - 22 }, GOLD);
        Tri({ cx - 7, cy - 7 }, { cx + 7, cy - 7 }, { cx, cy - 28 }, GOLD);
        Tri({ cx + 10, cy - 7 }, { cx + 16, cy - 7 }, { cx + 17, cy - 22 }, GOLD);
        Diamond({ cx, cy - 3 }, 3.5f, 3.5f, RED, ink);
        // Đại kiếm Hư Không bên tay phải
        Vector2 swBase = { bc.x + face * r * 0.78f, bc.y + r * 0.22f };
        Vector2 swTip = { swBase.x + face * r * 0.32f, swBase.y - r * 1.15f };
        DrawLineEx(swBase, swTip, 8.0f, ink);
        DrawLineEx(swBase, swTip, 5.0f, accent);
        DrawLineEx(swBase, swTip, 1.8f, WHITE);
        DrawLineEx({ swBase.x - 10.0f, swBase.y + 2.0f }, { swBase.x + 10.0f, swBase.y - 2.0f }, 4.5f, GOLD);
    } else if (e.type == CHARGER) {
        // Armored War-Boar: thú chiến bọc giáp sắt định hướng theo fwd, 4 vó, 2 ngà lớn
        if (e.chargeState == 1) {
            float k = 0.5f + 0.5f * sinf(t * 30.0f);
            Vector2 end = { c.x + e.chargeDir.x * 460.0f, c.y + e.chargeDir.y * 460.0f };
            DrawLineEx(c, end, r * 1.9f, Fade({ 255, 60, 40, 255 }, 0.10f + 0.10f * k));
            DrawLineEx(c, end, 3.0f, Fade({ 255, 90, 60, 255 }, 0.6f + 0.4f * k));
            OutlinedText("!", (int)c.x - 5, (int)(c.y - r - 34.0f), 30, { 255, 220, 60, 255 }, { 80, 10, 10, 255 });
        }
        Vector2 fwd = (e.chargeState >= 1 && e.chargeState <= 2) ? e.chargeDir : look;
        Vector2 perp = { -fwd.y, fwd.x };
        float angDeg = atan2f(fwd.y, fwd.x) * RAD2DEG;
        if (e.chargeState == 2) {
            for (int i = 1; i <= 3; ++i) {
                Vector2 ghostP = Vector2Subtract(c, Vector2Scale(fwd, i * r * 0.68f));
                Diamond(ghostP, r * (0.75f - i * 0.15f), r * (0.55f - i * 0.12f), Fade(base, 0.24f - i * 0.05f), Fade(GOLD, 0.18f));
            }
        }
        Vector2 bc = { c.x + (e.chargeState == 1 ? sinf(t * 60.0f) * 2.0f : 0.0f), c.y };
        Color hide = base;
        Color plate = TintCol({ 138, 146, 168, 255 }, e);
        float gallop = sinf(t * (e.chargeState == 2 ? 26.0f : 10.0f) + c.x * 0.1f) * 5.0f;
        // 4 chân vó đang phi
        for (int f2 = -1; f2 <= 1; f2 += 2) {
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 legP = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, f2 * r * 0.42f + f2 * gallop * 0.5f), Vector2Scale(perp, s2 * r * 0.62f)));
                BoxRot(legP, r * 0.34f, r * 0.24f, angDeg, Shade(hide, 0.55f), ink);
            }
        }
        // Thân hình bọc giáp hướng về fwd
        Vector2 rearL = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, -r * 0.82f), Vector2Scale(perp, -r * 0.56f)));
        Vector2 rearR = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, -r * 0.82f), Vector2Scale(perp,  r * 0.56f)));
        Vector2 midL  = Vector2Add(bc, Vector2Add(Vector2Scale(fwd,  r * 0.35f), Vector2Scale(perp, -r * 0.72f)));
        Vector2 midR  = Vector2Add(bc, Vector2Add(Vector2Scale(fwd,  r * 0.35f), Vector2Scale(perp,  r * 0.72f)));
        QuadOut(rearL, midL, midR, rearR, hide, ink, 1.9f);
        // Tấm giáp lưng sắt
        Vector2 armA = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, -r * 0.42f), Vector2Scale(perp, -r * 0.54f)));
        Vector2 armB = Vector2Add(bc, Vector2Add(Vector2Scale(fwd,  r * 0.30f), Vector2Scale(perp, -r * 0.62f)));
        Vector2 armC = Vector2Add(bc, Vector2Add(Vector2Scale(fwd,  r * 0.30f), Vector2Scale(perp,  r * 0.62f)));
        Vector2 armD = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, -r * 0.42f), Vector2Scale(perp,  r * 0.54f)));
        QuadOut(armA, armB, armC, armD, plate, ink, 1.5f);
        // Sống lưng gai
        DrawLineEx(Vector2Subtract(bc, Vector2Scale(fwd, r * 0.65f)), Vector2Add(bc, Vector2Scale(fwd, r * 0.35f)), 4.0f, Shade(plate, 0.5f));
        // Đầu nêm bọc sắt + ngà voi cong
        Vector2 snL = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, r * 0.92f), Vector2Scale(perp, -r * 0.28f)));
        Vector2 snR = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, r * 0.92f), Vector2Scale(perp,  r * 0.28f)));
        QuadOut(midL, snL, snR, midR, Shade(plate, 0.85f), ink, 1.8f);
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            Vector2 tBase = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, r * 0.62f), Vector2Scale(perp, s2 * r * 0.44f)));
            Vector2 tMid  = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, r * 1.05f), Vector2Scale(perp, s2 * r * 0.62f)));
            Vector2 tTip  = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, r * 1.40f), Vector2Scale(perp, s2 * r * 0.30f)));
            Tri(tBase, Vector2Add(tBase, Vector2Scale(perp, -s2 * 4.5f)), tMid, { 242, 234, 212, 255 });
            Tri(tBase, tMid, tTip, { 255, 248, 230, 255 });
            Vector2 eyeP = Vector2Add(bc, Vector2Add(Vector2Scale(fwd, r * 0.52f), Vector2Scale(perp, s2 * r * 0.32f)));
            Diamond(eyeP, 3.8f, 3.0f, e.chargeState == 3 ? YELLOW : Color{ 255, 70, 45, 255 }, ink);
        }
        // Choáng: sao 4 cánh quay quanh đầu
        if (e.chargeState == 3) {
            for (int i = 0; i < 3; ++i) {
                float a3 = t * 6.0f + i * 2.094f;
                Diamond({ c.x + cosf(a3) * r * 0.85f, c.y - r - 8.0f + sinf(a3) * 5.0f }, 4.5f, 4.5f, { 255, 230, 80, 255 }, ink);
            }
        }
    } else if (e.type == SUMMONER) {
        // Void Lich / Necromancer: áo choàng rách tầng, xương sườn ngực, hộp sọ + quyền trượng sọ
        if (e.chargeState == 1) {
            float k = 1.0f - e.chargeTimer;
            DrawPolyLinesEx(c, 6, 42.0f + 28.0f * k, t * 40.0f, 2.0f, Fade({ 190, 130, 255, 255 }, 0.9f));
            DrawCircleLines((int)c.x, (int)c.y, 22.0f + 16.0f * k, Fade({ 190, 130, 255, 255 }, 0.7f));
            for (int i = 0; i < 6; ++i) {
                float a = t * 3.0f + i * (PI / 3.0f);
                DrawLineEx({ c.x + cosf(a) * 22.0f, c.y + sinf(a) * 22.0f }, { c.x + cosf(a) * (40.0f + 30.0f * k), c.y + sinf(a) * (40.0f + 30.0f * k) }, 2.0f, Fade({ 210, 160, 255, 255 }, 0.7f));
            }
        }
        float bob = sinf(t * 2.5f + c.x) * 3.0f;
        Color robe = base;
        Color robeDark = Shade(robe, 0.52f);
        Color bone = TintCol({ 228, 222, 204, 255 }, e);
        // Áo choàng đa giác + tà rách
        QuadOut({ c.x - r * 0.55f, c.y - r * 0.25f + bob }, { c.x + r * 0.55f, c.y - r * 0.25f + bob },
                { c.x + r * 0.92f, c.y + r * 0.88f + bob }, { c.x - r * 0.92f, c.y + r * 0.88f + bob }, robeDark, ink, 1.8f);
        QuadOut({ c.x - r * 0.42f, c.y - r * 0.22f + bob }, { c.x + r * 0.42f, c.y - r * 0.22f + bob },
                { c.x + r * 0.62f, c.y + r * 0.82f + bob }, { c.x - r * 0.62f, c.y + r * 0.82f + bob }, robe, ink, 1.4f);
        // Xương sườn trước ngực + cổ áo gai vàng sẫm
        DrawLineEx({ c.x, c.y - r * 0.16f + bob }, { c.x, c.y + r * 0.34f + bob }, 3.0f, bone);
        for (int rb = 0; rb < 3; ++rb) {
            float ry = c.y - r * 0.08f + rb * r * 0.15f + bob;
            DrawLineEx({ c.x - r * 0.24f, ry }, { c.x + r * 0.24f, ry }, 2.4f, bone);
        }
        Tri({ c.x - r * 0.48f, c.y - r * 0.20f + bob }, { c.x - r * 0.22f, c.y - r * 0.30f + bob }, { c.x - r * 0.72f, c.y - r * 0.62f + bob }, TintCol({ 195, 160, 75, 255 }, e));
        Tri({ c.x + r * 0.48f, c.y - r * 0.20f + bob }, { c.x + r * 0.22f, c.y - r * 0.30f + bob }, { c.x + r * 0.72f, c.y - r * 0.62f + bob }, TintCol({ 195, 160, 75, 255 }, e));
        // Mũ trùm + đầu lâu sọ
        Vector2 hc = { c.x, c.y - r * 0.52f + bob };
        Tri({ hc.x - r * 0.56f, hc.y + r * 0.26f }, { hc.x + r * 0.56f, hc.y + r * 0.26f }, { hc.x, hc.y - r * 0.72f }, robeDark);
        QuadOut({ hc.x - r * 0.28f, hc.y - r * 0.20f }, { hc.x + r * 0.28f, hc.y - r * 0.20f },
                { hc.x + r * 0.20f, hc.y + r * 0.26f }, { hc.x - r * 0.20f, hc.y + r * 0.26f }, bone, ink, 1.5f);
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            Diamond({ hc.x + s2 * r * 0.12f, hc.y - r * 0.02f }, 3.8f, 3.8f, { 255, 110, 255, 255 }, { 30, 12, 42, 255 });
        }
        // Quyền trượng xương bên cạnh
        Vector2 stBot = { c.x + face * r * 0.74f, c.y + r * 0.75f + bob };
        Vector2 stTop = { c.x + face * r * 0.74f, c.y - r * 0.68f + bob };
        DrawLineEx(stBot, stTop, 3.4f, Shade(bone, 0.75f));
        Diamond(stTop, 6.0f, 8.0f, { 215, 155, 255, 255 }, ink);
        // 3 tinh thể linh hồn quay quanh
        for (int i = 0; i < 3; ++i) {
            float a = t * 2.2f + i * (2.0f * PI / 3.0f);
            Vector2 o = { c.x + cosf(a) * r * 1.35f, c.y + sinf(a) * r * 0.6f + bob - r * 0.1f };
            Diamond(o, 5.0f, 7.5f, { 215, 165, 255, 255 }, { 90, 40, 150, 255 });
        }
    } else if (e.type == BOMBER) {
        // Goblin Sapper vác Thùng thuốc nổ TNT: chân chạy gắt, đầu đội kính bảo hộ, thùng TNT sọc đỏ + dây cháy xẹt tia lửa
        float sprint = sinf(t * (e.fusing ? 34.0f : 18.0f) + c.x) * 5.0f;
        Color skin = TintCol(bomberBlink ? Color{ 255, 90, 75, 255 } : Color{ 130, 195, 92, 255 }, e);
        Color wood = TintCol(bomberBlink ? Color{ 230, 75, 65, 255 } : Color{ 156, 98, 52, 255 }, e);
        Color band = TintCol({ 68, 72, 86, 255 }, e);
        Color haz  = TintCol({ 228, 56, 48, 255 }, e);
        // Hai chân chạy nhanh
        BoxRot({ c.x - r * 0.34f, c.y + r * 0.76f + sprint * 0.45f }, r * 0.30f, r * 0.40f, -sprint * 3.0f, { 68, 48, 36, 255 }, ink);
        BoxRot({ c.x + r * 0.34f, c.y + r * 0.76f - sprint * 0.45f }, r * 0.30f, r * 0.40f, sprint * 3.0f, { 68, 48, 36, 255 }, ink);
        // Đầu Goblin nhô phía trên thùng thuốc nổ + kính bảo hộ
        Vector2 hc = { c.x, c.y - r * 0.56f };
        Tri({ hc.x - r * 0.38f, hc.y }, { hc.x - r * 0.20f, hc.y + r * 0.2f }, { hc.x - r * 0.96f, hc.y - r * 0.25f }, Shade(skin, 0.8f));
        Tri({ hc.x + r * 0.38f, hc.y }, { hc.x + r * 0.20f, hc.y + r * 0.2f }, { hc.x + r * 0.96f, hc.y - r * 0.25f }, Shade(skin, 0.8f));
        QuadOut({ hc.x - r * 0.42f, hc.y - r * 0.30f }, { hc.x + r * 0.42f, hc.y - r * 0.30f },
                { hc.x + r * 0.34f, hc.y + r * 0.22f }, { hc.x - r * 0.34f, hc.y + r * 0.22f }, skin, ink);
        BoxRot({ hc.x - r * 0.18f, hc.y - r * 0.04f }, r * 0.28f, r * 0.24f, 0.0f, { 255, 220, 90, 255 }, band, 2.0f);
        BoxRot({ hc.x + r * 0.18f, hc.y - r * 0.04f }, r * 0.28f, r * 0.24f, 0.0f, { 255, 220, 90, 255 }, band, 2.0f);
        // Thùng thuốc nổ TNT vác trước ngực
        float wob = sinf(t * (e.fusing ? 42.0f : 12.0f)) * 4.0f;
        Vector2 bc = { c.x, c.y + r * 0.12f };
        BoxRot(bc, r * 1.42f, r * 1.18f, wob, wood, ink, 2.0f);
        BoxRot(bc, r * 1.44f, r * 0.40f, wob, haz, ink, 1.4f);
        BoxRot({ bc.x, bc.y - r * 0.40f }, r * 1.44f, r * 0.16f, wob, band, ink, 1.2f);
        BoxRot({ bc.x, bc.y + r * 0.40f }, r * 1.44f, r * 0.16f, wob, band, ink, 1.2f);
        DrawText("TNT", (int)(bc.x - MeasureText("TNT", 10) / 2), (int)(bc.y - 5), 10, WHITE);
        // Ngòi nổ + tia lửa hình sao nhấp nháy
        Vector2 top = { bc.x + r * 0.26f, bc.y - r * 0.60f };
        Vector2 tip = { top.x + 8.0f, top.y - 12.0f };
        DrawLineEx(top, tip, 3.2f, { 110, 70, 40, 255 });
        float tw = 0.6f + 0.4f * sinf(t * 25.0f);
        Diamond(tip, 6.0f * tw + 2.0f, 6.0f * tw + 2.0f, { 255, 195, 50, 255 }, { 255, 90, 20, 255 });
        Diamond(tip, 2.8f * tw + 1.0f, 2.8f * tw + 1.0f, WHITE, YELLOW);
    }

    // Viền trạng thái
    if (e.slowTimer > 0) {
        DrawPolyLinesEx(c, 6, r + 5.0f, t * 25.0f, 2.0f, { 120, 210, 255, 220 });
    }
    if (e.burnTimer > 0) {
        float fl = 0.6f + 0.4f * sinf(t * 30.0f + c.x);
        DrawPolyLinesEx(c, 5, r + 7.0f, -t * 50.0f, 2.0f, Fade({ 255, 130, 30, 255 }, fl));
        Tri({ c.x - r * 0.55f, c.y - r }, { c.x - r * 0.35f, c.y - r }, { c.x - r * 0.45f, c.y - r - 8.0f * fl }, { 255, 185, 50, 255 });
    }
    if (e.stunTimer > 0.0f && !(e.type == CHARGER && e.chargeState == 3)) {
        Color starCol = (e.chillTimer > 0.0f) ? Color{ 150, 235, 255, 255 } : Color{ 255, 230, 80, 255 };
        for (int i = 0; i < 3; ++i) {
            float a3 = t * 6.5f + i * 2.094f;
            Diamond({ c.x + cosf(a3) * r * 0.85f, c.y - r - 8.0f + sinf(a3) * 5.0f }, 4.5f, 4.5f, starCol, ink);
        }
    }
}

inline void Game::DrawWeaponModel(Vector2 hand, Vector2 dir, int wt, float t, bool enchanted) {
    Vector2 perp = { -dir.y, dir.x };
    const Color ink = { 18, 16, 26, 255 };
    if (enchanted) {
        BeginBlendMode(BLEND_ADDITIVE);
        Vector2 gPos = Vector2Add(hand, Vector2Scale(dir, 15.0f));
        CircleGrad(gPos.x, gPos.y, 24.0f, Fade(SKYBLUE, 0.30f), Fade(SKYBLUE, 0.0f));
        EndBlendMode();
    }
    if (wt == WP_BOW) {
        float ang = atan2f(dir.y, dir.x) * RAD2DEG;
        Vector2 bowC = { hand.x + dir.x * 4.0f, hand.y + dir.y * 4.0f };
        DrawRing(bowC, 15.0f, 19.0f, ang - 75.0f, ang + 75.0f, 16, { 165, 112, 58, 255 });
        DrawRing(bowC, 17.5f, 19.5f, ang - 75.0f, ang + 75.0f, 16, { 110, 215, 130, 255 });
        DrawRing(bowC, 15.0f, 16.5f, ang - 65.0f, ang + 65.0f, 16, GOLD);
        Vector2 topLimb = { bowC.x + cosf((ang - 75.0f) * DEG2RAD) * 16.5f, bowC.y + sinf((ang - 75.0f) * DEG2RAD) * 16.5f };
        Vector2 botLimb = { bowC.x + cosf((ang + 75.0f) * DEG2RAD) * 16.5f, bowC.y + sinf((ang + 75.0f) * DEG2RAD) * 16.5f };
        DrawLineEx(topLimb, botLimb, 1.5f, Fade(WHITE, 0.85f));
        Vector2 arrTail = Vector2Subtract(bowC, Vector2Scale(dir, 5.0f));
        Vector2 arrTip = Vector2Add(bowC, Vector2Scale(dir, 24.0f));
        DrawLineEx(arrTail, arrTip, 2.5f, { 225, 200, 140, 255 });
        Tri({ arrTip.x + dir.x * 8.0f, arrTip.y + dir.y * 8.0f }, { arrTip.x + perp.x * 4.2f, arrTip.y + perp.y * 4.2f }, { arrTip.x - perp.x * 4.2f, arrTip.y - perp.y * 4.2f }, { 120, 245, 160, 255 });
    } else if (wt == WP_STAFF) {
        Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 14.0f));
        Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 28.0f));
        DrawLineEx(tail, tip, 5.4f, { 68, 40, 102, 255 });
        DrawLineEx(tail, tip, 2.8f, { 145, 95, 200, 255 });
        DrawLineEx(tail, tip, 1.0f, Fade(WHITE, 0.35f));
        Vector2 pL = Vector2Add(tip, Vector2Add(Vector2Scale(dir, 8.0f), Vector2Scale(perp, 6.5f)));
        Vector2 pR = Vector2Add(tip, Vector2Add(Vector2Scale(dir, 8.0f), Vector2Scale(perp, -6.5f)));
        DrawLineEx(tip, pL, 2.6f, GOLD);
        DrawLineEx(tip, pR, 2.6f, GOLD);
        Vector2 orb = Vector2Add(tip, Vector2Scale(dir, 8.0f));
        float pulse = 0.85f + 0.15f * sinf(t * 7.0f);
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(orb.x, orb.y, 16.0f * pulse, Fade({ 115, 235, 255, 255 }, 0.40f), Fade(SKYBLUE, 0.0f));
        EndBlendMode();
        Diamond(orb, 6.5f * pulse, 8.5f * pulse, { 115, 235, 255, 255 }, WHITE);
    } else if (wt == WP_HAMMER) {
        Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 8.0f));
        Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 27.0f));
        DrawLineEx(tail, tip, 5.2f, { 105, 70, 38, 255 });
        DrawLineEx(tail, tip, 2.2f, { 160, 112, 64, 255 });
        float deg = atan2f(dir.y, dir.x) * RAD2DEG;
        BoxRot(tip, 14.0f, 24.0f, deg, { 176, 186, 210, 255 }, ink, 1.8f);
        BoxRot(tip, 14.5f, 7.0f, deg, GOLD, ink, 1.2f);
        DrawLineEx(Vector2Add(tip, Vector2Scale(perp, -9.0f)), Vector2Add(tip, Vector2Scale(perp, 9.0f)), 1.6f, Fade(WHITE, 0.45f));
        Tri(Vector2Add(tip, Vector2Scale(dir, 7.0f)), Vector2Add(tip, Vector2Add(Vector2Scale(dir, 3.0f), Vector2Scale(perp, 5.0f))),
            Vector2Add(tip, Vector2Add(Vector2Scale(dir, 3.0f), Vector2Scale(perp, -5.0f))), GOLD);
    } else if (wt == WP_SCYTHE) {
        Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 12.0f));
        Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 34.0f));
        DrawLineEx(tail, tip, 4.8f, { 46, 38, 66, 255 });
        DrawLineEx(tail, tip, 2.2f, { 105, 92, 138, 255 });
        Vector2 bladeTip = Vector2Add(tip, Vector2Add(Vector2Scale(dir, -13.0f), Vector2Scale(perp, 25.0f)));
        Vector2 bladeMid = Vector2Add(tip, Vector2Add(Vector2Scale(dir, 6.0f), Vector2Scale(perp, 14.0f)));
        Tri(tip, bladeMid, bladeTip, { 195, 210, 235, 255 });
        DrawLineEx(tip, bladeMid, 2.2f, ink);
        DrawLineEx(bladeMid, bladeTip, 2.6f, { 210, 120, 255, 255 });
        DrawLineEx(tip, bladeTip, 1.2f, Fade(WHITE, 0.50f));
    } else if (wt == WP_AXE) {
        Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 8.0f));
        Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 28.0f));
        DrawLineEx(tail, tip, 5.0f, { 112, 74, 42, 255 });
        QuadOut(Vector2Add(tip, Vector2Add(Vector2Scale(dir, -10.0f), Vector2Scale(perp, 3.0f))),
                Vector2Add(tip, Vector2Add(Vector2Scale(dir,  6.0f), Vector2Scale(perp, 4.0f))),
                Vector2Add(tip, Vector2Add(Vector2Scale(dir,  4.0f), Vector2Scale(perp, 16.0f))),
                Vector2Add(tip, Vector2Add(Vector2Scale(dir, -14.0f), Vector2Scale(perp, 14.0f))), { 198, 206, 228, 255 }, ink, 1.4f);
        QuadOut(Vector2Add(tip, Vector2Add(Vector2Scale(dir, -10.0f), Vector2Scale(perp, -3.0f))),
                Vector2Add(tip, Vector2Add(Vector2Scale(dir,  6.0f), Vector2Scale(perp, -4.0f))),
                Vector2Add(tip, Vector2Add(Vector2Scale(dir,  4.0f), Vector2Scale(perp, -16.0f))),
                Vector2Add(tip, Vector2Add(Vector2Scale(dir, -14.0f), Vector2Scale(perp, -14.0f))), { 166, 174, 198, 255 }, ink, 1.4f);
        DrawLineEx(Vector2Add(tip, Vector2Scale(perp, 14.0f)), Vector2Add(tip, Vector2Add(Vector2Scale(dir, 4.0f), Vector2Scale(perp, 15.0f))), 1.8f, WHITE);
        DrawLineEx(Vector2Add(hand, Vector2Scale(perp, 6.0f)), Vector2Subtract(hand, Vector2Scale(perp, 6.0f)), 3.5f, GOLD);
    } else if (wt == WP_SPEAR) {
        Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 14.0f));
        Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 40.0f));
        DrawLineEx(tail, tip, 4.4f, { 126, 86, 52, 255 });
        Tri(tip, { tip.x - dir.x * 9.0f + perp.x * 5.0f, tip.y - dir.y * 9.0f + perp.y * 5.0f }, { tip.x - dir.x * 9.0f - perp.x * 5.0f, tip.y - dir.y * 9.0f - perp.y * 5.0f }, enchanted ? SKYBLUE : Color{ 215, 52, 68, 255 });
        Diamond(Vector2Add(tip, Vector2Scale(dir, 7.0f)), 4.0f, 4.0f, { 220, 232, 248, 255 }, ink);
        Tri(Vector2Add(tip, Vector2Scale(dir, 18.0f)),
            { tip.x + perp.x * 5.5f, tip.y + perp.y * 5.5f },
            { tip.x - perp.x * 5.5f, tip.y - perp.y * 5.5f }, enchanted ? Color{ 125, 240, 255, 255 } : Color{ 238, 244, 255, 255 });
        DrawLineEx(tip, Vector2Add(tip, Vector2Scale(dir, 18.0f)), 1.6f, WHITE);
    } else {
        // Sword / Dagger / Enchanted Runeblade: lưỡi đa giác vát nhọn + rãnh kiếm sáng & đá quý chuôi kiếm
        bool isDagger = (wt == WP_DAGGER);
        float wlen = isDagger ? 17.0f : (enchanted ? 31.0f : 26.0f);
        float hw = isDagger ? 2.6f : (enchanted ? 4.0f : 3.4f);
        Vector2 baseBlade = Vector2Add(hand, Vector2Scale(dir, 3.0f));
        Vector2 shoulder = Vector2Add(hand, Vector2Scale(dir, wlen - 6.0f));
        Vector2 tip = Vector2Add(hand, Vector2Scale(dir, wlen + (isDagger ? 5.0f : 8.0f)));
        Color bladeCol = enchanted ? Color{ 105, 235, 255, 255 } : (isDagger ? Color{ 205, 215, 238, 255 } : Color{ 230, 238, 252, 255 });
        if (enchanted) {
            DrawLineEx(baseBlade, tip, 10.0f, Fade({ 90, 225, 255, 255 }, 0.35f));
        }
        QuadOut(Vector2Add(baseBlade, Vector2Scale(perp, hw)), Vector2Add(shoulder, Vector2Scale(perp, hw * 0.82f)),
                Vector2Subtract(shoulder, Vector2Scale(perp, hw * 0.82f)), Vector2Subtract(baseBlade, Vector2Scale(perp, hw)),
                bladeCol, enchanted ? Color{ 195, 115, 255, 255 } : ink, 1.4f);
        Tri(Vector2Add(shoulder, Vector2Scale(perp, hw * 0.82f)), Vector2Subtract(shoulder, Vector2Scale(perp, hw * 0.82f)), tip, WHITE);
        DrawLineEx(baseBlade, tip, 1.8f, enchanted ? Color{ 220, 150, 255, 255 } : (isDagger ? Color{ 255, 85, 130, 255 } : WHITE));
        DrawLineEx({ hand.x + perp.x * (isDagger ? 6.0f : 9.0f), hand.y + perp.y * (isDagger ? 6.0f : 9.0f) },
                   { hand.x - perp.x * (isDagger ? 6.0f : 9.0f), hand.y - perp.y * (isDagger ? 6.0f : 9.0f) }, 4.2f, enchanted ? Color{ 165, 110, 255, 255 } : GOLD);
        DrawLineEx(Vector2Subtract(hand, Vector2Scale(dir, 6.0f)), hand, 3.6f, { 95, 60, 36, 255 });
    }
    Diamond(Vector2Subtract(hand, Vector2Scale(dir, 7.0f)), 3.0f, 3.0f, enchanted ? SKYBLUE : GOLD, WHITE);
    float hdeg = atan2f(dir.y, dir.x) * RAD2DEG;
    BoxRot(hand, 7.5f, 7.5f, hdeg, enchanted ? Color{ 110, 230, 255, 255 } : Color{ 150, 160, 188, 255 }, ink, 1.3f);
}

inline void Game::DrawHeroModel(Vector2 p, int heroClass, int wt, Vector2 aimDir, bool facingLeft, float walkTimer, bool moving,
                          float atkSweep, bool isInvuln, bool shieldActive, float holyBuffTimer, float ultActiveTimer,
                          int subClass, float swordOrbitTimer) {
    const float t = (float)GetTime();
    const float face = facingLeft ? -1.0f : 1.0f;
    const float back = -face;
    const Color ink = { 18, 16, 26, 255 };
    float bob = moving ? fabsf(sinf(walkTimer)) * -2.2f : sinf(t * 3.0f) * 1.0f;
    float sway = sinf(t * 8.0f) * 2.5f + (moving ? 4.5f : 0.0f);
    Vector2 body = { p.x, p.y + bob + 2.0f };
    Vector2 head = { body.x, body.y - 14.5f };
    float step = moving ? sinf(walkTimer) * 5.0f : 0.0f;

    // Additive class aura behind hero
    BeginBlendMode(BLEND_ADDITIVE);
    CircleGrad(body.x, body.y - 2.0f, 28.0f, Fade(kClasses[std::max(0, std::min(kNumClasses - 1, heroClass))].color, 0.22f), Fade(WHITE, 0.0f));
    EndBlendMode();

    if (holyBuffTimer > 0.0f) {
        float k = 0.65f + 0.35f * sinf(t * 10.0f);
        DrawPolyLinesEx({ body.x, body.y - 4.0f }, 8, 29.0f, t * 35.0f, 2.0f, Fade(GOLD, 0.70f * k));
    }

    if (heroClass == CLASS_KNIGHT) {
        // VANGUARD KNIGHT: áo choàng xanh hoàng gia 2 lớp lót đỏ, giáp ngực thép bạc, cầu vai vàng, khiên tháp & mũ Crusader
        Color cape = { 44, 74, 168, 255 };
        QuadOut({ body.x + back * 5.0f, body.y - 9.0f }, { body.x + back * 6.0f, body.y + 8.0f },
                { body.x + back * (30.0f + sway), body.y + 18.0f + sway * 0.35f }, { body.x + back * (28.0f + sway), body.y - 4.0f }, cape, ink, 1.5f);
        Tri({ body.x + back * 6.0f, body.y + 5.0f }, { body.x + back * (28.0f + sway), body.y + 15.0f + sway * 0.30f },
            { body.x + back * (22.0f + sway), body.y + 18.0f + sway * 0.35f }, { 148, 34, 48, 255 });
        DrawLineEx({ body.x + back * (28.0f + sway), body.y - 4.0f }, { body.x + back * (30.0f + sway), body.y + 18.0f + sway * 0.35f }, 2.2f, GOLD);
        // Giày giáp sắt
        BoxRot({ p.x - 6.0f, p.y + 15.0f + step * 0.55f }, 7.5f, 8.5f, -step * 2.2f, { 112, 122, 148, 255 }, ink);
        BoxRot({ p.x + 6.0f, p.y + 15.0f - step * 0.55f }, 7.5f, 8.5f,  step * 2.2f, { 112, 122, 148, 255 }, ink);
        // Lông mũ đỏ kép bay phía sau
        Tri({ head.x, head.y - 9.0f }, { head.x + back * (26.0f + sway * 0.45f), head.y - 16.0f + sway * 0.25f }, { head.x + back * 7.0f, head.y - 2.0f }, { 224, 52, 66, 255 });
        Tri({ head.x + back * 3.0f, head.y - 7.0f }, { head.x + back * (23.0f + sway * 0.35f), head.y - 10.0f + sway * 0.25f }, { head.x + back * 8.0f, head.y }, GOLD);
        // Giáp ngực hình thang + phản chiếu kim loại + chữ thập vàng + đai lưng + găng tay giáp
        QuadOut({ body.x - 13.0f, body.y - 8.0f }, { body.x + 13.0f, body.y - 8.0f },
                { body.x + 9.5f, body.y + 10.0f }, { body.x - 9.5f, body.y + 10.0f }, { 214, 222, 240, 255 }, ink, 1.8f);
        Quad({ body.x - 10.5f, body.y - 6.5f }, { body.x - 3.0f, body.y - 6.5f }, { body.x - 4.0f, body.y + 3.0f }, { body.x - 9.0f, body.y + 3.0f }, Fade(WHITE, 0.35f));
        DrawLineEx({ body.x, body.y - 5.0f }, { body.x, body.y + 7.0f }, 2.6f, GOLD);
        DrawLineEx({ body.x - 5.0f, body.y - 1.0f }, { body.x + 5.0f, body.y - 1.0f }, 2.4f, GOLD);
        BoxRot({ body.x, body.y + 7.5f }, 20.0f, 4.5f, 0.0f, { 92, 62, 42, 255 }, ink, 1.2f);
        BoxRot({ body.x, body.y + 7.5f }, 6.0f, 5.5f, 0.0f, GOLD, ink, 1.0f);
        // Cầu vai thép viền vàng & găng tay
        BoxRot({ body.x - 13.5f, body.y - 6.0f }, 9.0f, 8.5f, -18.0f, { 188, 198, 224, 255 }, GOLD, 1.4f);
        BoxRot({ body.x + 13.5f, body.y - 6.0f }, 9.0f, 8.5f,  18.0f, { 188, 198, 224, 255 }, GOLD, 1.4f);
        BoxRot({ body.x + back * 12.0f, body.y + 5.0f }, 6.0f, 6.0f, 0.0f, { 196, 206, 232, 255 }, GOLD, 1.0f);
        // Khiên phụ bên tay còn lại
        Vector2 sh = { body.x + back * 14.5f, body.y + 1.5f };
        QuadOut({ sh.x - 5.5f, sh.y - 8.0f }, { sh.x + 5.5f, sh.y - 8.0f }, { sh.x + 4.5f, sh.y + 4.0f }, { sh.x - 4.5f, sh.y + 4.0f }, { 44, 78, 172, 255 }, { 205, 214, 236, 255 }, 1.8f);
        Tri({ sh.x - 4.5f, sh.y + 4.0f }, { sh.x + 4.5f, sh.y + 4.0f }, { sh.x, sh.y + 10.5f }, { 205, 214, 236, 255 });
        DrawLineEx({ sh.x, sh.y - 5.0f }, { sh.x, sh.y + 6.0f }, 2.0f, GOLD);
        // Mũ giáp Crusader tách khối + cánh nhỏ & khe T cyan phát sáng
        QuadOut({ head.x - 10.0f, head.y - 8.5f }, { head.x + 10.0f, head.y - 8.5f },
                { head.x + 9.0f, head.y + 6.5f }, { head.x - 9.0f, head.y + 6.5f }, { 222, 228, 244, 255 }, ink, 1.7f);
        Tri({ head.x - 10.0f, head.y - 8.5f }, { head.x + 10.0f, head.y - 8.5f }, { head.x, head.y - 14.0f }, { 190, 202, 226, 255 });
        Tri({ head.x - 10.0f, head.y - 4.0f }, { head.x - 16.0f, head.y - 10.0f }, { head.x - 10.0f, head.y - 8.0f }, GOLD);
        Tri({ head.x + 10.0f, head.y - 4.0f }, { head.x + 16.0f, head.y - 10.0f }, { head.x + 10.0f, head.y - 8.0f }, GOLD);
        BoxRot({ head.x + face * 3.0f, head.y + 0.5f }, 12.0f, 4.2f, 0.0f, { 18, 24, 48, 255 }, ink, 1.0f);
        DrawRectangle((int)(head.x + face * 5.0f - 4), (int)(head.y - 0.5f), 8, 2, { 90, 230, 255, 255 });
        DrawRectangle((int)(head.x + face * 5.0f - 1), (int)(head.y - 0.5f), 2, 5, { 90, 230, 255, 255 });
    } else if (heroClass == CLASS_ASSASSIN) {
        // SHADOW ASSASSIN: khăn đỏ bay đôi viền sáng, giáp da sát thủ, túi phi tiêu thắt lưng, dao găm phụ, mắt đỏ
        Color scarf = { 212, 42, 74, 255 };
        Tri({ body.x + back * 3.0f, body.y - 9.0f }, { body.x + back * (34.0f + sway * 1.15f), body.y - 15.0f + sway * 0.45f }, { body.x + back * (26.0f + sway), body.y + 1.0f }, scarf);
        DrawLineEx({ body.x + back * 5.0f, body.y - 9.0f }, { body.x + back * (34.0f + sway * 1.15f), body.y - 15.0f + sway * 0.45f }, 1.6f, { 255, 115, 145, 255 });
        Tri({ body.x + back * 3.0f, body.y - 4.0f }, { body.x + back * (28.0f + sway), body.y + 4.0f }, { body.x + back * (22.0f + sway), body.y + 14.0f + sway * 0.35f }, Shade(scarf, 0.72f));
        // Ủng ninja gọn
        BoxRot({ p.x - 5.5f, p.y + 15.0f + step * 0.55f }, 6.5f, 8.0f, -step * 2.5f, { 34, 30, 44, 255 }, ink);
        BoxRot({ p.x + 5.5f, p.y + 15.0f - step * 0.55f }, 6.5f, 8.0f,  step * 2.5f, { 34, 30, 44, 255 }, ink);
        // Thân giáp da tối + dây chéo + túi kunai + đai đỏ
        QuadOut({ body.x - 11.5f, body.y - 8.5f }, { body.x + 11.5f, body.y - 8.5f },
                { body.x + 8.5f, body.y + 9.5f }, { body.x - 8.5f, body.y + 9.5f }, { 46, 44, 66, 255 }, ink, 1.7f);
        DrawLineEx({ body.x - 9.0f, body.y - 7.0f }, { body.x + 7.5f, body.y + 7.5f }, 2.2f, { 112, 92, 130, 255 });
        DrawLineEx({ body.x + 9.0f, body.y - 7.0f }, { body.x - 7.5f, body.y + 7.5f }, 2.2f, { 112, 92, 130, 255 });
        BoxRot({ body.x, body.y + 7.5f }, 17.5f, 4.0f, 0.0f, scarf, ink, 1.0f);
        BoxRot({ body.x - 8.0f, body.y + 7.5f }, 4.5f, 5.5f, 0.0f, { 90, 62, 46, 255 }, ink, 0.9f);
        BoxRot({ body.x + 8.0f, body.y + 7.5f }, 4.5f, 5.5f, 0.0f, { 90, 62, 46, 255 }, ink, 0.9f);
        // Vai góc cạnh
        Tri({ body.x - 10.0f, body.y - 8.0f }, { body.x - 16.5f, body.y - 4.0f }, { body.x - 11.0f, body.y }, { 70, 64, 96, 255 });
        Tri({ body.x + 10.0f, body.y - 8.0f }, { body.x + 16.5f, body.y - 4.0f }, { body.x + 11.0f, body.y }, { 70, 64, 96, 255 });
        // Dao găm phụ tay trái
        if (wt == WP_DAGGER) {
            Vector2 offH = { body.x - aimDir.x * 8.0f + back * 10.0f, body.y + 3.0f };
            DrawLineEx(offH, { offH.x + face * 12.0f, offH.y + 6.0f }, 3.0f, { 215, 224, 248, 255 });
            BoxRot(offH, 5.5f, 5.5f, 0.0f, { 70, 64, 96, 255 }, ink, 1.0f);
        }
        // Đầu trùm mũ ninja + mặt nạ + mắt đỏ rực
        QuadOut({ head.x - 9.5f, head.y - 7.5f }, { head.x + 9.5f, head.y - 7.5f },
                { head.x + 8.0f, head.y + 6.5f }, { head.x - 8.0f, head.y + 6.5f }, { 42, 38, 60, 255 }, ink, 1.6f);
        Tri({ head.x - 9.5f, head.y - 7.5f }, { head.x + 9.5f, head.y - 7.5f }, { head.x + back * 8.0f, head.y - 16.0f }, { 34, 30, 50, 255 });
        BoxRot({ head.x + face * 3.0f, head.y + 0.2f }, 11.5f, 5.2f, 0.0f, { 16, 12, 24, 255 }, ink, 1.0f);
        Diamond({ head.x + face * 6.0f, head.y + 0.2f }, 2.6f, 2.0f, { 255, 70, 125, 255 }, RED);
        Diamond({ head.x + face * 0.5f, head.y + 0.2f }, 2.6f, 2.0f, { 255, 70, 125, 255 }, RED);
        if (ultActiveTimer > 0.0f) {
            for (int i = 0; i < 4; ++i) {
                float a3 = t * 14.0f + i * (PI / 2.0f);
                DrawRing({ body.x, body.y - 4.0f }, 32.0f, 37.0f, a3 * RAD2DEG, a3 * RAD2DEG + 55.0f, 8, Fade({ 255, 70, 140, 255 }, 0.85f));
            }
        }
    } else if (heroClass == CLASS_MAGE) {
        // ARCANE MAGE: áo choàng tầng thêu chỉ vàng, sách phép + vòng rune, mũ phù thủy đính tinh thể treo chóp
        Color robe = { 82, 66, 178, 255 };
        BoxRot({ p.x - 5.0f, p.y + 16.0f + step * 0.35f }, 6.0f, 6.5f, 0.0f, { 42, 34, 72, 255 }, ink);
        BoxRot({ p.x + 5.0f, p.y + 16.0f - step * 0.35f }, 6.0f, 6.5f, 0.0f, { 42, 34, 72, 255 }, ink);
        // Váy áo choàng xòe tầng dưới + thân trên + dây đai rune vàng
        QuadOut({ body.x - 11.0f, body.y - 7.5f }, { body.x + 11.0f, body.y - 7.5f },
                { body.x + 16.0f, body.y + 15.0f }, { body.x - 16.0f, body.y + 15.0f }, Shade(robe, 0.78f), ink, 1.7f);
        QuadOut({ body.x - 10.5f, body.y - 8.5f }, { body.x + 10.5f, body.y - 8.5f },
                { body.x + 8.0f, body.y + 5.5f }, { body.x - 8.0f, body.y + 5.5f }, robe, ink, 1.5f);
        DrawLineEx({ body.x, body.y - 6.0f }, { body.x, body.y + 14.0f }, 3.2f, GOLD);
        DrawLineEx({ body.x - 15.0f, body.y + 14.0f }, { body.x + 15.0f, body.y + 14.0f }, 1.8f, Fade(GOLD, 0.75f));
        Diamond({ body.x, body.y - 2.0f }, 3.6f, 4.6f, { 110, 240, 255, 255 }, ink);
        BoxRot({ body.x - 12.0f, body.y - 5.5f }, 7.5f, 6.5f, -20.0f, Shade(robe, 1.25f), GOLD, 1.2f);
        BoxRot({ body.x + 12.0f, body.y - 5.5f }, 7.5f, 6.5f,  20.0f, Shade(robe, 1.25f), GOLD, 1.2f);
        // Sách phép mở + vòng tròn ma thuật bay ở tay còn lại
        Vector2 bk = { body.x + back * 15.5f, body.y - 1.0f + sinf(t * 4.5f) * 2.0f };
        DrawCircleLines((int)bk.x, (int)bk.y, 10.5f, Fade(SKYBLUE, 0.55f));
        BoxRot(bk, 10.0f, 11.0f, back * 14.0f, { 95, 42, 130, 255 }, GOLD, 1.2f);
        BoxRot(bk, 7.5f, 8.5f, back * 14.0f, { 238, 232, 210, 255 }, { 140, 120, 90, 255 }, 0.8f);
        // Đầu + khuôn mặt trong bóng râm + mắt sáng cyan
        QuadOut({ head.x - 8.5f, head.y - 5.0f }, { head.x + 8.5f, head.y - 5.0f },
                { head.x + 7.5f, head.y + 6.5f }, { head.x - 7.5f, head.y + 6.5f }, { 22, 16, 44, 255 }, ink, 1.5f);
        Diamond({ head.x + face * 5.5f, head.y + 1.0f }, 2.4f, 2.0f, { 105, 240, 255, 255 }, ink);
        Diamond({ head.x + face * 0.5f, head.y + 1.0f }, 2.4f, 2.0f, { 105, 240, 255, 255 }, ink);
        // Mũ phù thủy vành đa giác rộng + chóp cong nhọn + đai vàng + đèn pha lê treo chóp mũ
        QuadOut({ head.x - 19.0f, head.y - 5.5f }, { head.x + 19.0f, head.y - 5.5f },
                { head.x + 13.0f, head.y - 2.0f }, { head.x - 13.0f, head.y - 2.0f }, { 64, 48, 148, 255 }, ink, 1.6f);
        QuadOut({ head.x - 10.5f, head.y - 13.5f }, { head.x + 10.5f, head.y - 13.5f },
                { head.x + 11.5f, head.y - 5.5f }, { head.x - 11.5f, head.y - 5.5f }, { 78, 60, 170, 255 }, ink, 1.5f);
        Vector2 hatTip = { head.x + back * 12.0f, head.y - 26.5f };
        Tri({ head.x - 10.5f, head.y - 13.5f }, { head.x + 10.5f, head.y - 13.5f }, hatTip, { 68, 52, 152, 255 });
        Diamond({ hatTip.x, hatTip.y + 5.0f }, 3.0f, 4.2f, SKYBLUE, WHITE);
        DrawLineEx({ head.x - 11.0f, head.y - 6.5f }, { head.x + 11.0f, head.y - 6.5f }, 2.4f, GOLD);
        Diamond({ head.x + face * 2.5f, head.y - 9.5f }, 3.2f, 3.6f, { 110, 240, 255, 255 }, ink);
        for (int i = 0; i < 2; ++i) {
            float a3 = t * 3.2f + i * PI;
            Diamond({ body.x + cosf(a3) * 23.0f, body.y - 4.0f + sinf(a3) * 8.0f }, 3.4f, 4.4f, { 110, 235, 255, 255 }, ink);
        }
    } else if (heroClass == CLASS_RANGER) {
        // EMERALD RANGER: áo choàng lá rừng viền vàng, ống tên lông chim, giáp da & băng tay thợ săn, mũ trùm gắn ngọc lục bảo
        Color cloak = { 42, 134, 78, 255 };
        QuadOut({ body.x + back * 4.0f, body.y - 8.5f }, { body.x + back * 6.0f, body.y + 8.0f },
                { body.x + back * (28.0f + sway), body.y + 16.5f + sway * 0.3f }, { body.x + back * (24.0f + sway), body.y - 3.0f }, cloak, ink, 1.5f);
        DrawLineEx({ body.x + back * 6.0f, body.y + 7.5f }, { body.x + back * (28.0f + sway), body.y + 16.5f + sway * 0.3f }, 1.6f, { 130, 235, 150, 255 });
        // Ống tên đeo chéo sau vai + 2 đuôi lông tên
        DrawLineEx({ body.x + back * 7.0f, body.y - 14.0f }, { body.x + back * 14.0f, body.y + 5.0f }, 6.0f, { 126, 84, 46, 255 });
        Tri({ body.x + back * 5.0f, body.y - 16.0f }, { body.x + back * 9.0f, body.y - 15.0f }, { body.x + back * 6.5f, body.y - 21.0f }, { 255, 220, 110, 255 });
        Tri({ body.x + back * 8.0f, body.y - 15.0f }, { body.x + back * 12.0f, body.y - 14.0f }, { body.x + back * 10.5f, body.y - 20.0f }, { 120, 240, 160, 255 });
        // Ủng săn da nâu
        BoxRot({ p.x - 5.5f, p.y + 15.0f + step * 0.55f }, 7.0f, 8.5f, -step * 2.4f, { 88, 58, 38, 255 }, ink);
        BoxRot({ p.x + 5.5f, p.y + 15.0f - step * 0.55f }, 7.0f, 8.5f,  step * 2.4f, { 88, 58, 38, 255 }, ink);
        // Áo giáp da + đai chéo đựng tên + băng tay thợ săn
        QuadOut({ body.x - 12.0f, body.y - 8.0f }, { body.x + 12.0f, body.y - 8.0f },
                { body.x + 9.0f, body.y + 9.5f }, { body.x - 9.0f, body.y + 9.5f }, { 56, 168, 96, 255 }, ink, 1.7f);
        QuadOut({ body.x - 8.5f, body.y - 6.0f }, { body.x + 8.5f, body.y - 6.0f },
                { body.x + 7.5f, body.y + 5.5f }, { body.x - 7.5f, body.y + 5.5f }, { 116, 80, 48, 255 }, ink, 1.2f);
        DrawLineEx({ body.x - 9.0f, body.y - 7.0f }, { body.x + 7.0f, body.y + 8.0f }, 2.6f, { 150, 106, 64, 255 });
        Diamond({ body.x, body.y + 7.5f }, 3.2f, 2.8f, GOLD, ink);
        BoxRot({ body.x + back * 12.5f, body.y - 5.5f }, 8.0f, 7.0f, back * 16.0f, { 116, 80, 48, 255 }, ink, 1.3f);
        BoxRot({ body.x + back * 12.0f, body.y + 5.0f }, 5.5f, 5.5f, 0.0f, { 116, 80, 48, 255 }, GOLD, 1.0f);
        // Đầu + mũ trùm thợ săn + ngọc lục bảo + lông vàng + khăn che mặt
        QuadOut({ head.x - 9.5f, head.y - 7.5f }, { head.x + 9.5f, head.y - 7.5f },
                { head.x + 8.5f, head.y + 6.5f }, { head.x - 8.5f, head.y + 6.5f }, { 48, 148, 84, 255 }, ink, 1.6f);
        Tri({ head.x - 9.5f, head.y - 7.5f }, { head.x + 9.5f, head.y - 7.5f }, { head.x + back * 9.0f, head.y - 16.0f }, { 38, 120, 68, 255 });
        Tri({ head.x + back * 5.0f, head.y - 9.0f }, { head.x + back * 8.0f, head.y - 7.0f }, { head.x + back * 16.0f, head.y - 21.0f }, GOLD);
        Diamond({ head.x + face * 3.0f, head.y - 6.0f }, 2.8f, 3.2f, LIME, GOLD);
        BoxRot({ head.x + face * 3.0f, head.y + 0.5f }, 11.0f, 5.0f, 0.0f, { 18, 28, 22, 255 }, ink, 1.0f);
        Diamond({ head.x + face * 6.0f, head.y + 0.5f }, 2.4f, 2.0f, { 255, 225, 95, 255 }, ink);
        Diamond({ head.x + face * 0.5f, head.y + 0.5f }, 2.4f, 2.0f, { 255, 225, 95, 255 }, ink);
    } else {
        // ROYAL LANCEMASTER: áo choàng đỏ hoàng gia lót vàng, giáp bạc bạch kim chạm rồng, cầu vai đại bàng, mũ mào lông hoàng gia
        Color cloak = { 172, 34, 52, 255 };
        QuadOut({ body.x + back * 4.0f, body.y - 9.0f }, { body.x + back * 6.0f, body.y + 9.0f },
                { body.x + back * (32.0f + sway), body.y + 19.0f + sway * 0.35f }, { body.x + back * (28.0f + sway), body.y - 4.0f }, cloak, ink, 1.5f);
        DrawLineEx({ body.x + back * (28.0f + sway), body.y - 4.0f }, { body.x + back * (32.0f + sway), body.y + 19.0f + sway * 0.35f }, 2.4f, GOLD);
        BoxRot({ p.x - 6.0f, p.y + 15.0f + step * 0.55f }, 7.5f, 8.5f, -step * 2.2f, { 165, 174, 198, 255 }, ink);
        BoxRot({ p.x + 6.0f, p.y + 15.0f - step * 0.55f }, 7.5f, 8.5f,  step * 2.2f, { 165, 174, 198, 255 }, ink);
        // Giáp ngực bạch kim viền vàng + biểu tượng Long Văn
        QuadOut({ body.x - 13.5f, body.y - 8.5f }, { body.x + 13.5f, body.y - 8.5f },
                { body.x + 10.0f, body.y + 10.0f }, { body.x - 10.0f, body.y + 10.0f }, { 232, 236, 246, 255 }, ink, 1.8f);
        Diamond({ body.x, body.y - 1.0f }, 4.2f, 5.2f, { 255, 195, 55, 255 }, ink);
        BoxRot({ body.x, body.y + 7.5f }, 20.5f, 4.5f, 0.0f, { 165, 34, 48, 255 }, ink, 1.2f);
        BoxRot({ body.x, body.y + 7.5f }, 6.0f, 5.5f, 0.0f, GOLD, ink, 1.0f);
        // Cầu vai rồng vàng
        BoxRot({ body.x - 14.0f, body.y - 6.0f }, 9.5f, 8.5f, -20.0f, GOLD, ink, 1.4f);
        BoxRot({ body.x + 14.0f, body.y - 6.0f }, 9.5f, 8.5f,  20.0f, GOLD, ink, 1.4f);
        // Mũ chiến kỵ binh hoàng gia + mào lông trắng vàng + mắt xanh sáng
        QuadOut({ head.x - 10.0f, head.y - 8.5f }, { head.x + 10.0f, head.y - 8.5f },
                { head.x + 9.0f, head.y + 6.5f }, { head.x - 9.0f, head.y + 6.5f }, { 234, 238, 248, 255 }, ink, 1.7f);
        Tri({ head.x, head.y - 9.0f }, { head.x + back * (28.0f + sway * 0.4f), head.y - 17.0f + sway * 0.25f }, { head.x + back * 7.0f, head.y - 2.0f }, GOLD);
        Tri({ head.x + back * 3.0f, head.y - 7.0f }, { head.x + back * (22.0f + sway * 0.3f), head.y - 10.0f + sway * 0.2f }, { head.x + back * 8.0f, head.y }, { 245, 70, 90, 255 });
        BoxRot({ head.x + face * 3.0f, head.y + 0.5f }, 12.0f, 4.2f, 0.0f, { 22, 22, 38, 255 }, ink, 1.0f);
        DrawRectangle((int)(head.x + face * 5.0f - 4), (int)(head.y - 0.5f), 8, 2, { 255, 220, 95, 255 });
    }

    // Vũ khí theo hướng ngắm (Class quyết định model; Kiếm Sư dùng Gậy nhưng hiển thị model Kiếm Phép!)
    int visWt = VisualWeaponType(heroClass, subClass, wt);
    bool enchanted = (subClass > 0 && ((heroClass == CLASS_MAGE && subClass == 1) || (heroClass == CLASS_RANGER && subClass == 2) || heroClass == CLASS_TEMPLAR));
    float a = atan2f(aimDir.y, aimDir.x);
    float sa = a + atkSweep;
    Vector2 dir = { cosf(sa), sinf(sa) };
    Vector2 hand = { body.x + dir.x * 16.0f, body.y + dir.y * 16.0f };
    DrawWeaponModel(hand, dir, visWt, t, enchanted);

    if (subClass == 1 || subClass == 2) {
        const SubclassDef& sc = kSubclasses[std::max(0, std::min(kNumClasses - 1, heroClass))][subClass - 1];
        DrawPolyLinesEx({ body.x, body.y + 12.0f }, 6, 20.0f, t * 40.0f, 1.5f, Fade(sc.color, 0.55f));
        if (heroClass == CLASS_KNIGHT && subClass == 2) { // Berserker horned helm & blood aura
            Tri({ head.x - 9.0f, head.y - 6.0f }, { head.x - 16.0f, head.y - 16.0f }, { head.x - 4.0f, head.y - 8.5f }, { 235, 65, 65, 255 });
            Tri({ head.x + 9.0f, head.y - 6.0f }, { head.x + 16.0f, head.y - 16.0f }, { head.x + 4.0f, head.y - 8.5f }, { 235, 65, 65, 255 });
        } else if (heroClass == CLASS_ASSASSIN && subClass == 1) { // Phantom Reaper floating soul wisps
            for (int si = 0; si < 2; ++si) {
                float sa = t * 3.4f + si * PI;
                Diamond({ body.x + cosf(sa) * 24.0f, body.y - 6.0f + sinf(sa) * 9.0f }, 3.8f, 4.8f, { 195, 90, 255, 255 }, WHITE);
            }
        } else if (heroClass == CLASS_MAGE && subClass == 1) { // Spellblade (Kiem Su): vuong mien phep & kiem bay quanh nguoi
            DrawLineEx({ head.x - 9.0f, head.y - 9.5f }, { head.x + 9.0f, head.y - 9.5f }, 2.4f, { 95, 235, 255, 255 });
            Diamond({ head.x, head.y - 12.0f }, 3.5f, 4.5f, { 210, 110, 255, 255 }, WHITE);
            int orbBlades = (swordOrbitTimer > 0.0f) ? 6 : 3;
            float orbRad = (swordOrbitTimer > 0.0f) ? 46.0f : 28.0f;
            float spinSpd = (swordOrbitTimer > 0.0f) ? 6.5f : 2.6f;
            for (int bi = 0; bi < orbBlades; ++bi) {
                float ba = t * spinSpd + bi * (2.0f * PI / orbBlades);
                Vector2 bp = { body.x + cosf(ba) * orbRad, body.y - 3.0f + sinf(ba) * orbRad * (swordOrbitTimer > 0.0f ? 0.92f : 0.45f) };
                Vector2 bdir = { -sinf(ba), cosf(ba) };
                DrawLineEx(Vector2Subtract(bp, Vector2Scale(bdir, 8.0f)), Vector2Add(bp, Vector2Scale(bdir, 9.0f)), 3.0f, { 110, 240, 255, 255 });
                DrawLineEx(Vector2Subtract(bp, Vector2Scale(bdir, 5.0f)), Vector2Add(bp, Vector2Scale(bdir, 8.0f)), 1.4f, WHITE);
            }
        } else if (heroClass == CLASS_RANGER && subClass == 2) { // Valkyrie winged crown
            Tri({ head.x - 9.0f, head.y - 7.0f }, { head.x - 19.0f, head.y - 15.0f }, { head.x - 8.0f, head.y - 1.0f }, { 255, 220, 100, 255 });
            Tri({ head.x + 9.0f, head.y - 7.0f }, { head.x + 19.0f, head.y - 15.0f }, { head.x + 8.0f, head.y - 1.0f }, { 255, 220, 100, 255 });
        } else if (heroClass == CLASS_TEMPLAR) { // Royal Lancemaster subclass aura & dragon horns
            Color hCol = (subClass == 1) ? GOLD : Color{ 245, 75, 95, 255 };
            Tri({ head.x - 9.0f, head.y - 6.0f }, { head.x - 17.0f, head.y - 17.0f }, { head.x - 4.0f, head.y - 8.5f }, hCol);
            Tri({ head.x + 9.0f, head.y - 6.0f }, { head.x + 17.0f, head.y - 17.0f }, { head.x + 4.0f, head.y - 8.5f }, hCol);
        }
    }

    if (shieldActive) {
        DrawPolyLinesEx({ body.x, body.y - 4.0f }, 8, 28.0f, t * 20.0f, 2.0f, Fade(GOLD, 0.85f));
    }
    if (isInvuln) {
        float k = 0.5f + 0.5f * sinf(t * 30.0f);
        DrawPolyLinesEx({ body.x, body.y - 4.0f }, 8, 24.5f, -t * 45.0f, 1.8f, Fade(SKYBLUE, 0.4f + 0.5f * k));
    }
}

inline void Game::DrawPlayerSprite() {
    const bool moving = player.walkTimer > 0.01f && (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)
                          || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || (ctl.active && Vector2Length(ctl.move) > 0.1f));
    int wt = hasEquip[SLOT_WEAPON] ? equipped[SLOT_WEAPON].weaponType : PreferredWeaponType();
    int visWt = VisualWeaponType(player.heroClass, player.subClass, wt);
    float effRate = EffectiveAttackRate();
    float sinceAtk = (1.0f / std::max(0.1f, effRate)) - player.attackTimer;
    float off = 0.5f;
    if (visWt == WP_BOW || visWt == WP_STAFF) off = 0.0f;
    else if (player.attackTimer > 0.0f && sinceAtk < 0.16f) off = Lerp(-1.25f, 1.25f, sinceAtk / 0.16f);
    float tm = (float)GetTime();
    Color auraCol = nextHitCrit ? GOLD : HeroColor();
    DrawEllipseLines((int)player.pos.x, (int)player.pos.y + 14, 21.0f, 8.5f, Fade(auraCol, 0.35f));
    if (nextHitCrit || (hasEquip[SLOT_WEAPON] && equipPlus[SLOT_WEAPON] > 0)) {
        DrawPolyLinesEx({ player.pos.x, player.pos.y + 14.0f }, 6, 24.0f, tm * 45.0f, 1.5f, Fade(GOLD, 0.55f));
    }
    if (overdriveTimer > 0.0f || player.ultActiveTimer > 0.0f) {
        // Đôi Cánh Ánh Sáng 2.5D bung sau lưng khi Bùng Nổ Linh Hồn hoặc Tuyệt Kỹ
        Color wCol = (overdriveTimer > 0.0f) ? GOLD : HeroColor();
        float flap = sinf(tm * 9.0f) * 5.0f;
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(player.pos.x, player.pos.y - 4.0f, 44.0f, Fade(wCol, 0.34f), Fade(wCol, 0.0f));
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            for (int f = 0; f < 3; ++f) {
                float len = 36.0f - f * 7.0f;
                float yOff = -14.0f + f * 7.5f + flap * (1.0f - f * 0.25f);
                Vector2 root = { player.pos.x + s2 * 5.0f, player.pos.y - 3.0f + f * 3.0f };
                Vector2 tip = { player.pos.x + s2 * len, player.pos.y + yOff };
                Vector2 low = { player.pos.x + s2 * (len * 0.58f), player.pos.y + yOff + 10.0f };
                Tri(root, tip, low, Fade(wCol, 0.65f));
                DrawLineEx(root, tip, 2.0f, Fade(WHITE, 0.85f));
            }
        }
        EndBlendMode();
    }
    DrawHeroModel(player.pos, player.heroClass, wt, aimDirDraw, player.facingLeft, player.walkTimer, moving, off, player.isInvuln, player.shieldActive, player.holyBuffTimer, player.ultActiveTimer, player.subClass, player.swordOrbitTimer);
    if (isKing || peerageRank >= 7) {
        float cy = player.pos.y - 29.0f + sinf(tm * 3.5f) * 1.5f;
        Tri({ player.pos.x - 9.0f, cy + 3.0f }, { player.pos.x - 10.0f, cy - 6.0f }, { player.pos.x - 3.0f, cy + 3.0f }, GOLD);
        Tri({ player.pos.x - 4.0f, cy + 3.0f }, { player.pos.x,        cy - 9.0f }, { player.pos.x + 4.0f, cy + 3.0f }, GOLD);
        Tri({ player.pos.x + 3.0f, cy + 3.0f }, { player.pos.x + 10.0f, cy - 6.0f }, { player.pos.x + 9.0f, cy + 3.0f }, GOLD);
        DrawRectangle((int)player.pos.x - 9, (int)cy + 2, 18, 4, { 215, 165, 35, 255 });
        Diamond({ player.pos.x, cy + 1.0f }, 2.4f, 2.8f, isKing ? RED : SKYBLUE, WHITE);
    }
    if (hasEquip[SLOT_WEAPON] && equipPlus[SLOT_WEAPON] > 0) {
        int cnt = std::min(5, equipPlus[SLOT_WEAPON]);
        for (int i = 0; i < cnt; ++i) {
            float a = tm * 3.0f + i * (2.0f * PI / cnt);
            Vector2 mp = { player.pos.x + cosf(a) * 26.0f, player.pos.y - 4.0f + sinf(a) * 10.0f };
            Diamond(mp, 3.0f, 3.6f, (cnt >= 4) ? SKYBLUE : GOLD, WHITE);
        }
    }
}

inline void Game::DrawChest(const Chest& c) {
    float t = (float)GetTime();
    Vector2 p = c.pos;
    float s = c.vault ? 1.25f : (c.cursed ? 1.15f : 1.0f);
    Color wood = c.vault ? Color{ 110, 56, 140, 255 } : (c.cursed ? Color{ 58, 24, 52, 255 } : Color{ 150, 96, 52, 255 });
    Color band = c.vault ? Color{ 255, 224, 120, 255 } : (c.cursed ? Color{ 235, 55, 85, 255 } : Color{ 232, 190, 70, 255 });
    DrawEllipse((int)p.x, (int)p.y + 18, 28.0f * s, 9.0f * s, Fade(BLACK, 0.38f));
    if (!c.opened) {
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(p.x, p.y, 60.0f * s, Fade(band, 0.22f + 0.08f * sinf(t * 3.0f)), Fade(band, 0.0f));
        EndBlendMode();
    }
    // Thân rương
    DrawRectangle((int)(p.x - 22 * s), (int)(p.y - 4 * s), (int)(44 * s), (int)(22 * s), Shade(wood, 0.55f));
    DrawRectangle((int)(p.x - 20 * s), (int)(p.y - 2 * s), (int)(40 * s), (int)(18 * s), wood);
    for (int i = 0; i < 3; ++i) DrawLine((int)(p.x - 20 * s), (int)(p.y + (2 + i * 5) * s), (int)(p.x + 20 * s), (int)(p.y + (2 + i * 5) * s), Shade(wood, 0.7f));
    DrawRectangle((int)(p.x - 22 * s), (int)(p.y - 4 * s), (int)(44 * s), (int)(4 * s), band);
    DrawRectangle((int)(p.x - 3 * s), (int)(p.y - 4 * s), (int)(6 * s), (int)(14 * s), band);
    // Nắp
    if (!c.opened) {
        DrawRectangle((int)(p.x - 23 * s), (int)(p.y - 16 * s), (int)(46 * s), (int)(13 * s), Shade(wood, 0.55f));
        DrawRectangle((int)(p.x - 21 * s), (int)(p.y - 14 * s), (int)(42 * s), (int)(10 * s), Shade(wood, 1.15f));
        DrawRectangle((int)(p.x - 23 * s), (int)(p.y - 8 * s), (int)(46 * s), (int)(3 * s), band);
        DrawRectangle((int)(p.x - 4 * s), (int)(p.y - 6 * s), (int)(8 * s), (int)(8 * s), { 70, 50, 20, 255 });
        if (c.cursed) {
            DrawLineEx({ p.x - 21 * s, p.y - 15 * s }, { p.x + 21 * s, p.y + 15 * s }, 2.2f, band);
            DrawLineEx({ p.x + 21 * s, p.y - 15 * s }, { p.x - 21 * s, p.y + 15 * s }, 2.2f, band);
            Diamond({ p.x, p.y - 2 * s }, 5.0f, 6.5f, RED, GOLD);
            const char* cLbl = L("CURSED CHEST", "RƯƠNG NGUYỀN RỦA");
            DrawText(cLbl, (int)p.x - MeasureText(cLbl, 12) / 2, (int)(p.y + 22 * s), 12, { 255, 110, 130, 255 });
        }
    } else {
        float lift = c.anim * 16.0f * s;
        DrawRectangle((int)(p.x - 21 * s), (int)(p.y - 28 * s - lift * 0.3f), (int)(42 * s), (int)(10 * s), Shade(wood, 0.8f));
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(p.x, p.y - 4, 34.0f * s * (1.0f - c.anim * 0.5f), Fade({ 255, 230, 120, 255 }, 0.5f * (1.0f - c.anim)), Fade({ 255, 230, 120, 255 }, 0.0f));
        EndBlendMode();
    }
}

inline void Game::DrawMerchant() {
    float t = (float)GetTime();
    Vector2 p = merchantPos;
    float bob = sinf(t * 2.0f) * 2.0f;
    const Color ink = { 18, 16, 26, 255 };
    DrawEllipse((int)p.x, (int)p.y + 26, 36.0f, 11.0f, Fade(BLACK, 0.42f));
    // Quầy hàng gỗ bọc vải đỏ + đống vàng & bình thuốc
    DrawRectangle((int)p.x + 32, (int)p.y + 2, 52, 28, { 112, 72, 40, 255 });
    DrawRectangle((int)p.x + 32, (int)p.y + 2, 52, 6, { 164, 112, 64, 255 });
    DrawRectangleLines((int)p.x + 32, (int)p.y + 2, 52, 28, ink);
    DrawRectangle((int)p.x + 36, (int)p.y + 8, 44, 14, { 148, 36, 52, 255 });
    for (int i = 0; i < 4; ++i) DrawRectangle((int)p.x + 38 + i * 7, (int)p.y - 4 - (i % 2) * 3, 5, 6, GOLD);
    QuadOut({ p.x + 69.0f, p.y - 7.0f }, { p.x + 77.0f, p.y - 7.0f }, { p.x + 79.0f, p.y + 1.0f }, { p.x + 67.0f, p.y + 1.0f }, { 225, 48, 72, 255 }, WHITE, 1.0f);
    // Túi hành lý sau lưng thương nhân
    BoxRot({ p.x + 17.0f, p.y + 2.0f + bob }, 14.0f, 22.0f, 10.0f, { 116, 78, 46, 255 }, ink, 1.6f);
    // Ủng & áo choàng thương nhân nhiều lớp
    BoxRot({ p.x - 8.0f, p.y + 24.0f }, 9.0f, 8.0f, 0.0f, { 48, 36, 32, 255 }, ink);
    BoxRot({ p.x + 8.0f, p.y + 24.0f }, 9.0f, 8.0f, 0.0f, { 48, 36, 32, 255 }, ink);
    QuadOut({ p.x - 16.0f, p.y - 12.0f + bob }, { p.x + 16.0f, p.y - 12.0f + bob },
            { p.x + 25.0f, p.y + 22.0f + bob }, { p.x - 25.0f, p.y + 22.0f + bob }, { 92, 52, 150, 255 }, ink, 1.8f);
    QuadOut({ p.x - 12.0f, p.y - 12.0f + bob }, { p.x + 12.0f, p.y - 12.0f + bob },
            { p.x + 15.0f, p.y + 20.0f + bob }, { p.x - 15.0f, p.y + 20.0f + bob }, { 126, 78, 196, 255 }, ink, 1.4f);
    DrawLineEx({ p.x, p.y - 10.0f + bob }, { p.x, p.y + 20.0f + bob }, 3.2f, GOLD);
    BoxRot({ p.x, p.y + 6.0f + bob }, 34.0f, 5.0f, 0.0f, { 88, 56, 34, 255 }, ink, 1.2f);
    // Mũ trùm thương nhân + mặt tối + mắt sáng vàng
    Vector2 hc = { p.x, p.y - 24.0f + bob };
    Tri({ hc.x - 16.0f, hc.y + 10.0f }, { hc.x + 16.0f, hc.y + 10.0f }, { hc.x + 4.0f, hc.y - 20.0f }, { 70, 38, 120, 255 });
    QuadOut({ hc.x - 12.0f, hc.y - 6.0f }, { hc.x + 12.0f, hc.y - 6.0f },
            { hc.x + 10.0f, hc.y + 10.0f }, { hc.x - 10.0f, hc.y + 10.0f }, { 20, 12, 36, 255 }, ink, 1.6f);
    Diamond({ hc.x - 4.5f, hc.y + 2.0f }, 2.6f, 2.2f, { 255, 220, 90, 255 }, ink);
    Diamond({ hc.x + 4.5f, hc.y + 2.0f }, 2.6f, 2.2f, { 255, 220, 90, 255 }, ink);
    // Đèn lồng treo trên gậy
    Vector2 lan = { p.x - 44.0f + sinf(t * 2.5f) * 3.0f, p.y - 2.0f };
    DrawLineEx({ p.x - 18.0f, p.y - 6.0f + bob }, { lan.x, lan.y - 10.0f }, 2.4f, { 90, 70, 50, 255 });
    BeginBlendMode(BLEND_ADDITIVE);
    CircleGrad(lan.x, lan.y, 46.0f, Fade({ 255, 190, 80, 255 }, 0.4f), Fade({ 255, 190, 80, 255 }, 0.0f));
    EndBlendMode();
    DrawRectangle((int)lan.x - 5, (int)lan.y - 6, 10, 13, { 255, 214, 110, 255 });
    DrawRectangleLines((int)lan.x - 5, (int)lan.y - 6, 10, 13, { 90, 60, 30, 255 });
    // Biển hiệu nổi
    OutlinedText("SHOP", (int)p.x - MeasureText("SHOP", 20) / 2, (int)(p.y - 68.0f + bob), 20, GOLD, { 70, 36, 8, 255 });
    if (Vector2Distance(p, player.pos) < 130.0f) {
        const char* prompt = touchMode ? "" : (IsGamepadAvailable(0) ? "[D-pad Up] Trade" : "[F] Trade");
        OutlinedText(prompt, (int)p.x - MeasureText(prompt, 20) / 2, (int)p.y + 40, 20, WHITE, { 20, 24, 40, 255 });
    }
}

inline void Game::DrawItemIcon(int slot, Vector2 c, float s, Color col, int weaponType) {
    Color dark = Shade(col, 0.45f);
    switch (slot) {
        case SLOT_WEAPON: {
            if (weaponType == WP_DAGGER) {
                Vector2 a = { c.x - s * 0.28f, c.y + s * 0.28f }, b = { c.x + s * 0.34f, c.y - s * 0.34f };
                DrawLineEx(a, b, s * 0.16f, dark);
                DrawLineEx(a, b, s * 0.09f, col);
                DrawLineEx({ c.x - s * 0.28f, c.y + s * 0.04f }, { c.x - s * 0.04f, c.y + s * 0.28f }, s * 0.10f, GOLD);
            } else if (weaponType == WP_AXE) {
                Vector2 a = { c.x - s * 0.42f, c.y + s * 0.42f }, b = { c.x + s * 0.36f, c.y - s * 0.36f };
                DrawLineEx(a, b, s * 0.13f, { 120, 80, 48, 255 });
                Tri({ c.x + s * 0.04f, c.y - s * 0.18f }, { c.x + s * 0.44f, c.y - s * 0.44f }, { c.x + s * 0.38f, c.y + s * 0.08f }, col);
                Tri({ c.x + s * 0.16f, c.y - s * 0.04f }, { c.x - s * 0.12f, c.y - s * 0.42f }, { c.x + s * 0.42f, c.y - s * 0.44f }, Shade(col, 0.8f));
            } else if (weaponType == WP_SPEAR) {
                Vector2 a = { c.x - s * 0.46f, c.y + s * 0.46f }, b = { c.x + s * 0.26f, c.y - s * 0.26f };
                DrawLineEx(a, b, s * 0.11f, { 130, 88, 50, 255 });
                Tri({ c.x + s * 0.50f, c.y - s * 0.50f }, { c.x + s * 0.10f, c.y - s * 0.28f }, { c.x + s * 0.28f, c.y - s * 0.10f }, col);
            } else if (weaponType == WP_HAMMER) {
                Vector2 a = { c.x - s * 0.40f, c.y + s * 0.40f }, b = { c.x + s * 0.26f, c.y - s * 0.26f };
                DrawLineEx(a, b, s * 0.14f, { 125, 82, 46, 255 });
                DrawLineEx({ b.x - s * 0.24f, b.y - s * 0.24f }, { b.x + s * 0.24f, b.y + s * 0.24f }, s * 0.26f, dark);
                DrawLineEx({ b.x - s * 0.20f, b.y - s * 0.20f }, { b.x + s * 0.20f, b.y + s * 0.20f }, s * 0.18f, col);
            } else if (weaponType == WP_SCYTHE) {
                Vector2 a = { c.x - s * 0.42f, c.y + s * 0.44f }, b = { c.x + s * 0.30f, c.y - s * 0.34f };
                DrawLineEx(a, b, s * 0.11f, { 88, 72, 118, 255 });
                Tri(b, { c.x + s * 0.48f, c.y - s * 0.08f }, { c.x + s * 0.24f, c.y + s * 0.34f }, col);
                DrawLineEx(b, { c.x + s * 0.24f, c.y + s * 0.34f }, s * 0.06f, WHITE);
            } else if (weaponType == WP_BOW) {
                DrawRing({ c.x - s * 0.05f, c.y + s * 0.05f }, s * 0.34f, s * 0.44f, -120.0f, 30.0f, 16, { 165, 110, 56, 255 });
                DrawLineEx({ c.x - s * 0.24f, c.y - s * 0.30f }, { c.x + s * 0.30f, c.y + s * 0.24f }, 1.5f, WHITE);
                DrawLineEx({ c.x - s * 0.20f, c.y + s * 0.20f }, { c.x + s * 0.42f, c.y - s * 0.42f }, s * 0.09f, col);
            } else if (weaponType == WP_STAFF) {
                Vector2 a = { c.x - s * 0.44f, c.y + s * 0.44f }, b = { c.x + s * 0.22f, c.y - s * 0.22f };
                DrawLineEx(a, b, s * 0.12f, { 110, 70, 155, 255 });
                DrawCircleV({ c.x + s * 0.30f, c.y - s * 0.30f }, s * 0.20f, Fade(col, 0.45f));
                DrawCircleV({ c.x + s * 0.30f, c.y - s * 0.30f }, s * 0.13f, col);
                DrawCircleV({ c.x + s * 0.27f, c.y - s * 0.33f }, s * 0.05f, WHITE);
            } else {
                Vector2 a = { c.x - s * 0.42f, c.y + s * 0.42f }, b = { c.x + s * 0.46f, c.y - s * 0.46f };
                DrawLineEx(a, b, s * 0.20f, dark);
                DrawLineEx(a, b, s * 0.12f, col);
                DrawLineEx({ c.x - s * 0.12f, c.y - s * 0.12f }, { c.x + s * 0.05f, c.y + s * 0.05f }, 1.0f, WHITE);
                Vector2 g = { c.x - s * 0.18f, c.y + s * 0.18f };
                DrawLineEx({ g.x - s * 0.2f, g.y - s * 0.2f }, { g.x + s * 0.2f, g.y + s * 0.2f }, s * 0.12f, GOLD);
                DrawCircleV({ c.x - s * 0.48f, c.y + s * 0.48f }, s * 0.10f, GOLD);
            }
            break;
        }
        case SLOT_ARMOR: {
            DrawRectangle((int)(c.x - s * 0.46f), (int)(c.y - s * 0.50f), (int)(s * 0.92f), (int)(s * 0.55f), dark);
            DrawCircleSector({ c.x, c.y + s * 0.05f }, s * 0.46f, 0.0f, 180.0f, 20, dark);
            DrawRectangle((int)(c.x - s * 0.38f), (int)(c.y - s * 0.43f), (int)(s * 0.76f), (int)(s * 0.48f), col);
            DrawCircleSector({ c.x, c.y + s * 0.05f }, s * 0.38f, 0.0f, 180.0f, 20, col);
            DrawLineEx({ c.x, c.y - s * 0.35f }, { c.x, c.y + s * 0.34f }, s * 0.07f, Fade(WHITE, 0.55f));
            break;
        }
        case SLOT_BOOTS: {
            DrawRectangle((int)(c.x - s * 0.34f), (int)(c.y - s * 0.50f), (int)(s * 0.40f), (int)(s * 0.80f), dark);
            DrawRectangle((int)(c.x - s * 0.34f), (int)(c.y + s * 0.10f), (int)(s * 0.80f), (int)(s * 0.30f), dark);
            DrawRectangle((int)(c.x - s * 0.28f), (int)(c.y - s * 0.44f), (int)(s * 0.28f), (int)(s * 0.68f), col);
            DrawRectangle((int)(c.x - s * 0.28f), (int)(c.y + s * 0.14f), (int)(s * 0.70f), (int)(s * 0.22f), col);
            DrawRectangle((int)(c.x - s * 0.34f), (int)(c.y - s * 0.50f), (int)(s * 0.40f), (int)(s * 0.10f), GOLD);
            break;
        }
        default: {
            DrawRing(c, s * 0.26f, s * 0.46f, 0.0f, 360.0f, 28, dark);
            DrawRing(c, s * 0.30f, s * 0.42f, 0.0f, 360.0f, 28, col);
            DrawCircleV({ c.x, c.y - s * 0.46f }, s * 0.16f, Shade(col, 1.4f));
            DrawCircleV({ c.x - s * 0.04f, c.y - s * 0.50f }, s * 0.05f, WHITE);
            break;
        }
    }
}

inline void Game::DrawItemDrops() {
    float t = (float)GetTime();
    for (const auto& d : itemDrops) {
        Color rc = d.item.RarityColor();
        float bob = sinf(t * 3.0f + d.phase) * 4.0f;
        DrawEllipse((int)d.pos.x, (int)d.pos.y + 14, 14.0f, 5.0f, Fade(BLACK, 0.35f));
        BeginBlendMode(BLEND_ADDITIVE);
        CircleGrad(d.pos.x, d.pos.y - 6, 46.0f, Fade(rc, 0.45f), Fade(rc, 0.0f));
        EndBlendMode();
        if (d.item.rarity >= 2)
            DrawRectangleGradientV((int)d.pos.x - 5, (int)d.pos.y - 100, 10, 100, Fade(rc, 0.0f), Fade(rc, d.item.rarity == 3 ? 0.45f : 0.28f));
        DrawCircleV({ d.pos.x, d.pos.y - 8.0f + bob }, 17.0f, { 18, 22, 34, 230 });
        DrawCircleLines((int)d.pos.x, (int)(d.pos.y - 8.0f + bob), 17.0f, rc);
        DrawItemIcon(d.item.slot, { d.pos.x, d.pos.y - 8.0f + bob }, 22.0f, rc, d.item.weaponType);
        if (d.life < 8.0f && ((int)(t * 6.0f)) % 2 == 0) DrawCircleLines((int)d.pos.x, (int)(d.pos.y - 8.0f + bob), 21.0f, WHITE); // sắp biến mất
        if (Vector2Distance(d.pos, player.pos) < 130.0f) {
            std::string title = d.item.name;
            int tw = MeasureText(title.c_str(), 16);
            int ty = (int)(d.pos.y - 62.0f);
            auto lines = d.item.StatLines();
            int boxH = 24 + (int)lines.size() * 16;
            int boxW = std::max(tw, 120) + 20;
            DrawRectangle((int)d.pos.x - boxW / 2, ty - boxH + 18, boxW, boxH, { 10, 14, 24, 230 });
            DrawRectangleLines((int)d.pos.x - boxW / 2, ty - boxH + 18, boxW, boxH, rc);
            DrawText(title.c_str(), (int)d.pos.x - tw / 2, ty - boxH + 24, 16, rc);
            for (size_t i = 0; i < lines.size(); ++i)
                DrawText(lines[i].c_str(), (int)d.pos.x - boxW / 2 + 10, ty - boxH + 44 + (int)i * 16, 14, RAYWHITE);
        }
    }
}

inline void Game::DrawSurfacePrompt(Vector2 p, const char* title, const char* sub, Color accent, bool near) {
    int tw = MeasureText(title, 15);
    int sw = (sub && sub[0]) ? MeasureText(sub, 13) : 0;
    int bw = std::max(tw, sw) + 28;
    int bh = (sw > 0) ? 46 : 26;
    int bx = (int)p.x - bw / 2;
    int by = (int)p.y;
    DrawRectangle(bx, by, bw, bh, near ? Color{ 8, 12, 22, 252 } : Color{ 8, 12, 22, 248 });
    DrawRectangleLinesEx({ (float)bx, (float)by, (float)bw, (float)bh }, 1.6f, near ? GOLD : Fade(accent, 0.88f));
    DrawText(title, (int)p.x - tw / 2, by + 5, 15, near ? GOLD : accent);
    if (sw > 0) DrawText(sub, (int)p.x - sw / 2, by + 24, 13, near ? WHITE : Color{ 225, 234, 250, 255 });
}

inline void Game::DrawDetailedNpcModel(Vector2 pos, int npcKind, Color primaryCol, Color secondaryCol, Color hairCol, bool crown, float bobPhase) {
    float tm = (float)GetTime();
    float bob = sinf(tm * 3.0f + bobPhase) * 1.5f;
    float sway = cosf(tm * 2.2f + bobPhase) * 1.5f;
    const Color ink = { 22, 18, 28, 255 };
    const Color skin = { 255, 224, 194, 255 };

    // Bóng chân & vòng hào quang nhẹ
    DrawEllipse((int)pos.x, (int)pos.y + 17, 16, 6, Fade(BLACK, 0.40f));

    // Đôi chân / ủng nhỏ thò dưới gấu áo
    BoxRot({ pos.x - 4.5f, pos.y + 13.0f }, 5.0f, 7.0f, 0.0f, { 68, 44, 34, 255 }, ink, 1.2f);
    BoxRot({ pos.x + 4.5f, pos.y + 13.0f }, 5.0f, 7.0f, 0.0f, { 68, 44, 34, 255 }, ink, 1.2f);

    // Áo choàng lưng (Vua / Thánh Nữ / Công Chúa)
    if (npcKind == 2 || npcKind == 3 || npcKind == 4) {
        Tri({ pos.x - 13.0f, pos.y + 13.0f + bob }, { pos.x + 13.0f, pos.y + 13.0f + bob }, { pos.x, pos.y - 10.0f + bob }, Shade(primaryCol, 0.65f));
    }

    // Thân váy / Áo choàng nhiều lớp + thắt lưng & tạp dề
    QuadOut({ pos.x - 7.5f, pos.y - 6.0f + bob }, { pos.x + 7.5f, pos.y - 6.0f + bob },
            { pos.x + 12.5f, pos.y + 14.0f + bob }, { pos.x - 12.5f, pos.y + 14.0f + bob },
            primaryCol, ink, 1.6f);
    // Vạt áo giữa / tạp dề / dải lụa trang trí
    QuadOut({ pos.x - 3.5f, pos.y - 5.0f + bob }, { pos.x + 3.5f, pos.y - 5.0f + bob },
            { pos.x + 5.5f, pos.y + 14.0f + bob }, { pos.x - 5.5f, pos.y + 14.0f + bob },
            npcKind == 5 ? RAYWHITE : secondaryCol, ink, 1.0f);
    // Thắt lưng vàng / đai lụa
    DrawRectangle((int)(pos.x - 8), (int)(pos.y + 1 + bob), 16, 4, npcKind == 4 ? GOLD : secondaryCol);
    // Cổ áo lông thú (Vua) hoặc cổ sen (Vợ)
    DrawEllipse((int)pos.x, (int)(pos.y - 6.0f + bob), 9.0f, 3.5f, npcKind == 4 ? RAYWHITE : Shade(secondaryCol, 1.1f));

    // Hai tay cử động + Đạo cụ đặc trưng từng NPC
    Vector2 lh = { pos.x - 10.5f, pos.y + 2.0f + bob + sway * 0.5f };
    Vector2 rh = { pos.x + 11.0f, pos.y + 1.0f + bob - sway * 0.5f };
    DrawLineEx({ pos.x - 6.5f, pos.y - 4.0f + bob }, lh, 3.6f, Shade(primaryCol, 0.85f));
    DrawLineEx({ pos.x + 6.5f, pos.y - 4.0f + bob }, rh, 3.6f, Shade(primaryCol, 0.85f));
    DrawCircleV(lh, 2.8f, skin);
    DrawCircleV(rh, 2.8f, skin);

    if (npcKind == 0) { // Mai Linh: giỏ hoa tươi nở
        DrawCircleV({ rh.x + 2.0f, rh.y + 2.0f }, 5.0f, { 165, 110, 62, 255 });
        DrawCircleV({ rh.x + 1.0f, rh.y - 1.5f }, 2.4f, { 255, 130, 185, 255 });
        DrawCircleV({ rh.x + 4.0f, rh.y - 1.0f }, 2.2f, GOLD);
    } else if (npcKind == 1) { // Vân Anh: quạt lụa vàng
        Tri(rh, { rh.x + 9.0f, rh.y - 6.0f }, { rh.x + 9.0f, rh.y + 3.0f }, GOLD);
    } else if (npcKind == 2 || npcKind == 3 || npcKind == 4) { // Quyền trượng Thánh Nữ / Hoàng Gia / Vua
        DrawLineEx({ rh.x, rh.y + 9.0f }, { rh.x, rh.y - 14.0f }, 2.4f, { 145, 98, 48, 255 });
        Diamond({ rh.x, rh.y - 16.0f }, 4.5f, 5.5f, npcKind == 2 ? SKYBLUE : GOLD, WHITE);
    } else if (npcKind == 5) { // Chủ quán: cốc bia bọt tràn
        DrawRectangle((int)rh.x - 2, (int)rh.y - 7, 7, 9, { 138, 86, 42, 255 });
        DrawCircleV({ rh.x + 1.5f, rh.y - 8.0f }, 3.2f, RAYWHITE);
    } else { // Kiến trúc sư: cuộn bản vẽ
        BoxRot({ rh.x + 2.0f, rh.y - 2.0f }, 5.0f, 12.0f, 0.3f, SKYBLUE, WHITE, 1.0f);
    }

    // Đầu, tóc sau, khuôn mặt có mắt & má hồng, tóc mái & vương miện/hoa
    Vector2 head = { pos.x, pos.y - 13.5f + bob };
    if (npcKind <= 3) { // Tóc dài xõa vai cho 4 nàng dâu
        DrawCircleV({ head.x, head.y + 2.0f }, 9.5f, Shade(hairCol, 0.82f));
    }
    DrawCircleV(head, 7.6f, skin);
    DrawCircleLines((int)head.x, (int)head.y, 7.6f, ink);
    DrawCircleV({ head.x - 2.6f, head.y - 0.5f }, 1.2f, ink);
    DrawCircleV({ head.x + 2.6f, head.y - 0.5f }, 1.2f, ink);
    if (npcKind <= 3) {
        DrawCircleV({ head.x - 4.2f, head.y + 2.0f }, 1.5f, Fade({ 255, 110, 145, 255 }, 0.65f));
        DrawCircleV({ head.x + 4.2f, head.y + 2.0f }, 1.5f, Fade({ 255, 110, 145, 255 }, 0.65f));
    } else if (npcKind == 4) { // Râu bạc Đức Vua
        Tri({ head.x - 4.5f, head.y + 3.0f }, { head.x + 4.5f, head.y + 3.0f }, { head.x, head.y + 9.5f }, RAYWHITE);
    }
    DrawRing({ head.x, head.y - 1.5f }, 3.5f, 8.2f, 182.0f, 358.0f, 12, hairCol);
    if (crown) {
        Tri({ head.x - 5.5f, head.y - 7.5f }, { head.x, head.y - 7.5f }, { head.x - 3.5f, head.y - 13.5f }, GOLD);
        Tri({ head.x - 1.0f, head.y - 7.5f }, { head.x + 5.5f, head.y - 7.5f }, { head.x + 3.5f, head.y - 13.5f }, GOLD);
        Diamond({ head.x, head.y - 11.5f }, 3.8f, 4.5f, npcKind == 0 ? PINK : SKYBLUE, WHITE);
    }
}

inline void Game::DrawSurfaceExitDoor(Vector2 pos) {
    bool near = Vector2Distance(player.pos, pos) < 95.0f;
    DrawRectangle((int)pos.x - 42, (int)pos.y - 24, 84, 52, { 54, 42, 34, 255 });
    DrawRectangle((int)pos.x - 32, (int)pos.y - 16, 64, 40, near ? Color{ 62, 118, 165, 240 } : Color{ 34, 48, 72, 235 });
    DrawRectangleLines((int)pos.x - 34, (int)pos.y - 18, 68, 44, near ? GOLD : Color{ 165, 145, 105, 255 });
    DrawRectangle((int)pos.x - 36, (int)pos.y - 28, 72, 10, { 125, 116, 104, 255 });
    DrawSurfacePrompt({ pos.x, pos.y - 62.0f }, L("TOWN SQUARE EXIT", "CỬA RA QUẢNG TRƯỜNG"),
                      L("[F] / Walk Out   [0] Quick Exit", "[F] hoặc Đi Ra   [0] Ra Nhanh"), SKYBLUE, near);
}

inline void Game::DrawMiniHouseModel(Vector2 p, int tier, Color roofCol, Color trimCol) {
    float sc = 0.78f + tier * 0.14f;
    DrawEllipse((int)p.x, (int)p.y + 18, 36.0f * sc, 11.0f * sc, Fade(BLACK, 0.35f));
    DrawRectangle((int)(p.x - 24 * sc), (int)(p.y - 8 * sc), (int)(48 * sc), (int)(26 * sc), { 225, 212, 188, 255 });
    DrawRectangleLines((int)(p.x - 24 * sc), (int)(p.y - 8 * sc), (int)(48 * sc), (int)(26 * sc), { 35, 30, 38, 255 });
    Tri({ p.x - 30.0f * sc, p.y - 7.0f * sc }, { p.x + 30.0f * sc, p.y - 7.0f * sc }, { p.x, p.y - 31.0f * sc }, roofCol);
    DrawRectangle((int)(p.x - 6 * sc), (int)(p.y + 4 * sc), (int)(12 * sc), (int)(14 * sc), { 102, 58, 34, 255 });
    DrawRectangle((int)(p.x - 18 * sc), (int)(p.y - 1 * sc), (int)(8 * sc), (int)(8 * sc), { 255, 215, 115, 255 });
    DrawRectangle((int)(p.x + 10 * sc), (int)(p.y - 1 * sc), (int)(8 * sc), (int)(8 * sc), { 255, 215, 115, 255 });
    if (tier >= 3) Diamond({ p.x, p.y - 34.0f * sc }, 5.5f, 5.5f, GOLD, WHITE);
    DrawCircleLines((int)p.x, (int)p.y + 12, 34.0f, Fade(trimCol, 0.45f));
}
