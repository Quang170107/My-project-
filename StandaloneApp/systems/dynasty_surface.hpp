#pragma once
#include "../game.hpp"

inline int Game::TimePhase() const {
    if (timeOfDay >= 5.0f && timeOfDay < 8.0f) return 0;  // Dawn
    if (timeOfDay >= 8.0f && timeOfDay < 17.0f) return 1; // Day
    if (timeOfDay >= 17.0f && timeOfDay < 20.0f) return 2; // Dusk
    return 3; // Night
}

inline const char* Game::TimePhaseName() const {
    switch (TimePhase()) {
        case 0: return L("Dawn", "Bình Minh");
        case 1: return L("Day", "Ban Ngày");
        case 2: return L("Dusk", "Hoàng Hôn");
        default: return L("Night", "Ban Đêm");
    }
}

inline std::string Game::ClockString() const {
    int totMin = std::max(0, (int)floorf(timeOfDay * 60.0f)) % (24 * 60);
    int hh = totMin / 60, mm = totMin % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", hh, mm);
    return std::string(buf);
}

inline void Game::AdvanceTime(float hours) {
    if (hours <= 0.0f) return;
    timeOfDay += hours;
    while (timeOfDay >= 24.0f) {
        timeOfDay -= 24.0f;
        dayCount++;
    }
}

inline int Game::HeroAgeStage() const {
    if (heroAge < 30) return 0;
    if (heroAge < 45) return 1;
    if (heroAge < 60) return 2;
    return 3;
}

inline const char* Game::AgeStageName() const {
    switch (HeroAgeStage()) {
        case 0: return L("Youth", "Thanh Niên");
        case 1: return L("Prime", "Tráng Niên");
        case 2: return L("Middle Age", "Trung Niên");
        default: return L("Elder", "Lão Niên");
    }
}

inline Color Game::AgeStageColor() const {
    switch (HeroAgeStage()) {
        case 0: return { 130, 235, 160, 255 };
        case 1: return GOLD;
        case 2: return { 235, 175, 90, 255 };
        default: return { 200, 200, 215, 255 };
    }
}

inline float Game::AgeDamageMult() const {
    int st = HeroAgeStage();
    if (st == 1) return 1.08f;
    if (st == 3) return std::max(0.75f, 1.0f - 0.0125f * (heroAge - 59));
    return 1.0f;
}

inline float Game::AgeSpeedMult() const {
    int st = HeroAgeStage();
    if (st == 0) return 1.05f;
    if (st == 2) return 0.95f;
    if (st == 3) return std::max(0.72f, 0.92f - 0.01f * (heroAge - 60));
    return 1.0f;
}

inline float Game::AgeDamageTakenMult() const {
    int st = HeroAgeStage();
    if (st == 1) return 0.92f;
    if (st == 3) return std::min(1.25f, 1.0f + 0.0125f * (heroAge - 59));
    return 1.0f;
}

inline std::string Game::AgeEffectText() const {
    switch (HeroAgeStage()) {
        case 0: return L("+5% Move Speed", "+5% Tốc Chạy");
        case 1: return L("+8% DMG, -8% DMG Taken", "+8% ST, -8% ST Nhận");
        case 2: return L("-5% Move Speed", "-5% Tốc Chạy");
        default: return TextFormat(L("-%d%% DMG, -%d%% Speed", "-%d%% ST, -%d%% Tốc"),
                                   (int)roundf((1.0f - AgeDamageMult()) * 100.0f), (int)roundf((1.0f - AgeSpeedMult()) * 100.0f));
    }
}

inline int Game::OldestChildIdx() const {
    int best = -1;
    for (size_t k = 0; k < children.size(); ++k)
        if (best < 0 || children[k].age > children[best].age) best = (int)k;
    return best;
}

inline bool Game::UpdateAging(float dt) {
    if (dt <= 0.0f) return false;
    float yrs = dt / kRealSecondsPerYear;
    for (size_t k = 0; k < children.size(); ++k) {
        ChildInfo& ch = children[k];
        int before = ChildAgeYears(ch);
        ch.age += yrs;
        int after = ChildAgeYears(ch);
        if (after != before) {
            if (after == kChildTrainAge) {
                ShowBanner(ch.name + L(" turned 6! Bring them to train in the Dungeon (press [T] at home).",
                                        " đã tròn 6 tuổi! Có thể dẫn con xuống Hầm Ngục rèn luyện ([T] tại Tổ Ấm)."), 4.0f);
                PlaySound(snd.sndLevelUp);
            } else if (after == kChildAdultAge) {
                ShowBanner(ch.name + L(" has come of age (16)! Ready to inherit the Bloodline.",
                                        " đã trưởng thành (16 tuổi)! Đủ tư cách kế thừa gia tộc."), 4.0f);
                PlaySound(snd.sndLevelUp);
            } else if (!inSurface && ch.active) {
                SpawnPopup({ ch.pos.x, ch.pos.y - 30.0f }, TextFormat(L("%d YEARS OLD!", "%d TUỔI!"), after), SKYBLUE);
            }
        }
        if (ch.injuredTimer > 0.0f) {
            ch.injuredTimer -= dt;
            if (ch.injuredTimer <= 0.0f) {
                ch.injuredTimer = 0.0f;
                ShowBanner(ch.name + L(" has fully recovered and can train again!", " đã hồi phục, có thể tiếp tục rèn luyện!"), 2.8f);
            }
        }
    }
    yearClock += dt;
    while (yearClock >= kRealSecondsPerYear) {
        yearClock -= kRealSecondsPerYear;
        int prevStage = HeroAgeStage();
        heroAge++;
        SpawnPopup({ player.pos.x, player.pos.y - 44.0f }, TextFormat(L("BIRTHDAY! AGE %d", "SINH NHẬT! %d TUỔI"), heroAge), GOLD);
        if (HeroAgeStage() != prevStage) {
            ShowBanner(std::string(L("NEW LIFE STAGE: ", "BƯỚC SANG TUỔI ")) + AgeStageName() + " (" + AgeEffectText() + ")", 3.6f);
            PlaySound(snd.sndSpecial);
        } else if (heroAge >= kHeroLifespan - 5 && heroAge < kHeroLifespan) {
            ShowBanner(TextFormat(L("Age %d: your lifespan nears its end (%d). Prepare an heir!",
                                    "Tuổi %d: tuổi thọ sắp cạn (tối đa %d). Hãy chuẩn bị người nối dõi!"), heroAge, kHeroLifespan), 3.6f);
        }
        if (heroAge >= kHeroLifespan) {
            int heir = OldestChildIdx();
            if (heir >= 0) {
                std::string oldName = CurrentHeroDisplayName();
                SuccessionToChild(heir, true);
                ShowBanner(oldName + L(" passed away peacefully of old age. ", " đã an nghỉ vì tuổi già. ")
                           + CurrentHeroDisplayName() + L(" carries on the Bloodline!", " tiếp nối gia tộc!"), 4.6f);
                return true;
            } else if (heroAge == kHeroLifespan) {
                ShowBanner(L("You outlived your lifespan with no heir... strength keeps fading!",
                             "Đã quá tuổi thọ mà chưa có người nối dõi... sức lực ngày càng suy kiệt!"), 4.0f);
            }
        }
    }
    return false;
}

inline void Game::SetHeroName(const std::string& nm) {
    if (!nm.empty()) heroGivenName = nm;
}

inline void Game::SetChildName(int idx, const std::string& nm) {
    if (idx >= 0 && idx < (int)children.size() && !nm.empty()) children[idx].name = nm;
}

inline const char* Game::SpouseUnlockConditionText(int idx) const {
    switch (idx) {
        case 0: return L("Event: Rescue her from Dungeon Cage", "Sự kiện: Giải cứu ở Hầm Ngục");
        case 1: return TextFormat(L("Event/Quest: Rescue or Slay %d/15 Monsters", "Sự kiện/NV: Cứu hoặc Hạ %d/15 Quái"), std::min(15, spouseQuestProgress));
        case 2: return L("Event/Quest: Rescue or Conquer Trial/Boss", "Sự kiện/NV: Cứu hoặc Thắng Thử Thách/Trùm");
        default: return L("Quest: King's Commission or Royal Rescue", "NV/Sự kiện: Hoàn thành Ủy Thác Của Vua");
    }
}

