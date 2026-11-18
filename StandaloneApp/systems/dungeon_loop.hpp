#pragma once
#include "../game.hpp"

inline const SubclassDef* Game::ActiveSubclass() const {
    if (player.subClass != 1 && player.subClass != 2) return nullptr;
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    return &kSubclasses[hc][player.subClass - 1];
}

inline const char* Game::SubDisplayName(int hc, int branch) const {
    hc = std::max(0, std::min(kNumClasses - 1, hc));
    int b = (branch == 2) ? 1 : 0;
    return settings.language == 1 ? kSubclassesVi[hc][b].name : kSubclasses[hc][b].name;
}

inline const char* Game::HeroTitle() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].name : kSubclasses[hc][player.subClass - 1].name;
    return settings.language == 1 ? kClassesVi[hc].name : kClasses[hc].name;
}

inline int Game::LevelTitleTierForLevel(int lv) {
    if (lv >= 100) return 5;
    if (lv >= 80)  return 4;
    if (lv >= 60)  return 3;
    if (lv >= 40)  return 2;
    if (lv >= 20)  return 1;
    return 0;
}

inline const char* Game::HeroRole() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].role : kSubclasses[hc][player.subClass - 1].role;
    return settings.language == 1 ? kClassesVi[hc].role : kClasses[hc].role;
}

inline const char* Game::PassiveName() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].passiveName : kSubclasses[hc][player.subClass - 1].passiveName;
    return settings.language == 1 ? kClassesVi[hc].passiveName : kClasses[hc].passiveName;
}

inline const char* Game::PassiveDesc() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].passiveDesc : kSubclasses[hc][player.subClass - 1].passiveDesc;
    return settings.language == 1 ? kClassesVi[hc].passiveDesc : kClasses[hc].passiveDesc;
}

inline const char* Game::QName() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].qName : kSubclasses[hc][player.subClass - 1].qName;
    return settings.language == 1 ? kClassesVi[hc].qName : kClasses[hc].qName;
}

inline const char* Game::QDesc() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].qDesc : kSubclasses[hc][player.subClass - 1].qDesc;
    return settings.language == 1 ? kClassesVi[hc].qDesc : kClasses[hc].qDesc;
}

inline const char* Game::EName() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].eName : kSubclasses[hc][player.subClass - 1].eName;
    return settings.language == 1 ? kClassesVi[hc].eName : kClasses[hc].eName;
}

inline const char* Game::EDesc() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].eDesc : kSubclasses[hc][player.subClass - 1].eDesc;
    return settings.language == 1 ? kClassesVi[hc].eDesc : kClasses[hc].eDesc;
}

inline const char* Game::CName() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].cName : kSubclasses[hc][player.subClass - 1].cName;
    return "";
}

inline const char* Game::CDesc() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].cDesc : kSubclasses[hc][player.subClass - 1].cDesc;
    return "";
}

inline const char* Game::VName() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].vName : kSubclasses[hc][player.subClass - 1].vName;
    return "";
}

inline const char* Game::VDesc() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].vDesc : kSubclasses[hc][player.subClass - 1].vDesc;
    return "";
}

inline const char* Game::RName() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].rName : kSubclasses[hc][player.subClass - 1].rName;
    return settings.language == 1 ? kClassesVi[hc].rName : kClasses[hc].rName;
}

inline const char* Game::RDesc() const {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].rDesc : kSubclasses[hc][player.subClass - 1].rDesc;
    return settings.language == 1 ? kClassesVi[hc].rDesc : kClasses[hc].rDesc;
}

inline int Game::VisualWeaponType(int heroClass, int subClass, int itemWeaponType) {
    (void)itemWeaponType;
    int hc = std::max(0, std::min(kNumClasses - 1, heroClass));
    if (hc == CLASS_MAGE && subClass == 1) return WP_SWORD; // Kiem Su: dung Gay nhung model la Kiem Phep
    return ClassWeaponType(hc);
}

inline float Game::SkillCdrMult() const {
    float mult = std::max(0.45f, 1.0f - 0.12f * player.cdrUpgrades);
    if (player.heroClass == CLASS_MAGE) mult *= (player.subClass == 2 ? 0.78f : 0.85f);
    if (bloodlineTrait == 2) mult *= 0.90f; // Arcane Blood
    if (!inSurface && weatherKind == 2) mult *= 0.88f; // Arcane Ion Storm: -12% hồi chiêu
    return mult;
}

inline float Game::EffectiveAttackRate() const {
    float r = player.attackRate;
    if (player.holyBuffTimer > 0.0f) r *= ((player.heroClass == CLASS_KNIGHT || player.heroClass == CLASS_TEMPLAR) && player.subClass == 1 ? 1.32f : 1.25f);
    if (player.heroClass == CLASS_KNIGHT && player.subClass == 2) {
        float missingFrac = 1.0f - Clamp(player.hp / std::max(1.0f, player.maxHp), 0.0f, 1.0f);
        r *= (1.0f + 0.40f * missingFrac);
    }
    if (player.heroClass == CLASS_MAGE && player.subClass == 1 && (player.shieldActive || player.ultActiveTimer > 0.0f || player.swordOrbitTimer > 0.0f)) r *= 1.25f;
    if (player.hasteTimer > 0.0f) {
        if (player.heroClass == CLASS_RANGER) r *= (player.subClass == 1 ? 1.45f : 1.35f);
        else if (player.heroClass == CLASS_KNIGHT || player.heroClass == CLASS_MAGE || player.heroClass == CLASS_TEMPLAR) r *= 1.30f;
    }
    if (isKing) r *= 1.12f;
    if (comboCount >= 10) r *= 1.20f; // Frenzy Overdrive
    if (styleRank > 0) r *= (1.0f + 0.020f * styleRank);
    if (runeBuffTimer > 0.0f && activeRune == 3) r *= (1.0f + 0.12f + 0.05f * runeRank);
    for (const auto& h : hazards) {
        if (h.kind == 3 && Vector2Distance(player.pos, h.pos) < h.radius) { r *= 1.25f; break; }
    }
    return r;
}

inline float Game::EffectiveMoveSpeed() const {
    float s = player.speed;
    if (player.heroClass == CLASS_ASSASSIN) {
        if (player.ultActiveTimer > 0.0f) s *= (player.subClass == 2 ? 1.45f : 1.35f);
        else if (player.hasteTimer > 0.0f) s *= 1.25f;
    } else if (player.hasteTimer > 0.0f && (player.heroClass == CLASS_KNIGHT || player.heroClass == CLASS_RANGER || player.heroClass == CLASS_TEMPLAR)) {
        s *= 1.22f;
    }
    if (isKing) s *= 1.12f;
    if (comboCount >= 10) s *= 1.18f; // Frenzy Overdrive
    if (runeBuffTimer > 0.0f && activeRune == 3) s *= 1.15f;
    if (familiarKind == 3) s *= (1.0f + 0.05f * familiarRank); // Lôi Ưng Thần Tốc: +5%/cấp
    if (overdriveTimer > 0.0f) s *= 1.14f; // Bùng Nổ Linh Hồn
    if (!inSurface && weatherKind == 3) s *= 0.96f; // Frost Mist
    if (player.frostSlowTimer > 0.0f) s *= 0.72f;
    if (surfaceUnlocked && hunger < 25.0f) s *= 0.90f;
    if (bloodlineTrait == 1) s *= 1.06f; // Shadowstep
    s *= AgeSpeedMult();
    return s;
}

inline void Game::AddStyle(float pts) {
    if (pts <= 0.0f) return;
    styleDecayTimer = 3.2f;
    stylePoints += pts;
    while (stylePoints >= 100.0f && styleRank < 6) {
        stylePoints -= 100.0f;
        styleRank++;
    }
    if (styleRank >= 6) stylePoints = std::min(100.0f, stylePoints);
}

inline const char* Game::StyleRankLabel() const {
    static const char* kLabels[7] = { "D", "C", "B", "A", "S", "SS", "SSS" };
    return kLabels[std::max(0, std::min(6, styleRank))];
}

inline Color Game::StyleRankColor() const {
    static const Color kCols[7] = {
        { 145, 155, 175, 255 }, { 110, 215, 255, 255 }, { 115, 240, 150, 255 },
        { 255, 220, 95, 255 },  { 255, 145, 55, 255 },  { 255, 75, 115, 255 }, { 235, 110, 255, 255 }
    };
    return kCols[std::max(0, std::min(6, styleRank))];
}

inline const char* Game::BloodlineTraitName() const {
    switch (bloodlineTrait) {
        case 0: return L("Dragonheart", "Long Huyết");
        case 1: return L("Shadowstep", "Ảnh Bộ");
        case 2: return L("Arcane Blood", "Ma Huyết");
        default: return L("Sovereign", "Hoàng Tộc");
    }
}

inline const char* Game::BloodlineTraitDesc() const {
    switch (bloodlineTrait) {
        case 0: return L("+8% Max HP, +5% Base DMG", "+8% Máu tối đa, +5% ST cơ bản");
        case 1: return L("+6% Move Speed, +5% Crit", "+6% Tốc chạy, +5% Chí mạng");
        case 2: return L("+10% Skill CDR, +15% Reaction DMG", "+10% Hồi chiêu, +15% ST Phản ứng");
        default: return L("+15% Gold Gain, +2 Armor", "+15% Vàng nhặt, +2 Giáp");
    }
}

inline const char* Game::WeatherName() const {
    switch (weatherKind) {
        case 1: return L("CRIMSON ECLIPSE", "NHẬT THỰC HUYẾT NGUYỆT");
        case 2: return L("ARCANE ION STORM", "BÃO TỪ MA PHÁP");
        case 3: return L("FROST MIST", "SƯƠNG BĂNG CỰC HÀN");
        case 4: return L("EMBER ASHRAIN", "MƯA TRO DUNG NHAM");
        default: return L("CLEAR SKIES", "KHÍ HẬU QUANG ĐÃNG");
    }
}

inline const char* Game::WeatherDesc() const {
    switch (weatherKind) {
        case 1: return L("+8% Crit, +18% Gold, Foes +6% SPD", "+8% Chí mạng, +18% Vàng, Quái +6% Tốc");
        case 2: return L("-12% Skill CD, Sky Lightning Strikes", "-12% Hồi chiêu, Sét Thiên Lôi giáng");
        case 3: return L("Foe Shots -12% SPD, Freeze +30% Dur", "Đạn quái -12% tốc, Băng +30% thời gian");
        case 4: return L("+20% Burn/Reaction DMG, +20% Rage", "+20% ST Đốt/Phản ứng, +20% Tích Nộ");
        default: return L("Balanced Dungeon Atmosphere", "Không khí hầm ngục ổn định");
    }
}

inline Color Game::WeatherColor() const {
    switch (weatherKind) {
        case 1: return { 255, 95, 115, 255 };
        case 2: return { 115, 225, 255, 255 };
        case 3: return { 165, 235, 255, 255 };
        case 4: return { 255, 165, 75, 255 };
        default: return { 195, 210, 235, 255 };
    }
}

inline const char* Game::RuneName() const {
    switch (activeRune) {
        case 1: return L("Thunderclap Rune", "Cổ Ngữ Lôi Đình");
        case 2: return L("Frostfire Rune", "Cổ Ngữ Băng Hỏa");
        case 3: return L("Bloodthirst Rune", "Cổ Ngữ Huyết Vũ");
        case 4: return L("Aegis Rune", "Cổ Ngữ Kim Cang");
        default: return L("None", "Chưa có");
    }
}

inline Color Game::RuneColor() const {
    switch (activeRune) {
        case 1: return { 110, 225, 255, 255 };
        case 2: return { 255, 145, 95, 255 };
        case 3: return { 255, 85, 120, 255 };
        case 4: return { 255, 215, 95, 255 };
        default: return GRAY;
    }
}

inline void Game::UseRunicAnvil() {
    if (!hasRunicAnvil || runicAnvilUsed) return;
    int cost = RunicAnvilCost();
    if (player.gold < cost) {
        ShowBanner(TextFormat(L("Need %d Gold to forge Ancient Rune!", "Cần %d Vàng để khắc Cổ Ngữ!"), cost), 2.0f);
        PlaySound(snd.sndHurt);
        return;
    }
    player.gold -= cost;
    runicAnvilUsed = true;
    if (activeRune == 0 || runeRank >= 3) {
        activeRune = 1 + (rand() % 4);
        runeRank = std::min(3, std::max(1, runeRank + 1));
    } else {
        runeRank = std::min(3, runeRank + 1);
    }
    AddStyle(35.0f);
    PlaySound(snd.sndLevelUp);
    Shake(0.24f, 6.0f);
    Burst(runicAnvilPos, RuneColor(), 32, 290.0f, 0.75f);
    NovaVisual nv; nv.pos = runicAnvilPos; nv.maxRadius = 110.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = RuneColor();
    novas.push_back(nv);
    ShowBanner(TextFormat(L("RUNIC ANVIL FORGED: %s (Lv.%d)!", "ĐÀI RÈN CỔ NGỮ: %s (Cấp %d)!"), RuneName(), runeRank), 3.4f);
}

inline const char* Game::FamiliarName() const {
    switch (familiarKind) {
        case 1: return L("Solar Phoenix", "Linh Điểu Phượng Hoàng");
        case 2: return L("Frost Wyrm", "Băng Long Tinh Linh");
        case 3: return L("Storm Falcon", "Lôi Ưng Thần Tốc");
        case 4: return L("Void Wisp", "Hồn Ma Hư Không");
        default: return L("None", "Chưa có");
    }
}

inline Color Game::FamiliarColor() const {
    switch (familiarKind) {
        case 1: return ORANGE;
        case 2: return SKYBLUE;
        case 3: return { 120, 245, 170, 255 };
        case 4: return { 205, 115, 255, 255 };
        default: return GRAY;
    }
}

inline void Game::GrantOrUpgradeFamiliar() {
    if (familiarKind == 0) {
        familiarKind = 1 + (rand() % 4);
        familiarRank = 1;
        familiarPos = player.pos;
        familiarAtkTimer = 0.6f;
        ShowBanner(TextFormat(L("SPIRIT FAMILIAR AWAKENED: %s (Lv.1)!", "THỨC TỈNH LINH THÚ: %s (Cấp 1)!"), FamiliarName()), 3.4f);
    } else if (familiarRank < 3) {
        familiarRank++;
        ShowBanner(TextFormat(L("FAMILIAR ASCENDED: %s (Lv.%d)!", "LINH THÚ THĂNG CẤP: %s (Cấp %d)!"), FamiliarName(), familiarRank), 3.2f);
    } else {
        player.hp = std::min(player.maxHp, player.hp + 25.0f);
        AddRage(25.0f);
        SpawnPopup(player.pos, L("+25 HP & RAGE!", "+25 MÁU & NỘ!"), FamiliarColor());
    }
    PlaySound(snd.sndLevelUp);
    Burst(familiarPos, FamiliarColor(), 26, 260.0f, 0.6f);
}

inline void Game::UpdateFamiliar(float dt) {
    if (familiarKind <= 0 || inSurface) return;
    familiarAngle += dt * 2.4f;
    Vector2 targetPos = { player.pos.x + cosf(familiarAngle) * 44.0f, player.pos.y - 14.0f + sinf(familiarAngle) * 28.0f };
    familiarPos = Vector2Lerp(familiarPos, targetPos, std::min(1.0f, dt * 9.0f));
    familiarAtkTimer -= dt;
    if (familiarAtkTimer > 0.0f) return;

    float bestD = 380.0f;
    Vector2 bestPos = { 0, 0 };
    bool found = false;
    for (const auto& e : enemies) {
        if (e.hp <= 0) continue;
        float d = Vector2Distance(familiarPos, e.pos);
        if (d < bestD) { bestD = d; bestPos = e.pos; found = true; }
    }
    if (!found) { familiarAtkTimer = 0.2f; return; }

    Vector2 dir = Vector2Normalize(Vector2Subtract(bestPos, familiarPos));
    Projectile p;
    p.pos = familiarPos;
    p.fromPlayer = true;
    p.life = 1.6f;
    p.color = FamiliarColor();
    if (familiarKind == 1) { // Solar Phoenix
        p.kind = 4;
        p.vel = Vector2Scale(dir, 460.0f);
        p.damage = player.baseDmg * (0.42f + 0.12f * familiarRank);
        p.radius = 7.5f;
        p.applyBurn = true;
        familiarAtkTimer = 1.55f;
    } else if (familiarKind == 2) { // Frost Wyrm
        p.kind = 2;
        p.vel = Vector2Scale(dir, 440.0f);
        p.damage = player.baseDmg * (0.38f + 0.11f * familiarRank);
        p.radius = 7.0f;
        p.applySlow = true;
        p.pierce = 1;
        familiarAtkTimer = 1.70f;
    } else if (familiarKind == 3) { // Storm Falcon
        p.kind = 5;
        p.vel = Vector2Scale(dir, 460.0f);
        p.damage = player.baseDmg * (0.36f + 0.10f * familiarRank);
        p.radius = 6.5f;
        familiarAtkTimer = 1.30f;
    } else { // Void Wisp
        p.kind = 5;
        p.vel = Vector2Scale(dir, 440.0f);
        p.damage = player.baseDmg * (0.40f + 0.10f * familiarRank);
        p.radius = 7.0f;
        familiarAtkTimer = 1.65f;
        player.hp = std::min(player.maxHp, player.hp + 1.5f);
        AddRage(2.0f);
    }
    projectiles.push_back(p);
    Burst(familiarPos, p.color, 5, 110.0f, 0.25f);
}

inline void Game::QuickForgeWithEssence() {
    if (bossEssence <= 0) {
        ShowBanner(L("Need 1 Boss Essence ([B] Quick Forge)!", "Cần 1 Tinh Hoa Trùm để Rèn Nóng ([B])!"), 2.0f);
        PlaySound(snd.sndHurt);
        return;
    }
    int bestSlot = -1, minPlus = 99;
    for (int i = 0; i < 4; ++i) {
        if (hasEquip[i] && equipPlus[i] < 5 && equipPlus[i] < minPlus) {
            minPlus = equipPlus[i];
            bestSlot = i;
        }
    }
    if (bestSlot < 0) {
        ShowBanner(L("All equipped gear is already at +5 MAX!", "Toàn bộ trang bị đang mặc đã đạt +5 TỐI ĐA!"), 2.2f);
        return;
    }
    bossEssence--;
    ApplyEquipEnhanceStep(bestSlot);
}

inline void Game::TriggerElementalReaction(Enemy& e, int reactionKind, Vector2 pos) {
    if (e.hp <= 0 || e.reactionCooldown > 0.0f) return;
    e.reactionCooldown = 0.65f;
    elementalReactionsCount++;
    if (bountyActive && bountyKind == 1) bountyProgress++;
    AddStyle(24.0f);
    float reactMult = (bloodlineTrait == 2 ? 1.15f : 1.0f) * (HasDuoTempestFrost() ? 1.15f : 1.0f) * ((!inSurface && weatherKind == 4) ? 1.20f : 1.0f);
    if (reactionKind == 0) { // MELT / TAN CHAY
        float bd = player.baseDmg * 1.45f * reactMult;
        e.hp -= bd;
        e.burnTimer = std::max(0.0f, e.burnTimer - 0.8f);
        NovaVisual nv; nv.pos = pos; nv.maxRadius = 90.0f; nv.timer = 0.30f; nv.maxTimer = 0.30f; nv.color = { 255, 165, 95, 255 };
        novas.push_back(nv);
        Burst(pos, { 255, 175, 100, 255 }, 14, 220.0f, 0.45f);
        SpawnPopup({ pos.x, pos.y - 26.0f }, TextFormat(L("MELT! %d", "TAN CHẢY! %d"), (int)bd), { 255, 170, 75, 255 });
    } else if (reactionKind == 1) { // OVERLOAD / QUA TAI
        float bd = player.baseDmg * 1.35f * reactMult;
        NovaVisual nv; nv.pos = pos; nv.maxRadius = 125.0f; nv.timer = 0.34f; nv.maxTimer = 0.34f; nv.color = { 255, 95, 195, 255 };
        novas.push_back(nv);
        Burst(pos, { 255, 100, 210, 255 }, 18, 280.0f, 0.45f);
        Shake(0.14f, 4.0f);
        for (auto& other : enemies) {
            if (other.hp <= 0) continue;
            float d = Vector2Distance(pos, other.pos);
            if (d < 125.0f) {
                other.hp -= bd;
                Vector2 push = Vector2Normalize(Vector2Subtract(other.pos, pos));
                other.pos = Vector2Add(other.pos, Vector2Scale(push, 24.0f));
                if (&other != &e && other.hp <= 0) OnEnemyKilled(other);
            }
        }
        SpawnPopup({ pos.x, pos.y - 26.0f }, TextFormat(L("OVERLOAD! %d", "QUÁ TẢI! %d"), (int)bd), { 255, 110, 220, 255 });
    } else if (reactionKind == 2) { // VENOM SHOCK / DOC SET
        float bd = player.baseDmg * 1.15f * reactMult;
        e.hp -= bd;
        NovaVisual nv; nv.pos = pos; nv.maxRadius = 135.0f; nv.timer = 0.30f; nv.maxTimer = 0.30f; nv.color = { 120, 245, 130, 255 };
        novas.push_back(nv);
        for (auto& other : enemies) {
            if (&other != &e && other.hp > 0 && Vector2Distance(pos, other.pos) < 140.0f) {
                other.hp -= bd * 0.65f;
                other.poisonTimer = std::max(other.poisonTimer, 2.5f);
                other.poisonDmg = std::max(other.poisonDmg, player.baseDmg * 0.25f);
                if (other.hp <= 0) OnEnemyKilled(other);
            }
        }
        SpawnPopup({ pos.x, pos.y - 26.0f }, TextFormat(L("VENOM SHOCK! %d", "ĐỘC SÉT! %d"), (int)bd), { 120, 255, 135, 255 });
    } else { // SHATTER / PHA BANG
        float bd = player.baseDmg * 1.25f * reactMult;
        e.hp -= bd;
        Burst(pos, { 150, 235, 255, 255 }, 16, 250.0f, 0.45f);
        SpawnPopup({ pos.x, pos.y - 26.0f }, TextFormat(L("SHATTER! %d", "PHÁ BĂNG! %d"), (int)bd), { 155, 240, 255, 255 });
    }
    if ((e.type == BOSS || e.isElite || e.isWarden) && e.staggerBrokenTimer <= 0.0f) {
        e.stagger = std::min(e.maxStagger, e.stagger + 14.0f);
    }
}

