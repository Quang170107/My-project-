#pragma once
#include "../game.hpp"

inline void Game::FlushSpawns() {
    for (auto& m : pendingSpawns) enemies.push_back(m);
    pendingSpawns.clear();
}

inline void Game::SpawnMinionChaser(Vector2 pos) {
    float multHp = HpMult(), multDmg = DmgMult();
    Enemy m = BuildEnemy(CHASER, pos);
    m.isElite = false;
    m.isMinion = true;
    m.maxHp = 16.0f * multHp;
    m.hp = m.maxHp;
    m.speed = 195.0f;
    m.radius = 11.0f;
    m.contactDmg = 7.0f * multDmg;
    pendingSpawns.push_back(m);
}

inline void Game::SpawnSplitMini(Vector2 pos) {
    float multHp = HpMult(), multDmg = DmgMult();
    Enemy m = BuildEnemy(SPLITTER, pos);
    m.isElite = false;
    m.isMinion = true;
    m.splitLevel = 1;
    m.maxHp = 14.0f * multHp;
    m.hp = m.maxHp;
    m.speed = 215.0f;
    m.radius = 12.0f;
    m.contactDmg = 6.0f * multDmg;
    pendingSpawns.push_back(m);
}

inline bool Game::GiveRandomRelic() {
    std::vector<RelicType> pool;
    for (int i = 0; i < 10; ++i) if (!HasRelic((RelicType)i)) pool.push_back((RelicType)i);
    if (pool.empty()) return false;
    AddRelic(pool[rand() % pool.size()]);
    return true;
}

inline void Game::ThornBurst() {
    NovaVisual nv;
    nv.pos = player.pos;
    nv.radius = 0;
    nv.maxRadius = 120.0f;
    nv.timer = 0.3f;
    nv.maxTimer = 0.3f;
    nv.color = { 170, 120, 255, 255 };
    novas.push_back(nv);
    Burst(player.pos, { 190, 150, 255, 255 }, 18, 260.0f, 0.5f);
    for (auto& en : enemies) {
        if (en.hp <= 0) continue;
        if (Vector2Distance(player.pos, en.pos) <= 120.0f + en.radius * 0.5f) {
            Vector2 kb = Vector2Normalize(Vector2Subtract(en.pos, player.pos));
            DamageEnemy(en, player.baseDmg, kb, 200.0f, false);
        }
    }
}

inline void Game::FireSwordWave(Vector2 origin, Vector2 dir, float dmg, float spd, int pierce, Color col, float rad, bool applyBurn) {
    Projectile sw;
    sw.fromPlayer = true;
    sw.kind = 6;
    sw.pos = Vector2Add(origin, Vector2Scale(dir, 22.0f));
    sw.vel = Vector2Scale(dir, spd);
    sw.damage = dmg;
    sw.life = 1.25f;
    sw.radius = rad;
    sw.pierce = pierce;
    sw.applyBurn = applyBurn;
    sw.color = col;
    projectiles.push_back(sw);
}

inline void Game::MeleeAttack(Vector2 aimDir) {
    int wt = ClassWeaponType(player.heroClass);
    int visWt = VisualWeaponType(player.heroClass, player.subClass, wt);
    static const float wReach[8]  = { 45.0f, 38.0f, 40.0f, 0.0f, 46.0f, 26.0f, 20.0f, 20.0f };
    static const float wRadius[8] = { 75.0f, 64.0f, 98.0f, 0.0f, 85.0f, 92.0f, 0.0f,  0.0f  };
    static const float wDmg[8]    = { 1.00f, 0.85f, 1.35f, 1.10f, 1.55f, 1.20f, 0.95f, 1.15f };
    static const float wRate[8]   = { 1.00f, 1.30f, 0.75f, 0.95f, 0.65f, 0.85f, 1.15f, 0.90f };
    static const float wKb[8]     = { 1.00f, 0.80f, 1.40f, 1.00f, 1.80f, 0.85f, 1.00f, 1.00f };

    float effRate = EffectiveAttackRate();
    player.attackTimer = 1.0f / std::max(0.2f, effRate * wRate[wt]);
    player.attackCounter++;
    float critMul = CritMult();

    // Nội tại Hiệp Sĩ / Paladin: hồi máu & trừng phạt sấm thánh
    if (player.heroClass == CLASS_KNIGHT) {
        int cycle = (player.subClass == 1) ? 4 : 5;
        float healAmt = (player.subClass == 1) ? 6.0f : 4.0f;
        if ((player.subClass != 2) && (player.attackCounter % cycle == 0)) {
            if (player.hp < player.maxHp) {
                player.hp = std::min(player.maxHp, player.hp + healAmt);
                SpawnPopup({ player.pos.x, player.pos.y - 24.0f }, "+" + std::to_string((int)healAmt), GREEN);
                Burst(player.pos, GREEN, 6, 110.0f, 0.35f);
            }
            if (player.subClass == 1) {
                Vector2 smitePos = Vector2Add(player.pos, Vector2Scale(aimDir, 42.0f));
                NovaVisual sn; sn.pos = smitePos; sn.maxRadius = 115.0f; sn.timer = 0.25f; sn.maxTimer = 0.25f; sn.color = GOLD;
                novas.push_back(sn);
                Burst(smitePos, GOLD, 14, 220.0f, 0.4f);
                for (auto& en : enemies) {
                    if (en.hp > 0 && Vector2Distance(smitePos, en.pos) <= 115.0f + en.radius * 0.4f) {
                        Vector2 kb = Vector2Normalize(Vector2Subtract(en.pos, player.pos));
                        DamageEnemy(en, player.baseDmg * 0.65f, kb, 140.0f, false, false);
                        if (en.hp > 0) { en.burnTimer = std::max(en.burnTimer, 2.5f); en.burnDps = std::max(en.burnDps, player.baseDmg * 0.4f); }
                    }
                }
            }
        }
    }

    // Nội tại Long Kỵ Thương Vương: phóng Long Thương Khí & hồi HP
    if (player.heroClass == CLASS_TEMPLAR) {
        if (player.subClass == 2) {
            FireSwordWave(player.pos, aimDir, player.baseDmg * 0.95f, 580.0f, 3, { 255, 185, 65, 255 }, 14.0f, true);
            AddRage(3.0f);
        } else {
            int cycle = (player.subClass == 1) ? 2 : 3;
            if (player.attackCounter % cycle == 0) {
                Color wCol = (player.subClass == 1) ? Color{ 255, 225, 105, 255 } : GOLD;
                FireSwordWave(player.pos, aimDir, player.baseDmg * 1.45f, 610.0f, 3, wCol, 14.5f, player.subClass == 1);
                float hl = (player.subClass == 1) ? 6.0f : 5.0f;
                if (player.hp < player.maxHp) {
                    player.hp = std::min(player.maxHp, player.hp + hl);
                    SpawnPopup({ player.pos.x, player.pos.y - 24.0f }, "+" + std::to_string((int)hl), GREEN);
                }
            }
        }
    }

    // Nội tại Xạ Thủ / Sniper Deadeye: Tên Gió xuyên thấu
    bool eagleMark = (player.heroClass == CLASS_RANGER && (player.attackCounter % (player.subClass == 1 ? 2 : 3) == 0));

    SlashVisual sv;
    sv.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 35.0f));
    sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG;
    sv.kind = visWt;
    sv.color = (player.heroClass == CLASS_MAGE && player.subClass == 1) ? Color{ 85, 235, 255, 255 }
             : (player.heroClass == CLASS_KNIGHT && player.subClass == 2) ? Color{ 255, 85, 75, 255 }
             : (player.heroClass == CLASS_RANGER && player.subClass == 2 ? Color{ 100, 240, 255, 255 }
             : (eagleMark ? Color{ 85, 255, 165, 255 } : WHITE));
    slashes.push_back(sv);

    // Spellblade (Kiếm Sư): trang bị Gậy (WP_STAFF) nhưng thể hiện thành Kiếm Phép (model WP_SWORD), vừa chém gần vừa phóng kiếm khí nổ AoE
    if (player.heroClass == CLASS_MAGE && player.subClass == 1) {
        PlaySound(snd.sndSlash);
        PlaySound(snd.sndShoot);
        if (player.attackCounter % 3 == 0) {
            float ba = atan2f(aimDir.y, aimDir.x);
            for (int w = -1; w <= 1; w += 2) {
                float a = ba + w * 0.16f;
                FireSwordWave(player.pos, { cosf(a), sinf(a) }, player.baseDmg * wDmg[wt] * 1.05f, 600.0f, 3, { 125, 235, 255, 255 }, 14.0f, false);
                projectiles.back().splashRadius = 48.0f;
                projectiles.back().splashRatio = 0.45f;
            }
        } else {
            FireSwordWave(player.pos, aimDir, player.baseDmg * wDmg[wt] * 1.10f, 580.0f, 3, { 95, 235, 255, 255 }, 13.5f, false);
            projectiles.back().splashRadius = 52.0f;
            projectiles.back().splashRatio = 0.50f;
        }
        Vector2 hitC = Vector2Add(player.pos, Vector2Scale(aimDir, 45.0f));
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(hitC, e.pos) <= 78.0f + e.radius) {
                bool isCrit = (Rand01() < std::min(0.85f, player.critChance));
                float dmg = player.baseDmg * wDmg[wt] * (isCrit ? critMul : 1.0f);
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                DamageEnemy(e, dmg, kb, isCrit ? 220.0f : 150.0f, isCrit, true);
            }
        }
        TriggerBarrelsInRadius(hitC, 85.0f, player.baseDmg * wDmg[wt]);
        return;
    }

    // Vũ khí tầm xa: Cung (6) và Trượng (7)
    if (wt == WP_BOW || wt == WP_STAFF) {
        PlaySound(snd.sndShoot);
        int count = (wt == WP_STAFF && player.heroClass == CLASS_MAGE && player.subClass == 2) ? 2 : 1;
        Vector2 perp = { -aimDir.y, aimDir.x };
        bool isVal = (wt == WP_BOW && player.heroClass == CLASS_RANGER && player.subClass == 2);
        for (int c = 0; c < count; ++c) {
            float off = (count == 2) ? (c == 0 ? -9.0f : 9.0f) : 0.0f;
            Projectile p;
            p.fromPlayer = true;
            p.pos = Vector2Add(Vector2Add(player.pos, Vector2Scale(aimDir, 22.0f)), Vector2Scale(perp, off));
            float spd = (wt == WP_BOW) ? (eagleMark ? 780.0f : (player.subClass >= 1 ? 700.0f : 620.0f)) : (player.subClass == 2 ? 540.0f : 480.0f);
            p.vel = Vector2Scale(aimDir, spd);
            float emMul = eagleMark ? (player.subClass == 1 ? 1.75f : 1.50f) : 1.0f;
            p.damage = player.baseDmg * wDmg[wt] * emMul * (count == 2 ? 0.72f : 1.0f);
            p.life = 1.8f;
            p.kind = (wt == WP_BOW) ? 1 : 2;
            p.critBonus = eagleMark ? (player.subClass == 1 ? 0.15f : 0.10f) : 0.0f;
            if (wt == WP_BOW) {
                bool isCrit = (Rand01() < std::min(0.85f, player.critChance + p.critBonus));
                p.hasPreCrit = true;
                p.preCrit = isCrit;
                p.pierce = eagleMark ? 99 : ((isCrit ? 1 : 0) + (player.heroClass == CLASS_RANGER && player.subClass >= 1 ? 1 : 0));
                if (isVal) { p.splashRadius = 48.0f; p.splashRatio = 0.45f; }
                p.radius = eagleMark ? 9.0f : 6.5f;
                p.color = eagleMark ? Color{ 85, 255, 165, 255 } : (isVal ? Color{ 100, 240, 255, 255 } : (isCrit ? GOLD : Color{ 225, 245, 145, 255 }));
            } else {
                p.pierce = eagleMark ? 99 : 0;
                p.splashRadius = (count == 2) ? 62.0f : 52.0f;
                p.splashRatio = 0.50f;
                p.applyBurn = (count == 2);
                p.radius = 8.5f;
                p.color = eagleMark ? Color{ 85, 255, 165, 255 } : (count == 2 ? Color{ 195, 120, 255, 255 } : Color{ 110, 215, 255, 255 });
            }
            projectiles.push_back(p);
            Burst(p.pos, p.color, eagleMark ? 8 : 3, 120.0f, 0.25f);
        }
        return;
    }

    PlaySound(snd.sndSlash);
    // Hiệu ứng phóng kiếm khí / phi tiêu theo nhánh phụ cận chiến
    if (player.heroClass == CLASS_KNIGHT && player.subClass == 2) { // Dread Berserker: Blood Wave
        FireSwordWave(player.pos, aimDir, player.baseDmg * 0.80f, 520.0f, 2, { 255, 75, 85, 255 }, 12.0f, true);
        AddRage(2.5f);
    } else if (player.heroClass == CLASS_ASSASSIN && player.subClass == 2) { // Ninja Trickster: Lightning Shuriken
        Projectile sh;
        sh.fromPlayer = true;
        sh.kind = 3;
        sh.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 18.0f));
        sh.vel = Vector2Scale(aimDir, 600.0f);
        sh.damage = player.baseDmg * 0.70f;
        sh.life = 1.2f;
        sh.radius = 7.5f;
        sh.pierce = 1;
        sh.color = { 110, 235, 255, 255 };
        projectiles.push_back(sh);
    }

    if (eagleMark) {
        // Xạ thủ dùng vũ khí cận chiến vẫn phóng được luồng gió đòn thứ 2/3
        Projectile wp;
        wp.fromPlayer = true;
        wp.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 20.0f));
        wp.vel = Vector2Scale(aimDir, 680.0f);
        wp.damage = player.baseDmg * (player.subClass == 1 ? 1.75f : 1.50f);
        wp.life = 1.5f;
        wp.kind = 1;
        wp.pierce = 99;
        wp.critBonus = (player.subClass == 1 ? 0.15f : 0.10f);
        wp.radius = 9.0f;
        wp.color = { 85, 255, 165, 255 };
        projectiles.push_back(wp);
    }

    Vector2 hitCenter = Vector2Add(player.pos, Vector2Scale(aimDir, wReach[wt]));
    float hitRadius = wRadius[wt];
    Vector2 spearEnd = Vector2Add(player.pos, Vector2Scale(aimDir, 135.0f));
    bool hitAny = false;
    float maxHitDmg = 0.0f;

    if (wt == WP_HAMMER) {
        NovaVisual nv;
        nv.pos = hitCenter;
        nv.radius = 0.0f;
        nv.maxRadius = 85.0f;
        nv.timer = 0.22f;
        nv.maxTimer = 0.22f;
        nv.color = { 255, 195, 85, 255 };
        novas.push_back(nv);
    }

    for (auto& e : enemies) {
        if (e.hp <= 0) continue;
        bool inRange;
        if (wt == WP_SPEAR) {
            Vector2 ap = Vector2Subtract(e.pos, player.pos), ab = Vector2Subtract(spearEnd, player.pos);
            float tt = Clamp(Vector2DotProduct(ap, ab) / Vector2DotProduct(ab, ab), 0.0f, 1.0f);
            Vector2 closest = Vector2Add(player.pos, Vector2Scale(ab, tt));
            inRange = Vector2Distance(closest, e.pos) <= 26.0f + e.radius;
        } else {
            inRange = Vector2Distance(hitCenter, e.pos) <= hitRadius + e.radius;
        }
        if (inRange) {
            hitAny = true;
            float cc = std::min(0.85f, player.critChance + (eagleMark ? (player.subClass == 1 ? 0.15f : 0.10f) : 0.0f)
                                       + ((player.heroClass == CLASS_TEMPLAR && player.subClass == 2) ? 0.15f : 0.0f));
            bool isCrit = (Rand01() < cc);
            float dmg = player.baseDmg * wDmg[wt] * (eagleMark ? (player.subClass == 1 ? 1.75f : 1.50f) : 1.0f) * (isCrit ? critMul : 1.0f);
            maxHitDmg = std::max(maxHitDmg, dmg);
            Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
            float kbForce = (isCrit ? 240.0f : 160.0f) * wKb[wt];
            if (wt == WP_SCYTHE && Vector2Distance(player.pos, e.pos) > 48.0f) {
                kb = Vector2Negate(kb); // Lưỡi hái kéo nhẹ quái vào tầm
                kbForce = 180.0f;
            }
            DamageEnemy(e, dmg, kb, kbForce, isCrit, true);
            if (wt == WP_HAMMER && e.hp > 0) {
                e.stunTimer = std::max(e.stunTimer, 0.55f);
            }
        }
    }
    TriggerBarrelsInRadius(wt == WP_SPEAR ? Vector2Lerp(player.pos, spearEnd, 0.55f) : hitCenter, wt == WP_SPEAR ? 82.0f : hitRadius + 10.0f, player.baseDmg * wDmg[wt]);
    if (hitAny) {
        float ls = std::min(LifestealCap() + 0.05f, player.lifesteal + (wt == WP_SCYTHE ? 0.05f * LifestealScale() : 0.0f));
        if (ls > 0.0f && player.hp < player.maxHp) {
            float rawHl = maxHitDmg * ls;
            float heal = (settings.difficulty == 0) ? std::max(1.0f, rawHl) : rawHl;
            player.hp = std::min(player.maxHp, player.hp + heal);
            if (heal >= 1.0f) SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)heal), GREEN);
        }
        PlaySound(snd.sndHit);
        Shake(wt == WP_HAMMER ? 0.17f : 0.12f, wt == WP_HAMMER ? 5.0f : 3.5f);
    }
}

