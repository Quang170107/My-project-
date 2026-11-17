#pragma once
#include "../game.hpp"

inline void Game::BuildDecor() {
    torches.clear();
    decals.clear();
    float hw = roomW / 2.0f, hh = roomH / 2.0f;
    for (float x = -hw + 160.0f; x < hw - 100.0f; x += 330.0f) {
        torches.push_back({ x, -hh - 22.0f });
        torches.push_back({ x + 165.0f, hh + 22.0f });
    }
    for (float y = -hh + 190.0f; y < hh - 100.0f; y += 330.0f) {
        torches.push_back({ -hw - 22.0f, y });
        torches.push_back({ hw + 22.0f, y + 120.0f });
    }
    for (int i = 0; i < 52; ++i) {
        Decal d;
        d.pos = { (Rand01() - 0.5f) * (roomW - 96.0f), (Rand01() - 0.5f) * (roomH - 96.0f) };
        if (Vector2Length(d.pos) < 92.0f) d.pos.x += (d.pos.x >= 0 ? 110.0f : -110.0f);
        d.angle = Rand01() * 360.0f;
        d.size = 11.0f + 24.0f * Rand01();
        float r = Rand01();
        d.type = r < 0.25f ? 0 : (r < 0.45f ? 1 : (r < 0.65f ? 2 : 3));
        decals.push_back(d);
    }
}

inline void Game::DrawWalls(const BiomeColors& biome) {
    float hw = roomW / 2.0f, hh = roomH / 2.0f;
    const float th = 44.0f, bw = 48.0f, bh = 22.0f;
    DrawRectangle((int)(-hw - th), (int)(-hh - th), (int)(roomW + th * 2), (int)(roomH + th * 2), Shade(biome.wall, 0.45f));
    auto bricks = [&](float x0, float y0, float w, float h) {
        int row = 0;
        for (float y = y0; y < y0 + h - 0.1f; y += bh, ++row) {
            float offset = (row & 1) ? bw * 0.5f : 0.0f;
            for (float x = x0 - offset; x < x0 + w; x += bw) {
                int ix = (int)floorf(x / bw), iy = row + (int)y0;
                int hsh = ((ix * 73856093) ^ (iy * 19349663)) & 7;
                Color bc = Shade(biome.wall, 0.60f + 0.05f * hsh);
                float bx = std::max(x, x0), ex = std::min(x + bw, x0 + w);
                if (ex <= bx) continue;
                DrawRectangle((int)bx, (int)y, (int)(ex - bx) - 1, (int)bh - 1, bc);
                DrawRectangle((int)bx, (int)y, (int)(ex - bx) - 1, 2, Shade(bc, 1.3f));
            }
        }
    };
    bricks(-hw - th, -hh - th, roomW + th * 2, th);
    bricks(-hw - th, hh, roomW + th * 2, th);
    bricks(-hw - th, -hh, th, roomH);
    bricks(hw, -hh, th, roomH);
}

inline void Game::DrawDecals(const BiomeColors& biome) {
    if (inSurface) {
        float tm = (float)GetTime();
        for (const auto& d : decals) {
            Vector2 p = d.pos;
            float a = d.angle * DEG2RAD, s = d.size;
            if (surfaceRoom == 0 && (fabsf(p.x) < 110.0f || fabsf(p.y) < 110.0f)) continue;
            if (d.type == 0) {
                Vector2 p1 = { p.x + cosf(a) * s * 0.8f, p.y + sinf(a) * s * 0.8f };
                DrawLineEx(p, p1, 1.8f, Fade(BLACK, 0.20f));
            } else if (d.type == 1) {
                Color pc = (surfaceRoom == 0) ? Color{ 58, 148, 72, 90 }
                         : (surfaceRoom == 1 ? Fade(biome.accentCol, 0.14f) : Fade(biome.trimCol, 0.16f));
                DrawEllipse((int)p.x, (int)p.y, s * 1.15f, s * 0.68f, pc);
                if (surfaceRoom >= 2) DrawEllipseLines((int)p.x, (int)p.y, s * 1.15f, s * 0.68f, Fade(biome.trimCol, 0.32f));
            } else if (d.type == 2) {
                for (int i = 0; i < 3; ++i) {
                    Vector2 q = { p.x + cosf(a + i * 2.1f) * s * 0.45f, p.y + sinf(a + i * 2.1f) * s * 0.32f };
                    DrawCircleV(q, 2.5f + i * 0.7f, (surfaceRoom == 0 && i == 1) ? GOLD : Shade(biome.tileA, 1.38f));
                }
            } else {
                if (surfaceRoom == 0) { // Hoa & bụi cỏ nắng mai
                    for (int i = 0; i < 4; ++i) {
                        float aa = a + i * 1.57f;
                        DrawEllipse((int)(p.x + cosf(aa) * s * 0.30f), (int)(p.y + sinf(aa) * s * 0.30f), s * 0.28f, s * 0.15f, { 62, 165, 82, 200 });
                    }
                    float sway = sinf(tm * 2.2f + p.x) * 1.4f;
                    Diamond({ p.x + sway, p.y - 2.0f }, 4.5f, 4.5f, (int)d.angle & 1 ? Color{ 255, 130, 175, 255 } : GOLD, WHITE);
                } else if (surfaceRoom == 1) { // Đốm lửa ấm & gỗ khảm
                    Diamond(p, s * 0.28f, s * 0.22f, Fade(biome.trimCol, 0.45f), Fade(GOLD, 0.6f));
                } else if (surfaceRoom == 2) { // Dấu la bàn kiến trúc
                    DrawCircleLines((int)p.x, (int)p.y, s * 0.55f, Fade(SKYBLUE, 0.38f));
                    Diamond(p, 4.0f, 4.0f, SKYBLUE, WHITE);
                } else if (surfaceRoom == 3) { // Cánh hoa & huy hiệu gia tộc
                    Diamond(p, 5.0f, 3.5f, hasSpouse ? kSpouses[std::max(0, spouseIdx)].dressCol : GOLD, Fade(WHITE, 0.7f));
                } else { // Hoa văn hoàng kim đại điện
                    DrawCircleLines((int)p.x, (int)p.y, s * 0.45f, Fade(GOLD, 0.50f));
                    Diamond(p, 4.5f, 4.5f, GOLD, WHITE);
                }
            }
        }
        return;
    }
    int bi = BiomeIdx();
    float tm = (float)GetTime();
    for (const auto& d : decals) {
        Vector2 p = d.pos;
        float a = d.angle * DEG2RAD, s = d.size;
        if (d.type == 0) { // vết nứt / mạch năng lượng
            Vector2 p1 = { p.x + cosf(a) * s, p.y + sinf(a) * s };
            Vector2 p2 = { p1.x + cosf(a + 0.7f) * s * 0.7f, p1.y + sinf(a + 0.7f) * s * 0.7f };
            Vector2 p0 = { p.x - cosf(a - 0.4f) * s * 0.8f, p.y - sinf(a - 0.4f) * s * 0.8f };
            Color cc = (bi == 2) ? Color{ 255, 130, 45, 220 } : (bi == 5 ? Fade(biome.trimCol, 0.50f) : Fade(BLACK, 0.38f));
            DrawLineEx(p0, p, 2.0f, cc);
            DrawLineEx(p, p1, 2.0f, cc);
            DrawLineEx(p1, p2, 1.5f, cc);
        } else if (d.type == 1) { // thảm cỏ, vệt dầu, vệt rune
            Color pc = (bi == 1) ? Color{ 55, 155, 72, 95 }
                     : (bi == 3 ? Color{ 165, 225, 255, 55 }
                     : (bi == 4 ? Color{ 35, 175, 185, 95 }
                     : (bi == 5 ? Fade(biome.trimCol, 0.22f) : Fade(BLACK, 0.20f))));
            DrawEllipse((int)p.x, (int)p.y, s * 1.25f, s * 0.75f, pc);
            if (bi == 4 || bi == 5) DrawEllipseLines((int)p.x, (int)p.y, s * 1.25f, s * 0.75f, Fade(biome.accentCol, 0.42f));
        } else if (d.type == 2) { // cụm sỏi / tinh thể nhỏ / cánh hoa
            for (int i = 0; i < 3; ++i) {
                Vector2 q = { p.x + cosf(a + i * 2.1f) * s * 0.5f, p.y + sinf(a + i * 2.1f) * s * 0.35f };
                DrawCircleV({ q.x, q.y + 1.5f }, 3.0f + i, Fade(BLACK, 0.28f));
                Color gc = (bi == 1 && i == 1) ? biome.accentCol : (bi == 3 ? biome.trimCol : Shade(biome.tileA, 1.55f));
                DrawCircleV(q, 2.6f + i, gc);
            }
        } else { // phong cảnh đặc trưng cho từng Biome
            if (bi == 0) { // Royal Crypt: ấn ký cổ & xương chiến binh
                DrawCircleLines((int)p.x, (int)p.y, s * 0.75f, Fade(biome.accentCol, 0.45f));
                Diamond(p, s * 0.30f, s * 0.30f, Fade(biome.trimCol, 0.70f), Fade(biome.accentCol, 0.85f));
                Vector2 d1 = { cosf(a) * s * 0.55f, sinf(a) * s * 0.55f };
                Color bone = { 218, 212, 196, 255 };
                DrawLineEx({ p.x - d1.x, p.y - d1.y }, { p.x + d1.x, p.y + d1.y }, 2.6f, bone);
                DrawCircleV({ p.x + d1.x, p.y + d1.y }, 2.4f, bone);
            } else if (bi == 1) { // Emerald Grove: bụi dương xỉ & hoa pha lê nở rực rỡ
                for (int i = 0; i < 5; ++i) {
                    float aa = a + i * 1.25f;
                    DrawEllipse((int)(p.x + cosf(aa) * s * 0.35f), (int)(p.y + sinf(aa) * s * 0.35f), s * 0.34f, s * 0.18f, { 55, 165, 78, 215 });
                }
                float sway = sinf(tm * 2.5f + p.x) * 1.5f;
                Diamond({ p.x + sway, p.y - 2.0f }, 5.5f, 5.5f, biome.accentCol, WHITE);
                DrawCircleV({ p.x + sway, p.y - 2.0f }, 2.2f, GOLD);
            } else if (bi == 2) { // Molten Forge: hồ dung nham sôi & vỉ sắt đồng
                DrawEllipse((int)p.x, (int)p.y, s * 0.95f, s * 0.55f, Fade({ 255, 85, 20, 255 }, 0.48f));
                DrawEllipse((int)p.x, (int)p.y, s * 0.52f, s * 0.28f, Fade({ 255, 215, 75, 255 }, 0.65f));
                DrawEllipseLines((int)p.x, (int)p.y, s * 0.95f, s * 0.55f, { 110, 75, 50, 200 });
            } else if (bi == 3) { // Crystal Frost: cụm tinh thể băng & hoa tuyết
                for (int i = 0; i < 3; ++i) {
                    float aa = a + i * 2.1f;
                    Tri({ p.x, p.y }, { p.x + cosf(aa) * s * 0.72f, p.y + sinf(aa) * s * 0.72f },
                        { p.x + cosf(aa + 0.38f) * s * 0.45f, p.y + sinf(aa + 0.38f) * s * 0.45f },
                        i == 0 ? Fade(biome.accentCol, 0.75f) : Fade({ 185, 238, 255, 255 }, 0.72f));
                }
                Diamond(p, 4.0f, 6.0f, WHITE, biome.trimCol);
            } else if (bi == 4) { // Golden Sun Temple: hồ nước ngọc ốc đảo & biểu tượng mặt trời vàng
                DrawEllipse((int)p.x, (int)p.y, s * 1.05f, s * 0.62f, Fade({ 35, 195, 205, 255 }, 0.42f));
                DrawEllipseLines((int)p.x, (int)p.y, s * (0.65f + 0.12f * sinf(tm * 3.0f + p.x)), s * 0.38f, Fade({ 180, 255, 245, 255 }, 0.55f));
                DrawCircleLines((int)p.x, (int)p.y, s * 0.36f, Fade(GOLD, 0.75f));
                DrawCircleV(p, 3.0f, GOLD);
            } else { // Astral Void: chòm sao phát sáng & mảnh tinh tú
                Vector2 p1 = { p.x + cosf(a) * s * 0.8f, p.y + sinf(a) * s * 0.8f };
                Vector2 p2 = { p.x + cosf(a + 2.2f) * s * 0.75f, p.y + sinf(a + 2.2f) * s * 0.75f };
                DrawLineEx(p, p1, 1.6f, Fade(biome.accentCol, 0.55f));
                DrawLineEx(p1, p2, 1.4f, Fade(biome.trimCol, 0.50f));
                Diamond(p, 4.5f, 4.5f, biome.trimCol, WHITE);
                Diamond(p1, 3.5f, 3.5f, biome.accentCol, WHITE);
                Diamond(p2, 3.0f, 3.0f, WHITE, biome.trimCol);
            }
        }
    }
}

inline void Game::DrawPillar(Vector2 p, const BiomeColors& biome) {
    int bi = BiomeIdx();
    Color stone = Shade(biome.wall, 1.22f);
    Color ink = Shade(stone, 0.32f);
    DrawEllipse((int)p.x + 3, (int)p.y + 15, 32.0f, 13.0f, Fade(BLACK, 0.42f));
    // Chân cột đá vuông vát cạnh + thân bát giác + đỉnh cột chạm khắc 3D
    BoxRot({ p.x, p.y + 4.0f }, 52.0f, 44.0f, 0.0f, Shade(stone, 0.60f), ink, 1.8f);
    DrawPoly({ p.x, p.y - 2.0f }, 8, 24.5f, 22.5f, Shade(stone, 0.82f));
    DrawPolyLinesEx({ p.x, p.y - 2.0f }, 8, 24.5f, 22.5f, 1.8f, ink);
    BoxRot({ p.x, p.y - 10.0f }, 42.0f, 36.0f, 0.0f, stone, ink, 1.8f);
    BoxRot({ p.x, p.y - 11.0f }, 26.0f, 22.0f, 0.0f, Shade(stone, 1.24f), biome.trimCol, 1.4f);
    DrawLineEx({ p.x - 20.0f, p.y - 28.0f }, { p.x + 20.0f, p.y - 28.0f }, 2.0f, Fade(WHITE, 0.25f));
    if (inSurface) {
        if (surfaceRoom == 0) {
            Tri({ p.x + 6.0f, p.y + 8.0f }, { p.x + 21.0f, p.y + 6.0f }, { p.x + 15.0f, p.y + 19.0f }, Fade({ 78, 188, 92, 255 }, 0.85f));
            Diamond({ p.x, p.y - 11.0f }, 5.5f, 5.5f, GOLD, WHITE);
        } else if (surfaceRoom == 1) {
            DrawCircleV({ p.x, p.y - 11.0f }, 6.0f, ORANGE);
            DrawCircleV({ p.x, p.y - 11.0f }, 3.0f, GOLD);
        } else if (surfaceRoom == 2) {
            Diamond({ p.x, p.y - 11.0f }, 6.0f, 6.0f, SKYBLUE, WHITE);
        } else if (surfaceRoom == 3) {
            Diamond({ p.x, p.y - 11.0f }, 5.5f, 6.5f, hasSpouse ? Color{ 255, 130, 180, 255 } : GOLD, WHITE);
        } else {
            DrawCircleV({ p.x, p.y - 11.0f }, 6.5f, GOLD);
            Diamond({ p.x, p.y - 11.0f }, 4.5f, 4.5f, RED, WHITE);
        }
        return;
    }
    if (bi == 0) {
        Diamond({ p.x, p.y - 11.0f }, 5.0f, 5.0f, biome.trimCol, WHITE);
    } else if (bi == 1) { // rêu & hoa trên cột
        Tri({ p.x + 6.0f, p.y + 8.0f }, { p.x + 22.0f, p.y + 6.0f }, { p.x + 17.0f, p.y + 20.0f }, Fade({ 85, 195, 88, 255 }, 0.82f));
        Diamond({ p.x + 15.0f, p.y + 11.0f }, 3.2f, 3.2f, biome.accentCol, WHITE);
    } else if (bi == 2) { // mạch dung nham nứt
        DrawLineEx({ p.x - 10.0f, p.y + 5.0f }, { p.x + 3.0f, p.y + 16.0f }, 2.5f, { 255, 130, 40, 255 });
    } else if (bi == 3) { // mảnh băng trên đỉnh cột
        Tri({ p.x - 14.0f, p.y - 18.0f }, { p.x - 2.0f, p.y - 26.0f }, { p.x + 10.0f, p.y - 16.0f }, Fade({ 220, 244, 255, 255 }, 0.82f));
    } else if (bi == 4) { // biểu tượng mặt trời vàng
        DrawCircleV({ p.x, p.y - 11.0f }, 6.0f, GOLD);
        DrawCircleV({ p.x, p.y - 11.0f }, 2.8f, biome.accentCol);
    } else { // tinh thể tím phát sáng
        Diamond({ p.x, p.y - 11.0f }, 5.5f, 7.0f, biome.trimCol, biome.accentCol);
    }
}