inline void Game::TriggerPerfectDodge() {
    perfectDodgeTimer = 0.40f;
    nextHitCrit = true;
    perfectDodgesCount++;
    AddRage(14.0f);
    AddStyle(35.0f);
    player.specialTimer = std::max(0.0f, player.specialTimer - 1.2f);
    player.skill2Timer = std::max(0.0f, player.skill2Timer - 1.2f);
    player.skill3Timer = std::max(0.0f, player.skill3Timer - 1.2f);
    player.skill4Timer = std::max(0.0f, player.skill4Timer - 1.2f);

    // Phản Đạn Ánh Sáng (Bullet Parry Deflect): phản tối đa 2 viên đạn quái gần nhất thành tên ánh sáng truy đuổi
    int deflected = 0;
    for (auto& pr : projectiles) {
        if (deflected >= 2) break;
        if (!pr.fromPlayer && Vector2Distance(player.pos, pr.pos) < 92.0f) {
            Vector2 targetPos = Vector2Subtract(pr.pos, pr.vel);
            float bestD = 1e9f;
            for (const auto& en : enemies) {
                if (en.hp <= 0) continue;
                float ed = Vector2Distance(pr.pos, en.pos);
                if (ed < bestD) { bestD = ed; targetPos = en.pos; }
            }
            Vector2 rDir = Vector2Normalize(Vector2Subtract(targetPos, pr.pos));
            if (Vector2Length(rDir) < 0.01f) rDir = { 1.0f, 0.0f };
            pr.fromPlayer = true;
            pr.kind = 5;
            pr.vel = Vector2Scale(rDir, 460.0f);
            pr.damage = player.baseDmg * 1.65f;
            pr.color = GOLD;
            pr.life = 2.4f;
            pr.radius = 8.5f;
            pr.hasPreCrit = true;
            pr.preCrit = true;
            Burst(pr.pos, GOLD, 8, 170.0f, 0.35f);
            deflected++;
        }
    }
    ghosts.push_back({ player.pos, player.facingLeft, 0.45f, 0.45f });
    NovaVisual nv; nv.pos = player.pos; nv.maxRadius = deflected > 0 ? 96.0f : 78.0f; nv.timer = 0.28f; nv.maxTimer = 0.28f; nv.color = GOLD;
    novas.push_back(nv);
    Burst(player.pos, GOLD, 16, 240.0f, 0.45f);
    SpawnPopup({ player.pos.x, player.pos.y - 34.0f }, deflected > 0 ? L("PARRY DEFLECT!", "PHẢN ĐẠN!") : L("PERFECT DODGE!", "NÉ HOÀN HẢO!"), GOLD);
    PlaySound(snd.sndDash);
}

inline void Game::AcceptBloodPact() {
    if (!hasBloodPact || bloodPactUsed) return;
    bloodPactUsed = true;
    curseStacks++;
    float hpLoss = std::max(12.0f, roundf(player.maxHp * 0.14f));
    player.maxHp = std::max(50.0f, player.maxHp - hpLoss);
    player.hp = std::min(player.hp, player.maxHp);
    player.baseDmg += 10.0f;
    player.critChance = std::min(0.65f, player.critChance + 0.06f);
    bossEssence++;
    bool gotRelic = GiveRandomRelic();
    AddStyle(40.0f);
    PlaySound(snd.sndLevelUp);
    Shake(0.25f, 6.0f);
    Burst(bloodPactPos, { 220, 45, 75, 255 }, 28, 280.0f, 0.7f);
    ShowBanner(gotRelic
        ? L("BLOOD PACT SEALED: +Relic, +10 DMG, +6% Crit, +1 Essence (Enemies +8%)",
            "GIAO ƯỚC MÁU: +Thánh Vật, +10 ST, +6% Chí Mạng, +1 Tinh Hoa (Quái +8%)")
        : L("BLOOD PACT SEALED: +10 DMG, +6% Crit, +1 Essence (Enemies +8%)",
            "GIAO ƯỚC MÁU: +10 ST, +6% Chí Mạng, +1 Tinh Hoa (Quái +8%)"), 3.8f);
}

inline void Game::AddRage(float amt) {
    if (amt <= 0.0f || player.rage >= player.maxRage) return;
    if (!inSurface && weatherKind == 4) amt *= 1.20f; // Mưa Tro Dung Nham: +20% tốc độ tích Nộ
    float prev = player.rage;
    player.rage = std::min(player.maxRage, player.rage + amt);
    if (prev < player.maxRage && player.rage >= player.maxRage) {
        PlaySound(snd.sndShield);
        SpawnPopup({ player.pos.x, player.pos.y - 36.0f }, "RAGE FULL [R]!", GOLD);
        Burst(player.pos, { 255, 190, 50, 255 }, 16, 200.0f, 0.5f);
    }
}

inline void Game::Init() {
    InitGameFont();
    snd.Init();
    camera.zoom = 1.0f;
    camera.offset = { 1280 / 2.0f, 720 / 2.0f };
    InitMotes();
#ifdef __ANDROID__
    touchMode = true;
#endif
    lightmap = LoadRenderTexture(1280, 720);
    SetTextureFilter(lightmap.texture, TEXTURE_FILTER_BILINEAR);
    screenRT = LoadRenderTexture(1280 * kRenderScale, 720 * kRenderScale);
    SetTextureFilter(screenRT.texture, TEXTURE_FILTER_BILINEAR);
    LoadSettings();
    RefreshSaveSlots();
    BuildResList();
    ApplySettings();
    RestartRun();
    state = TITLE;
}

inline void Game::RestartRun() {
    ResetDynasty();
    roomW = 1200;
    roomH = 750;
    player = Player();
    int hc = std::max(0, std::min(kNumClasses - 1, selectedClass));
    const ClassDef& cd = kClasses[hc];
    player.heroClass = hc;
    player.maxHp = cd.baseHp;
    player.speed = cd.baseSpeed;
    player.baseDmg = cd.baseDmg - 3.0f; // vũ khí khởi đầu +3 DMG
    player.attackRate = cd.baseAtkRate;
    player.armor = cd.baseArmor;
    player.critChance = cd.baseCrit;
    player.specialCooldown = Skill1MaxCd();
    player.skill2Cooldown = Skill2MaxCd();
    player.skill3Cooldown = Skill3MaxCd();
    player.skill4Cooldown = Skill4MaxCd();
    player.pos = { 0, 240 };
    // Nâng cấp vĩnh viễn từ Thánh địa
    player.maxHp += 10.0f * meta[0];
    player.hp = player.maxHp;
    player.hpDisplay = player.hp;
    player.baseDmg += 2.0f * meta[1];
    player.speed += 5.0f * meta[2];
    player.gold += 10 * meta[3];
    xpMul = 1.0f + 0.08f * meta[4];
    bossesKilled = 0;
    floor = 1;
    enemiesSlain = 0;
    comboCount = 0;
    bestCombo = 0;
    damagePulse = 0.0f;
    hitStop = 0.0f;
    slowMoTimer = 0.0f;
    perfectDodgeTimer = 0.0f;
    nextHitCrit = false;
    stylePoints = 0.0f;
    styleRank = 0;
    styleDecayTimer = 0.0f;
    perfectDodgesCount = 0;
    elementalReactionsCount = 0;
    curseStacks = 0;
    hasBloodPact = false;
    bloodPactUsed = false;
    bossEssence = 0;
    for (int i = 0; i < 4; ++i) equipPlus[i] = 0;
    bountyActive = false;
    bountyProgress = 0;
    weatherKind = 0;
    weatherTimer = 6.0f;
    lightningFlash = 0.0f;
    bossHpLag = 1.0f;
    hasRunicAnvil = false;
    runicAnvilUsed = false;
    activeRune = 0;
    runeRank = 0;
    runeBuffTimer = 0.0f;
    familiarKind = 0;
    familiarRank = 0;
    familiarPos = player.pos;
    familiarAngle = 0.0f;
    familiarAtkTimer = 1.0f;
    overdriveTimer = 0.0f;
    bloodlineTrait = hc % 4;
    if (bloodlineTrait == 0) { player.maxHp = roundf(player.maxHp * 1.08f); player.hp = player.maxHp; player.baseDmg = roundf(player.baseDmg * 1.05f); }
    else if (bloodlineTrait == 1) { player.critChance += 0.05f; }
    else if (bloodlineTrait == 3) { player.armor += 2.0f; }
    particles.clear();
    groundEffects.clear();
    for (int i = 0; i < 4; ++i) hasEquip[i] = false;
    int dummySold = 0;
    TryEquipItem(MakeStarterWeapon(cd.startWeapon), dummySold);
    itemDrops.clear();
    pendingSpawns.clear();
    barrels.clear();
    hazards.clear();
    portalChoices.clear();
    wave2Spawns.clear();
    wave2Triggered = false;
    forcedNextMod = -1;
    forcedNextReward = -1;
    hasTrialTotem = false;
    trialUsed = false;
    trialActive = false;
    subclassChoosing = false;
    phoenixUsed = false;
    newRecord = false;
    state = PLAYING;
    BuildFloor();
    if (meta[7] > 0) GiveRandomRelic(); // Second Wind
}

inline Game::BiomeColors Game::SurfaceBiome() const {
    switch (surfaceRoom) {
        case 1: // Quán Trọ & Chợ Ẩm Thực (Gỗ sồi ấm & Lò sưởi vàng cam)
            return { { 28, 18, 14, 255 }, { 74, 52, 36, 255 }, { 60, 42, 28, 255 }, { 108, 68, 42, 255 },
                     { 245, 185, 92, 255 }, { 255, 130, 55, 255 }, { 126, 76, 52, 255 }, { 255, 205, 125, 255 },
                     "GOLDENHEARTH TAVERN", "QUÁN TRỌ LỬA ẤM" };
        case 2: // Phường Gia Trang (Đá cẩm thạch xám xanh & Bản thiết kế hoàng gia)
            return { { 18, 22, 32, 255 }, { 56, 66, 84, 255 }, { 46, 54, 70, 255 }, { 86, 102, 126, 255 },
                     { 130, 215, 255, 255 }, { 245, 200, 95, 255 }, { 96, 114, 142, 255 }, { 180, 232, 255, 255 },
                     "MASONS & ESTATE GUILD", "PHƯỜNG KIẾN TRÚC GIA TRANG" };
        case 3: // Tổ Ấm Gia Đình & Từ Đường (Nâng cấp lộng lẫy theo houseTier)
            if (houseTier <= 0) {
                return { { 20, 22, 28, 255 }, { 58, 62, 72, 255 }, { 48, 52, 60, 255 }, { 82, 74, 62, 255 },
                         { 195, 205, 225, 255 }, { 255, 190, 120, 255 }, { 88, 92, 106, 255 }, { 220, 230, 255, 255 },
                         "FAMILY HEARTH PLOT", "ĐẤT TỔ ẤM GIA ĐÌNH" };
            } else if (houseTier == 1) {
                return { { 24, 20, 16, 255 }, { 78, 58, 40, 255 }, { 64, 48, 34, 255 }, { 112, 82, 54, 255 },
                         { 140, 225, 155, 255 }, { 255, 205, 105, 255 }, { 118, 86, 58, 255 }, { 255, 235, 175, 255 },
                         "VILLAGE COTTAGE HOME", "NHÀ GỖ TỔ ẤM" };
            } else if (houseTier == 2) {
                return { { 20, 20, 34, 255 }, { 60, 54, 82, 255 }, { 48, 44, 68, 255 }, { 92, 74, 116, 255 },
                         { 130, 220, 255, 255 }, { 255, 155, 195, 255 }, { 102, 92, 134, 255 }, { 225, 210, 255, 255 },
                         "CITY MANOR & ANCESTRAL HALL", "BIỆT THỰ & TỪ ĐƯỜNG" };
            }
            return { { 28, 16, 24, 255 }, { 78, 46, 56, 255 }, { 62, 36, 46, 255 }, { 122, 48, 66, 255 },
                     { 255, 215, 92, 255 }, { 255, 140, 190, 255 }, { 136, 98, 72, 255 }, { 255, 228, 145, 255 },
                     "NOBLE DYNASTY PALACE HOME", "DINH THỰ QUÝ TỘC & TỪ ĐƯỜNG" };
        case 4: // Đại Điện Hoàng Cung (Đá đỏ hoàng gia & Vàng ròng)
            return { { 28, 14, 18, 255 }, { 74, 40, 48, 255 }, { 60, 32, 38, 255 }, { 132, 34, 48, 255 },
                     { 255, 218, 82, 255 }, { 255, 110, 120, 255 }, { 142, 108, 74, 255 }, { 255, 232, 135, 255 },
                     "IMPERIAL THRONE HALL", "ĐẠI ĐIỆN HOÀNG CUNG" };
        case 5: // Thành Quách Lãnh Địa Riêng (Đá hoa cương xanh thép & Vàng ròng)
            return { { 18, 22, 32, 255 }, { 54, 62, 78, 255 }, { 42, 48, 62, 255 }, { 98, 42, 56, 255 },
                     { 255, 215, 85, 255 }, { 115, 235, 255, 255 }, { 92, 104, 128, 255 }, { 215, 240, 255, 255 },
                     "PERSONAL FIEFDOM STRONGHOLD", "THÀNH QUÁCH LÃNH ĐỊA RIÊNG" };
        default: // Quảng Trường Làng Mạc & Kinh Thành (Đá vôi hoàng kim rực nắng)
            return { { 22, 28, 38, 255 }, { 62, 74, 64, 255 }, { 52, 64, 54, 255 }, { 126, 116, 98, 255 },
                     { 245, 205, 100, 255 }, { 110, 225, 255, 255 }, { 124, 112, 92, 255 }, { 255, 242, 180, 255 },
                     "ROYAL CITY SQUARE", "QUẢNG TRƯỜNG KINH THÀNH" };
    }
}

inline Game::BiomeColors Game::Biome() const {
    if (inSurface) return SurfaceBiome();
    switch (BiomeIdx()) {
        case 1: // Emerald Overgrown Ruins
            return { { 18, 30, 22, 255 }, { 38, 66, 48, 255 }, { 28, 52, 38, 255 }, { 82, 76, 56, 255 },
                     { 115, 225, 130, 255 }, { 255, 135, 195, 255 }, { 76, 118, 82, 255 }, { 170, 245, 155, 255 },
                     "EMERALD GROVE RUINS", "TÀN TÍCH RỪNG NGỌC" };
        case 2: // Obsidian Molten Forge
            return { { 30, 16, 16, 255 }, { 48, 34, 36, 255 }, { 36, 24, 28, 255 }, { 84, 56, 40, 255 },
                     { 255, 120, 40, 255 }, { 255, 205, 65, 255 }, { 128, 68, 48, 255 }, { 255, 165, 75, 255 },
                     "MOLTEN FORGE DEPTHS", "LÒ RÈN DUNG NHAM" };
        case 3: // Crystal Frost Sanctum
            return { { 14, 24, 38, 255 }, { 40, 66, 92, 255 }, { 30, 52, 76, 255 }, { 74, 106, 138, 255 },
                     { 125, 235, 255, 255 }, { 195, 155, 255, 255 }, { 112, 162, 198, 255 }, { 225, 248, 255, 255 },
                     "CRYSTAL FROST SANCTUM", "THÁNH ĐIỆN BĂNG TINH" };
        case 4: // Golden Sun Temple
            return { { 32, 24, 16, 255 }, { 82, 62, 42, 255 }, { 68, 50, 34, 255 }, { 30, 88, 102, 255 },
                     { 255, 210, 80, 255 }, { 85, 225, 205, 255 }, { 148, 108, 66, 255 }, { 255, 220, 125, 255 },
                     "GOLDEN SUN TEMPLE", "ĐỀN MẶT TRỜI HOÀNG KIM" };
        case 5: // Astral Void Nexus
            return { { 18, 14, 34, 255 }, { 40, 28, 66, 255 }, { 30, 20, 52, 255 }, { 48, 64, 108, 255 },
                     { 220, 95, 255, 255 }, { 95, 245, 255, 255 }, { 92, 66, 142, 255 }, { 215, 160, 255, 255 },
                     "ASTRAL VOID NEXUS", "HƯ KHÔNG TINH TÚ" };
        default: // Ancient Royal Crypt
            return { { 18, 22, 34, 255 }, { 36, 44, 62, 255 }, { 28, 35, 50, 255 }, { 88, 30, 46, 255 },
                     { 225, 180, 72, 255 }, { 95, 225, 255, 255 }, { 66, 78, 102, 255 }, { 195, 212, 245, 255 },
                     "ROYAL ANCIENT CRYPT", "LĂNG MỘ HOÀNG GIA" };
    }
}

inline void Game::Burst(Vector2 pos, Color c, int count, float speed, float life) {
    if (particles.size() > 700) return;
    if (particles.size() > 350) count = std::max(1, count / 3);
    if (settings.particles == 0) count = std::max(1, count / 3);
    else if (settings.particles == 1) count = std::max(1, count * 2 / 3);
    for (int i = 0; i < count; ++i) {
        float a = Rand01() * 2.0f * PI;
        float sp = speed * (0.4f + 0.6f * Rand01());
        float shade = 0.8f + 0.35f * Rand01();
        Particle p;
        p.pos = pos;
        p.vel = { cosf(a) * sp, sinf(a) * sp };
        p.color = { (unsigned char)std::min(255.0f, c.r * shade), (unsigned char)std::min(255.0f, c.g * shade),
                    (unsigned char)std::min(255.0f, c.b * shade), 255 };
        p.maxLife = life * (0.7f + 0.4f * Rand01());
        p.life = p.maxLife;
        p.size = 2.0f + 3.0f * Rand01();
        particles.push_back(p);
    }
}

inline void Game::InitMotes() {
    motes.clear();
    for (int i = 0; i < 40; ++i) {
        Mote m;
        m.pos = { (Rand01() - 0.5f) * roomW, (Rand01() - 0.5f) * roomH };
        m.speed = 8.0f + 22.0f * Rand01();
        m.phase = Rand01() * 10.0f;
        m.size = 1.2f + 2.0f * Rand01();
        motes.push_back(m);
    }
}

inline void Game::UpdateEffects(float dt) {
    for (size_t i = 0; i < particles.size();) {
        Particle& p = particles[i];
        p.life -= dt;
        if (p.life <= 0) { particles[i] = particles.back(); particles.pop_back(); continue; }
        p.pos = Vector2Add(p.pos, Vector2Scale(p.vel, dt));
        p.vel = Vector2Scale(p.vel, std::max(0.0f, 1.0f - 4.5f * dt));
        ++i;
    }
    for (auto& m : motes) {
        m.pos.y -= m.speed * dt;
        m.pos.x += sinf((float)GetTime() * 0.7f + m.phase) * 6.0f * dt;
        if (m.pos.y < -roomH / 2.0f) { m.pos.y = roomH / 2.0f; m.pos.x = (Rand01() - 0.5f) * roomW; }
    }
    if (damagePulse > 0) damagePulse = std::max(0.0f, damagePulse - dt * 2.2f);
}

inline Color Game::DeathColor(const Enemy& e) {
    if (e.isElite) return { 255, 217, 102, 255 };
    if (e.isWarden) return { 255, 199, 89, 255 };
    if (e.type == BOSS) return { 242, 77, 77, 255 };
    if (e.type == BOMBER) return { 255, 153, 38, 255 };
    if (e.type == CHARGER) return { 235, 140, 70, 255 };
    if (e.type == SPLITTER) return { 90, 220, 230, 255 };
    if (e.type == SUMMONER) return { 170, 110, 255, 255 };
    if (e.type == BRUTE) return { 153, 102, 242, 255 };
    if (e.type == RANGER) return { 242, 77, 89, 255 };
    return { 102, 230, 89, 255 };
}

inline std::string Game::UpgradeTitle(UpgradeType u) {
    if (settings.language == 1) {
        switch (u) {
            case UP_HP: return "Trái Tim Khổng Lồ";
            case UP_DMG: return "Lưỡi Đao Sắc Bén";
            case UP_SPEED: return "Thần Phong Bộ";
            case UP_ATTACK_RATE: return "Cuồng Phong Trảm";
            case UP_DASH: return "Tức Thời Bộ";
            case UP_LIFESTEAL: return "Hút Máu Huyết Ma";
            case UP_ARMOR: return "Giáp Thiết Thạch";
            case UP_CRIT: return "Ưng Nhãn Chí Mạng";
            case UP_NOVA: return "Tập Trung Ma Lực";
            case UP_IGNITE: return "Lưỡi Kiếm Lửa";
            case UP_FROST: return "Chạm Băng Giá";
        }
    }
    switch (u) {
        case UP_HP: return "Titan's Heart";
        case UP_DMG: return "Sharpened Edge";
        case UP_SPEED: return "Swift Wind";
        case UP_ATTACK_RATE: return "Flurry";
        case UP_DASH: return "Blink Step";
        case UP_LIFESTEAL: return "Vampiric Touch";
        case UP_ARMOR: return "Iron Aegis";
        case UP_CRIT: return "Eagle Eye";
        case UP_NOVA: return "Arcane Focus";
        case UP_IGNITE: return "Ember Edge";
        case UP_FROST: return "Frost Touch";
    }
    return "";
}

inline std::string Game::UpgradeDesc(UpgradeType u) {
    if (settings.language == 1) {
        switch (u) {
            case UP_HP: return "+25 Máu Tối Đa\n& Hồi Máu Ngay";
            case UP_DMG: return "+8 Sát Thương\nCho mọi đòn đánh";
            case UP_SPEED: return "+15% Tốc Độ Chạy\nNé đòn linh hoạt";
            case UP_ATTACK_RATE: return "+30% Tốc Độ Đánh\nRa đòn nhanh hơn";
            case UP_DASH: return "-0.2s Hồi Chiêu Lướt\nCơ động vượt trội";
            case UP_LIFESTEAL: return "+" + FmtPct(LifestealStep()) + " Hút Máu\nHồi máu khi trúng đòn";
            case UP_ARMOR: return "+2 Giáp Cố Định\nGiảm sát thương nhận";
            case UP_CRIT: return "+8% Tỉ Lệ Chí Mạng\nĐòn chí mạng x2 ST";
            case UP_NOVA: return "-12% Hồi Chiêu Q & E\nTung kỹ năng liên tục";
            case UP_IGNITE: return "Đòn đánh Thiêu Đốt 3s\n+50% ST/s mỗi cấp";
            case UP_FROST: return "Đòn đánh Làm Chậm 2s\n-15% tốc quái mỗi cấp";
        }
    }
    switch (u) {
        case UP_HP: return "+25 Max Health\n& Instant Recovery";
        case UP_DMG: return "+8 Melee Damage\nFor all attacks";
        case UP_SPEED: return "+15% Move Speed\nOutrun dangers";
        case UP_ATTACK_RATE: return "+30% Attack Speed\nRapid strikes";
        case UP_DASH: return "-0.2s Dash Cooldown\nGreater mobility";
        case UP_LIFESTEAL: return "+" + FmtPct(LifestealStep()) + " Lifesteal\nHeal on hit";
        case UP_ARMOR: return "+2 Flat Armor\nReduce damage";
        case UP_CRIT: return "+8% Crit Chance\nCrits deal x2 dmg";
        case UP_NOVA: return "-12% Skill Cooldowns\nCast Q & E faster";
        case UP_IGNITE: return "Hits burn enemies 3s\n+50% dmg/s per level";
        case UP_FROST: return "Hits slow enemies 2s\n-15% speed per level";
    }
    return "";
}

inline void Game::Shake(float dur, float mag) {
    if (!settings.vibration) return;
    screenShakeTimer = dur;
    screenShakeMag = mag;
    float k = std::min(1.0f, mag / 9.0f); // mọi cú rung màn hình cũng làm rung tay cầm
    Rumble(k * 0.9f, k * 0.55f, std::min(0.45f, dur + 0.08f));
}

inline Vector2 Game::SurfaceTownExitSpot(int roomIdx) {
    if (roomIdx == 5) return { 0.0f, 258.0f };
    if (roomIdx < 1 || roomIdx > 4) return { 0.0f, 180.0f };
    Vector2 bp = SurfaceLandmarkPos(roomIdx - 1);
    return { bp.x, bp.y + 84.0f };
}

inline Vector2 Game::HousingPedestalPos(int tierIdx) {
    if (tierIdx <= 1) return { -300.0f,  -55.0f };
    if (tierIdx == 2) return {    0.0f, -145.0f };
    return { 300.0f, -55.0f };
}

inline float Game::ChildAttackInterval(const ChildInfo& ch) const {
    static const float base[kNumClasses] = { 0.85f, 0.60f, 1.10f, 0.90f, 0.78f };
    int hc = std::max(0, std::min(kNumClasses - 1, ch.heroClass));
    return base[hc] * (1.30f - 0.30f * ChildGrowth(ch));
}

inline int Game::ActivePartyCount() const {
    int n = 0;
    for (const auto& ch : children) if (ch.active) n++;
    return n;
}