inline void Game::TriggerArcaneResonance(Vector2 aimDir) {
    if (player.heroClass != CLASS_MAGE) return;
    int n = (player.subClass == 2) ? 2 : 1;
    AddRage(player.subClass == 2 ? 10.0f : 6.0f);
    if (player.subClass == 1) player.shieldActive = true;
    float baseAng = atan2f(aimDir.y, aimDir.x);
    for (int i = -n; i <= n; ++i) {
        float a = baseAng + i * 0.42f;
        Projectile p;
        p.fromPlayer = true;
        p.kind = 5;
        p.pos = Vector2Add(player.pos, { cosf(a) * 18.0f, sinf(a) * 18.0f });
        p.vel = { cosf(a) * 440.0f, sinf(a) * 440.0f };
        p.damage = player.baseDmg * (player.subClass == 2 ? 0.65f : 0.55f);
        p.life = 2.2f;
        p.radius = 6.5f;
        p.color = (player.subClass == 2) ? Color{ 210, 125, 255, 255 } : Color{ 120, 230, 255, 255 };
        projectiles.push_back(p);
    }
}

inline void Game::CastSkill1(Vector2 aimDir) {
    player.specialCooldown = Skill1MaxCd();
    player.specialTimer = player.specialCooldown;
    float critMul = CritMult();
    const float bw = roomW / 2.0f - 30.0f, bh = roomH / 2.0f - 30.0f;

    if (player.heroClass == CLASS_KNIGHT) {
        // Knight / Paladin / Berserker: Xung Kích Khiên / Đại Nhảy Huyết Chiến
        float dist = (player.subClass == 2) ? 110.0f : 75.0f;
        float rad = (player.subClass == 2) ? 155.0f : 140.0f;
        float mul = (player.subClass == 2) ? 2.5f : (player.subClass == 1 ? 2.4f : 1.9f);
        Color cCol = (player.subClass == 2) ? Color{ 235, 65, 65, 255 } : GOLD;
        Vector2 dest = Vector2Add(player.pos, Vector2Scale(aimDir, dist));
        dest.x = Clamp(dest.x, -bw, bw);
        dest.y = Clamp(dest.y, -bh, bh);
        ResolvePillars(dest, 15.0f);
        ghosts.push_back({ player.pos, player.facingLeft, 0.25f, 0.25f });
        player.pos = dest;
        player.invulnTimer = std::max(player.invulnTimer, 0.28f);

        PlaySound(snd.sndSpecial);
        Shake(0.24f, 6.5f);
        Vector2 slamCenter = Vector2Add(player.pos, Vector2Scale(aimDir, 38.0f));
        Burst(slamCenter, cCol, 26, 320.0f, 0.55f);
        SlashVisual sv;
        sv.pos = slamCenter;
        sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG;
        sv.kind = 10;
        sv.timer = 0.22f;
        sv.maxTimer = 0.22f;
        sv.color = cCol;
        slashes.push_back(sv);

        NovaVisual nv; nv.pos = slamCenter; nv.maxRadius = rad; nv.timer = 0.28f; nv.maxTimer = 0.28f; nv.color = cCol;
        novas.push_back(nv);

        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(slamCenter, e.pos) <= rad + e.radius * 0.5f) {
                bool isCrit = (Rand01() < player.critChance);
                float dmg = player.baseDmg * mul * (isCrit ? critMul : 1.0f);
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                DamageEnemy(e, dmg, kb, 300.0f, isCrit, true);
                if (e.hp > 0) {
                    if (player.subClass != 2) e.stunTimer = std::max(e.stunTimer, player.subClass == 1 ? 1.5f : 1.2f);
                    if (player.subClass == 2) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, player.baseDmg * 0.65f); }
                }
            }
        }
        TriggerBarrelsInRadius(slamCenter, rad, player.baseDmg * mul);
        if (player.subClass == 1) {
            GroundEffect hz; hz.kind = 1; hz.pos = slamCenter; hz.radius = 140.0f; hz.maxTimer = 2.5f; hz.timer = 2.5f; hz.tickTimer = 0.4f;
            hz.damage = player.baseDmg * 0.6f; hz.color = GOLD;
            groundEffects.push_back(hz);
        } else if (player.subClass == 2 && player.hp < player.maxHp) {
            float heal = std::max(4.0f, player.maxHp * 0.10f);
            player.hp = std::min(player.maxHp, player.hp + heal);
            SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)heal), GREEN);
        }
    } else if (player.heroClass == CLASS_ASSASSIN) {
        // Assassin / Phantom Reaper (6 Death Scythes) / Ninja Trickster (8 Lightning Shurikens)
        PlaySound(snd.sndSlash);
        Shake(0.14f, 3.5f);
        float baseAng = atan2f(aimDir.y, aimDir.x);
        int half = (player.subClass == 2) ? 4 : 2;
        int total = (player.subClass == 1) ? 6 : (player.subClass == 2 ? 8 : 5);
        Color sc = (player.subClass == 1) ? Color{ 195, 90, 255, 255 } : (player.subClass == 2 ? Color{ 110, 235, 255, 255 } : Color{ 235, 70, 140, 255 });
        for (int idx = 0; idx < total; ++idx) {
            float offset = (idx - (total - 1) * 0.5f) * (player.subClass == 2 ? 0.14f : 0.19f);
            float a = baseAng + offset;
            Vector2 d = { cosf(a), sinf(a) };
            Projectile p;
            p.fromPlayer = true;
            p.kind = 3;
            p.pos = Vector2Add(player.pos, Vector2Scale(d, 18.0f));
            p.vel = Vector2Scale(d, player.subClass == 2 ? 640.0f : 560.0f);
            p.damage = player.baseDmg * (player.subClass == 1 ? 1.35f : (player.subClass == 2 ? 1.25f : 1.1f));
            p.life = 1.4f;
            p.radius = (player.subClass == 1) ? 10.5f : 8.0f;
            p.pierce = (player.subClass == 1) ? 3 : (player.subClass == 2 ? 2 : 1);
            p.applySlow = true;
            p.applyPoison = (player.subClass != 2);
            p.color = sc;
            projectiles.push_back(p);
        }
        (void)half;
        Burst(player.pos, sc, 14, 220.0f, 0.35f);
    } else if (player.heroClass == CLASS_MAGE) {
        if (player.subClass == 1) {
            // Spellblade (Kiếm Sư) [Q]: Dimensional Slash - lướt chém 115px + phóng 3 kiếm khí ánh trăng + Shield
            Vector2 origin = player.pos;
            Vector2 dest = Vector2Add(origin, Vector2Scale(aimDir, 115.0f));
            dest.x = Clamp(dest.x, -bw, bw);
            dest.y = Clamp(dest.y, -bh, bh);
            ResolvePillars(dest, 15.0f);
            ghosts.push_back({ origin, player.facingLeft, 0.25f, 0.25f });
            ghosts.push_back({ Vector2Lerp(origin, dest, 0.5f), player.facingLeft, 0.25f, 0.25f });
            player.pos = dest;
            player.invulnTimer = std::max(player.invulnTimer, 0.35f);
            player.shieldActive = true;

            PlaySound(snd.sndSpecial);
            Shake(0.24f, 6.0f);
            Burst(dest, { 85, 230, 255, 255 }, 24, 320.0f, 0.5f);
            SlashVisual sv; sv.pos = dest; sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG; sv.kind = 11; sv.timer = 0.22f; sv.maxTimer = 0.22f; sv.color = { 85, 230, 255, 255 };
            slashes.push_back(sv);
            NovaVisual nv; nv.pos = dest; nv.maxRadius = 155.0f; nv.timer = 0.26f; nv.maxTimer = 0.26f; nv.color = { 85, 230, 255, 255 };
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(dest, e.pos) <= 155.0f + e.radius * 0.4f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 2.4f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, origin));
                    DamageEnemy(e, dmg, kb, 220.0f, isCrit, true);
                }
            }
            TriggerBarrelsInRadius(dest, 155.0f, player.baseDmg * 2.4f);
            float baseAng = atan2f(aimDir.y, aimDir.x);
            for (int w = -1; w <= 1; ++w) {
                float a = baseAng + w * 0.24f;
                FireSwordWave(dest, { cosf(a), sinf(a) }, player.baseDmg * 1.45f, 620.0f, 3, { 95, 235, 255, 255 }, 14.5f, false);
            }
            TriggerArcaneResonance(aimDir);
        } else if (player.subClass == 2) {
            // Archmage [Q]: Twin Meteor Comets (2 cầu lửa lớn)
            PlaySound(snd.sndShoot);
            Shake(0.18f, 5.0f);
            float baseAng = atan2f(aimDir.y, aimDir.x);
            for (int c = -1; c <= 1; c += 2) {
                float a = baseAng + c * 0.11f;
                Vector2 d = { cosf(a), sinf(a) };
                Projectile p; p.fromPlayer = true; p.kind = 4;
                p.pos = Vector2Add(player.pos, Vector2Scale(d, 22.0f));
                p.vel = Vector2Scale(d, 500.0f);
                p.damage = player.baseDmg * 2.3f;
                p.life = 1.6f; p.radius = 13.0f; p.splashRadius = 150.0f; p.splashRatio = 1.0f; p.applyBurn = true;
                p.color = { 255, 130, 45, 255 };
                projectiles.push_back(p);
            }
            TriggerArcaneResonance(aimDir);
        } else {
            // Base Arcane Mage: Cầu Lửa Hủy Diệt
            PlaySound(snd.sndShoot);
            Shake(0.14f, 4.0f);
            Projectile p;
            p.fromPlayer = true; p.kind = 4;
            p.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 22.0f));
            p.vel = Vector2Scale(aimDir, 470.0f);
            p.damage = player.baseDmg * 2.3f;
            p.life = 1.6f; p.radius = 12.0f; p.splashRadius = 135.0f; p.splashRatio = 1.0f; p.applyBurn = true;
            p.color = { 255, 130, 45, 255 };
            projectiles.push_back(p);
            Burst(p.pos, ORANGE, 10, 160.0f, 0.35f);
            TriggerArcaneResonance(aimDir);
        }
    } else if (player.heroClass == CLASS_RANGER) {
        if (player.subClass == 2) {
            // Tempest Valkyrie [Q]: Thunder Volley (7 mũi tên sét nổ AoE + choáng 0.8s quanh thân)
            PlaySound(snd.sndShoot);
            PlaySound(snd.sndSpecial);
            Shake(0.18f, 4.5f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 135.0f; nv.timer = 0.25f; nv.maxTimer = 0.25f; nv.color = { 100, 240, 255, 255 };
            novas.push_back(nv);
            for (auto& e : enemies) {
                if (e.hp > 0 && Vector2Distance(player.pos, e.pos) <= 135.0f + e.radius) {
                    e.stunTimer = std::max(e.stunTimer, 0.8f);
                }
            }
            float baseAng = atan2f(aimDir.y, aimDir.x);
            for (int i = -3; i <= 3; ++i) {
                float a = baseAng + i * 0.14f;
                Vector2 d = { cosf(a), sinf(a) };
                Projectile p; p.fromPlayer = true; p.kind = 1;
                p.pos = Vector2Add(player.pos, Vector2Scale(d, 20.0f));
                p.vel = Vector2Scale(d, 720.0f);
                p.damage = player.baseDmg * 1.35f;
                p.life = 1.6f; p.radius = 8.0f; p.pierce = 3;
                p.splashRadius = 55.0f; p.splashRatio = 0.6f;
                p.color = { 100, 240, 255, 255 };
                projectiles.push_back(p);
            }
            Burst(Vector2Add(player.pos, Vector2Scale(aimDir, 20.0f)), { 100, 240, 255, 255 }, 16, 240.0f, 0.4f);
        } else {
            // Ranger / Sniper Deadeye [Q]: Liên Xạ Tiễn (7 mũi tên / 9 mũi tên xuyên đốt)
            PlaySound(snd.sndShoot);
            Shake(0.14f, 3.5f);
            float baseAng = atan2f(aimDir.y, aimDir.x);
            int half = (player.subClass == 1) ? 4 : 3;
            for (int i = -half; i <= half; ++i) {
                float a = baseAng + i * (player.subClass == 1 ? 0.11f : 0.14f);
                Vector2 d = { cosf(a), sinf(a) };
                Projectile p; p.fromPlayer = true; p.kind = 1;
                p.pos = Vector2Add(player.pos, Vector2Scale(d, 20.0f));
                p.vel = Vector2Scale(d, player.subClass == 1 ? 700.0f : 630.0f);
                p.damage = player.baseDmg * (player.subClass == 1 ? 1.2f : 1.0f);
                p.life = 1.6f; p.radius = 7.0f; p.pierce = (player.subClass == 1) ? 2 : 0;
                p.applyBurn = (player.subClass == 1);
                p.color = GOLD;
                projectiles.push_back(p);
            }
            Burst(Vector2Add(player.pos, Vector2Scale(aimDir, 20.0f)), GOLD, 12, 200.0f, 0.35f);
        }
    } else if (player.heroClass == CLASS_TEMPLAR) {
        // Royal Lancemaster / Grand Inquisitor / Dragonlord Monarch [Q]
        float dist = (player.subClass == 2) ? 135.0f : (player.subClass == 1 ? 130.0f : 115.0f);
        float mul = (player.subClass == 2) ? 2.65f : (player.subClass == 1 ? 2.55f : 2.25f);
        Color lCol = (player.subClass == 2) ? Color{ 245, 75, 95, 255 } : GOLD;
        Vector2 origin = player.pos;
        Vector2 dest = Vector2Add(origin, Vector2Scale(aimDir, dist));
        dest.x = Clamp(dest.x, -bw, bw);
        dest.y = Clamp(dest.y, -bh, bh);
        ResolvePillars(dest, 15.0f);
        ghosts.push_back({ origin, player.facingLeft, 0.25f, 0.25f });
        player.pos = dest;
        player.invulnTimer = std::max(player.invulnTimer, 0.32f);

        PlaySound(snd.sndSpecial);
        Shake(0.24f, 6.5f);
        Vector2 hitC = Vector2Add(dest, Vector2Scale(aimDir, 42.0f));
        Burst(hitC, lCol, 28, 320.0f, 0.55f);
        NovaVisual nv; nv.pos = hitC; nv.maxRadius = 150.0f; nv.timer = 0.28f; nv.maxTimer = 0.28f; nv.color = lCol;
        novas.push_back(nv);
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(hitC, e.pos) <= 150.0f + e.radius * 0.5f) {
                bool isCrit = (Rand01() < player.critChance);
                float dmg = player.baseDmg * mul * (isCrit ? critMul : 1.0f);
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, origin));
                if (e.type == BOSS || e.isElite || e.isWarden) e.stagger = std::min(e.maxStagger, e.stagger + 35.0f);
                DamageEnemy(e, dmg, kb, player.subClass == 2 ? -120.0f : 240.0f, isCrit, true);
                if (e.hp > 0) {
                    if (player.subClass == 1) e.stunTimer = std::max(e.stunTimer, 1.4f);
                    if (player.subClass >= 1) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, player.baseDmg * 0.6f); }
                }
            }
        }
        TriggerBarrelsInRadius(hitC, 150.0f, player.baseDmg * mul);
        float baseAng = atan2f(aimDir.y, aimDir.x);
        for (int i = -1; i <= 1; ++i) {
            float a = baseAng + i * 0.22f;
            FireSwordWave(dest, { cosf(a), sinf(a) }, player.baseDmg * 1.45f, 620.0f, 3, lCol, 14.5f, player.subClass >= 1);
        }
        if (player.subClass == 2 && player.hp < player.maxHp) {
            float hl = std::max(5.0f, roundf(player.maxHp * 0.10f));
            player.hp = std::min(player.maxHp, player.hp + hl);
            SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)hl), GREEN);
        }
    }
}