inline void Game::DrawTorch(Vector2 t) {
    float tm = (float)GetTime();
    float fl = 0.78f + 0.22f * sinf(tm * 13.0f + t.x * 0.1f) + 0.08f * sinf(tm * 29.0f + t.y);
    DrawRectangle((int)t.x - 3, (int)t.y - 2, 6, 15, { 70, 50, 36, 255 });
    DrawRectangle((int)t.x - 6, (int)t.y - 4, 12, 5, { 120, 88, 54, 255 });
    BeginBlendMode(BLEND_ADDITIVE);
    CircleGrad(t.x, t.y - 8, 58.0f, Fade({ 255, 150, 50, 255 }, 0.32f * fl), Fade({ 255, 150, 50, 255 }, 0.0f));
    EndBlendMode();
    DrawCircleV({ t.x, t.y - 8.0f }, 7.5f * fl, { 255, 120, 30, 255 });
    DrawCircleV({ t.x, t.y - 9.0f }, 4.8f * fl, { 255, 200, 70, 255 });
    DrawCircleV({ t.x, t.y - 10.0f }, 2.2f, { 255, 250, 205, 255 });
}

inline void Game::AddLight(Vector2 wp, float radius, Color col, float inten) {
    Vector2 sp = GetWorldToScreen2D(wp, camera);
    if (sp.x < -radius || sp.x > 1280 + radius || sp.y < -radius || sp.y > 720 + radius) return;
    CircleGrad(sp.x, sp.y, radius, Fade(col, inten), Fade(col, 0.0f));
}

inline void Game::RenderLightmap() {
    if (lightmap.id == 0) return;
    static const Color ambients[6] = {
        { 148, 156, 188, 255 }, // 0: Crypt
        { 150, 178, 152, 255 }, // 1: Emerald Grove
        { 182, 144, 132, 255 }, // 2: Molten Forge
        { 152, 178, 210, 255 }, // 3: Crystal Frost
        { 188, 168, 138, 255 }, // 4: Golden Sun
        { 156, 140, 194, 255 }  // 5: Astral Void
    };
    Color amb = ambients[BiomeIdx()];
    if (inSurface) {
        static const Color surfAmb[5] = {
            { 208, 204, 188, 255 }, // 0: Quảng trường
            { 186, 164, 142, 255 }, // 1: Quán trọ lửa sưởi
            { 184, 194, 212, 255 }, // 2: Phường Gia Trang
            { 196, 182, 174, 255 }, // 3: Tổ ấm gia đình
            { 188, 172, 180, 255 }  // 4: Hoàng cung
        };
        amb = surfAmb[std::max(0, std::min(4, surfaceRoom))];
        if (surfaceRoom == 0) {
            int ph = TimePhase();
            if (ph == 0) amb = { 215, 178, 142, 255 };      // Bình Minh ấm áp
            else if (ph == 1) amb = { 218, 214, 198, 255 }; // Ban Ngày rực nắng
            else if (ph == 2) amb = { 205, 138, 108, 255 }; // Hoàng Hôn ráng đỏ
            else amb = { 96, 108, 156, 255 };               // Ban Đêm huyền ảo (đuốc & đèn sáng rõ!)
        }
    }
    float tm = (float)GetTime();

    BeginTextureMode(lightmap);
    ClearBackground(amb);
    BeginBlendMode(BLEND_ADDITIVE);

    AddLight(player.pos, 400.0f, { 255, 244, 222, 255 }, 0.62f + 0.03f * sinf(tm * 7.0f));
    for (const auto& t : torches) {
        float fl = 0.85f + 0.15f * sinf(tm * 13.0f + t.x * 0.1f);
        AddLight({ t.x, t.y - 8.0f }, 260.0f, { 255, 150, 60, 255 }, 0.58f * fl);
    }
    if (inSurface) {
        if (surfaceRoom == 0) {
            AddLight({ 0.0f, 0.0f }, 250.0f, { 115, 220, 255, 255 }, 0.50f);
            AddLight(SurfaceLandmarkPos(4), 240.0f, { 175, 95, 255, 255 }, 0.72f);
            for (int bi = 0; bi < 4; ++bi) AddLight(SurfaceLandmarkPos(bi), 230.0f, { 255, 210, 125, 255 }, 0.45f);
        } else if (surfaceRoom == 1) {
            AddLight({ 0.0f, -roomH / 2.0f + 50.0f }, 300.0f, { 255, 135, 45, 255 }, 0.65f);
            for (int i = 0; i < 4; ++i) AddLight(TavernStallPos(i), 170.0f, { 255, 195, 105, 255 }, 0.45f);
        } else if (surfaceRoom == 2) {
            for (int t = 1; t <= 3; ++t) AddLight(HousingPedestalPos(t), 200.0f, { 145, 225, 255, 255 }, 0.50f);
        } else if (surfaceRoom == 3) {
            AddLight({ -235.0f, -235.0f }, 260.0f, { 255, 175, 110, 255 }, 0.52f);
            AddLight({  275.0f, -235.0f }, 220.0f, { 255, 215, 95, 255 }, 0.52f);
        } else if (surfaceRoom == 4) {
            AddLight({ 0.0f, -295.0f }, 300.0f, { 255, 220, 105, 255 }, 0.62f);
            AddLight({ -235.0f, -95.0f }, 200.0f, { 255, 215, 85, 255 }, 0.48f);
            AddLight({  235.0f, -95.0f }, 200.0f, { 130, 215, 255, 255 }, 0.48f);
        }
        AddLight(SurfaceInteriorExitPos(), 160.0f, { 140, 225, 255, 255 }, 0.42f);
    }
    if (portalOpen) {
        for (const auto& pc : portalChoices) AddLight(pc.pos, 220.0f, pc.color, 0.68f);
    }
    for (const auto& h : hazards) {
        if (h.kind == 3) AddLight(h.pos, h.radius * 2.2f, { 90, 225, 255, 255 }, 0.45f);
        else if (h.kind == 2 && h.timer > h.cycle * 0.48f) AddLight(h.pos, h.radius * 2.0f, { 255, 110, 30, 255 }, 0.55f);
    }
    for (const auto& b : barrels) {
        if (b.destroyed) continue;
        Color bc = (b.kind == 0) ? ORANGE : (b.kind == 1 ? SKYBLUE : GOLD);
        AddLight(b.pos, 95.0f, bc, 0.40f);
    }
    if (hasTrialTotem && !trialUsed) AddLight(trialPos, 170.0f, { 255, 195, 70, 255 }, 0.55f);
    if (hasCaptiveEvent && !inSurface) AddLight(captivePos, 210.0f, kSpouses[std::max(0, std::min(3, captiveSpouseIdx))].dressCol, captiveRescued ? 0.35f : 0.65f);
    if (hasShrine && !shrineUsed) AddLight({ shrinePos.x, shrinePos.y - 40.0f }, 190.0f, { 255, 50, 80, 255 }, 0.55f + 0.1f * sinf(shrinePulse));
    if (hasBloodPact && !bloodPactUsed) AddLight({ bloodPactPos.x, bloodPactPos.y - 28.0f }, 175.0f, { 215, 35, 70, 255 }, 0.50f + 0.12f * sinf((float)GetTime() * 4.0f));
    for (const auto& pr : projectiles) AddLight(pr.pos, pr.kind == 4 ? 130.0f : 80.0f, pr.color, pr.kind == 4 ? 0.85f : 0.7f);
    for (const auto& ge : groundEffects) AddLight(ge.pos, ge.radius * 1.15f + 40.0f, ge.color, 0.55f);
    for (const auto& nv : novas) {
        float k = nv.timer / nv.maxTimer;
        AddLight(nv.pos, nv.radius * 1.4f + 60.0f, nv.color, 0.55f * k);
    }
    for (const auto& d : itemDrops) AddLight(d.pos, 90.0f, d.item.RarityColor(), 0.55f);
    for (const auto& ch : chests) if (!ch.opened) AddLight(ch.pos, ch.vault ? 150.0f : 110.0f, { 255, 214, 90, 255 }, 0.5f);
    if (shopFloor) AddLight(merchantPos, 240.0f, { 255, 190, 100, 255 }, 0.6f);
    for (const auto& l : loots) {
        Color lc = (l.type == 1) ? Color{ 255, 215, 60, 255 } : (l.type == 2 ? Color{ 255, 100, 130, 255 } : Color{ 90, 200, 255, 255 });
        AddLight(l.pos, 70.0f, lc, 0.55f);
    }
    for (const auto& e : enemies) {
        if (e.hp <= 0) continue;
        if (e.type == BOMBER && e.fusing) AddLight(e.pos, 170.0f, { 255, 90, 40, 255 }, 0.55f + 0.25f * sinf(e.fuseTimer * 40.0f));
        else if (e.burnTimer > 0) AddLight(e.pos, 90.0f, { 255, 140, 40, 255 }, 0.5f);
        else if (e.type == BOSS) AddLight(e.pos, 260.0f, (e.bossKind == 1) ? Color{ 255, 140, 50, 255 } : (e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 170, 90, 255, 255 }), 0.5f);
        else if (e.isElite || e.isWarden) AddLight(e.pos, 110.0f, { 255, 215, 100, 255 }, 0.4f);
    }

    EndBlendMode();
    EndTextureMode();
}

inline void Game::ApplyLighting() {
    if (lightmap.id == 0) return;
    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureRec(lightmap.texture, { 0, 0, (float)lightmap.texture.width, -(float)lightmap.texture.height }, { 0, 0 }, WHITE);
    EndBlendMode();
}

inline void Game::DrawSurfaceTownSquare(const BiomeColors& biome, float tm) {
    // 8 Bồn hoa & Cây cảnh hoàng gia viền đá dọc các đại lộ (giữ nguyên toàn bộ nền gạch 3D sắc nét)
    Vector2 gardens[8] = {
        { -235.0f, -340.0f }, {  235.0f, -340.0f },
        { -235.0f,  340.0f }, {  235.0f,  340.0f },
        { -635.0f, -155.0f }, {  635.0f, -155.0f },
        { -635.0f,  155.0f }, {  635.0f,  155.0f }
    };
    for (int q = 0; q < 8; ++q) {
        Vector2 gp = gardens[q];
        DrawRectangle((int)gp.x - 54, (int)gp.y - 26, 108, 52, { 132, 124, 108, 255 });
        DrawRectangleLines((int)gp.x - 54, (int)gp.y - 26, 108, 52, { 42, 38, 34, 255 });
        DrawRectangle((int)gp.x - 46, (int)gp.y - 18, 92, 36, { 46, 118, 56, 255 });
        for (int f = -1; f <= 1; ++f) {
            float fx = gp.x + f * 26.0f;
            DrawCircleV({ fx, gp.y }, 9.0f, { 58, 152, 70, 255 });
            Diamond({ fx + sinf(tm * 2.0f + q + f) * 1.2f, gp.y - 2.0f }, 4.0f, 4.0f, (f == 0) ? GOLD : Color{ 255, 130, 185, 255 }, WHITE);
        }
    }

    // Đài Phun Nước Thánh Trung Tâm (3 tầng đá cẩm thạch + nước gợn sóng)
    DrawEllipse(4, 14, 78.0f, 34.0f, Fade(BLACK, 0.38f));
    DrawCircle(0, 0, 68.0f, { 142, 134, 122, 255 });
    DrawCircleLines(0, 0, 68.0f, { 34, 30, 38, 255 });
    DrawCircle(0, 0, 56.0f, { 42, 152, 212, 255 });
    DrawCircleLines(0, 0, 28.0f + sinf(tm * 3.2f) * 9.0f, Fade({ 210, 248, 255, 255 }, 0.75f));
    DrawCircleLines(0, 0, 44.0f + cosf(tm * 2.5f) * 6.0f, Fade(WHITE, 0.45f));
    DrawPoly({ 0, 0 }, 8, 22.0f, 22.5f, { 196, 190, 178, 255 });
    DrawPolyLinesEx({ 0, 0 }, 8, 22.0f, 22.5f, 1.8f, { 45, 42, 52, 255 });
    Diamond({ 0.0f, -6.0f + sinf(tm * 4.0f) * 2.5f }, 9.0f, 13.0f, SKYBLUE, WHITE);
    if (Vector2Length(player.pos) < 122.0f) {
        DrawSurfacePrompt({ 0.0f, 76.0f }, L("CRYSTAL FOUNTAIN", "ĐÀI PHUN NƯỚC THÁNH"), L("[F] Drink Sacred Water (+20 HP)", "[F] Uống Nước (+20 Máu)"), SKYBLUE, true);
    }

    // 4 Công Trình Kiến Trúc Chi Tiết (nhiều lớp tường, mái ngói, cửa sổ sáng, ống khói, bảng hiệu)
    struct BldgMeta { Color roof; Color facade; Color accent; const char* titleEn; const char* titleVi; const char* subEn; const char* subVi; };
    Color famRoof = houseTier == 0 ? Color{ 118, 108, 92, 255 } : (houseTier == 1 ? Color{ 176, 92, 46, 255 } : (houseTier == 2 ? Color{ 58, 118, 188, 255 } : Color{ 212, 158, 42, 255 }));
    Color famWall = houseTier >= 3 ? Color{ 238, 226, 204, 255 } : Color{ 218, 202, 178, 255 };
    BldgMeta bldgs[4] = {
        { { 186, 72, 38, 255 },  { 218, 184, 138, 255 }, ORANGE,                    "[1] TAVERN & FOOD MARKET", "[1] QUÁN TRỌ & CHỢ ẨM THỰC",   "Step In / [F] Meals & Rations", "Bước Vào / [F] Đồ Ăn & Lương Khô" },
        { { 58, 120, 168, 255 }, { 210, 212, 218, 255 }, SKYBLUE,                   "[2] HOUSING GUILD",        "[2] PHƯỜNG GIA TRANG",          "Step In / [F] Homes & Estates", "Bước Vào / [F] Xem & Mua Nhà" },
        { famRoof,               famWall,                { 255, 135, 185, 255 },    "[3] FAMILY MANOR & TREE",  "[3] TỔ ẤM & TỪ ĐƯỜNG GIA PHẢ",  "Step In / [F] Spouse, Heirs, RIP","Bước Vào / [F] Vợ Con & Truyền Ngôi" },
        { { 172, 38, 58, 255 },  { 236, 226, 206, 255 }, GOLD,                      "[4] ROYAL PALACE",         "[4] ĐẠI ĐIỆN HOÀNG CUNG",       "Step In / [F] King & Nobility", "Bước Vào / [F] Vua & Quý Tộc" }
    };
    for (int i = 0; i < 4; ++i) {
        Vector2 bp = SurfaceLandmarkPos(i);
        Vector2 door = { bp.x, bp.y + 38.0f };
        bool near = Vector2Distance(player.pos, door) < 105.0f;

        // Bóng đổ công trình & thềm đá lối vào
        DrawEllipse((int)bp.x + 4, (int)bp.y + 36, 106.0f, 26.0f, Fade(BLACK, 0.42f));
        DrawRectangle((int)bp.x - 36, (int)bp.y + 22, 72, 30, { 148, 140, 128, 255 });
        DrawRectangleLines((int)bp.x - 36, (int)bp.y + 22, 72, 30, Fade(biome.trimCol, 0.55f));

        // Thân nhà 3D + khung gỗ/đá
        DrawRectangle((int)bp.x - 92, (int)bp.y - 34, 184, 68, Shade(bldgs[i].facade, 0.78f));
        DrawRectangle((int)bp.x - 86, (int)bp.y - 32, 172, 62, bldgs[i].facade);
        DrawRectangleLinesEx({ bp.x - 92.0f, bp.y - 34.0f, 184.0f, 68.0f }, 2.2f, near ? GOLD : Color{ 38, 30, 28, 255 });
        // Xà gỗ dọc & ngang
        DrawRectangle((int)bp.x - 90, (int)bp.y - 10, 180, 6, { 98, 62, 38, 255 });
        DrawRectangle((int)bp.x - 52, (int)bp.y - 32, 6, 62, { 98, 62, 38, 255 });
        DrawRectangle((int)bp.x + 46, (int)bp.y - 32, 6, 62, { 98, 62, 38, 255 });

        // Cửa sổ kính ấm tỏa sáng
        Color winGlow = { 255, 205, 95, 255 };
        DrawRectangle((int)bp.x - 78, (int)bp.y - 24, 22, 22, { 68, 44, 28, 255 });
        DrawRectangle((int)bp.x - 75, (int)bp.y - 21, 16, 16, winGlow);
        DrawRectangle((int)bp.x + 56, (int)bp.y - 24, 22, 22, { 68, 44, 28, 255 });
        DrawRectangle((int)bp.x + 59, (int)bp.y - 21, 16, 16, winGlow);

        // Cổng vòm mở tối mời người chơi bước vào
        DrawRectangle((int)bp.x - 24, (int)bp.y - 8, 48, 42, { 72, 54, 42, 255 });
        DrawRectangle((int)bp.x - 18, (int)bp.y - 2, 36, 36, near ? Color{ 42, 54, 82, 255 } : Color{ 20, 16, 22, 255 });
        DrawRectangle((int)bp.x - 14, (int)bp.y + 24, 28, 12, Fade(bldgs[i].accent, near ? 0.55f : 0.25f));

        // Mái ngói chóp đa tầng + ống khói/vương miện
        if (i == 0 || i == 2) {
            DrawRectangle((int)bp.x + 42, (int)bp.y - 86, 18, 34, { 112, 86, 76, 255 });
            float sm = fmodf(tm * 0.9f + i, 1.0f);
            DrawCircleV({ bp.x + 51.0f + sinf(tm * 3.0f) * 4.0f, bp.y - 92.0f - sm * 18.0f }, 5.0f + sm * 4.0f, Fade(LIGHTGRAY, (1.0f - sm) * 0.45f));
        }
        Tri({ bp.x - 108.0f, bp.y - 30.0f }, { bp.x + 108.0f, bp.y - 30.0f }, { bp.x, bp.y - 94.0f }, Shade(bldgs[i].roof, 0.72f));
        Tri({ bp.x - 100.0f, bp.y - 32.0f }, { bp.x + 100.0f, bp.y - 32.0f }, { bp.x, bp.y - 88.0f }, bldgs[i].roof);
        DrawLineEx({ bp.x - 104.0f, bp.y - 31.0f }, { bp.x, bp.y - 92.0f }, 2.5f, Fade(GOLD, 0.5f));
        DrawLineEx({ bp.x + 104.0f, bp.y - 31.0f }, { bp.x, bp.y - 92.0f }, 2.5f, Fade(GOLD, 0.5f));
        Diamond({ bp.x, bp.y - 62.0f }, 8.0f, 10.0f, bldgs[i].accent, WHITE);

        DrawSurfacePrompt({ bp.x, bp.y + 58.0f }, L(bldgs[i].titleEn, bldgs[i].titleVi),
                          L(bldgs[i].subEn, bldgs[i].subVi), bldgs[i].accent, near);
    }

    // Vợ & Con đứng đón ở hiên Tổ Ấm (nếu đã cưới)
    Vector2 homeP = SurfaceLandmarkPos(2);
    if (hasSpouse && spouseIdx >= 0 && spouseIdx < 4) {
        DrawDetailedNpcModel({ homeP.x - 112.0f, homeP.y + 24.0f }, spouseIdx, kSpouses[spouseIdx].dressCol, GOLD, { 38, 28, 24, 255 }, true, 0.5f);
    }
    for (size_t k = 0; k < children.size(); ++k) {
        int chc = std::max(0, std::min(3, children[k].heroClass));
        DrawHeroModel({ homeP.x + 112.0f + k * 24.0f, homeP.y + 24.0f }, chc, ClassWeaponType(chc), { -0.8f, 0.2f }, true, tm * 2.0f + k, true, 0.2f, false, false, 0.0f, 0.0f, 0, 0.0f);
    }

    // Cổng Hầm Ngục phía Bắc để quay xuống hầm ngục
    Vector2 dp = SurfaceLandmarkPos(4);
    bool nearGate = Vector2Distance(player.pos, dp) < 96.0f;
    DrawRectangle((int)dp.x - 62, (int)dp.y - 46, 124, 20, Shade(biome.wall, 0.85f));
    DrawRectangleLines((int)dp.x - 62, (int)dp.y - 46, 124, 20, GOLD);
    BeginBlendMode(BLEND_ADDITIVE);
    CircleGrad(dp.x, dp.y, 70.0f + sinf(tm * 5.0f) * 6.0f, Fade({ 165, 85, 255, 255 }, 0.60f), Fade({ 165, 85, 255, 255 }, 0.0f));
    EndBlendMode();
    DrawRing(dp, 32.0f, 38.0f, tm * 70.0f, tm * 70.0f + 275.0f, 24, { 185, 110, 255, 255 });
    DrawCircle((int)dp.x, (int)dp.y, 22.0f, { 48, 18, 78, 245 });
    std::string dgLbl = std::string(L("DESCENT GATE (FLOOR ", "CỔNG HẦM NGỤC (TẦNG ")) + std::to_string(nextDungeonFloor) + ")";
    DrawSurfacePrompt({ dp.x, dp.y + 42.0f }, dgLbl.c_str(), L("[F / G] Enter Dungeon", "[F / G] Xuống Hầm Ngục"), { 205, 145, 255, 255 }, nearGate);
}