inline void Game::BuildPortalChoices() {
    portalChoices.clear();
    int nextF = floor + 1;
    if (nextF % 5 == 0 || IsShopFloor(nextF)) {
        PortalOption po;
        po.pos = portalPos;
        po.nextMod = -1;
        po.rewardKind = 0;
        if (nextF % 5 == 0) {
            po.title = L("BOSS GATE", "CỔNG TRÙM");
            po.subtitle = L("Defeat Boss for Relic", "Diệt Trùm nhận Thánh Tích");
            po.color = { 255, 85, 85, 255 };
        } else {
            po.title = L("MERCHANT SHOP", "TIỆM THƯƠNG NHÂN");
            po.subtitle = L("Spend Gold & Heal", "Mua đồ & Hồi máu");
            po.color = GOLD;
        }
        portalChoices.push_back(po);
    } else {
        struct Cand { int mod; int reward; const char* title; const char* sub; Color col; };
        Cand pool[4] = {
            { MOD_ELITE,  0, L("ELITE ARENA", "ĐẤU TRƯỜNG TINH ANH"),    L("High Gear Drops", "Tỉ lệ rơi đồ hiếm cao"), { 255, 165, 50, 255 } },
            { MOD_SWARM,  1, L("SWARM GAUNTLET", "BẦY QUÁI ĐÔNG"),       L("+Bonus Gold", "+Thêm Vàng thưởng"),         { 80, 235, 155, 255 } },
            { MOD_SNIPER, 3, L("SNIPER NEST", "Ổ XẠ THỦ"),               L("+40 Starting Rage", "+40 Nộ khởi đầu"),     { 255, 85, 105, 255 } },
            { MOD_NONE,   2, L("SANCTIFIED HALL", "SẢNH THANH TẨY"),     L("+35% HP Recovery", "Hồi +35% Máu tối đa"),  SKYBLUE }
        };
        std::vector<Cand> cands(pool, pool + 4);
        for (int i = 3; i > 0; --i) std::swap(cands[i], cands[rand() % (i + 1)]);
        if (Rand01() < 0.22f) {
            cands[2] = { MOD_TREASURE, 0, L("TREASURE VAULT", "KHO BÁU CỔ"), L("Free Vault Chest", "Rương kho báu miễn phí"), { 195, 110, 255, 255 } };
        }
        Vector2 positions[3] = {
            portalPos,
            { -240.0f, -roomH / 2.0f + 80.0f },
            {  240.0f, -roomH / 2.0f + 80.0f }
        };
        for (int i = 0; i < 3; ++i) {
            PortalOption po;
            po.pos = positions[i];
            po.nextMod = cands[i].mod;
            po.rewardKind = cands[i].reward;
            po.title = cands[i].title;
            po.subtitle = cands[i].sub;
            po.color = cands[i].col;
            portalChoices.push_back(po);
        }
    }
    // Sau khi vượt qua Tầng 10 (hoặc đã mở khóa Mặt Đất): Cổng Mặt Đất xuất hiện ở góc dưới trái (không đè lên portalPos cho bot)
    if (floor >= 10 || surfaceUnlocked) {
        PortalOption surf;
        surf.pos = { -190.0f, roomH / 2.0f - 90.0f };
        surf.nextMod = -99;
        surf.rewardKind = 0;
        surf.title = L("SURFACE GATE [F / G]", "CỔNG LÊN MẶT ĐẤT [F / G]");
        surf.subtitle = L("Village, City, Marriage & King", "Làng Mạc, Nhà Cửa, Vợ Con & Vua");
        surf.color = { 105, 245, 145, 255 };
        portalChoices.push_back(surf);
    }
}

inline void Game::BuildFloor() {
    enemies.clear();
    projectiles.clear();
    loots.clear();
    slashes.clear();
    novas.clear();
    ghosts.clear();
    groundEffects.clear();
    pillars.clear();
    particles.clear();
    itemDrops.clear();
    pendingSpawns.clear();
    alliedUnits.clear();
    barrels.clear();
    hazards.clear();
    portalChoices.clear();
    wave2Spawns.clear();
    wave2Triggered = false;
    hasTrialTotem = false;
    trialUsed = false;
    trialActive = false;
    player.ultActiveTimer = 0.0f;
    BuildDecor();

    portalOpen = false;
    portalPos = { 0, -roomH / 2.0f + 75.0f };
    player.pos = { 0, roomH / 2.0f - 90.0f };

    if (inRebellion) {
        roomW = 1360.0f;
        roomH = 920.0f;
        floorMod = MOD_NONE;
        shopFloor = false;
        hasShrine = hasCaptiveEvent = hasBloodPact = hasRunicAnvil = false;
        chests.clear();
        BuildDecor();
        portalPos = { 0.0f, -roomH / 2.0f + 75.0f };
        player.pos = { 0.0f, roomH / 2.0f - 105.0f };
        SpawnPartyChildren();
        layout = LAY_BLOCKS;
        pillars = {
            { -410.0f, -245.0f }, {  410.0f, -245.0f },
            { -410.0f,   45.0f }, {  410.0f,   45.0f }
        };
        torches = {
            { -150.0f, -280.0f }, {  150.0f, -280.0f },
            { -150.0f,  190.0f }, {  150.0f,  190.0f }
        };
        Enemy king = BuildEnemy(BOSS, { 0.0f, -155.0f });
        king.isKingBoss = true;
        king.kingClass = rand() % kNumClasses;
        float pwr = ComputePlayerPowerScore();
        king.kingPowerScore = (int)roundf(pwr);
        king.maxHp = roundf((1800.0f + player.maxHp * 16.0f + player.baseDmg * 58.0f + pwr * 2.8f) * DiffHp());
        king.hp = king.maxHp;
        king.contactDmg = roundf((18.0f + player.maxHp * 0.11f + player.baseDmg * 0.85f + pwr * 0.025f) * DiffDmg());
        king.speed = Clamp(140.0f + player.speed * 0.38f + pwr * 0.015f, 195.0f, 330.0f);
        king.radius = 30.0f;
        king.maxStagger = 140.0f + player.level * 1.5f;
        enemies.push_back(king);
        for (int gi = 0; gi < 3; ++gi) {
            Enemy g = BuildEnemy(gi == 0 ? BRUTE : CHASER, { (gi - 1) * 190.0f, -60.0f });
            g.isElite = true;
            g.eliteAffix = 1 + (gi % 4);
            g.maxHp *= 1.35f;
            g.hp = g.maxHp;
            enemies.push_back(g);
        }
        SpawnAlliedGarrison(fiefEstablished ? 3 : 1);
        weatherKind = 0;
        lightningFlash = 0.0f;
        bossHpLag = 1.0f;
        ShowBanner(TextFormat(L("THRONE REBELLION! HIS MAJESTY [%s - PWR %d] DESCENDS!",
                                "NỔI DẬY CHIẾM NGAI VÀNG! BẠO CHÚA [%s - UY LỰC %d] XUẤT TRẬN!"),
                              ClassDisplayName(king.kingClass), king.kingPowerScore), 4.0f);
        return;
    }
    if (inWarBattle) {
        roomW = 1360.0f;
        roomH = 920.0f;
        floorMod = MOD_NONE;
        shopFloor = false;
        hasShrine = hasCaptiveEvent = hasBloodPact = hasRunicAnvil = false;
        chests.clear();
        BuildDecor();
        portalPos = { 0.0f, -roomH / 2.0f + 75.0f };
        player.pos = { 0.0f, roomH / 2.0f - 105.0f };
        SpawnPartyChildren();
        layout = LAY_RING;
        BuildPillars();
        int ni = std::max(1, std::min(4, warBattleNation));
        Enemy gen = BuildEnemy(BOSS, { 0.0f, -155.0f });
        gen.isNationalGeneral = true;
        gen.generalNation = ni;
        float pwr = ComputePlayerPowerScore();
        gen.maxHp = roundf((950.0f + player.maxHp * 9.0f + pwr * 1.35f) * DiffHp());
        gen.hp = gen.maxHp;
        gen.contactDmg = roundf((15.0f + player.baseDmg * 0.65f + pwr * 0.015f) * DiffDmg());
        gen.speed = Clamp(145.0f + player.speed * 0.30f, 180.0f, 290.0f);
        gen.radius = 28.0f;
        enemies.push_back(gen);
        for (int gi = 0; gi < 6; ++gi) {
            enemies.push_back(BuildEnemy((gi % 2 == 0) ? RANGER : CHASER, { (gi - 2.5f) * 145.0f, -70.0f + (gi % 2) * 45.0f }));
        }
        SpawnAlliedGarrison(fiefEstablished ? 3 : 2);
        weatherKind = 0;
        lightningFlash = 0.0f;
        bossHpLag = 1.0f;
        ShowBanner(TextFormat(L("WAR BATTLEFIELD: %s!", "CHIẾN TRƯỜNG QUỐC GIA: %s!"),
                              L(nations[ni].nameEn, nations[ni].nameVi)), 3.5f);
        return;
    }
    SpawnPartyChildren();

    bool isBoss = (floor % 5 == 0);
    shopFloor = IsShopFloor(floor);

    // Biến thể tầng (từ tầng 2 hoặc theo Cổng đã chọn ở tầng trước)
    floorMod = MOD_NONE;
    if (!isBoss && !shopFloor && forcedNextMod >= 0) {
        floorMod = forcedNextMod;
    } else if (!isBoss && !shopFloor && floor >= 2) {
        float r = Rand01();
        if (r < 0.12f) floorMod = MOD_TREASURE;
        else if (r < 0.32f) floorMod = MOD_SWARM;
        else if (floor >= 3 && r < 0.50f) floorMod = MOD_ELITE;
        else if (floor >= 3 && r < 0.68f) floorMod = MOD_SNIPER;
    }
    forcedNextMod = -1;

    if (forcedNextReward == 1) {
        int bonusG = 30 + floor * 3;
        player.gold += bonusG;
        SpawnPopup(player.pos, "+" + std::to_string(bonusG) + "g PORTAL", GOLD);
    } else if (forcedNextReward == 2) {
        float hl = roundf(player.maxHp * 0.35f);
        player.hp = std::min(player.maxHp, player.hp + hl);
        SpawnPopup(player.pos, "+" + std::to_string((int)hl) + " HP", GREEN);
    } else if (forcedNextReward == 3) {
        AddRage(40.0f);
        SpawnPopup(player.pos, "+40 RAGE!", ORANGE);
    }
    forcedNextReward = -1;
    eliteChance = (floorMod == MOD_ELITE) ? std::min(0.70f, EliteChance() + 0.35f) : EliteChance();

    // Bố cục phòng: không lặp lại bố cục tầng trước
    std::vector<int> pool = { LAY_SCATTERED, LAY_RING, LAY_BLOCKS, LAY_OPEN };
    if (!isBoss && !shopFloor) pool.push_back(LAY_BASTION);
    if (lastLayout >= 0 && pool.size() > 1) pool.erase(std::remove(pool.begin(), pool.end(), lastLayout), pool.end());
    layout = (floor == 1) ? LAY_SCATTERED : pool[rand() % pool.size()];
    if (floorMod == MOD_TREASURE) layout = LAY_OPEN;
    lastLayout = layout;
    BuildPillars();

    std::string theme = "Ancient Crypt";
    if (((floor - 1) / 3) % 4 == 1) theme = "Overgrown Ruins";
    else if (((floor - 1) / 3) % 4 == 2) theme = "Molten Depths";
    else if (((floor - 1) / 3) % 4 == 3) theme = "Frozen Halls";

    ShowBanner("FLOOR " + std::to_string(floor) + " - " + theme + (shopFloor ? " - MERCHANT" : ModLabel()));

    merchantPos = { 0, -150 };
    if (isBoss) {
        SpawnEnemy(BOSS, { 0, -120 });
        for (int i = 0; i < 4; ++i) SpawnEnemy(CHASER, GetRandomSpawnPos());
    } else if (shopFloor) {
        StockShop();
        portalOpen = true; // tầng cửa hàng: không có quái, cổng mở sẵn
    } else if (floorMod == MOD_TREASURE) {
        portalOpen = true; // kho báu: không có quái
    } else {
        int chasers = std::min(10, 3 + floor);
        int spitters = std::min(8, std::max(0, floor - 1));
        int brutes = std::min(5, std::max(0, floor - 2));
        int chargers = (floor >= 2) ? std::min(3, 1 + (floor - 2) / 3) : 0;
        int splitters = (floor >= 2) ? std::min(3, 1 + floor / 5) : 0;
        int summoners = (floor >= 6) ? std::min(2, 1 + (floor - 6) / 5) : 0;
        int bombers = (floor >= 5) ? std::min(4, 1 + (floor - 5) / 2) : 0;
        if (floor == 4 || floor == 8) chasers = std::max(2, chasers - 3); // tầng có Warden: bớt quái nhỏ
        switch (floorMod) {
            case MOD_SWARM:
                chasers = std::min(16, (int)roundf((3 + floor) * 1.8f));
                spitters = std::min(8, std::max(0, floor - 1));
                brutes = 0; bombers = 0; chargers = std::min(chargers, 1); summoners = 0;
                break;
            case MOD_ELITE:
                chasers = 2 + floor / 2; spitters = floor / 2; brutes = std::min(8, 2 + floor / 2);
                break;
            case MOD_SNIPER:
                chasers = 2 + floor / 2; spitters = std::min(12, (int)roundf((floor + 2) * 1.5f)); brutes = 0;
                break;
            default: break;
        }

        // Trần tổng số quái để màn chơi còn đọc được: bớt quái nhỏ trước
        const int maxTotal = 28;
        int total = chasers + spitters + brutes + chargers + bombers + splitters + summoners + ((floor == 4 || floor == 8) ? 1 : 0);
        while (total > maxTotal && (chasers > 2 || spitters > 2)) {
            if (chasers > 2) chasers--; else spitters--;
            total--;
        }

        for (int i = 0; i < chasers; ++i) SpawnEnemy(CHASER, GetRandomSpawnPos());
        for (int i = 0; i < spitters; ++i) SpawnEnemy(RANGER, GetRandomSpawnPos());
        for (int i = 0; i < brutes; ++i) SpawnEnemy(BRUTE, GetRandomSpawnPos());
        for (int i = 0; i < chargers; ++i) SpawnEnemy(CHARGER, GetRandomSpawnPos());
        for (int i = 0; i < splitters; ++i) SpawnEnemy(SPLITTER, GetRandomSpawnPos());
        for (int i = 0; i < summoners; ++i) SpawnEnemy(SUMMONER, GetRandomSpawnPos());
        for (int i = 0; i < bombers; ++i) SpawnEnemy(BOMBER, GetRandomSpawnPos());

        // Warden Brute: mini-boss ở tầng 4 và 8 (tầng 3 là cửa hàng)
        if (floor == 4 || floor == 8) {
            SpawnWarden(GetRandomSpawnPos());
        }

        // Chia đợt quái thứ 2 trên các tầng đông để tạo nhịp dồn dập
        if (enemies.size() >= 7) {
            size_t w2Count = enemies.size() / 3;
            for (size_t k = 0; k < w2Count && !enemies.empty(); ++k) {
                if (enemies.back().isWarden) break;
                wave2Spawns.push_back(enemies.back());
                enemies.pop_back();
            }
        }
    }

    // Thùng nổ / Tinh thể băng / Trụ sấm chiến thuật & Bẫy địa hình theo biome
    if (!shopFloor && floorMod != MOD_TREASURE) {
        int barrelCount = 3 + (rand() % 3); // 3..5
        for (int i = 0; i < barrelCount; ++i) {
            ArenaBarrel b;
            b.kind = rand() % 3;
            b.pos = GetRandomSpawnPos();
            b.hp = 15.0f;
            b.maxHp = 15.0f;
            barrels.push_back(b);
        }
        int bio = ((floor - 1) / 3) % 4;
        int hazCount = 3;
        for (int i = 0; i < hazCount; ++i) {
            FloorHazard h;
            h.kind = (i == 0 && floor >= 2) ? 3 : bio;
            h.radius = 48.0f;
            h.timer = Rand01() * 3.0f;
            for (int tries = 0; tries < 30; ++tries) {
                Vector2 hp = GetRandomSpawnPos();
                if (hp.y < roomH / 2.0f - 160.0f) { h.pos = hp; break; }
                h.pos = { (Rand01() - 0.5f) * (roomW - 260.0f), (Rand01() - 0.5f) * (roomH - 320.0f) - 40.0f };
            }
            hazards.push_back(h);
        }
    }

    // Cột Thử Thách tùy chọn (từ tầng 2, không phải boss/cửa hàng/kho báu)
    hasTrialTotem = (!isBoss && !shopFloor && floorMod != MOD_TREASURE && floor >= 2 && Rand01() < 0.55f);
    if (hasTrialTotem) {
        trialPos = { (Rand01() < 0.5f ? -260.0f : 260.0f), 20.0f };
        ResolvePillars(trialPos, 35.0f);
    }

    // Rương: mỗi tầng 1 cái (2 cái ở tầng chia hết cho 3 hoặc kho báu), kho báu thêm rương trung tâm, tầng thường 2+ có Rương Nguyền Rủa
    chests.clear();
    {
        float cx = roomW / 2.0f - 80.0f, cy = roomH / 2.0f - 80.0f;
        int chestCount = (floor % 3 == 0 || floorMod == MOD_TREASURE) ? 2 : 1;
        Chest c1; c1.pos = { -cx, -cy };
        chests.push_back(c1);
        if (chestCount >= 2) { Chest c2; c2.pos = { cx, -cy }; chests.push_back(c2); }
        if (floorMod == MOD_TREASURE) { Chest v; v.pos = { 0, 40 }; v.vault = true; chests.push_back(v); }
        if (!isBoss && !shopFloor && floorMod != MOD_TREASURE && floor >= 2 && (floor % 2 == 1 || Rand01() < 0.45f)) {
            Chest cc; cc.pos = { 0.0f, -85.0f }; cc.cursed = true;
            ResolvePillars(cc.pos, 32.0f);
            chests.push_back(cc);
        }
    }

    // Bàn thờ máu: từ tầng 2, không phải tầng boss
    hasShrine = (floor >= 2 && floor % 5 != 0 && !shopFloor && floorMod != MOD_TREASURE && Rand01() < 0.3f);
    shrineUsed = false;
    shrineHinted = false;
    shrineRefuse = 0.0f;
    if (hasShrine) {
        for (int tries = 0; tries < 40; ++tries) {
            shrinePos = GetRandomSpawnPos();
            bool nearPillar = false;
            for (const auto& pl : pillars) if (Vector2Distance(shrinePos, pl) < 80.0f) nearPillar = true;
            if (!nearPillar) break;
        }
    }

    // Sự kiện ngẫu nhiên: Giải cứu người đẹp bị giam cầm trong Hầm Ngục
    hasCaptiveEvent = false;
    captiveRescued = false;
    if (!isBoss && !shopFloor && floorMod != MOD_TREASURE && floor >= 2) {
        std::vector<int> locked;
        for (int i = 0; i < 4; ++i) if (!spouseUnlocked[i]) locked.push_back(i);
        if (!locked.empty() && (floor % 2 == 0 || Rand01() < 0.65f)) {
            hasCaptiveEvent = true;
            captiveSpouseIdx = locked[rand() % locked.size()];
            captivePos = { (Rand01() < 0.5f ? -230.0f : 230.0f), -65.0f };
            ResolvePillars(captivePos, 42.0f);
        }
    }

    // Đài Giao Ước Bóng Tối (Slay the Spire / Isaac risk-reward altar)
    hasBloodPact = (!isBoss && !shopFloor && floorMod != MOD_TREASURE && floor >= 4 && (floor % 3 == 1 || floor % 5 == 4));
    bloodPactUsed = false;
    if (hasBloodPact) {
        bloodPactPos = { (Rand01() < 0.5f ? 225.0f : -225.0f), 95.0f };
        ResolvePillars(bloodPactPos, 40.0f);
    }

    // Thời tiết chiến thuật & Đài Rèn Cổ Ngữ (Ancient Runic Anvil)
    weatherKind = (floor == 1) ? 0 : (Rand01() < 0.25f ? 0 : (1 + (floor + rand()) % 4));
    weatherTimer = 5.5f;
    lightningFlash = 0.0f;
    bossHpLag = 1.0f;
    hasRunicAnvil = (!isBoss && !shopFloor && floorMod != MOD_TREASURE && floor >= 2 && (floor % 2 == 0 || Rand01() < 0.45f));
    runicAnvilUsed = false;
    if (hasRunicAnvil) {
        runicAnvilPos = { (Rand01() < 0.5f ? -215.0f : 215.0f), 145.0f };
        ResolvePillars(runicAnvilPos, 40.0f);
    }

    // Yêu Tinh Vàng (Treasure Goblin) ở tầng 2+ (30% cơ hội)
    if (!isBoss && !shopFloor && floor >= 2 && Rand01() < 0.30f) {
        Enemy gob = BuildEnemy(CHASER, GetRandomSpawnPos());
        gob.isGoldenGoblin = true;
        gob.isElite = false;
        gob.maxHp = 55.0f * HpMult();
        gob.hp = gob.maxHp;
        gob.speed = 235.0f;
        gob.contactDmg = 0.0f;
        gob.radius = 18.0f;
        gob.escapeTimer = 18.0f;
        enemies.push_back(gob);
    }

    if (portalOpen) BuildPortalChoices();
}

inline void Game::SpawnWarden(Vector2 pos) {
    SpawnEnemy(BRUTE, pos);
    Enemy& w = enemies.back();
    w.isWarden = true;
    w.isElite = false;
    w.maxHp = 210.0f * (HpMult());
    w.hp = w.maxHp;
    w.speed = 88.0f;
    w.contactDmg = 19.0f * (DmgMult());
    w.radius = 36.0f;
}

inline Vector2 Game::GetRandomSpawnPos() {
    for (int tries = 0; tries < 50; ++tries) {
        float x = ((float)rand() / RAND_MAX - 0.5f) * (roomW - 180);
        float y = ((float)rand() / RAND_MAX - 0.5f) * (roomH - 180);
        Vector2 p = { x, y };
        bool nearPillar = false;
        for (const auto& pl : pillars) if (Vector2Distance(p, pl) < 70.0f) nearPillar = true;
        if (!nearPillar && Vector2Distance(p, player.pos) > 180.0f) return p;
    }
    return { 0, -150 };
}