inline void Game::UnlockSpouse(int idx, const char* msgEn, const char* msgVi) {
    if (idx < 0 || idx >= 4 || spouseUnlocked[idx]) return;
    spouseUnlocked[idx] = true;
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, kSpouses[idx].dressCol, 34, 290.0f, 0.85f);
    ShowBanner(L(msgEn, msgVi), 4.2f);
}

inline void Game::RescueCaptiveSpouse() {
    if (!hasCaptiveEvent || captiveRescued) return;
    captiveRescued = true;
    int idx = std::max(0, std::min(3, captiveSpouseIdx));
    player.hp = std::min(player.maxHp, player.hp + 35.0f);
    AddRage(25.0f);
    SpawnPopup(captivePos, L("RESCUED!", "ĐÃ GIẢI CỨU!"), kSpouses[idx].dressCol);
    Burst(captivePos, GOLD, 28, 260.0f, 0.75f);
    std::string en = std::string("RESCUED ") + kSpouses[idx].nameEn + "! She now awaits you at Family Home!";
    std::string vi = std::string("ĐÃ GIẢI CỨU ") + kSpouses[idx].nameVi + "! Nàng đang đợi bạn tại Tổ Ấm!";
    UnlockSpouse(idx, en.c_str(), vi.c_str());
}

inline void Game::OpenNameModal(int mode, int childOrSlotIdx, const std::string& defName) {
    namingMode = mode;
    namingChildIdx = childOrSlotIdx;
    nameInputBuf = defName;
    StopRumble();
}

inline void Game::ConfirmNameModal() {
    std::string finalName = nameInputBuf;
    while (!finalName.empty() && finalName.front() == ' ') finalName.erase(finalName.begin());
    while (!finalName.empty() && finalName.back() == ' ') finalName.pop_back();
    if (finalName.empty()) finalName = kHeirNames[rand() % 10];
    if (namingMode == 1) {
        heroGivenName = finalName;
        namingMode = 0;
        state = PLAYING;
        PlaySound(snd.sndLevelUp);
        ShowBanner(std::string(L("HERO NAMED: ", "ĐẶT TÊN NHÂN VẬT: ")) + heroGivenName + "!", 2.8f);
    } else if (namingMode == 2) {
        SetChildName(namingChildIdx, finalName);
        namingMode = 0;
        PlaySound(snd.sndLevelUp);
        ShowBanner(std::string(L("CHILD NAMED: ", "ĐÃ ĐẶT TÊN CON: ")) + finalName + "!", 2.8f);
    } else if (namingMode == 3) {
        int slot = std::max(0, std::min(3, namingChildIdx));
        heroGivenName = finalName;
        namingMode = 0;
        SaveGameToSlot(slot);
    } else {
        namingMode = 0;
    }
}

inline void Game::UpdateNameModal() {
    int cp = GetCharPressed();
    while (cp > 0) {
        if (cp >= 0x20 && nameInputBuf.size() < 26) AppendUtf8(nameInputBuf, cp);
        cp = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        PopUtf8Char(nameInputBuf);
    }
    if (IsKeyPressed(KEY_TAB)) {
        nameInputBuf = kHeirNames[rand() % 10];
        PlaySound(snd.sndCoin);
    }
    Vector2 m = GetMousePosition();
    Rectangle okBtn = { 1280 / 2.0f - 210.0f, 488.0f, 195.0f, 44.0f };
    Rectangle randBtn = { 1280 / 2.0f + 15.0f, 488.0f, 195.0f, 44.0f };
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(m, okBtn)) { ConfirmNameModal(); return; }
        if (CheckCollisionPointRec(m, randBtn)) { nameInputBuf = kHeirNames[rand() % 10]; PlaySound(snd.sndCoin); return; }
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        ConfirmNameModal();
    }
}

inline void Game::DrawNameModal() {
    if (namingMode == 0) return;
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.80f));
    int mw = 580, mh = 340, mx = 1280 / 2 - mw / 2, my = 720 / 2 - mh / 2;
    DrawRectangle(mx, my, mw, mh, { 16, 22, 38, 252 });
    DrawRectangleLinesEx({ (float)mx, (float)my, (float)mw, (float)mh }, 2.5f, GOLD);

    const char* hdr = (namingMode == 1) ? L("NAME YOUR HERO", "ĐẶT TÊN CHO NHÂN VẬT")
                    : (namingMode == 2 ? L("NAME YOUR CHILD", "ĐẶT TÊN CHO CON NỐI DÕI")
                                       : L("NAME NEW SAVE FILE", "ĐẶT TÊN BẢN LƯU MỚI"));
    OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 26) / 2, my + 24, 26, GOLD, { 60, 35, 10, 255 });

    int pvClass = player.heroClass;
    int pvSub = player.subClass;
    if (namingMode == 2 && namingChildIdx >= 0 && namingChildIdx < (int)children.size()) {
        pvClass = children[namingChildIdx].heroClass;
        pvSub = 0;
    }
    DrawHeroModel({ 1280 / 2.0f, (float)my + 106.0f }, pvClass, ClassWeaponType(pvClass), { 0.8f, 0.2f }, false, (float)GetTime() * 4.0f, true, 0.3f, false, false, 0.0f, 0.0f, pvSub, 0.0f);

    const char* subHint = (namingMode == 2) ? L("Type a custom name or roll [Tab]:", "Nhập tên cho con bằng bàn phím hoặc [Tab] chọn ngẫu nhiên:")
                                            : L("Type hero/save name or press [Tab] for random:", "Nhập tên nhân vật/save hoặc nhấn [Tab] để chọn tên ngẫu nhiên:");
    DrawText(subHint, 1280 / 2 - MeasureText(subHint, 14) / 2, my + 148, 14, SKYBLUE);

    Rectangle box = { (float)mx + 56.0f, (float)my + 178.0f, (float)mw - 112.0f, 48.0f };
    DrawRectangleRec(box, { 10, 14, 24, 255 });
    DrawRectangleLinesEx(box, 2.0f, SKYBLUE);
    bool blink = ((int)(GetTime() * 2.5f) % 2) == 0;
    std::string disp = nameInputBuf + (blink ? "_" : "");
    DrawText(disp.c_str(), (int)box.x + 16, (int)box.y + 13, 20, WHITE);

    Rectangle okBtn = { 1280 / 2.0f - 210.0f, 488.0f, 195.0f, 44.0f };
    Rectangle randBtn = { 1280 / 2.0f + 15.0f, 488.0f, 195.0f, 44.0f };
    DrawRectangleRec(okBtn, { 34, 98, 56, 255 });
    DrawRectangleLinesEx(okBtn, 2.0f, { 120, 245, 160, 255 });
    const char* okTxt = L("CONFIRM [ENTER]", "XÁC NHẬN [ENTER]");
    DrawText(okTxt, (int)(okBtn.x + okBtn.width / 2) - MeasureText(okTxt, 16) / 2, (int)okBtn.y + 13, 16, WHITE);

    DrawRectangleRec(randBtn, { 42, 56, 98, 255 });
    DrawRectangleLinesEx(randBtn, 1.8f, SKYBLUE);
    const char* rdTxt = L("RANDOM [TAB]", "NGẪU NHIÊN [TAB]");
    DrawText(rdTxt, (int)(randBtn.x + randBtn.width / 2) - MeasureText(rdTxt, 16) / 2, (int)randBtn.y + 13, 16, GOLD);
}

inline float Game::HouseHungerMult() const {
    if (houseTier <= 0) return 1.0f;
    if (houseTier == 1) return 0.85f;
    if (houseTier == 2) return 0.70f;
    return 0.55f;
}

inline int Game::HouseUpgradeCost(int targetTier) const {
    int t = std::max(1, std::min(3, targetTier));
    int base = kHouses[t].cost;
    if (!isNoble) return base;
    return (t == 3) ? base / 2 : (int)roundf(base * 0.70f);
}