inline void Game::CastSkill2(Vector2 aimDir, Vector2 moveDir) {
    player.skill2Cooldown = Skill2MaxCd();
    player.skill2Timer = player.skill2Cooldown;
    float critMul = CritMult();
    const float bw = roomW / 2.0f - 30.0f, bh = roomH / 2.0f - 30.0f;

    if (player.heroClass == CLASS_KNIGHT) {
        PlaySound(snd.sndSpecial);
        Shake(0.25f, 6.0f);
        bool isZerk = (player.subClass == 2);
        Color nCol = isZerk ? Color{ 235, 65, 65, 255 } : GOLD;
        float rad = isZerk ? 195.0f : (player.subClass == 1 ? 210.0f : 190.0f);
        float mul = isZerk ? 2.0f : (player.subClass == 1 ? 2.2f : 1.8f);
        Burst(player.pos, nCol, 34, 380.0f, 0.6f);
        if (isZerk) {
            player.hasteTimer = 4.0f;
            AddRage(25.0f);
            SpawnPopup({ player.pos.x, player.pos.y - 34.0f }, "BLOOD FRENZY!", ORANGE);
        } else {
            player.holyBuffTimer = (player.subClass == 1) ? 4.5f : 3.5f;
            player.shieldActive = true;
            SpawnPopup({ player.pos.x, player.pos.y - 34.0f }, "HOLY BARRIER!", GOLD);
        }
        NovaVisual nv; nv.pos = player.pos; nv.maxRadius = rad; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = nCol;
        novas.push_back(nv);

        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(player.pos, e.pos) <= rad) {
                bool isCrit = (Rand01() < player.critChance);
                float dmg = player.baseDmg * mul * (isCrit ? critMul : 1.0f);
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                DamageEnemy(e, dmg, kb, 280.0f, isCrit, true);
                if (isZerk && e.hp > 0) { e.slowTimer = std::max(e.slowTimer, 3.0f); e.slowFactor = std::min(e.slowFactor, 0.55f); }
            }
        }
        TriggerBarrelsInRadius(player.pos, rad, player.baseDmg * mul);
    } else if (player.heroClass == CLASS_ASSASSIN) {
        Vector2 origin = player.pos;
        if (player.subClass == 2) {
            // Ninja Trickster: thả phân thân nổ 2.2x AoE tại vị trí cũ
            GroundEffect decoy; decoy.kind = 0; decoy.pos = origin; decoy.radius = 150.0f; decoy.maxTimer = 0.45f; decoy.timer = 0.45f;
            decoy.damage = player.baseDmg * 2.2f; decoy.color = { 110, 235, 255, 255 };
            groundEffects.push_back(decoy);
        } else {
            NovaVisual smoke; smoke.pos = origin; smoke.maxRadius = 130.0f; smoke.timer = 0.35f; smoke.maxTimer = 0.35f; smoke.color = { 130, 100, 175, 255 };
            novas.push_back(smoke);
            Burst(origin, { 120, 90, 160, 255 }, 20, 200.0f, 0.55f);
            for (auto& e : enemies) {
                if (e.hp > 0 && Vector2Distance(origin, e.pos) <= 130.0f) {
                    e.slowTimer = std::max(e.slowTimer, 3.0f); e.slowFactor = std::min(e.slowFactor, 0.55f);
                }
            }
        }

        float bDist = (player.subClass > 0) ? 230.0f : 210.0f;
        Vector2 dest = Vector2Add(origin, Vector2Scale(aimDir, bDist));
        dest.x = Clamp(dest.x, -bw, bw);
        dest.y = Clamp(dest.y, -bh, bh);
        ResolvePillars(dest, 15.0f);
        for (int k = 1; k <= 3; ++k) ghosts.push_back({ Vector2Lerp(origin, dest, k * 0.25f), player.facingLeft, 0.28f, 0.28f });
        player.pos = dest;
        player.invulnTimer = std::max(player.invulnTimer, 0.50f);
        if (player.subClass == 1) player.shieldActive = true; // Reaper Soul Shield

        Color sCol = (player.subClass == 1) ? Color{ 195, 90, 255, 255 } : (player.subClass == 2 ? Color{ 110, 235, 255, 255 } : Color{ 255, 75, 145, 255 });
        float sRad = (player.subClass == 1) ? 155.0f : 125.0f;
        float sMul = (player.subClass == 1) ? 2.6f : (player.subClass == 2 ? 2.4f : 2.2f);
        PlaySound(snd.sndSpecial);
        Shake(0.22f, 6.0f);
        Burst(dest, sCol, 28, 320.0f, 0.5f);
        SlashVisual sv; sv.pos = dest; sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG; sv.kind = 11; sv.timer = 0.22f; sv.maxTimer = 0.22f; sv.color = sCol;
        slashes.push_back(sv);
        NovaVisual slashRing; slashRing.pos = dest; slashRing.maxRadius = sRad; slashRing.timer = 0.26f; slashRing.maxTimer = 0.26f; slashRing.color = sCol;
        novas.push_back(slashRing);

        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(dest, e.pos) <= sRad + e.radius * 0.4f) {
                float dmg = player.baseDmg * sMul * critMul; // Chắc chắn chí mạng!
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, dest));
                DamageEnemy(e, dmg, kb, 220.0f, true, true);
            }
        }
        TriggerBarrelsInRadius(dest, sRad, player.baseDmg * sMul);
    } else if (player.heroClass == CLASS_MAGE) {
        if (player.subClass == 1) {
            // Spellblade (Kiếm Sư) [E]: Phantom Sword Ring - nổ 190px + choáng 1.4s + 3.2s Kiếm Hồn xoay quanh + Shield
            PlaySound(snd.sndFreeze);
            Shake(0.22f, 5.5f);
            Burst(player.pos, { 85, 230, 255, 255 }, 32, 360.0f, 0.6f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 190.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = { 120, 235, 255, 255 };
            novas.push_back(nv);
            player.swordOrbitTimer = 3.2f;
            player.swordOrbitTick = 0.0f;
            player.shieldActive = true;
            SpawnPopup({ player.pos.x, player.pos.y - 34.0f }, "PHANTOM BLADES!", { 85, 230, 255, 255 });
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 190.0f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 1.9f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 200.0f, isCrit, true);
                    if (e.hp > 0) {
                        e.stunTimer = std::max(e.stunTimer, 1.4f);
                        e.chillTimer = std::max(e.chillTimer, 3.0f);
                        e.burnTimer = std::max(e.burnTimer, 2.5f);
                        e.burnDps = std::max(e.burnDps, player.baseDmg * 0.45f);
                    }
                }
            }
            TriggerBarrelsInRadius(player.pos, 190.0f, player.baseDmg * 1.9f);
            TriggerArcaneResonance(aimDir);
        } else {
            // Arcane Mage / Archmage: Frost Nova / Absolute Zero Nova
            bool isArch = (player.subClass == 2);
            float rad = isArch ? 215.0f : 185.0f;
            float mul = isArch ? 1.9f : 1.5f;
            PlaySound(snd.sndFreeze);
            Shake(0.22f, 5.5f);
            Burst(player.pos, { 130, 235, 255, 255 }, 32, 360.0f, 0.6f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = rad; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = { 115, 230, 255, 255 };
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= rad) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * mul * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 200.0f, isCrit, true);
                    if (e.hp > 0) {
                        e.stunTimer = std::max(e.stunTimer, isArch ? 1.8f : 1.3f);
                        e.chillTimer = std::max(e.chillTimer, 3.5f);
                    }
                }
            }
            if (isArch) { // 6 radial ice bolts
                for (int k = 0; k < 6; ++k) {
                    float a = k * (PI / 3.0f);
                    Projectile ib; ib.fromPlayer = true; ib.kind = 2;
                    ib.pos = Vector2Add(player.pos, { cosf(a) * 18.0f, sinf(a) * 18.0f });
                    ib.vel = { cosf(a) * 460.0f, sinf(a) * 460.0f };
                    ib.damage = player.baseDmg * 1.15f; ib.life = 1.4f; ib.radius = 8.5f; ib.splashRadius = 55.0f;
                    ib.applySlow = true; ib.color = { 130, 235, 255, 255 };
                    projectiles.push_back(ib);
                }
            }
            TriggerBarrelsInRadius(player.pos, rad, player.baseDmg * mul);
            TriggerArcaneResonance(aimDir);
        }
    } else if (player.heroClass == CLASS_RANGER) {
        // Ranger / Sniper Deadeye / Valkyrie Lancer [E]
        GroundEffect trap;
        trap.kind = 0;
        trap.pos = player.pos;
        trap.radius = (player.subClass > 0) ? 165.0f : 140.0f;
        trap.maxTimer = 0.75f;
        trap.timer = 0.75f;
        trap.damage = player.baseDmg * (player.subClass == 1 ? 2.8f : (player.subClass == 2 ? 2.5f : 2.4f));
        trap.color = (player.subClass == 2) ? Color{ 90, 225, 255, 255 } : Color{ 90, 240, 145, 255 };
        groundEffects.push_back(trap);

        Vector2 backDir = Vector2Length(moveDir) > 0.1f ? Vector2Negate(moveDir) : Vector2Negate(aimDir);
        if (Vector2Length(aimDir) > 0.1f) backDir = Vector2Negate(aimDir);
        float vDist = (player.subClass == 1) ? 140.0f : 120.0f;
        Vector2 dest = Vector2Add(player.pos, Vector2Scale(backDir, vDist));
        dest.x = Clamp(dest.x, -bw, bw);
        dest.y = Clamp(dest.y, -bh, bh);
        ResolvePillars(dest, 15.0f);
        ghosts.push_back({ player.pos, player.facingLeft, 0.25f, 0.25f });
        player.pos = dest;
        player.invulnTimer = std::max(player.invulnTimer, 0.40f);
        if (player.subClass == 1) player.hasteTimer = std::max(player.hasteTimer, 2.5f);
        else if (player.subClass == 2) { player.shieldActive = true; player.hasteTimer = std::max(player.hasteTimer, 3.5f); }
        PlaySound(snd.sndDash);
        Burst(trap.pos, trap.color, 14, 180.0f, 0.4f);
    } else {
        // Royal Lancemaster / Grand Inquisitor / Dragonlord Monarch [E]
        PlaySound(snd.sndSpecial);
        Shake(0.25f, 6.0f);
        Color nCol = (player.subClass == 2) ? Color{ 245, 75, 95, 255 } : GOLD;
        float rad = 195.0f;
        float mul = (player.subClass == 2) ? 2.3f : (player.subClass == 1 ? 2.2f : 1.95f);
        Burst(player.pos, nCol, 34, 360.0f, 0.6f);
        player.holyBuffTimer = (player.subClass == 1) ? 4.5f : 3.5f;
        player.hasteTimer = std::max(player.hasteTimer, 4.0f);
        player.shieldActive = true;
        if (player.subClass == 2) AddRage(25.0f);
        SpawnPopup({ player.pos.x, player.pos.y - 34.0f },
                   player.subClass == 2 ? L("EMPEROR'S ROAR!", "LONG KHIẾU CHẤN THIÊN!") : L("ROYAL BANNER!", "HOÀNG KỲ BẢO HỘ!"), nCol);
        NovaVisual nv; nv.pos = player.pos; nv.maxRadius = rad; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = nCol;
        novas.push_back(nv);
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(player.pos, e.pos) <= rad + e.radius * 0.4f) {
                bool isCrit = (Rand01() < player.critChance);
                float dmg = player.baseDmg * mul * (isCrit ? critMul : 1.0f);
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                DamageEnemy(e, dmg, kb, 260.0f, isCrit, true);
                if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, player.subClass == 2 ? 1.5f : 1.2f);
            }
        }
        TriggerBarrelsInRadius(player.pos, rad, player.baseDmg * mul);
    }
}