inline Enemy Game::BuildEnemy(EnemyType type, Vector2 pos) {
    Enemy e;
    e.type = type;
    e.pos = pos;
    e.vel = { 0, 0 };
    float multHp = HpMult();
    float multDmg = DmgMult();

    if (type == CHASER) {
        e.maxHp = 44.0f * multHp;
        e.speed = 178.0f;
        e.contactDmg = 13.0f * multDmg;
        e.radius = 18.0f;
    } else if (type == RANGER) {
        e.maxHp = 36.0f * multHp;
        e.speed = 152.0f;
        e.contactDmg = 10.0f * multDmg;
        e.radius = 18.0f;
    } else if (type == BRUTE) {
        e.maxHp = 110.0f * multHp;
        e.speed = 98.0f;
        e.contactDmg = 19.0f * multDmg;
        e.radius = 26.0f;
    } else if (type == CHARGER) {
        e.maxHp = 62.0f * multHp;
        e.speed = 124.0f;
        e.contactDmg = 15.0f * multDmg;
        e.radius = 21.0f;
        e.chargeCooldown = 0.8f + 1.4f * Rand01();
    } else if (type == SPLITTER) {
        e.maxHp = 75.0f * multHp;
        e.speed = 128.0f;
        e.contactDmg = 15.0f * multDmg;
        e.radius = 26.0f;
    } else if (type == SUMMONER) {
        e.maxHp = 54.0f * multHp;
        e.speed = 108.0f;
        e.contactDmg = 10.0f * multDmg;
        e.radius = 20.0f;
        e.summonTimer = 2.5f + 2.0f * Rand01();
    } else if (type == BOMBER) {
        e.maxHp = 30.0f * multHp;
        e.speed = 194.0f;
        e.contactDmg = 8.0f * multDmg;
        e.radius = 17.0f;
    } else if (type == BOSS) {
        e.bossKind = std::max(0, ((floor / 5) - 1)) % 3; // Tầng 5: Void Sovereign (0), Tầng 10: Molten Colossus (1), Tầng 15: Frost Dragon Archon (2)
        if (e.bossKind == 1) {
            e.maxHp = 720.0f * multHp;
            e.speed = 106.0f;
            e.contactDmg = 28.0f * multDmg;
            e.radius = 48.0f;
            e.summonTimer = 0.0f;
        } else if (e.bossKind == 2) {
            e.maxHp = 660.0f * multHp;
            e.speed = 114.0f;
            e.contactDmg = 26.0f * multDmg;
            e.radius = 46.0f;
            e.summonTimer = 0.0f;
        } else {
            e.maxHp = 580.0f * multHp;
            e.speed = 124.0f;
            e.contactDmg = 24.0f * multDmg;
            e.radius = 42.0f;
        }
    }
    float spdScale = 1.0f + std::min(0.22f, (floor - 1) * 0.007f) + (settings.difficulty == 2 ? 0.08f : 0.0f);
    e.speed *= spdScale;
    e.hp = e.maxHp;

    // Elite variant chance on floor 2+ (hoặc tầng 1 ở độ khó Khó)
    if (type != BOSS && (floor >= 2 || settings.difficulty == 2) && ((float)rand() / RAND_MAX < eliteChance)) {
        e.isElite = true;
        e.eliteAffix = 1 + (rand() % 4);
        e.affixTimer = 2.0f + Rand01() * 2.0f;
        e.maxHp *= (e.eliteAffix == 4 ? 2.1f * 1.15f : 2.1f);
        e.hp = e.maxHp;
        e.contactDmg *= 1.35f;
        e.radius *= 1.2f;
    }

    return e;
}

inline void Game::SpawnPopup(Vector2 pos, const std::string& txt, Color col) {
    if (popups.size() >= 60) {
        popups.erase(popups.begin(), popups.begin() + (popups.size() - 59));
    }
    DamagePopup p;
    p.pos = pos;
    p.text = txt;
    p.color = col;
    popups.push_back(p);
}

inline bool Game::HasRelic(RelicType t) const {
    for (const auto& r : player.relics) {
        if (r.type == t) return true;
    }
    return false;
}

inline Relic Game::MakeRelic(RelicType t) {
    Relic r;
    r.type = t;
    if (t == RELIC_LIGHTNING) { r.name = "Thunder Conduit"; r.symbol = "⚡"; r.color = SKYBLUE; }
    else if (t == RELIC_TOXIC) { r.name = "Toxic Fang"; r.symbol = "☣"; r.color = LIME; }
    else if (t == RELIC_FROST) { r.name = "Frost Core"; r.symbol = "❄"; r.color = RAYWHITE; }
    else if (t == RELIC_AEGIS) { r.name = "Divine Aegis"; r.symbol = "🛡"; r.color = GOLD; }
    else if (t == RELIC_CHALICE) { r.name = "Blood Chalice"; r.symbol = "🩸"; r.color = RED; }
    else if (t == RELIC_COMBUSTION) { r.name = "Combustion Sigil"; r.symbol = "💥"; r.color = ORANGE; }
    else if (t == RELIC_PHOENIX) { r.name = "Phoenix Feather"; r.symbol = "F"; r.color = { 255, 140, 60, 255 }; }
    else if (t == RELIC_MIDAS) { r.name = "Midas Touch"; r.symbol = "G"; r.color = { 255, 235, 120, 255 }; }
    else if (t == RELIC_BERSERK) { r.name = "Berserker Mask"; r.symbol = "B"; r.color = { 255, 90, 150, 255 }; }
    else { r.name = "Thorn Mail"; r.symbol = "T"; r.color = { 170, 120, 255, 255 }; }
    return r;
}

inline void Game::AddRelic(RelicType t) {
    if (HasRelic(t)) return;
    Relic r = MakeRelic(t);
    if (t == RELIC_AEGIS) player.shieldActive = true;
    player.relics.push_back(r);
    PlaySound(snd.sndLevelUp);
    ShowBanner("RELIC: " + r.name + " - " + RelicDesc(t), 3.5f);
}