inline void Game::ResetDynasty() {
    surfaceUnlocked = false;
    inSurface = false;
    nextDungeonFloor = 11;
    hunger = 100.0f;
    maxHunger = 100.0f;
    starveTickTimer = 0.0f;
    foodRations = 2;
    generation = 1;
    heroGivenName = "";
    houseTier = 0;
    isNoble = false;
    royalQuestActive = false;
    royalQuestKills = 0;
    royalQuestBosses = 0;
    hasSpouse = false;
    spouseIdx = -1;
    for (int i = 0; i < 4; ++i) spouseUnlocked[i] = false;
    spouseQuestProgress = 0;
    hasCaptiveEvent = false;
    captiveRescued = false;
    captiveSpouseIdx = 0;
    timeOfDay = 8.5f;
    dayCount = 1;
    heroAge = 18;
    yearClock = 0.0f;
    lastChildKiller = -1;
    namingMode = 0;
    namingChildIdx = -1;
    nameInputBuf = "";
    children.clear();
    familyTree.clear();
    legacyHp = legacyDmg = legacyArmor = legacySpeed = 0.0f;
    surfaceBuilding = 0;
    surfaceSel = 0;
    surfaceRoom = 0;
    surfaceTownReturnPos = { 0.0f, 180.0f };
}

inline Vector2 Game::SurfaceLandmarkPos(int idx) {
    switch (idx) {
        case 0: return { -420.0f, -180.0f }; // Quán Trọ & Chợ Đồ Ăn
        case 1: return {  420.0f, -180.0f }; // Phường Gia Trang (Mua Nhà)
        case 2: return {  420.0f,  180.0f }; // Tổ Ấm Gia Đình (Cưới Vợ / Sinh Con / An Nghỉ)
        case 3: return { -420.0f,  180.0f }; // Hoàng Cung (Quý Tộc & Ủy Thác Của Vua)
        default: return {   0.0f, -385.0f }; // Cổng Trở Lại Hầm Ngục
    }
}

inline Vector2 Game::TavernStallPos(int i) {
    static const Vector2 pts[4] = {
        { -305.0f,  -65.0f },
        { -105.0f, -155.0f },
        {  105.0f, -155.0f },
        {  305.0f,  -65.0f }
    };
    return pts[std::max(0, std::min(3, i))];
}

inline Vector2 Game::SpouseCandPos(int i) {
    static const Vector2 pts[4] = {
        { -355.0f, -185.0f },
        { -185.0f, -225.0f },
        { -185.0f,  -45.0f },
        { -355.0f,   15.0f }
    };
    return pts[std::max(0, std::min(3, i))];
}

inline void Game::SetupSurfaceRoomEnvironment(int roomIdx, bool movePlayer) {
    surfaceRoom = std::max(0, std::min(4, roomIdx));
    if (surfaceRoom >= 1) surfaceBuilding = surfaceRoom - 1;
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
    barrels.clear();
    hazards.clear();
    portalChoices.clear();
    wave2Spawns.clear();
    chests.clear();
    wave2Triggered = false;
    hasTrialTotem = false;
    trialUsed = false;
    trialActive = false;
    shopFloor = false;
    hasShrine = false;
    portalOpen = false;
    player.isDashing = false;
    for (auto& ch : children) ch.active = false; // con cái ở nhà khi lên Mặt Đất

    if (surfaceRoom == 0) {
        roomW = 1560.0f;
        roomH = 1040.0f;
        pillars = {
            { -195.0f, -145.0f }, {  195.0f, -145.0f },
            { -195.0f,  145.0f }, {  195.0f,  145.0f },
            { -650.0f, -380.0f }, {  650.0f, -380.0f },
            { -650.0f,  380.0f }, {  650.0f,  380.0f }
        };
    } else if (surfaceRoom == 1) {
        roomW = 1160.0f;
        roomH = 840.0f;
        pillars = {
            { -415.0f, -230.0f }, {  415.0f, -230.0f },
            { -415.0f,  210.0f }, {  415.0f,  210.0f }
        };
    } else if (surfaceRoom == 2) {
        roomW = 1200.0f;
        roomH = 860.0f;
        pillars = {
            { -435.0f, -245.0f }, {  435.0f, -245.0f },
            { -435.0f,  220.0f }, {  435.0f,  220.0f }
        };
    } else if (surfaceRoom == 3) {
        if (houseTier <= 0) { roomW = 1120.0f; roomH = 820.0f; }
        else if (houseTier == 1) { roomW = 1200.0f; roomH = 860.0f; }
        else if (houseTier == 2) { roomW = 1300.0f; roomH = 900.0f; }
        else { roomW = 1420.0f; roomH = 960.0f; }
        float px = roomW * 0.39f, py = roomH * 0.30f;
        pillars = { { -px, -py }, { px, -py }, { -px, py }, { px, py } };
    } else {
        roomW = 1380.0f;
        roomH = 960.0f;
        pillars = {
            { -425.0f, -265.0f }, {  425.0f, -265.0f },
            { -425.0f,    0.0f }, {  425.0f,    0.0f },
            { -425.0f,  265.0f }, {  425.0f,  265.0f }
        };
    }

    BuildDecor();
    if (surfaceRoom == 0) {
        for (int bi = 0; bi < 4; ++bi) {
            Vector2 bp = SurfaceLandmarkPos(bi);
            torches.push_back({ bp.x - 62.0f, bp.y + 42.0f });
            torches.push_back({ bp.x + 62.0f, bp.y + 42.0f });
        }
        torches.push_back({ -82.0f, -345.0f });
        torches.push_back({  82.0f, -345.0f });
    } else if (surfaceRoom == 1) {
        torches.push_back({ -64.0f, -roomH / 2.0f + 42.0f });
        torches.push_back({  64.0f, -roomH / 2.0f + 42.0f });
        torches.push_back({ -360.0f, -65.0f });
        torches.push_back({  360.0f, -65.0f });
    } else if (surfaceRoom == 2) {
        torches.push_back({ -175.0f, -255.0f });
        torches.push_back({  175.0f, -255.0f });
        torches.push_back({ -355.0f,  -55.0f });
        torches.push_back({  355.0f,  -55.0f });
    } else if (surfaceRoom == 3) {
        torches.push_back({ -300.0f, -275.0f });
        torches.push_back({ -170.0f, -275.0f });
        torches.push_back({  210.0f, -250.0f });
        torches.push_back({  340.0f, -250.0f });
    } else if (surfaceRoom == 4) {
        torches.push_back({ -140.0f, -310.0f });
        torches.push_back({  140.0f, -310.0f });
        torches.push_back({ -130.0f,  -40.0f });
        torches.push_back({  130.0f,  -40.0f });
        torches.push_back({ -130.0f,  210.0f });
        torches.push_back({  130.0f,  210.0f });
    }
    InitMotes();

    if (movePlayer) {
        if (surfaceRoom == 0) player.pos = surfaceTownReturnPos;
        else player.pos = { 0.0f, roomH / 2.0f - 130.0f };
    }
    camera.target = player.pos;
}

inline void Game::EnterSurfaceRoom(int roomIdx) {
    if (!inSurface) return;
    int r = std::max(1, std::min(4, roomIdx));
    surfaceTownReturnPos = SurfaceTownExitSpot(r);
    SetupSurfaceRoomEnvironment(r, true);
    PlaySound(snd.sndCoin);
    Burst(player.pos, GOLD, 14, 150.0f, 0.45f);
    static const char* roomNamesEn[4] = {
        "ENTERED: GOLDENHEARTH TAVERN & MARKET",
        "ENTERED: MASONS & ESTATE GUILD",
        "ENTERED: FAMILY MANOR & ANCESTRAL HALL",
        "ENTERED: ROYAL PALACE THRONE HALL"
    };
    static const char* roomNamesVi[4] = {
        "VÀO PHÒNG: QUÁN TRỌ & CHỢ ẨM THỰC",
        "VÀO PHÒNG: PHƯỜNG KIẾN TRÚC GIA TRANG",
        "VÀO PHÒNG: TỔ ẤM GIA ĐÌNH & TỪ ĐƯỜNG",
        "VÀO PHÒNG: ĐẠI ĐIỆN HOÀNG CUNG"
    };
    ShowBanner(L(roomNamesEn[r - 1], roomNamesVi[r - 1]), 2.4f);
}

