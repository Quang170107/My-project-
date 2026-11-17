#pragma once
#include "../game.hpp"

inline void Game::RefreshSaveSlots() {
    for (int s = 0; s < 4; ++s) {
        SaveSlotInfo info;
        char* txt = LoadFileText(SaveFileName(s).c_str());
        if (txt) {
            std::istringstream in(txt);
            UnloadFileText(txt);
            std::string line;
            int existsFlag = 0;
            int hasW = 0;
            while (std::getline(in, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                size_t eq = line.find('=');
                if (eq == std::string::npos) continue;
                std::string k = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                if (k == "exists") existsFlag = atoi(val.c_str());
                else if (k == "player_heroClass") info.heroClass = std::max(0, std::min(3, atoi(val.c_str())));
                else if (k == "player_subClass") info.subClass = std::max(0, std::min(2, atoi(val.c_str())));
                else if (k == "floor") info.floor = std::max(1, atoi(val.c_str()));
                else if (k == "player_level") info.level = std::max(1, std::min(kMaxPlayerLevel, atoi(val.c_str())));
                else if (k == "player_hp") info.hp = atoi(val.c_str());
                else if (k == "player_maxHp") info.maxHp = std::max(1, atoi(val.c_str()));
                else if (k == "player_gold") info.gold = atoi(val.c_str());
                else if (k == "enemiesSlain") info.enemiesSlain = atoi(val.c_str());
                else if (k == "hasEquip_0") hasW = atoi(val.c_str());
                else if (k == "equip_0_weaponType") info.weaponType = std::max(0, std::min(7, atoi(val.c_str())));
                else if (k == "equip_0_name") info.weaponName = val;
                else if (k == "heroGivenName") info.heroName = val;
                else if (k == "timestamp") info.timestamp = val;
            }
            if (existsFlag == 1) {
                info.exists = true;
                info.weaponType = ClassWeaponType(info.heroClass);
                if (!hasW || info.weaponName.empty()) {
                    info.weaponName = MakeStarterWeapon(info.weaponType).name;
                }
            }
        }
        saveSlots[s] = info;
    }
}

inline void Game::DeleteSaveSlot(int slot) {
    if (slot < 0 || slot >= 4 || !saveSlots[slot].exists) return;
    std::string empty = "exists=0\n";
    SaveFileText(SaveFileName(slot).c_str(), (char*)empty.c_str());
    RefreshSaveSlots();
    PlaySound(snd.sndHurt);
    saveStatusMsg = "Cleared Slot " + std::to_string(slot + 1) + ".";
    saveStatusTimer = 2.5f;
}

inline bool Game::SaveGameToSlot(int slot) {
    if (slot < 0 || slot >= 4) return false;
    bool wasOccupied = saveSlots[slot].exists;
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char tbuf[32] = "Saved Run";
    if (lt) std::strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M", lt);

    int roomCleared = (portalOpen && enemies.empty()) ? 1 : 0;
    std::ostringstream o;
    o << "exists=1\n"
      << "timestamp=" << tbuf << "\n"
      << "floor=" << floor << "\n"
      << "enemiesSlain=" << enemiesSlain << "\n"
      << "bestCombo=" << bestCombo << "\n"
      << "bossesKilled=" << bossesKilled << "\n"
      << "phoenixUsed=" << (phoenixUsed ? 1 : 0) << "\n"
      << "roomCleared=" << roomCleared << "\n"
      << "selectedClass=" << selectedClass << "\n"
      << "player_heroClass=" << player.heroClass << "\n"
      << "player_subClass=" << player.subClass << "\n"
      << "player_hp=" << player.hp << "\n"
      << "player_maxHp=" << player.maxHp << "\n"
      << "player_speed=" << player.speed << "\n"
      << "player_baseDmg=" << player.baseDmg << "\n"
      << "player_attackRate=" << player.attackRate << "\n"
      << "player_dashCooldown=" << player.dashCooldown << "\n"
      << "player_cdrUpgrades=" << player.cdrUpgrades << "\n"
      << "player_critChance=" << player.critChance << "\n"
      << "player_lifesteal=" << player.lifesteal << "\n"
      << "player_armor=" << player.armor << "\n"
      << "player_level=" << player.level << "\n"
      << "player_titleTierClaimed=" << player.titleTierClaimed << "\n"
      << "player_xp=" << player.xp << "\n"
      << "player_xpNext=" << player.xpNext << "\n"
      << "player_gold=" << player.gold << "\n"
      << "player_rage=" << player.rage << "\n"
      << "player_igniteLevel=" << player.igniteLevel << "\n"
      << "player_frostLevel=" << player.frostLevel << "\n"
      << "player_shieldActive=" << (player.shieldActive ? 1 : 0) << "\n"
      << "surfaceUnlocked=" << (surfaceUnlocked ? 1 : 0) << "\n"
      << "inSurface=" << (inSurface ? 1 : 0) << "\n"
      << "surfaceRoom=" << surfaceRoom << "\n"
      << "surfaceTownReturnX=" << surfaceTownReturnPos.x << "\n"
      << "surfaceTownReturnY=" << surfaceTownReturnPos.y << "\n"
      << "nextDungeonFloor=" << nextDungeonFloor << "\n"
      << "hunger=" << hunger << "\n"
      << "foodRations=" << foodRations << "\n"
      << "generation=" << generation << "\n"
      << "heroGivenName=" << heroGivenName << "\n"
      << "houseTier=" << houseTier << "\n"
      << "isNoble=" << (isNoble ? 1 : 0) << "\n"
      << "royalQuestActive=" << (royalQuestActive ? 1 : 0) << "\n"
      << "royalQuestKills=" << royalQuestKills << "\n"
      << "royalQuestBosses=" << royalQuestBosses << "\n"
      << "hasSpouse=" << (hasSpouse ? 1 : 0) << "\n"
      << "spouseIdx=" << spouseIdx << "\n"
      << "spouseQuestProgress=" << spouseQuestProgress << "\n"
      << "spouseUnlocked_0=" << (spouseUnlocked[0] ? 1 : 0) << "\n"
      << "spouseUnlocked_1=" << (spouseUnlocked[1] ? 1 : 0) << "\n"
      << "spouseUnlocked_2=" << (spouseUnlocked[2] ? 1 : 0) << "\n"
      << "spouseUnlocked_3=" << (spouseUnlocked[3] ? 1 : 0) << "\n"
      << "timeOfDay=" << timeOfDay << "\n"
      << "dayCount=" << dayCount << "\n"
      << "heroAge=" << heroAge << "\n"
      << "legacyHp=" << legacyHp << "\n"
      << "legacyDmg=" << legacyDmg << "\n"
      << "legacyArmor=" << legacyArmor << "\n"
      << "legacySpeed=" << legacySpeed << "\n"
      << "childCount=" << (int)children.size() << "\n"
      << "ancestorCount=" << (int)familyTree.size() << "\n"
      << "relicCount=" << (int)player.relics.size() << "\n";
    for (size_t i = 0; i < children.size(); ++i) {
        o << "child_" << i << "_name=" << children[i].name << "\n"
          << "child_" << i << "_cls=" << children[i].heroClass << "\n"
          << "child_" << i << "_hp=" << children[i].bonusHp << "\n"
          << "child_" << i << "_dmg=" << children[i].bonusDmg << "\n"
          << "child_" << i << "_arm=" << children[i].bonusArmor << "\n"
          << "child_" << i << "_spd=" << children[i].bonusSpeed << "\n";
    }
    for (size_t i = 0; i < familyTree.size(); ++i) {
        o << "anc_" << i << "_gen=" << familyTree[i].generation << "\n"
          << "anc_" << i << "_name=" << familyTree[i].name << "\n"
          << "anc_" << i << "_cls=" << familyTree[i].heroClass << "\n"
          << "anc_" << i << "_sub=" << familyTree[i].subClass << "\n"
          << "anc_" << i << "_noble=" << (familyTree[i].wasNoble ? 1 : 0) << "\n"
          << "anc_" << i << "_spouse=" << familyTree[i].spouseName << "\n"
          << "anc_" << i << "_floor=" << familyTree[i].floorReached << "\n"
          << "anc_" << i << "_lvl=" << familyTree[i].levelReached << "\n"
          << "anc_" << i << "_slain=" << familyTree[i].enemiesSlain << "\n";
    }
    for (size_t i = 0; i < player.relics.size(); ++i) {
        o << "relic_" << i << "=" << (int)player.relics[i].type << "\n";
    }
    for (int i = 0; i < 4; ++i) {
        o << "hasEquip_" << i << "=" << (hasEquip[i] ? 1 : 0) << "\n";
        if (hasEquip[i]) {
            o << "equip_" << i << "_name=" << equipped[i].name << "\n"
              << "equip_" << i << "_slot=" << equipped[i].slot << "\n"
              << "equip_" << i << "_rarity=" << equipped[i].rarity << "\n"
              << "equip_" << i << "_weaponType=" << equipped[i].weaponType << "\n"
              << "equip_" << i << "_damage=" << equipped[i].damage << "\n"
              << "equip_" << i << "_maxHp=" << equipped[i].maxHp << "\n"
              << "equip_" << i << "_armor=" << equipped[i].armor << "\n"
              << "equip_" << i << "_moveSpeed=" << equipped[i].moveSpeed << "\n"
              << "equip_" << i << "_attackRate=" << equipped[i].attackRate << "\n"
              << "equip_" << i << "_lifesteal=" << equipped[i].lifesteal << "\n"
              << "equip_" << i << "_crit=" << equipped[i].crit << "\n";
        }
    }
    std::string payload = o.str();
    SaveFileText(SaveFileName(slot).c_str(), (char*)payload.c_str());
    RefreshSaveSlots();
    PlaySound(snd.sndLevelUp);
    saveStatusMsg = "Saved to Slot " + std::to_string(slot + 1) + (wasOccupied ? " (Overwritten)!" : "!");
    saveStatusTimer = 2.8f;
    return true;
}

inline bool Game::LoadGameFromSlot(int slot) {
    if (slot < 0 || slot >= 4 || !saveSlots[slot].exists) {
        saveStatusMsg = "Slot is empty!";
        saveStatusTimer = 2.0f;
        PlaySound(snd.sndHurt);
        return false;
    }
    char* txt = LoadFileText(SaveFileName(slot).c_str());
    if (!txt) {
        saveStatusMsg = "Failed to read save file!";
        saveStatusTimer = 2.0f;
        PlaySound(snd.sndHurt);
        return false;
    }
    std::istringstream in(txt);
    UnloadFileText(txt);

    Player loadedP;
    EquipItem loadedEq[4];
    bool loadedHas[4] = { false, false, false, false };
    int existsFlag = 0;
    int loadedFloor = 1, loadedSlain = 0, loadedBestCombo = 0, loadedBosses = 0, loadedSelClass = 0;
    bool loadedPhoenix = false;
    int roomCleared = 0;
    int relicCount = 0;
    bool hasLoadedTitleTier = false;
    std::vector<int> relicTypes;
    bool lSurfaceUnlocked = false, lInSurface = false, lNoble = false, lRqActive = false, lHasSpouse = false;
    bool lSpUnlocked[4] = { false, false, false, false };
    int lNextFloor = 11, lRations = 2, lGen = 1, lHouseTier = 0, lRqKills = 0, lRqBosses = 0, lSpouseIdx = -1, lSurfaceRoom = 0;
    int lSpQuest = 0, lDay = 1, lAge = 18;
    float lTimeOfDay = 8.5f;
    Vector2 lTownRet = { 0.0f, 220.0f };
    float lHunger = 100.0f, lLegHp = 0.0f, lLegDmg = 0.0f, lLegArm = 0.0f, lLegSpd = 0.0f;
    std::string lHeroName = "";
    int lChildCount = 0, lAncCount = 0;
    std::vector<ChildInfo> lChildren;
    std::vector<AncestorRecord> lAncestors;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        int iv = atoi(v.c_str());
        float fv = (float)atof(v.c_str());

        if (k == "exists") existsFlag = iv;
        else if (k == "floor") loadedFloor = std::max(1, iv);
        else if (k == "enemiesSlain") loadedSlain = std::max(0, iv);
        else if (k == "bestCombo") loadedBestCombo = std::max(0, iv);
        else if (k == "bossesKilled") loadedBosses = std::max(0, iv);
        else if (k == "phoenixUsed") loadedPhoenix = (iv != 0);
        else if (k == "roomCleared") roomCleared = iv;
        else if (k == "selectedClass") loadedSelClass = std::max(0, std::min(3, iv));
        else if (k == "player_heroClass") loadedP.heroClass = std::max(0, std::min(3, iv));
        else if (k == "player_subClass") loadedP.subClass = std::max(0, std::min(2, iv));
        else if (k == "player_hp") loadedP.hp = fv;
        else if (k == "player_maxHp") loadedP.maxHp = std::max(1.0f, fv);
        else if (k == "player_speed") loadedP.speed = std::max(80.0f, fv);
        else if (k == "player_baseDmg") loadedP.baseDmg = std::max(1.0f, fv);
        else if (k == "player_attackRate") loadedP.attackRate = std::max(0.4f, fv);
        else if (k == "player_dashCooldown") loadedP.dashCooldown = std::max(0.3f, fv);
        else if (k == "player_cdrUpgrades") loadedP.cdrUpgrades = std::max(0, std::min(5, iv));
        else if (k == "player_critChance") loadedP.critChance = std::max(0.0f, std::min(0.9f, fv));
        else if (k == "player_lifesteal") loadedP.lifesteal = std::max(0.0f, std::min(0.5f, fv));
        else if (k == "player_armor") loadedP.armor = std::max(0.0f, fv);
        else if (k == "player_level") loadedP.level = std::max(1, std::min(kMaxPlayerLevel, iv));
        else if (k == "player_titleTierClaimed") { loadedP.titleTierClaimed = std::max(0, std::min(5, iv)); hasLoadedTitleTier = true; }
        else if (k == "player_xp") loadedP.xp = std::max(0, iv);
        else if (k == "player_xpNext") loadedP.xpNext = std::max(10, iv);
        else if (k == "player_gold") loadedP.gold = std::max(0, iv);
        else if (k == "player_rage") loadedP.rage = std::max(0.0f, std::min(100.0f, fv));
        else if (k == "player_igniteLevel") loadedP.igniteLevel = std::max(0, iv);
        else if (k == "player_frostLevel") loadedP.frostLevel = std::max(0, iv);
        else if (k == "player_shieldActive") loadedP.shieldActive = (iv != 0);
        else if (k == "surfaceUnlocked") lSurfaceUnlocked = (iv != 0);
        else if (k == "inSurface") lInSurface = (iv != 0);
        else if (k == "surfaceRoom") lSurfaceRoom = std::max(0, std::min(4, iv));
        else if (k == "surfaceTownReturnX") lTownRet.x = fv;
        else if (k == "surfaceTownReturnY") lTownRet.y = fv;
        else if (k == "nextDungeonFloor") lNextFloor = std::max(1, iv);
        else if (k == "hunger") lHunger = Clamp(fv, 0.0f, 100.0f);
        else if (k == "foodRations") lRations = std::max(0, iv);
        else if (k == "generation") lGen = std::max(1, iv);
        else if (k == "heroGivenName") lHeroName = v;
        else if (k == "houseTier") lHouseTier = std::max(0, std::min(3, iv));
        else if (k == "isNoble") lNoble = (iv != 0);
        else if (k == "royalQuestActive") lRqActive = (iv != 0);
        else if (k == "royalQuestKills") lRqKills = std::max(0, iv);
        else if (k == "royalQuestBosses") lRqBosses = std::max(0, iv);
        else if (k == "hasSpouse") lHasSpouse = (iv != 0);
        else if (k == "spouseIdx") lSpouseIdx = std::max(-1, std::min(3, iv));
        else if (k == "spouseQuestProgress") lSpQuest = std::max(0, iv);
        else if (k == "spouseUnlocked_0") lSpUnlocked[0] = (iv != 0);
        else if (k == "spouseUnlocked_1") lSpUnlocked[1] = (iv != 0);
        else if (k == "spouseUnlocked_2") lSpUnlocked[2] = (iv != 0);
        else if (k == "spouseUnlocked_3") lSpUnlocked[3] = (iv != 0);
        else if (k == "timeOfDay") lTimeOfDay = Clamp(fv, 0.0f, 23.99f);
        else if (k == "dayCount") lDay = std::max(1, iv);
        else if (k == "heroAge") lAge = std::max(1, iv);
        else if (k == "legacyHp") lLegHp = std::max(0.0f, fv);
        else if (k == "legacyDmg") lLegDmg = std::max(0.0f, fv);
        else if (k == "legacyArmor") lLegArm = std::max(0.0f, fv);
        else if (k == "legacySpeed") lLegSpd = std::max(0.0f, fv);
        else if (k == "childCount") { lChildCount = std::max(0, std::min(8, iv)); lChildren.resize(lChildCount); }
        else if (k == "ancestorCount") { lAncCount = std::max(0, std::min(32, iv)); lAncestors.resize(lAncCount); }
        else if (k.compare(0, 6, "child_") == 0 && k.size() > 8) {
            int ci = atoi(k.c_str() + 6);
            size_t us = k.find('_', 6);
            if (us != std::string::npos && ci >= 0 && ci < 8) {
                if ((int)lChildren.size() <= ci) lChildren.resize(ci + 1);
                std::string f = k.substr(us + 1);
                if (f == "name") lChildren[ci].name = v;
                else if (f == "cls") lChildren[ci].heroClass = std::max(0, std::min(3, iv));
                else if (f == "hp") lChildren[ci].bonusHp = fv;
                else if (f == "dmg") lChildren[ci].bonusDmg = fv;
                else if (f == "arm") lChildren[ci].bonusArmor = fv;
                else if (f == "spd") lChildren[ci].bonusSpeed = fv;
            }
        } else if (k.compare(0, 4, "anc_") == 0 && k.size() > 6) {
            int ai = atoi(k.c_str() + 4);
            size_t us = k.find('_', 4);
            if (us != std::string::npos && ai >= 0 && ai < 32) {
                if ((int)lAncestors.size() <= ai) lAncestors.resize(ai + 1);
                std::string f = k.substr(us + 1);
                if (f == "gen") lAncestors[ai].generation = std::max(1, iv);
                else if (f == "name") lAncestors[ai].name = v;
                else if (f == "cls") lAncestors[ai].heroClass = std::max(0, std::min(3, iv));
                else if (f == "sub") lAncestors[ai].subClass = std::max(0, std::min(2, iv));
                else if (f == "noble") lAncestors[ai].wasNoble = (iv != 0);
                else if (f == "spouse") lAncestors[ai].spouseName = v;
                else if (f == "floor") lAncestors[ai].floorReached = std::max(1, iv);
                else if (f == "lvl") lAncestors[ai].levelReached = std::max(1, std::min(kMaxPlayerLevel, iv));
                else if (f == "slain") lAncestors[ai].enemiesSlain = std::max(0, iv);
            }
        }
        else if (k == "relicCount") relicCount = std::max(0, std::min(16, iv));
        else if (k.compare(0, 6, "relic_") == 0) {
            int rIdx = atoi(k.c_str() + 6);
            if (rIdx >= 0 && rIdx < 16) {
                if ((int)relicTypes.size() <= rIdx) relicTypes.resize(rIdx + 1, -1);
                relicTypes[rIdx] = iv;
            }
        } else if (k.compare(0, 9, "hasEquip_") == 0) {
            int ei = k[9] - '0';
            if (ei >= 0 && ei < 4) loadedHas[ei] = (iv != 0);
        } else if (k.compare(0, 6, "equip_") == 0 && k.size() > 8 && k[7] == '_') {
            int ei = k[6] - '0';
            std::string f = k.substr(8);
            if (ei >= 0 && ei < 4) {
                if (f == "name") loadedEq[ei].name = v;
                else if (f == "slot") loadedEq[ei].slot = std::max(0, std::min(3, iv));
                else if (f == "rarity") loadedEq[ei].rarity = std::max(0, std::min(3, iv));
                else if (f == "weaponType") loadedEq[ei].weaponType = std::max(0, std::min(7, iv));
                else if (f == "damage") loadedEq[ei].damage = fv;
                else if (f == "maxHp") loadedEq[ei].maxHp = fv;
                else if (f == "armor") loadedEq[ei].armor = fv;
                else if (f == "moveSpeed") loadedEq[ei].moveSpeed = fv;
                else if (f == "attackRate") loadedEq[ei].attackRate = fv;
                else if (f == "lifesteal") loadedEq[ei].lifesteal = fv;
                else if (f == "crit") loadedEq[ei].crit = fv;
            }
        }
    }

    if (existsFlag != 1) {
        saveStatusMsg = "Save slot is empty or invalid!";
        saveStatusTimer = 2.0f;
        PlaySound(snd.sndHurt);
        return false;
    }

    loadedP.hp = std::max(1.0f, std::min(loadedP.maxHp, loadedP.hp));
    loadedP.hpDisplay = loadedP.hp;
    if (!hasLoadedTitleTier) loadedP.titleTierClaimed = LevelTitleTierForLevel(loadedP.level);
    for (int i = 0; i < relicCount && i < (int)relicTypes.size(); ++i) {
        int rt = relicTypes[i];
        if (rt >= 0 && rt <= (int)RELIC_THORNS) loadedP.relics.push_back(MakeRelic((RelicType)rt));
    }

    player = loadedP;
    for (int i = 0; i < 4; ++i) {
        hasEquip[i] = loadedHas[i];
        equipped[i] = loadedEq[i];
    }
    if (hasEquip[SLOT_WEAPON]) {
        equipped[SLOT_WEAPON].weaponType = ClassWeaponType(player.heroClass);
    }
    floor = loadedFloor;
    enemiesSlain = loadedSlain;
    bestCombo = loadedBestCombo;
    bossesKilled = loadedBosses;
    phoenixUsed = loadedPhoenix;
    selectedClass = player.heroClass;
    subclassChoosing = false;
    comboCount = 0;
    comboTimer = 0.0f;
    damagePulse = 0.0f;
    hitStop = 0.0f;
    newRecord = false;
    (void)loadedSelClass;

    surfaceUnlocked = lSurfaceUnlocked;
    inSurface = lInSurface && lSurfaceUnlocked;
    nextDungeonFloor = lNextFloor;
    hunger = lHunger;
    foodRations = lRations;
    generation = lGen;
    heroGivenName = lHeroName;
    houseTier = lHouseTier;
    isNoble = lNoble;
    royalQuestActive = lRqActive;
    royalQuestKills = lRqKills;
    royalQuestBosses = lRqBosses;
    hasSpouse = lHasSpouse;
    spouseIdx = lSpouseIdx;
    for (int i = 0; i < 4; ++i) spouseUnlocked[i] = lSpUnlocked[i];
    if (hasSpouse && spouseIdx >= 0 && spouseIdx < 4) spouseUnlocked[spouseIdx] = true;
    spouseQuestProgress = lSpQuest;
    timeOfDay = lTimeOfDay;
    dayCount = lDay;
    heroAge = lAge;
    legacyHp = lLegHp;
    legacyDmg = lLegDmg;
    legacyArmor = lLegArm;
    legacySpeed = lLegSpd;
    if ((int)lChildren.size() > lChildCount) lChildren.resize(lChildCount);
    if ((int)lAncestors.size() > lAncCount) lAncestors.resize(lAncCount);
    children = lChildren;
    familyTree = lAncestors;

    if (inSurface) {
        surfaceTownReturnPos = lTownRet;
        SetupSurfaceRoomEnvironment(lSurfaceRoom, true);
    } else {
        roomW = 1200.0f;
        roomH = 750.0f;
        BuildFloor();
        player.pos = { 0, roomH / 2.0f - 90.0f };
        if (roomCleared) {
            enemies.clear();
            pendingSpawns.clear();
            wave2Spawns.clear();
            hasTrialTotem = false;
            portalOpen = true;
            BuildPortalChoices();
        }
    }
    player.hpDisplay = player.hp;
    player.specialCooldown = Skill1MaxCd();
    player.skill2Cooldown = Skill2MaxCd();
    player.skill3Cooldown = Skill3MaxCd();
    player.skill4Cooldown = Skill4MaxCd();
    camera.target = player.pos;
    state = PLAYING;
    PlaySound(snd.sndPortal);
    ShowBanner("LOADED SLOT " + std::to_string(slot + 1) + " - FLOOR " + std::to_string(floor), 3.0f);
    return true;
}