inline void Game::Update(float dt) {
    if (bannerTimer > 0) bannerTimer -= dt;
    if (screenShakeTimer > 0) screenShakeTimer -= dt;
    PollGamepad();
    PollTouchControls();
    if (rumbleTimer > 0) {
        rumbleTimer -= dt;
        if (rumbleTimer <= 0) RumbleRaw(0.0f, 0.0f);
    }

    // Hit-stop: chỉ dùng khoảnh khắc kết liễu Trùm, không làm khựng vòng lặp chiến đấu thường
    if (hitStop > 0.0f) {
        hitStop = std::min(hitStop, 0.04f) - dt;
        UpdateEffects(dt);
        return;
    }
    if (slowMoTimer > 0.0f) { slowMoTimer -= dt; dt *= 0.35f; }
    if (perfectDodgeTimer > 0.0f) perfectDodgeTimer -= dt;
    if (runeBuffTimer > 0.0f) runeBuffTimer -= dt;
    if (overdriveTimer > 0.0f) overdriveTimer -= dt;
    if (lightningFlash > 0.0f) lightningFlash = std::max(0.0f, lightningFlash - dt * 2.8f);
    if (styleDecayTimer > 0.0f) {
        styleDecayTimer -= dt;
    } else if (styleRank > 0 || stylePoints > 0.0f) {
        stylePoints -= dt * 28.0f;
        if (stylePoints < 0.0f) {
            if (styleRank > 0) { styleRank--; stylePoints = 75.0f; }
            else stylePoints = 0.0f;
        }
    }

    if (namingMode != 0) {
        UpdateEffects(dt);
        UpdateNameModal();
        return;
    }

#ifndef __ANDROID__
    if (IsKeyPressed(KEY_F11)) {
        settings.windowMode = (settings.windowMode == 1) ? 0 : 1;
        SaveSettings();
        ApplySettings();
    }
#endif

    if (state == TITLE) {
        UpdateEffects(dt);
        bannerTimer = 3.0f; // giữ thông báo tầng 1 cho đến khi bắt đầu chơi
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (showHelp) {
            if (EscPressed() || IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H)) showHelp = false;
            return;
        }
        if (IsKeyPressed(KEY_L)) { OpenSaveLoad(TITLE); return; }
        if (IsKeyPressed(KEY_O) || IsKeyPressed(KEY_F2)) { settingsReturn = TITLE; setSel = 0; state = SETTINGS; return; }
        if (IsKeyPressed(KEY_S)) { OpenSanctuary(TITLE); return; }
        if (IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H)) showHelp = true;
        else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || PadPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
            showHelp = false;
            classSel = selectedClass;
            state = CLASS_SELECT;
            PlaySound(snd.sndPotion);
        }
        return;
    }
    if (state == CLASS_SELECT) {
        UpdateEffects(dt);
        UpdateClassSelect();
        return;
    }
    if (state == SAVE_LOAD) {
        UpdateEffects(dt);
        UpdateSaveLoad(dt);
        return;
    }
    if (state == PAUSED) {
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (showHelp) {
            if (EscPressed() || IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H)) showHelp = false;
            return;
        }
        if (IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H)) { showHelp = true; return; }
        if (EscPressed() || IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || PadPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) { showHelp = false; state = PLAYING; return; }
        if (IsKeyPressed(KEY_F5) || IsKeyPressed(KEY_V)) { OpenSaveLoad(PAUSED); return; }
        if (IsKeyPressed(KEY_R)) { showHelp = false; RestartRun(); }
        if (IsKeyPressed(KEY_C)) { showHelp = false; classSel = selectedClass; state = CLASS_SELECT; }
        if (IsKeyPressed(KEY_I)) { prevState = PAUSED; state = INVENTORY; }
        if (IsKeyPressed(KEY_O)) { settingsReturn = PAUSED; setSel = 0; state = SETTINGS; }
        return;
    }
    if (state == SETTINGS) {
        UpdateSettingsMenu();
        return;
    }
    if (state == SHOP) {
        UpdateShop();
        return;
    }
    if (state == META) {
        UpdateMeta(dt);
        return;
    }
    if (state == SURFACE_UI) {
        UpdateSurfaceUi();
        return;
    }
    if (state == FAMILY_TREE) {
        UpdateFamilyTree();
        return;
    }
    if (state == WORLD_MAP) {
        UpdateWorldMap(dt);
        return;
    }
    if (state == INVENTORY) {
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (IsKeyPressed(KEY_Y)) { OpenFamilyTree(INVENTORY); return; }
        if (IsKeyPressed(KEY_F3) || IsKeyPressed(KEY_TAB)) { OpenWorldMap(INVENTORY); return; }
        if (IsKeyPressed(KEY_I) || EscPressed() || IsKeyPressed(KEY_ENTER) || PadPressed(GAMEPAD_BUTTON_MIDDLE_LEFT) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) state = prevState;
        return;
    }
    if (state == PLAYING && IsKeyPressed(KEY_Y)) {
        OpenFamilyTree(PLAYING);
        return;
    }
    if (state == PLAYING && (IsKeyPressed(KEY_F3) || (IsKeyPressed(KEY_TAB) && !inSurface) || (inSurface && IsKeyPressed(KEY_K)))) {
        OpenWorldMap(PLAYING);
        return;
    }
    if (state == PLAYING && IsKeyPressed(KEY_H)) {
        EatRation();
    }
    if (state == PLAYING && !inSurface && portalOpen && (floor >= 10 || surfaceUnlocked) && IsKeyPressed(KEY_G)) {
        EnterSurface(floor + 1);
        return;
    }
    if (state == PLAYING && (IsKeyPressed(KEY_I) || PadPressed(GAMEPAD_BUTTON_MIDDLE_LEFT))) {
        prevState = PLAYING;
        state = INVENTORY;
        return;
    }
    if (state == PLAYING && (EscPressed() || IsKeyPressed(KEY_P) || PadPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
        StopRumble();
        state = PAUSED;
        return;
    }

    UpdateEffects(dt);

    if (state == LEVEL_UP) {
        if (subclassChoosing) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mp = GetMousePosition();
                float cw = 450.0f, gap = 36.0f, sx = 1280 / 2.0f - (cw * 2 + gap) / 2.0f;
                for (int i = 0; i < 2; ++i) {
                    Rectangle cr = { sx + i * (cw + gap), 116.0f, cw, 554.0f };
                    if (CheckCollisionPointRec(mp, cr)) { ChooseSubclass(i + 1); return; }
                }
            }
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) ChooseSubclass(1);
            if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) ChooseSubclass(2);
            return;
        }
        // Number keys 1, 2, 3 to choose
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            float cardW = 240.0f, startX = 1280 / 2 - (cardW * 3 + 40) / 2;
            for (size_t i = 0; i < levelChoices.size(); ++i) {
                Rectangle cr = { startX + i * (cardW + 20), 240.0f, cardW, 280.0f };
                if (CheckCollisionPointRec(mp, cr)) { ChooseUpgrade(levelChoices[i]); return; }
            }
        }
        if ((IsKeyPressed(KEY_ONE) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) && levelChoices.size() >= 1) ChooseUpgrade(levelChoices[0]);
        if ((IsKeyPressed(KEY_TWO) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP)) && levelChoices.size() >= 2) ChooseUpgrade(levelChoices[1]);
        if ((IsKeyPressed(KEY_THREE) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) && levelChoices.size() >= 3) ChooseUpgrade(levelChoices[2]);
        return;
    }

    if (state == GAME_OVER) {
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (EscPressed()) { state = TITLE; return; }
        if (IsKeyPressed(KEY_L)) { OpenSaveLoad(GAME_OVER); return; }
        if (IsKeyPressed(KEY_S) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) { OpenSanctuary(GAME_OVER); return; }
        if (IsKeyPressed(KEY_C)) { classSel = selectedClass; state = CLASS_SELECT; return; }
        if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            RestartRun();
        }
        return;
    }

    // Toggle radar with M & cập nhật đồng hồ thời gian + tuổi
    if (IsKeyPressed(KEY_M)) showMiniMap = !showMiniMap;
    AdvanceTime(dt * 0.08f);
    if (UpdateAging(dt)) return; // 1 năm trong game = 180 giây chơi thực tế

    // Player Timers
    if (player.dashTimer > 0) player.dashTimer -= dt;
    if (player.specialTimer > 0) player.specialTimer -= dt;
    if (player.skill2Timer > 0) player.skill2Timer -= dt;
    if (player.skill3Timer > 0) player.skill3Timer -= dt;
    if (player.skill4Timer > 0) player.skill4Timer -= dt;
    if (player.attackTimer > 0) player.attackTimer -= dt;
    if (player.invulnTimer > 0) player.invulnTimer -= dt;
    if (player.holyBuffTimer > 0) player.holyBuffTimer -= dt;
    if (player.hasteTimer > 0) player.hasteTimer -= dt;
    if (player.frostSlowTimer > 0) player.frostSlowTimer -= dt;
    if (player.hpDisplay > player.hp) {
        player.hpDisplay = std::max(player.hp, player.hpDisplay - std::max(28.0f, (player.hpDisplay - player.hp) * 3.5f) * dt);
    } else {
        player.hpDisplay = player.hp;
    }
    player.isInvuln = (player.invulnTimer > 0 || player.isDashing);

    // Aegis Shield Recharging
    if (HasRelic(RELIC_AEGIS) && !player.shieldActive) {
        player.shieldTimer += dt;
        if (player.shieldTimer >= 12.0f) {
            player.shieldActive = true;
            player.shieldTimer = 0.0f;
            PlaySound(snd.sndShield);
        }
    }

    // Combo Decay
    if (comboTimer > 0) {
        comboTimer -= dt;
        if (comboTimer <= 0) comboCount = 0;
    }

    // Thanh Đói (chỉ xuất hiện và tiêu hao sau khi người chơi lên Mặt Đất qua Tầng 10)
    if (surfaceUnlocked) {
        float drainRate = inSurface ? 0.35f : (1.0f * HouseHungerMult());
        hunger = std::max(0.0f, hunger - drainRate * dt);
        if (hunger >= 70.0f && player.hp > 0.0f && player.hp < player.maxHp) {
            player.hp = std::min(player.maxHp, player.hp + 1.0f * dt);
        }
        if (hunger <= 0.0f) {
            starveTickTimer += dt;
            if (starveTickTimer >= 1.0f) {
                starveTickTimer = 0.0f;
                float minFloorHp = inSurface ? 5.0f : 0.0f;
                if (player.hp > minFloorHp) {
                    player.hp = std::max(minFloorHp, player.hp - 2.5f);
                    SpawnPopup(player.pos, L("-STARVE", "-ĐÓI"), RED);
                    if (!inSurface && player.hp <= 0.0f) {
                        state = GAME_OVER;
                        PlaySound(snd.sndDeath);
                        AwardSouls();
                        if (floor > bestFloor) { bestFloor = floor; SaveSettings(); newRecord = true; }
                        return;
                    }
                }
            }
        } else {
            starveTickTimer = 0.0f;
        }
    }

    // ── Input & Movement ──
    Vector2 move = { 0, 0 };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1;
    if (Vector2Length(move) > 0) move = Vector2Normalize(move);

    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 aimDir = Vector2Normalize(Vector2Subtract(mouseWorld, player.pos));
    if (Vector2Length(aimDir) < 0.01f) aimDir = { player.facingLeft ? -1.0f : 1.0f, 0.0f };
    if (fabs(aimDir.x) > 0.1f) player.facingLeft = (aimDir.x < 0);
    if (ctl.active) {
        move = ctl.move;
        if (Vector2Length(ctl.aim) > 0.01f) aimDir = Vector2Normalize(ctl.aim);
        if (fabs(aimDir.x) > 0.1f) player.facingLeft = (aimDir.x < 0);
    }
    aimDirDraw = aimDir;

    float effSpd = EffectiveMoveSpeed();
    if (player.isDashing) {
        player.pos = Vector2Add(player.pos, Vector2Scale(player.dashDir, effSpd * 2.8f * dt));
        if (player.dashTimer <= player.dashCooldown - player.dashDur) {
            player.isDashing = false;
        }
    } else {
        player.pos = Vector2Add(player.pos, Vector2Scale(move, effSpd * dt));
        if (Vector2Length(move) > 0.1f) player.walkTimer += dt * 10.0f;
    }

    // Bounds Clamp
    float boundW = roomW / 2.0f - 30;
    float boundH = roomH / 2.0f - 30;
    player.pos.x = Clamp(player.pos.x, -boundW, boundW);
    player.pos.y = Clamp(player.pos.y, -boundH, boundH);

    // Mặt Đất: đi lại trong Quảng Trường & từng phòng công trình riêng biệt
    if (inSurface) {
        ResolvePillars(player.pos, 15.0f);
        bool interact = (IsKeyPressed(KEY_F) || IsKeyPressed(KEY_ENTER) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP));
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_B)) {
            OpenSurfaceModal(surfaceRoom >= 1 ? surfaceRoom - 1 : surfaceBuilding);
            return;
        }
        for (int bi = 0; bi < 4; ++bi) {
            if (IsKeyPressed(KEY_ONE + bi)) { EnterSurfaceRoom(bi + 1); return; }
        }
        if (IsKeyPressed(KEY_FIVE)) { EnterSurfaceRoom(5); return; }
        if (surfaceRoom >= 1 && IsKeyPressed(KEY_ZERO)) {
            LeaveSurfaceRoomToTown();
            return;
        }

        if (surfaceRoom == 0) {
            // Va chạm vật lý với Đài Phun Nước trung tâm & khối tường các công trình
            float fDist = Vector2Length(player.pos);
            if (fDist < 79.0f) {
                Vector2 dir = (fDist > 0.01f) ? Vector2Scale(player.pos, 1.0f / fDist) : Vector2{ 0.0f, 1.0f };
                player.pos = Vector2Scale(dir, 79.0f);
            }
            for (int bi = 0; bi < 4; ++bi) {
                Vector2 bp = SurfaceLandmarkPos(bi);
                // Khối nhà phía trên (vẫn để mở hành lang cửa vào ở mặt trước y = bp.y + 38)
                float rx0 = bp.x - 94.0f, rx1 = bp.x + 94.0f;
                float ry0 = bp.y - 68.0f, ry1 = bp.y + 28.0f;
                float cx = Clamp(player.pos.x, rx0, rx1);
                float cy = Clamp(player.pos.y, ry0, ry1);
                Vector2 diff = Vector2Subtract(player.pos, { cx, cy });
                float dd = Vector2Length(diff);
                if (dd < 15.0f) {
                    if (dd > 0.01f) player.pos = Vector2Add({ cx, cy }, Vector2Scale(diff, 15.0f / dd));
                    else player.pos.y = ry1 + 15.0f;
                }
                Vector2 door = { bp.x, bp.y + 38.0f };
                float doorDist = Vector2Distance(player.pos, door);
                if (doorDist < 30.0f || (doorDist < 96.0f && interact)) {
                    EnterSurfaceRoom(bi + 1);
                    return;
                }
            }
            if (fDist < 122.0f && interact) {
                player.hp = std::min(player.maxHp, player.hp + 20.0f);
                PlaySound(snd.sndPotion);
                Burst({ 0.0f, 0.0f }, SKYBLUE, 18, 170.0f, 0.55f);
                SpawnPopup(player.pos, "+20 HP", SKYBLUE);
                ShowBanner(L("Drank Crystal Fountain Water (+20 HP)", "Đã uống nước Đài Phun Nước Thánh (+20 Máu)"), 2.0f);
            }
            Vector2 fiefGate = SurfaceLandmarkPos(5);
            if (Vector2Distance(player.pos, fiefGate) < 88.0f && interact) {
                EnterSurfaceRoom(5);
                return;
            }
            Vector2 dgPos = SurfaceLandmarkPos(4);
            float dgDist = Vector2Distance(player.pos, dgPos);
            if (dgDist < 54.0f || (dgDist < 96.0f && (interact || IsKeyPressed(KEY_G)))) {
                LeaveSurfaceToDungeon();
                return;
            }
        } else {
            // Cửa phía Nam trở ra Quảng Trường
            Vector2 exitPos = SurfaceInteriorExitPos();
            float ed = Vector2Distance(player.pos, exitPos);
            if (ed < 30.0f || (ed < 95.0f && interact)) {
                LeaveSurfaceRoomToTown();
                return;
            }
            // [T] tại Tổ Ấm: dẫn con theo / để con ở nhà
            if (surfaceRoom == 3 && IsKeyPressed(KEY_T) && !children.empty()) {
                int bestK = -1; float minKD = 95.0f;
                for (size_t k = 0; k < children.size(); ++k) {
                    float d = Vector2Distance(player.pos, HeirPosInHome((int)k));
                    if (d < minKD) { minKD = d; bestK = (int)k; }
                }
                if (bestK >= 0) ToggleChildParty(bestK);
                else ShowBanner(L("Walk up to a child and press [T] to bring them along.", "Lại gần một người con rồi nhấn [T] để dẫn theo."), 2.2f);
            }
            if (interact) {
                int bestAct = -1;
                float bestD = 9999.0f;
                auto checkCand = [&](int actId, Vector2 pos, float maxR) {
                    float d = Vector2Distance(player.pos, pos);
                    if (d < maxR && d < bestD) { bestD = d; bestAct = actId; }
                };
                if (surfaceRoom == 1) {
                    for (int i = 0; i < 4; ++i) checkCand(i, TavernStallPos(i), 94.0f);
                    checkCand(10, { 0.0f, -275.0f }, 110.0f); // Quầy chủ quán
                    if (bestAct >= 0 && bestAct <= 3) BuyMeal(bestAct);
                    else if (bestAct == 10) { OpenSurfaceModal(0); return; }
                } else if (surfaceRoom == 2) {
                    for (int t = 1; t <= 3; ++t) checkCand(t, HousingPedestalPos(t), 108.0f);
                    checkCand(10, { 0.0f, -295.0f }, 110.0f); // Bàn Kiến Trúc Sư
                    if (bestAct >= 1 && bestAct <= 3) BuyHouseUpgrade(bestAct);
                    else if (bestAct == 10) { OpenSurfaceModal(1); return; }
                } else if (surfaceRoom == 3) {
                    if (IsKeyPressed(KEY_N) && !children.empty() && !ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                        int bestK = 0; float minKD = 9999.0f;
                        for (size_t k = 0; k < children.size(); ++k) {
                            float d = Vector2Distance(player.pos, HeirPosInHome((int)k));
                            if (d < minKD) { minKD = d; bestK = (int)k; }
                        }
                        OpenNameModal(2, bestK, children[bestK].name);
                        return;
                    }
                    if (!hasSpouse) {
                        for (int i = 0; i < 4; ++i) checkCand(i, SpouseCandPos(i), 85.0f);
                    } else {
                        checkCand(10, { -235.0f, -175.0f }, 95.0f); // Vợ đang chờ bên lò sưởi
                    }
                    checkCand(20, { 0.0f, -235.0f }, 98.0f);      // Nôi Sinh Con
                    checkCand(21, { 275.0f, -235.0f }, 98.0f);    // Từ Đường Gia Phả
                    for (size_t k = 0; k < children.size(); ++k) {
                        checkCand(100 + (int)k, HeirPosInHome((int)k), 85.0f);
                    }
                    if (bestAct >= 0 && bestAct < 4) TryMarrySpouse(bestAct);
                    else if (bestAct == 10 && spouseIdx >= 0) {
                        player.hp = std::min(player.maxHp, player.hp + 25.0f);
                        hunger = std::min(maxHunger, hunger + 10.0f);
                        PlaySound(snd.sndPotion);
                        Burst({ -235.0f, -175.0f }, { 255, 130, 185, 255 }, 16, 150.0f, 0.55f);
                        ShowBanner(std::string(SpouseName(spouseIdx)) + L(": Stay safe in the Dungeon, my love! (+25 HP)", ": Chàng hãy giữ gìn sức khỏe dưới Hầm Ngục nhé! (+25 Máu)"), 2.8f);
                    } else if (bestAct == 20) TryHaveChild();
                    else if (bestAct == 21) { OpenFamilyTree(PLAYING); return; }
                    else if (bestAct >= 100) { SuccessionToChild(bestAct - 100); return; }
                } else if (surfaceRoom == 4) {
                    checkCand(1, { -235.0f, -95.0f }, 108.0f);    // Đài Quý Tộc (1500G)
                    checkCand(2, {  235.0f, -95.0f }, 108.0f);    // Bàn Chiến Lược Ủy Thác
                    checkCand(3, {    0.0f, -295.0f }, 115.0f);   // Ngai Vàng Đức Vua
                    if (bestAct == 1) BuyNobleTitleDirect();
                    else if (bestAct == 2) AcceptRoyalCommission();
                    else if (bestAct == 3) { OpenSurfaceModal(3); return; }
                } else if (surfaceRoom == 5) {
                    checkCand(10, {    0.0f, -265.0f }, 115.0f);  // Rương Kho & Hiến Chương Lãnh Địa
                    checkCand(11, {    0.0f,  -65.0f }, 105.0f);  // Doanh Trại Chiêu Binh
                    checkCand(12, {    0.0f,  125.0f }, 115.0f);  // Bàn Bản Đồ Thế Giới & Chiến Tranh
                    checkCand(0,  { -320.0f,  -60.0f }, 95.0f);   // Thành Quách
                    checkCand(1,  {  320.0f,  -60.0f }, 95.0f);   // Doanh Trại
                    checkCand(2,  { -320.0f,  125.0f }, 95.0f);   // Thương Điếm
                    checkCand(3,  {  320.0f,  125.0f }, 95.0f);   // Học Viện
                    if (bestAct == 10) EstablishFief();
                    else if (bestAct == 11) RecruitFiefSoldiers();
                    else if (bestAct == 12) { OpenWorldMap(PLAYING); return; }
                    else if (bestAct >= 0 && bestAct <= 3) UpgradeFiefBuilding(bestAct);
                }
            }
        }
        camera.target = player.pos;
        return;
    }

    bool dashPressed = ctl.active ? ctl.dash : (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_K));
    bool attackHeld = ctl.active ? ctl.attack : (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_J));
    bool skill1Pressed = ctl.active ? ctl.nova : (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_U));
    bool skill2Pressed = ctl.active ? ctl.skill2 : (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_L));
    bool skill3Pressed = ctl.active ? ctl.skill3 : (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_Z));
    bool skill4Pressed = ctl.active ? ctl.skill4 : (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_X));
    bool ultPressed = ctl.active ? ctl.ult : (IsKeyPressed(KEY_R) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE));

    if (dashPressed && player.dashTimer <= 0) {
        player.isDashing = true;
        player.dashTimer = player.dashCooldown;
        player.dashDir = (Vector2Length(move) > 0.1f) ? move : aimDir;
        PlaySound(snd.sndDash);
        ghosts.push_back({ player.pos, player.facingLeft, 0.22f, 0.22f });
        // Perfect Dodge (Né Hoàn Hảo) khi lướt sát đạn hoặc đòn quái
        bool nearDanger = false;
        for (const auto& pr : projectiles) {
            if (!pr.fromPlayer && Vector2Distance(player.pos, pr.pos) < 80.0f) { nearDanger = true; break; }
        }
        if (!nearDanger) {
            for (const auto& en : enemies) {
                if (en.hp > 0 && Vector2Distance(player.pos, en.pos) < en.radius + 72.0f &&
                    (en.isAttacking || (en.type == CHARGER && (en.chargeState == 1 || en.chargeState == 2)) || (en.type == BOMBER && en.fusing))) {
                    nearDanger = true; break;
                }
            }
        }
        if (nearDanger) TriggerPerfectDodge();
        if (player.heroClass == CLASS_ASSASSIN && player.subClass == 2) {
            GroundEffect decoy;
            decoy.kind = 0;
            decoy.pos = player.pos;
            decoy.radius = 115.0f;
            decoy.maxTimer = 0.50f;
            decoy.timer = 0.50f;
            decoy.damage = player.baseDmg * 1.5f;
            decoy.color = { 110, 235, 255, 255 };
            groundEffects.push_back(decoy);
            Burst(player.pos, { 110, 235, 255, 255 }, 10, 160.0f, 0.35f);
        }
    }

    // Basic Weapon Attack
    if (IsKeyPressed(KEY_B) && !ctl.active) {
        QuickForgeWithEssence();
    }
    if (attackHeld && player.attackTimer <= 0 && !player.isDashing) {
        MeleeAttack(aimDir);
    }

    // Skill 1 (Q / U)
    if (skill1Pressed && player.specialTimer <= 0) {
        CastSkill1(aimDir);
    }

    // Skill 2 (E / L)
    if (skill2Pressed && player.skill2Timer <= 0) {
        CastSkill2(aimDir, move);
    }

    // Kỹ năng Nhánh Phụ 3 [C/Z] & 4 [V/X] (mở khóa khi Chuyển Chức Lv.5)
    if (player.subClass != 0) {
        if (skill3Pressed && player.skill3Timer <= 0.0f) CastSkill3(aimDir);
        if (skill4Pressed && player.skill4Timer <= 0.0f) CastSkill4(aimDir, move);
    }

    // Ultimate / Rage Skill (R / MMB)
    if (ultPressed && player.rage >= player.maxRage) {
        CastUltimate(aimDir);
    }

    // Active Ultimates & Ground Effects (Traps, Consecrated Ground, Void Vortex, Meteors, Arrow Rain)
    UpdateActiveSkills(dt);

    // ── Enemies AI & Update ──
    for (auto& e : enemies) {
        if (e.hp <= 0) continue;

        if (e.hitFlash > 0) e.hitFlash -= dt;
        if (e.stunTimer > 0) e.stunTimer -= dt;
        if (e.reactionCooldown > 0.0f) e.reactionCooldown -= dt;
        if (e.staggerBrokenTimer > 0.0f) {
            e.staggerBrokenTimer -= dt;
            if (e.staggerBrokenTimer <= 0.0f) e.stagger = 0.0f;
        } else if (e.stagger > 0.0f) {
            e.stagger = std::max(0.0f, e.stagger - 4.0f * dt);
        }

        // Golden Goblin: chạy trốn kèm tia sáng vàng, biến mất nếu hết giờ
        if (e.isGoldenGoblin) {
            e.escapeTimer -= dt;
            if (Rand01() < 0.25f) Burst(e.pos, GOLD, 1, 70.0f, 0.35f);
            if (e.escapeTimer <= 0.0f) {
                Burst(e.pos, GOLD, 14, 180.0f, 0.5f);
                SpawnPopup(e.pos, L("ESCAPED!", "CHẠY THOÁT!"), YELLOW);
                e.hp = 0.0f;
                continue;
            }
            Vector2 away = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
            if (Vector2Length(away) < 0.1f) away = { 1.0f, 0.0f };
            float wob = sinf((float)GetTime() * 6.0f) * 0.45f;
            Vector2 runDir = { away.x * cosf(wob) - away.y * sinf(wob), away.x * sinf(wob) + away.y * cosf(wob) };
            e.facingLeft = (runDir.x < 0.0f);
            e.pos = Vector2Add(e.pos, Vector2Scale(runDir, e.speed * dt));
            e.pos.x = Clamp(e.pos.x, -roomW / 2.0f + e.radius + 16.0f, roomW / 2.0f - e.radius - 16.0f);
            e.pos.y = Clamp(e.pos.y, -roomH / 2.0f + e.radius + 16.0f, roomH / 2.0f - e.radius - 16.0f);
            ResolvePillars(e.pos, e.radius);
            continue;
        }

        // Poison DoT
        if (e.poisonTimer > 0) {
            e.poisonTimer -= dt;
            e.hp -= e.poisonDmg * dt;
            if (e.hp <= 0) { OnEnemyKilled(e); continue; }
        }
        // Chill
        float speedMod = 1.0f;
        if (e.chillTimer > 0) {
            e.chillTimer -= dt;
            speedMod = 0.5f;
        }
        // Frost Touch: làm chậm
        if (e.slowTimer > 0) {
            e.slowTimer -= dt;
            speedMod *= e.slowFactor;
        } else {
            e.slowFactor = 1.0f;
        }
        if (!inSurface && weatherKind == 1) speedMod *= 1.06f; // Nhật Thực Huyết Nguyệt: Quái +6% tốc độ
        // Ember Edge: đốt
        if (e.burnTimer > 0) {
            e.burnTimer -= dt;
            e.hp -= e.burnDps * dt;
            if (Rand01() < 0.15f) Burst(e.pos, { 255, 140, 40, 255 }, 1, 70.0f, 0.4f);
            if (e.hp <= 0) { OnEnemyKilled(e); continue; }
        }

        Vector2 toPlayer = Vector2Subtract(player.pos, e.pos);
        float dist = Vector2Length(toPlayer);
        Vector2 dir = dist > 0.01f ? Vector2Normalize(toPlayer) : Vector2{ 1, 0 };
        if (e.stunTimer <= 0.0f && fabs(dir.x) > 0.05f) e.facingLeft = (dir.x < 0);

        if (e.isElite && e.eliteAffix == 2 && e.stunTimer <= 0.0f) {
            e.affixTimer -= dt;
            if (e.affixTimer <= 0.0f && dist > 110.0f && dist < 380.0f) {
                e.affixTimer = 3.2f;
                Burst(e.pos, { 185, 85, 255, 255 }, 10, 180.0f, 0.35f);
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, 95.0f));
                Burst(e.pos, { 185, 85, 255, 255 }, 12, 200.0f, 0.35f);
                SpawnPopup(e.pos, "BLINK", { 195, 120, 255, 255 });
            }
        }
        if (e.isElite && e.eliteAffix == 4 && e.stunTimer <= 0.0f) {
            e.affixTimer -= dt;
            if (e.affixTimer <= 0.0f) {
                e.affixTimer = 3.8f;
                NovaVisual fn;
                fn.pos = e.pos; fn.maxRadius = 85.0f; fn.timer = 0.30f; fn.maxTimer = 0.30f; fn.color = SKYBLUE;
                novas.push_back(fn);
                if (dist < 85.0f && !player.isDashing) {
                    player.frostSlowTimer = std::max(player.frostSlowTimer, 1.5f);
                    Burst(player.pos, SKYBLUE, 8, 120.0f, 0.3f);
                    SpawnPopup(player.pos, "CHILLED!", SKYBLUE);
                }
            }
        }

        if (e.stunTimer > 0.0f) {
            // Đang bị choáng: đứng yên, không thể tấn công hoặc tiếp tục lao
        } else if (e.type == CHASER || e.type == SPLITTER) {
            e.bounceTimer += dt * 6.0f;
            float hop = std::max(0.2f, sinf(e.bounceTimer));
            e.vel = Vector2Scale(dir, e.speed * hop * speedMod);
            e.pos = Vector2Add(e.pos, Vector2Scale(e.vel, dt));
        } else if (e.type == RANGER) {
            e.attackTimer += dt;
            Vector2 moveDir = { 0, 0 };
            if (dist < 220) moveDir = Vector2Negate(dir);
            else if (dist > 340) moveDir = dir;
            else moveDir = { -dir.y, dir.x };
            e.pos = Vector2Add(e.pos, Vector2Scale(moveDir, e.speed * speedMod * dt));

            if (e.attackTimer >= 1.85f) {
                e.attackTimer = 0;
                PlaySound(snd.sndShoot);
                Projectile p;
                p.pos = Vector2Add(e.pos, Vector2Scale(dir, 20.0f));
                p.vel = Vector2Scale(dir, 305.0f * ((!inSurface && weatherKind == 3) ? 0.88f : 1.0f));
                p.damage = e.contactDmg * 0.9f;
                p.fromPlayer = false;
                p.life = 3.0f;
                p.radius = 7.0f;
                p.color = RED;
                projectiles.push_back(p);
            }
        } else if (e.type == SUMMONER) {
            if (e.chargeState == 0) {
                Vector2 moveDir = { 0, 0 };
                if (dist < 260.0f) moveDir = Vector2Negate(dir);
                else if (dist > 380.0f) moveDir = dir;
                else moveDir = { -dir.y, dir.x };
                e.pos = Vector2Add(e.pos, Vector2Scale(moveDir, e.speed * speedMod * dt));
                e.summonTimer -= dt;
                if (e.summonTimer <= 0) {
                    int aliveCount = (int)pendingSpawns.size();
                    for (const auto& o : enemies) if (o.hp > 0) aliveCount++;
                    if (aliveCount < 30) { e.chargeState = 1; e.chargeTimer = 1.0f; PlaySound(snd.sndShield); }
                    else e.summonTimer = 1.0f;
                }
            } else {
                e.chargeTimer -= dt; // đứng yên niệm chú
                if (e.chargeTimer <= 0) {
                    for (int k = 0; k < 2; ++k)
                        SpawnMinionChaser(Vector2Add(e.pos, { (k == 0 ? -34.0f : 34.0f), 10.0f }));
                    Burst(e.pos, { 170, 110, 255, 255 }, 20, 230.0f, 0.6f);
                    PlaySound(snd.sndSpecial);
                    e.chargeState = 0;
                    e.summonTimer = 6.5f;
                }
            }
        } else if (e.type == BOSS && (e.isKingBoss || e.isNationalGeneral)) {
            int effCls = e.isKingBoss ? e.kingClass : CLASS_TEMPLAR;
            if (!e.enraged && e.hp <= e.maxHp * 0.5f) {
                e.enraged = true;
                e.speed *= 1.25f;
                PlaySound(snd.sndSpecial);
                Shake(0.45f, 8.5f);
                ShowBanner(e.isKingBoss
                    ? L("HIS MAJESTY UNLEASHES IMPERIAL WRATH!", "QUỐC VƯƠNG BẠO CHÚA BÙNG NỔ LONG UY!")
                    : L("ENEMY GRAND GENERAL ENRAGES!", "ĐẠI TƯỚNG QUÂN ĐỊCH CUỒNG NỘ!"), 3.2f);
            }
            e.attackTimer += dt;
            float cd = e.enraged ? 1.30f : 1.85f;
            if (e.attackTimer >= cd) {
                e.attackTimer = 0.0f;
                e.summonTimer += 1.0f;
                PlaySound(snd.sndSpecial);
                float baseA = atan2f(dir.y, dir.x);
                float bDmg = e.contactDmg * (e.enraged ? 0.92f : 0.78f);
                if (effCls == CLASS_KNIGHT) {
                    e.pos = Vector2Add(e.pos, Vector2Scale(dir, std::min(140.0f, dist * 0.65f)));
                    NovaVisual nv; nv.pos = e.pos; nv.maxRadius = 155.0f; nv.timer = 0.32f; nv.maxTimer = 0.32f; nv.color = GOLD;
                    novas.push_back(nv);
                    if (Vector2Distance(e.pos, player.pos) <= 155.0f) DamagePlayer(bDmg * 1.15f);
                    for (int k = 0; k < 8; ++k) {
                        float a = k * (PI / 4.0f);
                        Projectile p; p.fromPlayer = false; p.pos = e.pos;
                        p.vel = { cosf(a) * 260.0f, sinf(a) * 260.0f };
                        p.damage = bDmg * 0.85f; p.life = 3.0f; p.radius = 8.5f; p.color = GOLD;
                        projectiles.push_back(p);
                    }
                } else if (effCls == CLASS_ASSASSIN) {
                    float bx = Clamp(player.pos.x - dir.x * 105.0f, -(roomW / 2.0f - 40.0f), roomW / 2.0f - 40.0f);
                    float by = Clamp(player.pos.y - dir.y * 105.0f, -(roomH / 2.0f - 40.0f), roomH / 2.0f - 40.0f);
                    ghosts.push_back({ e.pos, e.facingLeft, 0.25f, 0.25f });
                    e.pos = { bx, by };
                    int fan = e.enraged ? 9 : 7;
                    Vector2 toP = Vector2Normalize(Vector2Subtract(player.pos, e.pos));
                    float ta = atan2f(toP.y, toP.x);
                    for (int k = 0; k < fan; ++k) {
                        float a = ta + (k - (fan - 1) * 0.5f) * 0.20f;
                        Projectile p; p.fromPlayer = false; p.kind = 3; p.pos = e.pos;
                        p.vel = { cosf(a) * 340.0f, sinf(a) * 340.0f };
                        p.damage = bDmg * 0.82f; p.life = 2.6f; p.radius = 8.0f; p.color = { 255, 70, 120, 255 };
                        projectiles.push_back(p);
                    }
                } else if (effCls == CLASS_MAGE) {
                    for (int k = -1; k <= 1; k += 2) {
                        float a = baseA + k * 0.18f;
                        Projectile p; p.fromPlayer = false; p.kind = 4; p.pos = e.pos;
                        p.vel = { cosf(a) * 310.0f, sinf(a) * 310.0f };
                        p.damage = bDmg; p.life = 3.0f; p.radius = 11.0f; p.color = ORANGE;
                        projectiles.push_back(p);
                    }
                    if (((int)e.summonTimer) % 2 == 0) {
                        NovaVisual nv; nv.pos = e.pos; nv.maxRadius = 175.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = SKYBLUE;
                        novas.push_back(nv);
                        if (dist <= 175.0f) { DamagePlayer(bDmg); player.frostSlowTimer = std::max(player.frostSlowTimer, 1.4f); }
                    }
                } else if (effCls == CLASS_RANGER) {
                    int cnt = e.enraged ? 11 : 7;
                    for (int k = 0; k < cnt; ++k) {
                        float a = baseA + (k - (cnt - 1) * 0.5f) * 0.14f;
                        Projectile p; p.fromPlayer = false; p.kind = 1; p.pos = e.pos;
                        p.vel = { cosf(a) * 390.0f, sinf(a) * 390.0f };
                        p.damage = bDmg * 0.80f; p.life = 2.8f; p.radius = 7.5f; p.color = { 115, 245, 145, 255 };
                        projectiles.push_back(p);
                    }
                } else {
                    NovaVisual nv; nv.pos = e.pos; nv.maxRadius = 170.0f; nv.timer = 0.34f; nv.maxTimer = 0.34f; nv.color = GOLD;
                    novas.push_back(nv);
                    if (dist <= 170.0f) DamagePlayer(bDmg * 1.1f);
                    int waves = e.enraged ? 10 : 8;
                    for (int k = 0; k < waves; ++k) {
                        float a = k * (2.0f * PI / waves) + baseA;
                        Projectile p; p.fromPlayer = false; p.kind = 6; p.pos = e.pos;
                        p.vel = { cosf(a) * 305.0f, sinf(a) * 305.0f };
                        p.damage = bDmg * 0.85f; p.life = 3.0f; p.radius = 10.0f; p.color = { 255, 185, 65, 255 };
                        projectiles.push_back(p);
                    }
                }
            } else {
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, e.speed * speedMod * dt));
            }
        } else if (e.type == BOSS && e.bossKind == 1) {
            // Molten Colossus: đập đất (sóng + đạn lửa) và lao tới có báo hiệu
            if (!e.enraged && e.hp < e.maxHp * 0.5f) {
                e.enraged = true;
                ShowBanner("THE MOLTEN COLOSSUS ENRAGES!");
                Shake(0.4f, 8.0f);
                PlaySound(snd.sndSpecial);
            }
            float spd = e.speed * (e.enraged ? 1.3f : 1.0f) * speedMod;
            if (e.chargeState == 0) {
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, spd * 0.8f * dt));
                e.attackTimer += dt;
                if (e.attackTimer >= (e.enraged ? 2.2f : 3.2f)) {
                    e.attackTimer = 0;
                    e.summonTimer += 1.0f;
                    PlaySound(snd.sndShoot);
                    if (((int)e.summonTimer) % 3 == 0) { e.chargeState = 3; e.chargeTimer = 0.9f; e.chargeDir = dir; }
                    else { e.chargeState = 1; e.chargeTimer = 0.9f; }
                }
            } else if (e.chargeState == 1) {
                e.chargeTimer -= dt;
                if (e.chargeTimer <= 0) {
                    const float R = 190.0f;
                    PlaySound(snd.sndSpecial);
                    Shake(0.35f, 8.0f);
                    NovaVisual nv;
                    nv.pos = e.pos; nv.radius = 0; nv.maxRadius = R; nv.timer = 0.4f; nv.maxTimer = 0.4f; nv.color = ORANGE;
                    novas.push_back(nv);
                    Burst(e.pos, { 255, 150, 50, 255 }, 36, 360.0f, 0.7f);
                    if (Vector2Distance(e.pos, player.pos) <= R) DamagePlayer(e.contactDmg * 1.3f);
                    int n = e.enraged ? 14 : 10;
                    for (int i = 0; i < n; ++i) {
                        float a = i * (2.0f * PI / n);
                        Projectile p;
                        p.pos = e.pos;
                        float pSpd = 230.0f * ((!inSurface && weatherKind == 3) ? 0.88f : 1.0f);
                        p.vel = { cosf(a) * pSpd, sinf(a) * pSpd };
                        p.damage = e.contactDmg * 0.6f;
                        p.fromPlayer = false;
                        p.life = 3.0f;
                        p.radius = 8.0f;
                        p.color = { 255, 120, 40, 255 };
                        projectiles.push_back(p);
                    }
                    e.chargeState = 5;
                    e.chargeTimer = 0.7f;
                }
            } else if (e.chargeState == 3) {
                e.chargeTimer -= dt;
                if (e.chargeTimer > 0.25f) e.chargeDir = dir;
                if (e.chargeTimer <= 0) { e.chargeState = 4; e.chargeTimer = 0.65f; PlaySound(snd.sndDash); }
            } else if (e.chargeState == 4) {
                e.chargeTimer -= dt;
                e.pos = Vector2Add(e.pos, Vector2Scale(e.chargeDir, 600.0f * speedMod * dt));
                if (Rand01() < 0.6f) Burst(Vector2Subtract(e.pos, Vector2Scale(e.chargeDir, e.radius)), { 255, 140, 50, 255 }, 1, 80.0f, 0.4f);
                float bx = roomW / 2.0f - e.radius - 6.0f, by = roomH / 2.0f - e.radius - 6.0f;
                bool wall = fabsf(e.pos.x) > bx || fabsf(e.pos.y) > by;
                e.pos.x = Clamp(e.pos.x, -bx, bx);
                e.pos.y = Clamp(e.pos.y, -by, by);
                if (wall) { Shake(0.3f, 7.0f); Burst(e.pos, { 200, 160, 120, 255 }, 20, 260.0f, 0.5f); e.chargeTimer = 0; }
                if (e.chargeTimer <= 0) { e.chargeState = 5; e.chargeTimer = wall ? 1.4f : 0.7f; }
            } else {
                e.chargeTimer -= dt;
                if (e.chargeTimer <= 0) e.chargeState = 0;
            }
        } else if (e.type == BRUTE) {
            e.attackTimer += dt;
            if (dist < 110.0f && e.attackTimer >= 3.5f) {
                e.attackTimer = 0;
                PlaySound(snd.sndSpecial);
                Shake(0.2f, 5.0f);
                NovaVisual bv;
                bv.pos = e.pos;
                bv.radius = 0;
                bv.maxRadius = 140.0f;
                bv.timer = 0.3f;
                bv.maxTimer = 0.3f;
                bv.color = PURPLE;
                novas.push_back(bv);

                if (Vector2Distance(e.pos, player.pos) <= bv.maxRadius) {
                    DamagePlayer(e.contactDmg * 1.4f);
                }
            } else {
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, e.speed * speedMod * dt));
            }
        } else if (e.type == CHARGER) {
            if (e.chargeState == 0) {
                e.chargeCooldown -= dt;
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, e.speed * speedMod * dt));
                if (e.chargeCooldown <= 0 && dist < 440.0f && dist > 90.0f) {
                    e.chargeState = 1;
                    e.chargeTimer = 0.75f;
                    e.chargeDir = dir;
                    PlaySound(snd.sndShoot);
                }
            } else if (e.chargeState == 1) {
                // Báo hiệu: đứng yên, vẫn ngắm theo người chơi, khoá hướng ở 0.25s cuối
                e.chargeTimer -= dt;
                if (e.chargeTimer > 0.25f) e.chargeDir = dir;
                if (e.chargeTimer <= 0) {
                    e.chargeState = 2;
                    e.chargeTimer = 0.55f;
                    PlaySound(snd.sndDash);
                }
            } else if (e.chargeState == 2) {
                e.chargeTimer -= dt;
                e.pos = Vector2Add(e.pos, Vector2Scale(e.chargeDir, 560.0f * speedMod * dt));
                if (Rand01() < 0.5f) Burst(Vector2Subtract(e.pos, Vector2Scale(e.chargeDir, e.radius)), { 230, 200, 150, 255 }, 1, 60.0f, 0.3f);
                float bx = roomW / 2.0f - e.radius - 6.0f, by = roomH / 2.0f - e.radius - 6.0f;
                bool wall = fabsf(e.pos.x) > bx || fabsf(e.pos.y) > by;
                e.pos.x = Clamp(e.pos.x, -bx, bx);
                e.pos.y = Clamp(e.pos.y, -by, by);
                if (wall) {
                    Shake(0.18f, 5.0f);
                    Burst(e.pos, { 200, 190, 170, 255 }, 12, 200.0f, 0.4f);
                    e.chargeTimer = 0;
                }
                if (e.chargeTimer <= 0) {
                    e.chargeState = 3;
                    e.chargeTimer = wall ? 1.3f : 0.8f; // đâm tường thì choáng lâu hơn
                }
            } else {
                e.chargeTimer -= dt;
                if (e.chargeTimer <= 0) {
                    e.chargeState = 0;
                    e.chargeCooldown = 2.0f + 1.2f * Rand01();
                }
            }
        } else if (e.type == BOMBER) {
            if (!e.fusing) {
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, e.speed * speedMod * dt));
                if (dist < 78.0f) {
                    e.fusing = true;
                    e.fuseTimer = 0.0f;
                    PlaySound(snd.sndShoot);
                }
            } else {
                e.fuseTimer += dt;
                if (e.fuseTimer >= 0.75f) {
                    BomberExplode(e);
                    continue;
                }
            }
        } else if (e.type == BOSS && e.bossKind == 2) {
            // Băng Long Đế Quân: Phun quạt Thương Băng & Bão Tuyết Tuyệt Đối
            if (!e.enraged && e.hp <= e.maxHp * 0.5f) {
                e.enraged = true;
                e.speed *= 1.28f;
                PlaySound(snd.sndFreeze);
                Shake(0.4f, 8.0f);
                ShowBanner(L("THE FROST DRAGON ARCHON ENRAGES!", "BĂNG LONG ĐẾ QUÂN CUỒNG NỘ!"));
            }
            e.attackTimer += dt;
            float interval = e.enraged ? 1.55f : 2.15f;
            if (e.attackTimer >= interval) {
                e.attackTimer = 0.0f;
                e.summonTimer += 1.0f;
                PlaySound(snd.sndShoot);
                float baseA = atan2f(dir.y, dir.x);
                int fanCount = e.enraged ? 7 : 5;
                float pSpd = 295.0f * ((!inSurface && weatherKind == 3) ? 0.88f : 1.0f);
                for (int fi = 0; fi < fanCount; ++fi) {
                    float a = baseA + (fi - (fanCount - 1) * 0.5f) * 0.16f;
                    Projectile p;
                    p.pos = Vector2Add(e.pos, { cosf(a) * 34.0f, sinf(a) * 34.0f });
                    p.vel = { cosf(a) * pSpd, sinf(a) * pSpd };
                    p.damage = e.contactDmg * 0.72f;
                    p.fromPlayer = false;
                    p.applySlow = true;
                    p.life = 3.2f;
                    p.radius = 8.0f;
                    p.color = SKYBLUE;
                    projectiles.push_back(p);
                }
                if (((int)e.summonTimer) % 2 == 0) {
                    const float blizR = 165.0f;
                    NovaVisual nv; nv.pos = e.pos; nv.maxRadius = blizR; nv.timer = 0.42f; nv.maxTimer = 0.42f; nv.color = SKYBLUE;
                    novas.push_back(nv);
                    Burst(e.pos, { 165, 235, 255, 255 }, 28, 300.0f, 0.6f);
                    PlaySound(snd.sndFreeze);
                    if (dist <= blizR) {
                        DamagePlayer(e.contactDmg * 0.85f);
                        player.frostSlowTimer = std::max(player.frostSlowTimer, 1.5f);
                    }
                }
            } else {
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, e.speed * speedMod * dt));
            }
        } else if (e.type == BOSS) {
            if (!e.enraged && e.hp <= e.maxHp * 0.5f) {
                e.enraged = true;
                e.speed *= 1.3f;
                PlaySound(snd.sndSpecial);
                Shake(0.4f, 8.0f);
                ShowBanner("THE VOID SOVEREIGN ENRAGES!");
            }

            e.attackTimer += dt;
            float interval = e.enraged ? 1.65f : 2.35f;
            if (e.attackTimer >= interval) {
                e.attackTimer = 0;
                PlaySound(snd.sndShoot);
                // 12 or 16 way radial bullet burst
                int count = e.enraged ? 16 : 12;
                float step = (2.0f * PI) / count;
                for (int i = 0; i < count; ++i) {
                    float a = i * step;
                    Vector2 bDir = { cosf(a), sinf(a) };
                    Projectile p;
                    p.pos = Vector2Add(e.pos, Vector2Scale(bDir, 35.0f));
                    p.vel = Vector2Scale(bDir, 265.0f * ((!inSurface && weatherKind == 3) ? 0.88f : 1.0f));
                    p.damage = e.contactDmg * 0.72f;
                    p.fromPlayer = false;
                    p.life = 3.5f;
                    p.radius = 8.0f;
                    p.color = GOLD;
                    projectiles.push_back(p);
                }
            } else {
                e.pos = Vector2Add(e.pos, Vector2Scale(dir, e.speed * speedMod * dt));
            }
        }

        // Contact damage with player
        // Giữ quái trong phòng: quái bắn xa lùi ra ngoài sẽ không thể bị hạ, khiến phòng không bao giờ được dọn
        e.pos.x = Clamp(e.pos.x, -(roomW / 2.0f - e.radius - 6.0f), roomW / 2.0f - e.radius - 6.0f);
        e.pos.y = Clamp(e.pos.y, -(roomH / 2.0f - e.radius - 6.0f), roomH / 2.0f - e.radius - 6.0f);
        bool hitPillar = ResolvePillars(e.pos, e.radius * 0.85f);
        if (hitPillar && e.type == BOSS && e.bossKind == 1 && e.chargeState == 4) {
            e.chargeState = 5;
            e.chargeTimer = 1.4f;
            Shake(0.3f, 7.0f);
            Burst(e.pos, { 200, 160, 120, 255 }, 20, 260.0f, 0.5f);
        }
        if (hitPillar && e.type == CHARGER && e.chargeState == 2) { // đâm vào cột: choáng
            e.chargeState = 3;
            e.chargeTimer = 1.2f;
            Shake(0.15f, 4.0f);
            Burst(e.pos, { 200, 190, 170, 255 }, 10, 180.0f, 0.4f);
        }
        if (e.stunTimer <= 0.0f && Vector2Distance(e.pos, player.pos) < e.radius + 18.0f) {
            float contactMul = ((e.type == CHARGER && e.chargeState == 2) || (e.type == BOSS && e.bossKind == 1 && e.chargeState == 4)) ? 1.8f : 1.0f;
            if (DamagePlayer(e.contactDmg * contactMul) && e.isElite && e.eliteAffix == 1 && e.hp > 0) {
                float vh = std::min(e.maxHp - e.hp, e.maxHp * 0.12f);
                e.hp += vh;
                if (vh >= 1.0f) SpawnPopup(e.pos, "+VAMP", RED);
            }
        }
    }

    // ── Con cái theo cha rèn luyện, Quân Đồng Minh & Linh Thú Hộ Vệ ──
    UpdatePartyChildren(dt);
    UpdateAlliedUnits(dt);
    UpdateFamiliar(dt);

    // ── Projectiles ──
    FlushSpawns();
    for (size_t i = 0; i < projectiles.size();) {
        auto& p = projectiles[i];
        if (p.fromPlayer && p.kind == 5) { // tên ma thuật dẫn đường
            float bestD = 520.0f;
            Vector2 bestPos = p.pos;
            bool found = false;
            for (const auto& e : enemies) {
                if (e.hp <= 0) continue;
                float d = Vector2Distance(p.pos, e.pos);
                if (d < bestD) { bestD = d; bestPos = e.pos; found = true; }
            }
            if (found) {
                Vector2 desired = Vector2Scale(Vector2Normalize(Vector2Subtract(bestPos, p.pos)), 440.0f);
                p.vel = Vector2Lerp(p.vel, desired, std::min(1.0f, dt * 8.5f));
                if (Vector2Length(p.vel) > 1.0f) p.vel = Vector2Scale(Vector2Normalize(p.vel), 440.0f);
            }
        }
        p.pos = Vector2Add(p.pos, Vector2Scale(p.vel, dt));
        p.life -= dt;

        if (p.fromPlayer && Rand01() < (p.kind == 4 ? 0.85f : 0.35f)) {
            Burst(p.pos, p.color, 1, p.kind == 4 ? 65.0f : 35.0f, p.kind == 4 ? 0.28f : 0.18f);
        }

        bool hit = false;
        bool outOrExpired = false;
        if (!p.fromPlayer) {
            if (Vector2Distance(p.pos, player.pos) < p.radius + 18.0f) {
                float bulletScale = (activeRune == 4) ? (1.0f - 0.08f * runeRank) : 1.0f;
                if (DamagePlayer(p.damage * bulletScale) && p.applySlow) {
                    player.frostSlowTimer = std::max(player.frostSlowTimer, 1.0f);
                }
                hit = true;
            }
            for (size_t ck = 0; !hit && ck < children.size(); ++ck) { // đạn quái trúng con
                if (!children[ck].active) continue;
                if (Vector2Distance(p.pos, children[ck].pos) < p.radius + 13.0f) {
                    ChildTakeDamage((int)ck, p.damage);
                    hit = true;
                }
            }
            if (!hit) { // cột đá che chắn đạn của quái
                for (const auto& pl : pillars) {
                    if (Vector2Distance(p.pos, pl) < 27.0f + p.radius) { hit = true; Burst(p.pos, p.color, 3, 100.0f, 0.25f); break; }
                }
            }
        } else {
            for (size_t bi = 0; bi < barrels.size(); ++bi) {
                ArenaBarrel& b = barrels[bi];
                if (b.destroyed) continue;
                if (Vector2Distance(p.pos, b.pos) < p.radius + 22.0f) {
                    DamageBarrel(b, p.damage);
                    if (p.splashRadius > 0.0f) {
                        TriggerProjectileSplash(p.pos, p.damage * p.splashRatio, p.splashRadius, p.color, p.applyBurn, -1);
                    }
                    if (p.pierce > 0) p.pierce--;
                    else { hit = true; break; }
                }
            }
            for (size_t ei = 0; !hit && ei < enemies.size(); ++ei) {
                Enemy& e = enemies[ei];
                if (e.hp <= 0 || (int)ei == p.hitMask) continue;
                if (Vector2Distance(p.pos, e.pos) < p.radius + e.radius) {
                    if (p.childOwner >= 0) { // đạn của con: sát thương riêng, không kích hoạt hiệu ứng của cha
                        Vector2 ckb = Vector2Length(p.vel) > 0.1f ? Vector2Normalize(p.vel) : Vector2{ 1, 0 };
                        ChildDamageEnemy(e, p.damage, p.childOwner, ckb);
                        hit = true;
                        break;
                    }
                    float cc = std::min(0.85f, player.critChance + p.critBonus);
                    bool isCrit = p.hasPreCrit ? p.preCrit : (Rand01() < cc);
                    float critMul = CritMult();
                    float dmg = p.damage * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Length(p.vel) > 0.1f ? Vector2Normalize(p.vel) : Vector2{ 1, 0 };
                    DamageEnemy(e, dmg, kb, p.kind == 3 ? 110.0f : (p.kind == 6 ? 190.0f : 150.0f), isCrit, true);
                    if (p.applyBurn) {
                        e.burnTimer = std::max(e.burnTimer, 3.0f);
                        e.burnDps = std::max(e.burnDps, player.baseDmg * (0.4f + 0.5f * player.igniteLevel));
                    }
                    if (p.applySlow) {
                        e.slowTimer = std::max(e.slowTimer, 2.0f);
                        e.slowFactor = std::min(e.slowFactor, 0.65f);
                    }
                    if (p.applyPoison) {
                        e.poisonTimer = std::max(e.poisonTimer, 2.5f);
                        e.poisonDmg = std::max(e.poisonDmg, dmg * 0.22f);
                    }
                    if (p.applyStun && e.hp > 0) {
                        e.stunTimer = std::max(e.stunTimer, 1.2f);
                    }
                    if (player.lifesteal > 0.0f && player.hp < player.maxHp) {
                        float rawHl = dmg * std::min(LifestealCap() + 0.05f, player.lifesteal);
                        float hl = (settings.difficulty == 0) ? std::max(1.0f, rawHl) : rawHl;
                        player.hp = std::min(player.maxHp, player.hp + hl);
                    }
                    if (p.splashRadius > 0.0f) {
                        TriggerProjectileSplash(p.pos, p.damage * p.splashRatio, p.splashRadius, p.color, p.applyBurn, (int)ei);
                    }
                    if (p.pierce > 0) {
                        p.pierce--;
                        p.hitMask = (int)ei;
                    } else {
                        hit = true;
                        break;
                    }
                }
            }
            if (!hit && p.kind != 5) {
                for (const auto& pl : pillars) {
                    if (Vector2Distance(p.pos, pl) < 27.0f + p.radius) {
                        hit = true;
                        Burst(p.pos, p.color, 5, 130.0f, 0.3f);
                        if (p.kind == 4) TriggerProjectileSplash(p.pos, p.damage, p.splashRadius, p.color, true, -1);
                        break;
                    }
                }
            }
        }
        outOrExpired = (p.life <= 0 || fabs(p.pos.x) > roomW / 2.0f || fabs(p.pos.y) > roomH / 2.0f);
        if (!hit && outOrExpired && p.fromPlayer && p.kind == 4) {
            TriggerProjectileSplash(p.pos, p.damage, p.splashRadius, p.color, true, -1);
        }
        if (hit || outOrExpired) {
            projectiles.erase(projectiles.begin() + i);
        } else {
            ++i;
        }
    }

    // ── Loot collection ──
    for (size_t i = 0; i < loots.size();) {
        auto& l = loots[i];
        float d = Vector2Distance(l.pos, player.pos);
        if (d < (HasRelic(RELIC_MIDAS) ? 260.0f : 160.0f)) {
            Vector2 pull = Vector2Normalize(Vector2Subtract(player.pos, l.pos));
            l.pos = Vector2Add(l.pos, Vector2Scale(pull, 420.0f * dt));
        }
        if (d < 24.0f) {
            if (l.type == 0) { // XP (tối đa Cấp 100)
                if (player.level >= kMaxPlayerLevel) {
                    player.level = kMaxPlayerLevel;
                    player.xp = player.xpNext;
                    player.hp = std::min(player.maxHp, player.hp + 2.0f);
                    Burst(l.pos, GOLD, 3, 100.0f, 0.3f);
                    PlaySound(snd.sndCoin);
                } else {
                    player.xp += std::max(1, (int)roundf(l.value * xpMul * (1.0f + 0.06f * styleRank)));
                    Burst(l.pos, SKYBLUE, 3, 100.0f, 0.3f);
                    PlaySound(snd.sndCoin);
                    while (player.level < kMaxPlayerLevel && player.xp >= player.xpNext) {
                        player.xp -= player.xpNext;
                        player.level++;
                        player.xpNext = XpRequiredForNextLevel(player.level);
                        CheckAndGrantLevelTitleMilestones();
                        TriggerLevelUp();
                    }
                    if (player.level >= kMaxPlayerLevel) {
                        player.level = kMaxPlayerLevel;
                        player.xp = player.xpNext;
                    }
                }
            } else if (l.type == 1) { // Gold
                int gVal = HasRelic(RELIC_MIDAS) ? (int)roundf(l.value * 1.5f) : l.value;
                float gMult = (isNoble ? 1.25f : 1.0f) * (1.0f + 0.06f * styleRank) * (bloodlineTrait == 3 ? 1.15f : 1.0f) * ((!inSurface && weatherKind == 1) ? 1.18f : 1.0f);
                gVal = (int)roundf(gVal * gMult);
                player.gold += gVal;
                Burst(l.pos, GOLD, 4, 120.0f, 0.35f);
                PlaySound(snd.sndCoin);
            } else if (l.type == 2) { // Potion
                player.hp = std::min(player.maxHp, player.hp + l.value);
                SpawnPopup(player.pos, "+" + std::to_string(l.value), GREEN);
                Burst(l.pos, { 255, 100, 128, 255 }, 8, 150.0f, 0.45f);
                PlaySound(snd.sndPotion);
            }
            loots.erase(loots.begin() + i);
        } else {
            ++i;
        }
    }

    // ── Visual Animations ──
    for (size_t i = 0; i < popups.size();) {
        popups[i].pos.y -= 45.0f * dt;
        popups[i].timer -= dt;
        if (popups[i].timer <= 0) popups.erase(popups.begin() + i);
        else ++i;
    }
    for (size_t i = 0; i < slashes.size();) {
        slashes[i].timer -= dt;
        if (slashes[i].timer <= 0) slashes.erase(slashes.begin() + i);
        else ++i;
    }
    for (size_t i = 0; i < novas.size();) {
        float t = 1.0f - (novas[i].timer / novas[i].maxTimer);
        novas[i].radius = sqrtf(t) * novas[i].maxRadius;
        novas[i].timer -= dt;
        if (novas[i].timer <= 0) novas.erase(novas.begin() + i);
        else ++i;
    }
    for (size_t i = 0; i < ghosts.size();) {
        ghosts[i].timer -= dt;
        if (ghosts[i].timer <= 0) ghosts.erase(ghosts.begin() + i);
        else ++i;
    }

    ResolvePillars(player.pos, 15.0f);
    UpdateChests(dt);
    UpdateItemDrops(dt);

    // ── Thùng nổ / Tinh thể chiến thuật ──
    for (size_t i = 0; i < barrels.size();) {
        if (barrels[i].hitFlash > 0.0f) barrels[i].hitFlash -= dt;
        if (barrels[i].destroyed) { barrels.erase(barrels.begin() + i); continue; }
        ++i;
    }

    // ── Bẫy địa hình & Vòng năng lượng Leyline ──
    for (auto& h : hazards) {
        h.timer = fmodf(h.timer + dt, h.cycle);
        if (h.tickTimer > 0.0f) h.tickTimer -= dt;
        if (h.kind == 3) {
            if (Vector2Distance(player.pos, h.pos) < h.radius) {
                AddRage(4.0f * dt);
            }
        } else if (h.timer > h.cycle * 0.68f && h.tickTimer <= 0.0f) {
            h.tickTimer = 0.45f;
            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(h.pos, e.pos) <= h.radius + e.radius * 0.35f) {
                    float hdmg = 18.0f + floor * 4.0f;
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, h.pos));
                    DamageEnemy(e, hdmg, kb, 80.0f, false, false);
                    if (e.hp > 0) {
                        if (h.kind == 1) { e.poisonTimer = std::max(e.poisonTimer, 2.5f); e.poisonDmg = std::max(e.poisonDmg, 10.0f + floor * 2.5f); }
                        if (h.kind == 2) { e.burnTimer = std::max(e.burnTimer, 2.5f); e.burnDps = std::max(e.burnDps, 12.0f + floor * 3.0f); }
                    }
                }
            }
            if (Vector2Distance(player.pos, h.pos) < h.radius - 8.0f && !player.isDashing) {
                DamagePlayer(8.0f);
            }
        }
    }

    // ── Cột Thử Thách (Trial Totem) ──
    if (hasTrialTotem && !trialUsed) {
        float td = Vector2Distance(player.pos, trialPos);
        if (td < 95.0f && (IsKeyPressed(KEY_F) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP))) {
            trialUsed = true;
            trialActive = true;
            PlaySound(snd.sndSpecial);
            Shake(0.3f, 7.0f);
            ShowBanner("TRIAL ACTIVATED! DEFEAT THE ELITE AMBUSH!", 3.0f);
            static const EnemyType ambTypes[3] = { CHASER, RANGER, BRUTE };
            for (int i = 0; i < 3; ++i) {
                float a = i * (2.0f * PI / 3.0f);
                Vector2 sp = { Clamp(trialPos.x + cosf(a) * 140.0f, -(roomW / 2.0f - 60.0f), roomW / 2.0f - 60.0f),
                               Clamp(trialPos.y + sinf(a) * 120.0f, -(roomH / 2.0f - 60.0f), roomH / 2.0f - 60.0f) };
                Enemy amb = BuildEnemy(ambTypes[i], sp);
                amb.isElite = true;
                amb.eliteAffix = 1 + (i % 4);
                amb.affixTimer = 1.8f;
                amb.maxHp *= 1.35f;
                amb.hp = amb.maxHp;
                enemies.push_back(amb);
                NovaVisual nv; nv.pos = sp; nv.maxRadius = 65.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = GOLD;
                novas.push_back(nv);
            }
        }
    }

    // ── Thương nhân ──
    if (shopFloor) {
        float md = Vector2Distance(player.pos, merchantPos);
        if (!shopHinted && md < 240.0f) {
            shopHinted = true;
            ShowBanner("Merchant: press F to trade", 2.5f);
        }
        if (md < 130.0f && (IsKeyPressed(KEY_F) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP))) {
            state = SHOP;
            shopSel = 0;
            PlaySound(snd.sndPotion);
            return;
        }
    }

    // ── Bàn thờ máu ──
    if (hasShrine && !shrineUsed) {
        shrinePulse += dt * 3.0f;
        if (shrineRefuse > 0) shrineRefuse -= dt;
        float sd = Vector2Distance(player.pos, shrinePos);
        if (!shrineHinted && sd < 160.0f) {
            shrineHinted = true;
            ShowBanner("Blood Shrine - touch it to trade HP for a free upgrade", 3.0f);
        }
        if (sd < 44.0f && shrineRefuse <= 0) UseShrine();
    }

    // ── Đợt quái 2, Hoàn thành Trial & Kiểm tra Dọn Phòng ──
    int aliveCount = 0;
    for (const auto& e : enemies) {
        if (e.hp > 0) aliveCount++;
    }
    if (!wave2Spawns.empty() && !wave2Triggered && aliveCount <= 2) {
        wave2Triggered = true;
        PlaySound(snd.sndSpecial);
        Shake(0.22f, 5.0f);
        ShowBanner("WAVE 2 REINFORCEMENTS!", 2.5f);
        for (const auto& w : wave2Spawns) {
            enemies.push_back(w);
            NovaVisual nv; nv.pos = w.pos; nv.maxRadius = 60.0f; nv.timer = 0.32f; nv.maxTimer = 0.32f; nv.color = ORANGE;
            novas.push_back(nv);
            aliveCount++;
        }
        wave2Spawns.clear();
    }
    if (trialActive && aliveCount == 0 && pendingSpawns.empty()) {
        trialActive = false;
        PlaySound(snd.sndLevelUp);
        ShowBanner("TRIAL CONQUERED! VAULT CHEST UNLOCKED!", 3.0f);
        Chest vc; vc.pos = trialPos; vc.vault = true;
        chests.push_back(vc);
        player.gold += isNoble ? 44 : 35;
        AddRage(50.0f);
        UnlockSpouse(2, "TRIAL CONQUERED: PRIESTESS THIEN THANH WILL NOW MARRY YOU!",
                        "CHIẾN THẮNG THỬ THÁCH: THÁNH NỮ THIÊN THANH ĐÃ ĐỒNG Ý KẾT DUYÊN!");
    }

    // Giải cứu người đẹp bị giam cầm trong Hầm Ngục
    if (hasCaptiveEvent && !captiveRescued) {
        float cd = Vector2Distance(player.pos, captivePos);
        if ((cd < 92.0f && (IsKeyPressed(KEY_F) || IsKeyPressed(KEY_ENTER) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP)))
            || (portalOpen && cd < 64.0f)) {
            RescueCaptiveSpouse();
        }
    }

    // Đài Giao Ước Bóng Tối
    if (hasBloodPact && !bloodPactUsed) {
        float bd = Vector2Distance(player.pos, bloodPactPos);
        if (bd < 86.0f && (IsKeyPressed(KEY_F) || IsKeyPressed(KEY_G) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP))) {
            AcceptBloodPact();
        }
    }

    // Đài Rèn Cổ Ngữ (Ancient Runic Anvil)
    if (hasRunicAnvil && !runicAnvilUsed) {
        float ad = Vector2Distance(player.pos, runicAnvilPos);
        if (ad < 90.0f && (IsKeyPressed(KEY_F) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP))) {
            UseRunicAnvil();
        }
    }

    // Sét Thiên Lôi định kỳ khi thời tiết Bão Từ Ma Pháp
    if (!inSurface && weatherKind == 2 && aliveCount > 0) {
        weatherTimer -= dt;
        if (weatherTimer <= 0.0f) {
            weatherTimer = 6.5f + Rand01() * 2.5f;
            Vector2 strikePos = player.pos;
            std::vector<Vector2> livePos;
            for (const auto& en : enemies) if (en.hp > 0) livePos.push_back(en.pos);
            if (!livePos.empty() && Rand01() < 0.78f) strikePos = livePos[rand() % livePos.size()];
            strikePos.x = Clamp(strikePos.x, -(roomW / 2.0f - 75.0f), roomW / 2.0f - 75.0f);
            strikePos.y = Clamp(strikePos.y, -(roomH / 2.0f - 75.0f), roomH / 2.0f - 75.0f);
            GroundEffect bolt;
            bolt.kind = 6;
            bolt.pos = strikePos;
            bolt.radius = 68.0f;
            bolt.maxTimer = 0.85f;
            bolt.timer = 0.85f;
            bolt.damage = 38.0f + floor * 9.0f;
            bolt.color = { 115, 230, 255, 255 };
            groundEffects.push_back(bolt);
        }
    }

    // Theo dõi độ trễ thanh máu Boss
    {
        bool foundBoss = false;
        for (const auto& en : enemies) {
            if (en.type == BOSS && en.hp > 0) {
                foundBoss = true;
                float pct = Clamp(en.hp / std::max(1.0f, en.maxHp), 0.0f, 1.0f);
                if (bossHpLag > pct) bossHpLag = std::max(pct, bossHpLag - dt * 0.45f);
                else bossHpLag = pct;
                break;
            }
        }
        if (!foundBoss) bossHpLag = 1.0f;
    }

    bool anyAlive = (aliveCount > 0) || !pendingSpawns.empty() || !wave2Spawns.empty();
    if (!anyAlive && !portalOpen) {
        portalOpen = true;
        BuildPortalChoices();
        PlaySound(snd.sndPortal);
        ShowBanner(portalChoices.size() > 1 ? "ROOM CLEARED! CHOOSE YOUR NEXT PORTAL!" : "ROOM CLEARED! ENTER THE PORTAL!");
    }

    // Teleport to next floor (hỗ trợ 3 cổng rẽ nhánh + Cổng Mặt Đất sau Tầng 10)
    if (portalOpen) {
        for (const auto& pc : portalChoices) {
            if (pc.nextMod == -99) {
                if (Vector2Distance(player.pos, pc.pos) < 75.0f && (IsKeyPressed(KEY_F) || IsKeyPressed(KEY_G) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP))) {
                    AdvanceTime(1.5f);
                    EnterSurface(floor + 1);
                    break;
                }
                continue;
            }
            if (Vector2Distance(player.pos, pc.pos) < 46.0f) {
                forcedNextMod = pc.nextMod;
                forcedNextReward = pc.rewardKind;
                floor++;
                AdvanceTime(1.5f);
                if (isNoble) {
                    player.gold += 30;
                    SpawnPopup(player.pos, L("+30G NOBLE", "+30V QUÝ TỘC"), GOLD);
                }
                PlaySound(snd.sndPortal);
                BuildFloor();
                break;
            }
        }
    }

    // Camera Follow with Screen Shake
    camera.target = player.pos;
    if (screenShakeTimer > 0) {
        camera.target.x += ((float)rand() / RAND_MAX * 2.0f - 1.0f) * screenShakeMag;
        camera.target.y += ((float)rand() / RAND_MAX * 2.0f - 1.0f) * screenShakeMag;
    }
}