inline void Game::LeaveSurfaceRoomToTown() {
    if (!inSurface || surfaceRoom == 0) return;
    surfaceTownReturnPos = SurfaceTownExitSpot(surfaceRoom);
    SetupSurfaceRoomEnvironment(0, true);
    PlaySound(snd.sndCoin);
    Burst(player.pos, SKYBLUE, 12, 140.0f, 0.4f);
    ShowBanner(L("RETURNED TO TOWN SQUARE", "RA QUẢNG TRƯỜNG KINH THÀNH"), 1.8f);
}

inline void Game::OpenSurfaceModal(int tab) {
    surfaceBuilding = std::max(0, std::min(3, tab));
    surfaceSel = 0;
    StopRumble();
    state = SURFACE_UI;
    PlaySound(snd.sndCoin);
}

inline void Game::OpenFamilyTree(State from) {
    familyTreeReturn = from;
    StopRumble();
    state = FAMILY_TREE;
    PlaySound(snd.sndCoin);
}

inline void Game::EnterSurface(int nextF) {
    surfaceUnlocked = true;
    inSurface = true;
    nextDungeonFloor = std::max(nextDungeonFloor, std::max(1, nextF));
    surfaceTownReturnPos = { 0.0f, 220.0f };
    SetupSurfaceRoomEnvironment(0, true);
    PlaySound(snd.sndPortal);
    ShowBanner(L("SURFACE REACHED! Explore each building or press [1..4] to step inside!",
                 "ĐÃ LÊN MẶT ĐẤT! Bước vào từng công trình (hoặc nhấn [1..4]) để khám phá không gian riêng!"), 4.2f);
}

inline void Game::LeaveSurfaceToDungeon() {
    inSurface = false;
    roomW = 1200.0f;
    roomH = 750.0f;
    floor = std::max(1, nextDungeonFloor);
    PlaySound(snd.sndPortal);
    BuildFloor();
}

inline void Game::EatRation() {
    if (!surfaceUnlocked) return;
    if (foodRations <= 0) {
        ShowBanner(L("No Rations left! Buy food on the Surface.", "Hết Lương Khô! Hãy lên Mặt Đất mua đồ ăn."), 2.2f);
        PlaySound(snd.sndHurt);
        return;
    }
    if (hunger >= maxHunger - 0.5f && player.hp >= player.maxHp - 0.5f) {
        ShowBanner(L("Already Full & Healthy!", "Đã no và đầy máu!"), 1.5f);
        return;
    }
    foodRations--;
    hunger = std::min(maxHunger, hunger + 45.0f);
    AdvanceTime(0.5f);
    float hl = roundf(player.maxHp * 0.25f);
    player.hp = std::min(player.maxHp, player.hp + hl);
    PlaySound(snd.sndPotion);
    Burst(player.pos, { 120, 240, 150, 255 }, 14, 180.0f, 0.5f);
    SpawnPopup(player.pos, L("+45 HUNGER", "+45 NO"), { 130, 245, 150, 255 });
}

inline void Game::BuyMeal(int kind) {
    static const int basePrices[4] = { 15, 35, 70, 25 };
    if (kind < 0 || kind > 3) return;
    int price = MealPrice(basePrices[kind]);
    if (player.gold < price) {
        ShowBanner(L("Not enough Gold!", "Không đủ Vàng!"), 1.8f);
        PlaySound(snd.sndHurt);
        return;
    }
    player.gold -= price;
    PlaySound(snd.sndPotion);
    AdvanceTime(1.0f);
    if (kind == 0) {
        hunger = std::min(maxHunger, hunger + 35.0f);
        player.hp = std::min(player.maxHp, player.hp + 18.0f);
        ShowBanner(L("Ate Fresh Bread (+35 Hunger, +18 HP)", "Đã ăn Bánh Mì (+35 No, +18 Máu)"), 2.2f);
    } else if (kind == 1) {
        hunger = std::min(maxHunger, hunger + 70.0f);
        player.hp = std::min(player.maxHp, player.hp + roundf(player.maxHp * 0.40f));
        ShowBanner(L("Ate Roast Meat (+70 Hunger, +40% HP)", "Đã ăn Thịt Nướng Đậm Đà (+70 No, +40% Máu)"), 2.2f);
    } else if (kind == 2) {
        hunger = maxHunger;
        player.hp = player.maxHp;
        AddRage(25.0f);
        ShowBanner(L("Royal Banquet Feast (Full Hunger & HP!)", "Yến Tiệc Thịnh Soạn (Đầy No & Đầy Máu!)"), 2.4f);
    } else {
        foodRations += 2;
        ShowBanner(L("Bought +2 Travel Rations [H]", "Đã mua +2 Lương Khô Mang Theo [H]"), 2.2f);
    }
}

inline void Game::BuyHouseUpgrade(int targetTier) {
    if (targetTier <= houseTier || targetTier > 3) {
        ShowBanner(L("You already own this or higher house tier!", "Bạn đã sở hữu cấp nhà này hoặc cao hơn!"), 2.0f);
        return;
    }
    if (targetTier != houseTier + 1) {
        ShowBanner(L("Upgrade house tiers step by step!", "Hãy nâng cấp từng bậc nhà!"), 2.0f);
        return;
    }
    int cost = HouseUpgradeCost(targetTier);
    if (player.gold < cost) {
        ShowBanner(L("Not enough Gold to buy this house!", "Chưa đủ Vàng mua ngôi nhà này!"), 2.0f);
        PlaySound(snd.sndHurt);
        return;
    }
    player.gold -= cost;
    float hpGain = kHouses[targetTier].hpBonus - kHouses[houseTier].hpBonus;
    float dmgGain = kHouses[targetTier].dmgBonus - kHouses[houseTier].dmgBonus;
    houseTier = targetTier;
    player.maxHp += hpGain;
    player.hp = std::min(player.maxHp, player.hp + hpGain);
    player.baseDmg += dmgGain;
    if (inSurface && surfaceRoom == 3) SetupSurfaceRoomEnvironment(3, false);
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, GOLD, 24, 240.0f, 0.7f);
    ShowBanner(std::string(L("PURCHASED HOME: ", "ĐÃ MUA NHÀ: ")) + HouseName(targetTier) + "!", 3.2f);
}

inline void Game::TryMarrySpouse(int idx) {
    if (houseTier <= 0) {
        ShowBanner(L("Buy a House first before getting married!", "Hãy mua Nhà trước khi Cưới Vợ!"), 2.5f);
        PlaySound(snd.sndHurt);
        return;
    }
    if (hasSpouse) {
        ShowBanner(L("You are already happily married!", "Bạn đã lập gia đình rồi!"), 2.2f);
        return;
    }
    if (idx < 0 || idx >= 4) return;
    if (!spouseUnlocked[idx]) {
        ShowBanner(std::string(L("Cannot buy a spouse! ", "Không thể dùng Vàng mua vợ! ")) + SpouseUnlockConditionText(idx), 3.4f);
        PlaySound(snd.sndHurt);
        return;
    }
    hasSpouse = true;
    spouseIdx = idx;
    AdvanceTime(2.0f);
    const SpouseDef& sp = kSpouses[idx];
    player.maxHp += sp.hpBonus;
    player.hp = std::min(player.maxHp, player.hp + sp.hpBonus);
    player.baseDmg += sp.dmgBonus;
    player.critChance = std::min(0.6f, player.critChance + sp.critBonus);
    player.attackRate += sp.atkSpdBonus;
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, sp.dressCol, 32, 260.0f, 0.8f);
    ShowBanner(std::string(L("MARRIED ", "KẾT HÔN CÙNG ")) + SpouseName(idx) + "!", 3.4f);
}