inline void Game::DrawSurfaceTavernRoom(const BiomeColors& biome, float tm) {
    // Thảm dệt trung tâm & thảm lối vào ấm áp
    DrawRectangle(-68, -240, 136, (int)(roomH / 2 + 180), { 128, 44, 34, 220 });
    DrawRectangleLinesEx({ -68.0f, -240.0f, 136.0f, roomH / 2.0f + 180.0f }, 2.5f, GOLD);
    DrawRectangle(-165, -75, 330, 150, { 108, 38, 32, 215 });
    DrawRectangleLinesEx({ -165.0f, -75.0f, 330.0f, 150.0f }, 2.5f, Fade(GOLD, 0.85f));

    // Lò sưởi lớn phía Bắc & Vạc hầm
    float fy = -roomH / 2.0f + 26.0f;
    DrawRectangle(-92, (int)fy - 22, 184, 54, { 78, 72, 68, 255 });
    DrawRectangleLines(-92, (int)fy - 22, 184, 54, { 24, 20, 22, 255 });
    DrawRectangle(-52, (int)fy - 6, 104, 34, { 26, 18, 16, 255 });
    DrawCircleV({ 0.0f, fy + 14.0f }, 16.0f + sinf(tm * 11.0f) * 3.0f, ORANGE);
    DrawCircleV({ 0.0f, fy + 12.0f }, 8.0f + cosf(tm * 15.0f) * 2.0f, GOLD);

    // Quầy Bar Chủ Quán
    Vector2 barP = { 0.0f, -275.0f };
    bool nearBar = Vector2Distance(player.pos, barP) < 110.0f;
    DrawRectangle((int)barP.x - 135, (int)barP.y - 26, 270, 44, { 112, 68, 38, 255 });
    DrawRectangleLines((int)barP.x - 135, (int)barP.y - 26, 270, 44, nearBar ? GOLD : Color{ 42, 26, 18, 255 });
    DrawCircle((int)barP.x - 95, (int)barP.y - 6, 12.0f, { 145, 92, 48, 255 });
    DrawCircle((int)barP.x + 95, (int)barP.y - 6, 12.0f, { 145, 92, 48, 255 });
    DrawDetailedNpcModel({ barP.x, barP.y - 34.0f }, 5, { 195, 92, 46, 255 }, WHITE, { 78, 52, 34, 255 }, false, 0.2f);
    DrawSurfacePrompt({ barP.x, barP.y + 24.0f }, L("INNKEEPER COUNTER", "QUẦY CHỦ QUÁN"), L("[F] View Full Menu", "[F] Mở Bảng Thực Đơn"), ORANGE, nearBar);

    // Hai bàn ăn thực khách
    for (int ti = -1; ti <= 1; ti += 2) {
        Vector2 tp = { ti * 265.0f, 130.0f };
        DrawEllipse((int)tp.x, (int)tp.y + 8, 48.0f, 26.0f, Fade(BLACK, 0.35f));
        DrawEllipse((int)tp.x, (int)tp.y, 46.0f, 24.0f, { 126, 78, 44, 255 });
        DrawEllipseLines((int)tp.x, (int)tp.y, 46.0f, 24.0f, { 42, 24, 16, 255 });
        DrawCircle((int)tp.x - 14, (int)tp.y - 4, 6.0f, GOLD);
        DrawCircle((int)tp.x + 16, (int)tp.y + 2, 5.0f, WHITE);
    }

    // 4 Quầy Ẩm Thực Trực Tiếp [F]
    struct StallMeta { const char* nameEn; const char* nameVi; const char* effEn; const char* effVi; int baseCost; Color col; };
    static const StallMeta stalls[4] = {
        { "FRESH BREAD",    "BÁNH MÌ NÓNG",     "+35 Hunger, +18 HP",   "+35 No, +18 Máu",        15, { 245, 205, 115, 255 } },
        { "ROAST MEAT",     "THỊT NƯỚNG",       "+70 Hunger, +40% HP",  "+70 No, +40% Máu",       35, ORANGE },
        { "ROYAL BANQUET",  "YẾN TIỆC VUA",     "Full Hunger/HP +Rage", "Đầy No & Máu +25 Nộ",    70, GOLD },
        { "RATION CRATE",   "TÚI LƯƠNG KHÔ x2", "Buy +2 Rations [H]",   "Mua +2 Lương Khô [H]",   25, { 125, 235, 155, 255 } }
    };
    for (int i = 0; i < 4; ++i) {
        Vector2 sp = TavernStallPos(i);
        bool near = Vector2Distance(player.pos, sp) < 94.0f;
        int price = MealPrice(stalls[i].baseCost);
        DrawEllipse((int)sp.x, (int)sp.y + 18, 54.0f, 18.0f, Fade(BLACK, 0.38f));
        DrawRectangle((int)sp.x - 46, (int)sp.y - 20, 92, 42, i == 2 ? Color{ 126, 36, 48, 255 } : Color{ 106, 68, 42, 255 });
        DrawRectangleLinesEx({ sp.x - 46.0f, sp.y - 20.0f, 92.0f, 42.0f }, near ? 2.5f : 1.5f, near ? GOLD : stalls[i].col);
        if (i == 0) {
            DrawEllipse((int)sp.x - 14, (int)sp.y - 2, 13.0f, 8.0f, { 235, 185, 95, 255 });
            DrawEllipse((int)sp.x + 14, (int)sp.y - 2, 13.0f, 8.0f, { 245, 198, 108, 255 });
        } else if (i == 1) {
            DrawCircle((int)sp.x, (int)sp.y, 13.0f, { 205, 86, 42, 255 });
            DrawLineEx({ sp.x - 24.0f, sp.y }, { sp.x + 24.0f, sp.y }, 2.5f, LIGHTGRAY);
        } else if (i == 2) {
            DrawCircle((int)sp.x, (int)sp.y, 14.0f, GOLD);
            Diamond({ sp.x, sp.y - 4.0f }, 6.0f, 7.0f, SKYBLUE, WHITE);
        } else {
            DrawRectangle((int)sp.x - 20, (int)sp.y - 10, 40, 22, { 142, 102, 58, 255 });
            DrawRectangleLines((int)sp.x - 20, (int)sp.y - 10, 40, 22, GOLD);
        }
        std::string sub = std::string(L("[F] Buy ", "[F] Mua ")) + std::to_string(price) + L("G | ", "V | ") + L(stalls[i].effEn, stalls[i].effVi);
        DrawSurfacePrompt({ sp.x, sp.y + 28.0f }, L(stalls[i].nameEn, stalls[i].nameVi), sub.c_str(), stalls[i].col, near);
    }
    (void)biome;
    DrawSurfaceExitDoor(SurfaceInteriorExitPos());
}

inline void Game::DrawSurfaceHousingRoom(const BiomeColors& biome, float tm) {
    (void)tm;
    DrawPoly({ 0.0f, 40.0f }, 8, 220.0f, 22.5f, Fade(biome.trimCol, 0.14f));
    DrawPolyLinesEx({ 0.0f, 40.0f }, 8, 220.0f, 22.5f, 2.5f, Fade(GOLD, 0.55f));

    Vector2 deskP = { 0.0f, -295.0f };
    bool nearDesk = Vector2Distance(player.pos, deskP) < 110.0f;
    DrawRectangle((int)deskP.x - 110, (int)deskP.y - 24, 220, 42, { 88, 98, 118, 255 });
    DrawRectangleLines((int)deskP.x - 110, (int)deskP.y - 24, 220, 42, nearDesk ? GOLD : SKYBLUE);
    DrawRectangle((int)deskP.x - 42, (int)deskP.y - 14, 84, 22, { 175, 220, 255, 230 });
    DrawDetailedNpcModel({ deskP.x, deskP.y - 34.0f }, 6, { 56, 114, 178, 255 }, GOLD, { 185, 190, 205, 255 }, false, 0.4f);
    DrawSurfacePrompt({ deskP.x, deskP.y + 24.0f }, L("MASTER ARCHITECT", "BẬC THẦY KIẾN TRÚC"), L("[F] View Estate Ledger", "[F] Xem Sổ Gia Trang"), SKYBLUE, nearDesk);

    Color roofs[3] = { { 172, 92, 48, 255 }, { 58, 118, 188, 255 }, { 215, 165, 42, 255 } };
    for (int t = 1; t <= 3; ++t) {
        Vector2 hp = HousingPedestalPos(t);
        bool near = Vector2Distance(player.pos, hp) < 108.0f;
        bool owned = (houseTier >= t);
        bool nextT = (houseTier + 1 == t);
        int price = HouseUpgradeCost(t);
        Color col = owned ? GREEN : (nextT ? GOLD : GRAY);

        DrawEllipse((int)hp.x, (int)hp.y + 24, 62.0f, 22.0f, Fade(BLACK, 0.42f));
        DrawPoly(hp, 8, 54.0f, 22.5f, Shade(biome.wall, 0.78f));
        DrawPolyLinesEx(hp, 8, 54.0f, 22.5f, near ? 2.8f : 1.8f, near ? GOLD : col);
        DrawMiniHouseModel({ hp.x, hp.y - 6.0f }, t, roofs[t - 1], col);

        std::string sub = owned ? L("OWNED  (+Active Bonus)", "ĐÃ SỞ HỮU")
                        : (nextT ? TextFormat(L("[F] BUY %dG  (+%d HP/+%d DMG)", "[F] MUA %dV (+%d Máu/+%d ST)"), price, (int)kHouses[t].hpBonus, (int)kHouses[t].dmgBonus)
                                 : L("Requires Previous House Tier", "Cần nâng cấp Nhà trước"));
        DrawSurfacePrompt({ hp.x, hp.y + 42.0f }, HouseName(t), sub.c_str(), col, near);
    }
    DrawSurfaceExitDoor(SurfaceInteriorExitPos());
}