inline void Game::DamageBarrel(ArenaBarrel& b, float dmg) {
    if (b.destroyed) return;
    b.hp -= dmg;
    b.hitFlash = 0.08f;
    Burst(b.pos, { 220, 210, 190, 255 }, 4, 120.0f, 0.25f);
    if (b.hp > 0.0f) return;

    b.destroyed = true;
    PlaySound(snd.sndSpecial);
    Shake(0.22f, 6.5f);

    if (b.kind == 0) { // Explosive Keg
        const float rad = 145.0f;
        NovaVisual nv; nv.pos = b.pos; nv.maxRadius = rad; nv.timer = 0.34f; nv.maxTimer = 0.34f; nv.color = ORANGE;
        novas.push_back(nv);
        Burst(b.pos, ORANGE, 32, 340.0f, 0.6f);
        Burst(b.pos, YELLOW, 16, 220.0f, 0.45f);
        float dmgAoe = 45.0f + floor * 12.0f;
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(b.pos, e.pos) <= rad + e.radius * 0.4f) {
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, b.pos));
                DamageEnemy(e, dmgAoe, kb, 260.0f, false, false);
                if (e.hp > 0) { e.burnTimer = std::max(e.burnTimer, 3.0f); e.burnDps = std::max(e.burnDps, 12.0f + floor * 3.0f); }
            }
        }
        if (Vector2Distance(b.pos, player.pos) <= 110.0f) DamagePlayer(15.0f);
        TriggerBarrelsInRadius(b.pos, rad, 20.0f);
    } else if (b.kind == 1) { // Cryo Crystal
        const float rad = 155.0f;
        PlaySound(snd.sndFreeze);
        NovaVisual nv; nv.pos = b.pos; nv.maxRadius = rad; nv.timer = 0.36f; nv.maxTimer = 0.36f; nv.color = SKYBLUE;
        novas.push_back(nv);
        Burst(b.pos, SKYBLUE, 30, 320.0f, 0.6f);
        float dmgAoe = 30.0f + floor * 8.0f;
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(b.pos, e.pos) <= rad + e.radius * 0.4f) {
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, b.pos));
                DamageEnemy(e, dmgAoe, kb, 160.0f, false, false);
                if (e.hp > 0) { e.stunTimer = std::max(e.stunTimer, 1.6f); e.chillTimer = std::max(e.chillTimer, 3.5f); }
            }
        }
        TriggerBarrelsInRadius(b.pos, rad, 20.0f);
    } else { // Shock Conduit
        const float rad = 175.0f;
        NovaVisual nv; nv.pos = b.pos; nv.maxRadius = rad; nv.timer = 0.32f; nv.maxTimer = 0.32f; nv.color = GOLD;
        novas.push_back(nv);
        Burst(b.pos, GOLD, 26, 300.0f, 0.5f);
        AddRage(20.0f);
        SpawnPopup(b.pos, "+20 RAGE", GOLD);
        int hits = 0;
        float dmgAoe = 38.0f + floor * 10.0f;
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;
            if (Vector2Distance(b.pos, e.pos) <= 190.0f + e.radius * 0.4f) {
                Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, b.pos));
                DamageEnemy(e, dmgAoe, kb, 140.0f, false, false);
                if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 0.8f);
                for (int k = 1; k <= 4; ++k) Burst(Vector2Lerp(b.pos, e.pos, k * 0.2f), SKYBLUE, 1, 50.0f, 0.22f);
                if (++hits >= 5) break;
            }
        }
        TriggerBarrelsInRadius(b.pos, rad, 20.0f);
    }
}

