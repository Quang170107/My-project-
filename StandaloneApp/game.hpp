#pragma once
#include "core/game_data.hpp"

class Game {
public:
    SoundManager snd;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Loot> loots;
    std::vector<DamagePopup> popups;
    std::vector<SlashVisual> slashes;
    std::vector<NovaVisual> novas;
    std::vector<GhostTrail> ghosts;
    std::vector<GroundEffect> groundEffects;
    std::vector<Vector2> pillars;

    int floor = 1;
    int enemiesSlain = 0;
    int comboCount = 0;
    float comboTimer = 0.0f;
    int bestFloor = 1;
    bool newRecord = false;
    bool portalOpen = false;
    Vector2 portalPos = { 0, -320 };

    float roomW = 1200;
    float roomH = 750;

    Camera2D camera = { 0 };
    float screenShakeTimer = 0.0f;
    float screenShakeMag = 0.0f;

    enum State { PLAYING, LEVEL_UP, GAME_OVER, TITLE, PAUSED, INVENTORY, SETTINGS, SHOP, META, CLASS_SELECT, SAVE_LOAD, SURFACE_UI, FAMILY_TREE, WORLD_MAP };
    State state = PLAYING;
    int selectedClass = 0;
    int classSel = 0;
    bool quitRequested = false;
    bool showHelp = false;
    State prevState = PLAYING;