inline void Game::CastSkill3(Vector2 aimDir) {
    if (player.subClass == 0) return;
    player.skill3Cooldown = Skill3MaxCd();
    player.skill3Timer = player.skill3Cooldown;
    float critMul = CritMult();
    float baseAng = atan2f(aimDir.y, aimDir.x);

    if (player.heroClass == CLASS_KNIGHT) {
        if (player.subClass == 1) {
            // Holy Paladin: Búa Thánh Phán Quyết — Ném 3 búa thánh xuyên thấu (2.1x ST, Choáng 1.2s) & hồi 8% Máu
            PlaySound(snd.sndSpecial);
            Shake(0.22f, 5.0f);
            for (int i = -1; i <= 1; ++i) {
                float a = baseAng + i * 0.24f;
                Projectile hm;
                hm.fromPlayer = true; hm.kind = 6;
                hm.pos = Vector2Add(player.pos, { cosf(a) * 20.0f, sinf(a) * 20.0f });
                hm.vel = { cosf(a) * 520.0f, sinf(a) * 520.0f };
                hm.damage = player.baseDmg * 2.1f; hm.life = 1.4f; hm.radius = 15.0f;
                hm.pierce = 3; hm.splashRadius = 55.0f; hm.splashRatio = 0.45f;
                hm.applyStun = true; hm.color = GOLD;
                projectiles.push_back(hm);
            }
            float hl = std::max(5.0f, roundf(player.maxHp * 0.08f));
            player.hp = std::min(player.maxHp, player.hp + hl);
            SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)hl), GREEN);
            Burst(player.pos, GOLD, 16, 220.0f, 0.45f);
        } else {
            // Dread Berserker: Huyết Địa Chấn — 3 cột huyết nham tiến tới (2.4x AoE + Đốt) & hồi 12% Máu
            PlaySound(snd.sndSpecial);
            Shake(0.28f, 6.5f);
            const Color bCol = { 245, 55, 65, 255 };
            for (int k = 1; k <= 3; ++k) {
                Vector2 cPos = Vector2Add(player.pos, Vector2Scale(aimDir, 85.0f * k));
                NovaVisual nv; nv.pos = cPos; nv.maxRadius = 95.0f; nv.timer = 0.30f; nv.maxTimer = 0.30f; nv.color = bCol;
                novas.push_back(nv);
                Burst(cPos, bCol, 18, 250.0f, 0.5f);
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    if (Vector2Distance(cPos, e.pos) <= 95.0f + e.radius * 0.5f) {
                        bool isCrit = (Rand01() < player.critChance);
                        float dmg = player.baseDmg * 2.4f * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, cPos));
                        DamageEnemy(e, dmg, kb, 210.0f, isCrit, true);
                        if (e.hp > 0) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, player.baseDmg * 0.65f); }
                    }
                }
                TriggerBarrelsInRadius(cPos, 95.0f, player.baseDmg * 2.4f);
            }
            float hl = std::max(6.0f, roundf(player.maxHp * 0.12f));
            player.hp = std::min(player.maxHp, player.hp + hl);
            SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)hl), GREEN);
        }
    } else if (player.heroClass == CLASS_ASSASSIN) {
        if (player.subClass == 1) {
            // Phantom Reaper: Bầy Oan Hồn — Thả 6 Tinh Linh dẫn đường (1.35x ST + Độc/Chậm) & hồi 18 HP
            PlaySound(snd.sndShoot);
            Shake(0.14f, 3.0f);
            const Color sCol = { 195, 90, 255, 255 };
            for (int i = 0; i < 6; ++i) {
                float a = baseAng + (i - 2.5f) * 0.45f;
                Projectile w;
                w.fromPlayer = true; w.kind = 5;
                w.pos = Vector2Add(player.pos, { cosf(a) * 20.0f, sinf(a) * 20.0f });
                w.vel = { cosf(a) * 440.0f, sinf(a) * 440.0f };
                w.damage = player.baseDmg * 1.35f; w.life = 2.4f; w.radius = 8.5f;
                w.applyPoison = true; w.applySlow = true; w.color = sCol;
                projectiles.push_back(w);
            }
            player.hp = std::min(player.maxHp, player.hp + 18.0f);
            SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+18", GREEN);
            Burst(player.pos, sCol, 16, 200.0f, 0.45f);
        } else {
            // Ninja Trickster: Mưa Chông Điện — Rải 3 bẫy điện (1.9x AoE + Chậm) & ném 5 phi tiêu sét
            PlaySound(snd.sndSlash);
            Shake(0.16f, 4.0f);
            const Color eCol = { 110, 235, 255, 255 };
            Vector2 perp = { -aimDir.y, aimDir.x };
            for (int k = -1; k <= 1; ++k) {
                GroundEffect trap;
                trap.kind = 0;
                trap.pos = Vector2Add(Vector2Add(player.pos, Vector2Scale(aimDir, 45.0f)), Vector2Scale(perp, k * 75.0f));
                trap.radius = 125.0f; trap.maxTimer = 0.45f; trap.timer = 0.45f;
                trap.damage = player.baseDmg * 1.9f; trap.color = eCol;
                groundEffects.push_back(trap);
            }
            for (int i = -2; i <= 2; ++i) {
                float a = baseAng + i * 0.22f;
                Projectile sh;
                sh.fromPlayer = true; sh.kind = 3;
                sh.pos = Vector2Add(player.pos, { cosf(a) * 18.0f, sinf(a) * 18.0f });
                sh.vel = { cosf(a) * 640.0f, sinf(a) * 640.0f };
                sh.damage = player.baseDmg * 1.1f; sh.life = 1.2f; sh.radius = 8.0f;
                sh.pierce = 2; sh.applySlow = true; sh.color = eCol;
                projectiles.push_back(sh);
            }
            Burst(player.pos, eCol, 16, 220.0f, 0.4f);
        }
    } else if (player.heroClass == CLASS_MAGE) {
        if (player.subClass == 1) {
            // Spellblade: Thập Tự Ánh Trăng — Chém thập tự (2.3x ST) phóng 5 Kiếm Khí Ánh Trăng + Khiên
            PlaySound(snd.sndSlash);
            PlaySound(snd.sndShield);
            Shake(0.22f, 5.5f);
            const Color mCol = { 85, 235, 255, 255 };
            Vector2 cPos = Vector2Add(player.pos, Vector2Scale(aimDir, 52.0f));
            for (int k = -1; k <= 1; k += 2) {
                SlashVisual sv; sv.pos = cPos; sv.angle = baseAng * RAD2DEG + k * 35.0f; sv.kind = 11;
                sv.timer = 0.24f; sv.maxTimer = 0.24f; sv.color = mCol;
                slashes.push_back(sv);
            }
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(cPos, e.pos) <= 115.0f + e.radius * 0.5f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 2.3f * (isCrit ? critMul : 1.0f);
                    DamageEnemy(e, dmg, Vector2Normalize(Vector2Subtract(e.pos, player.pos)), 220.0f, isCrit, true);
                }
            }
            TriggerBarrelsInRadius(cPos, 115.0f, player.baseDmg * 2.3f);
            for (int i = -2; i <= 2; ++i) {
                float a = baseAng + i * 0.20f;
                FireSwordWave(player.pos, { cosf(a), sinf(a) }, player.baseDmg * 1.25f, 620.0f, 3, mCol, 14.0f, false);
            }
            player.shieldActive = true;
            Burst(cPos, mCol, 22, 260.0f, 0.5f);
            TriggerArcaneResonance(aimDir);
        } else {
            // Archmage: Cầu Sấm Liên Hoàn — 3 cầu sấm (1.9x AoE) + giật sét lan 6 mục tiêu (Choáng 0.9s)
            PlaySound(snd.sndSpecial);
            Shake(0.20f, 5.0f);
            const Color tCol = { 130, 225, 255, 255 };
            for (int i = -1; i <= 1; ++i) {
                float a = baseAng + i * 0.28f;
                Projectile orb;
                orb.fromPlayer = true; orb.kind = 2;
                orb.pos = Vector2Add(player.pos, { cosf(a) * 22.0f, sinf(a) * 22.0f });
                orb.vel = { cosf(a) * 480.0f, sinf(a) * 480.0f };
                orb.damage = player.baseDmg * 1.9f; orb.life = 1.4f; orb.radius = 12.0f;
                orb.splashRadius = 90.0f; orb.splashRatio = 0.65f; orb.applyStun = true; orb.color = tCol;
                projectiles.push_back(orb);
            }
            int zaps = 0;
            for (auto& e : enemies) {
                if (e.hp <= 0 || zaps >= 6) continue;
                if (Vector2Distance(player.pos, e.pos) <= 360.0f) {
                    zaps++;
                    bool isCrit = (Rand01() < player.critChance);
                    DamageEnemy(e, player.baseDmg * 1.6f * (isCrit ? critMul : 1.0f), Vector2Normalize(Vector2Subtract(e.pos, player.pos)), 120.0f, isCrit, true);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 0.9f);
                    Burst(e.pos, GOLD, 10, 180.0f, 0.35f);
                }
            }
            TriggerArcaneResonance(aimDir);
        }
    } else if (player.heroClass == CLASS_RANGER) {
        if (player.subClass == 1) {
            // Sniper Deadeye: Tiễn Xuyên Phá — Bắn Tên Siêu Thanh 3.0x Chí Mạng xuyên toàn bộ & nổ 80px
            PlaySound(snd.sndShoot);
            Shake(0.22f, 5.5f);
            Projectile rail;
            rail.fromPlayer = true; rail.kind = 1;
            rail.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 22.0f));
            rail.vel = Vector2Scale(aimDir, 1050.0f);
            rail.damage = player.baseDmg * 3.0f; rail.life = 1.6f; rail.radius = 12.0f;
            rail.pierce = 99; rail.hasPreCrit = true; rail.preCrit = true;
            rail.splashRadius = 80.0f; rail.splashRatio = 0.6f;
            rail.color = { 125, 255, 185, 255 };
            projectiles.push_back(rail);
            Burst(rail.pos, rail.color, 18, 280.0f, 0.45f);
        } else {
            // Tempest Valkyrie: Lao Sấm Valkyrie — Phóng 3 ngọn Lao Sét xuyên 4 mục tiêu (2.1x AoE) + Khiên Sấm
            PlaySound(snd.sndShoot);
            PlaySound(snd.sndShield);
            Shake(0.20f, 5.0f);
            const Color vCol = { 100, 235, 255, 255 };
            for (int i = -1; i <= 1; ++i) {
                float a = baseAng + i * 0.22f;
                Projectile sp;
                sp.fromPlayer = true; sp.kind = 1;
                sp.pos = Vector2Add(player.pos, { cosf(a) * 20.0f, sinf(a) * 20.0f });
                sp.vel = { cosf(a) * 780.0f, sinf(a) * 780.0f };
                sp.damage = player.baseDmg * 2.1f; sp.life = 1.5f; sp.radius = 10.5f;
                sp.pierce = 4; sp.splashRadius = 65.0f; sp.splashRatio = 0.55f;
                sp.applyStun = true; sp.color = vCol;
                projectiles.push_back(sp);
            }
            player.shieldActive = true;
            Burst(player.pos, vCol, 18, 240.0f, 0.45f);
        }
    } else {
        // CLASS_TEMPLAR [C]
        if (player.subClass == 1) {
            PlaySound(snd.sndShoot);
            Shake(0.20f, 5.0f);
            for (int i = -2; i <= 2; ++i) {
                float a = baseAng + i * 0.16f;
                Projectile jv; jv.fromPlayer = true; jv.kind = 1;
                jv.pos = Vector2Add(player.pos, { cosf(a) * 20.0f, sinf(a) * 20.0f });
                jv.vel = { cosf(a) * 760.0f, sinf(a) * 760.0f };
                jv.damage = player.baseDmg * 2.15f; jv.life = 1.5f; jv.radius = 10.5f;
                jv.pierce = 4; jv.applyStun = true; jv.applyBurn = true; jv.color = GOLD;
                projectiles.push_back(jv);
            }
            float hl = std::max(5.0f, roundf(player.maxHp * 0.10f));
            player.hp = std::min(player.maxHp, player.hp + hl);
            SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)hl), GREEN);
            Burst(player.pos, GOLD, 20, 250.0f, 0.45f);
        } else {
            PlaySound(snd.sndSlash);
            Shake(0.24f, 6.0f);
            const Color dCol = { 255, 195, 75, 255 };
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 175.0f; nv.timer = 0.30f; nv.maxTimer = 0.30f; nv.color = dCol;
            novas.push_back(nv);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 175.0f + e.radius * 0.5f) {
                    bool isCrit = (Rand01() < player.critChance);
                    DamageEnemy(e, player.baseDmg * 2.4f * (isCrit ? critMul : 1.0f), Vector2Normalize(Vector2Subtract(e.pos, player.pos)), 220.0f, isCrit, true);
                }
            }
            TriggerBarrelsInRadius(player.pos, 175.0f, player.baseDmg * 2.4f);
            for (int i = 0; i < 8; ++i) {
                float a = i * (PI / 4.0f);
                FireSwordWave(player.pos, { cosf(a), sinf(a) }, player.baseDmg * 1.65f, 580.0f, 3, dCol, 14.0f, true);
            }
            Burst(player.pos, dCol, 24, 280.0f, 0.5f);
        }
    }
}