inline void Game::TriggerBarrelsInRadius(Vector2 center, float radius, float dmg) {
    for (size_t i = 0; i < barrels.size(); ++i) {
        if (barrels[i].destroyed) continue;
        if (Vector2Distance(center, barrels[i].pos) <= radius + 22.0f) {
            DamageBarrel(barrels[i], dmg);
        }
    }
}

inline void Game::TriggerProjectileSplash(Vector2 pos, float baseSplashDmg, float radius, Color col, bool applyBurn, int skipIdx) {
    NovaVisual nv;
    nv.pos = pos;
    nv.radius = 0.0f;
    nv.maxRadius = radius;
    nv.timer = 0.28f;
    nv.maxTimer = 0.28f;
    nv.color = col;
    novas.push_back(nv);
    Burst(pos, col, radius > 90.0f ? 26 : 12, radius * 2.2f, 0.45f);
    if (radius > 90.0f) {
        PlaySound(snd.sndSpecial);
        Shake(0.18f, 5.0f);
    }
    TriggerBarrelsInRadius(pos, radius, baseSplashDmg);
    for (size_t i = 0; i < enemies.size(); ++i) {
        if ((int)i == skipIdx) continue;
        Enemy& other = enemies[i];
        if (other.hp <= 0) continue;
        if (Vector2Distance(pos, other.pos) <= radius + other.radius * 0.5f) {
            bool isCrit = (Rand01() < player.critChance);
            float critMul = (player.heroClass == CLASS_ASSASSIN ? 2.4f : 2.0f);
            float dmg = baseSplashDmg * (isCrit ? critMul : 1.0f);
            Vector2 kb = Vector2Normalize(Vector2Subtract(other.pos, pos));
            DamageEnemy(other, dmg, kb, 160.0f, isCrit, true);
            if (applyBurn) {
                other.burnTimer = std::max(other.burnTimer, 3.0f);
                other.burnDps = std::max(other.burnDps, player.baseDmg * (0.4f + 0.5f * player.igniteLevel));
            }
        }
    }
}

inline void Game::DamageEnemy(Enemy& e, float dmg, Vector2 kb, float kbForce, bool isCrit, bool allowRage) {
    if (!isCrit && !inSurface && weatherKind == 1 && !e.isGoldenGoblin && Rand01() < 0.08f) {
        isCrit = true;
        dmg *= 1.5f;
    }
    if (nextHitCrit && !e.isGoldenGoblin) {
        nextHitCrit = false;
        if (!isCrit) { isCrit = true; dmg *= 1.65f; }
    }
    if (e.type == CHARGER && e.chargeState == 3) dmg *= 1.5f; // choáng sau cú lao: dễ trúng đòn
    if (e.type == BOSS && e.bossKind == 1 && e.chargeState == 5) dmg *= 1.3f; // Colossus kiệt sức: nhận thêm sát thương
    if (e.stunTimer > 0.0f) dmg *= 1.20f; // Kẻ địch đang choáng nhận thêm 20% sát thương
    if (e.staggerBrokenTimer > 0.0f) dmg *= 1.35f; // Phá Giáp: nhận thêm +35% sát thương
    if (overdriveTimer > 0.0f) dmg *= 1.18f; // Bùng Nổ Linh Hồn: +18% sát thương
    if (peerageRank > 0 || isKing) dmg *= (1.0f + peerageRank * 0.04f + (isKing ? 0.15f : 0.0f));
    if (HasRelic(RELIC_BERSERK) && player.hp < player.maxHp * 0.4f) dmg *= 1.4f;
    dmg *= HungerDamageMult();
    dmg *= AgeDamageMult();
    e.hp -= dmg;
    e.pos = Vector2Add(e.pos, Vector2Scale(kb, kbForce * 0.05f));
    SpawnPopup(e.pos, (isCrit ? "!" : "") + std::to_string((int)dmg), isCrit ? YELLOW : WHITE);
    e.hitFlash = 0.09f;
    Burst(e.pos, isCrit ? Color{ 255, 217, 51, 255 } : WHITE, isCrit ? 9 : 4, isCrit ? 230.0f : 160.0f, isCrit ? 0.4f : 0.28f);
    AddStyle(isCrit ? 7.0f : 3.5f);
    if (allowRage) AddRage((isCrit ? 4.0f : 2.5f) + (comboCount >= 10 ? 1.5f : 0.0f));

    // Nội tại Sát Thủ: đòn chí mạng gây chảy máu/độc 2s
    if (player.heroClass == CLASS_ASSASSIN && isCrit) {
        e.poisonTimer = std::max(e.poisonTimer, 2.0f);
        e.poisonDmg = std::max(e.poisonDmg, dmg * 0.18f);
    }

    // Ember Edge / Frost Touch & Elemental Reactions
    float dotMult = (HasDuoPlaguefire() ? 1.25f : 1.0f) * ((!inSurface && weatherKind == 4) ? 1.20f : 1.0f);
    float chillMult = (!inSurface && weatherKind == 3) ? 1.30f : 1.0f;
    if (player.igniteLevel > 0) {
        e.burnTimer = 3.0f;
        e.burnDps = std::max(e.burnDps, player.baseDmg * 0.5f * player.igniteLevel * dotMult);
        if (e.chillTimer > 0.0f || e.slowTimer > 0.0f) TriggerElementalReaction(e, 0, e.pos);
    }
    if (player.frostLevel > 0) {
        e.slowTimer = (2.0f + (HasDuoTempestFrost() ? 1.0f : 0.0f)) * chillMult;
        e.slowFactor = std::max(0.4f, 1.0f - 0.15f * player.frostLevel);
        if (e.burnTimer > 0.0f) TriggerElementalReaction(e, 0, e.pos);
        else if (isCrit) TriggerElementalReaction(e, 3, e.pos);
    }

    // Relics on hit
    if (HasRelic(RELIC_FROST) && isCrit) {
        e.chillTimer = (2.5f + (HasDuoTempestFrost() ? 1.0f : 0.0f)) * chillMult;
        PlaySound(snd.sndFreeze);
        if (e.burnTimer > 0.0f) TriggerElementalReaction(e, 0, e.pos);
        else TriggerElementalReaction(e, 3, e.pos);
    }
    if (HasRelic(RELIC_TOXIC)) {
        e.poisonTimer = 3.0f;
        e.poisonDmg = dmg * 0.25f * dotMult;
    }
    if (HasRelic(RELIC_LIGHTNING)) {
        if (e.burnTimer > 0.0f) TriggerElementalReaction(e, 1, e.pos);
        else if (e.poisonTimer > 0.0f) TriggerElementalReaction(e, 2, e.pos);
        // Zap another nearby enemy
        for (auto& other : enemies) {
            if (&other != &e && other.hp > 0 && Vector2Distance(e.pos, other.pos) < 180.0f) {
                other.hp -= dmg * 0.5f;
                SpawnPopup(other.pos, std::to_string((int)(dmg * 0.5f)), SKYBLUE);
                if (other.hp <= 0) OnEnemyKilled(other);
                break;
            }
        }
    }

    // Hiệu ứng Cổ Ngữ (Ancient Runes)
    if (activeRune == 1 && Rand01() < (0.25f + 0.05f * runeRank)) { // Cổ Ngữ Lôi Đình
        float chainDmg = player.baseDmg * (0.35f + 0.12f * runeRank);
        int maxHits = (runeRank >= 3) ? 2 : 1, hits = 0;
        for (auto& other : enemies) {
            if (&other != &e && other.hp > 0 && Vector2Distance(e.pos, other.pos) < 165.0f) {
                other.hp -= chainDmg;
                Burst(other.pos, { 110, 225, 255, 255 }, 8, 180.0f, 0.35f);
                SpawnPopup(other.pos, TextFormat("ZAP %d", (int)chainDmg), { 115, 225, 255, 255 });
                if (other.hp <= 0) OnEnemyKilled(other);
                if (++hits >= maxHits) break;
            }
        }
    } else if (activeRune == 2 && isCrit && e.hp > 0) { // Cổ Ngữ Băng Hỏa
        e.burnTimer = std::max(e.burnTimer, 1.8f);
        e.burnDps = std::max(e.burnDps, player.baseDmg * 0.35f * runeRank * dotMult);
        if (e.chillTimer > 0.0f || e.slowTimer > 0.0f) {
            TriggerElementalReaction(e, 0, e.pos);
        } else if (runeRank >= 2) {
            e.slowTimer = std::max(e.slowTimer, 1.5f * chillMult);
            e.slowFactor = std::min(e.slowFactor, 0.72f);
        }
    }

    // Cơ Chế Phá Giáp (Stagger / Posture Break) cho Tinh Anh, Warden & Trùm
    if (e.hp > 0 && (e.type == BOSS || e.isElite || e.isWarden) && e.staggerBrokenTimer <= 0.0f) {
        e.stagger += (e.type == BOSS ? 5.5f : 12.5f) * (isCrit ? 1.55f : 1.0f);
        if (e.stagger >= e.maxStagger) {
            e.stagger = e.maxStagger;
            e.staggerBrokenTimer = 4.5f;
            e.stunTimer = std::max(e.stunTimer, e.type == BOSS ? 1.35f : 1.80f);
            NovaVisual nv; nv.pos = e.pos; nv.maxRadius = e.radius * 2.4f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = GOLD;
            novas.push_back(nv);
            Burst(e.pos, GOLD, 24, 280.0f, 0.55f);
            Shake(0.22f, 5.5f);
            AddStyle(28.0f);
            SpawnPopup({ e.pos.x, e.pos.y - e.radius - 20.0f }, L("ARMOR BROKEN! +35% DMG", "PHÁ GIÁP! +35% ST"), GOLD);
        }
    }

    if (e.hp <= 0) {
        OnEnemyKilled(e);
    }
}