inline void Game::TryHaveChild() {
    if (!hasSpouse || houseTier <= 0) {
        ShowBanner(L("Requires a House and Marriage first!", "Cần có Nhà và đã Cưới Vợ để sinh con!"), 2.4f);
        PlaySound(snd.sndHurt);
        return;
    }
    int maxKids = kHouses[std::max(0, std::min(3, houseTier))].maxKids;
    if ((int)children.size() >= maxKids) {
        ShowBanner(L("Home is at full child capacity — upgrade House!", "Nhà đã đủ số con tối đa — hãy nâng cấp Nhà lớn hơn!"), 2.6f);
        PlaySound(snd.sndHurt);
        return;
    }
    int cost = MealPrice(100);
    if (player.gold < cost) {
        ShowBanner(L("Need more Gold to welcome a child!", "Không đủ Vàng cho chi phí nuôi dạy con!"), 2.0f);
        PlaySound(snd.sndHurt);
        return;
    }
    player.gold -= cost;
    AdvanceTime(1.5f);
    ChildInfo ch;
    int nameIdx = (generation * 3 + (int)children.size() + (int)familyTree.size()) % 10;
    ch.name = kHeirNames[nameIdx];
    ch.heroClass = (player.heroClass + (int)children.size() + 1) % 4;
    ch.bonusHp = 18.0f + houseTier * 8.0f + (isNoble ? 14.0f : 0.0f) + (spouseIdx >= 0 ? kSpouses[spouseIdx].hpBonus * 0.45f : 0.0f);
    ch.bonusDmg = 3.0f + houseTier * 2.0f + (isNoble ? 3.5f : 0.0f) + (spouseIdx >= 0 ? kSpouses[spouseIdx].dmgBonus * 0.50f : 0.0f);
    ch.bonusArmor = 0.5f + (isNoble ? 0.6f : 0.0f);
    ch.bonusSpeed = 8.0f + houseTier * 4.0f;
    children.push_back(ch);
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, SKYBLUE, 28, 250.0f, 0.75f);
    ShowBanner(std::string(L("CHILD BORN: ", "SINH CON THÀNH CÔNG: ")) + ch.name + " (" + ClassDisplayName(ch.heroClass) + ")!", 3.5f);
    if (!ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
        OpenNameModal(2, (int)children.size() - 1, ch.name);
    }
}

inline void Game::AcceptRoyalCommission() {
    if (isNoble) {
        ShowBanner(L("You already hold a Noble Title!", "Bạn đã mang Tước Vị Quý Tộc rồi!"), 2.2f);
        return;
    }
    if (royalQuestActive) {
        if (royalQuestKills >= 35 && royalQuestBosses >= 1) CompleteRoyalCommission();
        else ShowBanner(L("Commission active! Slay Dungeon monsters & 1 Boss!", "Đang nhận Ủy Thác! Hãy xuống Hầm Ngục diệt 35 Quái & 1 Trùm!"), 2.8f);
        return;
    }
    royalQuestActive = true;
    royalQuestKills = 0;
    royalQuestBosses = 0;
    PlaySound(snd.sndSpecial);
    ShowBanner(L("KING'S COMMISSION ACCEPTED: Slay 35 Enemies & 1 Boss in the Dungeon!",
                 "ĐÃ NHẬN ỦY THÁC CỦA VUA: Hạ 35 Quái & 1 Trùm dưới Hầm Ngục để phong Quý Tộc!"), 3.8f);
}

inline void Game::CompleteRoyalCommission() {
    if (isNoble) return;
    isNoble = true;
    royalQuestActive = false;
    player.gold += 300;
    player.maxHp += 35.0f;
    player.hp = std::min(player.maxHp, player.hp + 35.0f);
    player.baseDmg += 6.0f;
    player.armor += 1.5f;
    GiveRandomRelic();
    PlaySound(snd.sndLevelUp);
    Shake(0.35f, 6.0f);
    UnlockSpouse(3, "ROYAL DECREE: NOBILITY & PRINCESS BAO NGOC BETROTHED TO YOU!",
                    "NHIỆM VỤ HOÀNG GIA HOÀN TẤT: PHONG QUÝ TỘC & VUA GẢ CÔNG CHÚA BẢO NGỌC!");
}

inline void Game::BuyNobleTitleDirect() {
    if (isNoble) {
        ShowBanner(L("You are already Noble!", "Bạn đã là Quý Tộc rồi!"), 2.0f);
        return;
    }
    const int noblePrice = 1500;
    if (player.gold < noblePrice) {
        ShowBanner(L("Need 1500 Gold to buy Nobility directly (or complete the King's Commission)!",
                     "Cần 1500 Vàng để mua Tước Quý Tộc trực tiếp (hoặc làm Ủy Thác của Vua)!"), 3.0f);
        PlaySound(snd.sndHurt);
        return;
    }
    player.gold -= noblePrice;
    isNoble = true;
    royalQuestActive = false;
    player.maxHp += 35.0f;
    player.hp = std::min(player.maxHp, player.hp + 35.0f);
    player.baseDmg += 6.0f;
    player.armor += 1.5f;
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, GOLD, 36, 300.0f, 0.9f);
    ShowBanner(L("ROYAL PROMOTION: PURCHASED NOBLE TITLE! (+Stats, +25% Gold, Discounts)",
                 "SẮC PHONG QUÝ TỘC THÀNH CÔNG! (+Chỉ số, +25% Vàng, Giảm giá 30-50%)"), 4.0f);
}

inline void Game::ApplyEquipEnhanceStep(int s) {
    if (s < 0 || s >= 4 || !hasEquip[s] || equipPlus[s] >= 5) return;
    ApplyItem(equipped[s], -1.0f);
    if (equipped[s].damage > 0) equipped[s].damage = roundf(equipped[s].damage * 1.22f + 1.0f);
    if (equipped[s].maxHp > 0) equipped[s].maxHp = roundf(equipped[s].maxHp * 1.22f + 3.0f);
    if (equipped[s].armor > 0) equipped[s].armor = R1(equipped[s].armor * 1.22f + 0.5f);
    if (equipped[s].attackRate > 0) equipped[s].attackRate = R1(equipped[s].attackRate * 1.12f + 0.05f);
    if (equipped[s].crit > 0) equipped[s].crit = std::min(0.35f, equipped[s].crit + 0.02f);
    equipPlus[s]++;
    ApplyItem(equipped[s], 1.0f);
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, GOLD, 24, 240.0f, 0.6f);
    ShowBanner(TextFormat(L("FORGED %s TO +%d!", "CƯỜNG HÓA %s LÊN +%d THÀNH CÔNG!"),
                          equipped[s].name.c_str(), equipPlus[s]), 3.0f);
}

inline void Game::ReforgeEquipSlot(int s) {
    if (s < 0 || s >= 4 || !hasEquip[s]) {
        ShowBanner(L("No gear equipped in that slot!", "Chưa mặc trang bị ở ô này!"), 2.0f);
        return;
    }
    if (equipPlus[s] >= 5) {
        ShowBanner(L("Equipment already at +5 Max Enhancement!", "Trang bị đã đạt cường hóa tối đa +5!"), 2.2f);
        return;
    }
    int goldCost = MealPrice(90 + equipPlus[s] * 70);
    int essCost = (equipPlus[s] >= 2) ? 1 : 0;
    if (player.gold < goldCost || bossEssence < essCost) {
        ShowBanner(TextFormat(L("Need %dG%s to reinforce!", "Cần %d Vàng%s để cường hóa!"),
                              goldCost, essCost > 0 ? L(" + 1 Boss Essence", " + 1 Tinh Hoa Trùm") : ""), 2.4f);
        PlaySound(snd.sndHurt);
        return;
    }
    player.gold -= goldCost;
    bossEssence -= essCost;
    AdvanceTime(1.0f);
    ApplyEquipEnhanceStep(s);
}