inline void Game::DrawSurfaceFamilyHomeRoom(const BiomeColors& biome, float tm) {
    (void)biome;
    // Thảm đại sảnh gia đình nâng cấp theo House Tier (sắc nét, không che mờ nền gạch)
    Color rugCol = (houseTier == 0) ? Color{ 102, 86, 68, 220 } : (houseTier == 1 ? Color{ 44, 108, 62, 225 } : (houseTier == 2 ? Color{ 50, 68, 138, 230 } : Color{ 138, 30, 50, 235 }));
    Color rugTrim = (houseTier >= 3) ? GOLD : Color{ 225, 205, 160, 235 };
    float rw = 290.0f + houseTier * 35.0f, rh = 190.0f + houseTier * 20.0f;
    DrawRectangle((int)(-rw / 2), (int)(-rh / 2), (int)rw, (int)rh, rugCol);
    DrawRectangleLinesEx({ -rw / 2, -rh / 2, rw, rh }, 3.0f, rugTrim);
    DrawRectangle(-58, -10, 116, (int)(roomH / 2 - 42), rugCol);
    DrawRectangleLinesEx({ -58.0f, -10.0f, 116.0f, roomH / 2.0f - 42.0f }, 2.2f, rugTrim);

    // Lò sưởi ấm áp bên Tây Bắc
    DrawRectangle(-290, (int)(-roomH / 2 + 12), 110, 44, { 92, 84, 78, 255 });
    DrawRectangle(-265, (int)(-roomH / 2 + 28), 60, 26, { 30, 18, 16, 255 });
    DrawCircleV({ -235.0f, -roomH / 2.0f + 42.0f }, 12.0f + sinf(tm * 10.0f) * 2.5f, ORANGE);

    // Giường & Bàn ăn trang trí theo cấp Nhà
    if (houseTier >= 1) {
        DrawRectangle((int)(-roomW / 2 + 65), -260, 90, 56, { 118, 72, 42, 255 });
        DrawRectangle((int)(-roomW / 2 + 72), -252, 76, 40, hasSpouse ? kSpouses[std::max(0, spouseIdx)].dressCol : Color{ 185, 195, 220, 255 });
    }

    // Khu Vực Cưới Vợ / Vợ Đứng Chờ (Tây) — Vợ không mua bằng vàng mà mở khóa từ Sự Kiện / Nhiệm Vụ
    if (!hasSpouse) {
        for (int i = 0; i < 4; ++i) {
            Vector2 cp = SpouseCandPos(i);
            bool near = Vector2Distance(player.pos, cp) < 85.0f;
            const SpouseDef& sp = kSpouses[i];
            bool unlocked = spouseUnlocked[i];
            DrawCircleLines((int)cp.x, (int)cp.y + 4, 28.0f, Fade(unlocked ? sp.dressCol : GRAY, near ? 0.9f : 0.4f));
            DrawDetailedNpcModel(cp, i, unlocked ? sp.dressCol : LerpColor(sp.dressCol, { 100, 108, 128, 255 }, 0.38f), unlocked ? GOLD : LIGHTGRAY, { 38, 28, 32, 255 }, unlocked, i * 1.1f);
            std::string sub = !unlocked
                            ? std::string(SpouseUnlockConditionText(i))
                            : ((houseTier <= 0) ? std::string(L("Buy House First!", "Đã mở khóa! Hãy Mua Nhà trước"))
                                                : TextFormat(L("[F] Marry Free (+%dHP +%dDMG)", "[F] Kết Hôn (+%dM +%dST)"), (int)sp.hpBonus, (int)sp.dmgBonus));
            DrawSurfacePrompt({ cp.x, cp.y + 26.0f }, SpouseName(i), sub.c_str(), unlocked ? sp.dressCol : GRAY, near);
        }
    } else {
        Vector2 wp = { -235.0f, -175.0f };
        bool nearW = Vector2Distance(player.pos, wp) < 95.0f;
        const SpouseDef& sp = kSpouses[std::max(0, std::min(3, spouseIdx))];
        DrawDetailedNpcModel(wp, spouseIdx, sp.dressCol, GOLD, { 42, 26, 30, 255 }, true, 0.2f);
        Diamond({ wp.x, wp.y - 30.0f + sinf(tm * 3.5f) * 3.0f }, 5.0f, 5.0f, { 255, 110, 165, 255 }, WHITE);
        DrawSurfacePrompt({ wp.x, wp.y + 24.0f }, SpouseName(spouseIdx), L("[F] Warm Embrace (+25 HP)", "[F] Trò Chuyện (+25 Máu)"), sp.dressCol, nearW);
    }

    // Nôi Sinh Con (Bắc Giữa)
    Vector2 crP = { 0.0f, -235.0f };
    bool nearCr = Vector2Distance(player.pos, crP) < 98.0f;
    DrawRectangle((int)crP.x - 36, (int)crP.y - 20, 72, 40, { 138, 88, 52, 255 });
    DrawRectangle((int)crP.x - 28, (int)crP.y - 12, 56, 24, { 175, 225, 255, 255 });
    DrawRectangleLines((int)crP.x - 36, (int)crP.y - 20, 72, 40, nearCr ? GOLD : WHITE);
    int maxKids = kHouses[std::max(0, std::min(3, houseTier))].maxKids;
    std::string crSub = (!hasSpouse || houseTier <= 0) ? std::string(L("Requires House & Spouse", "Cần Có Nhà & Đã Cưới"))
                      : ((int)children.size() >= maxKids ? std::string(L("Upgrade House for More", "Nâng cấp Nhà để thêm con"))
                      : TextFormat(L("[F] Have Heir (%dG) [%d/%d]", "[F] Sinh Con (%dV) [%d/%d]"), MealPrice(100), (int)children.size(), maxKids));
    DrawSurfacePrompt({ crP.x, crP.y + 28.0f }, L("NURSERY CRADLE", "NÔI SINH CON NỐI DÕI"), crSub.c_str(), { 140, 230, 255, 255 }, nearCr);

    // Từ Đường & Cây Gia Phả (Đông Bắc)
    Vector2 shP = { 275.0f, -235.0f };
    bool nearSh = Vector2Distance(player.pos, shP) < 98.0f;
    DrawRectangle((int)shP.x - 44, (int)shP.y - 22, 88, 44, { 72, 56, 48, 255 });
    DrawRectangleLines((int)shP.x - 44, (int)shP.y - 22, 88, 44, GOLD);
    Diamond({ shP.x, shP.y - 6.0f + sinf(tm * 2.5f) * 2.0f }, 8.0f, 11.0f, GOLD, WHITE);
    DrawSurfacePrompt({ shP.x, shP.y + 28.0f }, L("ANCESTRAL SHRINE", "TỪ ĐƯỜNG GIA PHẢ"), L("[F / Y] Open Family Tree", "[F / Y] Mở Cây Gia Phả"), GOLD, nearSh);

    // Khu Vực Thế Hệ Con Cái (Đông): Rest in Peace / Kế Thừa
    if (children.empty()) {
        DrawRectangleLines(140, -70, 250, 120, Fade(GOLD, 0.30f));
        const char* noKid = L("No Heirs Yet", "Chưa Có Thế Hệ Con");
        DrawText(noKid, 265 - MeasureText(noKid, 14) / 2, -16, 14, { 180, 190, 215, 255 });
    } else {
        for (size_t k = 0; k < children.size(); ++k) {
            Vector2 hp = HeirPosInHome((int)k);
            const ChildInfo& ch = children[k];
            bool nearH = Vector2Distance(player.pos, hp) < 85.0f;
            int hc = std::max(0, std::min(3, ch.heroClass));
            Color cc = kClasses[hc].color;
            DrawCircleLines((int)hp.x, (int)hp.y + 6, 30.0f, Fade(nearH ? GOLD : (ch.inParty ? Color{ 120, 245, 170, 255 } : cc), 0.72f));
            {
                float sc = Clamp(0.45f + ch.age / 18.0f * 0.55f, 0.45f, 1.0f); // con lớn dần theo tuổi
                rlPushMatrix();
                rlTranslatef(hp.x, hp.y, 0.0f);
                rlScalef(sc, sc, 1.0f);
                DrawHeroModel({ 0.0f, 0.0f }, hc, ClassWeaponType(hc), { -0.8f, 0.2f }, true, tm * 2.5f + k, true, 0.2f, false, false, 0.0f, 0.0f, 0, 0.0f);
                rlPopMatrix();
            }
            std::string title = TextFormat(L("%s - Age %d - Lv%d", "%s - %d tuổi - Lv%d"), ch.name.c_str(), ChildAgeYears(ch), ch.level);
            std::string ripPart = (ch.age >= (float)kChildAdultAge)
                ? std::string(L("[F] Rest in Peace", "[F] An Nghỉ - Kế Thừa"))
                : std::string(TextFormat(L("Inherit at %d", "Kế thừa khi %d tuổi"), kChildAdultAge));
            std::string trainPart = (ch.injuredTimer > 0.0f) ? std::string(TextFormat(L("Wounded %ds", "Dưỡng thương %ds"), (int)ceilf(ch.injuredTimer)))
                : (ch.age < (float)kChildTrainAge) ? std::string(TextFormat(L("Train at %d", "Rèn luyện khi %d tuổi"), kChildTrainAge))
                : (ch.inParty ? std::string(L("[T] Stay Home", "[T] Ở Nhà")) : std::string(L("[T] Bring to Dungeon", "[T] Dẫn Theo")));
            std::string sub = ripPart + "  |  " + trainPart + "  |  " + L("[N] Rename", "[N] Đổi Tên");
            DrawSurfacePrompt({ hp.x, hp.y + 26.0f }, title.c_str(), sub.c_str(), ch.inParty ? Color{ 120, 245, 170, 255 } : cc, nearH);
        }
    }
    DrawSurfaceExitDoor(SurfaceInteriorExitPos());
}

inline void Game::DrawSurfacePalaceRoom(const BiomeColors& biome, float tm) {
    (void)biome;
    // Thảm đỏ hoàng gia trải dọc đại điện từ cửa tới ngai vàng
    DrawRectangle(-95, -280, 190, (int)(roomH / 2 + 240), { 142, 26, 42, 235 });
    DrawRectangleLinesEx({ -95.0f, -280.0f, 190.0f, roomH / 2.0f + 240.0f }, 3.5f, GOLD);

    // Ngự Lâm Quân canh gác đại điện
    Vector2 guardPts[4] = { { -145, -270 }, { 145, -270 }, { -160, 110 }, { 160, 110 } };
    for (int gi = 0; gi < 4; ++gi) {
        Vector2 dir = (guardPts[gi].x < 0) ? Vector2{ 1.0f, 0.0f } : Vector2{ -1.0f, 0.0f };
        DrawHeroModel(guardPts[gi], CLASS_KNIGHT, WP_SWORD, dir, guardPts[gi].x > 0, 0.0f, false, 0.0f, false, true, 0.0f, 0.0f, 1, 0.0f);
    }

    // Bục Ngai Vàng & Đức Vua
    Vector2 thP = { 0.0f, -295.0f };
    bool nearKing = Vector2Distance(player.pos, thP) < 115.0f;
    DrawRectangle(-170, (int)thP.y - 60, 340, 110, { 98, 72, 54, 255 });
    DrawRectangleLines(-170, (int)thP.y - 60, 340, 110, GOLD);
    DrawRectangle(-42, (int)thP.y - 44, 84, 54, GOLD);
    DrawRectangle(-32, (int)thP.y - 34, 64, 42, { 165, 28, 44, 255 });
    DrawDetailedNpcModel({ 0.0f, thP.y - 12.0f }, 4, { 188, 32, 52, 255 }, GOLD, { 225, 215, 200, 255 }, true, tm * 0.1f);
    DrawSurfacePrompt({ thP.x, thP.y + 38.0f }, L("HIS MAJESTY THE KING", "ĐỨC VUA HOÀNG TRIỀU"), L("[F] Royal Audience", "[F] Yết Kiến Đức Vua"), GOLD, nearKing);

    // Đài Quý Tộc Mua Trực Tiếp (Tây)
    Vector2 trP = { -235.0f, -95.0f };
    bool nearTr = Vector2Distance(player.pos, trP) < 108.0f;
    DrawPoly(trP, 6, 48.0f, 0.0f, { 72, 48, 36, 255 });
    DrawPolyLinesEx(trP, 6, 48.0f, 0.0f, 2.5f, isNoble ? GREEN : GOLD);
    DrawRectangle((int)trP.x - 22, (int)trP.y - 14, 44, 28, GOLD);
    DrawRectangleLines((int)trP.x - 22, (int)trP.y - 14, 44, 28, { 42, 24, 16, 255 });
    Diamond({ trP.x, trP.y - 18.0f }, 8.0f, 8.0f, SKYBLUE, WHITE);
    DrawSurfacePrompt({ trP.x, trP.y + 42.0f }, L("ROYAL TREASURER", "ĐÀI SẮC PHONG QUÝ TỘC"),
                      isNoble ? L("NOBILITY GRANTED", "ĐÃ MANG TƯỚC QUÝ TỘC") : L("[F] Buy Nobility (1500 Gold)", "[F] Mua Quý Tộc (1500 Vàng)"), GOLD, nearTr);

    // Bàn Chiến Lược & Ủy Thác Của Vua (Đông)
    Vector2 wtP = { 235.0f, -95.0f };
    bool nearWt = Vector2Distance(player.pos, wtP) < 108.0f;
    DrawRectangle((int)wtP.x - 56, (int)wtP.y - 34, 112, 68, { 68, 52, 44, 255 });
    DrawRectangleLines((int)wtP.x - 56, (int)wtP.y - 34, 112, 68, nearWt ? GOLD : SKYBLUE);
    DrawRectangle((int)wtP.x - 42, (int)wtP.y - 22, 84, 44, { 34, 62, 88, 255 });
    Diamond({ wtP.x + 12.0f, wtP.y }, 6.0f, 6.0f, RED, GOLD);
    std::string wSub = isNoble ? L("Royal Commission Completed!", "Đã nhận Tước Vị Quý Tộc!")
                     : (royalQuestActive
                        ? ((royalQuestKills >= 35 && royalQuestBosses >= 1)
                           ? std::string(L("[F] CLAIM NOBLE TITLE!", "[F] LĨNH THƯỞNG PHONG QUÝ TỘC!"))
                           : TextFormat(L("Progress: %d/35 Monsters, %d/1 Boss", "Tiến độ: %d/35 Quái, %d/1 Trùm"), royalQuestKills, royalQuestBosses))
                        : std::string(L("[F] Accept King's Quest (35 Monsters + Boss)", "[F] Nhận Ủy Thác (35 Quái + 1 Trùm)")));
    DrawSurfacePrompt({ wtP.x, wtP.y + 42.0f }, L("KING'S WAR TABLE", "BÀN ỦY THÁC THẢO PHẠT"), wSub.c_str(), ORANGE, nearWt);

    DrawSurfaceExitDoor(SurfaceInteriorExitPos());
}

inline void Game::DrawSurfaceWorld(const BiomeColors& biome) {
    float tm = (float)GetTime();
    if (surfaceRoom == 1) DrawSurfaceTavernRoom(biome, tm);
    else if (surfaceRoom == 2) DrawSurfaceHousingRoom(biome, tm);
    else if (surfaceRoom == 3) DrawSurfaceFamilyHomeRoom(biome, tm);
    else if (surfaceRoom == 4) DrawSurfacePalaceRoom(biome, tm);
    else DrawSurfaceTownSquare(biome, tm);
}