inline void Game::OnEnemyKilled(Enemy& e) {
    Burst(e.pos, DeathColor(e), e.type == BOSS ? 40 : 16, 260.0f, 0.6f);

    // Splitter bản lớn chia thành 2 mảnh nhỏ (đưa vào hàng đợi vì có thể đang duyệt danh sách quái)
    if (e.type == SPLITTER && e.splitLevel == 0) {
        SpawnSplitMini(Vector2Add(e.pos, { -22.0f, 6.0f }));
        SpawnSplitMini(Vector2Add(e.pos, { 22.0f, 6.0f }));
        Burst(e.pos, { 90, 220, 230, 255 }, 14, 200.0f, 0.5f);
    }

    // Bomber tự nổ: không rơi loot, không tính combo
    if (e.type == BOMBER && e.exploded) {
        PlaySound(snd.sndDeath);
        return;
    }

    enemiesSlain++;
    comboCount++;
    if (comboCount > bestCombo) bestCombo = comboCount;
    comboTimer = 3.0f;
    PlaySound(snd.sndDeath);
    AddStyle(e.type == BOSS ? 45.0f : (e.isElite || e.isWarden ? 20.0f : 11.0f));
    AddRage(e.type == BOSS ? 35.0f : (e.isElite || e.isWarden ? 15.0f : 6.0f));
    if (activeRune == 3) { // Cổ Ngữ Huyết Vũ: hạ quái tăng Cuồng Tốc + hồi máu
        runeBuffTimer = std::max(runeBuffTimer, 2.5f + 0.4f * runeRank);
        if (player.hp < player.maxHp) {
            float rh = 1.5f * runeRank;
            player.hp = std::min(player.maxHp, player.hp + rh);
        }
    }
    if (e.isElite && bountyActive && bountyKind == 0) bountyProgress++;
    if (e.isGoldenGoblin) {
        int gobGold = 120 + floor * 12;
        player.gold += gobGold;
        bool ess = (Rand01() < 0.55f);
        if (ess) bossEssence++;
        if (bountyActive && bountyKind == 2) bountyProgress++;
        SpawnItemDrop(e.pos);
        Burst(e.pos, GOLD, 36, 320.0f, 0.7f);
        SpawnPopup(e.pos, TextFormat("+%dG!", gobGold), GOLD);
        ShowBanner(ess ? L("GOLDEN GOBLIN SLAIN! +GOLD, +GEAR, +1 BOSS ESSENCE!", "HẠ YÊU TINH VÀNG! +VÀNG, +TRANG BỊ, +1 TINH HOA!")
                       : L("GOLDEN GOBLIN SLAIN! +GOLD & TREASURE GEAR!", "HẠ YÊU TINH VÀNG! +VÀNG & TRANG BỊ HIẾM!"), 3.2f);
    }
    if (royalQuestActive && !isNoble && !(e.type == SPLITTER && e.splitLevel > 0)) {
        royalQuestKills++;
        if (e.type == BOSS) royalQuestBosses++;
        if (royalQuestKills >= 35 && royalQuestBosses >= 1) CompleteRoyalCommission();
    }
    if (!(e.type == SPLITTER && e.splitLevel > 0)) {
        spouseQuestProgress++;
        if (spouseQuestProgress == 15 && !spouseUnlocked[1]) {
            UnlockSpouse(1, "MERCHANT QUEST COMPLETE: VAN ANH HAS BEEN BETROTHED TO YOU!",
                            "HOÀN THÀNH ỦY THÁC THƯƠNG NHÂN: PHÚ ÔNG GẢ CON GÁI VÂN ANH CHO BẠN!");
        }
        if (e.type == BOSS && !spouseUnlocked[2]) {
            UnlockSpouse(2, "BOSS SLAIN! PRIESTESS THIEN THANH AGREES TO MARRY YOU!",
                            "HẠ TRÙM HẦM NGỤC! THÁNH NỮ THIÊN THANH CẢM PHỤC ĐỒNG Ý KẾT DUYÊN!");
        }
    }

    // Nội tại Sát Thủ: hạ gục hoàn 50% hồi chiêu Dash và tăng 25% tốc chạy trong 2s
    if (player.heroClass == CLASS_ASSASSIN) {
        player.dashTimer = std::max(0.0f, player.dashTimer - player.dashCooldown * 0.5f);
        player.hasteTimer = std::max(player.hasteTimer, 2.0f);
        if (player.subClass == 1) { // Phantom Reaper: Soul Harvest hồi 4 HP & bắn 1 linh hồn dẫn đường
            if (player.hp < player.maxHp) {
                player.hp = std::min(player.maxHp, player.hp + 4.0f);
                SpawnPopup({ player.pos.x, player.pos.y - 24.0f }, "+4", GREEN);
            }
            float a = Rand01() * 2.0f * PI;
            Projectile sw;
            sw.fromPlayer = true;
            sw.kind = 5;
            sw.pos = e.pos;
            sw.vel = { cosf(a) * 440.0f, sinf(a) * 440.0f };
            sw.damage = player.baseDmg * 1.15f;
            sw.life = 2.4f;
            sw.radius = 7.5f;
            sw.color = { 195, 95, 255, 255 };
            projectiles.push_back(sw);
            Burst(e.pos, { 195, 95, 255, 255 }, 6, 150.0f, 0.35f);
        }
    }

    // Combo gold payout every 5 kills & Soul Overdrive every 15 kills
    if (comboCount % 5 == 0) {
        int cg = comboCount * 2;
        if (isNoble) cg = (int)roundf(cg * 1.25f);
        player.gold += cg;
        SpawnPopup(player.pos, "+GOLD COMBO!", GOLD);
    }
    if (comboCount > 0 && comboCount % 15 == 0) {
        overdriveTimer = 4.5f;
        NovaVisual nv; nv.pos = player.pos; nv.maxRadius = 145.0f; nv.timer = 0.35f; nv.maxTimer = 0.35f; nv.color = GOLD;
        novas.push_back(nv);
        Burst(player.pos, GOLD, 28, 300.0f, 0.6f);
        for (auto& other : enemies) {
            if (&other != &e && other.hp > 0 && Vector2Distance(player.pos, other.pos) < 145.0f) {
                Vector2 push = Vector2Normalize(Vector2Subtract(other.pos, player.pos));
                other.pos = Vector2Add(other.pos, Vector2Scale(push, 26.0f));
            }
        }
        SpawnPopup({ player.pos.x, player.pos.y - 42.0f }, L("SOUL OVERDRIVE! +18% DMG", "BÙNG NỔ LINH HỒN! +18% ST"), GOLD);
        PlaySound(snd.sndShield);
    }

    // Drops
    if (e.isWarden) {
        loots.push_back({ 2, Vector2Add(e.pos, { 0, 18 }), 35, 20.0f });
        loots.push_back({ 1, Vector2Add(e.pos, { 18, 0 }), 8, 20.0f });
        ShowBanner("WARDEN BRUTE DEFEATED!");
        GrantOrUpgradeFamiliar();
    }
    if (e.type == BOSS) {
        hitStop = 0.03f;
        slowMoTimer = 0.25f;
        bossesKilled++;
        int ess = (floor >= 15 ? 2 : 1);
        bossEssence += ess;
        if (bountyActive && bountyKind == 2) bountyProgress++;
        SpawnPopup({ e.pos.x, e.pos.y - 36.0f }, TextFormat(L("+%d BOSS ESSENCE", "+%d TINH HOA TRÙM"), ess), GOLD);
        GrantOrUpgradeFamiliar();
        if (e.isKingBoss) {
            isKing = true;
            isNoble = true;
            peerageRank = 8;
            inRebellion = false;
            player.maxHp += 120.0f;
            player.hp = player.maxHp;
            player.baseDmg += 25.0f;
            player.armor += 3.0f;
            player.gold += 800;
            bossEssence += 3;
            warMerit += 200;
            for (auto& other : enemies) if (&other != &e) other.hp = 0.0f;
            pendingSpawns.clear();
            ShowBanner(L("THE KING IS FALLEN! YOU ARE CROWNED SOVEREIGN MONARCH!",
                         "ĐÃ LẬT ĐỔ BẠO CHÚA! BẠN ĐĂNG QUANG QUỐC VƯƠNG TỐI CAO!"), 5.0f);
        } else if (e.isNationalGeneral) {
            int ni = std::max(1, std::min(4, e.generalNation));
            nations[ni].status = 3;
            nations[ni].warProgress = 100.0f;
            nations[0].territories += nations[ni].territories;
            nations[ni].territories = 0;
            inWarBattle = false;
            warMerit += 120;
            player.gold += 400;
            bossEssence += 2;
            player.maxHp += 30.0f;
            player.hp = std::min(player.maxHp, player.hp + 30.0f);
            player.baseDmg += 6.0f;
            for (auto& other : enemies) if (&other != &e) other.hp = 0.0f;
            pendingSpawns.clear();
            ShowBanner(TextFormat(L("VICTORY! %s HAS BEEN CONQUERED!", "ĐẠI THẮNG! ĐÃ CHINH PHẠT %s!"),
                                  L(nations[ni].nameEn, nations[ni].nameVi)), 4.5f);
        }
    }
    loots.push_back({ 0, e.pos, (e.type == BOSS ? 150 : (e.isWarden ? 90 : (e.isElite ? 35 : (e.isMinion ? 5 : 15)))), 20.0f });
    {
        float dropChance = (e.type == BOSS || e.isWarden) ? 1.0f : (e.isElite ? 0.20f : 0.055f) + 0.015f * meta[5];
        if (!e.isMinion && Rand01() < dropChance) SpawnItemDrop(e.pos);
    }
    if ((float)rand() / RAND_MAX < 0.6f || e.isElite) {
        loots.push_back({ 1, Vector2Add(e.pos, { 10, 0 }), rand() % 4 + 1, 20.0f });
    }
    if ((float)rand() / RAND_MAX < 0.09f || (HasRelic(RELIC_CHALICE) && (float)rand() / RAND_MAX < 0.18f)) {
        loots.push_back({ 2, Vector2Add(e.pos, { -10, 0 }), 25, 20.0f });
    }

    // Relic: Combustion explosion
    if (HasRelic(RELIC_COMBUSTION)) {
        NovaVisual exp;
        exp.pos = e.pos;
        exp.radius = 0;
        exp.maxRadius = 120.0f;
        exp.timer = 0.25f;
        exp.maxTimer = 0.25f;
        exp.color = ORANGE;
        novas.push_back(exp);
        for (auto& other : enemies) {
            if (&other != &e && other.hp > 0 && Vector2Distance(e.pos, other.pos) <= exp.maxRadius) {
                other.hp -= 25.0f;
                SpawnPopup(other.pos, "25", ORANGE);
            }
        }
    }

    // Boss drops Relic!
    if (e.type == BOSS) {
        GiveRandomRelic();
    }
    GrantPartyXp(e); // con theo cha rèn luyện nhận EXP
    if (e.isElite && e.eliteAffix == 3) { // MOLTEN: rơi bom nham thạch
        GroundEffect bomb;
        bomb.kind = 4;
        bomb.pos = e.pos;
        bomb.radius = 90.0f;
        bomb.maxTimer = 0.70f;
        bomb.timer = 0.70f;
        bomb.damage = player.baseDmg * 0.8f;
        bomb.color = ORANGE;
        groundEffects.push_back(bomb);
    }
    if (e.isElite && e.eliteAffix == 1) { // VAMPIRIC: rơi bình máu phụ
        loots.push_back({ 2, Vector2Add(e.pos, { 0, -14 }), 25, 20.0f });
    }
}

inline void Game::KnightCounterShockwave() {
    AddRage(8.0f);
    NovaVisual nv;
    nv.pos = player.pos;
    nv.radius = 0.0f;
    nv.maxRadius = 120.0f;
    nv.timer = 0.26f;
    nv.maxTimer = 0.26f;
    nv.color = GOLD;
    novas.push_back(nv);
    Burst(player.pos, GOLD, 14, 220.0f, 0.4f);
    for (auto& en : enemies) {
        if (en.hp <= 0) continue;
        if (Vector2Distance(player.pos, en.pos) <= 120.0f + en.radius * 0.5f) {
            Vector2 kb = Vector2Normalize(Vector2Subtract(en.pos, player.pos));
            DamageEnemy(en, player.baseDmg * 0.30f, kb, 180.0f, false, false);
        }
    }
}

inline bool Game::DamagePlayer(float dmg) {
    if (player.isInvuln || player.hp <= 0) return false;

    // Khiên chặn đòn (Divine Aegis hoặc Thánh Quang Xung của Knight)
    if (player.shieldActive) {
        player.shieldActive = false;
        player.shieldTimer = 0.0f;
        PlaySound(snd.sndShield);
        SpawnPopup(player.pos, "BLOCKED!", GOLD);
        Shake(0.12f, 3.0f);
        if (player.heroClass == CLASS_KNIGHT) KnightCounterShockwave();
        return false;
    }

    float effArmor = EffectiveArmor() + (HasDuoImmortalBastion() ? 3.0f : 0.0f);
    float effDmg = dmg * 100.0f / (100.0f + effArmor * 7.5f);
    if (player.heroClass == CLASS_KNIGHT) effDmg *= (player.subClass == 1 ? 0.80f : 0.85f);
    if (player.heroClass == CLASS_ASSASSIN && player.ultActiveTimer > 0.0f) effDmg *= 0.50f;
    effDmg *= AgeDamageTakenMult();
    effDmg = std::max(1.0f, effDmg);
    player.hp = std::max(0.0f, player.hp - effDmg);
    comboCount = 0;
    if (styleRank > 0) { styleRank--; stylePoints = 35.0f; }
    else stylePoints = 0.0f;
    damagePulse = std::min(1.0f, 0.45f + effDmg / 60.0f);
    player.invulnTimer = 0.36f;
    PlaySound(snd.sndHurt);
    Shake(0.18f, 5.0f);
    SpawnPopup(player.pos, std::to_string((int)effDmg), RED);
    AddRage(8.0f);
    if (player.heroClass == CLASS_KNIGHT && player.hp > 0) KnightCounterShockwave();

    if (player.hp <= 0 && HasRelic(RELIC_PHOENIX) && !phoenixUsed) {
        // Phoenix Feather: hồi sinh một lần với nửa máu
        phoenixUsed = true;
        player.hp = player.maxHp * 0.5f;
        player.invulnTimer = 2.5f;
        for (size_t ri = 0; ri < player.relics.size(); ++ri)
            if (player.relics[ri].type == RELIC_PHOENIX) { player.relics.erase(player.relics.begin() + ri); break; }
        Burst(player.pos, { 255, 160, 60, 255 }, 40, 380.0f, 0.9f);
        Shake(0.4f, 8.0f);
        PlaySound(snd.sndLevelUp);
        ShowBanner("THE PHOENIX FEATHER BRINGS YOU BACK!", 3.0f);
    } else if (player.hp > 0 && HasRelic(RELIC_THORNS)) {
        ThornBurst();
    }

    if (player.hp <= 0) {
        inRebellion = false;
        inWarBattle = false;
        alliedUnits.clear();
        state = GAME_OVER;
        PlaySound(snd.sndDeath);
        AwardSouls();
        if (floor > bestFloor) {
            bestFloor = floor;
            SaveSettings();
            newRecord = true;
        }
    }
    return true;
}

inline void Game::BomberExplode(Enemy& e) {
    const float blastRadius = 120.0f;
    e.exploded = true;
    PlaySound(snd.sndSpecial);
    Shake(0.3f, 9.0f);

    NovaVisual nv;
    nv.pos = e.pos;
    nv.radius = 0;
    nv.maxRadius = blastRadius;
    nv.timer = 0.3f;
    nv.maxTimer = 0.3f;
    nv.color = ORANGE;
    novas.push_back(nv);
    Burst(e.pos, { 255, 179, 51, 255 }, 30, 330.0f, 0.65f);
    Burst(e.pos, { 255, 77, 26, 255 }, 14, 200.0f, 0.5f);

    if (Vector2Distance(e.pos, player.pos) <= blastRadius + 18.0f) DamagePlayer(20.0f);

    // Friendly fire: vụ nổ cũng làm đau quái khác (không kích hoạt relic)
    for (auto& other : enemies) {
        if (&other == &e || other.hp <= 0) continue;
        if (Vector2Distance(e.pos, other.pos) <= blastRadius + other.radius * 0.5f) {
            other.hp -= 40.0f;
            SpawnPopup(other.pos, "40", ORANGE);
            if (other.hp <= 0) OnEnemyKilled(other);
        }
    }

    e.hp = 0;
    OnEnemyKilled(e);
}

inline void Game::UseShrine() {
    float cost = std::max(10.0f, roundf(player.maxHp * 0.15f));

    // Không bao giờ giết người chơi
    if (player.hp <= cost + 5.0f) {
        shrineRefuse = 2.0f;
        SpawnPopup({ shrinePos.x, shrinePos.y - 50.0f }, "NEED " + std::to_string((int)cost) + " HP", LIGHTGRAY);
        return;
    }

    shrineUsed = true;
    player.hp -= cost;
    UpgradeType up = (UpgradeType)(rand() % 11);
    ApplyUpgrade(up);

    ShowBanner("BLOOD SHRINE: -" + std::to_string((int)cost) + " HP  ->  " + UpgradeTitle(up), 3.0f);
    SpawnPopup(player.pos, "-" + std::to_string((int)cost), RED);
    PlaySound(snd.sndSpecial);
    Shake(0.2f, 5.0f);
    Burst({ shrinePos.x, shrinePos.y - 34.0f }, { 255, 51, 64, 255 }, 28, 300.0f, 0.8f);
    damagePulse = 0.6f;
}

inline void Game::CheckAndGrantLevelTitleMilestones() {
    player.level = std::max(1, std::min(kMaxPlayerLevel, player.level));
    if (player.level >= kMaxPlayerLevel) player.xp = player.xpNext;
    int targetTier = LevelTitleTierForLevel(player.level);
    while (player.titleTierClaimed < targetTier) {
        player.titleTierClaimed++;
        const LevelTitleDef& td = kLevelTitles[player.titleTierClaimed];
        player.maxHp += td.hpBonus;
        player.hp = std::min(player.maxHp, player.hp + td.hpBonus);
        player.baseDmg += td.dmgBonus;
        player.armor += td.armorBonus;
        player.critChance = std::min(0.75f, player.critChance + td.critBonus);
        player.attackRate += td.atkRateBonus;
        if (player.titleTierClaimed == 5) {
            player.hp = player.maxHp;
            AddRage(100.0f);
        }
        PlaySound(snd.sndSpecial);
        Burst(player.pos, td.color, 36, 320.0f, 0.9f);
        ShowBanner(std::string(L("TITLE UNLOCKED [LV.", "ĐẠT DANH HIỆU [CẤP ")) + std::to_string(td.minLevel) + "]: " + LevelTitleNameForLevel(td.minLevel) + "!", 3.8f);
    }
}

inline void Game::TriggerLevelUp() {
    player.level = std::min(kMaxPlayerLevel, player.level);
    CheckAndGrantLevelTitleMilestones();
    state = LEVEL_UP;
    subclassChoosing = (player.level >= 5 && player.subClass == 0);
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, { 255, 235, 115, 255 }, 26, 300.0f, 0.8f);
    Burst(player.pos, { 128, 242, 255, 255 }, 14, 180.0f, 0.7f);
    levelChoices = { UP_HP, UP_DMG, UP_ATTACK_RATE };
    std::vector<UpgradeType> all = { UP_HP, UP_DMG, UP_SPEED, UP_ATTACK_RATE, UP_DASH, UP_LIFESTEAL, UP_ARMOR, UP_CRIT, UP_NOVA, UP_IGNITE, UP_FROST };
    std::mt19937 rng((unsigned)rand());
    std::shuffle(all.begin(), all.end(), rng);
    levelChoices = { all[0], all[1], all[2] };
}

inline void Game::ChooseSubclass(int branch) {
    int hc = std::max(0, std::min(kNumClasses - 1, player.heroClass));
    int b = (branch == 2) ? 2 : 1;
    player.subClass = b;
    subclassChoosing = false;
    const SubclassDef& sc = kSubclasses[hc][b - 1];

    player.maxHp += roundf(sc.bonusHp * 0.65f);
    player.hp = std::min(player.maxHp, player.hp + player.maxHp * 0.20f + roundf(sc.bonusHp * 0.65f));
    player.baseDmg += roundf(sc.bonusDmg * 0.50f);
    player.armor += sc.bonusArmor * 0.65f;
    player.speed += sc.bonusSpeed * 0.75f;
    player.attackRate += sc.bonusAtkRate * 0.50f;
    player.critChance = std::min(0.6f, player.critChance + sc.bonusCrit * 0.65f);
    player.lifesteal = std::min(LifestealCap() + 0.04f * LifestealScale(), player.lifesteal + sc.bonusLifesteal * 0.55f * LifestealScale());
    player.specialCooldown = Skill1MaxCd();
    player.skill2Cooldown = Skill2MaxCd();
    player.skill3Cooldown = Skill3MaxCd();
    player.skill4Cooldown = Skill4MaxCd();
    player.specialTimer = 0.0f;
    player.skill2Timer = 0.0f;
    player.skill3Timer = 0.0f;
    player.skill4Timer = 0.0f;
    AddRage(30.0f);

    // Rèn lại tên & chỉ số vũ khí trấn phái theo đúng vũ khí đặc trưng của class
    int cw = ClassWeaponType(hc);
    if (hasEquip[SLOT_WEAPON]) {
        equipped[SLOT_WEAPON].weaponType = cw;
        equipped[SLOT_WEAPON].name = std::string(equipped[SLOT_WEAPON].RarityName()) + " " + sc.weaponTitle;
        equipped[SLOT_WEAPON].damage += 2.0f;
        player.baseDmg += 2.0f;
    } else {
        EquipItem w = MakeStarterWeapon(cw);
        w.name = sc.weaponTitle;
        int sold = 0;
        TryEquipItem(w, sold);
    }

    PlaySound(snd.sndLevelUp);
    PlaySound(snd.sndSpecial);
    Shake(0.35f, 7.5f);
    Burst(player.pos, sc.color, 36, 360.0f, 0.85f);
    Burst(player.pos, sc.accent, 22, 240.0f, 0.70f);
    ShowBanner(std::string("AWAKENED SUBCLASS: ") + sc.name + "!", 3.5f);
    state = PLAYING;
}

inline void Game::ChooseUpgrade(UpgradeType up) {
    if (subclassChoosing) {
        int autoBranch = (player.heroClass == CLASS_MAGE) ? 1 : ((enemiesSlain % 2) + 1);
        ChooseSubclass(autoBranch);
        return;
    }
    ApplyUpgrade(up);
    PlaySound(snd.sndPotion);
    state = PLAYING;
}

inline void Game::ApplyUpgrade(UpgradeType up) {
    if (up == UP_HP) { player.maxHp += 20; player.hp += 20; }
    else if (up == UP_DMG) { player.baseDmg += 5; }
    else if (up == UP_SPEED) { player.speed += 22; }
    else if (up == UP_ATTACK_RATE) { player.attackRate += 0.35f; }
    else if (up == UP_DASH) { player.dashCooldown = std::max(0.4f, player.dashCooldown - 0.2f); }
    else if (up == UP_LIFESTEAL) { player.lifesteal = std::min(LifestealCap(), player.lifesteal + LifestealStep()); }
    else if (up == UP_ARMOR) { player.armor += 1.5f; }
    else if (up == UP_CRIT) { player.critChance = std::min(0.6f, player.critChance + 0.06f); }
    else if (up == UP_NOVA) {
        player.cdrUpgrades = std::min(5, player.cdrUpgrades + 1);
        player.specialCooldown = Skill1MaxCd();
        player.skill2Cooldown = Skill2MaxCd();
        player.skill3Cooldown = Skill3MaxCd();
        player.skill4Cooldown = Skill4MaxCd();
        player.specialTimer = std::min(player.specialTimer, player.specialCooldown);
        player.skill2Timer = std::min(player.skill2Timer, player.skill2Cooldown);
        player.skill3Timer = std::min(player.skill3Timer, player.skill3Cooldown);
        player.skill4Timer = std::min(player.skill4Timer, player.skill4Cooldown);
    }
    else if (up == UP_IGNITE) { player.igniteLevel++; }
    else if (up == UP_FROST) { player.frostLevel++; }
}

inline Color Game::TintCol(Color col, const Enemy& e) {
    if (e.chillTimer > 0) col = LerpColor(col, SKYBLUE, 0.55f);
    if (e.hitFlash > 0) col = LerpColor(col, WHITE, 0.80f);
    return col;
}

inline void Game::QuadOut(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color fill, Color border, float thick) {
    Quad(a, b, c, d, fill);
    DrawLineEx(a, b, thick, border);
    DrawLineEx(b, c, thick, border);
    DrawLineEx(c, d, thick, border);
    DrawLineEx(d, a, thick, border);
}

inline void Game::BoxRot(Vector2 center, float w, float h, float deg, Color fill, Color border, float thick) {
    float rad = deg * DEG2RAD;
    float ca = cosf(rad), sa = sinf(rad);
    float hw = w * 0.5f, hh = h * 0.5f;
    Vector2 p0 = { center.x + (-hw * ca - (-hh) * sa), center.y + (-hw * sa + (-hh) * ca) };
    Vector2 p1 = { center.x + ( hw * ca - (-hh) * sa), center.y + ( hw * sa + (-hh) * ca) };
    Vector2 p2 = { center.x + ( hw * ca - ( hh) * sa), center.y + ( hw * sa + ( hh) * ca) };
    Vector2 p3 = { center.x + (-hw * ca - ( hh) * sa), center.y + (-hw * sa + ( hh) * ca) };
    QuadOut(p0, p1, p2, p3, fill, border, thick);
}

inline void Game::Diamond(Vector2 c, float rx, float ry, Color fill, Color border) {
    Vector2 top = { c.x, c.y - ry };
    Vector2 right = { c.x + rx, c.y };
    Vector2 bot = { c.x, c.y + ry };
    Vector2 left = { c.x - rx, c.y };
    QuadOut(top, right, bot, left, fill, border, 1.4f);
    Tri(top, left, c, Shade(fill, 1.35f));
    Tri(c, right, bot, Shade(fill, 0.70f));
}

inline void Game::Eye(Vector2 c, float rx, float ry, Vector2 look, Color iris) {
    DrawEllipse((int)c.x, (int)c.y, rx + 1.3f, ry + 1.3f, { 18, 18, 26, 255 });
    DrawEllipse((int)c.x, (int)c.y, rx, ry, WHITE);
    float px = c.x + look.x * rx * 0.42f, py = c.y + look.y * ry * 0.42f;
    DrawEllipse((int)px, (int)py, rx * 0.60f, ry * 0.66f, iris);
    DrawEllipse((int)px, (int)py, rx * 0.30f, ry * 0.36f, { 12, 10, 16, 255 });
    DrawCircle((int)(px - rx * 0.2f), (int)(py - ry * 0.25f), std::max(1.0f, rx * 0.18f), WHITE);
}

inline const char* Game::RelicDesc(RelicType t) {
    switch (t) {
        case RELIC_LIGHTNING:  return L("Hits arc lightning to a nearby enemy", "Đòn đánh phóng sét lan sang địch gần");
        case RELIC_TOXIC:      return L("Hits poison enemies", "Đòn đánh gây độc lên kẻ địch");
        case RELIC_FROST:      return L("Crits freeze enemies", "Đòn chí mạng đóng băng kẻ địch");
        case RELIC_AEGIS:      return L("Blocks one hit every 12s", "Chặn 1 đòn mỗi 12 giây");
        case RELIC_CHALICE:    return L("More healing drops", "Tăng tỉ lệ rơi cầu hồi máu");
        case RELIC_COMBUSTION: return L("Enemies explode on death", "Kẻ địch phát nổ khi bị hạ");
        case RELIC_PHOENIX:    return L("Revive once at half HP", "Hồi sinh 1 lần với 50% máu");
        case RELIC_MIDAS:      return L("+50% gold, bigger pickup range", "+50% vàng & tầm nhặt rộng hơn");
        case RELIC_BERSERK:    return L("+40% damage below 40% HP", "+40% sát thương khi máu < 40%");
        case RELIC_THORNS:     return L("Getting hurt unleashes a shockwave", "Bị đánh sẽ nổ sóng xung kích");
    }
    return "";
}