inline void Game::CastSkill4(Vector2 aimDir, Vector2 moveDir) {
    if (player.subClass == 0) return;
    (void)moveDir;
    player.skill4Cooldown = Skill4MaxCd();
    player.skill4Timer = player.skill4Cooldown;
    float critMul = CritMult();
    float baseAng = atan2f(aimDir.y, aimDir.x);
    const float bw = roomW / 2.0f - 30.0f, bh = roomH / 2.0f - 30.0f;

    if (player.heroClass == CLASS_KNIGHT) {
        if (player.subClass == 1) {
            // Holy Paladin: Xiềng Xích Thiên Đường — 210px xích quang kéo quái tụ lại (2.3x ST + Đốt) & +15 Nộ
            PlaySound(snd.sndSpecial);
            Shake(0.26f, 6.0f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 210.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = GOLD;
            novas.push_back(nv);
            Burst(player.pos, GOLD, 26, 300.0f, 0.55f);
            Vector2 pullCenter = Vector2Add(player.pos, Vector2Scale(aimDir, 42.0f));
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 210.0f + e.radius * 0.5f) {
                    Vector2 pullDir = Vector2Normalize(Vector2Subtract(pullCenter, e.pos));
                    e.pos = Vector2Add(e.pos, Vector2Scale(pullDir, std::min(95.0f, Vector2Distance(e.pos, pullCenter) * 0.65f)));
                    bool isCrit = (Rand01() < player.critChance);
                    DamageEnemy(e, player.baseDmg * 2.3f * (isCrit ? critMul : 1.0f), { 0, 0 }, 0.0f, isCrit, true);
                    if (e.hp > 0) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, player.baseDmg * 0.55f); }
                }
            }
            TriggerBarrelsInRadius(player.pos, 210.0f, player.baseDmg * 2.3f);
            AddRage(15.0f);
        } else {
            // Dread Berserker: Cơn Lốc Xé Xác — Phóng 8 Huyết Kiếm Khí tỏa tròn (1.85x ST) + Tăng Tốc 2.5s
            PlaySound(snd.sndSlash);
            Shake(0.24f, 6.0f);
            const Color bCol = { 255, 75, 75, 255 };
            for (int i = 0; i < 8; ++i) {
                float a = i * (PI / 4.0f);
                FireSwordWave(player.pos, { cosf(a), sinf(a) }, player.baseDmg * 1.85f, 560.0f, 3, bCol, 14.0f, true);
            }
            player.hasteTimer = std::max(player.hasteTimer, 2.5f);
            Burst(player.pos, bCol, 24, 280.0f, 0.5f);
        }
    } else if (player.heroClass == CLASS_ASSASSIN) {
        if (player.subClass == 1) {
            // Phantom Reaper: Lưỡi Hái Tử Địa — Quét 185px (2.8x Chí Mạng + Đóng Băng 1.4s) & +20 Nộ
            PlaySound(snd.sndSlash);
            PlaySound(snd.sndFreeze);
            Shake(0.26f, 6.0f);
            const Color rCol = { 195, 95, 255, 255 };
            SlashVisual sv; sv.pos = player.pos; sv.angle = baseAng * RAD2DEG; sv.kind = 12;
            sv.timer = 0.28f; sv.maxTimer = 0.28f; sv.color = rCol;
            slashes.push_back(sv);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 185.0f; nv.timer = 0.30f; nv.maxTimer = 0.30f; nv.color = rCol;
            novas.push_back(nv);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 185.0f + e.radius * 0.5f) {
                    DamageEnemy(e, player.baseDmg * 2.8f * critMul, Vector2Normalize(Vector2Subtract(e.pos, player.pos)), 220.0f, true, true);
                    if (e.hp > 0) { e.stunTimer = std::max(e.stunTimer, 1.4f); e.chillTimer = std::max(e.chillTimer, 3.0f); }
                }
            }
            TriggerBarrelsInRadius(player.pos, 185.0f, player.baseDmg * 2.8f);
            AddRage(20.0f);
            Burst(player.pos, rCol, 26, 300.0f, 0.55f);
        } else {
            // Ninja Trickster: Lôi Thiểm Nhất Trảm — Lướt sét 180px (2.7x Chí Mạng + Choáng 1.1s) & hồi ngay Dash
            PlaySound(snd.sndDash);
            PlaySound(snd.sndSlash);
            Shake(0.24f, 6.0f);
            const Color lCol = { 110, 235, 255, 255 };
            Vector2 startP = player.pos;
            Vector2 dest = Vector2Add(startP, Vector2Scale(aimDir, 180.0f));
            dest.x = Clamp(dest.x, -bw, bw); dest.y = Clamp(dest.y, -bh, bh);
            ResolvePillars(dest, 15.0f);
            ghosts.push_back({ startP, player.facingLeft, 0.28f, 0.28f });
            player.pos = dest;
            player.invulnTimer = std::max(player.invulnTimer, 0.35f);
            player.dashTimer = 0.0f; // Hồi ngay Dash
            Vector2 mid = Vector2Scale(Vector2Add(startP, dest), 0.5f);
            SlashVisual sv; sv.pos = mid; sv.angle = baseAng * RAD2DEG; sv.kind = 11;
            sv.timer = 0.25f; sv.maxTimer = 0.25f; sv.color = lCol;
            slashes.push_back(sv);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(startP, e.pos) <= 130.0f || Vector2Distance(mid, e.pos) <= 130.0f || Vector2Distance(dest, e.pos) <= 130.0f) {
                    DamageEnemy(e, player.baseDmg * 2.7f * critMul, aimDir, 220.0f, true, true);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.1f);
                }
            }
            TriggerBarrelsInRadius(dest, 120.0f, player.baseDmg * 2.7f);
            Burst(dest, lCol, 24, 280.0f, 0.5f);
        }
    } else if (player.heroClass == CLASS_MAGE) {
        if (player.subClass == 1) {
            // Spellblade: Kiếm Trận Tinh Tú — Triệu 6 Thần Kiếm giáng xuống (2.1x AoE + Đốt) & +18 Nộ
            PlaySound(snd.sndSpecial);
            Shake(0.24f, 5.5f);
            const Color sCol = { 95, 235, 255, 255 };
            Vector2 targetC = Vector2Add(player.pos, Vector2Scale(aimDir, 140.0f));
            targetC.x = Clamp(targetC.x, -bw + 40.0f, bw - 40.0f);
            targetC.y = Clamp(targetC.y, -bh + 40.0f, bh - 40.0f);
            for (int k = 0; k < 6; ++k) {
                float a = k * (PI / 3.0f);
                GroundEffect sw;
                sw.kind = 5;
                sw.pos = { Clamp(targetC.x + cosf(a) * 88.0f, -bw, bw), Clamp(targetC.y + sinf(a) * 88.0f, -bh, bh) };
                sw.radius = 95.0f;
                sw.maxTimer = 0.22f + k * 0.08f;
                sw.timer = sw.maxTimer;
                sw.damage = player.baseDmg * 2.1f;
                sw.color = (k & 1) ? sCol : GOLD;
                groundEffects.push_back(sw);
            }
            AddRage(18.0f);
            TriggerArcaneResonance(aimDir);
        } else {
            // Archmage: Cột Sáng Hủy Diệt — Cột quang (2.6x AoE + vùng đốt 3s) & 6 tia dẫn đường
            PlaySound(snd.sndSpecial);
            Shake(0.28f, 7.0f);
            const Color cCol = { 255, 195, 75, 255 };
            Vector2 pPos = Vector2Add(player.pos, Vector2Scale(aimDir, 150.0f));
            pPos.x = Clamp(pPos.x, -bw + 30.0f, bw - 30.0f);
            pPos.y = Clamp(pPos.y, -bh + 30.0f, bh - 30.0f);
            NovaVisual nv; nv.pos = pPos; nv.maxRadius = 175.0f; nv.timer = 0.40f; nv.maxTimer = 0.40f; nv.color = cCol;
            novas.push_back(nv);
            Burst(pPos, cCol, 32, 340.0f, 0.65f);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(pPos, e.pos) <= 175.0f + e.radius * 0.5f) {
                    bool isCrit = (Rand01() < player.critChance);
                    DamageEnemy(e, player.baseDmg * 2.6f * (isCrit ? critMul : 1.0f), Vector2Normalize(Vector2Subtract(e.pos, pPos)), 240.0f, isCrit, true);
                    if (e.hp > 0) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, player.baseDmg * 0.7f); }
                }
            }
            TriggerBarrelsInRadius(pPos, 175.0f, player.baseDmg * 2.6f);
            GroundEffect hz; hz.kind = 1; hz.pos = pPos; hz.radius = 155.0f; hz.maxTimer = 3.0f; hz.timer = 3.0f; hz.tickTimer = 0.35f;
            hz.damage = player.baseDmg * 0.75f; hz.color = cCol;
            groundEffects.push_back(hz);
            for (int m = 0; m < 6; ++m) {
                float a = m * (PI / 3.0f);
                Projectile hm; hm.fromPlayer = true; hm.kind = 5;
                hm.pos = Vector2Add(player.pos, { cosf(a) * 20.0f, sinf(a) * 20.0f });
                hm.vel = { cosf(a) * 440.0f, sinf(a) * 440.0f };
                hm.damage = player.baseDmg * 1.2f; hm.life = 2.4f; hm.radius = 8.0f;
                hm.color = { 195, 115, 255, 255 };
                projectiles.push_back(hm);
            }
            TriggerArcaneResonance(aimDir);
        }
    } else if (player.heroClass == CLASS_RANGER) {
        if (player.subClass == 1) {
            // Sniper Deadeye: Pháo Sáng & Ẩn Thân — 6 tên pháo sáng (1.7x AoE) + 1.5s Bất Tử & 3s Tăng Tốc
            PlaySound(snd.sndShoot);
            PlaySound(snd.sndShield);
            Shake(0.20f, 5.0f);
            const Color fCol = { 255, 220, 85, 255 };
            for (int i = 0; i < 6; ++i) {
                float a = baseAng + (i - 2.5f) * 0.16f;
                Projectile fl; fl.fromPlayer = true; fl.kind = 1;
                fl.pos = Vector2Add(player.pos, { cosf(a) * 20.0f, sinf(a) * 20.0f });
                fl.vel = { cosf(a) * 700.0f, sinf(a) * 700.0f };
                fl.damage = player.baseDmg * 1.7f; fl.life = 1.4f; fl.radius = 9.0f;
                fl.splashRadius = 70.0f; fl.splashRatio = 0.55f; fl.applyBurn = true; fl.color = fCol;
                projectiles.push_back(fl);
            }
            player.invulnTimer = std::max(player.invulnTimer, 1.5f);
            player.hasteTimer = std::max(player.hasteTimer, 3.0f);
            Burst(player.pos, fCol, 22, 260.0f, 0.5f);
        } else {
            // Tempest Valkyrie: Tâm Bão Lôi Đình — Sóng sét 200px (2.2x + Choáng 1.1s), 8 tên sét & +20 Nộ
            PlaySound(snd.sndSpecial);
            Shake(0.26f, 6.5f);
            const Color vCol = { 100, 235, 255, 255 };
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 200.0f; nv.timer = 0.34f; nv.maxTimer = 0.34f; nv.color = vCol;
            novas.push_back(nv);
            Burst(player.pos, vCol, 28, 320.0f, 0.55f);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 200.0f + e.radius * 0.5f) {
                    bool isCrit = (Rand01() < player.critChance);
                    DamageEnemy(e, player.baseDmg * 2.2f * (isCrit ? critMul : 1.0f), Vector2Normalize(Vector2Subtract(e.pos, player.pos)), 360.0f, isCrit, true);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.1f);
                }
            }
            TriggerBarrelsInRadius(player.pos, 200.0f, player.baseDmg * 2.2f);
            for (int i = 0; i < 8; ++i) {
                float a = i * (PI / 4.0f);
                Projectile sa; sa.fromPlayer = true; sa.kind = 1;
                sa.pos = Vector2Add(player.pos, { cosf(a) * 18.0f, sinf(a) * 18.0f });
                sa.vel = { cosf(a) * 700.0f, sinf(a) * 700.0f };
                sa.damage = player.baseDmg * 1.35f; sa.life = 1.4f; sa.radius = 8.5f;
                sa.pierce = 2; sa.color = vCol;
                projectiles.push_back(sa);
            }
            AddRage(20.0f);
        }
    } else {
        // CLASS_TEMPLAR [V]
        if (player.subClass == 1) {
            PlaySound(snd.sndSpecial);
            Shake(0.26f, 6.0f);
            Vector2 cPos = Vector2Add(player.pos, Vector2Scale(aimDir, 95.0f));
            cPos.x = Clamp(cPos.x, -bw + 30.0f, bw - 30.0f);
            cPos.y = Clamp(cPos.y, -bh + 30.0f, bh - 30.0f);
            NovaVisual nv; nv.pos = cPos; nv.maxRadius = 180.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = GOLD;
            novas.push_back(nv);
            Burst(cPos, GOLD, 30, 320.0f, 0.55f);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(cPos, e.pos) <= 180.0f + e.radius * 0.5f) {
                    bool isCrit = (Rand01() < player.critChance);
                    if (e.type == BOSS || e.isElite || e.isWarden) e.stagger = std::min(e.maxStagger, e.stagger + 45.0f);
                    DamageEnemy(e, player.baseDmg * 2.65f * (isCrit ? critMul : 1.0f), Vector2Normalize(Vector2Subtract(e.pos, cPos)), 220.0f, isCrit, true);
                    if (e.hp > 0) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, player.baseDmg * 0.65f); }
                }
            }
            TriggerBarrelsInRadius(cPos, 180.0f, player.baseDmg * 2.65f);
            GroundEffect hz; hz.kind = 1; hz.pos = cPos; hz.radius = 150.0f; hz.maxTimer = 2.5f; hz.timer = 2.5f; hz.tickTimer = 0.4f;
            hz.damage = player.baseDmg * 0.7f; hz.color = GOLD;
            groundEffects.push_back(hz);
            AddRage(20.0f);
        } else {
            PlaySound(snd.sndSpecial);
            Shake(0.24f, 6.0f);
            const Color dCol = { 245, 75, 95, 255 };
            Vector2 targetC = Vector2Add(player.pos, Vector2Scale(aimDir, 130.0f));
            targetC.x = Clamp(targetC.x, -bw + 40.0f, bw - 40.0f);
            targetC.y = Clamp(targetC.y, -bh + 40.0f, bh - 40.0f);
            for (int k = 0; k < 6; ++k) {
                float a = k * (PI / 3.0f);
                GroundEffect pl; pl.kind = 4;
                pl.pos = { Clamp(targetC.x + cosf(a) * 90.0f, -bw, bw), Clamp(targetC.y + sinf(a) * 90.0f, -bh, bh) };
                pl.radius = 100.0f; pl.maxTimer = 0.24f + k * 0.08f; pl.timer = pl.maxTimer;
                pl.damage = player.baseDmg * 2.3f; pl.color = (k & 1) ? GOLD : dCol;
                groundEffects.push_back(pl);
            }
            player.dashTimer = 0.0f;
            AddRage(20.0f);
        }
    }
}