inline void Game::Draw() {
    if (settings.lighting) RenderLightmap();
    BeginTextureMode(screenRT);
    BiomeColors biome = Biome();
    ClearBackground(biome.bg);

    BeginMode2D(camera);

    // 1. Tường gạch, sàn đa sắc vùng hành lang/viền/trung tâm, bóng viền, huy hiệu, cảnh quan biome, đuốc, cột (Dùng chung cả Hầm Ngục & Mặt Đất)
    int halfW = (int)roomW / 2;
    int halfH = (int)roomH / 2;
    int tileSize = 48;
    DrawWalls(biome);
    for (int y = -halfH; y < halfH; y += tileSize) {
        for (int x = -halfW; x < halfW; x += tileSize) {
            int tx = x / tileSize, ty = y / tileSize;
            bool alt = ((tx + ty) & 1) == 0;
            float cxT = x + tileSize * 0.5f, cyT = y + tileSize * 0.5f;
            bool isBorder = (x <= -halfW + tileSize || x >= halfW - tileSize * 2 || y <= -halfH + tileSize || y >= halfH - tileSize * 2);
            bool isPath = (fabsf(cxT) < 72.0f || fabsf(cyT) < 72.0f);
            bool isCornerPlaza = (fabsf(cxT) > halfW - 150.0f && fabsf(cyT) > halfH - 150.0f);
            Color tileCol = (isPath || isBorder || isCornerPlaza)
                          ? (alt ? biome.pathCol : Shade(biome.pathCol, 0.88f))
                          : (alt ? biome.tileA : biome.tileB);
            int th = ((tx * 73856093) ^ (ty * 19349663)) & 15;
            tileCol = Shade(tileCol, 0.96f + (th & 3) * 0.025f);
            DrawRectangle(x, y, tileSize, tileSize, tileCol);
            if (isPath || isCornerPlaza) {
                DrawRectangleLines(x + 4, y + 4, tileSize - 8, tileSize - 8, Fade(biome.trimCol, 0.22f));
            }
            DrawRectangle(x, y, tileSize, 2, Fade(WHITE, 0.05f));
            DrawRectangle(x, y, 2, tileSize, Fade(WHITE, 0.04f));
            DrawRectangleLines(x, y, tileSize, tileSize, Fade(BLACK, 0.16f));
            if (th == 0) DrawRectangle(x + 12, y + 16, 6, 4, Fade(BLACK, 0.12f));
            else if (th == 7) DrawRectangle(x + 26, y + 26, 6, 6, Fade(biome.accentCol, 0.16f));
        }
    }
    // Đường viền hành lang chữ thập & Huy hiệu trung tâm đa sắc
    DrawRectangleLinesEx({ -68.0f, (float)-halfH + 48.0f, 136.0f, roomH - 96.0f }, 2.0f, Fade(biome.trimCol, 0.28f));
    DrawRectangleLinesEx({ (float)-halfW + 48.0f, -68.0f, roomW - 96.0f, 136.0f }, 2.0f, Fade(biome.trimCol, 0.28f));
    DrawPoly({ 0, 0 }, 8, 148.0f, 22.5f, Fade(biome.pathCol, 0.85f));
    DrawPolyLinesEx({ 0, 0 }, 8, 148.0f, 22.5f, 3.0f, Fade(biome.trimCol, 0.65f));
    DrawPolyLinesEx({ 0, 0 }, 8, 108.0f, 0.0f, 2.0f, Fade(biome.accentCol, 0.45f));
    for (int ci = 0; ci < 8; ++ci) {
        float ca = ci * (PI / 4.0f);
        DrawLineEx({ cosf(ca) * 32.0f, sinf(ca) * 32.0f }, { cosf(ca) * 102.0f, sinf(ca) * 102.0f }, 2.0f, Fade(biome.trimCol, 0.35f));
    }
    Diamond({ 0, 0 }, 24.0f, 24.0f, Fade(biome.trimCol, 0.45f), Fade(biome.accentCol, 0.75f));

    // Bóng đổ từ tường xuống sàn
    DrawRectangleGradientV(-halfW, -halfH, (int)roomW, 46, Fade(BLACK, 0.50f), Fade(BLACK, 0.0f));
    DrawRectangleGradientV(-halfW, halfH - 46, (int)roomW, 46, Fade(BLACK, 0.0f), Fade(BLACK, 0.50f));
    DrawRectangleGradientH(-halfW, -halfH, 46, (int)roomH, Fade(BLACK, 0.50f), Fade(BLACK, 0.0f));
    DrawRectangleGradientH(halfW - 46, -halfH, 46, (int)roomH, Fade(BLACK, 0.0f), Fade(BLACK, 0.50f));
    DrawRectangleLinesEx({ -roomW / 2.0f, -roomH / 2.0f, roomW, roomH }, 3.0f, biome.trimCol);

    DrawDecals(biome);

    if (inSurface) {
        DrawSurfaceWorld(biome);
        for (const auto& m : motes) {
            float a2 = 0.10f + 0.16f * (0.5f + 0.5f * sinf((float)GetTime() * 1.6f + m.phase));
            DrawCircleV(m.pos, m.size, Fade(biome.mote, a2));
        }
        for (const auto& tp : torches) DrawTorch(tp);
        for (const auto& pl : pillars) DrawPillar(pl, biome);
    } else {

    // Bẫy địa hình & Vòng năng lượng Arcane Leyline
    {
        float tm = (float)GetTime();
        for (const auto& h : hazards) {
            bool warn = (h.timer > h.cycle * 0.48f);
            bool active = (h.timer > h.cycle * 0.68f);
            if (h.kind == 0) { // Spike Grate
                BoxRot(h.pos, h.radius * 1.45f, h.radius * 1.45f, 0.0f, { 34, 38, 48, 235 }, active ? Color{ 255, 110, 90, 255 } : (warn ? ORANGE : Color{ 85, 92, 112, 255 }), 2.0f);
                for (int gx = -1; gx <= 1; ++gx) {
                    for (int gy = -1; gy <= 1; ++gy) {
                        Vector2 gp = { h.pos.x + gx * 18.0f, h.pos.y + gy * 18.0f };
                        DrawCircle((int)gp.x, (int)gp.y, 4.0f, { 14, 16, 22, 255 });
                        if (active) {
                            Tri({ gp.x - 4.5f, gp.y + 3.0f }, { gp.x + 4.5f, gp.y + 3.0f }, { gp.x, gp.y - 13.0f }, { 225, 235, 250, 255 });
                        } else if (warn) {
                            DrawCircleV(gp, 2.2f, { 255, 195, 70, 255 });
                        }
                    }
                }
            } else if (h.kind == 1) { // Toxic Vent
                DrawPoly(h.pos, 8, h.radius * 0.86f, 22.5f, active ? Color{ 42, 130, 56, 220 } : Color{ 26, 56, 36, 220 });
                DrawPolyLinesEx(h.pos, 8, h.radius * 0.86f, 22.5f, 2.0f, active ? LIME : (warn ? YELLOW : Color{ 70, 130, 78, 255 }));
                if (active) {
                    DrawCircleV(h.pos, h.radius * (0.7f + 0.25f * sinf(tm * 12.0f)), Fade(LIME, 0.28f));
                    DrawCircleLines((int)h.pos.x, (int)h.pos.y, h.radius, Fade(LIME, 0.75f));
                } else if (warn) {
                    DrawCircleV(h.pos, h.radius * 0.45f, Fade(LIME, 0.22f));
                }
            } else if (h.kind == 2) { // Magma Geyser
                DrawPoly(h.pos, 7, h.radius * 0.86f, 0.0f, active ? Color{ 165, 52, 20, 240 } : Color{ 52, 26, 22, 230 });
                DrawPolyLinesEx(h.pos, 7, h.radius * 0.86f, 0.0f, 2.0f, active ? ORANGE : (warn ? GOLD : Color{ 125, 62, 42, 255 }));
                if (active) {
                    DrawCircleV(h.pos, h.radius * 0.92f, Fade(ORANGE, 0.30f));
                    for (int k = -1; k <= 1; ++k)
                        Tri({ h.pos.x + k * 14.0f - 7.0f, h.pos.y + 4.0f }, { h.pos.x + k * 14.0f + 7.0f, h.pos.y + 4.0f }, { h.pos.x + k * 14.0f, h.pos.y - 20.0f - 5.0f * sinf(tm * 18.0f + k) }, { 255, 195, 55, 255 });
                } else if (warn) {
                    DrawCircleV(h.pos, h.radius * 0.48f, Fade({ 255, 120, 35, 255 }, 0.30f));
                }
            } else { // Arcane Leyline (Buff Pad)
                bool inside = Vector2Distance(player.pos, h.pos) < h.radius;
                DrawCircleV(h.pos, h.radius, Fade({ 80, 220, 255, 255 }, inside ? 0.18f : 0.09f));
                DrawPolyLinesEx(h.pos, 6, h.radius, tm * 35.0f, 2.0f, inside ? GOLD : Color{ 95, 230, 255, 220 });
                DrawRing(h.pos, h.radius * 0.58f, h.radius * 0.66f, -tm * 55.0f, -tm * 55.0f + 280.0f, 18, Fade(GOLD, 0.70f));
                const char* lt = L("LEYLINE +SPD/+RAGE", "MẠCH MA PHÁP +TỐC/+NỘ");
                DrawText(lt, (int)h.pos.x - MeasureText(lt, 11) / 2, (int)(h.pos.y + h.radius + 4.0f), 11, inside ? GOLD : SKYBLUE);
            }
        }
    }

    // Bụi lơ lửng
    for (const auto& m : motes) {
        float a2 = 0.10f + 0.16f * (0.5f + 0.5f * sinf((float)GetTime() * 1.6f + m.phase));
        DrawCircleV(m.pos, m.size, Fade(biome.mote, a2));
    }

    for (const auto& tp : torches) DrawTorch(tp);
    for (const auto& pl : pillars) DrawPillar(pl, biome);

    // Thùng nổ / Tinh thể băng / Trụ sấm chiến thuật
    for (const auto& b : barrels) {
        if (b.destroyed) continue;
        Vector2 p = b.pos;
        const Color ink = { 18, 16, 26, 255 };
        DrawEllipse((int)p.x, (int)p.y + 15, 18.0f, 6.0f, Fade(BLACK, 0.38f));
        if (b.kind == 0) { // Explosive Keg
            Color bodyC = b.hitFlash > 0.0f ? WHITE : Color{ 165, 84, 44, 255 };
            BoxRot(p, 26.0f, 30.0f, 0.0f, bodyC, ink, 1.8f);
            BoxRot(p, 27.0f, 10.0f, 0.0f, b.hitFlash > 0.0f ? WHITE : Color{ 225, 58, 44, 255 }, ink, 1.3f);
            DrawText("BOOM", (int)p.x - MeasureText("BOOM", 8) / 2, (int)p.y - 4, 8, WHITE);
        } else if (b.kind == 1) { // Cryo Crystal
            Color cry = b.hitFlash > 0.0f ? WHITE : SKYBLUE;
            Tri({ p.x - 12.0f, p.y + 10.0f }, { p.x - 2.0f, p.y + 10.0f }, { p.x - 10.0f, p.y - 14.0f }, Shade(cry, 0.8f));
            Tri({ p.x + 12.0f, p.y + 10.0f }, { p.x + 2.0f, p.y + 10.0f }, { p.x + 10.0f, p.y - 12.0f }, Shade(cry, 0.8f));
            Diamond({ p.x, p.y - 2.0f }, 11.0f, 18.0f, cry, WHITE);
        } else { // Shock Conduit
            Color col = b.hitFlash > 0.0f ? WHITE : Color{ 115, 100, 195, 255 };
            QuadOut({ p.x - 10.0f, p.y - 12.0f }, { p.x + 10.0f, p.y - 12.0f }, { p.x + 14.0f, p.y + 14.0f }, { p.x - 14.0f, p.y + 14.0f }, col, ink, 1.8f);
            Diamond({ p.x, p.y - 18.0f + sinf((float)GetTime() * 6.0f) * 3.0f }, 6.5f, 8.5f, GOLD, WHITE);
        }
    }

    for (const auto& ch : chests) DrawChest(ch);
    if (shopFloor) DrawMerchant();

    // Cột Thử Thách (Trial Totem)
    if (hasTrialTotem) {
        float tm = (float)GetTime();
        Vector2 tp = trialPos;
        const Color ink = { 18, 16, 26, 255 };
        DrawEllipse((int)tp.x, (int)tp.y + 22, 26.0f, 9.0f, Fade(BLACK, 0.40f));
        if (!trialUsed) {
            DrawPolyLinesEx(tp, 6, 36.0f, tm * 30.0f, 2.0f, Fade(GOLD, 0.70f));
        }
        QuadOut({ tp.x - 16.0f, tp.y + 8.0f }, { tp.x + 16.0f, tp.y + 8.0f }, { tp.x + 20.0f, tp.y + 20.0f }, { tp.x - 20.0f, tp.y + 20.0f }, trialUsed ? Color{ 68, 66, 78, 255 } : Color{ 98, 72, 138, 255 }, ink, 1.8f);
        QuadOut({ tp.x - 11.0f, tp.y - 24.0f }, { tp.x + 11.0f, tp.y - 24.0f }, { tp.x + 14.0f, tp.y + 10.0f }, { tp.x - 14.0f, tp.y + 10.0f }, trialUsed ? Color{ 56, 54, 68, 255 } : Color{ 126, 92, 178, 255 }, GOLD, 1.6f);
        if (!trialUsed) {
            Diamond({ tp.x, tp.y - 36.0f + sinf(tm * 3.5f) * 3.0f }, 7.5f, 10.5f, GOLD, WHITE);
            const char* ttLbl = L("TRIAL TOTEM", "CỘT THỬ THÁCH");
            DrawText(ttLbl, (int)tp.x - MeasureText(ttLbl, 13) / 2, (int)tp.y + 25, 13, GOLD);
            if (Vector2Distance(tp, player.pos) < 120.0f) {
                const char* pr = L("[F] Start Ambush (Vault Chest)", "[F] Mở Phục Kích (Rương Kho Báu)");
                OutlinedText(pr, (int)tp.x - MeasureText(pr, 16) / 2, (int)tp.y + 42, 16, WHITE, { 20, 24, 40, 255 });
            }
        }
    }

    // Blood Shrine
    if (hasShrine) {
        float sx = shrinePos.x, sy = shrinePos.y;
        if (!shrineUsed) {
            float tm = (float)GetTime();
            DrawCircle((int)sx, (int)sy + 8, 42.0f, Fade({ 255, 40, 70, 255 }, 0.10f));
            DrawCircleLines((int)sx, (int)sy + 8, 42.0f, Fade({ 255, 60, 90, 255 }, 0.45f));
            for (int i = 0; i < 6; ++i) {
                float a = tm * 1.2f + i * (PI / 3.0f);
                DrawCircleV({ sx + cosf(a) * 42.0f, sy + 8.0f + sinf(a) * 20.0f }, 2.5f, { 255, 110, 130, 255 });
            }
        }
        DrawEllipse((int)sx, (int)sy + 26, 30, 10, { 0, 0, 0, 100 });
        Color stone = shrineUsed ? Color{ 90, 88, 100, 255 } : Color{ 110, 104, 130, 255 };
        DrawRectangle((int)sx - 22, (int)sy + 14, 44, 14, stone);
        DrawRectangle((int)sx - 14, (int)sy - 20, 28, 36, stone);
        DrawRectangle((int)sx - 20, (int)sy - 28, 40, 10, stone);
        DrawRectangleLines((int)sx - 14, (int)sy - 20, 28, 36, { 34, 30, 44, 255 });
        DrawRectangleLines((int)sx - 20, (int)sy - 28, 40, 10, { 34, 30, 44, 255 });
        if (!shrineUsed) {
            float k = 0.75f + 0.25f * sinf(shrinePulse);
            DrawCircle((int)sx, (int)sy - 42, 18 + 3 * sinf(shrinePulse), Fade({ 255, 40, 70, 255 }, 0.18f));
            DrawCircle((int)sx, (int)sy - 42, 9, { 255, (unsigned char)(90 * k), (unsigned char)(100 * k), 255 });
            DrawCircleLines((int)sx, (int)sy - 42, 9, { 255, 170, 180, 255 });
            const char* bsLbl = L("BLOOD SHRINE", "BỆ THỜ HUYẾT TẾ");
            DrawText(bsLbl, (int)sx - MeasureText(bsLbl, 14) / 2, (int)sy + 34, 14, { 255, 120, 130, 255 });
        } else {
            DrawCircle((int)sx, (int)sy - 42, 8, { 64, 50, 56, 255 });
        }
    }

    // Đài Giao Ước Bóng Tối
    if (hasBloodPact) {
        float bx = bloodPactPos.x, by = bloodPactPos.y;
        float tm = (float)GetTime();
        DrawEllipse((int)bx, (int)by + 24, 28, 9, { 0, 0, 0, 105 });
        if (!bloodPactUsed) {
            DrawCircle((int)bx, (int)by + 4, 38.0f, Fade({ 190, 25, 65, 255 }, 0.14f + 0.06f * sinf(tm * 4.0f)));
            DrawCircleLines((int)bx, (int)by + 4, 38.0f, { 225, 55, 95, 255 });
        }
        DrawRectangle((int)bx - 16, (int)by - 16, 32, 34, bloodPactUsed ? Color{ 55, 48, 58, 255 } : Color{ 82, 26, 44, 255 });
        DrawRectangleLines((int)bx - 16, (int)by - 16, 32, 34, bloodPactUsed ? GRAY : Color{ 255, 95, 130, 255 });
        if (!bloodPactUsed) {
            Diamond({ bx, by - 28.0f + sinf(tm * 3.5f) * 3.0f }, 8.0f, 12.0f, { 255, 65, 110, 255 }, GOLD);
            const char* pLbl = L("[F] BLOOD PACT (-14% MaxHP -> Relic & +10 DMG)", "[F] GIAO ƯỚC MÁU (-14% Máu -> Thánh Vật & +10 ST)");
            if (Vector2Distance(player.pos, bloodPactPos) < 110.0f) {
                DrawText(pLbl, (int)bx - MeasureText(pLbl, 14) / 2, (int)by + 30, 14, { 255, 145, 165, 255 });
            } else {
                const char* sLbl = L("DARK PACT", "GIAO ƯỚC");
                DrawText(sLbl, (int)bx - MeasureText(sLbl, 13) / 2, (int)by + 30, 13, { 220, 105, 125, 255 });
            }
        }
    }

    // Sự kiện ngẫu nhiên trong Hầm Ngục: Lồng Giam Cứu Người Đẹp (mở khóa Cưới Vợ)
    if (hasCaptiveEvent) {
        float tm = (float)GetTime();
        int cIdx = std::max(0, std::min(3, captiveSpouseIdx));
        const SpouseDef& sp = kSpouses[cIdx];
        Vector2 cp = captivePos;
        bool nearC = Vector2Distance(player.pos, cp) < 92.0f;
        DrawEllipse((int)cp.x, (int)cp.y + 22, 34.0f, 11.0f, Fade(BLACK, 0.45f));
        if (!captiveRescued) {
            DrawCircleLines((int)cp.x, (int)cp.y, 44.0f, Fade(sp.dressCol, 0.45f + 0.25f * sinf(tm * 4.0f)));
            DrawDetailedNpcModel(cp, cIdx, sp.dressCol, GOLD, { 38, 28, 32, 255 }, true, tm * 0.5f);
            // Song sắt lồng giam bao quanh
            DrawRectangleLinesEx({ cp.x - 24.0f, cp.y - 30.0f, 48.0f, 52.0f }, 2.5f, { 155, 165, 185, 255 });
            for (int b = -16; b <= 16; b += 8) {
                DrawLineEx({ cp.x + b, cp.y - 30.0f }, { cp.x + b, cp.y + 22.0f }, 2.0f, { 115, 125, 145, 230 });
            }
            DrawSurfacePrompt({ cp.x, cp.y + 28.0f }, SpouseName(cIdx),
                              nearC ? L("[F] RESCUE FROM CAGE!", "[F] GIẢI CỨU KHỎI LỒNG GIAM!") : L("Dungeon Rescue Event", "Sự Kiện Giải Cứu [F]"),
                              sp.dressCol, nearC);
        } else {
            DrawRectangleLinesEx({ cp.x - 24.0f, cp.y - 30.0f, 48.0f, 52.0f }, 1.5f, Fade(GRAY, 0.35f));
            DrawSurfacePrompt({ cp.x, cp.y + 24.0f }, L("RESCUED!", "ĐÃ GIẢI CỨU!"), SpouseName(cIdx), GOLD, false);
        }
    }

    // Exit Portals (tối đa 3 cổng rẽ nhánh sau khi dọn phòng)
    if (portalOpen) {
        float tm = (float)GetTime();
        float pulse = sinf(tm * 6.0f) * 5.0f;
        for (const auto& pc : portalChoices) {
            BeginBlendMode(BLEND_ADDITIVE);
            CircleGrad(pc.pos.x, pc.pos.y, 58.0f + pulse, Fade(pc.color, 0.45f), Fade(pc.color, 0.0f));
            EndBlendMode();
            DrawRing(pc.pos, 32.0f, 36.0f, tm * 70.0f, tm * 70.0f + 270.0f, 24, Fade(pc.color, 0.85f));
            DrawRing(pc.pos, 23.0f, 27.0f, -tm * 110.0f, -tm * 110.0f + 260.0f, 20, Fade(WHITE, 0.85f));
            for (int i = 0; i < 8; ++i) {
                float a = tm * 2.2f + i * (PI / 4.0f);
                DrawCircleV({ pc.pos.x + cosf(a) * 34.0f, pc.pos.y + sinf(a) * 34.0f }, 3.0f, WHITE);
            }
            DrawCircle((int)pc.pos.x, (int)pc.pos.y, 19.0f, Fade(pc.color, 0.85f));
            DrawCircle((int)pc.pos.x, (int)pc.pos.y, 11.0f, WHITE);
            OutlinedText(pc.title.c_str(), (int)pc.pos.x - MeasureText(pc.title.c_str(), 14) / 2, (int)pc.pos.y + 39, 14, pc.color, { 12, 14, 24, 255 });
            DrawText(pc.subtitle.c_str(), (int)pc.pos.x - MeasureText(pc.subtitle.c_str(), 12) / 2, (int)pc.pos.y + 55, 12, WHITE);
        }
    }

    // Ground Effects (Traps, Consecrated Ground, Void Vortex, Phoenix Rain, Falling Meteors/Arrows)
    for (const auto& ge : groundEffects) {
        float tm = (float)GetTime();
        float prog = 1.0f - (ge.timer / std::max(0.01f, ge.maxTimer));
        if (ge.kind == 0) { // Bẫy nổ Ranger
            DrawCircleV(ge.pos, ge.radius, Fade(ge.color, 0.07f));
            DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius, Fade(ge.color, 0.45f));
            DrawRing(ge.pos, 18.0f, 22.0f, 0.0f, prog * 360.0f, 18, ORANGE);
            DrawCircleV(ge.pos, 11.0f, { 40, 90, 60, 240 });
            DrawCircleV(ge.pos, 5.5f, (sinf(tm * 24.0f) > 0) ? YELLOW : ORANGE);
        } else if (ge.kind == 1) { // Thánh vực Knight
            float alpha = std::min(1.0f, ge.timer);
            DrawCircleV(ge.pos, ge.radius, Fade(GOLD, 0.10f * alpha));
            DrawRing(ge.pos, ge.radius - 6.0f, ge.radius, tm * 45.0f, tm * 45.0f + 300.0f, 36, Fade(GOLD, 0.70f * alpha));
            DrawRing(ge.pos, ge.radius * 0.55f, ge.radius * 0.57f, -tm * 60.0f, -tm * 60.0f + 260.0f, 28, Fade(WHITE, 0.55f * alpha));
            // Đại kiếm ánh sáng giáng xuống ở 0.7s đầu
            if (ge.timer > ge.maxTimer - 0.7f) {
                float k = 1.0f - (ge.maxTimer - ge.timer) / 0.7f;
                float yOff = (1.0f - std::min(1.0f, (ge.maxTimer - ge.timer) / 0.18f)) * -260.0f;
                Vector2 tip = { ge.pos.x, ge.pos.y + yOff };
                DrawRectangle((int)tip.x - 12, (int)(tip.y - 220), 24, 210, Fade(GOLD, 0.85f * k));
                DrawRectangle((int)tip.x - 5, (int)(tip.y - 220), 10, 215, Fade(WHITE, 0.95f * k));
                DrawRectangle((int)tip.x - 42, (int)(tip.y - 165), 84, 14, Fade(GOLD, 0.90f * k));
                Tri({ tip.x - 12, tip.y - 10 }, { tip.x + 12, tip.y - 10 }, { tip.x, tip.y + 16 }, Fade(WHITE, k));
            }
        } else if (ge.kind == 2) { // Hố đen Mage / Blood Vortex Berserker
            float alpha = std::min(1.0f, ge.timer);
            DrawCircleV(ge.pos, ge.radius, Fade(ge.color, 0.12f * alpha));
            DrawRing(ge.pos, ge.radius - 5.0f, ge.radius, -tm * 80.0f, -tm * 80.0f + 310.0f, 36, Fade(ge.color, 0.78f * alpha));
            DrawRing(ge.pos, 45.0f, 54.0f, tm * 160.0f, tm * 160.0f + 260.0f, 20, Fade(WHITE, 0.75f * alpha));
            DrawCircleV(ge.pos, 26.0f, Fade({ 15, 8, 30, 255 }, 0.9f * alpha));
        } else if (ge.kind == 3) { // Vùng Mưa Tên / Mưa Kiếm / Thương Sấm
            float alpha = std::min(1.0f, ge.timer);
            DrawCircleV(ge.pos, ge.radius, Fade(ge.color, 0.09f * alpha));
            DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius, Fade(ge.color, 0.70f * alpha));
        } else if (ge.kind == 4) { // Thiên thạch rơi
            float rem = std::max(0.0f, ge.timer / ge.maxTimer);
            DrawCircleV(ge.pos, ge.radius, Fade(ge.color, 0.12f));
            DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius * (1.0f - rem * 0.5f), Fade(ge.color, 0.85f));
            Vector2 skyPos = { ge.pos.x + rem * 110.0f, ge.pos.y - rem * 240.0f };
            DrawLineEx(skyPos, { skyPos.x + 28.0f, skyPos.y - 58.0f }, 10.0f, Fade(ge.color, 0.7f));
            DrawCircleV(skyPos, 15.0f, ge.color);
            DrawCircleV(skyPos, 8.0f, WHITE);
        } else if (ge.kind == 5) { // Mũi tên / Kiếm / Thương từ trời rơi
            float rem = std::max(0.0f, ge.timer / ge.maxTimer);
            DrawCircleV(ge.pos, ge.radius, Fade(ge.color, 0.10f));
            DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius * (1.0f - rem * 0.5f), Fade(ge.color, 0.85f));
            Vector2 skyPos = { ge.pos.x - rem * 45.0f, ge.pos.y - rem * 210.0f };
            DrawLineEx(skyPos, { skyPos.x - 14.0f, skyPos.y - 42.0f }, 5.5f, ge.color);
            DrawLineEx(skyPos, { skyPos.x - 10.0f, skyPos.y - 30.0f }, 2.5f, WHITE);
        }
    }

    // Loots: tinh thể XP, đồng vàng bát giác, bình máu ruby
    for (const auto& l : loots) {
        float bob = sinf((float)GetTime() * 4.5f + l.pos.x * 0.1f) * 2.5f;
        Vector2 lp = { l.pos.x, l.pos.y + bob };
        if (l.type == 0) {
            Diamond(lp, 6.2f, 9.0f, SKYBLUE, WHITE);
        } else if (l.type == 1) {
            DrawPoly(lp, 8, 8.2f, 22.5f, GOLD);
            DrawPolyLinesEx(lp, 8, 8.2f, 22.5f, 1.5f, { 120, 80, 10, 255 });
            DrawRectangle((int)lp.x - 1, (int)lp.y - 4, 2, 8, { 255, 245, 170, 255 });
        } else {
            QuadOut({ lp.x - 5.5f, lp.y - 3.0f }, { lp.x + 5.5f, lp.y - 3.0f }, { lp.x + 6.5f, lp.y + 7.0f }, { lp.x - 6.5f, lp.y + 7.0f }, { 225, 45, 70, 255 }, WHITE, 1.2f);
            DrawRectangle((int)lp.x - 2, (int)lp.y - 7, 4, 4, { 205, 215, 230, 255 });
            DrawRectangle((int)lp.x - 3, (int)lp.y - 9, 6, 2, GOLD);
        }
    }

    // Ghost Trails: bóng dáng nhân vật khi lướt
    for (const auto& g : ghosts) {
        float a = (g.timer / g.maxTimer) * 0.45f;
        Color gc = Fade(HeroColor(), a);
        float gf = g.facingLeft ? -1.0f : 1.0f;
        Tri({ g.pos.x - gf * 4.0f, g.pos.y - 8.0f }, { g.pos.x - gf * 22.0f, g.pos.y - 2.0f }, { g.pos.x - gf * 18.0f, g.pos.y + 14.0f }, Fade(gc, a * 0.7f));
        Quad({ g.pos.x - 9.0f, g.pos.y - 6.0f }, { g.pos.x + 9.0f, g.pos.y - 6.0f }, { g.pos.x + 7.0f, g.pos.y + 11.0f }, { g.pos.x - 7.0f, g.pos.y + 11.0f }, gc);
        Quad({ g.pos.x - 7.5f, g.pos.y - 20.0f }, { g.pos.x + 7.5f, g.pos.y - 20.0f }, { g.pos.x + 6.5f, g.pos.y - 7.0f }, { g.pos.x - 6.5f, g.pos.y - 7.0f }, gc);
    }

    // Slashes: vệt cung kiếm & kỹ năng
    for (const auto& sl : slashes) {
        float rad = sl.angle * DEG2RAD;
        float k = sl.timer / sl.maxTimer;
        Vector2 centre = Vector2Subtract(sl.pos, { cosf(rad) * 35.0f, sinf(rad) * 35.0f });
        if (sl.kind == WP_SPEAR) {
            Vector2 dirv = { cosf(rad), sinf(rad) };
            Vector2 tipv = Vector2Add(centre, Vector2Scale(dirv, 140.0f));
            DrawLineEx(Vector2Add(centre, Vector2Scale(dirv, 20.0f)), tipv, 10.0f * k + 2.0f, Fade(sl.color, 0.5f * k));
            DrawLineEx(Vector2Add(centre, Vector2Scale(dirv, 20.0f)), tipv, 4.0f * k + 1.0f, Fade(WHITE, 0.95f * k));
        } else if (sl.kind == WP_AXE || sl.kind == WP_SCYTHE) {
            Color arcC = (sl.kind == WP_SCYTHE) ? Color{ 195, 110, 255, 255 } : Color{ 255, 190, 110, 255 };
            float span = (sl.kind == WP_SCYTHE) ? 110.0f : 85.0f;
            DrawRing(centre, 40.0f, 76.0f, sl.angle - span, sl.angle + span, 30, Fade(sl.color, 0.8f * k));
            DrawRing(centre, 32.0f, 46.0f, sl.angle - span * 0.8f, sl.angle + span * 0.8f, 26, Fade(arcC, 0.7f * k));
        } else if (sl.kind == WP_HAMMER) {
            DrawRing(centre, 36.0f, 68.0f, sl.angle - 75.0f, sl.angle + 75.0f, 24, Fade({ 255, 195, 85, 255 }, 0.85f * k));
            DrawRing(centre, 28.0f, 40.0f, sl.angle - 55.0f, sl.angle + 55.0f, 18, Fade(WHITE, 0.8f * k));
        } else if (sl.kind == WP_DAGGER) {
            DrawRing(centre, 34.0f, 44.0f, sl.angle - 42.0f, sl.angle + 42.0f, 18, Fade(sl.color, 0.85f * k));
        } else if (sl.kind == WP_BOW || sl.kind == WP_STAFF) {
            Vector2 dirv = { cosf(rad), sinf(rad) };
            DrawCircleV(Vector2Add(centre, Vector2Scale(dirv, 24.0f)), 14.0f * k, Fade(sl.color, 0.55f * k));
        } else if (sl.kind == 10) { // Shield Bash
            DrawRing(centre, 48.0f, 95.0f, sl.angle - 70.0f, sl.angle + 70.0f, 24, Fade(GOLD, 0.85f * k));
        } else if (sl.kind == 11) { // Shadow Step Cross Slash
            DrawRing(sl.pos, 45.0f, 118.0f, sl.angle - 95.0f, sl.angle + 95.0f, 26, Fade({ 255, 75, 145, 255 }, 0.85f * k));
            DrawRing(sl.pos, 60.0f, 95.0f, sl.angle + 85.0f, sl.angle + 275.0f, 26, Fade({ 180, 75, 255, 255 }, 0.75f * k));
        } else if (sl.kind == 12) { // Blade Storm / Phantom Blades
            DrawRing(sl.pos, 110.0f, 178.0f, sl.angle, sl.angle + 135.0f, 22, Fade(sl.color, 0.75f * k));
            DrawRing(sl.pos, 110.0f, 178.0f, sl.angle + 180.0f, sl.angle + 315.0f, 22, Fade(WHITE, 0.65f * k));
        } else {
            DrawRing(centre, 36.0f, 52.0f, sl.angle - 60.0f, sl.angle + 60.0f, 24, Fade(sl.color, 0.85f * k));
            DrawRing(centre, 30.0f, 38.0f, sl.angle - 45.0f, sl.angle + 45.0f, 20, Fade({ 120, 220, 255, 255 }, 0.7f * k));
        }
    }

    // Novas
    for (const auto& n : novas) {
        float a = n.timer / n.maxTimer;
        DrawCircleLines((int)n.pos.x, (int)n.pos.y, n.radius, Fade(n.color, a));
        DrawCircleLines((int)n.pos.x, (int)n.pos.y, n.radius * 0.9f, Fade(WHITE, a * 0.7f));
    }

    // Enemies
    for (const auto& e : enemies) {
        if (e.hp <= 0) continue;
        // Shadow
        DrawEllipse((int)e.pos.x, (int)e.pos.y + e.radius * 0.8f, e.radius, e.radius * 0.4f, { 0, 0, 0, 90 });

        DrawEnemyBody(e);

        // Health bar + Elite affix tag
        float barW = e.radius * 2.2f;
        float pct = std::max(0.0f, e.hp / e.maxHp);
        DrawRectangle((int)(e.pos.x - barW / 2), (int)(e.pos.y - e.radius - 12), (int)barW, 5, BLACK);
        DrawRectangle((int)(e.pos.x - barW / 2), (int)(e.pos.y - e.radius - 12), (int)(barW * pct), 5, RED);
        if (e.isElite && e.eliteAffix >= 1 && e.eliteAffix <= 4) {
            static const char* affNames[5] = { "", "VAMPIRIC", "BLINK", "MOLTEN", "FROSTWARD" };
            static const Color affCols[5]  = { WHITE, { 255, 80, 110, 255 }, { 195, 115, 255, 255 }, ORANGE, SKYBLUE };
            const char* an = affNames[e.eliteAffix];
            DrawText(an, (int)e.pos.x - MeasureText(an, 10) / 2, (int)(e.pos.y - e.radius - 24), 10, affCols[e.eliteAffix]);
        }
    }

    // Projectiles: mũi tên, phi tiêu, sao chổi lửa & tinh thể phép định hướng
    for (const auto& p : projectiles) {
        Vector2 dir = Vector2Length(p.vel) > 0.1f ? Vector2Normalize(p.vel) : Vector2{ 1, 0 };
        Vector2 perp = { -dir.y, dir.x };
        if (p.kind == 1) { // Mũi tên có đuôi lông & mũi nhọn tam giác
            Vector2 tail = Vector2Subtract(p.pos, Vector2Scale(dir, 18.0f));
            DrawLineEx(tail, p.pos, 4.5f, Fade(p.color, 0.35f));
            DrawLineEx(tail, p.pos, 2.5f, p.color);
            Tri(Vector2Add(p.pos, Vector2Scale(dir, 7.0f)), Vector2Add(p.pos, Vector2Scale(perp, 3.8f)), Vector2Subtract(p.pos, Vector2Scale(perp, 3.8f)), WHITE);
            DrawLineEx(tail, Vector2Add(tail, Vector2Add(Vector2Scale(dir, -4.0f), Vector2Scale(perp, 4.0f))), 1.5f, p.color);
            DrawLineEx(tail, Vector2Add(tail, Vector2Add(Vector2Scale(dir, -4.0f), Vector2Scale(perp, -4.0f))), 1.5f, p.color);
        } else if (p.kind == 3) { // Phi tiêu 4 cánh xoay
            float spin = (float)GetTime() * 18.0f;
            for (int k = 0; k < 4; ++k) {
                float a = spin + k * (PI / 2.0f);
                float a2 = a + 0.45f;
                Tri(p.pos, { p.pos.x + cosf(a) * 10.0f, p.pos.y + sinf(a) * 10.0f }, { p.pos.x + cosf(a2) * 5.0f, p.pos.y + sinf(a2) * 5.0f }, p.color);
            }
            Diamond(p.pos, 3.2f, 3.2f, WHITE, p.color);
        } else if (p.kind == 4) { // Cầu lửa sao chổi rực cháy
            Vector2 tail = Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 2.8f));
            Tri(tail, Vector2Add(p.pos, Vector2Scale(perp, p.radius * 1.1f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 1.1f)), Fade(ORANGE, 0.55f));
            Tri(Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 1.7f)), Vector2Add(p.pos, Vector2Scale(perp, p.radius * 0.65f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 0.65f)), p.color);
            Quad(Vector2Add(p.pos, Vector2Scale(dir, p.radius * 1.4f)), Vector2Add(p.pos, Vector2Scale(perp, p.radius * 0.9f)),
                 Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 0.8f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 0.9f)), YELLOW);
            Diamond(p.pos, p.radius * 0.6f, p.radius * 0.6f, WHITE, ORANGE);
        } else if (p.kind == 6) { // Kiếm khí trăng khuyết (Spellblade / Berserker)
            float ang = atan2f(dir.y, dir.x) * RAD2DEG;
            Vector2 arcC = Vector2Subtract(p.pos, Vector2Scale(dir, 14.0f));
            DrawRing(arcC, p.radius + 5.0f, p.radius + 15.0f, ang - 65.0f, ang + 65.0f, 16, Fade(p.color, 0.85f));
            DrawRing(arcC, p.radius + 11.0f, p.radius + 16.0f, ang - 52.0f, ang + 52.0f, 16, WHITE);
            Diamond(Vector2Add(p.pos, Vector2Scale(dir, 3.0f)), 4.2f, 4.2f, WHITE, p.color);
        } else { // Tinh thể năng lượng định hướng
            Vector2 tail = Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 2.4f));
            Tri(tail, Vector2Add(p.pos, Vector2Scale(perp, p.radius * 0.9f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 0.9f)), Fade(p.color, 0.42f));
            Vector2 head = Vector2Add(p.pos, Vector2Scale(dir, p.radius * 1.35f));
            QuadOut(head, Vector2Add(p.pos, Vector2Scale(perp, p.radius * 0.85f)),
                    Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 0.95f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 0.85f)),
                    p.color, WHITE, 1.2f);
        }
    }
    }

    // Player & Danh hiệu cấp độ (20/40/60/80/100)
    if (CurrentTitleTier() >= 1) {
        float tm = (float)GetTime();
        Color tCol = CurrentLevelTitleColor();
        int sides = 4 + CurrentTitleTier();
        DrawPolyLinesEx(player.pos, sides, 24.0f + CurrentTitleTier() * 1.5f, -tm * (35.0f + CurrentTitleTier() * 12.0f), 1.6f, Fade(tCol, 0.55f));
    }
    if (comboCount >= 10) {
        float tm = (float)GetTime();
        DrawPolyLinesEx(player.pos, 6, 30.0f + sinf(tm * 12.0f) * 2.0f, tm * 120.0f, 2.2f, Fade(ORANGE, 0.78f));
    }
    DrawPartyChildren(); // con cái theo cha rèn luyện
    DrawEllipse((int)player.pos.x, (int)player.pos.y + 16, 20, 8, { 0, 0, 0, 100 }); // Shadow
    DrawPlayerSprite();
    if (CurrentTitleTier() >= 1 || inSurface) {
        const char* ltTxt = CurrentLevelTitle();
        OutlinedText(ltTxt, (int)player.pos.x - MeasureText(ltTxt, 11) / 2, (int)player.pos.y - 38, 11, CurrentLevelTitleColor(), { 12, 14, 22, 240 });
    }

    if (!inSurface) DrawItemDrops();

    // Hạt
    for (const auto& pt : particles) {
        float k = pt.life / pt.maxLife;
        DrawCircleV(pt.pos, pt.size * (0.3f + 0.7f * k), Fade(pt.color, k * k));
    }

    // Damage Popups
    for (const auto& p : popups) {
        DrawText(p.text.c_str(), (int)p.pos.x - 10, (int)p.pos.y, 18, p.color);
    }

    EndMode2D();

    if (settings.lighting) ApplyLighting();

    // Vignette + viền đỏ khi bị đau (dùng chung cho cả Hầm Ngục & Mặt Đất)
    if (settings.vignette) {
        Color edge = LerpColor({ 0, 0, 0, 120 }, { 170, 0, 20, 200 }, damagePulse);
        Color clear = { edge.r, edge.g, edge.b, 0 };
        int bh = 150, bw = 240;
        DrawRectangleGradientV(0, 0, 1280, bh, edge, clear);
        DrawRectangleGradientV(0, 720 - bh, 1280, bh, clear, edge);
        DrawRectangleGradientH(0, 0, bw, 720, edge, clear);
        DrawRectangleGradientH(1280 - bw, 0, bw, 720, clear, edge);
    }

    // ── 2D Screen Space UI ──
    Color hCol = HeroColor();
    const char* hTitle = HeroTitle();
    std::string badgeLeft = (isNoble ? std::string(L("[NOBLE] ", "[QT] ")) : std::string(""))
                          + (!heroGivenName.empty() ? (heroGivenName + " - ") : (generation > 1 ? (std::string(L("Gen", "Đ")) + std::to_string(generation) + " ") : std::string("")))
                          + hTitle;
    // Top-Left: Class / Subclass Badge, HP (with damage lag), XP, Rage, Hunger, Gold
    DrawRectangle(20, 14, 320, 22, { 18, 22, 34, 225 });
    DrawRectangleLines(20, 14, 320, 22, Fade(isNoble ? GOLD : hCol, 0.85f));
    DrawText(badgeLeft.c_str(), 28, 17, 15, isNoble ? GOLD : hCol);
    std::string clsRight = (player.level >= kMaxPlayerLevel)
                         ? std::string(L("LVL 100 MAX", "CẤP 100 MAX"))
                         : (std::string(L("LVL ", "CẤP ")) + std::to_string(player.level) + "/100");
    DrawText(clsRight.c_str(), 332 - MeasureText(clsRight.c_str(), 14), 17, 14, player.level >= kMaxPlayerLevel ? GOLD : CurrentLevelTitleColor());

    DrawRectangle(20, 40, 320, 24, { 35, 38, 48, 220 });
    float lagPct = std::max(0.0f, std::min(1.0f, player.hpDisplay / std::max(1.0f, player.maxHp)));
    float hpPct = std::max(0.0f, std::min(1.0f, player.hp / std::max(1.0f, player.maxHp)));
    DrawRectangle(22, 42, (int)(316 * lagPct), 20, { 245, 220, 220, 210 });
    DrawRectangle(22, 42, (int)(316 * hpPct), 20, { 225, 48, 62, 255 });
    std::string hpTxt = std::string(L("HP ", "MÁU ")) + std::to_string((int)player.hp) + " / " + std::to_string((int)player.maxHp);
    DrawText(hpTxt.c_str(), 180 - MeasureText(hpTxt.c_str(), 16) / 2, 44, 16, WHITE);

    DrawRectangle(20, 68, 320, 15, { 35, 38, 48, 220 });
    bool atMaxLv = (player.level >= kMaxPlayerLevel);
    float xpPct = atMaxLv ? 1.0f : std::min(1.0f, (float)player.xp / std::max(1, player.xpNext));
    DrawRectangle(22, 70, (int)(316 * xpPct), 11, atMaxLv ? Color{ 165, 112, 18, 255 } : Color{ 42, 118, 195, 255 });
    std::string xpTxt = atMaxLv
                      ? std::string(CurrentLevelTitle()) + L(" (LV.100 MAX)", " (CẤP 100 MAX)")
                      : (std::string(L("XP ", "KN ")) + std::to_string(player.xp) + "/" + std::to_string(player.xpNext) + "  -  " + CurrentLevelTitle());
    DrawText(xpTxt.c_str(), 180 - MeasureText(xpTxt.c_str(), 11) / 2, 69, 11, atMaxLv ? GOLD : WHITE);

    // Thanh Nộ (Rage Bar)
    DrawRectangle(20, 86, 320, 16, { 35, 26, 22, 225 });
    float ragePct = std::min(1.0f, player.rage / player.maxRage);
    bool rageReady = (player.rage >= player.maxRage);
    Color rageCol = rageReady ? ((int)(GetTime() * 8.0f) % 2 == 0 ? GOLD : ORANGE) : Color{ 235, 110, 35, 255 };
    DrawRectangle(22, 88, (int)(316 * ragePct), 12, rageCol);
    DrawRectangleLines(20, 86, 320, 16, rageReady ? GOLD : Color{ 90, 60, 40, 255 });
    std::string rageTxt = rageReady ? L("RAGE 100/100  [R] ULTIMATE READY!", "NỘ 100/100  [R] TUYỆT KỸ SẴN SÀNG!") : (std::string(L("RAGE ", "NỘ ")) + std::to_string((int)player.rage) + " / 100");
    DrawText(rageTxt.c_str(), 180 - MeasureText(rageTxt.c_str(), 12) / 2, 88, 12, WHITE);

    int yOff = 0;
    if (surfaceUnlocked) {
        yOff = 22;
        DrawRectangle(20, 106, 320, 16, { 30, 24, 16, 225 });
        float huPct = std::max(0.0f, std::min(1.0f, hunger / maxHunger));
        Color huCol = (hunger >= 70.0f) ? Color{ 100, 225, 120, 255 } : ((hunger < 25.0f) ? RED : Color{ 235, 175, 60, 255 });
        DrawRectangle(22, 108, (int)(316 * huPct), 12, huCol);
        DrawRectangleLines(20, 106, 320, 16, hunger < 25.0f ? RED : Color{ 110, 88, 52, 255 });
        std::string huTxt = std::string(L("HUNGER ", "ĐỘ NO ")) + std::to_string((int)ceilf(hunger)) + "/100   "
                          + L("[H] Ration:", "[H] Lương:") + std::to_string(foodRations);
        DrawText(huTxt.c_str(), 180 - MeasureText(huTxt.c_str(), 12) / 2, 108, 12, WHITE);
    }

    std::string goldTxt = std::string(L("Gold: ", "Vàng: ")) + std::to_string(player.gold);
    DrawText(goldTxt.c_str(), 24, 108 + yOff, 18, GOLD);

    // Relics Row
    const char* relPrefix = L("Relics: ", "Thánh Tích: ");
    DrawText(relPrefix, 24, 132 + yOff, 15, SKYBLUE);
    if (player.relics.empty()) {
        DrawText(L("None", "Chưa có"), 24 + MeasureText(relPrefix, 15), 132 + yOff, 15, SKYBLUE);
    } else {
        int rx = 24 + MeasureText(relPrefix, 15);
        for (const auto& r : player.relics) {
            DrawCircle(rx + 6, 139 + yOff, 5.5f, r.color);
            DrawCircleLines(rx + 6, 139 + yOff, 5.5f, WHITE);
            DrawText(r.name.c_str(), rx + 16, 132 + yOff, 15, r.color);
            rx += 16 + MeasureText(r.name.c_str(), 15) + 12;
        }
    }

    // Top-Center: Surface / Floor, Time & Age Header
    std::string timeSub = TextFormat(L("Day %d  %s (%s)  |  Age %d  |  Gen %d", "Ngày %d  %s (%s)  |  Tuổi %d  |  Đời %d"),
                                     dayCount, ClockString().c_str(), TimePhaseName(), heroAge, generation);
    if (inSurface) {
        const char* surfTitle = L(biome.nameEn, biome.nameVi);
        DrawText(surfTitle, 1280 / 2 - MeasureText(surfTitle, 22) / 2, 12, 22, biome.trimCol);
        std::string subS = timeSub + "  |  " + HouseName(houseTier) + (isNoble ? L(" [NOBLE]", " [QUÝ TỘC]") : "");
        DrawText(subS.c_str(), 1280 / 2 - MeasureText(subS.c_str(), 13) / 2, 39, 13, GOLD);
    } else {
        std::string floorStr = std::string(L("FLOOR ", "TẦNG ")) + std::to_string(floor);
        DrawText(floorStr.c_str(), 1280 / 2 - MeasureText(floorStr.c_str(), 28) / 2, 10, 28, RAYWHITE);
        std::string subD = std::string(L(biome.nameEn, biome.nameVi)) + "  |  " + timeSub;
        DrawText(subD.c_str(), 1280 / 2 - MeasureText(subD.c_str(), 13) / 2, 42, 13, biome.accentCol);
    }

    if (comboCount > 1 && !inSurface) {
        std::string comboStr = "COMBO x" + std::to_string(comboCount) + "!";
        Color cCol = comboCount >= 20 ? RED : (comboCount >= 10 ? ORANGE : YELLOW);
        DrawText(comboStr.c_str(), 1280 - 240, 154, 26, cCol);
        if (comboCount >= 10) {
            DrawText(L("FRENZY OVERDRIVE!", "CUỒNG NỘ BÙNG NỔ!"), 1280 - 240, 182, 14, GOLD);
            DrawText(L("+20% ATK SPD  +18% SPD", "+20% Tốc Đánh  +18% Chạy"), 1280 - 240, 198, 12, { 255, 200, 110, 255 });
        }
    }

    // Style Rank Badge (D..SSS), Perfect Dodge / Next Crit & Boss Essence / Bounty
    if (!inSurface) {
        Color sCol = StyleRankColor();
        DrawRectangle(1280 - 240, 218, 218, 44, { 14, 18, 30, 210 });
        DrawRectangleLines(1280 - 240, 218, 218, 44, sCol);
        DrawText(StyleRankLabel(), 1280 - 228, 224, 28, sCol);
        DrawText(TextFormat(L("STYLE  +%d%% ATK  +%d%% LOOT", "PHONG ĐỘ  +%d%% ĐÁNH  +%d%% RƠI"),
                            (int)roundf(styleRank * 3.5f), styleRank * 6),
                 1280 - 180, 224, 12, RAYWHITE);
        DrawRectangle(1280 - 180, 244, 146, 8, { 28, 34, 52, 255 });
        DrawRectangle(1280 - 180, 244, (int)(146.0f * Clamp(stylePoints / 100.0f, 0.0f, 1.0f)), 8, sCol);
        int hyRight = 268;
        if (nextHitCrit) {
            DrawText(L("SHADOW CRIT READY!", "SẴN SÀNG CHÍ MẠNG!"), 1280 - 240, hyRight, 14, GOLD);
            hyRight += 18;
        }
        if (bossEssence > 0) {
            DrawText(TextFormat(L("Boss Essence: %d", "Tinh Hoa Trùm: %d"), bossEssence), 1280 - 240, hyRight, 13, { 235, 165, 255, 255 });
            hyRight += 17;
        }
        if (bountyActive) {
            const char* bDesc = (bountyKind == 0) ? L("Elites", "Tinh Anh") : (bountyKind == 1 ? L("Reactions", "Phản Ứng") : L("Boss/Goblin", "Trùm/Yêu Tinh"));
            DrawText(TextFormat(L("Bounty (%s): %d/%d", "Truy Nã (%s): %d/%d"), bDesc, bountyProgress, bountyTarget),
                     1280 - 240, hyRight, 13, (bountyProgress >= bountyTarget) ? Color{ 120, 245, 150, 255 } : SKYBLUE);
        }
    }

    // Trang bị đang đeo (HUD)
    for (int i = 0; i < 4; ++i) {
        int hx = 24 + i * 36, hy = 156 + yOff;
        Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 60, 68, 90, 255 };
        DrawRectangle(hx, hy, 30, 30, { 14, 18, 30, 210 });
        DrawRectangleLines(hx, hy, 30, 30, rc);
        DrawItemIcon(i, { hx + 15.0f, hy + 15.0f }, 17.0f, hasEquip[i] ? rc : Color{ 70, 78, 100, 255 }, hasEquip[i] ? equipped[i].weaponType : 0);
    }
    DrawText(L("[I] Gear & Skills   [Y] Family Tree", "[I] Đồ & Kỹ Năng   [Y] Gia Phả"), 24 + 4 * 36 + 4, 164 + yOff, 14, { 150, 165, 200, 255 });

    // Tuổi & tiến độ năm (1 năm = 3 phút) + Đoàn con theo rèn luyện
    {
        int ay = 192 + yOff;
        std::string ageTxt = TextFormat(L("AGE %d  %s  (%s)", "TUỔI %d  %s  (%s)"), heroAge, AgeStageName(), AgeEffectText().c_str());
        DrawText(ageTxt.c_str(), 24, ay, 12, AgeStageColor());
        DrawRectangle(24, ay + 15, 236, 5, { 26, 30, 46, 230 });
        DrawRectangle(24, ay + 15, (int)(236.0f * YearProgress()), 5, AgeStageColor());
        DrawPartyHud(ay + 26);
    }

    if (touchMode && state == PLAYING) DrawTouchControls();

    // Thanh máu boss
    bool bossBarActive = false;
    for (const auto& e : enemies) {
        if (e.type == BOSS && e.hp > 0) {
            bossBarActive = true;
            int bw = 460, bx = 1280 / 2 - bw / 2, by = 66;
            float pct = std::max(0.0f, e.hp / e.maxHp);
            DrawRectangle(bx - 3, by - 3, bw + 6, 22, { 10, 12, 20, 230 });
            DrawRectangle(bx, by, (int)(bw * pct), 16, (e.bossKind == 1) ? (e.enraged ? Color{ 255, 90, 30, 255 } : Color{ 255, 150, 40, 255 }) : (e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 190, 60, 200, 255 }));
            DrawRectangleLines(bx - 3, by - 3, bw + 6, 22, GOLD);
            const char* nm = (e.bossKind == 1)
                ? (e.enraged ? L("THE MOLTEN COLOSSUS - ENRAGED", "KHỔNG LỒ DUNG NHAM - CUỒNG NỘ") : L("THE MOLTEN COLOSSUS", "KHỔNG LỒ DUNG NHAM"))
                : (e.enraged ? L("THE VOID SOVEREIGN - ENRAGED", "CHÚA TỂ HƯ KHÔNG - CUỒNG NỘ") : L("THE VOID SOVEREIGN", "CHÚA TỂ HƯ KHÔNG"));
            DrawText(nm, 1280 / 2 - MeasureText(nm, 14) / 2, by + 1, 14, WHITE);
            break;
        }
    }
    if (trialActive) {
        const char* tt = L("TRIAL AMBUSH ACTIVE!", "ĐANG TRONG PHỤC KÍCH THỬ THÁCH!");
        DrawText(tt, 1280 / 2 - MeasureText(tt, 17) / 2, bossBarActive ? 92 : 66, 17, GOLD);
    }
    if (royalQuestActive && !isNoble) {
        std::string rqTxt = TextFormat(L("KING'S COMMISSION: %d/35 Monsters  |  %d/1 Boss", "ỦY THÁC CỦA VUA: %d/35 Quái  |  %d/1 Trùm"), royalQuestKills, royalQuestBosses);
        int rqY = inSurface ? 64 : (bossBarActive ? (trialActive ? 114 : 92) : (trialActive ? 88 : 66));
        DrawText(rqTxt.c_str(), 1280 / 2 - MeasureText(rqTxt.c_str(), 14) / 2, rqY, 14, ORANGE);
    }

    // Mini-Map Radar (Top-Right)
    if (showMiniMap) {
        int mapX = 1280 - 180;
        int mapY = 20;
        int mapW = 160;
        int mapH = 110;
        DrawRectangle(mapX, mapY, mapW, mapH, { 15, 20, 30, 220 });
        DrawRectangleLines(mapX, mapY, mapW, mapH, { 70, 90, 130, 255 });
        const char* radLbl = L("RADAR [M]", "BẢN ĐỒ [M]");
        DrawText(radLbl, mapX + mapW / 2 - MeasureText(radLbl, 11) / 2, mapY + 4, 11, SKYBLUE);

        float sX = (mapW - 16) / roomW;
        float sY = (mapH - 16) / roomH;
        float midX = mapX + mapW / 2.0f;
        float midY = mapY + mapH / 2.0f;

        // Player dot
        DrawCircle((int)(midX + player.pos.x * sX), (int)(midY + player.pos.y * sY), 3.5f, GREEN);
        if (inSurface) {
            if (surfaceRoom == 0) {
                Color bCols[4] = { ORANGE, SKYBLUE, Color{ 255, 135, 185, 255 }, GOLD };
                for (int bi = 0; bi < 4; ++bi) {
                    Vector2 bp = SurfaceLandmarkPos(bi);
                    DrawCircle((int)(midX + bp.x * sX), (int)(midY + bp.y * sY), 4.0f, bCols[bi]);
                }
                Vector2 dg = SurfaceLandmarkPos(4);
                DrawCircle((int)(midX + dg.x * sX), (int)(midY + dg.y * sY), 4.5f, Color{ 195, 120, 255, 255 });
            } else {
                Vector2 ex = SurfaceInteriorExitPos();
                DrawCircle((int)(midX + ex.x * sX), (int)(midY + ex.y * sY), 4.5f, SKYBLUE);
            }
        }
        // Trial Totem dot
        if (hasTrialTotem && !trialUsed) DrawCircle((int)(midX + trialPos.x * sX), (int)(midY + trialPos.y * sY), 3.5f, GOLD);
        // Portal dots
        if (portalOpen) {
            for (const auto& pc : portalChoices)
                DrawCircle((int)(midX + pc.pos.x * sX), (int)(midY + pc.pos.y * sY), 4.5f, pc.color);
        }
        // Enemy dots
        for (const auto& e : enemies) {
            if (e.hp <= 0) continue;
            DrawCircle((int)(midX + e.pos.x * sX), (int)(midY + e.pos.y * sY), e.type == BOSS ? 5.0f : 2.5f, e.type == BOSS ? GOLD : RED);
        }
    }

    // Thanh 5 ô kỹ năng ở giữa đáy màn hình (hoặc thanh phím nhanh trên Mặt Đất)
    if (inSurface && state == PLAYING) {
        const char* sBar = (surfaceRoom == 0)
            ? L("[F] Enter / Interact   [1..4] Quick Room   [Tab] Menu   [Y] Family Tree   [H] Ration",
                "[F] Vào Phòng / Tương Tác   [1..4] Vào Nhanh   [Tab] Bảng Chung   [Y] Gia Phả   [H] Lương")
            : L("[0] Town Square   [1..4] Switch Room   [F] Interact   [Tab] Room Menu   [Y] Tree   [H] Ration",
                "[0] Ra Quảng Trường   [1..4] Đổi Phòng   [F] Tương Tác   [Tab] Menu   [Y] Gia Phả   [H] Lương");
        int sbW = MeasureText(sBar, 14) + 28;
        DrawRectangle(1280 / 2 - sbW / 2, 720 - 44, sbW, 30, { 14, 20, 32, 230 });
        DrawRectangleLines(1280 / 2 - sbW / 2, 720 - 44, sbW, 30, GOLD);
        DrawText(sBar, 1280 / 2 - MeasureText(sBar, 14) / 2, 720 - 36, 14, WHITE);
    } else if (!touchMode) {
        struct SlotUI { const char* key; int iconKind; Color col; float cd; float maxCd; bool isUlt; bool ready; };
        std::vector<SlotUI> slots;
        slots.push_back({ L("PSV", "NT"), 0, hCol, 0.0f, 1.0f, false, true });
        slots.push_back({ "RMB", -1, YELLOW, player.dashTimer, player.dashCooldown, false, player.dashTimer <= 0.0f });
        slots.push_back({ "Q", 1, SKYBLUE, player.specialTimer, std::max(0.1f, player.specialCooldown), false, player.specialTimer <= 0.0f });
        slots.push_back({ "E", 2, { 110, 235, 175, 255 }, player.skill2Timer, std::max(0.1f, player.skill2Cooldown), false, player.skill2Timer <= 0.0f });
        if (player.subClass != 0) {
            slots.push_back({ "C", 4, { 245, 200, 95, 255 }, player.skill3Timer, std::max(0.1f, player.skill3Cooldown), false, player.skill3Timer <= 0.0f });
            slots.push_back({ "V", 5, { 225, 115, 255, 255 }, player.skill4Timer, std::max(0.1f, player.skill4Cooldown), false, player.skill4Timer <= 0.0f });
        }
        slots.push_back({ "R", 3, ORANGE, 0.0f, 1.0f, true, rageReady });

        const int count = (int)slots.size();
        const int slotW = (count > 5) ? 60 : 66;
        const int slotH = (count > 5) ? 60 : 66;
        const int gap = (count > 5) ? 8 : 10;
        const int dockX0 = 1280 / 2 - (slotW * count + gap * (count - 1)) / 2;
        const int dockY = 720 - (slotH + 14);
        for (int s = 0; s < count; ++s) {
            int sx = dockX0 + s * (slotW + gap);
            const SlotUI& ui = slots[s];
            DrawRectangle(sx, dockY, slotW, slotH, { 18, 22, 34, 230 });
            if (ui.isUlt) {
                int fillH = (int)(slotH * ragePct);
                DrawRectangle(sx, dockY + slotH - fillH, slotW, fillH, Fade(ui.ready ? GOLD : ORANGE, ui.ready ? 0.35f : 0.22f));
            }
            Color border = ui.ready ? (ui.isUlt ? ((int)(GetTime() * 8.0f) % 2 == 0 ? GOLD : WHITE) : ui.col) : Color{ 60, 68, 90, 255 };
            DrawRectangleLinesEx({ (float)sx, (float)dockY, (float)slotW, (float)slotH }, ui.ready && ui.isUlt ? 2.5f : 1.5f, border);
            DrawSkillIcon(player.heroClass, ui.iconKind, { sx + slotW / 2.0f, dockY + slotH * 0.43f }, (float)slotW * 0.54f, ui.ready ? ui.col : GRAY);
            if (ui.cd > 0.0f) {
                float pct = std::min(1.0f, ui.cd / std::max(0.01f, ui.maxCd));
                DrawRectangle(sx, dockY + (int)((1.0f - pct) * slotH), slotW, (int)(pct * slotH), Fade(BLACK, 0.72f));
                const char* cdt = TextFormat("%.1f", ui.cd);
                DrawText(cdt, sx + slotW / 2 - MeasureText(cdt, 17) / 2, dockY + 19, 17, WHITE);
            } else if (ui.isUlt && !ui.ready) {
                const char* rpt = TextFormat("%d%%", (int)player.rage);
                DrawText(rpt, sx + slotW / 2 - MeasureText(rpt, 13) / 2, dockY + slotH - 18, 13, GOLD);
            }
            DrawRectangle(sx + 2, dockY + 2, MeasureText(ui.key, 11) + 6, 14, { 10, 12, 20, 210 });
            DrawText(ui.key, sx + 5, dockY + 3, 11, ui.ready ? WHITE : GRAY);
        }
    }

    // Banner Toast
    if (bannerTimer > 0 && (state == PLAYING || state == LEVEL_UP)) {
        int bw = MeasureText(bannerText.c_str(), 26) + 60;
        DrawRectangle(1280 / 2 - bw / 2, 140, bw, 46, { 10, 15, 25, 230 });
        DrawRectangleLines(1280 / 2 - bw / 2, 140, bw, 46, GOLD);
        DrawText(bannerText.c_str(), 1280 / 2 - bw / 2 + 30, 150, 26, GOLD);
    }

    // ── Level Up Modal (hoặc Màn Chuyển Chức Nhánh Phụ ở Level 5) ──
    if (state == LEVEL_UP) {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
        if (subclassChoosing) {
            float tm = (float)GetTime();
            int hc = std::max(0, std::min(3, player.heroClass));
            const char* hdr = L("LEVEL 5 AWAKENING! CHOOSE YOUR SUBCLASS (6 SKILLS)", "THỨC TỈNH CẤP 5! CHỌN NHÁNH CHUYỂN CHỨC (6 KỸ NĂNG)");
            OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 30) / 2, 30, 30, GOLD, { 80, 42, 10, 255 });
            std::string sub = std::string(ClassDisplayName(hc)) + L(" Evolution — Permanent Class Branch, Signature Weapon & 6 Skills", " Tiến Hóa — Nhánh Chức Vĩnh Viễn, Vũ Khí Trấn Phái & 6 Kỹ Năng");
            DrawText(sub.c_str(), 1280 / 2 - MeasureText(sub.c_str(), 17) / 2, 72, 17, SKYBLUE);

            const float cw = 450.0f, gap = 36.0f, ch = 554.0f;
            const float sx = 1280 / 2.0f - (cw * 2 + gap) / 2.0f, cy = 116.0f;
            Vector2 mp = GetMousePosition();
            for (int i = 0; i < 2; ++i) {
                const SubclassDef& sc = kSubclasses[hc][i];
                const ClassTextVi& scVi = kSubclassesVi[hc][i];
                Rectangle r = { sx + i * (cw + gap), cy, cw, ch };
                bool hov = CheckCollisionPointRec(mp, r);
                DrawRectangleRec(r, hov ? Color{ 28, 36, 62, 252 } : Color{ 18, 24, 42, 246 });
                DrawRectangle((int)r.x, (int)r.y, (int)cw, 44, Fade(sc.color, hov ? 0.35f : 0.22f));
                DrawRectangleLinesEx(r, hov ? 3.0f : 2.0f, hov ? GOLD : sc.color);

                DrawText(TextFormat("[%d] %s", i + 1, SubDisplayName(hc, i + 1)), (int)r.x + 18, (int)r.y + 6, 21, sc.color);
                DrawText(settings.language == 1 ? scVi.role : sc.role, (int)r.x + 18, (int)r.y + 28, 12, { 215, 225, 245, 255 });

                // Xem trước nhân vật với vũ khí trấn phái mới
                Vector2 pv = { r.x + cw - 66.0f, r.y + 92.0f };
                DrawCircleV(pv, 38.0f, Fade(sc.color, 0.16f));
                DrawHeroModel(pv, hc, sc.signatureWeapon, { 0.85f, sinf(tm * 2.2f + i) * 0.3f }, false, tm * 6.0f, true, sinf(tm * 5.0f) * 0.3f, false, true, 0.0f, 0.0f, i + 1, 0.0f);

                DrawText(TextFormat(L("Signature: %s", "Trấn Phái: %s"), settings.language == 1 ? scVi.weaponTitle : sc.weaponTitle), (int)r.x + 18, (int)r.y + 52, 15, GOLD);
                DrawText(TextFormat(L("+%d HP  +%d DMG  +%.1f ARM  +%.2f ATK/s", "+%d Máu  +%d ST  +%.1f Giáp  +%.2f Đánh/s"), (int)sc.bonusHp, (int)sc.bonusDmg, sc.bonusArmor, sc.bonusAtkRate), (int)r.x + 18, (int)r.y + 72, 13, RAYWHITE);
                std::string lsStr = FmtPct(sc.bonusLifesteal * LifestealScale());
                DrawText(TextFormat(L("+%d SPD  +%d%% CRIT  +%s Lifesteal", "+%d Chạy  +%d%% Chí Mạng  +%s Hút Máu"), (int)sc.bonusSpeed, (int)roundf(sc.bonusCrit * 100), lsStr.c_str()), (int)r.x + 18, (int)r.y + 90, 13, SKYBLUE);
                DrawText(L("+35% HP Recovery & +30 Rage immediately", "Hồi ngay +35% Máu & +30 Nộ"), (int)r.x + 18, (int)r.y + 108, 12, { 120, 235, 160, 255 });

                const char* tags[6] = { L("PASSIVE", "NỘI TẠI"), L("SKILL [Q]", "CHIÊU [Q]"), L("SKILL [E]", "CHIÊU [E]"), L("SKILL [C]", "CHIÊU [C]"), L("SKILL [V]", "CHIÊU [V]"), L("ULTIMATE [R]", "TUYỆT KỸ [R]") };
                const char* sn[6] = {
                    settings.language == 1 ? scVi.passiveName : sc.passiveName,
                    settings.language == 1 ? scVi.qName       : sc.qName,
                    settings.language == 1 ? scVi.eName       : sc.eName,
                    settings.language == 1 ? scVi.cName       : sc.cName,
                    settings.language == 1 ? scVi.vName       : sc.vName,
                    settings.language == 1 ? scVi.rName       : sc.rName
                };
                const char* sd[6] = {
                    settings.language == 1 ? scVi.passiveDesc : sc.passiveDesc,
                    settings.language == 1 ? scVi.qDesc       : sc.qDesc,
                    settings.language == 1 ? scVi.eDesc       : sc.eDesc,
                    settings.language == 1 ? scVi.cDesc       : sc.cDesc,
                    settings.language == 1 ? scVi.vDesc       : sc.vDesc,
                    settings.language == 1 ? scVi.rDesc       : sc.rDesc
                };
                Color cols[6] = { sc.color, SKYBLUE, { 110, 235, 175, 255 }, { 245, 200, 95, 255 }, { 225, 115, 255, 255 }, ORANGE };
                int ky = (int)r.y + 132;
                for (int k = 0; k < 6; ++k) {
                    int by = ky + k * 60;
                    DrawRectangle((int)r.x + 14, by, (int)cw - 28, 54, { 12, 16, 28, 225 });
                    DrawRectangleLines((int)r.x + 14, by, (int)cw - 28, 54, Fade(cols[k], 0.45f));
                    DrawText(tags[k], (int)r.x + 22, by + 4, 11, cols[k]);
                    DrawText(sn[k], (int)r.x + 114, by + 3, 14, WHITE);
                    DrawWrappedText(sd[k], (int)r.x + 22, by + 20, (int)cw - 44, 12, 14, { 195, 210, 235, 255 });
                }

                Rectangle btn = { r.x + 24.0f, r.y + ch - 46.0f, cw - 48.0f, 36.0f };
                DrawRectangleRec(btn, hov ? Shade(sc.color, 0.55f) : Color{ 32, 76, 118, 255 });
                DrawRectangleLinesEx(btn, 2.0f, hov ? GOLD : sc.color);
                std::string bTxt = touchMode ? std::string(L("TAP TO AWAKEN ", "CHẠM ĐỂ THỨC TỈNH ")) + SubDisplayName(hc, i + 1)
                                             : std::string(TextFormat(L("PRESS [%d] OR CLICK TO AWAKEN", "NHẤN [%d] HOẶC NHẤP ĐỂ THỨC TỈNH"), i + 1));
                DrawText(bTxt.c_str(), (int)(btn.x + btn.width / 2) - MeasureText(bTxt.c_str(), 15) / 2, (int)btn.y + 10, 15, WHITE);
            }
        } else {
            const char* luHdr = L("LEVEL UP! CHOOSE AN UPGRADE", "LÊN CẤP! CHỌN NÂNG CẤP");
            DrawText(luHdr, 1280 / 2 - MeasureText(luHdr, 34) / 2, 150, 34, GOLD);

            float cardW = 240;
            float startX = 1280 / 2 - (cardW * 3 + 40) / 2;

            for (size_t i = 0; i < levelChoices.size(); ++i) {
                float cx = startX + i * (cardW + 20);
                float cy = 240;
                DrawRectangle((int)cx, (int)cy, (int)cardW, 280, { 25, 32, 45, 245 });
                DrawRectangleLines((int)cx, (int)cy, (int)cardW, 280, SKYBLUE);

                std::string title = UpgradeTitle(levelChoices[i]);
                std::string desc = UpgradeDesc(levelChoices[i]);

                DrawText(title.c_str(), (int)cx + 18, (int)cy + 28, 21, GOLD);
                DrawWrappedText(desc.c_str(), (int)cx + 18, (int)cy + 78, (int)cardW - 36, 16, 20, RAYWHITE);

                std::string btn = touchMode ? std::string(L("TAP TO CHOOSE", "CHẠM ĐỂ CHỌN")) : std::string(L("PRESS [", "NHẤN [")) + std::to_string(i + 1) + "]";
                DrawRectangle((int)cx + 30, (int)cy + 210, 180, 42, DARKBLUE);
                DrawText(btn.c_str(), (int)cx + 120 - MeasureText(btn.c_str(), 18) / 2, (int)cy + 222, 18, WHITE);
            }
        }
    }

    // ── Game Over Modal ──
    if (state == GAME_OVER) {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.88f));
        const char* goHdr = L("YOU HAVE FALLEN", "BẠN ĐÃ GỤC NGÃ");
        DrawText(goHdr, 1280 / 2 - MeasureText(goHdr, 44) / 2, 128, 44, RED);

        std::string st0 = std::string(L("Class: ", "Hệ Tướng: ")) + HeroTitle() + (generation > 1 ? (" (" + std::string(L("Gen ", "Đời ")) + std::to_string(generation) + ")") : "");
        std::string st1 = std::string(L("Floor Reached: ", "Tầng Đạt Được: ")) + std::to_string(floor);
        std::string st2 = std::string(L("Final Level: ", "Cấp Độ Cuối: ")) + std::to_string(player.level) + "/100  (" + CurrentLevelTitle() + ")";
        std::string st3 = std::string(L("Enemies Slain: ", "Kẻ Địch Đã Hạ: ")) + std::to_string(enemiesSlain);
        std::string st4 = std::string(L("Gold Accumulated: ", "Vàng Tích Lũy: ")) + std::to_string(player.gold);
        std::string st5 = std::string(L("Best Record: Floor ", "Kỷ Lục Cao Nhất: Tầng ")) + std::to_string(bestFloor) + (newRecord ? L("  (NEW RECORD!)", "  (KỶ LỤC MỚI!)") : "");

        DrawText(st0.c_str(), 1280 / 2 - 150, 186, 21, hCol);
        DrawText(st1.c_str(), 1280 / 2 - 150, 217, 21, RAYWHITE);
        DrawText(st2.c_str(), 1280 / 2 - 150, 248, 21, RAYWHITE);
        DrawText(st3.c_str(), 1280 / 2 - 150, 279, 21, RAYWHITE);
        DrawText(st4.c_str(), 1280 / 2 - 150, 310, 21, GOLD);
        DrawText(st5.c_str(), 1280 / 2 - 150, 341, 21, newRecord ? GREEN : SKYBLUE);
        if (bestCombo >= 2) {
            std::string st6 = std::string(L("Best Combo: x", "Combo Cao Nhất: x")) + std::to_string(bestCombo);
            DrawText(st6.c_str(), 1280 / 2 - 150, 372, 21, ORANGE);
        }

        std::string st7 = std::string(L("Souls earned: +", "Linh Hồn nhận: +")) + std::to_string(lastRunSouls) + L("   (total ", "   (tổng ") + std::to_string(souls) + ")";
        DrawText(st7.c_str(), 1280 / 2 - 150, 403, 21, { 190, 225, 255, 255 });

        // nút CONTINUE AS HEIR / PLAY AGAIN / CHANGE HERO / SANCTUARY được vẽ bởi DrawButtons()
    }

    if (state == SURFACE_UI) DrawSurfaceUi();
    if (state == FAMILY_TREE) DrawFamilyTreeScreen();
    if (state == INVENTORY) DrawInventory();
    if (state == SETTINGS) DrawSettingsScreen();
    if (state == SHOP) DrawShopScreen();
    if (state == META) DrawMetaScreen();
    if (state == CLASS_SELECT) DrawClassSelectScreen();
    if (state == SAVE_LOAD) DrawSaveLoadScreen();
    if (state == TITLE) DrawTitleScreen();
    if (state == PAUSED) DrawPauseScreen();

    DrawButtons();
    if (namingMode != 0) DrawNameModal();

    if (settings.showFps) {
        std::string f = "FPS: " + std::to_string(GetFPS());
        DrawText(f.c_str(), 1280 - 12 - MeasureText(f.c_str(), 18), 720 - 26, 18, { 120, 255, 140, 255 });
    }

    EndTextureMode();
    Present();
}