    struct SaveSlotInfo {
        bool exists = false;
        int heroClass = 0;
        int subClass = 0;
        int floor = 1;
        int level = 1;
        int hp = 120, maxHp = 120;
        int gold = 0;
        int enemiesSlain = 0;
        int weaponType = 0;
        std::string heroName = "";
        std::string weaponName = "Sword";
        std::string timestamp = "";
    };
    SaveSlotInfo saveSlots[4];
    State saveLoadReturn = TITLE;
    int saveSlotSel = 0;
    std::string saveStatusMsg = "";
    float saveStatusTimer = 0.0f;
    EquipItem equipped[4];
    bool hasEquip[4] = { false, false, false, false };
    std::vector<ItemDrop> itemDrops;
    Settings settings;
    int lastAppliedWinMode = -1, lastAppliedW = 0, lastAppliedH = 0;
    std::vector<int> resList;
    RenderTexture2D screenRT = { 0 };
    int setSel = 0;
    int floorMod = MOD_NONE;
    int layout = LAY_SCATTERED;
    int lastLayout = -1;
    float eliteChance = 0.2f;
    bool shopFloor = false;
    Vector2 merchantPos = { 0, -150 };
    ShopSlot shopSlots[5];
    int shopSel = 0;
    bool shopHinted = false;
    std::vector<Chest> chests;
    std::vector<Enemy> pendingSpawns;
    std::vector<ArenaBarrel> barrels;
    std::vector<FloorHazard> hazards;
    std::vector<PortalOption> portalChoices;
    std::vector<Enemy> wave2Spawns;
    bool wave2Triggered = false;
    int forcedNextMod = -1;
    int forcedNextReward = -1;
    bool hasTrialTotem = false;
    bool trialUsed = false;
    bool trialActive = false;
    Vector2 trialPos = { 0, 0 };
    bool phoenixUsed = false;
    int souls = 0, runs = 0, lastRunSouls = 0, bossesKilled = 0;
    int meta[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    State metaReturn = TITLE;
    int metaSel = 0;
    std::string metaMsg;
    float metaMsgTimer = 0.0f;
    float xpMul = 1.0f;

    // Điều khiển thay thế (bot mô phỏng / tay cầm): khi active, dùng giá trị này thay cho bàn phím và chuột
    struct ControlOverride {
        bool active = false;
        Vector2 move = { 0, 0 };
        Vector2 aim = { 1, 0 };
        bool attack = false, dash = false, nova = false, skill2 = false, skill3 = false, skill4 = false, ult = false;
        bool external = false; // true: bot mô phỏng đang điều khiển, bỏ qua tay cầm
    };
    ControlOverride ctl;
    float rumbleTimer = 0.0f;
    // Cảm ứng
    bool touchMode = false;
    std::vector<std::pair<int, Vector2>> touches;
    int joyId = -1;
    Vector2 joyOrigin = { 150, 590 }, joyPos = { 150, 590 };
    bool touchDashWas = false, touchNovaWas = false, touchSkill2Was = false, touchUltWas = false, touchUiWas = false;
    float presentDx = 0.0f, presentDy = 0.0f, presentScale = 1.0f;
    bool useFakeTouches = false;           // dùng cho kiểm thử
    std::vector<Vector2> fakeTouches;
    Vector2 padAim = { 1, 0 };
    bool padLtWas = false;
    State settingsReturn = TITLE;
    std::vector<UpgradeType> levelChoices;
    bool subclassChoosing = false;
    std::string bannerText = "";
    float bannerTimer = 0.0f;
    bool showMiniMap = true;

    // Hiệu ứng hình ảnh & sự kiện
    std::vector<Particle> particles;
    std::vector<Mote> motes;
    float damagePulse = 0.0f;   // viền màn hình nhấp đỏ khi bị đau
    float hitStop = 0.0f;       // khựng khung hình ngắn khi chí mạng
    float slowMoTimer = 0.0f;   // làm chậm thời gian khi hạ Trùm / Né Hoàn Hảo
    float perfectDodgeTimer = 0.0f;
    bool nextHitCrit = false;
    float stylePoints = 0.0f;
    int styleRank = 0;          // 0:D, 1:C, 2:B, 3:A, 4:S, 5:SS, 6:SSS
    float styleDecayTimer = 0.0f;
    int perfectDodgesCount = 0;
    int elementalReactionsCount = 0;
    int curseStacks = 0;
    bool hasBloodPact = false;
    bool bloodPactUsed = false;
    Vector2 bloodPactPos = { 0.0f, 0.0f };
    int bossEssence = 0;
    int equipPlus[4] = { 0, 0, 0, 0 };
    bool bountyActive = false;
    int bountyKind = 0;         // 0: Elites, 1: Elemental Reactions, 2: Boss / Golden Goblin
    int bountyProgress = 0;
    int bountyTarget = 6;
    int bountyRewardGold = 200;
    int bloodlineTrait = 0;     // 0: Dragonheart, 1: Shadowstep, 2: Arcane Blood, 3: Sovereign
    int bestCombo = 0;
    int weatherKind = 0;        // 0: Clear, 1: Crimson Eclipse, 2: Arcane Ion Storm, 3: Frost Mist, 4: Ember Ashrain
    float weatherTimer = 6.0f;  // Đếm ngược sự kiện thời tiết (vd: sét thiên lôi trong Bão Từ)
    float lightningFlash = 0.0f; // Độ sáng chớp màn hình khi sét thiên lôi giáng xuống
    float bossHpLag = 1.0f;     // Hiệu ứng tụt máu trễ trên thanh máu Boss
    bool hasRunicAnvil = false; // Đài Rèn Cổ Ngữ trong tầng hầm ngục
    bool runicAnvilUsed = false;
    Vector2 runicAnvilPos = { 0.0f, 0.0f };
    int activeRune = 0;         // 0: None, 1: Thunderclap, 2: Frostfire, 3: Bloodthirst, 4: Aegis
    int runeRank = 0;           // Cấp Cổ Ngữ (1..3)
    float runeBuffTimer = 0.0f; // Thời gian hiệu lực buff Cuồng Tốc của Cổ Ngữ Huyết Vũ
    int familiarKind = 0;       // 0: None, 1: Solar Phoenix, 2: Frost Wyrm, 3: Storm Falcon, 4: Void Wisp
    int familiarRank = 0;       // Cấp Linh Thú (1..3)
    Vector2 familiarPos = { 0.0f, 0.0f };
    float familiarAngle = 0.0f;
    float familiarAtkTimer = 1.2f;
    float overdriveTimer = 0.0f; // > 0: Bùng Nổ Linh Hồn (+18% ST, +14% Tốc chạy, Đôi Cánh Ánh Sáng)
    Vector2 aimDirDraw = { 1, 0 };
    RenderTexture2D lightmap = { 0 };
    std::vector<Vector2> torches;
    std::vector<Decal> decals;
    bool hasShrine = false;
    bool shrineUsed = false;
    bool shrineHinted = false;
    Vector2 shrinePos = { 0, 0 };
    float shrineRefuse = 0.0f;
    float shrinePulse = 0.0f;

    // ── Hệ thống Mặt Đất, Đói, Nhà, Gia Đình, Gia Phả & Quý Tộc ──
    bool surfaceUnlocked = false;
    bool inSurface = false;
    int nextDungeonFloor = 11;
    float hunger = 100.0f;
    float maxHunger = 100.0f;
    float starveTickTimer = 0.0f;
    int foodRations = 2;
    int generation = 1;
    std::string heroGivenName = "";
    int houseTier = 0;
    bool isNoble = false;
    bool royalQuestActive = false;
    int royalQuestKills = 0;
    int royalQuestBosses = 0;
    bool hasSpouse = false;
    int spouseIdx = -1;
    bool spouseUnlocked[4] = { false, false, false, false };
    int spouseQuestProgress = 0;
    bool hasCaptiveEvent = false;
    bool captiveRescued = false;
    int captiveSpouseIdx = 0;
    Vector2 captivePos = { 0.0f, 0.0f };
    float timeOfDay = 8.5f;
    int dayCount = 1;
    int heroAge = 18;
    float yearClock = 0.0f;      // giây thực đã trôi trong năm hiện tại (180s = 1 tuổi)
    int lastChildKiller = -1;    // con vừa ra đòn kết liễu (để cộng thêm EXP rèn luyện)
    int namingMode = 0; // 0: off, 1: New Hero, 2: Child Name, 3: New Save Name
    int namingChildIdx = -1;
    std::string nameInputBuf = "";
    std::vector<ChildInfo> children;
    std::vector<AncestorRecord> familyTree;
    float legacyHp = 0.0f;
    float legacyDmg = 0.0f;
    float legacyArmor = 0.0f;
    float legacySpeed = 0.0f;
    int surfaceBuilding = 0; // 0: Quán trọ, 1: Mua nhà, 2: Gia đình & Đời con, 3: Hoàng cung & Vua, 4: Lãnh Địa Riêng
    int surfaceSel = 0;
    int surfaceRoom = 0;   // 0: Quảng trường, 1: Quán Trọ, 2: Phường Gia Trang, 3: Tổ Ấm & Từ Đường, 4: Hoàng Cung, 5: Lãnh Địa Phong Ấp
    Vector2 surfaceTownReturnPos = { 0.0f, 180.0f };
    State familyTreeReturn = PLAYING;

    // ── Tước Vị Chuẩn Vương Quốc Anh (0..8), Lãnh Địa Riêng, Bản Đồ Thế Giới & Nổi Dậy ──
    int peerageRank = 0;         // 0: Thường Dân .. 7: Công Tước, 8: Quốc Vương
    int warMerit = 0;            // Chiến công quốc gia
    bool isKing = false;         // Đã lật đổ Bạo Chúa và đăng quang Quốc Vương
    bool fiefEstablished = false;
    std::string fiefName = "";
    int fiefLevel = 0;           // 0..5
    int fiefBuildings[4] = { 0, 0, 0, 0 }; // 0: Thành Quách, 1: Doanh Trại, 2: Thương Điếm, 3: Học Viện
    int fiefSoldiers = 0;
    int fiefTreasury = 0;
    NationState nations[5];
    int worldMapSel = 0;
    State worldMapReturn = PLAYING;
    bool inWarBattle = false;
    int warBattleNation = 1;
    bool inRebellion = false;
    std::vector<AlliedUnit> alliedUnits;

    const char* PeerageTitleName(int rank = -1) const;
    const char* PeerageStyleName(int rank = -1) const;
    Color PeerageColor(int rank = -1) const;
    void PromotePeerage();
    void EstablishFief();
    void UpgradeFiefBuilding(int bIdx);
    void RecruitFiefSoldiers();
    int FiefDailyGoldIncome() const;
    void CollectFiefIncome();
    void InitNations();
    void OpenWorldMap(State from);
    void UpdateWorldMap(float dt);
    void DrawWorldMapScreen();
    void DeclareWarOrAlliance(int natIdx);
    void DispatchArmyCampaign(int natIdx);
    void StartNationalBattlefield(int natIdx);
    float ComputePlayerPowerScore() const;
    void StartThroneRebellion(int forcedKingClass = -1);
    void SpawnAlliedGarrison(int count);
    void UpdateAlliedUnits(float dt);
    void DrawAlliedUnits();

    const char* L(const char* en, const char* vi) const { return settings.language == 1 ? vi : en; }
    int TimePhase() const;
    const char* TimePhaseName() const;
    std::string ClockString() const;
    void AdvanceTime(float hours);
    // ── Giai đoạn tuổi của nhân vật chính ──
    // 0: Thanh Niên (<30) +5% tốc chạy | 1: Tráng Niên (30-44) +8% ST, -8% ST nhận
    // 2: Trung Niên (45-59) -5% tốc chạy | 3: Lão Niên (60+) suy giảm dần theo từng năm
    int HeroAgeStage() const;
    const char* AgeStageName() const;
    Color AgeStageColor() const;
    float AgeDamageMult() const;
    float AgeSpeedMult() const;
    float AgeDamageTakenMult() const;
    std::string AgeEffectText() const;
    float YearProgress() const { return Clamp(yearClock / kRealSecondsPerYear, 0.0f, 1.0f); }
    int ChildAgeYears(const ChildInfo& ch) const { return (int)floorf(ch.age); }
    bool ChildCanTrain(const ChildInfo& ch) const { return ch.age >= (float)kChildTrainAge && ch.injuredTimer <= 0.0f; }
    int OldestChildIdx() const;

    // Trả về true nếu thế giới đã thay đổi (vd: nhân vật qua đời vì tuổi già → kế thừa)
    bool UpdateAging(float dt);
    void SetHeroName(const std::string& nm);
    void SetChildName(int idx, const std::string& nm);
    const char* SpouseUnlockConditionText(int idx) const;
    void UnlockSpouse(int idx, const char* msgEn, const char* msgVi);
    void RescueCaptiveSpouse();
    void OpenNameModal(int mode, int childOrSlotIdx, const std::string& defName);
    void ConfirmNameModal();
    void UpdateNameModal();
    void DrawNameModal();
    const char* SpouseName(int idx) const {
        if (idx < 0 || idx >= 4) return L("None", "Chưa có");
        return settings.language == 1 ? kSpouses[idx].nameVi : kSpouses[idx].nameEn;
    }
    const char* HouseName(int tier) const {
        int t = std::max(0, std::min(3, tier));
        return settings.language == 1 ? kHouses[t].nameVi : kHouses[t].nameEn;
    }
    float HouseHungerMult() const;
    float HungerDamageMult() const {
        return (surfaceUnlocked && hunger >= 70.0f) ? 1.10f : 1.0f;
    }
    int MealPrice(int basePrice) const {
        return isNoble ? std::max(5, (int)roundf(basePrice * 0.70f)) : basePrice;
    }
    int HouseUpgradeCost(int targetTier) const;
    std::string CurrentHeroDisplayName() const {
        return heroGivenName.empty() ? std::string(ClassDisplayName(player.heroClass)) : heroGivenName;
    }
    void ResetDynasty();
    const ClassDef& BaseClass() const { return kClasses[std::max(0, std::min(kNumClasses - 1, player.heroClass))]; }
    const SubclassDef* ActiveSubclass() const;
    const char* ClassDisplayName(int hc) const {
        hc = std::max(0, std::min(kNumClasses - 1, hc));
        return settings.language == 1 ? kClassesVi[hc].name : kClasses[hc].name;
    }
    const char* SubDisplayName(int hc, int branch) const;
    const char* HeroTitle() const;
    static int LevelTitleTierForLevel(int lv);
    int CurrentTitleTier() const { return LevelTitleTierForLevel(player.level); }
    const char* LevelTitleNameForLevel(int lv) const {
        int t = LevelTitleTierForLevel(lv);
        return settings.language == 1 ? kLevelTitles[t].nameVi : kLevelTitles[t].nameEn;
    }
    const char* CurrentLevelTitle() const { return LevelTitleNameForLevel(player.level); }
    Color LevelTitleColorForLevel(int lv) const { return kLevelTitles[LevelTitleTierForLevel(lv)].color; }
    Color CurrentLevelTitleColor() const { return LevelTitleColorForLevel(player.level); }
    const char* HeroRole() const;
    Color HeroColor() const { const SubclassDef* sc = ActiveSubclass(); return sc ? sc->color : BaseClass().color; }
    Color HeroAccent() const { const SubclassDef* sc = ActiveSubclass(); return sc ? sc->accent : BaseClass().accent; }
    const char* PassiveName() const;
    const char* PassiveDesc() const;
    const char* QName() const;
    const char* QDesc() const;
    const char* EName() const;
    const char* EDesc() const;
    const char* CName() const;
    const char* CDesc() const;
    const char* VName() const;
    const char* VDesc() const;
    const char* RName() const;
    const char* RDesc() const;
    static int ClassWeaponType(int heroClass) {
        int hc = std::max(0, std::min(kNumClasses - 1, heroClass));
        return kClasses[hc].startWeapon; // Knight: Sword, Assassin: Dagger, Mage: Staff, Ranger: Bow, Templar: Spear
    }
    static int VisualWeaponType(int heroClass, int subClass, int itemWeaponType);
    bool CanUseWeapon(int weaponType) const { return weaponType == ClassWeaponType(player.heroClass); }
    int PreferredWeaponType() const { return ClassWeaponType(player.heroClass); }
    float CritMult() const { return player.heroClass == CLASS_ASSASSIN ? (player.subClass == 1 ? 2.6f : 2.4f) : 2.0f; }

    float SkillCdrMult() const;
    float LifestealStep() const {
        return settings.difficulty == 0 ? 0.08f : (settings.difficulty == 1 ? 0.02f : 0.005f);
    }
    float LifestealCap() const {
        return settings.difficulty == 0 ? 0.30f : (settings.difficulty == 1 ? 0.10f : 0.03f);
    }
    float LifestealScale() const {
        return settings.difficulty == 0 ? 1.0f : (settings.difficulty == 1 ? 0.25f : 0.0625f);
    }
    float Skill1MaxCd() const { return kClasses[std::max(0, std::min(kNumClasses - 1, player.heroClass))].qCooldown * SkillCdrMult(); }
    float Skill2MaxCd() const { return kClasses[std::max(0, std::min(kNumClasses - 1, player.heroClass))].eCooldown * SkillCdrMult(); }
    float Skill3MaxCd() const {
        const SubclassDef* sc = ActiveSubclass();
        return (sc ? sc->cCooldown : 7.5f) * SkillCdrMult();
    }
    float Skill4MaxCd() const {
        const SubclassDef* sc = ActiveSubclass();
        return (sc ? sc->vCooldown : 9.5f) * SkillCdrMult();
    }
    float EffectiveArmor() const { return player.armor + (player.holyBuffTimer > 0.0f ? (player.subClass == 1 ? 6.0f : 4.0f) : 0.0f) + (activeRune == 4 ? 1.8f * runeRank : 0.0f) + (familiarKind == 2 ? 1.5f * familiarRank : 0.0f); }
    float EffectiveAttackRate() const;
    float EffectiveMoveSpeed() const;
    void AddRage(float amt);

    void Init();

    void RestartRun();

    // ── Bioma: 6 quần xã phong phú đổi màu & cảnh quan mỗi 2 tầng ──
    struct BiomeColors {
        Color bg, tileA, tileB, pathCol, trimCol, accentCol, wall, mote;
        const char* nameEn;
        const char* nameVi;
    };
    int BiomeIdx() const { return ((std::max(1, floor) - 1) / 2) % 6; }
    BiomeColors SurfaceBiome() const;
    BiomeColors Biome() const;

    // ── Hiệu ứng hạt ──
    void Burst(Vector2 pos, Color c, int count, float speed, float life);

    void InitMotes();

    void UpdateEffects(float dt);

    Color DeathColor(const Enemy& e);

    // ── Nâng cấp ──
    std::string UpgradeTitle(UpgradeType u);

    std::string UpgradeDesc(UpgradeType u);

    void ShowBanner(const std::string& txt, float dur = 2.5f) {
        bannerText = txt;
        bannerTimer = dur;
    }

    void Shake(float dur, float mag);

    static Vector2 SurfaceLandmarkPos(int idx);

    static Vector2 SurfaceTownExitSpot(int roomIdx);

    Vector2 SurfaceInteriorExitPos() const {
        return { 0.0f, roomH / 2.0f - 48.0f };
    }

    static Vector2 TavernStallPos(int i);

    static Vector2 HousingPedestalPos(int tierIdx);

    static Vector2 SpouseCandPos(int i);

    static Vector2 HeirPosInHome(int k) {
        return { 185.0f + (k % 2) * 155.0f, -45.0f + (k / 2) * 135.0f };
    }

    void SetupSurfaceRoomEnvironment(int roomIdx, bool movePlayer);

    void EnterSurfaceRoom(int roomIdx);

    void LeaveSurfaceRoomToTown();

    int SpouseCost(int idx) const {
        (void)idx;
        return 0; // Vợ không mua bằng vàng — chỉ mở khóa qua giải cứu / nhiệm vụ!
    }

    void OpenSurfaceModal(int tab);

    void OpenFamilyTree(State from);

    void EnterSurface(int nextF);

    void LeaveSurfaceToDungeon();

    void EatRation();

    void BuyMeal(int kind);

    void BuyHouseUpgrade(int targetTier);

    void TryMarrySpouse(int idx);

    void TryHaveChild();

    void AcceptRoyalCommission();

    void CompleteRoyalCommission();

    void BuyNobleTitleDirect();

    void SuccessionToChild(int childIdx, bool forced = false);

    // ══════════════ HỆ THỐNG DẪN CON XUỐNG HẦM NGỤC RÈN LUYỆN ══════════════
    static bool ChildIsRanged(int hc) { return hc == CLASS_MAGE || hc == CLASS_RANGER; }
    float ChildGrowth(const ChildInfo& ch) const { return Clamp(0.45f + (ch.age / 18.0f) * 0.55f, 0.45f, 1.0f); }
    float ChildMaxHpStat(const ChildInfo& ch) const {
        return std::max(20.0f, (player.maxHp * (0.30f + 0.012f * ch.level) + ch.bonusHp) * ChildGrowth(ch));
    }
    float ChildDamageStat(const ChildInfo& ch) const {
        return std::max(2.0f, (player.baseDmg * (0.18f + 0.008f * ch.level) + ch.bonusDmg * 0.5f) * ChildGrowth(ch));
    }
    float ChildArmorStat(const ChildInfo& ch) const { return 1.0f + ch.bonusArmor + ch.level * 0.08f; }
    float ChildAttackInterval(const ChildInfo& ch) const;
    Vector2 ChildFormationOffset(int k) const {
        static const Vector2 offs[4] = { { -46.0f, 40.0f }, { 46.0f, 40.0f }, { 0.0f, 62.0f }, { -70.0f, 0.0f } };
        return offs[std::max(0, k) % 4];
    }
    int ActivePartyCount() const;

    void ToggleChildParty(int idx);

    void SpawnPartyChildren();

    void ChildGainXp(ChildInfo& ch, int amount);

    void GrantPartyXp(const Enemy& e);

    void ChildDamageEnemy(Enemy& e, float dmg, int ci, Vector2 kb);

    void ChildTakeDamage(int ci, float dmg);

    void UpdatePartyChildren(float dt);

    void DrawPartyChildren();

    void DrawPartyHud(int y);

    void BuildPortalChoices();

    void BuildFloor();

    void SpawnWarden(Vector2 pos);

    Vector2 GetRandomSpawnPos();

    void SpawnEnemy(EnemyType type, Vector2 pos) { enemies.push_back(BuildEnemy(type, pos)); }

    Enemy BuildEnemy(EnemyType type, Vector2 pos);

    void SpawnPopup(Vector2 pos, const std::string& txt, Color col);

    bool HasRelic(RelicType t) const;

    static Relic MakeRelic(RelicType t);

    void AddRelic(RelicType t);

    void Update(float dt);

    void DamageBarrel(ArenaBarrel& b, float dmg);

    void TriggerBarrelsInRadius(Vector2 center, float radius, float dmg);

    void TriggerProjectileSplash(Vector2 pos, float baseSplashDmg, float radius, Color col, bool applyBurn, int skipIdx);

    void DamageEnemy(Enemy& e, float dmg, Vector2 kb, float kbForce, bool isCrit, bool allowRage = true);

    void OnEnemyKilled(Enemy& e);

    void KnightCounterShockwave();

    bool DamagePlayer(float dmg);

    void BomberExplode(Enemy& e);

    void UseShrine();

    void CheckAndGrantLevelTitleMilestones();

    void TriggerLevelUp();

    void ChooseSubclass(int branch);

    void ChooseUpgrade(UpgradeType up);

    void ApplyUpgrade(UpgradeType up);

    // ── Vẽ nhân vật & quái bằng mô hình đa giác phân mảnh (khớp chi & giáp) ──
    static void Tri(Vector2 a, Vector2 b, Vector2 c, Color col) {
        DrawTriangle(a, b, c, col);
        DrawTriangle(a, c, b, col); // vẽ cả hai chiều để không bị culling
    }

    static Color Shade(Color c, float k) { // k < 1: tối hơn, k > 1: sáng hơn
        return { (unsigned char)std::min(255.0f, c.r * k), (unsigned char)std::min(255.0f, c.g * k),
                 (unsigned char)std::min(255.0f, c.b * k), c.a };
    }

    static Color TintCol(Color col, const Enemy& e);

    static void Quad(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color col) {
        Tri(a, b, c, col);
        Tri(a, c, d, col);
    }

    static void QuadOut(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color fill, Color border, float thick = 1.6f);

    static void BoxRot(Vector2 center, float w, float h, float deg, Color fill, Color border, float thick = 1.5f);

    static void Diamond(Vector2 c, float rx, float ry, Color fill, Color border);

    // Mắt nhìn theo hướng look (vector chuẩn hoá)
    static void Eye(Vector2 c, float rx, float ry, Vector2 look, Color iris);

    void DrawEnemyBody(const Enemy& e);

    static void DrawWeaponModel(Vector2 hand, Vector2 dir, int wt, float t, bool enchanted = false);

    static void DrawHeroModel(Vector2 p, int heroClass, int wt, Vector2 aimDir, bool facingLeft, float walkTimer, bool moving,
                              float atkSweep, bool isInvuln, bool shieldActive, float holyBuffTimer, float ultActiveTimer,
                              int subClass = 0, float swordOrbitTimer = 0.0f);

    void DrawPlayerSprite();

    // Độ khó theo tầng: scaling bậc hai để tránh người chơi out-power quái ở tầng sau và phân tách rõ Dễ/Thường/Khó
    float DiffHp() const { return settings.difficulty == 0 ? 0.85f : (settings.difficulty == 2 ? 2.15f : 1.35f); }
    float DiffDmg() const { return settings.difficulty == 0 ? 0.75f : (settings.difficulty == 2 ? 1.85f : 1.25f); }
    float DiffSouls() const { return settings.difficulty == 0 ? 0.75f : (settings.difficulty == 2 ? 1.5f : 1.0f); }
    float HpMult() const {
        float f = (float)(floor - 1);
        float curve = 1.0f + f * 0.34f + f * f * 0.026f + std::max(0, floor - 8) * 0.28f;
        return curve * DiffHp() * (1.0f + curseStacks * 0.12f);
    }
    float DmgMult() const {
        float f = (float)(floor - 1);
        float curve = 1.0f + f * 0.18f + f * f * 0.008f + std::max(0, floor - 8) * 0.18f;
        return curve * DiffDmg() * (1.0f + curseStacks * 0.12f);
    }
    float EliteChance() const {
        float base = (settings.difficulty == 2) ? 0.30f : (settings.difficulty == 1 ? 0.20f : 0.14f);
        return std::min(0.65f, base + (floor - 1) * 0.016f);
    }

    void AddStyle(float pts);
    const char* StyleRankLabel() const;
    Color StyleRankColor() const;
    void TriggerElementalReaction(Enemy& e, int reactionKind, Vector2 pos);
    void TriggerPerfectDodge();
    void AcceptBloodPact();
    void ReforgeEquipSlot(int slot);
    void AcceptOrClaimBounty();
    const char* BloodlineTraitName() const;
    const char* BloodlineTraitDesc() const;
    const char* WeatherName() const;
    const char* WeatherDesc() const;
    Color WeatherColor() const;
    const char* RuneName() const;
    Color RuneColor() const;
    int RunicAnvilCost() const { return 35 + floor * 12; }
    void UseRunicAnvil();
    const char* FamiliarName() const;
    Color FamiliarColor() const;
    void GrantOrUpgradeFamiliar();
    void UpdateFamiliar(float dt);
    void DrawFamiliarSprite();
    void ApplyEquipEnhanceStep(int slot);
    void QuickForgeWithEssence();
    bool HasDuoTempestFrost() const { return HasRelic(RELIC_LIGHTNING) && HasRelic(RELIC_FROST); }
    bool HasDuoPlaguefire() const { return HasRelic(RELIC_TOXIC) && HasRelic(RELIC_COMBUSTION); }
    bool HasDuoImmortalBastion() const { return HasRelic(RELIC_AEGIS) && HasRelic(RELIC_CHALICE); }

    // ── Nút bấm trên màn hình (dùng được cho chuột lẫn chạm) ──
    enum UiAction { UI_NONE = 0, UI_START, UI_SAVE_LOAD, UI_SANCT, UI_SETTINGS, UI_HELP, UI_QUIT, UI_RESUME, UI_EQUIP, UI_RESTART, UI_BACK, UI_CLOSE_HELP, UI_CLOSE_INV, UI_CHANGE_CLASS, UI_MAIN_MENU, UI_FAMILY_TREE, UI_CONTINUE_HEIR, UI_SURFACE_RETURN, UI_WORLD_MAP };
    struct UiBtn { Rectangle r; std::string label; int action; Color col; };

    std::vector<UiBtn> BuildButtons();

    int HitTestButtons(Vector2 m);

    int HitButton();

    void HandleUiAction(int a);

    void DrawButtons();

    // ── Điều khiển cảm ứng: cần ảo nổi bên trái, nút chém / lướt / Nova bên phải ──
    void PollTouches();

    static constexpr float kAtkR = 78.0f, kDashR = 54.0f, kNovaR = 48.0f, kSmallR = 26.0f;
    Vector2 AtkC() const { return { 1150, 570 }; }
    Vector2 DashC() const { return { 1010, 650 }; }
    Vector2 NovaC() const { return { 1020, 508 }; }
    Vector2 Skill2C() const { return { 1110, 428 }; }
    Vector2 UltC() const { return { 1218, 434 }; }
    Vector2 PauseC() const { return { 1236, 172 }; }
    Vector2 GearC() const { return { 1170, 172 }; }
    Rectangle TradeRect() const { return { 560, 536, 160, 56 }; }

    void PollTouchControls();

    void DrawTouchControls();

    // ── Tay cầm ──
    bool PadPressed(int button) { return IsGamepadAvailable(0) && IsGamepadButtonPressed(0, button); }

    // Rung tay cầm (chỉ khi bật Vibration và có tay cầm)
    void Rumble(float strong, float weak, float duration);

    void StopRumble() { rumbleTimer = 0.0f; RumbleRaw(0.0f, 0.0f); }

    // Cần trái di chuyển, cần phải ngắm (không có thì tự ngắm vào quái gần nhất)
    void PollGamepad();

    // ── Quái phụ sinh ra khi đang duyệt danh sách: đưa vào hàng đợi, thêm vào cuối khung ──
    void FlushSpawns();

    void SpawnMinionChaser(Vector2 pos);

    void SpawnSplitMini(Vector2 pos);

    // Relic chưa có, chọn ngẫu nhiên. Trả về true nếu còn relic để trao.
    bool GiveRandomRelic();

    const char* RelicDesc(RelicType t);

    // Đòn thorn: nổ ra sóng xung kích khi người chơi bị thương
    void ThornBurst();

    // Helper phóng kiếm khí trăng khuyết (Kiếm Sư / Cuồng Chiến)
    void FireSwordWave(Vector2 origin, Vector2 dir, float dmg, float spd, int pierce, Color col, float rad = 13.0f, bool applyBurn = false);

    // Đòn đánh cơ bản theo vũ khí đặc trưng từng class + hiệu ứng nhánh phụ
    void MeleeAttack(Vector2 aimDir);

    // ── Nội tại Pháp Sư / Đại Pháp Sư: mỗi lần dùng kỹ năng bắn 3 (hoặc 5) tia ma thuật dẫn đường ──
    void TriggerArcaneResonance(Vector2 aimDir);

    // ── Kỹ năng 1 [Q] ──
    void CastSkill1(Vector2 aimDir);

    // ── Kỹ năng 2 [E] ──
    void CastSkill2(Vector2 aimDir, Vector2 moveDir);

    // ── Kỹ năng Nhánh Phụ 3 [C / Z] ──
    void CastSkill3(Vector2 aimDir);

    // ── Kỹ năng Nhánh Phụ 4 [V / X] ──
    void CastSkill4(Vector2 aimDir, Vector2 moveDir);

    // ── Kỹ năng Nộ [R] ──
    void CastUltimate(Vector2 aimDir);

    void UpdateActiveSkills(float dt);

    // ── Bố cục phòng & va chạm cột ──
    void BuildPillars();

    // Đẩy một vật tròn ra khỏi các cột. Trả về true nếu có va chạm.
    bool ResolvePillars(Vector2& pos, float radius);

    const char* ModLabel();

    // ── Rương ──
    void UpdateChests(float dt);

    void DrawChest(const Chest& c);

    // ── Cửa hàng ──
    void StockShop();

    void BuyShopSlot(int i);

    void UpdateShop();

    void DrawShopScreen();

    void DrawMerchant();

    // ── Thánh địa ──
    int MetaCost(int i) { return kMeta[i].baseCost + kMeta[i].costStep * meta[i]; }

    void AwardSouls();

    void BuyMeta(int i);

    void OpenSanctuary(State from);

    void CloseSanctuary();

    void UpdateMeta(float dt);

    void DrawMetaScreen();

    // ── Cài đặt ──
    void LoadSettings();

    void SaveSettings();

    void BuildResList();

    void ApplySettings();

    void ChangeSetting(int row, int d);

    std::string SettingValue(int row);

    void UpdateSettingsMenu();

    void DrawSettingsScreen();

    // Vẽ khung 1280x720 lên cửa sổ với viền đen giữ tỉ lệ, và ánh xạ chuột về toạ độ ảo
    void Present();

    // ── Trang bị ──
    void ApplyItem(const EquipItem& it, float sign);

    // Mặc nếu mạnh hơn món đang đeo và đúng loại vũ khí của class (món cũ bán lấy vàng).
    bool TryEquipItem(const EquipItem& it, int& soldGold);

    void SpawnItemDrop(Vector2 pos);

    void UpdateItemDrops(float dt);

    static void DrawItemIcon(int slot, Vector2 c, float s, Color col, int weaponType = 0);

    void DrawItemDrops();

    void DrawInventory();

    // ── Môi trường: tường gạch, decal, đuốc, cột, ánh sáng ──
    void BuildDecor();

    void DrawWalls(const BiomeColors& biome);

    void DrawDecals(const BiomeColors& biome);

    void DrawPillar(Vector2 p, const BiomeColors& biome);

    void DrawTorch(Vector2 t);

    void AddLight(Vector2 wp, float radius, Color col, float inten);

    // Bản đồ ánh sáng nhân với cảnh: xa nguồn sáng thì tối dần
    void RenderLightmap();

    void ApplyLighting();

    // ── Vẽ chữ tự xuống dòng & biểu tượng kỹ năng ──
    static int DrawWrappedText(const char* txt, int x, int y, int maxW, int fontSize, int lineH, Color col);

    static void DrawSkillIcon(int heroClass, int slotKind, Vector2 c, float s, Color col);

    // ── Màn hình chọn nhân vật ──
    void UpdateClassSelect();

    void DrawClassSelectScreen();

    // ── Hệ thống Lưu / Tải game (tối đa 4 slot, hỗ trợ ghi đè) ──
    static std::string SaveFileName(int slot) {
        return "save_slot_" + std::to_string(std::max(0, std::min(3, slot)) + 1) + ".sav";
    }

    void RefreshSaveSlots();

    void DeleteSaveSlot(int slot);

    bool SaveGameToSlot(int slot);

    bool LoadGameFromSlot(int slot);

    void OpenSaveLoad(State from);

    void UpdateSaveLoad(float dt);

    void DrawSaveLoadScreen();

    // ── Màn hình tiêu đề / tạm dừng / hướng dẫn ──
    static void OutlinedText(const char* txt, int x, int y, int size, Color col, Color outline);

    void DrawHelpPanel();

    void DrawTitleScreen();

    void DrawPauseScreen();

    int SurfaceOptionCount() const;

    void TriggerSurfaceOption(int tab, int opt);

    void UpdateSurfaceUi();

    void DrawSurfaceUi();

    void UpdateFamilyTree();

    void DrawFamilyTreeScreen();

    static void DrawSurfacePrompt(Vector2 p, const char* title, const char* sub, Color accent, bool near);

    // npcKind: 0..3 = 4 Nàng Dâu, 4 = Đức Vua, 5 = Chủ Quán Trọ, 6 = Kiến Trúc Sư
    static void DrawDetailedNpcModel(Vector2 pos, int npcKind, Color primaryCol, Color secondaryCol, Color hairCol, bool crown, float bobPhase);

    void DrawSurfaceExitDoor(Vector2 pos);

    void DrawMiniHouseModel(Vector2 p, int tier, Color roofCol, Color trimCol);

    void DrawSurfaceTownSquare(const BiomeColors& biome, float tm);

    void DrawSurfaceTavernRoom(const BiomeColors& biome, float tm);

    void DrawSurfaceHousingRoom(const BiomeColors& biome, float tm);

    void DrawSurfaceFamilyHomeRoom(const BiomeColors& biome, float tm);

    void DrawSurfacePalaceRoom(const BiomeColors& biome, float tm);

    void DrawSurfaceFiefRoom(const BiomeColors& biome, float tm);

    void DrawSurfaceWorld(const BiomeColors& biome);

    // ── Drawing ──
    void Draw();
};