inline void Game::CastUltimate(Vector2 aimDir) {
    player.rage = 0.0f;
    PlaySound(snd.sndSpecial);
    PlaySound(snd.sndLevelUp);
    Shake(0.45f, 9.0f);
    float critMul = CritMult();

    if (player.heroClass == CLASS_KNIGHT) {
        if (player.subClass == 2) {
            // Dread Berserker [R]: Cataclysmic Guillotine + 4s Blood Vortex
            ShowBanner("CATACLYSMIC GUILLOTINE!", 2.4f);
            player.hasteTimer = std::max(player.hasteTimer, 4.0f);
            Burst(player.pos, { 235, 65, 65, 255 }, 48, 440.0f, 0.85f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 270.0f; nv.timer = 0.45f; nv.maxTimer = 0.45f; nv.color = { 235, 65, 65, 255 };
            novas.push_back(nv);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 270.0f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 4.2f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 260.0f, isCrit, false);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.2f);
                }
            }
            TriggerBarrelsInRadius(player.pos, 270.0f, player.baseDmg * 4.2f);
            GroundEffect bv; bv.kind = 2; bv.pos = player.pos; bv.radius = 250.0f; bv.maxTimer = 4.0f; bv.timer = 4.0f; bv.tickTimer = 0.15f;
            bv.damage = player.baseDmg * 1.45f; bv.color = { 235, 55, 65, 255 };
            groundEffects.push_back(bv);
        } else {
            // Vanguard Knight / Holy Paladin [R]: Judgment of Solaris
            bool isPal = (player.subClass == 1);
            float rad = isPal ? 280.0f : 250.0f;
            float mul = isPal ? 4.2f : 3.5f;
            ShowBanner(isPal ? "JUDGMENT OF SOLARIS!" : "HEAVEN'S JUDGMENT!", 2.2f);
            Burst(player.pos, GOLD, 48, 440.0f, 0.85f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = rad; nv.timer = 0.45f; nv.maxTimer = 0.45f; nv.color = GOLD;
            novas.push_back(nv);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= rad) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * mul * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 260.0f, isCrit, false);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, isPal ? 2.0f : 1.5f);
                }
            }
            TriggerBarrelsInRadius(player.pos, rad, player.baseDmg * mul);
            GroundEffect zone; zone.kind = 1; zone.pos = player.pos; zone.radius = rad;
            zone.maxTimer = isPal ? 4.5f : 4.0f; zone.timer = zone.maxTimer; zone.tickTimer = 0.40f;
            zone.damage = player.baseDmg * (isPal ? 1.1f : 0.85f); zone.color = GOLD;
            groundEffects.push_back(zone);
        }
    } else if (player.heroClass == CLASS_ASSASSIN) {
        ShowBanner(player.subClass == 1 ? "WRAITH HURRICANE!" : (player.subClass == 2 ? "THOUSAND BLADES!" : "BLADE STORM!"), 2.2f);
        player.ultActiveTimer = (player.subClass == 1) ? 4.0f : (player.subClass == 2 ? 3.8f : 3.5f);
        player.ultTickTimer = 0.0f;
        player.invulnTimer = std::max(player.invulnTimer, 0.50f);
        Color uc = (player.subClass == 1) ? Color{ 195, 90, 255, 255 } : (player.subClass == 2 ? Color{ 110, 235, 255, 255 } : Color{ 255, 65, 130, 255 });
        Burst(player.pos, uc, 42, 400.0f, 0.7f);
    } else if (player.heroClass == CLASS_MAGE) {
        if (player.subClass == 1) {
            // Spellblade (Kiếm Sư) [R]: Excalibur Astral Storm
            ShowBanner("EXCALIBUR ASTRAL STORM!", 2.4f);
            Burst(player.pos, { 85, 230, 255, 255 }, 48, 440.0f, 0.85f);
            NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 260.0f; nv.timer = 0.45f; nv.maxTimer = 0.45f; nv.color = { 85, 230, 255, 255 };
            novas.push_back(nv);
            player.swordOrbitTimer = 4.0f;
            player.swordOrbitTick = 0.0f;
            player.hasteTimer = std::max(player.hasteTimer, 4.0f);
            player.shieldActive = true;
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 260.0f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 3.8f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 260.0f, isCrit, false);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.5f);
                }
            }
            TriggerBarrelsInRadius(player.pos, 260.0f, player.baseDmg * 3.8f);
            Vector2 center = Vector2Add(player.pos, Vector2Scale(aimDir, 110.0f));
            center.x = Clamp(center.x, -(roomW / 2.0f - 120.0f), roomW / 2.0f - 120.0f);
            center.y = Clamp(center.y, -(roomH / 2.0f - 100.0f), roomH / 2.0f - 100.0f);
            GroundEffect srain; srain.kind = 3; srain.pos = center; srain.radius = 260.0f; srain.maxTimer = 4.0f; srain.timer = 4.0f;
            srain.tickTimer = 0.10f; srain.damage = player.baseDmg * 1.35f; srain.color = { 105, 230, 255, 255 };
            groundEffects.push_back(srain);
            TriggerArcaneResonance(aimDir);
        } else {
            // Arcane Mage / Archmage [R]: Void Cataclysm / Cosmic Singularity
            bool isArch = (player.subClass == 2);
            ShowBanner(isArch ? "COSMIC SINGULARITY!" : "VOID CATACLYSM!", 2.2f);
            if (isArch && player.hp < player.maxHp) {
                float heal = player.maxHp * 0.25f;
                player.hp = std::min(player.maxHp, player.hp + heal);
                SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)heal), GREEN);
            }
            Vector2 center = Vector2Add(player.pos, Vector2Scale(aimDir, 150.0f));
            center.x = Clamp(center.x, -(roomW / 2.0f - 120.0f), roomW / 2.0f - 120.0f);
            center.y = Clamp(center.y, -(roomH / 2.0f - 100.0f), roomH / 2.0f - 100.0f);
            GroundEffect vortex; vortex.kind = 2; vortex.pos = center;
            vortex.radius = isArch ? 270.0f : 240.0f;
            vortex.maxTimer = isArch ? 4.2f : 3.8f; vortex.timer = vortex.maxTimer; vortex.tickTimer = 0.15f;
            vortex.damage = player.baseDmg * (isArch ? 1.75f : 1.5f);
            vortex.color = { 175, 90, 255, 255 };
            groundEffects.push_back(vortex);
            Burst(center, { 175, 90, 255, 255 }, 38, 360.0f, 0.8f);
            TriggerArcaneResonance(aimDir);
        }
    } else if (player.heroClass == CLASS_RANGER) {
        // Emerald Ranger / Sniper Deadeye / Valkyrie Lancer [R]
        bool isVal = (player.subClass == 2);
        ShowBanner(isVal ? "GUNGNIR SKYFALL!" : (player.subClass == 1 ? "SOLARIS ARROW TEMPEST!" : "PHOENIX ARROW RAIN!"), 2.2f);
        player.hasteTimer = (player.subClass > 0) ? 4.0f : 3.6f;
        if (isVal) { player.holyBuffTimer = 4.0f; player.shieldActive = true; }
        Vector2 center = Vector2Add(player.pos, Vector2Scale(aimDir, 130.0f));
        center.x = Clamp(center.x, -(roomW / 2.0f - 120.0f), roomW / 2.0f - 120.0f);
        center.y = Clamp(center.y, -(roomH / 2.0f - 100.0f), roomH / 2.0f - 100.0f);
        GroundEffect rain; rain.kind = 3; rain.pos = center;
        rain.radius = (player.subClass > 0) ? 275.0f : 250.0f;
        rain.maxTimer = (player.subClass == 1) ? 4.0f : (isVal ? 3.8f : 3.6f);
        rain.timer = rain.maxTimer; rain.tickTimer = 0.10f;
        rain.damage = player.baseDmg * (player.subClass > 0 ? 1.35f : 1.15f);
        rain.color = isVal ? Color{ 100, 235, 255, 255 } : Color{ 255, 160, 55, 255 };
        groundEffects.push_back(rain);
        Burst(player.pos, rain.color, 32, 340.0f, 0.65f);
    } else {
        // Royal Lancemaster / Grand Inquisitor / Dragonlord Monarch [R]
        bool isDrag = (player.subClass == 2);
        ShowBanner(isDrag ? L("DYNASTY ANNIHILATION!", "HOÀNG TRIỀU HỦY DIỆT!") : (player.subClass == 1 ? L("HEAVENSFALL PHALANX!", "THIÊN PHẠT ĐẠI TRẬN!") : L("SOVEREIGN'S PHALANX!", "THIÊN QUÂN VẠN MÃ!")), 2.5f);
        overdriveTimer = std::max(overdriveTimer, 4.5f);
        player.holyBuffTimer = 4.5f;
        player.shieldActive = true;
        Color uCol = isDrag ? Color{ 245, 75, 95, 255 } : GOLD;
        float mul = isDrag ? 4.5f : (player.subClass == 1 ? 4.3f : 3.8f);
        Burst(player.pos, uCol, 50, 440.0f, 0.85f);
        NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 275.0f; nv.timer = 0.45f; nv.maxTimer = 0.45f; nv.color = uCol;
        novas.push_back(nv);
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(player.pos, e.pos) <= 275.0f) {
                bool isCrit = (Rand01() < player.critChance);
                DamageEnemy(e, player.baseDmg * mul * (isCrit ? critMul : 1.0f), Vector2Normalize(Vector2Subtract(e.pos, player.pos)), 260.0f, isCrit, false);
                if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.6f);
            }
        }
        TriggerBarrelsInRadius(player.pos, 275.0f, player.baseDmg * mul);
        Vector2 center = Vector2Add(player.pos, Vector2Scale(aimDir, 120.0f));
        center.x = Clamp(center.x, -(roomW / 2.0f - 120.0f), roomW / 2.0f - 120.0f);
        center.y = Clamp(center.y, -(roomH / 2.0f - 100.0f), roomH / 2.0f - 100.0f);
        if (isDrag) {
            GroundEffect bv; bv.kind = 2; bv.pos = center; bv.radius = 260.0f; bv.maxTimer = 4.2f; bv.timer = 4.2f; bv.tickTimer = 0.14f;
            bv.damage = player.baseDmg * 1.55f; bv.color = uCol;
            groundEffects.push_back(bv);
        } else {
            GroundEffect srain; srain.kind = 3; srain.pos = center; srain.radius = 275.0f; srain.maxTimer = 4.2f; srain.timer = 4.2f;
            srain.tickTimer = 0.10f; srain.damage = player.baseDmg * 1.35f; srain.color = GOLD;
            groundEffects.push_back(srain);
            if (player.subClass == 1) {
                GroundEffect hz; hz.kind = 1; hz.pos = player.pos; hz.radius = 220.0f; hz.maxTimer = 4.5f; hz.timer = 4.5f; hz.tickTimer = 0.40f;
                hz.damage = player.baseDmg * 0.95f; hz.color = GOLD;
                groundEffects.push_back(hz);
            }
        }
    }
}