inline void Game::AcceptOrClaimBounty() {
    if (!bountyActive) {
        bountyActive = true;
        bountyKind = rand() % 3;
        bountyProgress = 0;
        bountyTarget = (bountyKind == 0) ? 8 : (bountyKind == 1 ? 12 : 1);
        bountyRewardGold = (bountyKind == 2) ? 280 : 210;
        PlaySound(snd.sndSpecial);
        ShowBanner(L("ACCEPTED TAVERN BOUNTY! Check Tavern / HUD for progress.",
                     "ĐÃ NHẬN LỆNH TRUY NÃ QUÁN TRỌ! Hoàn thành để nhận Vàng & Tinh Hoa!"), 3.0f);
        return;
    }
    if (bountyProgress >= bountyTarget) {
        player.gold += bountyRewardGold;
        int ess = (bountyKind == 2) ? 2 : 1;
        bossEssence += ess;
        bountyActive = false;
        bountyProgress = 0;
        PlaySound(snd.sndLevelUp);
        Burst(player.pos, GOLD, 28, 250.0f, 0.65f);
        ShowBanner(TextFormat(L("BOUNTY CLAIMED: +%dG & +%d Boss Essence!",
                                "TRẢ TRUY NÃ THÀNH CÔNG: +%d Vàng & +%d Tinh Hoa Trùm!"), bountyRewardGold, ess), 3.4f);
    } else {
        ShowBanner(TextFormat(L("Bounty Progress: %d / %d", "Tiến độ Truy Nã: %d / %d"), bountyProgress, bountyTarget), 2.2f);
    }
}

inline void Game::SuccessionToChild(int childIdx, bool forced) {
    if (childIdx < 0 || childIdx >= (int)children.size()) return;
    if (!forced && children[childIdx].age < (float)kChildAdultAge) {
        ShowBanner(TextFormat(L("%s is only %d years old — heirs must be %d+ to inherit (1 year = 3 min).",
                                "%s mới %d tuổi — phải đủ %d tuổi mới được kế thừa (1 năm = 3 phút)."),
                              children[childIdx].name.c_str(), ChildAgeYears(children[childIdx]), kChildAdultAge), 3.4f);
        PlaySound(snd.sndHurt);
        return;
    }
    ChildInfo heir = children[childIdx];

    AncestorRecord rec;
    rec.generation = generation;
    rec.name = CurrentHeroDisplayName();
    rec.heroClass = player.heroClass;
    rec.subClass = player.subClass;
    rec.wasNoble = isNoble;
    rec.spouseName = (hasSpouse && spouseIdx >= 0) ? SpouseName(spouseIdx) : L("Unmarried", "Độc Thân");
    rec.floorReached = std::max(floor, nextDungeonFloor);
    rec.levelReached = player.level;
    rec.enemiesSlain = enemiesSlain;
    familyTree.push_back(rec);

    int keepGold = player.gold;
    std::vector<Relic> keepRelics = player.relics;
    EquipItem keepEq[4];
    bool keepHas[4] = { false, false, false, false };
    for (int i = 0; i < 4; ++i) {
        keepHas[i] = hasEquip[i];
        if (hasEquip[i]) keepEq[i] = equipped[i];
    }

    generation++;
    heroGivenName = heir.name;
    bloodlineTrait = heir.heroClass % 4;
    legacyHp += heir.bonusHp;
    legacyDmg += heir.bonusDmg;
    legacyArmor += heir.bonusArmor;
    legacySpeed += heir.bonusSpeed;
    hasSpouse = false;
    spouseIdx = -1;
    for (int i = 0; i < 4; ++i) spouseUnlocked[i] = false;
    spouseQuestProgress = 0;
    hasCaptiveEvent = false;
    captiveRescued = false;
    captiveSpouseIdx = 0;
    dayCount += 30;
    timeOfDay = 8.5f;
    heroAge = std::max(kChildAdultAge, ChildAgeYears(heir));
    yearClock = 0.0f;
    lastChildKiller = -1;
    namingMode = 0;
    namingChildIdx = -1;
    nameInputBuf = "";
    children.clear();

    player = Player();
    int hc = std::max(0, std::min(3, heir.heroClass));
    selectedClass = hc;
    const ClassDef& cd = kClasses[hc];
    player.heroClass = hc;
    player.subClass = 0;
    player.maxHp = cd.baseHp + 10.0f * meta[0] + kHouses[houseTier].hpBonus + (isNoble ? 35.0f : 0.0f) + legacyHp;
    player.baseDmg = (cd.baseDmg - 3.0f) + 2.0f * meta[1] + kHouses[houseTier].dmgBonus + (isNoble ? 6.0f : 0.0f) + legacyDmg;
    player.speed = cd.baseSpeed + 5.0f * meta[2] + legacySpeed;
    player.attackRate = cd.baseAtkRate;
    player.armor = cd.baseArmor + (isNoble ? 1.5f : 0.0f) + legacyArmor;
    player.critChance = cd.baseCrit;
    player.specialCooldown = Skill1MaxCd();
    player.skill2Cooldown = Skill2MaxCd();
    player.skill3Cooldown = Skill3MaxCd();
    player.skill4Cooldown = Skill4MaxCd();
    player.gold = keepGold;
    player.relics = keepRelics;
    for (const auto& r : player.relics) if (r.type == RELIC_AEGIS) player.shieldActive = true;

    for (int i = 0; i < 4; ++i) hasEquip[i] = false;
    int dummySold = 0;
    if (keepHas[SLOT_WEAPON]) TryEquipItem(keepEq[SLOT_WEAPON], dummySold);
    else TryEquipItem(MakeStarterWeapon(cd.startWeapon), dummySold);
    for (int i = 1; i < 4; ++i) if (keepHas[i]) TryEquipItem(keepEq[i], dummySold);

    player.hp = player.maxHp;
    player.hpDisplay = player.hp;
    hunger = maxHunger;
    starveTickTimer = 0.0f;
    phoenixUsed = false;
    subclassChoosing = false;
    enemiesSlain = 0;
    comboCount = 0;
    surfaceUnlocked = true;
    inSurface = true;
    floor = std::max(1, nextDungeonFloor);
    surfaceTownReturnPos = SurfaceTownExitSpot(3);
    SetupSurfaceRoomEnvironment(3, true);
    state = PLAYING;
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, GOLD, 36, 300.0f, 0.9f);
    ShowBanner(std::string(L("REST IN PEACE! Gen ", "AN NGHỈ! Đời ")) + std::to_string(generation) + ": " + heir.name + L(" inherits the Bloodline!", " chính thức kế thừa gia tộc!"), 4.2f);
}

inline void Game::ToggleChildParty(int idx) {
    if (idx < 0 || idx >= (int)children.size()) return;
    ChildInfo& ch = children[idx];
    if (ch.inParty) {
        ch.inParty = false;
        ch.active = false;
        PlaySound(snd.sndCoin);
        ShowBanner(ch.name + L(" will stay home.", " sẽ ở nhà."), 2.0f);
        return;
    }
    if (ch.age < (float)kChildTrainAge) {
        ShowBanner(TextFormat(L("%s is %d — must be %d+ to enter the Dungeon (1 year = 3 min).",
                                "%s mới %d tuổi — cần đủ %d tuổi mới được xuống Hầm Ngục (1 năm = 3 phút)."),
                              ch.name.c_str(), ChildAgeYears(ch), kChildTrainAge), 3.2f);
        PlaySound(snd.sndHurt);
        return;
    }
    if (ch.injuredTimer > 0.0f) {
        ShowBanner(TextFormat(L("%s is still recovering from wounds (%ds).", "%s vẫn đang dưỡng thương (%ds)."),
                              ch.name.c_str(), (int)ceilf(ch.injuredTimer)), 2.6f);
        PlaySound(snd.sndHurt);
        return;
    }
    ch.inParty = true;
    PlaySound(snd.sndLevelUp);
    Burst(player.pos, kClasses[std::max(0, std::min(3, ch.heroClass))].color, 18, 200.0f, 0.55f);
    ShowBanner(ch.name + L(" will follow you into the Dungeon to train!", " sẽ theo cha xuống Hầm Ngục rèn luyện!"), 2.8f);
}