inline void Game::OpenSaveLoad(State from) {
    saveLoadReturn = from;
    saveStatusMsg.clear();
    saveStatusTimer = 0.0f;
    RefreshSaveSlots();
    state = SAVE_LOAD;
    PlaySound(snd.sndCoin);
}

inline void Game::UpdateSaveLoad(float dt) {
    { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
    if (saveStatusTimer > 0.0f) saveStatusTimer -= dt;
    if (EscPressed() || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
        state = saveLoadReturn;
        return;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) saveSlotSel = (saveSlotSel + 1) % 4;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) saveSlotSel = (saveSlotSel + 3) % 4;
    for (int i = 0; i < 4; ++i) if (IsKeyPressed(KEY_ONE + i)) saveSlotSel = i;

    const int cardW = 280, gap = 18, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 98, cardH = 505;
    Vector2 m = GetMousePosition();
    for (int i = 0; i < 4; ++i) {
        Rectangle r = { (float)(x0 + i * (cardW + gap)), (float)cy, (float)cardW, (float)cardH };
        const SaveSlotInfo& info = saveSlots[i];
        if (CheckCollisionPointRec(m, r) && (GetMouseDelta().x != 0 || GetMouseDelta().y != 0)) saveSlotSel = i;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(m, r)) {
            saveSlotSel = i;
            if (saveLoadReturn == PAUSED) {
                Rectangle saveBtn = { r.x + 16.0f, r.y + r.height - 96.0f, r.width - 32.0f, 38.0f };
                if (CheckCollisionPointRec(m, saveBtn)) {
                    if (!info.exists && !ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                        OpenNameModal(3, i, heroGivenName.empty() ? kHeirNames[(selectedClass * 2) % 10] : heroGivenName);
                    } else {
                        SaveGameToSlot(i);
                    }
                    return;
                }
                if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 94.0f, 36.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 50.0f, 54.0f, 36.0f };
                    if (CheckCollisionPointRec(m, loadBtn)) { LoadGameFromSlot(i); return; }
                    if (CheckCollisionPointRec(m, delBtn)) { DeleteSaveSlot(i); return; }
                }
            } else {
                if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 94.0f, 40.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 54.0f, 54.0f, 40.0f };
                    if (CheckCollisionPointRec(m, loadBtn)) { LoadGameFromSlot(i); return; }
                    if (CheckCollisionPointRec(m, delBtn)) { DeleteSaveSlot(i); return; }
                } else {
                    Rectangle newBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 32.0f, 40.0f };
                    if (CheckCollisionPointRec(m, newBtn)) {
                        RestartRun();
                        if (!ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                            OpenNameModal(3, i, kHeirNames[(selectedClass * 2) % 10]);
                        } else {
                            SaveGameToSlot(i);
                        }
                        return;
                    }
                }
            }
        }
    }

    if (IsKeyPressed(KEY_N) && !ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
        if (saveLoadReturn != PAUSED && !saveSlots[saveSlotSel].exists) RestartRun();
        OpenNameModal(3, saveSlotSel, heroGivenName.empty() ? kHeirNames[(selectedClass * 2) % 10] : heroGivenName);
        return;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        if (saveLoadReturn == PAUSED) {
            if (!saveSlots[saveSlotSel].exists && !ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                OpenNameModal(3, saveSlotSel, heroGivenName.empty() ? kHeirNames[(selectedClass * 2) % 10] : heroGivenName);
            } else {
                SaveGameToSlot(saveSlotSel);
            }
        } else if (saveSlots[saveSlotSel].exists) {
            LoadGameFromSlot(saveSlotSel);
        } else {
            RestartRun();
            if (!ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                OpenNameModal(3, saveSlotSel, kHeirNames[(selectedClass * 2) % 10]);
            } else {
                SaveGameToSlot(saveSlotSel);
            }
        }
    } else if (IsKeyPressed(KEY_L) && saveLoadReturn == PAUSED) {
        LoadGameFromSlot(saveSlotSel);
    } else if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X)) {
        DeleteSaveSlot(saveSlotSel);
    }
}