inline void Game::UpdateActiveSkills(float dt) {
    float critMul = CritMult();
    // Kiếm Hồn xoay quanh Spellblade (Kiếm Sư)
    if (player.swordOrbitTimer > 0.0f) {
        player.swordOrbitTimer -= dt;
        player.swordOrbitTick -= dt;
        if (player.swordOrbitTick <= 0.0f) {
            player.swordOrbitTick = 0.22f;
            PlaySound(snd.sndSlash);
            SlashVisual sv;
            sv.pos = player.pos;
            sv.angle = fmodf((float)GetTime() * 820.0f, 360.0f);
            sv.kind = 12;
            sv.timer = 0.18f;
            sv.maxTimer = 0.18f;
            sv.color = { 85, 230, 255, 255 };
            slashes.push_back(sv);
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 170.0f + e.radius * 0.4f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 0.75f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 120.0f, isCrit, false);
                }
            }
            TriggerBarrelsInRadius(player.pos, 170.0f, player.baseDmg * 0.75f);
        }
    }

    // Bão kiếm Sát Thủ / Tử Thần / Nhẫn Giả
    if (player.ultActiveTimer > 0.0f) {
        player.ultActiveTimer -= dt;
        if (player.heroClass == CLASS_ASSASSIN) {
            player.ultTickTimer -= dt;
            if (player.ultTickTimer <= 0.0f) {
                player.ultTickTimer = (player.subClass == 2) ? 0.16f : ((player.subClass == 1) ? 0.18f : 0.20f);
                PlaySound(snd.sndSlash);
                Color uc = (player.subClass == 1) ? Color{ 195, 90, 255, 255 } : (player.subClass == 2 ? Color{ 110, 235, 255, 255 } : Color{ 255, 70, 145, 255 });
                float uRad = (player.subClass == 1) ? 210.0f : 185.0f;
                float uMul = (player.subClass == 1) ? 1.05f : 0.85f;
                SlashVisual sv;
                sv.pos = player.pos;
                sv.angle = fmodf((float)GetTime() * 900.0f, 360.0f);
                sv.kind = 12;
                sv.timer = 0.16f;
                sv.maxTimer = 0.16f;
                sv.color = uc;
                slashes.push_back(sv);
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    if (Vector2Distance(player.pos, e.pos) <= uRad + e.radius * 0.4f) {
                        bool isCrit = (Rand01() < player.critChance);
                        float dmg = player.baseDmg * uMul * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                        DamageEnemy(e, dmg, kb, 110.0f, isCrit, false);
                        if (player.subClass == 1 && player.hp < player.maxHp) {
                            player.hp = std::min(player.maxHp, player.hp + 2.0f);
                        }
                    }
                }
                if (player.subClass == 2) { // Trickster: 4-way shurikens
                    float baseA = (float)GetTime() * 3.5f;
                    for (int k = 0; k < 4; ++k) {
                        float a = baseA + k * (PI / 2.0f);
                        Projectile p; p.fromPlayer = true; p.kind = 3;
                        p.pos = Vector2Add(player.pos, { cosf(a) * 18.0f, sinf(a) * 18.0f });
                        p.vel = { cosf(a) * 560.0f, sinf(a) * 560.0f };
                        p.damage = player.baseDmg * 0.55f; p.life = 1.0f; p.radius = 7.0f; p.pierce = 1; p.color = uc;
                        projectiles.push_back(p);
                    }
                }
                TriggerBarrelsInRadius(player.pos, uRad, player.baseDmg * uMul);
            }
        }
    }

    // Hiệu ứng mặt đất: Bẫy, Thánh vực, Hố đen, Mưa tên, Thiên thạch rơi
    std::vector<GroundEffect> spawned;
    for (size_t i = 0; i < groundEffects.size();) {
        GroundEffect& ge = groundEffects[i];
        ge.timer -= dt;

        if (ge.kind == 0) { // Bẫy nổ Ranger / Trickster
            bool triggered = (ge.timer <= 0.0f);
            if (!triggered && ge.timer <= ge.maxTimer - 0.15f) {
                for (const auto& e : enemies) {
                    if (e.hp > 0 && Vector2Distance(ge.pos, e.pos) <= 55.0f + e.radius) { triggered = true; break; }
                }
            }
            if (triggered) {
                PlaySound(snd.sndSpecial);
                Shake(0.24f, 6.0f);
                NovaVisual nv;
                nv.pos = ge.pos;
                nv.maxRadius = ge.radius;
                nv.timer = 0.32f;
                nv.maxTimer = 0.32f;
                nv.color = ge.color;
                novas.push_back(nv);
                Burst(ge.pos, ge.color, 30, 340.0f, 0.55f);
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    if (Vector2Distance(ge.pos, e.pos) <= ge.radius + e.radius * 0.4f) {
                        bool isCrit = (Rand01() < player.critChance);
                        float dmg = ge.damage * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                        DamageEnemy(e, dmg, kb, 260.0f, isCrit, true);
                        if (e.hp > 0) {
                            e.slowTimer = std::max(e.slowTimer, 3.0f);
                            e.slowFactor = std::min(e.slowFactor, 0.55f);
                            if (player.heroClass == CLASS_RANGER && player.subClass == 1) e.stunTimer = std::max(e.stunTimer, 0.9f);
                        }
                    }
                }
                TriggerBarrelsInRadius(ge.pos, ge.radius, ge.damage);
                groundEffects.erase(groundEffects.begin() + i);
                continue;
            }
        } else if (ge.kind == 1) { // Thánh vực Knight
            ge.tickTimer -= dt;
            if (ge.tickTimer <= 0.0f) {
                ge.tickTimer = 0.40f;
                Burst(ge.pos, GOLD, 8, 220.0f, 0.4f);
                if (player.hp < player.maxHp) {
                    float heal = std::max(1.0f, player.maxHp * (player.subClass == 1 ? 0.024f : 0.016f));
                    player.hp = std::min(player.maxHp, player.hp + heal);
                    SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)roundf(heal)), GREEN);
                }
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    if (Vector2Distance(ge.pos, e.pos) <= ge.radius) {
                        bool isCrit = (Rand01() < player.critChance);
                        float dmg = ge.damage * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                        DamageEnemy(e, dmg, kb, 80.0f, isCrit, false);
                    }
                }
            }
        } else if (ge.kind == 2) { // Hố đen Mage / Blood Vortex Berserker
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                float d = Vector2Distance(ge.pos, e.pos);
                if (d <= ge.radius && d > 18.0f) {
                    Vector2 pull = Vector2Normalize(Vector2Subtract(ge.pos, e.pos));
                    e.pos = Vector2Add(e.pos, Vector2Scale(pull, (e.type == BOSS ? 45.0f : 115.0f) * dt));
                }
            }
            ge.tickTimer -= dt;
            if (ge.tickTimer <= 0.0f) {
                ge.tickTimer = (player.heroClass == CLASS_MAGE && player.subClass == 2) ? 0.24f : 0.30f;
                Vector2 target = ge.pos;
                std::vector<Vector2> cands;
                for (const auto& e : enemies) if (e.hp > 0 && Vector2Distance(ge.pos, e.pos) <= ge.radius * 0.85f) cands.push_back(e.pos);
                if (!cands.empty() && Rand01() < 0.7f) {
                    Vector2 c = cands[rand() % cands.size()];
                    target = { c.x + (Rand01() - 0.5f) * 40.0f, c.y + (Rand01() - 0.5f) * 40.0f };
                } else {
                    float a = Rand01() * 2.0f * PI, rr = sqrtf(Rand01()) * (ge.radius * 0.82f);
                    target = { ge.pos.x + cosf(a) * rr, ge.pos.y + sinf(a) * rr };
                }
                GroundEffect met;
                met.kind = 4;
                met.pos = target;
                met.radius = 95.0f;
                met.maxTimer = 0.35f;
                met.timer = 0.35f;
                met.damage = ge.damage;
                met.color = ge.color;
                spawned.push_back(met);
            }
        } else if (ge.kind == 3) { // Mưa Tên / Mưa Kiếm / Thương Sấm
            ge.tickTimer -= dt;
            if (ge.tickTimer <= 0.0f) {
                ge.tickTimer = (player.heroClass == CLASS_RANGER && player.subClass == 1) ? 0.18f : 0.22f;
                Vector2 target = ge.pos;
                std::vector<Vector2> cands;
                for (const auto& e : enemies) if (e.hp > 0 && Vector2Distance(ge.pos, e.pos) <= ge.radius * 0.9f) cands.push_back(e.pos);
                if (!cands.empty() && Rand01() < 0.75f) {
                    Vector2 c = cands[rand() % cands.size()];
                    target = { c.x + (Rand01() - 0.5f) * 35.0f, c.y + (Rand01() - 0.5f) * 35.0f };
                } else {
                    float a = Rand01() * 2.0f * PI, rr = sqrtf(Rand01()) * (ge.radius * 0.88f);
                    target = { ge.pos.x + cosf(a) * rr, ge.pos.y + sinf(a) * rr };
                }
                GroundEffect arr;
                arr.kind = 5;
                arr.pos = target;
                arr.radius = 78.0f;
                arr.maxTimer = 0.25f;
                arr.timer = 0.25f;
                arr.damage = ge.damage;
                arr.color = ge.color;
                spawned.push_back(arr);
            }
        } else if (ge.kind == 4 || ge.kind == 5) { // Thiên thạch / Kiếm ánh sáng / Mũi tên rơi
            if (ge.timer <= 0.0f) {
                bool isMet = (ge.kind == 4);
                PlaySound(isMet ? snd.sndSpecial : snd.sndHit);
                Shake(0.10f, isMet ? 4.0f : 2.5f);
                NovaVisual nv;
                nv.pos = ge.pos;
                nv.maxRadius = ge.radius;
                nv.timer = 0.25f;
                nv.maxTimer = 0.25f;
                nv.color = ge.color;
                novas.push_back(nv);
                Burst(ge.pos, ge.color, isMet ? 20 : 12, 260.0f, 0.45f);
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    if (Vector2Distance(ge.pos, e.pos) <= ge.radius + e.radius * 0.4f) {
                        bool isCrit = (Rand01() < player.critChance);
                        float dmg = ge.damage * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                        DamageEnemy(e, dmg, kb, 140.0f, isCrit, false);
                        if (!isMet && e.hp > 0) {
                            e.burnTimer = std::max(e.burnTimer, 2.5f);
                            e.burnDps = std::max(e.burnDps, player.baseDmg * (0.4f + 0.5f * player.igniteLevel));
                        }
                    }
                }
                TriggerBarrelsInRadius(ge.pos, ge.radius, ge.damage);
                groundEffects.erase(groundEffects.begin() + i);
                continue;
            }
        } else if (ge.kind == 6) { // Sét Thiên Lôi từ Thời Tiết Bão Từ Ma Pháp
            if (ge.timer <= 0.0f) {
                lightningFlash = std::max(lightningFlash, 0.55f);
                PlaySound(snd.sndSpecial);
                Shake(0.22f, 5.5f);
                NovaVisual nv;
                nv.pos = ge.pos;
                nv.maxRadius = ge.radius * 1.15f;
                nv.timer = 0.28f;
                nv.maxTimer = 0.28f;
                nv.color = ge.color;
                novas.push_back(nv);
                Burst(ge.pos, { 140, 235, 255, 255 }, 24, 300.0f, 0.45f);
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    if (Vector2Distance(ge.pos, e.pos) <= ge.radius + e.radius * 0.45f) {
                        bool isCrit = (Rand01() < player.critChance);
                        float dmg = ge.damage * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                        if ((e.type == BOSS || e.isElite || e.isWarden) && e.staggerBrokenTimer <= 0.0f) {
                            e.stagger = std::min(e.maxStagger, e.stagger + 35.0f);
                        }
                        DamageEnemy(e, dmg, kb, 170.0f, isCrit, false);
                        if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 0.8f);
                    }
                }
                if (Vector2Distance(ge.pos, player.pos) <= ge.radius * 0.82f && !player.isDashing) {
                    DamagePlayer(13.0f);
                }
                TriggerBarrelsInRadius(ge.pos, ge.radius, ge.damage);
                groundEffects.erase(groundEffects.begin() + i);
                continue;
            }
        }

        if (ge.timer <= 0.0f) {
            groundEffects.erase(groundEffects.begin() + i);
        } else {
            ++i;
        }
    }
    for (const auto& s : spawned) groundEffects.push_back(s);
}