inline void Game::SpawnPartyChildren() {
    int slot = 0;
    for (size_t k = 0; k < children.size(); ++k) {
        ChildInfo& ch = children[k];
        bool wasActive = ch.active;
        ch.active = !inSurface && ch.inParty && ChildCanTrain(ch);
        if (!ch.active) continue;
        ch.maxHp = ChildMaxHpStat(ch);
        ch.hp = wasActive ? std::min(ch.maxHp, ch.hp + ch.maxHp * 0.35f) : ch.maxHp;
        ch.pos = Vector2Add(player.pos, ChildFormationOffset(slot++));
        ch.atkTimer = 0.4f;
        ch.invuln = 1.0f;
        ch.swing = 0.0f;
    }
}

inline void Game::ChildGainXp(ChildInfo& ch, int amount) {
    if (amount <= 0 || ch.level >= kChildMaxLevel) return;
    ch.xp += amount;
    while (ch.level < kChildMaxLevel && ch.xp >= ChildXpNext(ch.level)) {
        ch.xp -= ChildXpNext(ch.level);
        ch.level++;
        // Thành quả rèn luyện được ghi vào chỉ số kế thừa
        ch.bonusHp += 2.5f;
        ch.bonusDmg += 0.5f;
        ch.bonusArmor += 0.04f;
        ch.bonusSpeed += 0.4f;
        float oldMax = ch.maxHp;
        ch.maxHp = ChildMaxHpStat(ch);
        ch.hp = std::min(ch.maxHp, ch.hp + std::max(0.0f, ch.maxHp - oldMax) + ch.maxHp * 0.25f);
        if (ch.active) {
            SpawnPopup({ ch.pos.x - 10.0f, ch.pos.y - 34.0f }, TextFormat("%s LV %d!", ch.name.c_str(), ch.level), { 140, 255, 190, 255 });
            Burst(ch.pos, { 140, 255, 190, 255 }, 14, 180.0f, 0.5f);
        }
        PlaySound(snd.sndCoin);
    }
    if (ch.level >= kChildMaxLevel) ch.xp = 0;
}

inline void Game::GrantPartyXp(const Enemy& e) {
    int base = (e.type == BOSS) ? 60 : (e.isWarden ? 25 : (e.isElite ? 12 : ((e.isMinion || (e.type == SPLITTER && e.splitLevel > 0)) ? 2 : 6)));
    for (size_t k = 0; k < children.size(); ++k) {
        ChildInfo& ch = children[k];
        if (!ch.active) continue;
        ChildGainXp(ch, base + ((int)k == lastChildKiller ? base : 0));
    }
}

inline void Game::ChildDamageEnemy(Enemy& e, float dmg, int ci, Vector2 kb) {
    if (e.hp <= 0.0f || ci < 0 || ci >= (int)children.size()) return;
    const ChildInfo& ch = children[ci];
    bool crit = Rand01() < (0.08f + ch.level * 0.004f);
    if (crit) dmg *= 1.8f;
    if (e.stunTimer > 0.0f) dmg *= 1.20f;
    e.hp -= dmg;
    e.pos = Vector2Add(e.pos, Vector2Scale(kb, 4.0f));
    e.hitFlash = 0.09f;
    Color cc = kClasses[std::max(0, std::min(3, ch.heroClass))].color;
    SpawnPopup({ e.pos.x + 8.0f, e.pos.y - 6.0f }, (crit ? "!" : "") + std::to_string((int)dmg), crit ? YELLOW : LerpColor(cc, WHITE, 0.45f));
    Burst(e.pos, cc, crit ? 7 : 3, 150.0f, 0.25f);
    if (e.hp <= 0.0f) {
        lastChildKiller = ci;
        OnEnemyKilled(e);
        lastChildKiller = -1;
    }
}

inline void Game::ChildTakeDamage(int ci, float dmg) {
    if (ci < 0 || ci >= (int)children.size()) return;
    ChildInfo& ch = children[ci];
    if (!ch.active || ch.invuln > 0.0f) return;
    float eff = std::max(1.0f, dmg * 100.0f / (100.0f + ChildArmorStat(ch) * 10.0f));
    ch.hp -= eff;
    ch.invuln = 0.55f;
    SpawnPopup(ch.pos, std::to_string((int)eff), { 255, 120, 150, 255 });
    if (ch.hp <= 0.0f) {
        ch.hp = 0.0f;
        ch.active = false;
        ch.inParty = false;
        ch.injuredTimer = kChildInjuryTime;
        Burst(ch.pos, WHITE, 26, 240.0f, 0.7f);
        PlaySound(snd.sndHurt);
        Shake(0.2f, 4.0f);
        ShowBanner(ch.name + TextFormat(L(" was badly wounded! Carried home to recover (%ds).",
                                          " bị trọng thương! Đã được đưa về Tổ Ấm dưỡng thương (%ds)."), (int)kChildInjuryTime), 3.4f);
    }
}

inline void Game::UpdatePartyChildren(float dt) {
    if (inSurface) return;
    int slot = 0;
    bool roomQuiet = true;
    for (const auto& e : enemies) if (e.hp > 0.0f) { roomQuiet = false; break; }
    for (size_t k = 0; k < children.size(); ++k) {
        ChildInfo& ch = children[k];
        if (!ch.active) continue;
        int mySlot = slot++;
        int hc = std::max(0, std::min(3, ch.heroClass));
        bool ranged = ChildIsRanged(hc);
        if (ch.atkTimer > 0.0f) ch.atkTimer -= dt;
        if (ch.invuln > 0.0f) ch.invuln -= dt;
        if (ch.swing > 0.0f) ch.swing -= dt;
        ch.maxHp = ChildMaxHpStat(ch);
        if (ch.hp > ch.maxHp) ch.hp = ch.maxHp;
        if (roomQuiet) ch.hp = std::min(ch.maxHp, ch.hp + ch.maxHp * 0.06f * dt);

        // Chọn mục tiêu: quái gần con nhất trong phạm vi bảo vệ quanh cha
        Enemy* target = nullptr;
        float bestD = 1e9f;
        for (auto& e : enemies) {
            if (e.hp <= 0.0f) continue;
            if (Vector2Distance(e.pos, player.pos) > 520.0f) continue;
            float d = Vector2Distance(e.pos, ch.pos);
            if (d < bestD) { bestD = d; target = &e; }
        }

        float spd = std::max(200.0f, player.speed * 0.95f + ch.bonusSpeed * 0.3f);
        Vector2 goal = Vector2Add(player.pos, ChildFormationOffset(mySlot));
        float distToPlayer = Vector2Distance(ch.pos, player.pos);
        bool leash = distToPlayer > 480.0f;
        if (distToPlayer > 900.0f) { // lạc quá xa: chạy tới bên cha ngay
            ch.pos = goal;
            Burst(ch.pos, SKYBLUE, 8, 120.0f, 0.3f);
        }

        Vector2 moveDir = { 0.0f, 0.0f };
        if (target && !leash) {
            Vector2 to = Vector2Subtract(target->pos, ch.pos);
            float d = Vector2Length(to);
            Vector2 dir = d > 0.01f ? Vector2Scale(to, 1.0f / d) : Vector2{ 1.0f, 0.0f };
            ch.aim = dir;
            float range = ranged ? 250.0f : (target->radius + 24.0f);
            if (d > range) moveDir = dir;
            else if (ranged && d < 130.0f) moveDir = Vector2Scale(dir, -0.8f);
            if (d <= range + 12.0f && ch.atkTimer <= 0.0f) {
                ch.atkTimer = ChildAttackInterval(ch);
                ch.swing = 0.18f;
                float dmg = ChildDamageStat(ch);
                Color cc = kClasses[hc].color;
                if (ranged) {
                    Projectile p;
                    p.fromPlayer = true;
                    p.kind = (hc == CLASS_RANGER) ? 1 : 2;
                    p.pos = Vector2Add(ch.pos, Vector2Scale(dir, 14.0f));
                    p.vel = Vector2Scale(dir, 540.0f);
                    p.damage = dmg;
                    p.life = 1.1f;
                    p.radius = 6.0f;
                    p.color = LerpColor(cc, WHITE, 0.25f);
                    p.childOwner = (int)k;
                    projectiles.push_back(p);
                } else {
                    ChildDamageEnemy(*target, dmg, (int)k, dir);
                    // Kiếm sĩ nhí chém lan nhẹ quanh mục tiêu
                    if (hc == CLASS_KNIGHT) {
                        for (auto& o : enemies) {
                            if (&o == target || o.hp <= 0.0f) continue;
                            if (Vector2Distance(o.pos, target->pos) < 55.0f + o.radius)
                                ChildDamageEnemy(o, dmg * 0.45f, (int)k, Vector2Normalize(Vector2Subtract(o.pos, ch.pos)));
                        }
                    }
                    NovaVisual nv;
                    nv.pos = Vector2Add(ch.pos, Vector2Scale(dir, 18.0f));
                    nv.radius = 0.0f; nv.maxRadius = 34.0f; nv.timer = 0.16f; nv.maxTimer = 0.16f; nv.color = cc;
                    novas.push_back(nv);
                }
            }
        } else {
            Vector2 to = Vector2Subtract(goal, ch.pos);
            float d = Vector2Length(to);
            if (d > 22.0f) moveDir = Vector2Scale(to, 1.0f / d);
            if (d > 220.0f) spd *= 1.35f;
            if (Vector2Length(player.vel) > 1.0f || d > 22.0f) ch.aim = d > 0.01f ? Vector2Scale(to, 1.0f / std::max(0.01f, d)) : ch.aim;
        }
        ch.moving = Vector2Length(moveDir) > 0.05f;
        if (ch.moving) {
            ch.pos = Vector2Add(ch.pos, Vector2Scale(moveDir, spd * dt));
            ch.walkTimer += dt * 10.0f;
        }
        if (fabsf(ch.aim.x) > 0.1f) ch.facingLeft = ch.aim.x < 0.0f;
        // Tách nhẹ khỏi cha & anh chị em để không chồng lên nhau
        Vector2 sep = Vector2Subtract(ch.pos, player.pos);
        float sd = Vector2Length(sep);
        if (sd < 28.0f && sd > 0.01f) ch.pos = Vector2Add(player.pos, Vector2Scale(sep, 28.0f / sd));
        for (size_t j = 0; j < children.size(); ++j) {
            if (j == k || !children[j].active) continue;
            Vector2 s2 = Vector2Subtract(ch.pos, children[j].pos);
            float d2 = Vector2Length(s2);
            if (d2 < 24.0f && d2 > 0.01f) ch.pos = Vector2Add(ch.pos, Vector2Scale(s2, (24.0f - d2) * 0.5f / d2));
        }
        ch.pos.x = Clamp(ch.pos.x, -(roomW / 2.0f - 30.0f), roomW / 2.0f - 30.0f);
        ch.pos.y = Clamp(ch.pos.y, -(roomH / 2.0f - 30.0f), roomH / 2.0f - 30.0f);
        ResolvePillars(ch.pos, 12.0f);

        // Va chạm với quái: con bị thương
        for (const auto& e : enemies) {
            if (e.hp <= 0.0f || e.stunTimer > 0.0f) continue;
            if (Vector2Distance(e.pos, ch.pos) < e.radius + 13.0f) {
                float mul = ((e.type == CHARGER && e.chargeState == 2) || (e.type == BOSS && e.bossKind == 1 && e.chargeState == 4)) ? 1.8f : 1.0f;
                ChildTakeDamage((int)k, e.contactDmg * mul);
                break;
            }
        }
    }
}

inline void Game::DrawPartyChildren() {
    if (inSurface) return;
    float tm = (float)GetTime();
    for (size_t k = 0; k < children.size(); ++k) {
        const ChildInfo& ch = children[k];
        if (!ch.active) continue;
        int hc = std::max(0, std::min(3, ch.heroClass));
        Color cc = kClasses[hc].color;
        float sc = 0.62f + 0.38f * Clamp((ch.age - kChildTrainAge) / (float)(18 - kChildTrainAge), 0.0f, 1.0f);
        DrawEllipse((int)ch.pos.x, (int)(ch.pos.y + 16.0f * sc), (int)(18.0f * sc), (int)(7.0f * sc), { 0, 0, 0, 90 });
        DrawCircleLines((int)ch.pos.x, (int)(ch.pos.y + 14.0f * sc), 15.0f * sc, Fade(cc, 0.55f));
        bool blink = ch.invuln > 0.0f && ((int)(tm * 20.0f) % 2 == 0);
        if (!blink) {
            float off = ChildIsRanged(hc) ? 0.0f : (ch.swing > 0.0f ? Lerp(1.25f, -1.25f, ch.swing / 0.18f) : 0.5f);
            rlPushMatrix();
            rlTranslatef(ch.pos.x, ch.pos.y, 0.0f);
            rlScalef(sc, sc, 1.0f);
            DrawHeroModel({ 0.0f, 0.0f }, hc, ClassWeaponType(hc), ch.aim, ch.facingLeft, ch.walkTimer, ch.moving, off, false, false, 0.0f, 0.0f, 0, 0.0f);
            rlPopMatrix();
        }
        // Tên, tuổi, cấp & thanh máu
        float topY = ch.pos.y - 36.0f * sc - 14.0f;
        std::string tag = TextFormat(L("%s (%dy) Lv%d", "%s (%dt) Lv%d"), ch.name.c_str(), ChildAgeYears(ch), ch.level);
        OutlinedText(tag.c_str(), (int)ch.pos.x - MeasureText(tag.c_str(), 10) / 2, (int)topY - 10, 10, LerpColor(cc, WHITE, 0.35f), { 10, 12, 20, 235 });
        float pct = Clamp(ch.hp / std::max(1.0f, ch.maxHp), 0.0f, 1.0f);
        DrawRectangle((int)ch.pos.x - 17, (int)topY + 2, 34, 5, { 20, 20, 28, 220 });
        DrawRectangle((int)ch.pos.x - 16, (int)topY + 3, (int)(32 * pct), 3, pct > 0.35f ? Color{ 110, 230, 140, 255 } : RED);
    }
}

inline void Game::DrawPartyHud(int y) {
    bool any = false;
    for (const auto& ch : children) if (ch.inParty || ch.injuredTimer > 0.0f) { any = true; break; }
    if (!any) return;
    DrawText(L("TRAINING PARTY", "ĐOÀN RÈN LUYỆN"), 24, y, 13, { 150, 230, 190, 255 });
    y += 17;
    for (const auto& ch : children) {
        if (!ch.inParty && ch.injuredTimer <= 0.0f) continue;
        int hc = std::max(0, std::min(3, ch.heroClass));
        Color cc = kClasses[hc].color;
        DrawRectangle(24, y, 236, 26, { 14, 18, 30, 210 });
        DrawRectangleLines(24, y, 236, 26, Fade(cc, 0.8f));
        std::string ln = TextFormat(L("%s  %dy  Lv%d", "%s  %d tuổi  Lv%d"), ch.name.c_str(), ChildAgeYears(ch), ch.level);
        DrawText(ln.c_str(), 30, y + 3, 11, cc);
        if (ch.injuredTimer > 0.0f) {
            std::string inj = TextFormat(L("Wounded %ds", "Dưỡng thương %ds"), (int)ceilf(ch.injuredTimer));
            DrawText(inj.c_str(), 30, y + 14, 10, { 255, 130, 130, 255 });
        } else if (!ch.active) {
            DrawText(inSurface ? L("Waiting at home", "Đang chờ ở nhà") : L("Not in this floor", "Không có mặt"), 30, y + 14, 10, LIGHTGRAY);
        } else {
            float hpP = Clamp(ch.hp / std::max(1.0f, ch.maxHp), 0.0f, 1.0f);
            DrawRectangle(30, y + 16, 110, 5, { 40, 30, 36, 255 });
            DrawRectangle(30, y + 16, (int)(110 * hpP), 5, hpP > 0.35f ? Color{ 110, 230, 140, 255 } : RED);
            float xpP = ch.level >= kChildMaxLevel ? 1.0f : Clamp((float)ch.xp / ChildXpNext(ch.level), 0.0f, 1.0f);
            DrawRectangle(146, y + 16, 106, 5, { 26, 32, 48, 255 });
            DrawRectangle(146, y + 16, (int)(106 * xpP), 5, SKYBLUE);
        }
        y += 30;
    }
}
