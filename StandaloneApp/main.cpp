#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <random>
#include <ctime>

// ── Audio Procedural Synthesizer ───────────────────────────────────────────
struct SoundManager {
    Sound sndSlash;
    Sound sndHit;
    Sound sndDeath;
    Sound sndShoot;
    Sound sndDash;
    Sound sndCoin;
    Sound sndPotion;
    Sound sndLevelUp;
    Sound sndPortal;
    Sound sndHurt;
    Sound sndSpecial;
    Sound sndShield;
    Sound sndFreeze;

    void Init() {
        InitAudioDevice();
        sndSlash   = GenTone(0.12f, 440, 150, 3);
        sndHit     = GenTone(0.08f, 220, 80, 1);
        sndDeath   = GenTone(0.25f, 300, 50, 4);
        sndShoot   = GenTone(0.14f, 880, 440, 0);
        sndDash    = GenTone(0.15f, 600, 200, 3);
        sndHurt    = GenTone(0.12f, 180, 90, 2);
        sndSpecial = GenTone(0.35f, 180, 40, 0);
        sndFreeze  = GenTone(0.18f, 1200, 600, 0);

        sndCoin    = GenChime({ 988.0f, 1318.0f }, 0.08f);
        sndPotion  = GenChime({ 523.0f, 659.0f, 784.0f }, 0.07f);
        sndLevelUp = GenChime({ 523.0f, 659.0f, 784.0f, 1046.0f }, 0.12f);
        sndPortal  = GenChime({ 440.0f, 554.0f, 659.0f, 880.0f }, 0.15f);
        sndShield  = GenChime({ 880.0f, 1320.0f }, 0.09f);
    }

    void Close() {
        UnloadSound(sndSlash);
        UnloadSound(sndHit);
        UnloadSound(sndDeath);
        UnloadSound(sndShoot);
        UnloadSound(sndDash);
        UnloadSound(sndCoin);
        UnloadSound(sndPotion);
        UnloadSound(sndLevelUp);
        UnloadSound(sndPortal);
        UnloadSound(sndHurt);
        UnloadSound(sndSpecial);
        UnloadSound(sndShield);
        UnloadSound(sndFreeze);
        CloseAudioDevice();
    }

private:
    Sound GenTone(float duration, float startFreq, float endFreq, int type) {
        int sampleRate = 44100;
        int sampleCount = (int)(duration * sampleRate);
        short* samples = (short*)malloc(sampleCount * sizeof(short));
        float phase = 0.0f;

        for (int i = 0; i < sampleCount; ++i) {
            float t = (float)i / sampleCount;
            float freq = startFreq + (endFreq - startFreq) * t;
            float env = 1.0f - t;
            float s = 0.0f;
            if (type == 0) s = sinf(phase);
            else if (type == 1) s = (sinf(phase) >= 0 ? 0.6f : -0.6f);
            else if (type == 2) s = (2.0f * (phase / (2.0f * PI) - floorf(phase / (2.0f * PI) + 0.5f))) * 0.7f;
            else if (type == 3) s = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.5f;
            else if (type == 4) s = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * (1.0f - t * 0.8f);

            phase += (2.0f * PI * freq) / sampleRate;
            samples[i] = (short)(s * env * 24000.0f);
        }

        Wave wave = { 0 };
        wave.frameCount = sampleCount;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples;
        Sound snd = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return snd;
    }

    Sound GenChime(const std::vector<float>& freqs, float noteDur) {
        int sampleRate = 44100;
        int noteSamples = (int)(noteDur * sampleRate);
        int totalSamples = (int)(noteSamples * freqs.size());
        short* samples = (short*)malloc(totalSamples * sizeof(short));

        for (size_t n = 0; n < freqs.size(); ++n) {
            float freq = freqs[n];
            float phase = 0.0f;
            for (int i = 0; i < noteSamples; ++i) {
                int idx = (int)(n * noteSamples + i);
                float t = (float)i / noteSamples;
                float env = powf(1.0f - t, 1.5f);
                float s = sinf(phase) * 0.7f;
                phase += (2.0f * PI * freq) / sampleRate;
                samples[idx] = (short)(s * env * 22000.0f);
            }
        }

        Wave wave = { 0 };
        wave.frameCount = totalSamples;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples;
        Sound snd = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return snd;
    }
};

// ── Game Data Structures ───────────────────────────────────────────────────
enum EnemyType { CHASER, RANGER, BRUTE, BOSS, BOMBER, CHARGER, SPLITTER, SUMMONER };
enum RelicType { RELIC_LIGHTNING, RELIC_TOXIC, RELIC_FROST, RELIC_AEGIS, RELIC_CHALICE, RELIC_COMBUSTION, RELIC_PHOENIX, RELIC_MIDAS, RELIC_BERSERK, RELIC_THORNS };
enum UpgradeType { UP_HP, UP_DMG, UP_SPEED, UP_ATTACK_RATE, UP_DASH, UP_LIFESTEAL, UP_ARMOR, UP_CRIT, UP_NOVA, UP_IGNITE, UP_FROST };
enum HeroClass { CLASS_KNIGHT = 0, CLASS_ASSASSIN = 1, CLASS_MAGE = 2, CLASS_RANGER = 3 };
enum WeaponType { WP_SWORD = 0, WP_DAGGER = 1, WP_AXE = 2, WP_SPEAR = 3, WP_HAMMER = 4, WP_SCYTHE = 5, WP_BOW = 6, WP_STAFF = 7 };

struct ClassDef {
    const char* name;
    const char* role;
    Color color;
    Color accent;
    int startWeapon;
    float baseHp;
    float baseSpeed;
    float baseDmg;
    float baseAtkRate;
    float baseArmor;
    float baseCrit;
    float qCooldown;
    float eCooldown;
    const char* passiveName;
    const char* passiveDesc;
    const char* qName;
    const char* qDesc;
    const char* eName;
    const char* eDesc;
    const char* rName;
    const char* rDesc;
};

static const ClassDef kClasses[4] = {
    {
        "Vanguard Knight", "Armored Bruiser", { 70, 145, 245, 255 }, { 255, 210, 80, 255 }, WP_SWORD,
        120.0f, 275.0f, 26.0f, 2.5f, 2.0f, 0.05f, 5.5f, 7.0f,
        "Unyielding Aegis", "-15% dmg taken; hits/blocks +8 Rage & reflect shockwave; 5th hit heals 4 HP",
        "Shield Bash", "Lunge forward, slam 1.9x DMG & Stun 1.2s",
        "Holy Nova", "1.8x radial DMG + 3.5s Barrier (+4 ARM, +25% ATK SPD, block 1 hit)",
        "Heaven's Judgment", "3.5x Light Sword + Stun, 4s holy zone (0.85x ticks & 4% HP/s heal)"
    },
    {
        "Shadow Assassin", "Crit Striker", { 225, 65, 110, 255 }, { 185, 90, 255, 255 }, WP_DAGGER,
        85.0f, 320.0f, 22.0f, 3.2f, 0.0f, 0.18f, 4.5f, 6.5f,
        "Lethal Momentum", "Crits deal 2.4x DMG + poison; kills refund 50% Dash CD & +25% SPD 2s",
        "Fan of Knives", "Throw 5 piercing shadow daggers (1.1x DMG, poison & slow)",
        "Shadow Step", "Blink up to 210px, drop smoke bomb & 2.2x guaranteed-crit slash",
        "Blade Storm", "3.5s shadow whirlwind (+35% SPD, -50% dmg taken, 0.85x slashes/0.2s)"
    },
    {
        "Arcane Mage", "Elemental Caster", { 125, 110, 255, 255 }, { 95, 230, 255, 255 }, WP_STAFF,
        80.0f, 285.0f, 28.0f, 2.3f, 0.5f, 0.10f, 5.0f, 7.5f,
        "Arcane Resonance", "-15% Skill CDs; casting Q/E/R fires 3 homing missiles & +6 Rage",
        "Fireball", "Blazing comet explodes in 135px AoE for 2.3x DMG & Burn",
        "Frost Nova", "185px ice blast dealing 1.5x DMG, Freeze/Stun 1.3s & Chill",
        "Void Cataclysm", "3.8s black hole pulls enemies & rains 12 meteors (1.5x AoE each)"
    },
    {
        "Emerald Ranger", "Marksman", { 75, 210, 125, 255 }, { 255, 200, 75, 255 }, WP_BOW,
        90.0f, 310.0f, 24.0f, 2.9f, 0.5f, 0.14f, 4.8f, 7.0f,
        "Eagle Mark", "Every 3rd attack fires a piercing 1.5x Wind Shot (+10% Crit)",
        "Multishot Volley", "Fire a 7-arrow fan (1.0x DMG each) with knockback",
        "Explosive Vault", "Drop an explosive trap (2.4x AoE + slow) & vault back",
        "Phoenix Rain", "3.6s storm of 16 fiery arrows (1.15x + Burn) & +35% ATK SPD"
    }
};

struct Relic {
    RelicType type;
    std::string name;
    std::string symbol;
    Color color;
};

struct Player {
    int heroClass = CLASS_KNIGHT;
    Vector2 pos;
    Vector2 vel;
    float maxHp = 120.0f;
    float hp = 120.0f;
    float hpDisplay = 120.0f;
    float speed = 275.0f;
    float baseDmg = 26.0f;
    float attackRate = 2.5f;
    float attackTimer = 0.0f;
    float dashTimer = 0.0f;
    float dashCooldown = 1.2f;
    bool isDashing = false;
    float dashDur = 0.2f;
    Vector2 dashDir;
    float specialTimer = 0.0f;
    float specialCooldown = 5.5f;
    float skill2Timer = 0.0f;
    float skill2Cooldown = 7.0f;
    float rage = 0.0f;
    float maxRage = 100.0f;
    float ultActiveTimer = 0.0f;
    float ultTickTimer = 0.0f;
    float holyBuffTimer = 0.0f;
    float hasteTimer = 0.0f;
    int attackCounter = 0;
    int cdrUpgrades = 0;
    float critChance = 0.05f;
    float lifesteal = 0.0f;
    float armor = 2.0f;
    int level = 1;
    int xp = 0;
    int xpNext = 50;
    int gold = 0;
    bool isInvuln = false;
    float invulnTimer = 0.0f;
    bool facingLeft = false;
    float walkTimer = 0.0f;
    std::vector<Relic> relics;
    bool shieldActive = false;
    float shieldTimer = 0.0f;
    int igniteLevel = 0; // Ember Edge: mỗi cấp = 50% sát thương cơ bản/giây trong 3s
    int frostLevel = 0;  // Frost Touch: mỗi cấp giảm 15% tốc độ quái (tối thiểu còn 40%)
};

struct Enemy {
    EnemyType type;
    Vector2 pos;
    Vector2 vel;
    float maxHp;
    float hp;
    float speed;
    float contactDmg;
    float radius;
    bool isElite = false;
    float bounceTimer = 0.0f;
    float attackCooldown = 0.0f;
    bool isAttacking = false;
    float attackTimer = 0.0f;
    bool facingLeft = false;
    float poisonTimer = 0.0f;
    float poisonDmg = 0.0f;
    float chillTimer = 0.0f;
    bool enraged = false;
    bool isWarden = false;   // mini-boss ở tầng 3 và 8
    bool fusing = false;     // Bomber đang cháy ngòi
    float fuseTimer = 0.0f;
    bool exploded = false;   // Bomber đã tự nổ (không rơi loot, không tính combo)
    float burnTimer = 0.0f;  // Ember Edge
    float burnDps = 0.0f;
    float slowTimer = 0.0f;  // Frost Touch
    float slowFactor = 1.0f;
    float stunTimer = 0.0f;  // Choáng từ kỹ năng / Búa chiến
    float hitFlash = 0.0f;   // nháy trắng khi trúng đòn
    int chargeState = 0;     // Charger: 0 tiến lại, 1 báo hiệu, 2 lao tới, 3 choáng
    float chargeTimer = 0.0f;
    float chargeCooldown = 1.2f;
    Vector2 chargeDir = { 1, 0 };
    bool isMinion = false;     // quái phụ: không rơi trang bị, ít XP
    int splitLevel = 0;        // Splitter: 0 = bản lớn, 1 = mảnh nhỏ
    float summonTimer = 3.0f;  // Summoner: đếm ngược triệu hồi; Colossus: đếm số đòn
    int bossKind = 0;          // 0 = Void Sovereign, 1 = Molten Colossus
};

struct Projectile {
    Vector2 pos;
    Vector2 vel;
    float damage;
    bool fromPlayer;
    float life;
    float radius;
    Color color;
    int kind = 0;          // 0: đạn quái, 1: cung tên, 2: cầu phép trượng, 3: phi tiêu, 4: cầu lửa, 5: tên dẫn đường
    int pierce = 0;
    int hitMask = -1;
    float splashRadius = 0.0f;
    float splashRatio = 0.5f;
    bool applyBurn = false;
    bool applySlow = false;
    bool applyPoison = false;
    bool hasPreCrit = false;
    bool preCrit = false;
    float critBonus = 0.0f;
};

struct GroundEffect {
    int kind = 0; // 0: Bẫy nổ Ranger, 1: Thánh vực Knight, 2: Hố đen Mage, 3: Vùng Mưa Tên Ranger, 4: Thiên thạch rơi, 5: Mũi tên Phượng Hoàng rơi
    Vector2 pos;
    float radius = 100.0f;
    float timer = 1.0f;
    float maxTimer = 1.0f;
    float tickTimer = 0.0f;
    float damage = 0.0f;
    Color color = GOLD;
};

struct Loot {
    int type; // 0: XP, 1: Gold, 2: Potion
    Vector2 pos;
    int value;
    float life;
};

struct DamagePopup {
    Vector2 pos;
    std::string text;
    Color color;
    float timer = 0.65f;
    float maxTimer = 0.65f;
};

struct SlashVisual {
    Vector2 pos;
    float angle;
    int kind = 0; // 0..7 vũ khí; 10: xung kích khiên, 11: ám tập, 12: bão kiếm
    float timer = 0.15f;
    float maxTimer = 0.15f;
    Color color = WHITE;
};

struct NovaVisual {
    Vector2 pos;
    float radius = 0.0f;
    float maxRadius = 180.0f;
    float timer = 0.35f;
    float maxTimer = 0.35f;
    Color color;
};

struct GhostTrail {
    Vector2 pos;
    bool facingLeft;
    float timer = 0.22f;
    float maxTimer = 0.22f;
};

struct Particle {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float life;
    float maxLife;
    float size;
};

struct Mote {
    Vector2 pos;
    float speed;
    float phase;
    float size;
};

// ── Rung tay cầm Xbox trên Windows (XInput nạp động, không cần thư viện thêm) ──
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall LoadLibraryA(const char*);
    __declspec(dllimport) void* __stdcall GetProcAddress(void*, const char*);
}
struct XVibration { unsigned short left, right; };
typedef unsigned long (__stdcall *XSetStateFn)(unsigned long, XVibration*);
static XSetStateFn g_xSetState = nullptr;
static bool g_xTried = false;
static void RumbleRaw(float left, float right) {
    if (!g_xTried) {
        g_xTried = true;
        void* h = LoadLibraryA("xinput1_4.dll");
        if (!h) h = LoadLibraryA("xinput9_1_0.dll");
        if (h) g_xSetState = (XSetStateFn)GetProcAddress(h, "XInputSetState");
    }
    if (g_xSetState) {
        XVibration v;
        v.left = (unsigned short)(std::max(0.0f, std::min(1.0f, left)) * 65535.0f);
        v.right = (unsigned short)(std::max(0.0f, std::min(1.0f, right)) * 65535.0f);
        g_xSetState(0, &v);
    }
}
#else
static void RumbleRaw(float, float) {}
#endif

struct Decal {
    Vector2 pos;
    float angle;
    float size;
    int type;
};

static Color LerpColor(Color a, Color b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return { (unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t),
             (unsigned char)(a.b + (b.b - a.b) * t), (unsigned char)(a.a + (b.a - a.a) * t) };
}

static bool EscPressed() { return IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACK); } // Back của Android coi như Esc

static float Rand01() { return (float)rand() / (float)RAND_MAX; }

static void CircleGrad(float x, float y, float r, Color inner, Color outer) {
#if RAYLIB_VERSION_MAJOR >= 6
    DrawCircleGradient({ x, y }, r, inner, outer);
#else
    DrawCircleGradient((int)x, (int)y, r, inner, outer);
#endif
}

// ── Trang bị (4 ô: vũ khí, giáp, giày, nhẫn) ─────────────────────────────
enum EquipSlot { SLOT_WEAPON, SLOT_ARMOR, SLOT_BOOTS, SLOT_RING };

struct EquipItem {
    std::string name;
    int slot = 0;
    int rarity = 0; // 0 thường, 1 hiếm, 2 sử thi, 3 huyền thoại
    int weaponType = 0; // 0 kiếm, 1 dao găm, 2 rìu, 3 giáo, 4 búa, 5 lưỡi hái, 6 cung, 7 trượng
    float damage = 0, maxHp = 0, armor = 0, moveSpeed = 0, attackRate = 0, lifesteal = 0, crit = 0;

    float Score() const { return damage + maxHp * 0.3f + armor * 4.0f + moveSpeed * 6.0f + attackRate * 8.0f + lifesteal * 100.0f + crit * 80.0f; }
    int SellValue() const { return (int)roundf(Score() * 0.5f) + 5; }
    Color RarityColor() const {
        switch (rarity) {
            case 1: return { 77, 153, 255, 255 };
            case 2: return { 179, 77, 255, 255 };
            case 3: return { 255, 153, 26, 255 };
            default: return { 217, 217, 217, 255 };
        }
    }
    const char* RarityName() const {
        static const char* n[4] = { "Common", "Rare", "Epic", "Legendary" };
        return n[std::max(0, std::min(3, rarity))];
    }
    std::vector<std::string> StatLines() const {
        std::vector<std::string> v;
        if (slot == SLOT_WEAPON) {
            static const char* trait[8] = {
                "Balanced melee swing",
                "Fast twin-blade strikes",
                "Heavy wide cleave",
                "Long piercing thrust",
                "Crushing slam (stuns)",
                "Wide reap (+5% lifesteal)",
                "Ranged high-speed arrows",
                "Ranged explosive arcane bolts"
            };
            v.push_back(trait[std::max(0, std::min(7, weaponType))]);
        }
        auto fmt = [](float x) { std::ostringstream ss; ss << (x == (int)x ? (int)x : roundf(x * 10) / 10); return ss.str(); };
        if (damage > 0) v.push_back("+" + fmt(damage) + " Damage");
        if (maxHp > 0) v.push_back("+" + fmt(maxHp) + " Max HP");
        if (armor > 0) v.push_back("+" + fmt(armor) + " Armor");
        if (moveSpeed > 0) v.push_back("+" + fmt(moveSpeed) + " Move Speed");
        if (attackRate > 0) v.push_back("+" + fmt(attackRate) + " Attacks/s");
        if (lifesteal > 0) v.push_back("+" + std::to_string((int)roundf(lifesteal * 100)) + "% Lifesteal");
        if (crit > 0) v.push_back("+" + std::to_string((int)roundf(crit * 100)) + "% Crit");
        return v;
    }
};

struct ItemDrop {
    Vector2 pos;
    EquipItem item;
    float life;
    float phase;
};

static float R1(float v) { return roundf(v * 10.0f) / 10.0f; }

static EquipItem MakeStarterWeapon(int weaponType) {
    static const char* starterNames[8] = {
        "Knight's Sword", "Shadow Dagger", "Iron Axe", "Guard Spear",
        "Oak Hammer", "Reaper Scythe", "Hunter Bow", "Initiate Staff"
    };
    EquipItem it;
    it.slot = SLOT_WEAPON;
    it.rarity = 0;
    it.weaponType = std::max(0, std::min(7, weaponType));
    it.damage = 3.0f;
    it.name = starterNames[it.weaponType];
    return it;
}

static EquipItem GenerateItem(int floor) {
    static const char* weaponNames[8] = { "Sword", "Dagger", "Axe", "Spear", "Hammer", "Scythe", "Bow", "Staff" };
    static const char* names[4][4] = {
        { "Sword", "Dagger", "Axe", "Spear" }, { "Plate", "Mail", "Vest", "Cuirass" },
        { "Boots", "Greaves", "Sandals", "Treads" }, { "Ring", "Band", "Signet", "Loop" } };
    static const char* prefix[4][4] = {
        { "Rusty", "Worn", "Plain", "Sturdy" }, { "Fine", "Tempered", "Keen", "Reinforced" },
        { "Runed", "Shadow", "Storm", "Arcane" }, { "Void", "Dragon", "Eternal", "Sovereign's" } };
    EquipItem it;
    it.slot = rand() % 4;
    float roll = Rand01();
    it.rarity = roll < 0.03f ? 3 : (roll < 0.13f ? 2 : (roll < 0.40f ? 1 : 0));
    float rm = it.rarity == 3 ? 3.2f : (it.rarity == 2 ? 2.2f : (it.rarity == 1 ? 1.5f : 1.0f));
    float m = rm * (1.0f + (std::max(1, floor) - 1) * 0.12f);
    switch (it.slot) {
        case SLOT_WEAPON:
            it.damage = roundf(6.0f * m);
            it.attackRate = std::min(2.0f, R1(0.2f * m));
            it.crit = std::min(0.2f, roundf(0.02f * m * 100.0f) / 100.0f);
            break;
        case SLOT_ARMOR:
            it.armor = R1(1.2f * m);
            it.maxHp = roundf(10.0f * m);
            break;
        case SLOT_BOOTS:
            it.moveSpeed = std::min(2.5f, R1(0.3f * m));
            it.maxHp = roundf(6.0f * m);
            break;
        default:
            it.lifesteal = std::min(0.15f, roundf(0.02f * m * 100.0f) / 100.0f);
            it.attackRate = std::min(2.0f, R1(0.25f * m));
            break;
    }
    if (it.slot == SLOT_WEAPON) {
        int wi = rand() % 8;
        it.weaponType = wi;
        it.name = std::string(prefix[it.rarity][rand() % 4]) + " " + weaponNames[wi];
    } else {
        int ni = rand() % 4;
        it.name = std::string(prefix[it.rarity][rand() % 4]) + " " + names[it.slot][ni];
    }
    return it;
}


// ── Biến thể tầng, bố cục, rương, cửa hàng ───────────────────────────────
enum FloorMod { MOD_NONE, MOD_SWARM, MOD_ELITE, MOD_SNIPER, MOD_TREASURE };
enum Layout { LAY_SCATTERED, LAY_RING, LAY_BLOCKS, LAY_BASTION, LAY_OPEN };

struct Chest {
    Vector2 pos;
    bool opened = false;
    float anim = 0.0f;
    bool vault = false; // rương trong kho báu: chắc chắn có trang bị tốt
};

struct ShopSlot {
    bool potion = false;
    EquipItem item;
    float heal = 0.0f;
    int price = 0;
    bool sold = false;
};

// Cửa hàng mở sau mỗi 3 tầng (3, 6, 9, 12, 18...), tầng boss (5, 10...) luôn là tầng chiến đấu
static bool IsShopFloor(int f) { return f > 1 && f % 3 == 0 && f % 5 != 0; }

static int ItemPrice(const EquipItem& it) {
    float c = it.Score() * 1.2f + 10.0f;
    if (it.rarity == 1) c *= 1.3f;
    else if (it.rarity == 2) c *= 1.8f;
    else if (it.rarity == 3) c *= 2.5f;
    return std::max(10, (int)roundf(c));
}

// ── Thánh địa: nâng cấp vĩnh viễn mua bằng Linh hồn ──────────────────────
struct MetaUpgrade {
    const char* name;
    const char* desc;
    int maxLevel;
    int baseCost;
    int costStep;
};
static const MetaUpgrade kMeta[8] = {
    { "Vitality", "+10 max HP per level", 10, 30, 25 },
    { "Might", "+2 damage per level", 10, 40, 30 },
    { "Swiftness", "+5 move speed per level", 6, 30, 30 },
    { "Prosperity", "+10 starting gold per level", 8, 20, 20 },
    { "Scholar", "+8% XP gain per level", 8, 40, 35 },
    { "Lucky Charm", "+2% gear drop chance per level", 5, 60, 50 },
    { "Haggler", "-5% shop prices per level", 6, 50, 40 },
    { "Second Wind", "Start every run with a random relic", 1, 400, 0 },
};

// ── Cài đặt (lưu vào settings.cfg cạnh file exe) ─────────────────────────
struct Settings {
    int resW = 1280, resH = 720;
    int windowMode = 0;   // 0: Windowed, 1: Windowed Fullscreen (Borderless), 2: Fullscreen
    bool vsync = true;
    int fpsIndex = 1;     // chỉ số trong kFpsOptions
    bool showFps = false;
    bool vibration = true; // rung màn hình
    bool lighting = true;
#ifdef __ANDROID__
    int particles = 1;    // 0 thấp, 1 vừa, 2 cao (mặc định vừa trên điện thoại)
#else
    int particles = 2;    // 0 thấp, 1 vừa, 2 cao
#endif
    bool vignette = true;
    int volume = 10;      // 0..10
    int difficulty = 1;   // 0 dễ, 1 thường, 2 khó
};
static const int kFpsOptions[5] = { 30, 60, 120, 144, 0 }; // 0 = không giới hạn
static const int kResOptions[5][2] = { { 1280, 720 }, { 1366, 768 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 } };

// ── Game State ─────────────────────────────────────────────────────────────
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

    enum State { PLAYING, LEVEL_UP, GAME_OVER, TITLE, PAUSED, INVENTORY, SETTINGS, SHOP, META, CLASS_SELECT, SAVE_LOAD };
    State state = PLAYING;
    int selectedClass = 0;
    int classSel = 0;
    bool quitRequested = false;
    bool showHelp = false;
    State prevState = PLAYING;

    struct SaveSlotInfo {
        bool exists = false;
        int heroClass = 0;
        int floor = 1;
        int level = 1;
        int hp = 120, maxHp = 120;
        int gold = 0;
        int enemiesSlain = 0;
        int weaponType = 0;
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
        bool attack = false, dash = false, nova = false, skill2 = false, ult = false;
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
    std::string bannerText = "";
    float bannerTimer = 0.0f;
    bool showMiniMap = true;

    // Hiệu ứng hình ảnh & sự kiện
    std::vector<Particle> particles;
    std::vector<Mote> motes;
    float damagePulse = 0.0f;   // viền màn hình nhấp đỏ khi bị đau
    float hitStop = 0.0f;       // khựng khung hình ngắn khi chí mạng
    int bestCombo = 0;
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

    float SkillCdrMult() const {
        float mult = std::max(0.45f, 1.0f - 0.12f * player.cdrUpgrades);
        if (player.heroClass == CLASS_MAGE) mult *= 0.85f;
        return mult;
    }
    float Skill1MaxCd() const { return kClasses[std::max(0, std::min(3, player.heroClass))].qCooldown * SkillCdrMult(); }
    float Skill2MaxCd() const { return kClasses[std::max(0, std::min(3, player.heroClass))].eCooldown * SkillCdrMult(); }
    float EffectiveArmor() const { return player.armor + (player.holyBuffTimer > 0.0f ? 4.0f : 0.0f); }
    float EffectiveAttackRate() const {
        float r = player.attackRate;
        if (player.holyBuffTimer > 0.0f) r *= 1.25f;
        if (player.heroClass == CLASS_RANGER && player.hasteTimer > 0.0f) r *= 1.35f;
        return r;
    }
    float EffectiveMoveSpeed() const {
        float s = player.speed;
        if (player.heroClass == CLASS_ASSASSIN) {
            if (player.ultActiveTimer > 0.0f) s *= 1.35f;
            else if (player.hasteTimer > 0.0f) s *= 1.25f;
        }
        return s;
    }
    void AddRage(float amt) {
        if (amt <= 0.0f || player.rage >= player.maxRage) return;
        float prev = player.rage;
        player.rage = std::min(player.maxRage, player.rage + amt);
        if (prev < player.maxRage && player.rage >= player.maxRage) {
            PlaySound(snd.sndShield);
            SpawnPopup({ player.pos.x, player.pos.y - 36.0f }, "RAGE FULL [R]!", GOLD);
            Burst(player.pos, { 255, 190, 50, 255 }, 16, 200.0f, 0.5f);
        }
    }

    void Init() {
        snd.Init();
        camera.zoom = 1.0f;
        camera.offset = { 1280 / 2.0f, 720 / 2.0f };
        InitMotes();
#ifdef __ANDROID__
        touchMode = true;
#endif
        lightmap = LoadRenderTexture(1280, 720);
        screenRT = LoadRenderTexture(1280, 720);
        SetTextureFilter(screenRT.texture, TEXTURE_FILTER_BILINEAR);
        LoadSettings();
        RefreshSaveSlots();
        BuildResList();
        ApplySettings();
        RestartRun();
        state = TITLE;
    }

    void RestartRun() {
        player = Player();
        int hc = std::max(0, std::min(3, selectedClass));
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
        particles.clear();
        groundEffects.clear();
        for (int i = 0; i < 4; ++i) hasEquip[i] = false;
        int dummySold = 0;
        TryEquipItem(MakeStarterWeapon(cd.startWeapon), dummySold);
        itemDrops.clear();
        pendingSpawns.clear();
        phoenixUsed = false;
        newRecord = false;
        state = PLAYING;
        BuildFloor();
        if (meta[7] > 0) GiveRandomRelic(); // Second Wind
    }

    // ── Bioma: đổi màu phòng mỗi 3 tầng ──
    struct BiomeColors { Color bg, tileA, tileB, wall, mote; };
    BiomeColors Biome() {
        switch (((floor - 1) / 3) % 4) {
            case 1: return { { 16, 26, 20, 255 }, { 36, 54, 44, 255 }, { 30, 46, 38, 255 }, { 72, 105, 78, 255 }, { 170, 235, 150, 255 } };  // Overgrown Ruins
            case 2: return { { 30, 16, 14, 255 }, { 62, 40, 38, 255 }, { 52, 33, 32, 255 }, { 130, 72, 52, 255 }, { 255, 160, 70, 255 } };   // Molten Depths
            case 3: return { { 14, 24, 34, 255 }, { 42, 62, 78, 255 }, { 35, 54, 70, 255 }, { 120, 165, 195, 255 }, { 235, 245, 255, 255 } }; // Frozen Halls
            default: return { { 20, 24, 32, 255 }, { 38, 44, 58, 255 }, { 32, 38, 50, 255 }, { 60, 70, 90, 255 }, { 190, 205, 235, 255 } };  // Ancient Crypt
        }
    }

    // ── Hiệu ứng hạt ──
    void Burst(Vector2 pos, Color c, int count, float speed, float life) {
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

    void InitMotes() {
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

    void UpdateEffects(float dt) {
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

    Color DeathColor(const Enemy& e) {
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

    // ── Nâng cấp ──
    std::string UpgradeTitle(UpgradeType u) {
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

    std::string UpgradeDesc(UpgradeType u) {
        switch (u) {
            case UP_HP: return "+25 Max Health\n& Instant Recovery";
            case UP_DMG: return "+8 Melee Damage\nFor all attacks";
            case UP_SPEED: return "+15% Move Speed\nOutrun dangers";
            case UP_ATTACK_RATE: return "+30% Attack Speed\nRapid strikes";
            case UP_DASH: return "-0.2s Dash Cooldown\nGreater mobility";
            case UP_LIFESTEAL: return "+8% Lifesteal\nHeal on hit";
            case UP_ARMOR: return "+2 Flat Armor\nReduce damage";
            case UP_CRIT: return "+8% Crit Chance\nCrits deal x2 dmg";
            case UP_NOVA: return "-12% Skill Cooldowns\nCast Q & E faster";
            case UP_IGNITE: return "Hits burn enemies 3s\n+50% dmg/s per level";
            case UP_FROST: return "Hits slow enemies 2s\n-15% speed per level";
        }
        return "";
    }

    void ShowBanner(const std::string& txt, float dur = 2.5f) {
        bannerText = txt;
        bannerTimer = dur;
    }

    void Shake(float dur, float mag) {
        if (!settings.vibration) return;
        screenShakeTimer = dur;
        screenShakeMag = mag;
        float k = std::min(1.0f, mag / 9.0f); // mọi cú rung màn hình cũng làm rung tay cầm
        Rumble(k * 0.9f, k * 0.55f, std::min(0.45f, dur + 0.08f));
    }

    void BuildFloor() {
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
        player.ultActiveTimer = 0.0f;
        BuildDecor();

        portalOpen = false;
        portalPos = { 0, -roomH / 2.0f + 70.0f };
        player.pos = { 0, roomH / 2.0f - 90.0f };

        bool isBoss = (floor % 5 == 0);
        shopFloor = IsShopFloor(floor);

        // Biến thể tầng (từ tầng 2, không áp dụng cho boss / cửa hàng)
        floorMod = MOD_NONE;
        if (!isBoss && !shopFloor && floor >= 2) {
            float r = Rand01();
            if (r < 0.12f) floorMod = MOD_TREASURE;
            else if (r < 0.32f) floorMod = MOD_SWARM;
            else if (floor >= 3 && r < 0.50f) floorMod = MOD_ELITE;
            else if (floor >= 3 && r < 0.68f) floorMod = MOD_SNIPER;
        }
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
        }

        // Rương: mỗi tầng 1 cái (2 cái ở tầng chia hết cho 3 hoặc kho báu), kho báu thêm rương trung tâm
        chests.clear();
        {
            float cx = roomW / 2.0f - 80.0f, cy = roomH / 2.0f - 80.0f;
            int chestCount = (floor % 3 == 0 || floorMod == MOD_TREASURE) ? 2 : 1;
            Chest c1; c1.pos = { -cx, -cy };
            chests.push_back(c1);
            if (chestCount >= 2) { Chest c2; c2.pos = { cx, -cy }; chests.push_back(c2); }
            if (floorMod == MOD_TREASURE) { Chest v; v.pos = { 0, 40 }; v.vault = true; chests.push_back(v); }
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
    }

    void SpawnWarden(Vector2 pos) {
        SpawnEnemy(BRUTE, pos);
        Enemy& w = enemies.back();
        w.isWarden = true;
        w.isElite = false;
        w.maxHp = 150.0f * (HpMult());
        w.hp = w.maxHp;
        w.speed = 85.0f;
        w.contactDmg = 16.0f * (DmgMult());
        w.radius = 36.0f;
    }

    Vector2 GetRandomSpawnPos() {
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

    void SpawnEnemy(EnemyType type, Vector2 pos) { enemies.push_back(BuildEnemy(type, pos)); }

    Enemy BuildEnemy(EnemyType type, Vector2 pos) {
        Enemy e;
        e.type = type;
        e.pos = pos;
        e.vel = { 0, 0 };
        float multHp = HpMult();
        float multDmg = DmgMult();

        if (type == CHASER) {
            e.maxHp = 35.0f * multHp;
            e.speed = 175.0f;
            e.contactDmg = 12.0f * multDmg;
            e.radius = 18.0f;
        } else if (type == RANGER) {
            e.maxHp = 28.0f * multHp;
            e.speed = 150.0f;
            e.contactDmg = 8.0f * multDmg;
            e.radius = 18.0f;
        } else if (type == BRUTE) {
            e.maxHp = 85.0f * multHp;
            e.speed = 95.0f;
            e.contactDmg = 16.0f * multDmg;
            e.radius = 26.0f;
        } else if (type == CHARGER) {
            e.maxHp = 48.0f * multHp;
            e.speed = 120.0f;
            e.contactDmg = 12.0f * multDmg;
            e.radius = 21.0f;
            e.chargeCooldown = 0.8f + 1.4f * Rand01();
        } else if (type == SPLITTER) {
            e.maxHp = 60.0f * multHp;
            e.speed = 125.0f;
            e.contactDmg = 14.0f * multDmg;
            e.radius = 26.0f;
        } else if (type == SUMMONER) {
            e.maxHp = 42.0f * multHp;
            e.speed = 105.0f;
            e.contactDmg = 8.0f * multDmg;
            e.radius = 20.0f;
            e.summonTimer = 2.5f + 2.0f * Rand01();
        } else if (type == BOMBER) {
            e.maxHp = 22.0f * multHp;
            e.speed = 190.0f;
            e.contactDmg = 6.0f * multDmg;
            e.radius = 17.0f;
        } else if (type == BOSS) {
            e.bossKind = ((floor / 5) % 2 == 0) ? 1 : 0; // tầng 5, 15, 25: Void Sovereign; tầng 10, 20, 30: Molten Colossus
            if (e.bossKind == 1) {
                e.maxHp = 360.0f * multHp;
                e.speed = 95.0f;
                e.contactDmg = 24.0f * multDmg;
                e.radius = 48.0f;
                e.summonTimer = 0.0f;
            } else {
                e.maxHp = 260.0f * multHp;
                e.speed = 115.0f;
                e.contactDmg = 20.0f * multDmg;
                e.radius = 42.0f;
            }
        }
        e.hp = e.maxHp;

        // Elite variant chance on floor 3+
        if (type != BOSS && floor >= 3 && ((float)rand() / RAND_MAX < eliteChance)) {
            e.isElite = true;
            e.maxHp *= 1.8f;
            e.hp = e.maxHp;
            e.contactDmg *= 1.3f;
            e.radius *= 1.2f;
        }

        return e;
    }

    void SpawnPopup(Vector2 pos, const std::string& txt, Color col) {
        DamagePopup p;
        p.pos = pos;
        p.text = txt;
        p.color = col;
        popups.push_back(p);
    }

    bool HasRelic(RelicType t) {
        for (const auto& r : player.relics) {
            if (r.type == t) return true;
        }
        return false;
    }

    static Relic MakeRelic(RelicType t) {
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

    void AddRelic(RelicType t) {
        if (HasRelic(t)) return;
        Relic r = MakeRelic(t);
        if (t == RELIC_AEGIS) player.shieldActive = true;
        player.relics.push_back(r);
        PlaySound(snd.sndLevelUp);
        ShowBanner("RELIC: " + r.name + " - " + RelicDesc(t), 3.5f);
    }

    void Update(float dt) {
        if (bannerTimer > 0) bannerTimer -= dt;
        if (screenShakeTimer > 0) screenShakeTimer -= dt;
        PollGamepad();
        PollTouchControls();
        if (rumbleTimer > 0) {
            rumbleTimer -= dt;
            if (rumbleTimer <= 0) RumbleRaw(0.0f, 0.0f);
        }

        // Hit-stop: khựng khung hình rất ngắn khi chí mạng
        if (hitStop > 0) { hitStop -= dt; return; }

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
        if (state == INVENTORY) {
            { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
            if (IsKeyPressed(KEY_I) || EscPressed() || IsKeyPressed(KEY_ENTER) || PadPressed(GAMEPAD_BUTTON_MIDDLE_LEFT) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) state = prevState;
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

        // Toggle radar with M
        if (IsKeyPressed(KEY_M)) showMiniMap = !showMiniMap;

        // Player Timers
        if (player.dashTimer > 0) player.dashTimer -= dt;
        if (player.specialTimer > 0) player.specialTimer -= dt;
        if (player.skill2Timer > 0) player.skill2Timer -= dt;
        if (player.attackTimer > 0) player.attackTimer -= dt;
        if (player.invulnTimer > 0) player.invulnTimer -= dt;
        if (player.holyBuffTimer > 0) player.holyBuffTimer -= dt;
        if (player.hasteTimer > 0) player.hasteTimer -= dt;
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

        // Actions: Dash, Basic Attack, Skill 1 (Q), Skill 2 (E), Ultimate (R)
        bool dashPressed = ctl.active ? ctl.dash : (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_K));
        bool attackHeld = ctl.active ? ctl.attack : (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_J));
        bool skill1Pressed = ctl.active ? ctl.nova : (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_U));
        bool skill2Pressed = ctl.active ? ctl.skill2 : (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_L));
        bool ultPressed = ctl.active ? ctl.ult : (IsKeyPressed(KEY_R) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE));

        if (dashPressed && player.dashTimer <= 0) {
            player.isDashing = true;
            player.dashTimer = player.dashCooldown;
            player.dashDir = (Vector2Length(move) > 0.1f) ? move : aimDir;
            PlaySound(snd.sndDash);
            ghosts.push_back({ player.pos, player.facingLeft, 0.22f, 0.22f });
        }

        // Basic Weapon Attack
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

                if (e.attackTimer >= 2.2f) {
                    e.attackTimer = 0;
                    PlaySound(snd.sndShoot);
                    Projectile p;
                    p.pos = Vector2Add(e.pos, Vector2Scale(dir, 20.0f));
                    p.vel = Vector2Scale(dir, 280.0f);
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
                            p.vel = { cosf(a) * 230.0f, sinf(a) * 230.0f };
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
            } else if (e.type == BOSS) {
                if (!e.enraged && e.hp <= e.maxHp * 0.5f) {
                    e.enraged = true;
                    e.speed *= 1.3f;
                    PlaySound(snd.sndSpecial);
                    Shake(0.4f, 8.0f);
                    ShowBanner("THE VOID SOVEREIGN ENRAGES!");
                }

                e.attackTimer += dt;
                float interval = e.enraged ? 2.0f : 3.0f;
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
                        p.vel = Vector2Scale(bDir, 220.0f);
                        p.damage = 16.0f;
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
                DamagePlayer(e.contactDmg * contactMul);
            }
        }

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
                    DamagePlayer(p.damage);
                    hit = true;
                }
                if (!hit) { // cột đá che chắn đạn của quái
                    for (const auto& pl : pillars) {
                        if (Vector2Distance(p.pos, pl) < 27.0f + p.radius) { hit = true; Burst(p.pos, p.color, 3, 100.0f, 0.25f); break; }
                    }
                }
            } else {
                for (size_t ei = 0; ei < enemies.size(); ++ei) {
                    Enemy& e = enemies[ei];
                    if (e.hp <= 0 || (int)ei == p.hitMask) continue;
                    if (Vector2Distance(p.pos, e.pos) < p.radius + e.radius) {
                        float cc = std::min(0.85f, player.critChance + p.critBonus);
                        bool isCrit = p.hasPreCrit ? p.preCrit : (Rand01() < cc);
                        float critMul = (player.heroClass == CLASS_ASSASSIN ? 2.4f : 2.0f);
                        float dmg = p.damage * (isCrit ? critMul : 1.0f);
                        Vector2 kb = Vector2Length(p.vel) > 0.1f ? Vector2Normalize(p.vel) : Vector2{ 1, 0 };
                        DamageEnemy(e, dmg, kb, p.kind == 3 ? 110.0f : 150.0f, isCrit, true);
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
                        if (player.lifesteal > 0.0f && player.hp < player.maxHp) {
                            float hl = std::max(1.0f, dmg * std::min(0.35f, player.lifesteal));
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
                if (l.type == 0) { // XP
                    player.xp += std::max(1, (int)roundf(l.value * xpMul));
                    Burst(l.pos, SKYBLUE, 3, 100.0f, 0.3f);
                    PlaySound(snd.sndCoin);
                    while (player.xp >= player.xpNext) {
                        player.xp -= player.xpNext;
                        player.level++;
                        player.xpNext = (int)(player.xpNext * 1.45f);
                        TriggerLevelUp();
                    }
                } else if (l.type == 1) { // Gold
                    player.gold += HasRelic(RELIC_MIDAS) ? (int)roundf(l.value * 1.5f) : l.value;
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

        // ── Check Room Clearance ──
        bool anyAlive = !pendingSpawns.empty();
        for (const auto& e : enemies) {
            if (e.hp > 0) { anyAlive = true; break; }
        }
        if (!anyAlive && !portalOpen) {
            portalOpen = true;
            PlaySound(snd.sndPortal);
            ShowBanner("ROOM CLEARED! ENTER THE PORTAL!");
        }

        // Teleport to next floor
        if (portalOpen && Vector2Distance(player.pos, portalPos) < 45.0f) {
            floor++;
            PlaySound(snd.sndPortal);
            BuildFloor();
        }

        // Camera Follow with Screen Shake
        camera.target = player.pos;
        if (screenShakeTimer > 0) {
            camera.target.x += ((float)rand() / RAND_MAX * 2.0f - 1.0f) * screenShakeMag;
            camera.target.y += ((float)rand() / RAND_MAX * 2.0f - 1.0f) * screenShakeMag;
        }
    }

    void TriggerProjectileSplash(Vector2 pos, float baseSplashDmg, float radius, Color col, bool applyBurn, int skipIdx) {
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

    void DamageEnemy(Enemy& e, float dmg, Vector2 kb, float kbForce, bool isCrit, bool allowRage = true) {
        if (e.type == CHARGER && e.chargeState == 3) dmg *= 1.5f; // choáng sau cú lao: dễ trúng đòn
        if (e.type == BOSS && e.bossKind == 1 && e.chargeState == 5) dmg *= 1.3f; // Colossus kiệt sức: nhận thêm sát thương
        if (e.stunTimer > 0.0f) dmg *= 1.20f; // Kẻ địch đang choáng nhận thêm 20% sát thương
        if (HasRelic(RELIC_BERSERK) && player.hp < player.maxHp * 0.4f) dmg *= 1.4f;
        e.hp -= dmg;
        e.pos = Vector2Add(e.pos, Vector2Scale(kb, kbForce * 0.05f));
        SpawnPopup(e.pos, (isCrit ? "!" : "") + std::to_string((int)dmg), isCrit ? YELLOW : WHITE);
        e.hitFlash = 0.09f;
        Burst(e.pos, isCrit ? Color{ 255, 217, 51, 255 } : WHITE, isCrit ? 9 : 4, isCrit ? 230.0f : 160.0f, isCrit ? 0.4f : 0.28f);
        if (isCrit) hitStop = 0.035f;
        if (allowRage) AddRage(isCrit ? 4.0f : 2.5f);

        // Nội tại Sát Thủ: đòn chí mạng gây chảy máu/độc 2s
        if (player.heroClass == CLASS_ASSASSIN && isCrit) {
            e.poisonTimer = std::max(e.poisonTimer, 2.0f);
            e.poisonDmg = std::max(e.poisonDmg, dmg * 0.18f);
        }

        // Ember Edge / Frost Touch
        if (player.igniteLevel > 0) {
            e.burnTimer = 3.0f;
            e.burnDps = std::max(e.burnDps, player.baseDmg * 0.5f * player.igniteLevel);
        }
        if (player.frostLevel > 0) {
            e.slowTimer = 2.0f;
            e.slowFactor = std::max(0.4f, 1.0f - 0.15f * player.frostLevel);
        }

        // Relics on hit
        if (HasRelic(RELIC_FROST) && isCrit) {
            e.chillTimer = 2.5f;
            PlaySound(snd.sndFreeze);
        }
        if (HasRelic(RELIC_TOXIC)) {
            e.poisonTimer = 3.0f;
            e.poisonDmg = dmg * 0.25f;
        }
        if (HasRelic(RELIC_LIGHTNING)) {
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

        if (e.hp <= 0) {
            OnEnemyKilled(e);
        }
    }

    void OnEnemyKilled(Enemy& e) {
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
        AddRage(e.type == BOSS ? 35.0f : (e.isElite || e.isWarden ? 15.0f : 6.0f));

        // Nội tại Sát Thủ: hạ gục hoàn 50% hồi chiêu Dash và tăng 25% tốc chạy trong 2s
        if (player.heroClass == CLASS_ASSASSIN) {
            player.dashTimer = std::max(0.0f, player.dashTimer - player.dashCooldown * 0.5f);
            player.hasteTimer = std::max(player.hasteTimer, 2.0f);
        }

        // Combo gold payout every 5 kills
        if (comboCount % 5 == 0) {
            player.gold += comboCount * 2;
            SpawnPopup(player.pos, "+GOLD COMBO!", GOLD);
        }

        // Drops
        if (e.isWarden) {
            loots.push_back({ 2, Vector2Add(e.pos, { 0, 18 }), 35, 20.0f });
            loots.push_back({ 1, Vector2Add(e.pos, { 18, 0 }), 8, 20.0f });
            ShowBanner("WARDEN BRUTE DEFEATED!");
            hitStop = 0.08f;
        }
        if (e.type == BOSS) { hitStop = 0.12f; bossesKilled++; }
        loots.push_back({ 0, e.pos, (e.type == BOSS ? 150 : (e.isWarden ? 90 : (e.isElite ? 35 : (e.isMinion ? 5 : 15)))), 20.0f });
        {
            float dropChance = (e.type == BOSS || e.isWarden) ? 1.0f : (e.isElite ? 0.30f : 0.12f) + 0.02f * meta[5];
            if (!e.isMinion && Rand01() < dropChance) SpawnItemDrop(e.pos);
        }
        if ((float)rand() / RAND_MAX < 0.6f || e.isElite) {
            loots.push_back({ 1, Vector2Add(e.pos, { 10, 0 }), rand() % 4 + 1, 20.0f });
        }
        if ((float)rand() / RAND_MAX < 0.2f || (HasRelic(RELIC_CHALICE) && (float)rand() / RAND_MAX < 0.3f)) {
            loots.push_back({ 2, Vector2Add(e.pos, { -10, 0 }), 35, 20.0f });
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
    }

    void KnightCounterShockwave() {
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

    void DamagePlayer(float dmg) {
        if (player.isInvuln || player.hp <= 0) return;

        // Khiên chặn đòn (Divine Aegis hoặc Thánh Quang Xung của Knight)
        if (player.shieldActive) {
            player.shieldActive = false;
            player.shieldTimer = 0.0f;
            PlaySound(snd.sndShield);
            SpawnPopup(player.pos, "BLOCKED!", GOLD);
            Shake(0.12f, 3.0f);
            if (player.heroClass == CLASS_KNIGHT) KnightCounterShockwave();
            return;
        }

        float effArmor = EffectiveArmor();
        float effDmg = dmg * 100.0f / (100.0f + effArmor * 10.0f);
        if (player.heroClass == CLASS_KNIGHT) effDmg *= 0.85f;
        if (player.heroClass == CLASS_ASSASSIN && player.ultActiveTimer > 0.0f) effDmg *= 0.50f;
        effDmg = std::max(1.0f, effDmg);
        player.hp = std::max(0.0f, player.hp - effDmg);
        comboCount = 0;
        damagePulse = std::min(1.0f, 0.45f + effDmg / 60.0f);
        player.invulnTimer = 0.45f;
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
            state = GAME_OVER;
            PlaySound(snd.sndDeath);
            AwardSouls();
            if (floor > bestFloor) {
                bestFloor = floor;
                SaveSettings();
                newRecord = true;
            }
        }
    }

    void BomberExplode(Enemy& e) {
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

    void UseShrine() {
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

    void TriggerLevelUp() {
        state = LEVEL_UP;
        PlaySound(snd.sndLevelUp);
        Burst(player.pos, { 255, 235, 115, 255 }, 26, 300.0f, 0.8f);
        Burst(player.pos, { 128, 242, 255, 255 }, 14, 180.0f, 0.7f);
        levelChoices = { UP_HP, UP_DMG, UP_ATTACK_RATE };
        std::vector<UpgradeType> all = { UP_HP, UP_DMG, UP_SPEED, UP_ATTACK_RATE, UP_DASH, UP_LIFESTEAL, UP_ARMOR, UP_CRIT, UP_NOVA, UP_IGNITE, UP_FROST };
        std::mt19937 rng((unsigned)rand());
        std::shuffle(all.begin(), all.end(), rng);
        levelChoices = { all[0], all[1], all[2] };
    }

    void ChooseUpgrade(UpgradeType up) {
        ApplyUpgrade(up);
        PlaySound(snd.sndPotion);
        state = PLAYING;
    }

    void ApplyUpgrade(UpgradeType up) {
        if (up == UP_HP) { player.maxHp += 25; player.hp += 25; }
        else if (up == UP_DMG) { player.baseDmg += 8; }
        else if (up == UP_SPEED) { player.speed += 30; }
        else if (up == UP_ATTACK_RATE) { player.attackRate += 0.8f; }
        else if (up == UP_DASH) { player.dashCooldown = std::max(0.4f, player.dashCooldown - 0.2f); }
        else if (up == UP_LIFESTEAL) { player.lifesteal = std::min(0.3f, player.lifesteal + 0.08f); }
        else if (up == UP_ARMOR) { player.armor += 2.0f; }
        else if (up == UP_CRIT) { player.critChance = std::min(0.6f, player.critChance + 0.08f); }
        else if (up == UP_NOVA) {
            player.cdrUpgrades = std::min(5, player.cdrUpgrades + 1);
            player.specialCooldown = Skill1MaxCd();
            player.skill2Cooldown = Skill2MaxCd();
            player.specialTimer = std::min(player.specialTimer, player.specialCooldown);
            player.skill2Timer = std::min(player.skill2Timer, player.skill2Cooldown);
        }
        else if (up == UP_IGNITE) { player.igniteLevel++; }
        else if (up == UP_FROST) { player.frostLevel++; }
    }

    // ── Vẽ nhân vật & quái bằng mô hình đa giác phân mảnh (khớp chi & giáp) ──
    static void Tri(Vector2 a, Vector2 b, Vector2 c, Color col) {
        DrawTriangle(a, b, c, col);
        DrawTriangle(a, c, b, col); // vẽ cả hai chiều để không bị culling
    }

    static Color Shade(Color c, float k) { // k < 1: tối hơn, k > 1: sáng hơn
        return { (unsigned char)std::min(255.0f, c.r * k), (unsigned char)std::min(255.0f, c.g * k),
                 (unsigned char)std::min(255.0f, c.b * k), c.a };
    }

    static Color TintCol(Color col, const Enemy& e) {
        if (e.chillTimer > 0) col = LerpColor(col, SKYBLUE, 0.55f);
        if (e.hitFlash > 0) col = LerpColor(col, WHITE, 0.80f);
        return col;
    }

    static void Quad(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color col) {
        Tri(a, b, c, col);
        Tri(a, c, d, col);
    }

    static void QuadOut(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color fill, Color border, float thick = 1.6f) {
        Quad(a, b, c, d, fill);
        DrawLineEx(a, b, thick, border);
        DrawLineEx(b, c, thick, border);
        DrawLineEx(c, d, thick, border);
        DrawLineEx(d, a, thick, border);
    }

    static void BoxRot(Vector2 center, float w, float h, float deg, Color fill, Color border, float thick = 1.5f) {
        float rad = deg * DEG2RAD;
        float ca = cosf(rad), sa = sinf(rad);
        float hw = w * 0.5f, hh = h * 0.5f;
        Vector2 p0 = { center.x + (-hw * ca - (-hh) * sa), center.y + (-hw * sa + (-hh) * ca) };
        Vector2 p1 = { center.x + ( hw * ca - (-hh) * sa), center.y + ( hw * sa + (-hh) * ca) };
        Vector2 p2 = { center.x + ( hw * ca - ( hh) * sa), center.y + ( hw * sa + ( hh) * ca) };
        Vector2 p3 = { center.x + (-hw * ca - ( hh) * sa), center.y + (-hw * sa + ( hh) * ca) };
        QuadOut(p0, p1, p2, p3, fill, border, thick);
    }

    static void Diamond(Vector2 c, float rx, float ry, Color fill, Color border) {
        Vector2 top = { c.x, c.y - ry };
        Vector2 right = { c.x + rx, c.y };
        Vector2 bot = { c.x, c.y + ry };
        Vector2 left = { c.x - rx, c.y };
        QuadOut(top, right, bot, left, fill, border, 1.4f);
        Tri(top, left, c, Shade(fill, 1.35f));
        Tri(c, right, bot, Shade(fill, 0.70f));
    }

    // Mắt nhìn theo hướng look (vector chuẩn hoá)
    static void Eye(Vector2 c, float rx, float ry, Vector2 look, Color iris) {
        DrawEllipse((int)c.x, (int)c.y, rx + 1.3f, ry + 1.3f, { 18, 18, 26, 255 });
        DrawEllipse((int)c.x, (int)c.y, rx, ry, WHITE);
        float px = c.x + look.x * rx * 0.42f, py = c.y + look.y * ry * 0.42f;
        DrawEllipse((int)px, (int)py, rx * 0.60f, ry * 0.66f, iris);
        DrawEllipse((int)px, (int)py, rx * 0.30f, ry * 0.36f, { 12, 10, 16, 255 });
        DrawCircle((int)(px - rx * 0.2f), (int)(py - ry * 0.25f), std::max(1.0f, rx * 0.18f), WHITE);
    }

    void DrawEnemyBody(const Enemy& e) {
        const float t = (float)GetTime();
        const Vector2 c = e.pos;
        float r = e.radius;
        Vector2 look = Vector2Subtract(player.pos, e.pos);
        look = (Vector2Length(look) > 0.01f) ? Vector2Normalize(look) : Vector2{ 0, 1 };
        const float face = (look.x < -0.10f) ? -1.0f : (look.x > 0.10f ? 1.0f : (e.facingLeft ? -1.0f : 1.0f));
        const Color ink = { 18, 16, 26, 255 };

        Color base = { 90, 210, 90, 255 };
        if (e.type == SPLITTER) base = { 70, 190, 205, 255 };
        else if (e.type == SUMMONER) base = { 96, 60, 168, 255 };
        else if (e.type == RANGER) base = { 228, 72, 88, 255 };
        else if (e.type == BRUTE) base = e.isWarden ? Color{ 214, 156, 62, 255 } : Color{ 146, 92, 206, 255 };
        else if (e.type == BOSS) base = (e.bossKind == 1) ? (e.enraged ? Color{ 205, 62, 30, 255 } : Color{ 165, 78, 44, 255 }) : (e.enraged ? Color{ 150, 40, 70, 255 } : Color{ 88, 46, 142, 255 });
        else if (e.type == BOMBER) base = { 255, 140, 38, 255 };
        else if (e.type == CHARGER) base = { 205, 120, 66, 255 };

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

        // Hào quang elite / boss
        if (e.isElite) {
            float k = 0.5f + 0.5f * sinf(t * 5.0f + c.x);
            DrawRing(c, r + 8.0f, r + 11.5f, t * 45.0f, t * 45.0f + 300.0f, 20, Fade(GOLD, 0.50f + 0.4f * k));
        }
        if (e.type == BOSS) {
            float k = 0.5f + 0.5f * sinf(t * 3.0f);
            Color aura = (e.bossKind == 1) ? (e.enraged ? Color{ 255, 90, 30, 255 } : Color{ 255, 150, 50, 255 }) : (e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 170, 80, 255, 255 });
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
            // Vết nứt dung nham phát sáng + lõi lò ngực
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
            // Đầu Titan góc cạnh + mắt khe dung nham + ngọn lửa hình đa giác
            Vector2 hc = { c.x, c.y - r * 0.56f };
            QuadOut({ hc.x - r * 0.44f, hc.y - r * 0.34f }, { hc.x + r * 0.44f, hc.y - r * 0.34f },
                    { hc.x + r * 0.32f, hc.y + r * 0.32f }, { hc.x - r * 0.32f, hc.y + r * 0.32f }, rockDark, ink, 2.2f);
            Color eyeC = e.enraged ? Color{ 255, 245, 130, 255 } : lava;
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                BoxRot({ hc.x + s2 * r * 0.19f, hc.y - r * 0.04f }, r * 0.22f, r * 0.10f, -s2 * 14.0f, eyeC, ink, 1.0f);
            }
            for (int i = -1; i <= 1; ++i) {
                float fl = 0.7f + 0.3f * sinf(t * 14.0f + i * 2.0f);
                float fx = hc.x + i * r * 0.22f, fy = hc.y - r * 0.34f;
                Tri({ fx - 6.0f, fy }, { fx + 6.0f, fy }, { fx, fy - r * 0.42f * fl - 6.0f }, { 255, 130, 35, 255 });
                Tri({ fx - 3.0f, fy }, { fx + 3.0f, fy }, { fx, fy - r * 0.26f * fl - 3.0f }, { 255, 240, 140, 255 });
            }
        } else if (e.type == BOSS) {
            // THE VOID SOVEREIGN: Hoàng đế Hư Không - cánh kiếm nổi, áo choàng, vương miện, đại kiếm Void
            float hover = sinf(t * 2.5f) * 4.0f;
            Vector2 bc = { c.x, c.y + hover * 0.5f };
            Color darkVoid = Shade(base, 0.42f);
            Color accent = e.enraged ? Color{ 255, 80, 90, 255 } : Color{ 195, 110, 255, 255 };
            // Cánh kiếm obsidian nổi phía sau 2 bên
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                for (int w = 0; w < 2; ++w) {
                    float wy = bc.y - r * 0.28f + w * r * 0.34f + sinf(t * 3.0f + w) * 3.0f;
                    Tri({ bc.x + s2 * r * 0.42f, wy },
                        { bc.x + s2 * r * (1.28f - w * 0.18f), wy - r * 0.48f },
                        { bc.x + s2 * r * (0.95f - w * 0.12f), wy + r * 0.20f }, darkVoid);
                    DrawLineEx({ bc.x + s2 * r * 0.42f, wy }, { bc.x + s2 * r * (1.28f - w * 0.18f), wy - r * 0.48f }, 2.0f, accent);
                }
            }
            // Áo choàng hoàng gia rộng
            QuadOut({ bc.x - r * 0.56f, bc.y - r * 0.30f }, { bc.x + r * 0.56f, bc.y - r * 0.30f },
                    { bc.x + r * 0.88f, bc.y + r * 0.96f }, { bc.x - r * 0.88f, bc.y + r * 0.96f }, darkVoid, ink, 2.2f);
            // Thân giáp Hư Không + lõi pha lê ngực
            QuadOut({ bc.x - r * 0.58f, bc.y - r * 0.38f }, { bc.x + r * 0.58f, bc.y - r * 0.38f },
                    { bc.x + r * 0.40f, bc.y + r * 0.54f }, { bc.x - r * 0.40f, bc.y + r * 0.54f }, base, ink, 2.2f);
            Diamond({ bc.x, bc.y + r * 0.06f }, r * 0.20f, r * 0.28f, accent, GOLD);
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                BoxRot({ bc.x + s2 * r * 0.68f, bc.y - r * 0.28f }, r * 0.44f, r * 0.34f, s2 * 22.0f, Shade(base, 1.25f), GOLD, 1.8f);
            }
            // Đầu giáp sừng + vương miện 3 ngọn + mắt rực
            Vector2 hc = { bc.x, bc.y - r * 0.64f };
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

    static void DrawWeaponModel(Vector2 hand, Vector2 dir, int wt, float t) {
        Vector2 perp = { -dir.y, dir.x };
        const Color ink = { 18, 16, 26, 255 };
        if (wt == WP_BOW) {
            float ang = atan2f(dir.y, dir.x) * RAD2DEG;
            Vector2 bowC = { hand.x + dir.x * 4.0f, hand.y + dir.y * 4.0f };
            DrawRing(bowC, 15.0f, 19.0f, ang - 75.0f, ang + 75.0f, 16, { 165, 112, 58, 255 });
            DrawRing(bowC, 17.5f, 19.5f, ang - 75.0f, ang + 75.0f, 16, { 110, 215, 130, 255 });
            Vector2 topLimb = { bowC.x + cosf((ang - 75.0f) * DEG2RAD) * 16.5f, bowC.y + sinf((ang - 75.0f) * DEG2RAD) * 16.5f };
            Vector2 botLimb = { bowC.x + cosf((ang + 75.0f) * DEG2RAD) * 16.5f, bowC.y + sinf((ang + 75.0f) * DEG2RAD) * 16.5f };
            DrawLineEx(topLimb, botLimb, 1.5f, Fade(WHITE, 0.75f));
            Vector2 arrTail = Vector2Subtract(bowC, Vector2Scale(dir, 5.0f));
            Vector2 arrTip = Vector2Add(bowC, Vector2Scale(dir, 24.0f));
            DrawLineEx(arrTail, arrTip, 2.5f, { 225, 200, 140, 255 });
            Tri({ arrTip.x + dir.x * 8.0f, arrTip.y + dir.y * 8.0f }, { arrTip.x + perp.x * 4.2f, arrTip.y + perp.y * 4.2f }, { arrTip.x - perp.x * 4.2f, arrTip.y - perp.y * 4.2f }, { 120, 245, 160, 255 });
        } else if (wt == WP_STAFF) {
            Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 14.0f));
            Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 28.0f));
            DrawLineEx(tail, tip, 5.4f, { 68, 40, 102, 255 });
            DrawLineEx(tail, tip, 2.8f, { 145, 95, 200, 255 });
            Vector2 pL = Vector2Add(tip, Vector2Add(Vector2Scale(dir, 8.0f), Vector2Scale(perp, 6.5f)));
            Vector2 pR = Vector2Add(tip, Vector2Add(Vector2Scale(dir, 8.0f), Vector2Scale(perp, -6.5f)));
            DrawLineEx(tip, pL, 2.6f, GOLD);
            DrawLineEx(tip, pR, 2.6f, GOLD);
            Vector2 orb = Vector2Add(tip, Vector2Scale(dir, 8.0f));
            float pulse = 0.85f + 0.15f * sinf(t * 7.0f);
            Diamond(orb, 6.5f * pulse, 8.5f * pulse, { 115, 235, 255, 255 }, WHITE);
        } else if (wt == WP_HAMMER) {
            Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 8.0f));
            Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 27.0f));
            DrawLineEx(tail, tip, 5.2f, { 105, 70, 38, 255 });
            DrawLineEx(tail, tip, 2.2f, { 160, 112, 64, 255 });
            float deg = atan2f(dir.y, dir.x) * RAD2DEG;
            BoxRot(tip, 14.0f, 24.0f, deg, { 176, 186, 210, 255 }, ink, 1.8f);
            BoxRot(tip, 14.5f, 7.0f, deg, GOLD, ink, 1.2f);
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
            DrawLineEx(Vector2Add(hand, Vector2Scale(perp, 6.0f)), Vector2Subtract(hand, Vector2Scale(perp, 6.0f)), 3.5f, GOLD);
        } else if (wt == WP_SPEAR) {
            Vector2 tail = Vector2Subtract(hand, Vector2Scale(dir, 14.0f));
            Vector2 tip = Vector2Add(hand, Vector2Scale(dir, 40.0f));
            DrawLineEx(tail, tip, 4.4f, { 126, 86, 52, 255 });
            Tri(tip, { tip.x - dir.x * 9.0f + perp.x * 5.0f, tip.y - dir.y * 9.0f + perp.y * 5.0f }, { tip.x - dir.x * 9.0f - perp.x * 5.0f, tip.y - dir.y * 9.0f - perp.y * 5.0f }, { 215, 52, 68, 255 });
            Diamond(Vector2Add(tip, Vector2Scale(dir, 7.0f)), 4.0f, 4.0f, { 220, 232, 248, 255 }, ink);
            Tri(Vector2Add(tip, Vector2Scale(dir, 18.0f)),
                { tip.x + perp.x * 5.5f, tip.y + perp.y * 5.5f },
                { tip.x - perp.x * 5.5f, tip.y - perp.y * 5.5f }, { 238, 244, 255, 255 });
            DrawLineEx(tip, Vector2Add(tip, Vector2Scale(dir, 18.0f)), 1.6f, WHITE);
        } else {
            // Sword / Dagger: lưỡi đa giác vát nhọn + sống kiếm + chắn tay vàng
            bool isDagger = (wt == WP_DAGGER);
            float wlen = isDagger ? 17.0f : 26.0f;
            float hw = isDagger ? 2.6f : 3.4f;
            Vector2 baseBlade = Vector2Add(hand, Vector2Scale(dir, 3.0f));
            Vector2 shoulder = Vector2Add(hand, Vector2Scale(dir, wlen - 6.0f));
            Vector2 tip = Vector2Add(hand, Vector2Scale(dir, wlen + (isDagger ? 5.0f : 8.0f)));
            QuadOut(Vector2Add(baseBlade, Vector2Scale(perp, hw)), Vector2Add(shoulder, Vector2Scale(perp, hw * 0.82f)),
                    Vector2Subtract(shoulder, Vector2Scale(perp, hw * 0.82f)), Vector2Subtract(baseBlade, Vector2Scale(perp, hw)),
                    isDagger ? Color{ 205, 215, 238, 255 } : Color{ 230, 238, 252, 255 }, ink, 1.4f);
            Tri(Vector2Add(shoulder, Vector2Scale(perp, hw * 0.82f)), Vector2Subtract(shoulder, Vector2Scale(perp, hw * 0.82f)), tip, WHITE);
            DrawLineEx(baseBlade, tip, 1.4f, isDagger ? Color{ 255, 85, 130, 255 } : WHITE);
            DrawLineEx({ hand.x + perp.x * (isDagger ? 6.0f : 8.0f), hand.y + perp.y * (isDagger ? 6.0f : 8.0f) },
                       { hand.x - perp.x * (isDagger ? 6.0f : 8.0f), hand.y - perp.y * (isDagger ? 6.0f : 8.0f) }, 4.2f, GOLD);
            DrawLineEx(Vector2Subtract(hand, Vector2Scale(dir, 6.0f)), hand, 3.6f, { 95, 60, 36, 255 });
        }
        float hdeg = atan2f(dir.y, dir.x) * RAD2DEG;
        BoxRot(hand, 7.5f, 7.5f, hdeg, { 150, 160, 188, 255 }, ink, 1.3f);
    }

    static void DrawHeroModel(Vector2 p, int heroClass, int wt, Vector2 aimDir, bool facingLeft, float walkTimer, bool moving,
                              float atkSweep, bool isInvuln, bool shieldActive, float holyBuffTimer, float ultActiveTimer) {
        const float t = (float)GetTime();
        const float face = facingLeft ? -1.0f : 1.0f;
        const float back = -face;
        const Color ink = { 18, 16, 26, 255 };
        float bob = moving ? fabsf(sinf(walkTimer)) * -2.2f : sinf(t * 3.0f) * 1.0f;
        float sway = sinf(t * 8.0f) * 2.5f + (moving ? 4.5f : 0.0f);
        Vector2 body = { p.x, p.y + bob + 2.0f };
        Vector2 head = { body.x, body.y - 14.5f };
        float step = moving ? sinf(walkTimer) * 5.0f : 0.0f;

        if (holyBuffTimer > 0.0f) {
            float k = 0.65f + 0.35f * sinf(t * 10.0f);
            DrawPolyLinesEx({ body.x, body.y - 4.0f }, 8, 29.0f, t * 35.0f, 2.0f, Fade(GOLD, 0.70f * k));
        }

        if (heroClass == CLASS_KNIGHT) {
            // VANGUARD KNIGHT: áo choàng xanh hoàng gia, giáp ngực thép bạc, cầu vai vàng, khiên tháp & mũ Crusader
            Color cape = { 44, 74, 168, 255 };
            QuadOut({ body.x + back * 5.0f, body.y - 9.0f }, { body.x + back * 6.0f, body.y + 8.0f },
                    { body.x + back * (30.0f + sway), body.y + 18.0f + sway * 0.35f }, { body.x + back * (28.0f + sway), body.y - 4.0f }, cape, ink, 1.5f);
            DrawLineEx({ body.x + back * (28.0f + sway), body.y - 4.0f }, { body.x + back * (30.0f + sway), body.y + 18.0f + sway * 0.35f }, 2.2f, GOLD);
            // Giày giáp sắt
            BoxRot({ p.x - 6.0f, p.y + 15.0f + step * 0.55f }, 7.5f, 8.5f, -step * 2.2f, { 112, 122, 148, 255 }, ink);
            BoxRot({ p.x + 6.0f, p.y + 15.0f - step * 0.55f }, 7.5f, 8.5f,  step * 2.2f, { 112, 122, 148, 255 }, ink);
            // Lông mũ đỏ bay phía sau
            Tri({ head.x, head.y - 9.0f }, { head.x + back * (26.0f + sway * 0.45f), head.y - 16.0f + sway * 0.25f }, { head.x + back * 7.0f, head.y - 2.0f }, { 224, 52, 66, 255 });
            // Giáp ngực hình thang + chữ thập vàng + đai lưng
            QuadOut({ body.x - 13.0f, body.y - 8.0f }, { body.x + 13.0f, body.y - 8.0f },
                    { body.x + 9.5f, body.y + 10.0f }, { body.x - 9.5f, body.y + 10.0f }, { 214, 222, 240, 255 }, ink, 1.8f);
            DrawLineEx({ body.x, body.y - 5.0f }, { body.x, body.y + 7.0f }, 2.6f, GOLD);
            DrawLineEx({ body.x - 5.0f, body.y - 1.0f }, { body.x + 5.0f, body.y - 1.0f }, 2.4f, GOLD);
            BoxRot({ body.x, body.y + 7.5f }, 20.0f, 4.5f, 0.0f, { 92, 62, 42, 255 }, ink, 1.2f);
            BoxRot({ body.x, body.y + 7.5f }, 6.0f, 5.5f, 0.0f, GOLD, ink, 1.0f);
            // Cầu vai thép viền vàng
            BoxRot({ body.x - 13.5f, body.y - 6.0f }, 9.0f, 8.5f, -18.0f, { 188, 198, 224, 255 }, GOLD, 1.4f);
            BoxRot({ body.x + 13.5f, body.y - 6.0f }, 9.0f, 8.5f,  18.0f, { 188, 198, 224, 255 }, GOLD, 1.4f);
            // Khiên phụ bên tay còn lại
            Vector2 sh = { body.x + back * 14.5f, body.y + 1.5f };
            QuadOut({ sh.x - 5.5f, sh.y - 8.0f }, { sh.x + 5.5f, sh.y - 8.0f }, { sh.x + 4.5f, sh.y + 4.0f }, { sh.x - 4.5f, sh.y + 4.0f }, { 44, 78, 172, 255 }, { 205, 214, 236, 255 }, 1.8f);
            Tri({ sh.x - 4.5f, sh.y + 4.0f }, { sh.x + 4.5f, sh.y + 4.0f }, { sh.x, sh.y + 10.5f }, { 205, 214, 236, 255 });
            DrawLineEx({ sh.x, sh.y - 5.0f }, { sh.x, sh.y + 6.0f }, 2.0f, GOLD);
            // Mũ giáp Crusader tách khối rõ ràng
            QuadOut({ head.x - 10.0f, head.y - 8.5f }, { head.x + 10.0f, head.y - 8.5f },
                    { head.x + 9.0f, head.y + 6.5f }, { head.x - 9.0f, head.y + 6.5f }, { 222, 228, 244, 255 }, ink, 1.7f);
            Tri({ head.x - 10.0f, head.y - 8.5f }, { head.x + 10.0f, head.y - 8.5f }, { head.x, head.y - 13.5f }, { 190, 202, 226, 255 });
            BoxRot({ head.x + face * 3.0f, head.y + 0.5f }, 12.0f, 4.2f, 0.0f, { 18, 24, 48, 255 }, ink, 1.0f);
            DrawRectangle((int)(head.x + face * 5.0f - 4), (int)(head.y - 0.5f), 8, 2, { 90, 230, 255, 255 });
        } else if (heroClass == CLASS_ASSASSIN) {
            // SHADOW ASSASSIN: khăn đỏ bay đôi, giáp da sát thủ, dao găm phụ, đầu mũ ninja & mặt nạ
            Color scarf = { 212, 42, 74, 255 };
            Tri({ body.x + back * 3.0f, body.y - 9.0f }, { body.x + back * (32.0f + sway * 1.15f), body.y - 14.0f + sway * 0.45f }, { body.x + back * (25.0f + sway), body.y + 1.0f }, scarf);
            Tri({ body.x + back * 3.0f, body.y - 4.0f }, { body.x + back * (28.0f + sway), body.y + 4.0f }, { body.x + back * (22.0f + sway), body.y + 14.0f + sway * 0.35f }, Shade(scarf, 0.72f));
            // Ủng ninja gọn
            BoxRot({ p.x - 5.5f, p.y + 15.0f + step * 0.55f }, 6.5f, 8.0f, -step * 2.5f, { 34, 30, 44, 255 }, ink);
            BoxRot({ p.x + 5.5f, p.y + 15.0f - step * 0.55f }, 6.5f, 8.0f,  step * 2.5f, { 34, 30, 44, 255 }, ink);
            // Thân giáp da tối + dây chéo + đai đỏ
            QuadOut({ body.x - 11.5f, body.y - 8.5f }, { body.x + 11.5f, body.y - 8.5f },
                    { body.x + 8.5f, body.y + 9.5f }, { body.x - 8.5f, body.y + 9.5f }, { 46, 44, 66, 255 }, ink, 1.7f);
            DrawLineEx({ body.x - 9.0f, body.y - 7.0f }, { body.x + 7.5f, body.y + 7.5f }, 2.2f, { 112, 92, 130, 255 });
            DrawLineEx({ body.x + 9.0f, body.y - 7.0f }, { body.x - 7.5f, body.y + 7.5f }, 2.2f, { 112, 92, 130, 255 });
            BoxRot({ body.x, body.y + 7.5f }, 17.5f, 4.0f, 0.0f, scarf, ink, 1.0f);
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
            // ARCANE MAGE: áo choàng tầng thêu chỉ vàng, sách phép bay tay trái, mũ phù thủy vành rộng chóp lệch
            Color robe = { 82, 66, 178, 255 };
            BoxRot({ p.x - 5.0f, p.y + 16.0f + step * 0.35f }, 6.0f, 6.5f, 0.0f, { 42, 34, 72, 255 }, ink);
            BoxRot({ p.x + 5.0f, p.y + 16.0f - step * 0.35f }, 6.0f, 6.5f, 0.0f, { 42, 34, 72, 255 }, ink);
            // Váy áo choàng xòe tầng dưới + thân trên + dây đai rune vàng
            QuadOut({ body.x - 11.0f, body.y - 7.5f }, { body.x + 11.0f, body.y - 7.5f },
                    { body.x + 16.0f, body.y + 15.0f }, { body.x - 16.0f, body.y + 15.0f }, Shade(robe, 0.78f), ink, 1.7f);
            QuadOut({ body.x - 10.5f, body.y - 8.5f }, { body.x + 10.5f, body.y - 8.5f },
                    { body.x + 8.0f, body.y + 5.5f }, { body.x - 8.0f, body.y + 5.5f }, robe, ink, 1.5f);
            DrawLineEx({ body.x, body.y - 6.0f }, { body.x, body.y + 14.0f }, 3.2f, GOLD);
            Diamond({ body.x, body.y - 2.0f }, 3.6f, 4.6f, { 110, 240, 255, 255 }, ink);
            BoxRot({ body.x - 12.0f, body.y - 5.5f }, 7.5f, 6.5f, -20.0f, Shade(robe, 1.25f), GOLD, 1.2f);
            BoxRot({ body.x + 12.0f, body.y - 5.5f }, 7.5f, 6.5f,  20.0f, Shade(robe, 1.25f), GOLD, 1.2f);
            // Sách phép mở bay ở tay còn lại
            Vector2 bk = { body.x + back * 15.5f, body.y - 1.0f + sinf(t * 4.5f) * 2.0f };
            BoxRot(bk, 10.0f, 11.0f, back * 14.0f, { 95, 42, 130, 255 }, GOLD, 1.2f);
            BoxRot(bk, 7.5f, 8.5f, back * 14.0f, { 238, 232, 210, 255 }, { 140, 120, 90, 255 }, 0.8f);
            // Đầu + khuôn mặt trong bóng râm + mắt sáng cyan
            QuadOut({ head.x - 8.5f, head.y - 5.0f }, { head.x + 8.5f, head.y - 5.0f },
                    { head.x + 7.5f, head.y + 6.5f }, { head.x - 7.5f, head.y + 6.5f }, { 22, 16, 44, 255 }, ink, 1.5f);
            Diamond({ head.x + face * 5.5f, head.y + 1.0f }, 2.4f, 2.0f, { 105, 240, 255, 255 }, ink);
            Diamond({ head.x + face * 0.5f, head.y + 1.0f }, 2.4f, 2.0f, { 105, 240, 255, 255 }, ink);
            // Mũ phù thủy vành đa giác rộng + chóp cong nhọn + đai vàng
            QuadOut({ head.x - 19.0f, head.y - 5.5f }, { head.x + 19.0f, head.y - 5.5f },
                    { head.x + 13.0f, head.y - 2.0f }, { head.x - 13.0f, head.y - 2.0f }, { 64, 48, 148, 255 }, ink, 1.6f);
            QuadOut({ head.x - 10.5f, head.y - 13.5f }, { head.x + 10.5f, head.y - 13.5f },
                    { head.x + 11.5f, head.y - 5.5f }, { head.x - 11.5f, head.y - 5.5f }, { 78, 60, 170, 255 }, ink, 1.5f);
            Tri({ head.x - 10.5f, head.y - 13.5f }, { head.x + 10.5f, head.y - 13.5f }, { head.x + back * 12.0f, head.y - 26.5f }, { 68, 52, 152, 255 });
            DrawLineEx({ head.x - 11.0f, head.y - 6.5f }, { head.x + 11.0f, head.y - 6.5f }, 2.4f, GOLD);
            Diamond({ head.x + face * 2.5f, head.y - 9.5f }, 3.2f, 3.6f, { 110, 240, 255, 255 }, ink);
            for (int i = 0; i < 2; ++i) {
                float a3 = t * 3.2f + i * PI;
                Diamond({ body.x + cosf(a3) * 23.0f, body.y - 4.0f + sinf(a3) * 8.0f }, 3.4f, 4.4f, { 110, 235, 255, 255 }, ink);
            }
        } else {
            // EMERALD RANGER: áo choàng rừng xanh, ống tên lông chim sau lưng, giáp da thợ săn, mũ trùm gắn lông vàng
            Color cloak = { 42, 134, 78, 255 };
            QuadOut({ body.x + back * 4.0f, body.y - 8.5f }, { body.x + back * 6.0f, body.y + 8.0f },
                    { body.x + back * (28.0f + sway), body.y + 16.5f + sway * 0.3f }, { body.x + back * (24.0f + sway), body.y - 3.0f }, cloak, ink, 1.5f);
            // Ống tên đeo chéo sau vai + 2 đuôi lông tên
            DrawLineEx({ body.x + back * 7.0f, body.y - 14.0f }, { body.x + back * 14.0f, body.y + 5.0f }, 6.0f, { 126, 84, 46, 255 });
            Tri({ body.x + back * 5.0f, body.y - 16.0f }, { body.x + back * 9.0f, body.y - 15.0f }, { body.x + back * 6.5f, body.y - 21.0f }, { 255, 220, 110, 255 });
            Tri({ body.x + back * 8.0f, body.y - 15.0f }, { body.x + back * 12.0f, body.y - 14.0f }, { body.x + back * 10.5f, body.y - 20.0f }, { 120, 240, 160, 255 });
            // Ủng săn da nâu
            BoxRot({ p.x - 5.5f, p.y + 15.0f + step * 0.55f }, 7.0f, 8.5f, -step * 2.4f, { 88, 58, 38, 255 }, ink);
            BoxRot({ p.x + 5.5f, p.y + 15.0f - step * 0.55f }, 7.0f, 8.5f,  step * 2.4f, { 88, 58, 38, 255 }, ink);
            // Áo giáp da + đai chéo đựng tên
            QuadOut({ body.x - 12.0f, body.y - 8.0f }, { body.x + 12.0f, body.y - 8.0f },
                    { body.x + 9.0f, body.y + 9.5f }, { body.x - 9.0f, body.y + 9.5f }, { 56, 168, 96, 255 }, ink, 1.7f);
            QuadOut({ body.x - 8.5f, body.y - 6.0f }, { body.x + 8.5f, body.y - 6.0f },
                    { body.x + 7.5f, body.y + 5.5f }, { body.x - 7.5f, body.y + 5.5f }, { 116, 80, 48, 255 }, ink, 1.2f);
            DrawLineEx({ body.x - 9.0f, body.y - 7.0f }, { body.x + 7.0f, body.y + 8.0f }, 2.6f, { 150, 106, 64, 255 });
            Diamond({ body.x, body.y + 7.5f }, 3.2f, 2.8f, GOLD, ink);
            BoxRot({ body.x + back * 12.5f, body.y - 5.5f }, 8.0f, 7.0f, back * 16.0f, { 116, 80, 48, 255 }, ink, 1.3f);
            // Đầu + mũ trùm thợ săn + lông vàng trên mũ + khăn che mặt
            QuadOut({ head.x - 9.5f, head.y - 7.5f }, { head.x + 9.5f, head.y - 7.5f },
                    { head.x + 8.5f, head.y + 6.5f }, { head.x - 8.5f, head.y + 6.5f }, { 48, 148, 84, 255 }, ink, 1.6f);
            Tri({ head.x - 9.5f, head.y - 7.5f }, { head.x + 9.5f, head.y - 7.5f }, { head.x + back * 9.0f, head.y - 16.0f }, { 38, 120, 68, 255 });
            Tri({ head.x + back * 5.0f, head.y - 9.0f }, { head.x + back * 8.0f, head.y - 7.0f }, { head.x + back * 16.0f, head.y - 21.0f }, GOLD);
            BoxRot({ head.x + face * 3.0f, head.y + 0.5f }, 11.0f, 5.0f, 0.0f, { 18, 28, 22, 255 }, ink, 1.0f);
            Diamond({ head.x + face * 6.0f, head.y + 0.5f }, 2.4f, 2.0f, { 255, 225, 95, 255 }, ink);
            Diamond({ head.x + face * 0.5f, head.y + 0.5f }, 2.4f, 2.0f, { 255, 225, 95, 255 }, ink);
        }

        // Vũ khí theo hướng ngắm
        float a = atan2f(aimDir.y, aimDir.x);
        float sa = a + atkSweep;
        Vector2 dir = { cosf(sa), sinf(sa) };
        Vector2 hand = { body.x + dir.x * 16.0f, body.y + dir.y * 16.0f };
        DrawWeaponModel(hand, dir, wt, t);

        if (shieldActive) {
            DrawPolyLinesEx({ body.x, body.y - 4.0f }, 8, 28.0f, t * 20.0f, 2.0f, Fade(GOLD, 0.85f));
        }
        if (isInvuln) {
            float k = 0.5f + 0.5f * sinf(t * 30.0f);
            DrawPolyLinesEx({ body.x, body.y - 4.0f }, 8, 24.5f, -t * 45.0f, 1.8f, Fade(SKYBLUE, 0.4f + 0.5f * k));
        }
    }

    void DrawPlayerSprite() {
        const bool moving = player.walkTimer > 0.01f && (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)
                              || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || (ctl.active && Vector2Length(ctl.move) > 0.1f));
        int wt = hasEquip[SLOT_WEAPON] ? equipped[SLOT_WEAPON].weaponType : kClasses[player.heroClass].startWeapon;
        float effRate = EffectiveAttackRate();
        float sinceAtk = (1.0f / std::max(0.1f, effRate)) - player.attackTimer;
        float off = 0.5f;
        if (wt == WP_BOW || wt == WP_STAFF) off = 0.0f;
        else if (player.attackTimer > 0.0f && sinceAtk < 0.16f) off = Lerp(-1.25f, 1.25f, sinceAtk / 0.16f);
        DrawHeroModel(player.pos, player.heroClass, wt, aimDirDraw, player.facingLeft, player.walkTimer, moving, off, player.isInvuln, player.shieldActive, player.holyBuffTimer, player.ultActiveTimer);
    }

    // Độ khó theo tầng: tăng đều, rồi dốc hơn từ tầng 9 để theo kịp sức mạnh của người chơi (cân bằng bằng bot mô phỏng)
    float DiffHp() const { return settings.difficulty == 0 ? 0.90f : (settings.difficulty == 2 ? 1.20f : 1.0f); }
    float DiffDmg() const { return settings.difficulty == 0 ? 0.75f : (settings.difficulty == 2 ? 1.20f : 1.0f); }
    float DiffSouls() const { return settings.difficulty == 0 ? 0.75f : (settings.difficulty == 2 ? 1.5f : 1.0f); }
    float HpMult() const { return (1.0f + (floor - 1) * 0.15f + std::max(0, floor - 8) * 0.14f) * DiffHp(); }
    float DmgMult() const { return (1.0f + (floor - 1) * 0.10f + std::max(0, floor - 8) * 0.11f) * DiffDmg(); }
    float EliteChance() const { return std::min(0.45f, 0.20f + std::max(0, floor - 8) * 0.015f); }

    // ── Nút bấm trên màn hình (dùng được cho chuột lẫn chạm) ──
    enum UiAction { UI_NONE = 0, UI_START, UI_SAVE_LOAD, UI_SANCT, UI_SETTINGS, UI_HELP, UI_QUIT, UI_RESUME, UI_EQUIP, UI_RESTART, UI_BACK, UI_CLOSE_HELP, UI_CLOSE_INV, UI_CHANGE_CLASS, UI_MAIN_MENU };
    struct UiBtn { Rectangle r; std::string label; int action; Color col; };

    std::vector<UiBtn> BuildButtons() {
        std::vector<UiBtn> b;
        Color green = { 36, 104, 62, 255 }, teal = { 36, 92, 118, 255 }, blue = { 36, 60, 120, 255 }, purple = { 70, 50, 120, 255 }, red = { 120, 40, 50, 255 }, grey = { 44, 50, 72, 255 };
        switch (state) {
            case TITLE:
                if (showHelp) {
                    b.push_back({ { 540, 650, 200, 44 }, "CLOSE", UI_CLOSE_HELP, grey });
                } else {
                    b.push_back({ { 470, 392, 340, 56 }, "NEW GAME", UI_START, green });
                    b.push_back({ { 470, 458, 340, 48 }, "LOAD GAME (4 SLOTS)", UI_SAVE_LOAD, teal });
                    b.push_back({ { 470, 516, 166, 44 }, "SANCTUARY", UI_SANCT, purple });
                    b.push_back({ { 644, 516, 166, 44 }, "SETTINGS", UI_SETTINGS, blue });
                    b.push_back({ { 470, 570, 166, 40 }, "CONTROLS", UI_HELP, grey });
#ifndef __ANDROID__
                    b.push_back({ { 644, 570, 166, 40 }, "QUIT", UI_QUIT, red });
#endif
                }
                break;
            case PAUSED:
                if (showHelp) {
                    b.push_back({ { 540, 650, 200, 44 }, "CLOSE", UI_CLOSE_HELP, grey });
                } else {
                    float y = 212;
                    b.push_back({ { 480, y, 320, 38 }, "RESUME", UI_RESUME, green }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "SAVE / LOAD GAME", UI_SAVE_LOAD, teal }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "EQUIPMENT & SKILLS", UI_EQUIP, blue }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "CHANGE HERO", UI_CHANGE_CLASS, purple }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "SETTINGS", UI_SETTINGS, blue }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "RESTART RUN", UI_RESTART, purple }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "CONTROLS", UI_HELP, grey }); y += 43;
                    b.push_back({ { 480, y, 320, 38 }, "MAIN MENU", UI_MAIN_MENU, red });
                }
                break;
            case GAME_OVER:
                b.push_back({ { 510, 496, 260, 44 }, "PLAY AGAIN", UI_RESTART, red });
                b.push_back({ { 510, 548, 260, 40 }, "CHANGE HERO", UI_CHANGE_CLASS, blue });
                b.push_back({ { 510, 596, 260, 40 }, "SANCTUARY", UI_SANCT, purple });
                b.push_back({ { 510, 644, 260, 40 }, "LOAD SAVE", UI_SAVE_LOAD, teal });
                break;
            case INVENTORY:
                b.push_back({ { 1040, 622, 146, 40 }, "CLOSE", UI_CLOSE_INV, grey });
                break;
            case META:
            case SHOP:
            case SAVE_LOAD:
                b.push_back({ { 30, 28, 150, 44 }, "< BACK", UI_BACK, grey });
                break;
            default:
                break;
        }
        return b;
    }

    int HitTestButtons(Vector2 m) {
        if ((state == TITLE || state == PAUSED) && showHelp) return UI_CLOSE_HELP; // chạm vào đâu cũng đóng bảng hướng dẫn
        for (const auto& bt : BuildButtons()) if (CheckCollisionPointRec(m, bt.r)) return bt.action;
        return UI_NONE;
    }

    int HitButton() {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return UI_NONE;
        return HitTestButtons(GetMousePosition());
    }

    void HandleUiAction(int a) {
        switch (a) {
            case UI_START: showHelp = false; classSel = selectedClass; state = CLASS_SELECT; PlaySound(snd.sndPotion); break;
            case UI_SAVE_LOAD: showHelp = false; OpenSaveLoad(state); break;
            case UI_CHANGE_CLASS: showHelp = false; classSel = selectedClass; state = CLASS_SELECT; PlaySound(snd.sndPotion); break;
            case UI_SANCT: OpenSanctuary(state == GAME_OVER ? GAME_OVER : TITLE); break;
            case UI_SETTINGS: settingsReturn = (state == PAUSED) ? PAUSED : TITLE; setSel = 0; state = SETTINGS; break;
            case UI_HELP: showHelp = true; break;
            case UI_CLOSE_HELP: showHelp = false; break;
            case UI_QUIT: quitRequested = true; break;
            case UI_MAIN_MENU: showHelp = false; state = TITLE; break;
            case UI_RESUME: showHelp = false; state = PLAYING; break;
            case UI_EQUIP: prevState = PAUSED; state = INVENTORY; break;
            case UI_RESTART: showHelp = false; RestartRun(); break;
            case UI_BACK: if (state == META) CloseSanctuary(); else if (state == SHOP) state = PLAYING; else if (state == SAVE_LOAD) state = saveLoadReturn; break;
            case UI_CLOSE_INV: state = prevState; break;
            default: break;
        }
    }

    void DrawButtons() {
        Vector2 m = GetMousePosition();
        for (const auto& bt : BuildButtons()) {
            bool hover = CheckCollisionPointRec(m, bt.r) && !touchMode;
            DrawRectangleRec(bt.r, hover ? Shade(bt.col, 1.35f) : bt.col);
            DrawRectangleLinesEx(bt.r, 2.0f, hover ? WHITE : Shade(bt.col, 2.1f));
            int fs = bt.r.height >= 50 ? 26 : 20;
            DrawText(bt.label.c_str(), (int)(bt.r.x + bt.r.width / 2) - MeasureText(bt.label.c_str(), fs) / 2, (int)(bt.r.y + bt.r.height / 2) - fs / 2, fs, WHITE);
        }
    }

    // ── Điều khiển cảm ứng: cần ảo nổi bên trái, nút chém / lướt / Nova bên phải ──
    void PollTouches() {
        touches.clear();
        if (useFakeTouches) {
            for (size_t i = 0; i < fakeTouches.size(); ++i) touches.push_back({ (int)i, fakeTouches[i] });
        } else {
            int n = GetTouchPointCount();
            for (int i = 0; i < n; ++i) {
                Vector2 p = GetTouchPosition(i);
                touches.push_back({ GetTouchPointId(i), { (p.x - presentDx) / presentScale, (p.y - presentDy) / presentScale } });
            }
        }
        if (!touches.empty()) touchMode = true;
    }

    static constexpr float kAtkR = 78.0f, kDashR = 54.0f, kNovaR = 48.0f, kSmallR = 26.0f;
    Vector2 AtkC() const { return { 1150, 570 }; }
    Vector2 DashC() const { return { 1010, 650 }; }
    Vector2 NovaC() const { return { 1020, 508 }; }
    Vector2 Skill2C() const { return { 1110, 428 }; }
    Vector2 UltC() const { return { 1218, 434 }; }
    Vector2 PauseC() const { return { 1236, 172 }; }
    Vector2 GearC() const { return { 1170, 172 }; }
    Rectangle TradeRect() const { return { 560, 536, 160, 56 }; }

    void PollTouchControls() {
        PollTouches();
        if (!touchMode) return;
        if (state != PLAYING) { joyId = -1; touchDashWas = touchNovaWas = touchSkill2Was = touchUltWas = touchUiWas = false; return; }

        bool atk = false, dashNow = false, novaNow = false, skill2Now = false, ultNow = false, uiNow = false, pauseTap = false, gearTap = false, tradeTap = false;
        bool joyStillDown = false;
        for (const auto& t : touches) if (t.first == joyId) joyStillDown = true;
        if (!joyStillDown) joyId = -1;

        for (const auto& t : touches) {
            Vector2 p = t.second;
            if (CheckCollisionPointCircle(p, AtkC(), kAtkR * 1.15f)) { atk = true; continue; }
            if (CheckCollisionPointCircle(p, DashC(), kDashR * 1.15f)) { dashNow = true; continue; }
            if (CheckCollisionPointCircle(p, NovaC(), kNovaR * 1.15f)) { novaNow = true; continue; }
            if (CheckCollisionPointCircle(p, Skill2C(), kNovaR * 1.15f)) { skill2Now = true; continue; }
            if (CheckCollisionPointCircle(p, UltC(), kNovaR * 1.15f)) { ultNow = true; continue; }
            if (CheckCollisionPointCircle(p, PauseC(), kSmallR * 1.4f)) { uiNow = true; pauseTap = !touchUiWas; continue; }
            if (CheckCollisionPointCircle(p, GearC(), kSmallR * 1.4f)) { uiNow = true; gearTap = !touchUiWas; continue; }
            if (shopFloor && Vector2Distance(player.pos, merchantPos) < 130.0f && CheckCollisionPointRec(p, TradeRect())) { uiNow = true; tradeTap = !touchUiWas; continue; }
            if (t.first == joyId) continue;
            if (joyId == -1 && p.x < 640.0f) { joyId = t.first; joyOrigin = p; }
        }
        touchUiWas = uiNow;

        if (pauseTap) { StopRumble(); state = PAUSED; return; }
        if (gearTap) { prevState = PLAYING; state = INVENTORY; return; }
        if (tradeTap) { state = SHOP; shopSel = 0; PlaySound(snd.sndPotion); return; }

        Vector2 mv = { 0, 0 };
        if (joyId != -1) {
            for (const auto& t : touches) {
                if (t.first != joyId) continue;
                Vector2 d = Vector2Subtract(t.second, joyOrigin);
                float len = Vector2Length(d);
                if (len > 90.0f) { joyOrigin = Vector2Subtract(t.second, Vector2Scale(d, 90.0f / len)); d = Vector2Subtract(t.second, joyOrigin); len = 90.0f; }
                if (len > 12.0f) mv = Vector2Scale(d, 1.0f / 90.0f);
                joyPos = t.second;
            }
        }
        bool dashEdge = dashNow && !touchDashWas, novaEdge = novaNow && !touchNovaWas;
        bool skill2Edge = skill2Now && !touchSkill2Was, ultEdge = ultNow && !touchUltWas;
        touchDashWas = dashNow;
        touchNovaWas = novaNow;
        touchSkill2Was = skill2Now;
        touchUltWas = ultNow;
        if (joyId == -1 && !atk && !dashEdge && !novaEdge && !skill2Edge && !ultEdge) return;

        // tự ngắm vào quái gần nhất
        float best = 420.0f; bool found = false;
        for (const auto& e : enemies) {
            if (e.hp <= 0) continue;
            float d = Vector2Distance(player.pos, e.pos);
            if (d < best) { best = d; padAim = Vector2Normalize(Vector2Subtract(e.pos, player.pos)); found = true; }
        }
        if (!found && Vector2Length(mv) > 0.3f) padAim = Vector2Normalize(mv);
        ctl.active = true;
        if (Vector2Length(mv) > 1.0f) mv = Vector2Normalize(mv);
        ctl.move = mv;
        ctl.aim = padAim;
        ctl.attack = atk;
        ctl.dash = dashEdge;
        ctl.nova = novaEdge;
        ctl.skill2 = skill2Edge;
        ctl.ult = ultEdge;
    }

    void DrawTouchControls() {
        auto ring = [&](Vector2 c, float r, Color col, float frac) {
            if (frac >= 0.999f) DrawRing(c, r - 7.0f, r - 2.0f, 0.0f, 360.0f, 40, Fade(col, 0.85f));
            else DrawRing(c, r - 7.0f, r - 2.0f, -90.0f, -90.0f + 360.0f * std::max(0.0f, frac), 40, Fade(col, 0.85f));
        };
        // cần ảo
        Vector2 jo = (joyId != -1) ? joyOrigin : Vector2{ 150, 590 };
        DrawCircleV(jo, 90.0f, Fade(WHITE, joyId != -1 ? 0.10f : 0.06f));
        DrawCircleLines((int)jo.x, (int)jo.y, 90.0f, Fade(WHITE, 0.35f));
        Vector2 knob = (joyId != -1) ? joyPos : jo;
        if (joyId != -1) { Vector2 dd = Vector2Subtract(knob, jo); if (Vector2Length(dd) > 90.0f) knob = Vector2Add(jo, Vector2Scale(Vector2Normalize(dd), 90.0f)); }
        DrawCircleV(knob, 38.0f, Fade(WHITE, joyId != -1 ? 0.35f : 0.18f));

        // nút chém
        Vector2 a = AtkC();
        DrawCircleV(a, kAtkR, Fade({ 220, 70, 70, 255 }, 0.30f));
        DrawCircleLines((int)a.x, (int)a.y, kAtkR, Fade({ 255, 150, 150, 255 }, 0.7f));
        DrawItemIcon(SLOT_WEAPON, a, 62.0f, { 255, 225, 225, 255 }, hasEquip[SLOT_WEAPON] ? equipped[SLOT_WEAPON].weaponType : 0);
        // lướt
        Vector2 d = DashC();
        float dashFrac = player.dashCooldown > 0 ? 1.0f - std::max(0.0f, player.dashTimer) / player.dashCooldown : 1.0f;
        DrawCircleV(d, kDashR, Fade({ 240, 220, 80, 255 }, 0.25f));
        ring(d, kDashR, { 255, 230, 90, 255 }, dashFrac);
        DrawText("DASH", (int)d.x - MeasureText("DASH", 18) / 2, (int)d.y - 9, 18, Fade(WHITE, 0.95f));
        // Skill 1 (Q)
        Vector2 n = NovaC();
        float qFrac = player.specialCooldown > 0 ? 1.0f - std::max(0.0f, player.specialTimer) / player.specialCooldown : 1.0f;
        DrawCircleV(n, kNovaR, Fade({ 80, 190, 255, 255 }, 0.25f));
        ring(n, kNovaR, { 110, 210, 255, 255 }, qFrac);
        DrawText("SK1", (int)n.x - MeasureText("SK1", 18) / 2, (int)n.y - 9, 18, Fade(WHITE, 0.95f));
        // Skill 2 (E)
        Vector2 s2 = Skill2C();
        float eFrac = player.skill2Cooldown > 0 ? 1.0f - std::max(0.0f, player.skill2Timer) / player.skill2Cooldown : 1.0f;
        DrawCircleV(s2, kNovaR, Fade({ 90, 220, 150, 255 }, 0.25f));
        ring(s2, kNovaR, { 110, 235, 175, 255 }, eFrac);
        DrawText("SK2", (int)s2.x - MeasureText("SK2", 18) / 2, (int)s2.y - 9, 18, Fade(WHITE, 0.95f));
        // Ultimate (R)
        Vector2 u = UltC();
        float rFrac = std::min(1.0f, player.rage / player.maxRage);
        DrawCircleV(u, kNovaR, Fade(rFrac >= 1.0f ? GOLD : ORANGE, rFrac >= 1.0f ? 0.38f : 0.22f));
        ring(u, kNovaR, rFrac >= 1.0f ? GOLD : ORANGE, rFrac);
        DrawText("ULT", (int)u.x - MeasureText("ULT", 18) / 2, (int)u.y - 9, 18, Fade(WHITE, 0.95f));
        // tạm dừng / trang bị
        Vector2 pc = PauseC(), gc = GearC();
        DrawCircleV(pc, kSmallR, Fade(BLACK, 0.45f));
        DrawCircleLines((int)pc.x, (int)pc.y, kSmallR, Fade(WHITE, 0.6f));
        DrawRectangle((int)pc.x - 8, (int)pc.y - 9, 5, 18, Fade(WHITE, 0.9f));
        DrawRectangle((int)pc.x + 3, (int)pc.y - 9, 5, 18, Fade(WHITE, 0.9f));
        DrawCircleV(gc, kSmallR, Fade(BLACK, 0.45f));
        DrawCircleLines((int)gc.x, (int)gc.y, kSmallR, Fade(WHITE, 0.6f));
        DrawItemIcon(SLOT_ARMOR, gc, 26.0f, { 220, 230, 255, 255 });
        // nút thương nhân
        if (shopFloor && Vector2Distance(player.pos, merchantPos) < 130.0f) {
            Rectangle tr = TradeRect();
            DrawRectangleRec(tr, Fade({ 60, 40, 100, 255 }, 0.9f));
            DrawRectangleLinesEx(tr, 3.0f, GOLD);
            DrawText("TRADE", (int)(tr.x + tr.width / 2) - MeasureText("TRADE", 28) / 2, (int)tr.y + 14, 28, GOLD);
        }
    }

    // ── Tay cầm ──
    bool PadPressed(int button) { return IsGamepadAvailable(0) && IsGamepadButtonPressed(0, button); }

    // Rung tay cầm (chỉ khi bật Vibration và có tay cầm)
    void Rumble(float strong, float weak, float duration) {
        if (!settings.vibration || !IsGamepadAvailable(0)) return;
        RumbleRaw(strong, weak);
        rumbleTimer = duration;
    }

    void StopRumble() { rumbleTimer = 0.0f; RumbleRaw(0.0f, 0.0f); }

    // Cần trái di chuyển, cần phải ngắm (không có thì tự ngắm vào quái gần nhất)
    void PollGamepad() {
        if (ctl.external) return;
        ctl.active = false;
        ctl.attack = ctl.dash = ctl.nova = ctl.skill2 = ctl.ult = false;
        ctl.move = { 0, 0 };
        if (!IsGamepadAvailable(0)) return;
        auto dz = [](float v) { return fabsf(v) < 0.2f ? 0.0f : v; };
        Vector2 mv = { dz(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X)), dz(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)) };
        Vector2 stick = { GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X), GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) };
        bool aimStick = Vector2Length(stick) > 0.35f;
        float rt = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER);
        float lt = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER);
        bool atk = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || rt > 0.3f;
        bool ltDown = lt > 0.5f;
        bool dashBtn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || (ltDown && !padLtWas);
        padLtWas = ltDown;
        bool novaBtn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        bool skill2Btn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
        bool ultBtn = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
        if (Vector2Length(mv) < 0.01f && !aimStick && !atk && !dashBtn && !novaBtn && !skill2Btn && !ultBtn) return; // tay cầm đang rảnh: dùng bàn phím/chuột

        if (aimStick) {
            padAim = Vector2Normalize(stick);
        } else {
            // tự ngắm quái gần nhất trong tầm
            float best = 380.0f; bool found = false;
            for (const auto& e : enemies) {
                if (e.hp <= 0) continue;
                float d = Vector2Distance(player.pos, e.pos);
                if (d < best) { best = d; padAim = Vector2Normalize(Vector2Subtract(e.pos, player.pos)); found = true; }
            }
            if (!found && Vector2Length(mv) > 0.3f) padAim = Vector2Normalize(mv);
        }
        ctl.active = true;
        if (Vector2Length(mv) > 1.0f) mv = Vector2Normalize(mv);
        ctl.move = mv;
        ctl.aim = padAim;
        ctl.attack = atk;
        ctl.dash = dashBtn;
        ctl.nova = novaBtn;
        ctl.skill2 = skill2Btn;
        ctl.ult = ultBtn;
    }

    // ── Quái phụ sinh ra khi đang duyệt danh sách: đưa vào hàng đợi, thêm vào cuối khung ──
    void FlushSpawns() {
        for (auto& m : pendingSpawns) enemies.push_back(m);
        pendingSpawns.clear();
    }

    void SpawnMinionChaser(Vector2 pos) {
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

    void SpawnSplitMini(Vector2 pos) {
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

    // Relic chưa có, chọn ngẫu nhiên. Trả về true nếu còn relic để trao.
    bool GiveRandomRelic() {
        std::vector<RelicType> pool;
        for (int i = 0; i < 10; ++i) if (!HasRelic((RelicType)i)) pool.push_back((RelicType)i);
        if (pool.empty()) return false;
        AddRelic(pool[rand() % pool.size()]);
        return true;
    }

    const char* RelicDesc(RelicType t) {
        switch (t) {
            case RELIC_LIGHTNING: return "Hits arc lightning to a nearby enemy";
            case RELIC_TOXIC: return "Hits poison enemies";
            case RELIC_FROST: return "Crits freeze enemies";
            case RELIC_AEGIS: return "Blocks one hit every 12s";
            case RELIC_CHALICE: return "More healing drops";
            case RELIC_COMBUSTION: return "Enemies explode on death";
            case RELIC_PHOENIX: return "Revive once at half HP";
            case RELIC_MIDAS: return "+50% gold, bigger pickup range";
            case RELIC_BERSERK: return "+40% damage below 40% HP";
            case RELIC_THORNS: return "Getting hurt unleashes a shockwave";
        }
        return "";
    }

    // Đòn thorn: nổ ra sóng xung kích khi người chơi bị thương
    void ThornBurst() {
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

    // Đòn đánh cơ bản theo 8 loại vũ khí + nội tại từng hệ nhân vật
    void MeleeAttack(Vector2 aimDir) {
        int wt = hasEquip[SLOT_WEAPON] ? equipped[SLOT_WEAPON].weaponType : kClasses[player.heroClass].startWeapon;
        wt = std::max(0, std::min(7, wt));
        static const float wReach[8]  = { 45.0f, 38.0f, 40.0f, 0.0f, 46.0f, 26.0f, 20.0f, 20.0f };
        static const float wRadius[8] = { 75.0f, 64.0f, 98.0f, 0.0f, 85.0f, 92.0f, 0.0f,  0.0f  };
        static const float wDmg[8]    = { 1.00f, 0.85f, 1.35f, 1.10f, 1.55f, 1.20f, 0.95f, 1.15f };
        static const float wRate[8]   = { 1.00f, 1.30f, 0.75f, 0.95f, 0.65f, 0.85f, 1.15f, 0.90f };
        static const float wKb[8]     = { 1.00f, 0.80f, 1.40f, 1.00f, 1.80f, 0.85f, 1.00f, 1.00f };

        float effRate = EffectiveAttackRate();
        player.attackTimer = 1.0f / std::max(0.2f, effRate * wRate[wt]);
        player.attackCounter++;

        // Nội tại Hiệp Sĩ: đòn đánh thứ 5 hồi 4 HP
        if (player.heroClass == CLASS_KNIGHT && (player.attackCounter % 5 == 0) && player.hp < player.maxHp) {
            player.hp = std::min(player.maxHp, player.hp + 4.0f);
            SpawnPopup({ player.pos.x, player.pos.y - 24.0f }, "+4", GREEN);
            Burst(player.pos, GREEN, 6, 110.0f, 0.35f);
        }

        // Nội tại Xạ Thủ: đòn thứ 3 bắn Tên Gió xuyên thấu (+50% DMG, +10% Crit)
        bool eagleMark = (player.heroClass == CLASS_RANGER && (player.attackCounter % 3 == 0));

        SlashVisual sv;
        sv.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 35.0f));
        sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG;
        sv.kind = wt;
        sv.color = eagleMark ? Color{ 85, 255, 165, 255 } : WHITE;
        slashes.push_back(sv);

        // Vũ khí tầm xa: Cung (6) và Trượng (7)
        if (wt == WP_BOW || wt == WP_STAFF) {
            PlaySound(snd.sndShoot);
            Projectile p;
            p.fromPlayer = true;
            p.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 22.0f));
            float spd = (wt == WP_BOW) ? (eagleMark ? 740.0f : 620.0f) : 480.0f;
            p.vel = Vector2Scale(aimDir, spd);
            p.damage = player.baseDmg * wDmg[wt] * (eagleMark ? 1.50f : 1.0f);
            p.life = 1.8f;
            p.kind = (wt == WP_BOW) ? 1 : 2;
            p.critBonus = eagleMark ? 0.10f : 0.0f;
            if (wt == WP_BOW) {
                bool isCrit = (Rand01() < std::min(0.85f, player.critChance + p.critBonus));
                p.hasPreCrit = true;
                p.preCrit = isCrit;
                p.pierce = eagleMark ? 99 : (isCrit ? 1 : 0);
                p.radius = eagleMark ? 9.0f : 6.5f;
                p.color = eagleMark ? Color{ 85, 255, 165, 255 } : (isCrit ? GOLD : Color{ 225, 245, 145, 255 });
            } else {
                p.pierce = eagleMark ? 99 : 0;
                p.splashRadius = 52.0f;
                p.splashRatio = 0.50f;
                p.radius = 8.5f;
                p.color = eagleMark ? Color{ 85, 255, 165, 255 } : Color{ 110, 215, 255, 255 };
            }
            projectiles.push_back(p);
            Burst(p.pos, p.color, eagleMark ? 8 : 3, 120.0f, 0.25f);
            return;
        }

        PlaySound(snd.sndSlash);
        if (eagleMark) {
            // Xạ thủ dùng vũ khí cận chiến vẫn phóng được luồng gió đòn thứ 3
            Projectile wp;
            wp.fromPlayer = true;
            wp.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 20.0f));
            wp.vel = Vector2Scale(aimDir, 680.0f);
            wp.damage = player.baseDmg * 1.50f;
            wp.life = 1.5f;
            wp.kind = 1;
            wp.pierce = 99;
            wp.critBonus = 0.10f;
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
                float cc = std::min(0.85f, player.critChance + (eagleMark ? 0.10f : 0.0f));
                bool isCrit = (Rand01() < cc);
                float critMul = (player.heroClass == CLASS_ASSASSIN ? 2.4f : 2.0f);
                float dmg = player.baseDmg * wDmg[wt] * (eagleMark ? 1.50f : 1.0f) * (isCrit ? critMul : 1.0f);
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
        if (hitAny) {
            float ls = std::min(0.35f, player.lifesteal + (wt == WP_SCYTHE ? 0.05f : 0.0f));
            if (ls > 0.0f && player.hp < player.maxHp) {
                float heal = std::max(1.0f, maxHitDmg * ls);
                player.hp = std::min(player.maxHp, player.hp + heal);
                SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)heal), GREEN);
            }
            PlaySound(snd.sndHit);
            Shake(wt == WP_HAMMER ? 0.17f : 0.12f, wt == WP_HAMMER ? 5.0f : 3.5f);
        }
    }

    // ── Nội tại Pháp Sư: mỗi lần dùng kỹ năng bắn 3 tia ma thuật dẫn đường + 6 Nộ ──
    void TriggerArcaneResonance(Vector2 aimDir) {
        if (player.heroClass != CLASS_MAGE) return;
        AddRage(6.0f);
        float baseAng = atan2f(aimDir.y, aimDir.x);
        for (int i = -1; i <= 1; ++i) {
            float a = baseAng + i * 0.55f;
            Projectile p;
            p.fromPlayer = true;
            p.kind = 5;
            p.pos = Vector2Add(player.pos, { cosf(a) * 18.0f, sinf(a) * 18.0f });
            p.vel = { cosf(a) * 440.0f, sinf(a) * 440.0f };
            p.damage = player.baseDmg * 0.55f;
            p.life = 2.2f;
            p.radius = 6.5f;
            p.color = { 120, 230, 255, 255 };
            projectiles.push_back(p);
        }
    }

    // ── Kỹ năng 1 [Q] ──
    void CastSkill1(Vector2 aimDir) {
        player.specialCooldown = Skill1MaxCd();
        player.specialTimer = player.specialCooldown;
        float critMul = (player.heroClass == CLASS_ASSASSIN ? 2.4f : 2.0f);

        if (player.heroClass == CLASS_KNIGHT) {
            // Xung Kích Khiên: lướt tới 75px, húc choáng 1.2s
            Vector2 dest = Vector2Add(player.pos, Vector2Scale(aimDir, 75.0f));
            float bw = roomW / 2.0f - 30.0f, bh = roomH / 2.0f - 30.0f;
            dest.x = Clamp(dest.x, -bw, bw);
            dest.y = Clamp(dest.y, -bh, bh);
            ResolvePillars(dest, 15.0f);
            ghosts.push_back({ player.pos, player.facingLeft, 0.25f, 0.25f });
            player.pos = dest;
            player.invulnTimer = std::max(player.invulnTimer, 0.28f);

            PlaySound(snd.sndSpecial);
            Shake(0.24f, 6.5f);
            Vector2 slamCenter = Vector2Add(player.pos, Vector2Scale(aimDir, 38.0f));
            Burst(slamCenter, GOLD, 24, 320.0f, 0.55f);
            SlashVisual sv;
            sv.pos = slamCenter;
            sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG;
            sv.kind = 10;
            sv.timer = 0.22f;
            sv.maxTimer = 0.22f;
            sv.color = GOLD;
            slashes.push_back(sv);

            NovaVisual nv;
            nv.pos = slamCenter;
            nv.maxRadius = 140.0f;
            nv.timer = 0.28f;
            nv.maxTimer = 0.28f;
            nv.color = GOLD;
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(slamCenter, e.pos) <= 140.0f + e.radius * 0.5f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 1.9f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 300.0f, isCrit, true);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.2f);
                }
            }
        } else if (player.heroClass == CLASS_ASSASSIN) {
            // Phi Tiêu Bóng Tối: phóng 5 phi tiêu xuyên 1 mục tiêu, làm chậm & trúng độc
            PlaySound(snd.sndSlash);
            Shake(0.14f, 3.5f);
            float baseAng = atan2f(aimDir.y, aimDir.x);
            for (int i = -2; i <= 2; ++i) {
                float a = baseAng + i * 0.19f;
                Vector2 d = { cosf(a), sinf(a) };
                Projectile p;
                p.fromPlayer = true;
                p.kind = 3;
                p.pos = Vector2Add(player.pos, Vector2Scale(d, 18.0f));
                p.vel = Vector2Scale(d, 560.0f);
                p.damage = player.baseDmg * 1.1f;
                p.life = 1.4f;
                p.radius = 8.0f;
                p.pierce = 1;
                p.applySlow = true;
                p.applyPoison = true;
                p.color = { 235, 70, 140, 255 };
                projectiles.push_back(p);
            }
            Burst(player.pos, { 225, 65, 130, 255 }, 12, 220.0f, 0.35f);
        } else if (player.heroClass == CLASS_MAGE) {
            // Cầu Lửa Hủy Diệt: nổ 135px AoE 2.3x + thiêu đốt
            PlaySound(snd.sndShoot);
            Shake(0.14f, 4.0f);
            Projectile p;
            p.fromPlayer = true;
            p.kind = 4;
            p.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 22.0f));
            p.vel = Vector2Scale(aimDir, 470.0f);
            p.damage = player.baseDmg * 2.3f;
            p.life = 1.6f;
            p.radius = 12.0f;
            p.splashRadius = 135.0f;
            p.splashRatio = 1.0f;
            p.applyBurn = true;
            p.color = { 255, 130, 45, 255 };
            projectiles.push_back(p);
            Burst(p.pos, ORANGE, 10, 160.0f, 0.35f);
            TriggerArcaneResonance(aimDir);
        } else {
            // Liên Xạ Tiễn: bắn quạt 7 mũi tên vàng
            PlaySound(snd.sndShoot);
            Shake(0.14f, 3.5f);
            float baseAng = atan2f(aimDir.y, aimDir.x);
            for (int i = -3; i <= 3; ++i) {
                float a = baseAng + i * 0.14f;
                Vector2 d = { cosf(a), sinf(a) };
                Projectile p;
                p.fromPlayer = true;
                p.kind = 1;
                p.pos = Vector2Add(player.pos, Vector2Scale(d, 20.0f));
                p.vel = Vector2Scale(d, 630.0f);
                p.damage = player.baseDmg * 1.0f;
                p.life = 1.6f;
                p.radius = 7.0f;
                p.color = GOLD;
                projectiles.push_back(p);
            }
            Burst(Vector2Add(player.pos, Vector2Scale(aimDir, 20.0f)), GOLD, 12, 200.0f, 0.35f);
        }
    }

    // ── Kỹ năng 2 [E] ──
    void CastSkill2(Vector2 aimDir, Vector2 moveDir) {
        player.skill2Cooldown = Skill2MaxCd();
        player.skill2Timer = player.skill2Cooldown;
        float critMul = (player.heroClass == CLASS_ASSASSIN ? 2.4f : 2.0f);

        if (player.heroClass == CLASS_KNIGHT) {
            // Thánh Quang Xung: nổ 190px 1.8x + buff 3.5s (+4 Armor, +25% Atk Speed, khiên chặn 1 đòn)
            PlaySound(snd.sndSpecial);
            Shake(0.25f, 6.0f);
            Burst(player.pos, GOLD, 34, 380.0f, 0.6f);
            player.holyBuffTimer = 3.5f;
            player.shieldActive = true;
            SpawnPopup({ player.pos.x, player.pos.y - 34.0f }, "HOLY BARRIER!", GOLD);

            NovaVisual nv;
            nv.pos = player.pos;
            nv.maxRadius = 190.0f;
            nv.timer = 0.35f;
            nv.maxTimer = 0.35f;
            nv.color = { 255, 220, 95, 255 };
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= nv.maxRadius) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 1.8f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 280.0f, isCrit, true);
                }
            }
        } else if (player.heroClass == CLASS_ASSASSIN) {
            // Ám Tập: bom khói tại chỗ cũ, dịch chuyển 210px & chém tất chí mạng 2.2x
            Vector2 origin = player.pos;
            NovaVisual smoke;
            smoke.pos = origin;
            smoke.maxRadius = 130.0f;
            smoke.timer = 0.35f;
            smoke.maxTimer = 0.35f;
            smoke.color = { 130, 100, 175, 255 };
            novas.push_back(smoke);
            Burst(origin, { 120, 90, 160, 255 }, 20, 200.0f, 0.55f);
            for (auto& e : enemies) {
                if (e.hp > 0 && Vector2Distance(origin, e.pos) <= 130.0f) {
                    e.slowTimer = std::max(e.slowTimer, 3.0f);
                    e.slowFactor = std::min(e.slowFactor, 0.55f);
                }
            }

            Vector2 dest = Vector2Add(origin, Vector2Scale(aimDir, 210.0f));
            float bw = roomW / 2.0f - 30.0f, bh = roomH / 2.0f - 30.0f;
            dest.x = Clamp(dest.x, -bw, bw);
            dest.y = Clamp(dest.y, -bh, bh);
            ResolvePillars(dest, 15.0f);
            for (int k = 1; k <= 3; ++k) {
                ghosts.push_back({ Vector2Lerp(origin, dest, k * 0.25f), player.facingLeft, 0.28f, 0.28f });
            }
            player.pos = dest;
            player.invulnTimer = std::max(player.invulnTimer, 0.50f);

            PlaySound(snd.sndSpecial);
            Shake(0.22f, 6.0f);
            Burst(dest, { 255, 70, 140, 255 }, 28, 320.0f, 0.5f);
            SlashVisual sv;
            sv.pos = dest;
            sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG;
            sv.kind = 11;
            sv.timer = 0.22f;
            sv.maxTimer = 0.22f;
            sv.color = { 255, 75, 145, 255 };
            slashes.push_back(sv);

            NovaVisual slashRing;
            slashRing.pos = dest;
            slashRing.maxRadius = 125.0f;
            slashRing.timer = 0.26f;
            slashRing.maxTimer = 0.26f;
            slashRing.color = { 255, 75, 145, 255 };
            novas.push_back(slashRing);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(dest, e.pos) <= 125.0f + e.radius * 0.4f) {
                    float dmg = player.baseDmg * 2.2f * critMul; // Chắc chắn chí mạng!
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, dest));
                    DamageEnemy(e, dmg, kb, 220.0f, true, true);
                }
            }
        } else if (player.heroClass == CLASS_MAGE) {
            // Vòng Băng Giá: nổ băng 185px, đóng băng/choáng 1.3s + làm chậm 3s
            PlaySound(snd.sndFreeze);
            Shake(0.22f, 5.5f);
            Burst(player.pos, { 130, 235, 255, 255 }, 32, 360.0f, 0.6f);
            NovaVisual nv;
            nv.pos = player.pos;
            nv.maxRadius = 185.0f;
            nv.timer = 0.35f;
            nv.maxTimer = 0.35f;
            nv.color = { 115, 230, 255, 255 };
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= nv.maxRadius) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 1.5f * (isCrit ? critMul : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 200.0f, isCrit, true);
                    if (e.hp > 0) {
                        e.stunTimer = std::max(e.stunTimer, 1.3f);
                        e.chillTimer = std::max(e.chillTimer, 3.0f);
                    }
                }
            }
            TriggerArcaneResonance(aimDir);
        } else {
            // Bẫy Nổ & Nhào Lộn: đặt bẫy tại chỗ và lộn ngược 120px
            GroundEffect trap;
            trap.kind = 0;
            trap.pos = player.pos;
            trap.radius = 140.0f;
            trap.maxTimer = 0.75f;
            trap.timer = 0.75f;
            trap.damage = player.baseDmg * 2.4f;
            trap.color = { 90, 240, 145, 255 };
            groundEffects.push_back(trap);

            Vector2 backDir = Vector2Length(moveDir) > 0.1f ? Vector2Negate(moveDir) : Vector2Negate(aimDir);
            if (Vector2Length(aimDir) > 0.1f) backDir = Vector2Negate(aimDir);
            Vector2 dest = Vector2Add(player.pos, Vector2Scale(backDir, 120.0f));
            float bw = roomW / 2.0f - 30.0f, bh = roomH / 2.0f - 30.0f;
            dest.x = Clamp(dest.x, -bw, bw);
            dest.y = Clamp(dest.y, -bh, bh);
            ResolvePillars(dest, 15.0f);
            ghosts.push_back({ player.pos, player.facingLeft, 0.25f, 0.25f });
            player.pos = dest;
            player.invulnTimer = std::max(player.invulnTimer, 0.40f);
            PlaySound(snd.sndDash);
            Burst(trap.pos, { 110, 235, 145, 255 }, 14, 180.0f, 0.4f);
        }
    }

    // ── Kỹ năng Nộ [R] ──
    void CastUltimate(Vector2 aimDir) {
        player.rage = 0.0f;
        PlaySound(snd.sndSpecial);
        PlaySound(snd.sndLevelUp);
        Shake(0.45f, 9.0f);
        hitStop = 0.06f;

        if (player.heroClass == CLASS_KNIGHT) {
            // Phán Quyết Thiên Đường: đại kiếm ánh sáng 250px, 3.5x DMG + choáng 1.5s + thánh vực 4s
            ShowBanner("HEAVEN'S JUDGMENT!", 2.2f);
            Burst(player.pos, GOLD, 48, 440.0f, 0.85f);
            NovaVisual nv;
            nv.pos = player.pos;
            nv.maxRadius = 250.0f;
            nv.timer = 0.45f;
            nv.maxTimer = 0.45f;
            nv.color = GOLD;
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= 250.0f) {
                    bool isCrit = (Rand01() < player.critChance);
                    float dmg = player.baseDmg * 3.5f * (isCrit ? 2.0f : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 260.0f, isCrit, false);
                    if (e.hp > 0) e.stunTimer = std::max(e.stunTimer, 1.5f);
                }
            }
            GroundEffect zone;
            zone.kind = 1;
            zone.pos = player.pos;
            zone.radius = 250.0f;
            zone.maxTimer = 4.0f;
            zone.timer = 4.0f;
            zone.tickTimer = 0.40f;
            zone.damage = player.baseDmg * 0.85f;
            zone.color = GOLD;
            groundEffects.push_back(zone);
        } else if (player.heroClass == CLASS_ASSASSIN) {
            // Vũ Điệu Tử Thần: 3.5s bão kiếm xung quanh người chơi
            ShowBanner("BLADE STORM!", 2.2f);
            player.ultActiveTimer = 3.5f;
            player.ultTickTimer = 0.0f;
            player.invulnTimer = std::max(player.invulnTimer, 0.45f);
            Burst(player.pos, { 255, 65, 130, 255 }, 42, 400.0f, 0.7f);
        } else if (player.heroClass == CLASS_MAGE) {
            // Thiên Thạch Hư Không: hố đen hút quái & mưa 12 thiên thạch
            ShowBanner("VOID CATACLYSM!", 2.2f);
            Vector2 center = Vector2Add(player.pos, Vector2Scale(aimDir, 150.0f));
            center.x = Clamp(center.x, -(roomW / 2.0f - 120.0f), roomW / 2.0f - 120.0f);
            center.y = Clamp(center.y, -(roomH / 2.0f - 100.0f), roomH / 2.0f - 100.0f);
            GroundEffect vortex;
            vortex.kind = 2;
            vortex.pos = center;
            vortex.radius = 240.0f;
            vortex.maxTimer = 3.8f;
            vortex.timer = 3.8f;
            vortex.tickTimer = 0.15f;
            vortex.damage = player.baseDmg * 1.5f;
            vortex.color = { 175, 90, 255, 255 };
            groundEffects.push_back(vortex);
            Burst(center, { 175, 90, 255, 255 }, 38, 360.0f, 0.8f);
            TriggerArcaneResonance(aimDir);
        } else {
            // Mưa Tên Phượng Hoàng: 16 mũi tên lửa rơi + tăng 35% tốc đánh
            ShowBanner("PHOENIX ARROW RAIN!", 2.2f);
            player.hasteTimer = 3.6f;
            Vector2 center = Vector2Add(player.pos, Vector2Scale(aimDir, 130.0f));
            center.x = Clamp(center.x, -(roomW / 2.0f - 120.0f), roomW / 2.0f - 120.0f);
            center.y = Clamp(center.y, -(roomH / 2.0f - 100.0f), roomH / 2.0f - 100.0f);
            GroundEffect rain;
            rain.kind = 3;
            rain.pos = center;
            rain.radius = 250.0f;
            rain.maxTimer = 3.6f;
            rain.timer = 3.6f;
            rain.tickTimer = 0.10f;
            rain.damage = player.baseDmg * 1.15f;
            rain.color = { 255, 160, 55, 255 };
            groundEffects.push_back(rain);
            Burst(player.pos, { 255, 175, 65, 255 }, 32, 340.0f, 0.65f);
        }
    }

    void UpdateActiveSkills(float dt) {
        // Bão kiếm Sát Thủ
        if (player.ultActiveTimer > 0.0f) {
            player.ultActiveTimer -= dt;
            if (player.heroClass == CLASS_ASSASSIN) {
                player.ultTickTimer -= dt;
                if (player.ultTickTimer <= 0.0f) {
                    player.ultTickTimer = 0.20f;
                    PlaySound(snd.sndSlash);
                    SlashVisual sv;
                    sv.pos = player.pos;
                    sv.angle = fmodf((float)GetTime() * 900.0f, 360.0f);
                    sv.kind = 12;
                    sv.timer = 0.16f;
                    sv.maxTimer = 0.16f;
                    sv.color = { 255, 70, 145, 255 };
                    slashes.push_back(sv);
                    for (auto& e : enemies) {
                        if (e.hp <= 0) continue;
                        if (Vector2Distance(player.pos, e.pos) <= 185.0f + e.radius * 0.4f) {
                            bool isCrit = (Rand01() < player.critChance);
                            float dmg = player.baseDmg * 0.85f * (isCrit ? 2.4f : 1.0f);
                            Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                            DamageEnemy(e, dmg, kb, 110.0f, isCrit, false);
                        }
                    }
                }
            }
        }

        // Hiệu ứng mặt đất: Bẫy, Thánh vực, Hố đen, Mưa tên, Thiên thạch rơi
        std::vector<GroundEffect> spawned;
        for (size_t i = 0; i < groundEffects.size();) {
            GroundEffect& ge = groundEffects[i];
            ge.timer -= dt;

            if (ge.kind == 0) { // Bẫy nổ Ranger
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
                    nv.color = { 255, 175, 65, 255 };
                    novas.push_back(nv);
                    Burst(ge.pos, { 255, 175, 65, 255 }, 30, 340.0f, 0.55f);
                    for (auto& e : enemies) {
                        if (e.hp <= 0) continue;
                        if (Vector2Distance(ge.pos, e.pos) <= ge.radius + e.radius * 0.4f) {
                            bool isCrit = (Rand01() < player.critChance);
                            float dmg = ge.damage * (isCrit ? 2.0f : 1.0f);
                            Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                            DamageEnemy(e, dmg, kb, 260.0f, isCrit, true);
                            if (e.hp > 0) {
                                e.slowTimer = std::max(e.slowTimer, 3.0f);
                                e.slowFactor = std::min(e.slowFactor, 0.55f);
                            }
                        }
                    }
                    groundEffects.erase(groundEffects.begin() + i);
                    continue;
                }
            } else if (ge.kind == 1) { // Thánh vực Knight
                ge.tickTimer -= dt;
                if (ge.tickTimer <= 0.0f) {
                    ge.tickTimer = 0.40f;
                    Burst(ge.pos, GOLD, 8, 220.0f, 0.4f);
                    if (player.hp < player.maxHp) {
                        float heal = std::max(1.0f, player.maxHp * 0.016f);
                        player.hp = std::min(player.maxHp, player.hp + heal);
                        SpawnPopup({ player.pos.x, player.pos.y - 26.0f }, "+" + std::to_string((int)roundf(heal)), GREEN);
                    }
                    for (auto& e : enemies) {
                        if (e.hp <= 0) continue;
                        if (Vector2Distance(ge.pos, e.pos) <= ge.radius) {
                            bool isCrit = (Rand01() < player.critChance);
                            float dmg = ge.damage * (isCrit ? 2.0f : 1.0f);
                            Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                            DamageEnemy(e, dmg, kb, 80.0f, isCrit, false);
                        }
                    }
                }
            } else if (ge.kind == 2) { // Hố đen Mage
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
                    ge.tickTimer = 0.30f;
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
                    met.color = { 185, 95, 255, 255 };
                    spawned.push_back(met);
                }
            } else if (ge.kind == 3) { // Mưa Tên Phượng Hoàng Ranger
                ge.tickTimer -= dt;
                if (ge.tickTimer <= 0.0f) {
                    ge.tickTimer = 0.22f;
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
                    arr.radius = 75.0f;
                    arr.maxTimer = 0.25f;
                    arr.timer = 0.25f;
                    arr.damage = ge.damage;
                    arr.color = { 255, 165, 55, 255 };
                    spawned.push_back(arr);
                }
            } else if (ge.kind == 4 || ge.kind == 5) { // Thiên thạch / Mũi tên lửa rơi
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
                            float dmg = ge.damage * (isCrit ? 2.0f : 1.0f);
                            Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, ge.pos));
                            DamageEnemy(e, dmg, kb, 140.0f, isCrit, false);
                            if (!isMet && e.hp > 0) {
                                e.burnTimer = std::max(e.burnTimer, 2.5f);
                                e.burnDps = std::max(e.burnDps, player.baseDmg * (0.4f + 0.5f * player.igniteLevel));
                            }
                        }
                    }
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

    // ── Bố cục phòng & va chạm cột ──
    void BuildPillars() {
        pillars.clear();
        switch (layout) {
            case LAY_SCATTERED:
                pillars.push_back({ -350, -100 });
                pillars.push_back({ 350, -100 });
                pillars.push_back({ -200, 120 });
                pillars.push_back({ 200, 120 });
                if (floor % 5 != 0) pillars.push_back({ 0, 0 });
                break;
            case LAY_RING:
                for (int i = 0; i < 8; ++i) {
                    float a = i * (2.0f * PI / 8.0f) + PI / 8.0f;
                    pillars.push_back({ cosf(a) * 250.0f, -20.0f + sinf(a) * 190.0f });
                }
                break;
            case LAY_BLOCKS:
                for (int i = 0; i < 3; ++i) {
                    Vector2 c = (i == 0) ? Vector2{ -330, -110 } : (i == 1 ? Vector2{ 330, -110 } : Vector2{ 0, 50 });
                    pillars.push_back({ c.x - 30.0f, c.y });
                    pillars.push_back({ c.x + 30.0f, c.y });
                    pillars.push_back({ c.x, c.y + 56.0f });
                }
                break;
            case LAY_BASTION:
                for (int i = -3; i <= 3; ++i) if (i != 0) pillars.push_back({ i * 66.0f, -70.0f });
                pillars.push_back({ -200, 60 });
                pillars.push_back({ 200, 60 });
                break;
            default:
                break;
        }
    }

    // Đẩy một vật tròn ra khỏi các cột. Trả về true nếu có va chạm.
    bool ResolvePillars(Vector2& pos, float radius) {
        bool hit = false;
        for (const auto& pl : pillars) {
            Vector2 d = Vector2Subtract(pos, pl);
            float dist = Vector2Length(d);
            float minD = 27.0f + radius;
            if (dist < minD) {
                if (dist < 0.001f) { d = { 1, 0 }; dist = 1.0f; }
                pos = Vector2Add(pl, Vector2Scale(d, minD / dist));
                hit = true;
            }
        }
        return hit;
    }

    const char* ModLabel() {
        switch (floorMod) {
            case MOD_SWARM: return " - SWARM!";
            case MOD_ELITE: return " - ELITE HUNT!";
            case MOD_SNIPER: return " - SNIPER NEST!";
            case MOD_TREASURE: return " - TREASURE VAULT";
            default: return "";
        }
    }

    // ── Rương ──
    void UpdateChests(float dt) {
        for (auto& c : chests) {
            if (c.opened) { if (c.anim < 1.0f) c.anim = std::min(1.0f, c.anim + dt * 4.0f); continue; }
            if (Vector2Distance(c.pos, player.pos) < 42.0f) {
                c.opened = true;
                PlaySound(snd.sndPortal);
                Burst({ c.pos.x, c.pos.y - 10.0f }, { 255, 217, 64, 255 }, 24, 300.0f, 0.7f);
                Shake(0.1f, 3.0f);
                loots.push_back({ 1, { c.pos.x - 22.0f, c.pos.y + 6.0f }, 15, 20.0f });
                loots.push_back({ 1, { c.pos.x + 22.0f, c.pos.y + 6.0f }, 15, 20.0f });
                loots.push_back({ 2, { c.pos.x, c.pos.y - 28.0f }, 40, 20.0f });
                loots.push_back({ 0, { c.pos.x, c.pos.y + 26.0f }, 50, 20.0f });
                SpawnPopup({ c.pos.x, c.pos.y - 30.0f }, "+GOLD", YELLOW);
                if (c.vault) { // kho báu: chắc chắn có trang bị hiếm trở lên (và một relic nếu còn)
                    GiveRandomRelic();
                    ItemDrop d;
                    d.item = GenerateItem(floor + 1);
                    for (int tries = 0; tries < 30 && d.item.rarity < 1; ++tries) d.item = GenerateItem(floor + 1);
                    d.pos = { c.pos.x, c.pos.y + 50.0f };
                    d.life = 90.0f;
                    d.phase = Rand01() * 6.0f;
                    itemDrops.push_back(d);
                }
            }
        }
    }

    void DrawChest(const Chest& c) {
        float t = (float)GetTime();
        Vector2 p = c.pos;
        float s = c.vault ? 1.25f : 1.0f;
        Color wood = c.vault ? Color{ 110, 56, 140, 255 } : Color{ 150, 96, 52, 255 };
        Color band = c.vault ? Color{ 255, 224, 120, 255 } : Color{ 232, 190, 70, 255 };
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
        } else {
            float lift = c.anim * 16.0f * s;
            DrawRectangle((int)(p.x - 21 * s), (int)(p.y - 28 * s - lift * 0.3f), (int)(42 * s), (int)(10 * s), Shade(wood, 0.8f));
            BeginBlendMode(BLEND_ADDITIVE);
            CircleGrad(p.x, p.y - 4, 34.0f * s * (1.0f - c.anim * 0.5f), Fade({ 255, 230, 120, 255 }, 0.5f * (1.0f - c.anim)), Fade({ 255, 230, 120, 255 }, 0.0f));
            EndBlendMode();
        }
    }

    // ── Cửa hàng ──
    void StockShop() {
        for (int i = 0; i < 4; ++i) {
            shopSlots[i] = ShopSlot();
            shopSlots[i].item = GenerateItem(floor);
            shopSlots[i].price = std::max(1, (int)roundf(ItemPrice(shopSlots[i].item) * (1.0f - 0.05f * meta[6])));
        }
        shopSlots[4] = ShopSlot();
        shopSlots[4].potion = true;
        shopSlots[4].heal = 40.0f + floor * 5.0f;
        shopSlots[4].price = std::max(1, (int)roundf((15 + floor * 3) * (1.0f - 0.05f * meta[6])));
        shopSel = 0;
        shopHinted = false;
    }

    void BuyShopSlot(int i) {
        if (i < 0 || i > 4) return;
        ShopSlot& s = shopSlots[i];
        if (s.sold) return;
        if (s.potion && player.hp >= player.maxHp) { ShowBanner("HP is already full!", 1.5f); return; }
        if (!s.potion && hasEquip[s.item.slot] && s.item.Score() <= equipped[s.item.slot].Score()) {
            ShowBanner("Not an upgrade - you already wear better!", 2.0f);
            return;
        }
        if (player.gold < s.price) { ShowBanner("Not enough Gold!", 1.5f); PlaySound(snd.sndHurt); return; }
        player.gold -= s.price;
        if (s.potion) {
            player.hp = std::min(player.maxHp, player.hp + s.heal);
            ShowBanner("Healed " + std::to_string((int)s.heal) + " HP!", 2.0f);
        } else {
            int soldOld = 0;
            if (TryEquipItem(s.item, soldOld)) ShowBanner("EQUIPPED: " + s.item.name, 2.5f);
            else { player.gold += s.item.SellValue(); ShowBanner("Bought & sold " + s.item.name, 2.0f); }
        }
        PlaySound(snd.sndCoin);
        s.sold = true;
    }

    void UpdateShop() {
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (EscPressed() || IsKeyPressed(KEY_F) || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_UP)) { state = PLAYING; return; }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) shopSel = (shopSel + 1) % 5;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || PadPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) shopSel = (shopSel + 4) % 5;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) BuyShopSlot(shopSel);
        for (int i = 0; i < 5; ++i) if (IsKeyPressed(KEY_ONE + i)) { shopSel = i; BuyShopSlot(i); }
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 5; ++i) {
            Rectangle r = { 116.0f + i * 212.0f, 170.0f, 200.0f, 340.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) shopSel = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { shopSel = i; BuyShopSlot(i); }
            }
        }
    }

    void DrawShopScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
        const char* title = "MERCHANT'S SHOP";
        OutlinedText(title, 1280 / 2 - MeasureText(title, 44) / 2, 60, 44, GOLD, { 90, 50, 10, 255 });
        std::string g = "Gold: " + std::to_string(player.gold);
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
                DrawText("Healing Potion", (int)r.x + 12, (int)r.y + 124, 17, rc);
                DrawText(TextFormat("Heals %d HP", (int)s.heal), (int)r.x + 12, (int)r.y + 156, 16, RAYWHITE);
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
                DrawText("SOLD", (int)r.x + 100 - MeasureText("SOLD", 30) / 2, (int)r.y + 150, 30, { 255, 120, 120, 255 });
            } else {
                bool afford = player.gold >= s.price;
                DrawRectangle((int)r.x + 10, (int)r.y + 296, 180, 32, afford ? Color{ 30, 80, 40, 255 } : Color{ 80, 30, 30, 255 });
                DrawRectangleLines((int)r.x + 10, (int)r.y + 296, 180, 32, afford ? Color{ 120, 235, 140, 255 } : Color{ 235, 120, 120, 255 });
                std::string pr = std::to_string(s.price) + " g";
                DrawText(pr.c_str(), (int)r.x + 100 - MeasureText(pr.c_str(), 20) / 2, (int)r.y + 302, 20, afford ? Color{ 255, 230, 120, 255 } : Color{ 255, 160, 160, 255 });
            }
        }
        const char* foot = "1-5 / click / Enter: buy      A/D: select      Esc or F: leave";
        DrawText(foot, 1280 / 2 - MeasureText(foot, 18) / 2, 550, 18, { 150, 165, 200, 255 });
        const char* foot2 = "Gear you buy is equipped automatically if it beats what you wear.";
        DrawText(foot2, 1280 / 2 - MeasureText(foot2, 16) / 2, 580, 16, { 120, 135, 165, 255 });
    }

    void DrawMerchant() {
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

    // ── Thánh địa ──
    int MetaCost(int i) { return kMeta[i].baseCost + kMeta[i].costStep * meta[i]; }

    void AwardSouls() {
        lastRunSouls = (int)roundf((floor * 10 + enemiesSlain / 2 + bossesKilled * 25 + (floor > bestFloor ? 20 : 0)) * DiffSouls());
        souls += lastRunSouls;
        runs++;
        SaveSettings();
    }

    void BuyMeta(int i) {
        if (i < 0 || i >= 8) return;
        if (meta[i] >= kMeta[i].maxLevel) { metaMsg = "Already at max level"; metaMsgTimer = 2.0f; return; }
        int cost = MetaCost(i);
        if (souls < cost) { metaMsg = "Not enough Souls"; metaMsgTimer = 2.0f; PlaySound(snd.sndHurt); return; }
        souls -= cost;
        meta[i]++;
        SaveSettings();
        PlaySound(snd.sndLevelUp);
        metaMsg = std::string(kMeta[i].name) + " upgraded to level " + std::to_string(meta[i]) + "!";
        metaMsgTimer = 2.5f;
    }

    void OpenSanctuary(State from) {
        metaReturn = from;
        metaSel = 0;
        metaMsg.clear();
        state = META;
    }

    void CloseSanctuary() {
        if (metaReturn == TITLE) { RestartRun(); state = TITLE; } // áp dụng nâng cấp mới cho lượt chơi sắp bắt đầu
        else state = metaReturn;
    }

    void UpdateMeta(float dt) {
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (metaMsgTimer > 0) metaMsgTimer -= dt;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) metaSel = (metaSel + 1) % 8;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) metaSel = (metaSel + 7) % 8;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) BuyMeta(metaSel);
        for (int i = 0; i < 8; ++i) if (IsKeyPressed(KEY_ONE + i)) { metaSel = i; BuyMeta(i); }
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 8; ++i) {
            Rectangle r = { 190.0f, 150.0f + i * 56.0f, 900.0f, 50.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) metaSel = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { metaSel = i; BuyMeta(i); }
            }
        }
        if (EscPressed() || IsKeyPressed(KEY_BACKSPACE)) CloseSanctuary();
    }

    void DrawMetaScreen() {
        float t = (float)GetTime();
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.88f));
        const char* title = "SANCTUARY";
        OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 40, 48, { 190, 150, 255, 255 }, { 40, 20, 80, 255 });
        const char* sub = "Spend Souls on upgrades that last across every run";
        DrawText(sub, 1280 / 2 - MeasureText(sub, 18) / 2, 94, 18, { 160, 150, 200, 255 });

        // Linh hồn
        std::string sv = "Souls: " + std::to_string(souls);
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
            DrawText(kMeta[i].name, (int)r.x + 50, (int)r.y + 6, 20, maxed ? Color{ 255, 215, 90, 255 } : WHITE);
            DrawText(kMeta[i].desc, (int)r.x + 50, (int)r.y + 29, 14, { 170, 180, 210, 255 });
            // Các ô cấp độ
            int pips = kMeta[i].maxLevel;
            for (int k = 0; k < pips; ++k)
                DrawRectangle((int)r.x + 430 + k * 16, (int)r.y + 18, 12, 14, k < meta[i] ? Color{ 190, 150, 255, 255 } : Color{ 48, 44, 80, 255 });
            // Giá
            Rectangle b = { r.x + r.width - 140.0f, r.y + 8.0f, 126.0f, 34.0f };
            if (maxed) {
                DrawRectangleRec(b, { 70, 56, 20, 255 });
                DrawRectangleLinesEx(b, 1.0f, GOLD);
                DrawText("MAX", (int)(b.x + b.width / 2) - MeasureText("MAX", 20) / 2, (int)b.y + 7, 20, GOLD);
            } else {
                DrawRectangleRec(b, afford ? Color{ 30, 80, 60, 255 } : Color{ 70, 30, 40, 255 });
                DrawRectangleLinesEx(b, 1.0f, afford ? Color{ 120, 235, 170, 255 } : Color{ 235, 120, 130, 255 });
                std::string c = std::to_string(MetaCost(i));
                DrawText(c.c_str(), (int)(b.x + b.width / 2) - MeasureText(c.c_str(), 20) / 2, (int)b.y + 7, 20, afford ? Color{ 200, 255, 220, 255 } : Color{ 255, 170, 170, 255 });
            }
        }
        if (metaMsgTimer > 0) DrawText(metaMsg.c_str(), 1280 / 2 - MeasureText(metaMsg.c_str(), 22) / 2, 640, 22, GOLD);
        const char* foot = "1-8 / click / Enter: buy      W/S: select      Esc: back";
        DrawText(foot, 1280 / 2 - MeasureText(foot, 16) / 2, 676, 16, { 150, 160, 200, 255 });
    }

    // ── Cài đặt ──
    void LoadSettings() {
        char* txt = LoadFileText("settings.cfg"); // trên Android raylib tự đọc từ bộ nhớ trong của ứng dụng
        if (!txt) return;
        std::istringstream in(txt);
        UnloadFileText(txt);
        std::string line;
        while (std::getline(in, line)) {
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string k = line.substr(0, eq);
            int v = atoi(line.substr(eq + 1).c_str());
            if (k == "resW") settings.resW = v;
            else if (k == "resH") settings.resH = v;
            else if (k == "windowMode") settings.windowMode = std::max(0, std::min(2, v));
            else if (k == "fullscreen") settings.windowMode = (v == 1 ? 2 : (v == 2 ? 1 : 0));
            else if (k == "vsync") settings.vsync = v != 0;
            else if (k == "fpsIndex") settings.fpsIndex = std::max(0, std::min(4, v));
            else if (k == "showFps") settings.showFps = v != 0;
            else if (k == "vibration") settings.vibration = v != 0;
            else if (k == "lighting") settings.lighting = v != 0;
            else if (k == "particles") settings.particles = std::max(0, std::min(2, v));
            else if (k == "vignette") settings.vignette = v != 0;
            else if (k == "volume") settings.volume = std::max(0, std::min(10, v));
            else if (k == "difficulty") settings.difficulty = std::max(0, std::min(2, v));
            else if (k == "best") bestFloor = std::max(1, v);
            else if (k == "souls") souls = std::max(0, v);
            else if (k == "runs") runs = std::max(0, v);
            else if (k == "heroClass") selectedClass = std::max(0, std::min(3, v));
            else if (k.size() == 5 && k.compare(0, 4, "meta") == 0) {
                int mi = k[4] - '0';
                if (mi >= 0 && mi < 8) meta[mi] = std::max(0, std::min(kMeta[mi].maxLevel, v));
            }
        }
    }

    void SaveSettings() {
        std::ostringstream o;
        o << "resW=" << settings.resW << "\nresH=" << settings.resH << "\nwindowMode=" << settings.windowMode
          << "\nvsync=" << settings.vsync << "\nfpsIndex=" << settings.fpsIndex << "\nshowFps=" << settings.showFps
          << "\nvibration=" << settings.vibration << "\nlighting=" << settings.lighting << "\nparticles=" << settings.particles
          << "\nvignette=" << settings.vignette << "\nvolume=" << settings.volume << "\ndifficulty=" << settings.difficulty
          << "\nbest=" << bestFloor << "\nsouls=" << souls << "\nruns=" << runs << "\nheroClass=" << selectedClass;
        for (int i = 0; i < 8; ++i) o << "\nmeta" << i << "=" << meta[i];
        o << "\n";
        std::string s = o.str();
        SaveFileText("settings.cfg", (char*)s.c_str());
    }

    void BuildResList() {
        resList.clear();
#ifdef __ANDROID__
        resList.push_back(0);
        return;
#endif
        int mw = GetMonitorWidth(GetCurrentMonitor()), mh = GetMonitorHeight(GetCurrentMonitor());
        for (int i = 0; i < 5; ++i)
            if (kResOptions[i][0] <= mw && kResOptions[i][1] <= mh) resList.push_back(i);
        if (resList.empty()) resList.push_back(0);
        // Nếu độ phân giải đã lưu không còn khả dụng thì quay về lựa chọn lớn nhất hiện có
        bool ok = false;
        for (int idx : resList) if (kResOptions[idx][0] == settings.resW && kResOptions[idx][1] == settings.resH) ok = true;
        if (!ok) { settings.resW = kResOptions[resList.back()][0]; settings.resH = kResOptions[resList.back()][1]; }
    }

    void ApplySettings() {
#ifndef __ANDROID__
        if (lastAppliedWinMode != settings.windowMode || lastAppliedW != settings.resW || lastAppliedH != settings.resH) {
            if (settings.windowMode != 2 && IsWindowFullscreen()) ToggleFullscreen();
            if (settings.windowMode != 1 && IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();

            if (settings.windowMode == 1) {
                if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
            } else if (settings.windowMode == 2) {
                int mon = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon));
                if (!IsWindowFullscreen()) ToggleFullscreen();
            } else {
                SetWindowSize(settings.resW, settings.resH);
                if (lastAppliedWinMode == 1 || lastAppliedWinMode == 2) {
                    int mon = GetCurrentMonitor();
                    int mw = GetMonitorWidth(mon), mh = GetMonitorHeight(mon);
                    SetWindowPosition(std::max(0, (mw - settings.resW) / 2), std::max(0, (mh - settings.resH) / 2));
                }
            }
            lastAppliedWinMode = settings.windowMode;
            lastAppliedW = settings.resW;
            lastAppliedH = settings.resH;
        }
        if (settings.vsync) SetWindowState(FLAG_VSYNC_HINT); else ClearWindowState(FLAG_VSYNC_HINT);
#endif
        SetTargetFPS(kFpsOptions[settings.fpsIndex]);
        SetMasterVolume(settings.volume / 10.0f);
    }

    void ChangeSetting(int row, int d) {
#ifdef __ANDROID__
        if (row <= 2) return; // độ phân giải, toàn màn hình, VSync do hệ điều hành quản lý
#endif
        switch (row) {
            case 0: {
                int cur = 0;
                for (size_t i = 0; i < resList.size(); ++i)
                    if (kResOptions[resList[i]][0] == settings.resW && kResOptions[resList[i]][1] == settings.resH) cur = (int)i;
                cur = (cur + d + (int)resList.size()) % (int)resList.size();
                settings.resW = kResOptions[resList[cur]][0];
                settings.resH = kResOptions[resList[cur]][1];
                break;
            }
            case 1: settings.windowMode = (settings.windowMode + d + 3) % 3; break;
            case 2: settings.vsync = !settings.vsync; break;
            case 3: settings.fpsIndex = (settings.fpsIndex + d + 5) % 5; break;
            case 4: settings.showFps = !settings.showFps; break;
            case 5: settings.vibration = !settings.vibration; break;
            case 6: settings.lighting = !settings.lighting; break;
            case 7: settings.particles = (settings.particles + d + 3) % 3; break;
            case 8: settings.vignette = !settings.vignette; break;
            case 9: settings.volume = std::max(0, std::min(10, settings.volume + d)); break;
            case 10: settings.difficulty = (settings.difficulty + d + 3) % 3; break;
        }
    }

    std::string SettingValue(int row) {
#ifdef __ANDROID__
        if (row <= 2) return "Auto";
#endif
        switch (row) {
            case 0: return settings.windowMode == 1 ? "Desktop (Borderless)" : (settings.windowMode == 2 ? "Native (Fullscreen)" : std::to_string(settings.resW) + " x " + std::to_string(settings.resH));
            case 1: return settings.windowMode == 0 ? "Windowed" : (settings.windowMode == 1 ? "Windowed Fullscreen" : "Fullscreen");
            case 2: return settings.vsync ? "ON" : "OFF";
            case 3: return kFpsOptions[settings.fpsIndex] == 0 ? "Unlimited" : std::to_string(kFpsOptions[settings.fpsIndex]);
            case 4: return settings.showFps ? "ON" : "OFF";
            case 5: return settings.vibration ? "ON" : "OFF";
            case 6: return settings.lighting ? "ON" : "OFF";
            case 7: return settings.particles == 0 ? "Low" : (settings.particles == 1 ? "Medium" : "High");
            case 8: return settings.vignette ? "ON" : "OFF";
            case 9: return std::to_string(settings.volume * 10) + "%";
            case 10: return settings.difficulty == 0 ? "Easy" : (settings.difficulty == 1 ? "Normal" : "Hard");
        }
        return "";
    }

    void UpdateSettingsMenu() {
        const int rows = 12; // 11 mục + Back
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) setSel = (setSel + 1) % rows;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) setSel = (setSel + rows - 1) % rows;
        int delta = 0;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) delta = 1;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) delta = -1;

        Vector2 m = GetMousePosition();
        for (int i = 0; i < rows; ++i) {
            Rectangle r = { 330.0f, 138.0f + i * 36.0f, 620.0f, 32.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) setSel = i;
                if (setSel == i) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) delta = 1;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) delta = -1;
                }
            }
        }
        if (EscPressed() || IsKeyPressed(KEY_BACKSPACE)) { state = settingsReturn; return; }
        if (delta != 0) {
            if (setSel == 11) { state = settingsReturn; return; }
            ChangeSetting(setSel, delta);
            SaveSettings();
            ApplySettings();
        }
    }

    void DrawSettingsScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.86f));
        const char* title = "SETTINGS";
        OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 70, 48, GOLD, { 90, 50, 10, 255 });
        static const char* labels[11] = { "Resolution", "Display Mode", "VSync", "FPS Limit", "Show FPS", "Vibration (shake + controller)", "Lighting", "Particles", "Vignette", "Volume", "Difficulty (next floors)" };
        for (int i = 0; i < 12; ++i) {
            Rectangle r = { 330.0f, 138.0f + i * 36.0f, 620.0f, 32.0f };
            bool sel = (setSel == i);
            DrawRectangleRec(r, sel ? Color{ 40, 52, 84, 255 } : Color{ 20, 26, 42, 255 });
            if (sel) DrawRectangleLinesEx(r, 2.0f, GOLD);
            if (i < 11) {
                DrawText(labels[i], (int)r.x + 16, (int)r.y + 6, 20, sel ? WHITE : Color{ 190, 205, 235, 255 });
                std::string v = SettingValue(i);
                Color vc = (v == "ON") ? Color{ 120, 235, 140, 255 } : (v == "OFF" ? Color{ 255, 130, 130, 255 } : Color{ 255, 215, 90, 255 });
                if (i == 1) vc = (settings.windowMode == 1) ? Color{ 110, 235, 195, 255 } : (settings.windowMode == 2 ? Color{ 120, 235, 140, 255 } : SKYBLUE);
                int vw = MeasureText(v.c_str(), 20);
                if (i == 10) vc = settings.difficulty == 0 ? Color{ 120, 235, 140, 255 } : (settings.difficulty == 2 ? Color{ 255, 120, 110, 255 } : Color{ 255, 215, 90, 255 });
                DrawText(v.c_str(), (int)(r.x + r.width) - 40 - vw, (int)r.y + 6, 20, vc);
                if (sel) {
                    DrawText("<", (int)(r.x + r.width) - 40 - vw - 22, (int)r.y + 6, 20, GOLD);
                    DrawText(">", (int)(r.x + r.width) - 28, (int)r.y + 6, 20, GOLD);
                }
                if (i == 9) { // thanh âm lượng
                    for (int b = 0; b < 10; ++b)
                        DrawRectangle((int)r.x + 220 + b * 12, (int)r.y + 9, 9, 14, b < settings.volume ? Color{ 255, 215, 90, 255 } : Color{ 50, 58, 84, 255 });
                }
            } else {
                const char* back = "Back";
                DrawText(back, (int)(r.x + r.width / 2) - MeasureText(back, 22) / 2, (int)r.y + 5, 22, sel ? WHITE : Color{ 190, 205, 235, 255 });
            }
        }
        DrawText("W/S or mouse: select     A/D, Enter or click: change     Esc: back", 1280 / 2 - MeasureText("W/S or mouse: select     A/D, Enter or click: change     Esc: back", 16) / 2, 590, 16, { 150, 165, 200, 255 });
        DrawText("Resolution applies in windowed mode; fullscreen uses your desktop resolution. Settings are saved automatically.", 1280 / 2 - MeasureText("Resolution applies in windowed mode; fullscreen uses your desktop resolution. Settings are saved automatically.", 14) / 2, 616, 14, { 120, 135, 165, 255 });
    }

    // Vẽ khung 1280x720 lên cửa sổ với viền đen giữ tỉ lệ, và ánh xạ chuột về toạ độ ảo
    void Present() {
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        float scale = std::min(sw / 1280.0f, sh / 720.0f);
        float dw = 1280.0f * scale, dh = 720.0f * scale;
        float dx = (sw - dw) * 0.5f, dy = (sh - dh) * 0.5f;
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(screenRT.texture, { 0, 0, 1280.0f, -720.0f }, { dx, dy, dw, dh }, { 0, 0 }, 0.0f, WHITE);
        EndDrawing();
        SetMouseOffset(-(int)dx, -(int)dy);
        SetMouseScale(1.0f / scale, 1.0f / scale);
        presentDx = dx;
        presentDy = dy;
        presentScale = scale;
    }

    // ── Trang bị ──
    void ApplyItem(const EquipItem& it, float sign) {
        player.baseDmg += sign * it.damage;
        player.maxHp += sign * it.maxHp;
        if (sign > 0) player.hp += it.maxHp;
        else player.hp = std::max(1.0f, std::min(player.hp - it.maxHp, player.maxHp));
        player.armor += sign * it.armor;
        player.speed += sign * it.moveSpeed * 50.0f;
        player.attackRate += sign * it.attackRate;
        player.lifesteal += sign * it.lifesteal;
        player.critChance += sign * it.crit;
    }

    // Mặc nếu mạnh hơn món đang đeo (món cũ được bán lấy vàng). Trả về false nếu không mặc.
    bool TryEquipItem(const EquipItem& it, int& soldGold) {
        soldGold = 0;
        int s = it.slot;
        if (hasEquip[s]) {
            if (it.Score() <= equipped[s].Score()) return false;
            ApplyItem(equipped[s], -1.0f);
            soldGold = equipped[s].SellValue();
            player.gold += soldGold;
        }
        equipped[s] = it;
        hasEquip[s] = true;
        ApplyItem(it, 1.0f);
        return true;
    }

    void SpawnItemDrop(Vector2 pos) {
        ItemDrop d;
        d.pos = Vector2Add(pos, { (Rand01() - 0.5f) * 30.0f, (Rand01() - 0.5f) * 30.0f });
        d.item = GenerateItem(floor);
        d.life = 90.0f;
        d.phase = Rand01() * 6.0f;
        itemDrops.push_back(d);
    }

    void UpdateItemDrops(float dt) {
        for (size_t i = 0; i < itemDrops.size();) {
            ItemDrop& d = itemDrops[i];
            d.life -= dt;
            bool remove = d.life <= 0;
            if (!remove && Vector2Distance(d.pos, player.pos) < 40.0f) {
                int sold = 0;
                Color rc = d.item.RarityColor();
                if (TryEquipItem(d.item, sold)) {
                    ShowBanner("EQUIPPED: " + d.item.name + (sold > 0 ? "  (old gear sold +" + std::to_string(sold) + "g)" : ""), 3.0f);
                    PlaySound(snd.sndLevelUp);
                    Burst(player.pos, rc, 24, 260.0f, 0.7f);
                } else {
                    int v = d.item.SellValue();
                    player.gold += v;
                    SpawnPopup(player.pos, "Sold +" + std::to_string(v) + "g", GOLD);
                    PlaySound(snd.sndCoin);
                    Burst(player.pos, GOLD, 5, 140.0f, 0.4f);
                }
                remove = true;
            }
            if (remove) { itemDrops[i] = itemDrops.back(); itemDrops.pop_back(); continue; }
            ++i;
        }
    }

    static void DrawItemIcon(int slot, Vector2 c, float s, Color col, int weaponType = 0) {
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

    void DrawItemDrops() {
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

    void DrawInventory() {
        const ClassDef& cd = kClasses[std::max(0, std::min(3, player.heroClass))];
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
        int px = 70, py = 46, pw = 1140, ph = 632;
        DrawRectangle(px, py, pw, ph, { 14, 18, 30, 248 });
        DrawRectangleLines(px, py, pw, ph, GOLD);
        std::string title = std::string("EQUIPMENT & SKILLS  -  ") + cd.name;
        DrawText(title.c_str(), 1280 / 2 - MeasureText(title.c_str(), 30) / 2, py + 14, 30, cd.color);

        static const char* slotNames[4] = { "WEAPON", "ARMOR", "BOOTS", "RING" };
        int cw = 260, gap = 14, cx0 = px + (pw - (cw * 4 + gap * 3)) / 2, cy = py + 58, ch = 228;
        for (int i = 0; i < 4; ++i) {
            int cx = cx0 + i * (cw + gap);
            Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 70, 80, 105, 255 };
            DrawRectangle(cx, cy, cw, ch, { 22, 28, 44, 255 });
            DrawRectangleLines(cx, cy, cw, ch, rc);
            DrawRectangle(cx, cy, cw, 26, Fade(rc, 0.25f));
            DrawText(slotNames[i], cx + 12, cy + 5, 17, rc);
            if (hasEquip[i]) {
                DrawCircleV({ (float)cx + cw / 2.0f, (float)cy + 66 }, 32.0f, Fade(rc, 0.18f));
                DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 66 }, 52.0f, rc, equipped[i].weaponType);
                DrawText(equipped[i].name.c_str(), cx + 12, cy + 108, 17, rc);
                DrawText(equipped[i].RarityName(), cx + 12, cy + 128, 13, Fade(rc, 0.8f));
                auto lines = equipped[i].StatLines();
                for (size_t k = 0; k < lines.size(); ++k) DrawText(lines[k].c_str(), cx + 12, cy + 148 + (int)k * 18, 15, RAYWHITE);
            } else {
                DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 66 }, 52.0f, { 60, 68, 90, 255 }, 0);
                DrawText("- Empty -", cx + cw / 2 - MeasureText("- Empty -", 18) / 2, cy + 128, 18, { 110, 120, 145, 255 });
            }
        }

        // Chỉ số nhân vật
        int sy = cy + ch + 18;
        DrawText("CHARACTER", px + 36, sy, 20, SKYBLUE);
        char buf[96];
        auto stat = [&](int col, int row, const char* label, const std::string& val) {
            DrawText(label, px + 36 + col * 345, sy + 28 + row * 24, 16, { 170, 185, 215, 255 });
            DrawText(val.c_str(), px + 36 + col * 345 + 155, sy + 28 + row * 24, 16, WHITE);
        };
        snprintf(buf, sizeof(buf), "%d / %d", (int)player.hp, (int)player.maxHp); stat(0, 0, "Health", buf);
        snprintf(buf, sizeof(buf), "%.0f", player.baseDmg); stat(0, 1, "Damage", buf);
        snprintf(buf, sizeof(buf), "%.1f", EffectiveArmor()); stat(0, 2, "Armor", buf);
        snprintf(buf, sizeof(buf), "%.0f", EffectiveMoveSpeed()); stat(1, 0, "Move Speed", buf);
        snprintf(buf, sizeof(buf), "%.2f /s", EffectiveAttackRate()); stat(1, 1, "Attack Speed", buf);
        snprintf(buf, sizeof(buf), "%d%%", (int)roundf(std::min(0.6f, player.critChance) * 100)); stat(1, 2, "Crit Chance", buf);
        snprintf(buf, sizeof(buf), "%d%%", (int)roundf(std::min(0.3f, player.lifesteal) * 100)); stat(2, 0, "Lifesteal", buf);
        snprintf(buf, sizeof(buf), "%.1fs", player.dashCooldown); stat(2, 1, "Dash Cooldown", buf);
        snprintf(buf, sizeof(buf), "%.1fs / %.1fs", Skill1MaxCd(), Skill2MaxCd()); stat(2, 2, "Q / E Cooldown", buf);
        std::string extra = "Rage: " + std::to_string((int)player.rage) + "/100   Ember Lv" + std::to_string(player.igniteLevel) + "   Frost Lv" + std::to_string(player.frostLevel);
        DrawText(extra.c_str(), px + pw - 36 - MeasureText(extra.c_str(), 15), sy + 2, 15, { 150, 165, 200, 255 });

        // Bộ 4 kỹ năng của Class
        int ky = sy + 106;
        DrawText("SKILLS", px + 36, ky, 18, cd.accent);
        const char* tags[4] = { "[PASSIVE]", "[Q]", "[E]", "[R] RAGE" };
        const char* sNames[4] = { cd.passiveName, cd.qName, cd.eName, cd.rName };
        const char* sDescs[4] = { cd.passiveDesc, cd.qDesc, cd.eDesc, cd.rDesc };
        for (int s = 0; s < 4; ++s) {
            int rowY = ky + 24 + s * 22;
            DrawText(tags[s], px + 36, rowY, 15, s == 3 ? ORANGE : (s == 0 ? cd.color : SKYBLUE));
            DrawText(sNames[s], px + 132, rowY, 15, WHITE);
            DrawText(sDescs[s], px + 305, rowY, 14, { 190, 205, 230, 255 });
        }

        if (!player.relics.empty()) {
            DrawText("RELICS", px + 36, py + ph - 92, 15, SKYBLUE);
            int rx = px + 110, ry = py + ph - 92;
            for (const auto& r : player.relics) {
                std::string label = r.name + " (" + RelicDesc(r.type) + ")";
                int w = MeasureText(label.c_str(), 13) + 24;
                if (rx + w > px + pw - 20) { rx = px + 110; ry += 16; }
                DrawCircle(rx + 5, ry + 7, 4.5f, r.color);
                DrawText(label.c_str(), rx + 14, ry + 1, 13, r.color);
                rx += w + 12;
            }
        }
        DrawText("Gear drops from enemies. Better gear equips automatically; weaker gear sells for gold.", px + 36, py + ph - 46, 15, { 170, 185, 215, 255 });
        DrawText("Press I or Esc to close", px + 36, py + ph - 24, 15, { 120, 135, 165, 255 });
    }

    // ── Môi trường: tường gạch, decal, đuốc, cột, ánh sáng ──
    void BuildDecor() {
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
        for (int i = 0; i < 34; ++i) {
            Decal d;
            d.pos = { (Rand01() - 0.5f) * (roomW - 90.0f), (Rand01() - 0.5f) * (roomH - 90.0f) };
            d.angle = Rand01() * 360.0f;
            d.size = 10.0f + 24.0f * Rand01();
            float r = Rand01();
            d.type = r < 0.35f ? 0 : (r < 0.55f ? 1 : (r < 0.72f ? 2 : 3));
            decals.push_back(d);
        }
    }

    void DrawWalls(const BiomeColors& biome) {
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

    void DrawDecals(const BiomeColors& biome) {
        int bi = ((floor - 1) / 3) % 4;
        for (const auto& d : decals) {
            Vector2 p = d.pos;
            float a = d.angle * DEG2RAD, s = d.size;
            if (d.type == 0) { // vết nứt
                Vector2 p1 = { p.x + cosf(a) * s, p.y + sinf(a) * s };
                Vector2 p2 = { p1.x + cosf(a + 0.7f) * s * 0.7f, p1.y + sinf(a + 0.7f) * s * 0.7f };
                Vector2 p0 = { p.x - cosf(a - 0.4f) * s * 0.8f, p.y - sinf(a - 0.4f) * s * 0.8f };
                Color cc = (bi == 2) ? Color{ 255, 120, 40, 210 } : Fade(BLACK, 0.40f);
                DrawLineEx(p0, p, 2.0f, cc);
                DrawLineEx(p, p1, 2.0f, cc);
                DrawLineEx(p1, p2, 1.5f, cc);
            } else if (d.type == 1) { // vết bẩn
                DrawEllipse((int)p.x, (int)p.y, s * 1.2f, s * 0.7f, Fade(bi == 1 ? Color{ 20, 80, 30, 255 } : BLACK, 0.18f));
            } else if (d.type == 2) { // sỏi
                for (int i = 0; i < 3; ++i) {
                    Vector2 q = { p.x + cosf(a + i * 2.1f) * s * 0.5f, p.y + sinf(a + i * 2.1f) * s * 0.35f };
                    DrawCircleV({ q.x, q.y + 1.5f }, 3.0f + i, Fade(BLACK, 0.3f));
                    DrawCircleV(q, 2.6f + i, Shade(biome.tileA, 1.45f));
                }
            } else { // đồ trang trí theo bioma
                if (bi == 0) { // xương
                    Vector2 d1 = { cosf(a) * s * 0.6f, sinf(a) * s * 0.6f };
                    Vector2 d2 = { cosf(a + 1.2f) * s * 0.6f, sinf(a + 1.2f) * s * 0.6f };
                    Color bone = { 214, 208, 190, 255 };
                    DrawLineEx({ p.x - d1.x, p.y - d1.y }, { p.x + d1.x, p.y + d1.y }, 3.0f, bone);
                    DrawLineEx({ p.x - d2.x, p.y - d2.y }, { p.x + d2.x, p.y + d2.y }, 3.0f, bone);
                    DrawCircleV({ p.x + d1.x, p.y + d1.y }, 2.5f, bone);
                    DrawCircleV({ p.x - d2.x, p.y - d2.y }, 2.5f, bone);
                } else if (bi == 1) { // rêu
                    for (int i = 0; i < 5; ++i)
                        DrawCircleV({ p.x + cosf(a + i * 1.3f) * s * 0.6f, p.y + sinf(a + i * 1.3f) * s * 0.4f }, 4.0f + (i % 3) * 2.0f, Fade({ 90, 170, 80, 255 }, 0.32f));
                } else if (bi == 2) { // vũng dung nham
                    DrawEllipse((int)p.x, (int)p.y, s * 0.9f, s * 0.5f, Fade({ 255, 90, 20, 255 }, 0.35f));
                    DrawEllipse((int)p.x, (int)p.y, s * 0.5f, s * 0.28f, Fade({ 255, 200, 60, 255 }, 0.45f));
                } else { // mảnh băng
                    for (int i = 0; i < 3; ++i) {
                        float aa = a + i * 2.1f;
                        Tri({ p.x, p.y }, { p.x + cosf(aa) * s * 0.7f, p.y + sinf(aa) * s * 0.7f }, { p.x + cosf(aa + 0.35f) * s * 0.4f, p.y + sinf(aa + 0.35f) * s * 0.4f }, Fade({ 220, 240, 255, 255 }, 0.55f));
                    }
                }
            }
        }
    }

    void DrawPillar(Vector2 p, const BiomeColors& biome) {
        int bi = ((floor - 1) / 3) % 4;
        Color stone = Shade(biome.wall, 1.18f);
        Color ink = Shade(stone, 0.32f);
        DrawEllipse((int)p.x + 3, (int)p.y + 15, 32.0f, 13.0f, Fade(BLACK, 0.42f));
        // Chân cột đá vuông vát cạnh + thân bát giác + đỉnh cột chạm khắc 3D
        BoxRot({ p.x, p.y + 4.0f }, 52.0f, 44.0f, 0.0f, Shade(stone, 0.58f), ink, 1.8f);
        DrawPoly({ p.x, p.y - 2.0f }, 8, 24.5f, 22.5f, Shade(stone, 0.80f));
        DrawPolyLinesEx({ p.x, p.y - 2.0f }, 8, 24.5f, 22.5f, 1.8f, ink);
        BoxRot({ p.x, p.y - 10.0f }, 42.0f, 36.0f, 0.0f, stone, ink, 1.8f);
        BoxRot({ p.x, p.y - 11.0f }, 26.0f, 22.0f, 0.0f, Shade(stone, 1.24f), Shade(stone, 0.65f), 1.4f);
        DrawLineEx({ p.x - 20.0f, p.y - 28.0f }, { p.x + 20.0f, p.y - 28.0f }, 2.0f, Fade(WHITE, 0.25f));
        DrawLineEx({ p.x + 5.0f, p.y - 14.0f }, { p.x + 12.0f, p.y - 3.0f }, 2.0f, Fade(BLACK, 0.35f));
        DrawLineEx({ p.x + 12.0f, p.y - 3.0f }, { p.x + 7.0f, p.y + 9.0f }, 2.0f, Fade(BLACK, 0.35f));
        if (bi == 1) { // rêu phủ góc cột
            Tri({ p.x + 6.0f, p.y + 8.0f }, { p.x + 22.0f, p.y + 6.0f }, { p.x + 17.0f, p.y + 20.0f }, Fade({ 85, 175, 78, 255 }, 0.72f));
        }
        if (bi == 2) { // mạch dung nham nứt
            DrawLineEx({ p.x - 10.0f, p.y + 5.0f }, { p.x + 3.0f, p.y + 16.0f }, 2.5f, { 255, 130, 40, 255 });
        }
        if (bi == 3) { // mảnh băng trên đỉnh cột
            Tri({ p.x - 14.0f, p.y - 18.0f }, { p.x - 2.0f, p.y - 26.0f }, { p.x + 10.0f, p.y - 16.0f }, Fade({ 220, 244, 255, 255 }, 0.75f));
        }
    }

    void DrawTorch(Vector2 t) {
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

    void AddLight(Vector2 wp, float radius, Color col, float inten) {
        Vector2 sp = GetWorldToScreen2D(wp, camera);
        if (sp.x < -radius || sp.x > 1280 + radius || sp.y < -radius || sp.y > 720 + radius) return;
        CircleGrad(sp.x, sp.y, radius, Fade(col, inten), Fade(col, 0.0f));
    }

    // Bản đồ ánh sáng nhân với cảnh: xa nguồn sáng thì tối dần
    void RenderLightmap() {
        if (lightmap.id == 0) return;
        static const Color ambients[4] = { { 134, 142, 172, 255 }, { 136, 158, 138, 255 }, { 170, 132, 120, 255 }, { 142, 164, 196, 255 } };
        Color amb = ambients[((floor - 1) / 3) % 4];
        float tm = (float)GetTime();

        BeginTextureMode(lightmap);
        ClearBackground(amb);
        BeginBlendMode(BLEND_ADDITIVE);

        AddLight(player.pos, 400.0f, { 255, 244, 222, 255 }, 0.62f + 0.03f * sinf(tm * 7.0f));
        for (const auto& t : torches) {
            float fl = 0.85f + 0.15f * sinf(tm * 13.0f + t.x * 0.1f);
            AddLight({ t.x, t.y - 8.0f }, 260.0f, { 255, 150, 60, 255 }, 0.58f * fl);
        }
        if (portalOpen) AddLight(portalPos, 230.0f, { 80, 220, 255, 255 }, 0.7f);
        if (hasShrine && !shrineUsed) AddLight({ shrinePos.x, shrinePos.y - 40.0f }, 190.0f, { 255, 50, 80, 255 }, 0.55f + 0.1f * sinf(shrinePulse));
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

    void ApplyLighting() {
        if (lightmap.id == 0) return;
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawTextureRec(lightmap.texture, { 0, 0, (float)lightmap.texture.width, -(float)lightmap.texture.height }, { 0, 0 }, WHITE);
        EndBlendMode();
    }

    // ── Vẽ chữ tự xuống dòng & biểu tượng kỹ năng ──
    static int DrawWrappedText(const char* txt, int x, int y, int maxW, int fontSize, int lineH, Color col) {
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

    static void DrawSkillIcon(int heroClass, int slotKind, Vector2 c, float s, Color col) {
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
            } else { // R: Phoenix Arrow Rain
                for (int i = -1; i <= 1; ++i) {
                    DrawLineEx({ c.x + i * s * 0.18f - s * 0.12f, c.y - s * 0.34f }, { c.x + i * s * 0.18f + s * 0.12f, c.y + s * 0.34f }, 2.5f, { 255, 165, 55, 255 });
                }
            }
        }
    }

    // ── Màn hình chọn nhân vật ──
    void UpdateClassSelect() {
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
        }
    }

    void DrawClassSelectScreen() {
        float t = (float)GetTime();
        DrawRectangleGradientV(0, 0, 1280, 720, Fade({ 8, 10, 18, 255 }, 0.92f), Fade({ 12, 16, 28, 255 }, 0.88f));
        const char* hdr = "CHOOSE YOUR HERO";
        OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 42) / 2, 22, 42, GOLD, { 80, 40, 10, 255 });

        const int cardW = 285, gap = 16, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 84, cardH = 532;
        static const char* wpNames[8] = { "Sword", "Dagger", "Axe", "Spear", "Hammer", "Scythe", "Bow", "Staff" };
        for (int i = 0; i < 4; ++i) {
            const ClassDef& cd = kClasses[i];
            int cx = x0 + i * (cardW + gap);
            bool sel = (classSel == i);
            Rectangle r = { (float)cx, (float)cy, (float)cardW, (float)cardH };
            DrawRectangleRec(r, sel ? Color{ 26, 34, 56, 252 } : Color{ 16, 20, 34, 240 });
            DrawRectangle(cx, cy, cardW, 44, Fade(cd.color, sel ? 0.35f : 0.16f));
            DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : Fade(cd.color, 0.55f));

            DrawText(TextFormat("[%d]", i + 1), cx + 10, cy + 12, 16, GOLD);
            DrawText(cd.name, cx + 40, cy + 8, 20, sel ? WHITE : cd.color);
            DrawText(cd.role, cx + 40, cy + 28, 12, { 185, 198, 225, 255 });

            // Xem trước nhân vật sống động
            Vector2 prevPos = { cx + cardW / 2.0f, (float)cy + 102.0f };
            DrawCircleV(prevPos, 38.0f, Fade(cd.color, sel ? 0.18f : 0.08f));
            float aimA = sel ? sinf(t * 1.8f) * 0.45f : 0.25f;
            DrawHeroModel(prevPos, i, cd.startWeapon, { cosf(aimA), sinf(aimA) }, false, t * (sel ? 6.0f : 0.0f), sel, sel ? sinf(t * 5.0f) * 0.3f : 0.35f, false, false, 0.0f, 0.0f);

            // Chỉ số cơ bản
            int stY = cy + 152;
            DrawRectangle(cx + 10, stY, cardW - 20, 56, { 12, 15, 26, 220 });
            DrawText(TextFormat("HP: %d   DMG: %d   ARM: %.1f", (int)cd.baseHp, (int)cd.baseDmg, cd.baseArmor), cx + 18, stY + 8, 13, RAYWHITE);
            DrawText(TextFormat("SPD: %d   ATK: %.1f/s   CRIT: %d%%", (int)cd.baseSpeed, cd.baseAtkRate, (int)roundf(cd.baseCrit * 100)), cx + 18, stY + 24, 13, SKYBLUE);
            DrawText(TextFormat("Weapon: %s", wpNames[cd.startWeapon]), cx + 18, stY + 40, 13, GOLD);

            // 4 kỹ năng: 1 Passive, 2 thường (Q, E), 1 Nộ (R)
            int sy = stY + 66;
            const char* tags[4] = { "PASSIVE", "SKILL [Q]", "SKILL [E]", "ULTIMATE [R]" };
            const char* sn[4] = { cd.passiveName, cd.qName, cd.eName, cd.rName };
            const char* sd[4] = { cd.passiveDesc, cd.qDesc, cd.eDesc, cd.rDesc };
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

        const ClassDef& cur = kClasses[classSel];
        Rectangle startBtn = { 1280 / 2.0f - 180.0f, 634.0f, 360.0f, 46.0f };
        DrawRectangleRec(startBtn, { 34, 88, 56, 255 });
        DrawRectangleLinesEx(startBtn, 2.0f, GOLD);
        std::string bTxt = std::string("PLAY AS ") + cur.name + " [ENTER]";
        DrawText(bTxt.c_str(), 1280 / 2 - MeasureText(bTxt.c_str(), 18) / 2, 648, 18, WHITE);
        const char* foot = "1-4 / A-D / Click: select hero     Enter / Double-Click: start     Esc: back";
        DrawText(foot, 1280 / 2 - MeasureText(foot, 15) / 2, 692, 15, { 150, 165, 200, 255 });
    }

    // ── Hệ thống Lưu / Tải game (tối đa 4 slot, hỗ trợ ghi đè) ──
    static std::string SaveFileName(int slot) {
        return "save_slot_" + std::to_string(std::max(0, std::min(3, slot)) + 1) + ".sav";
    }

    void RefreshSaveSlots() {
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
                    else if (k == "floor") info.floor = std::max(1, atoi(val.c_str()));
                    else if (k == "player_level") info.level = std::max(1, atoi(val.c_str()));
                    else if (k == "player_hp") info.hp = atoi(val.c_str());
                    else if (k == "player_maxHp") info.maxHp = std::max(1, atoi(val.c_str()));
                    else if (k == "player_gold") info.gold = atoi(val.c_str());
                    else if (k == "enemiesSlain") info.enemiesSlain = atoi(val.c_str());
                    else if (k == "hasEquip_0") hasW = atoi(val.c_str());
                    else if (k == "equip_0_weaponType") info.weaponType = std::max(0, std::min(7, atoi(val.c_str())));
                    else if (k == "equip_0_name") info.weaponName = val;
                    else if (k == "timestamp") info.timestamp = val;
                }
                if (existsFlag == 1) {
                    info.exists = true;
                    if (!hasW || info.weaponName.empty()) {
                        info.weaponType = kClasses[info.heroClass].startWeapon;
                        info.weaponName = MakeStarterWeapon(info.weaponType).name;
                    }
                }
            }
            saveSlots[s] = info;
        }
    }

    void DeleteSaveSlot(int slot) {
        if (slot < 0 || slot >= 4 || !saveSlots[slot].exists) return;
        std::string empty = "exists=0\n";
        SaveFileText(SaveFileName(slot).c_str(), (char*)empty.c_str());
        RefreshSaveSlots();
        PlaySound(snd.sndHurt);
        saveStatusMsg = "Cleared Slot " + std::to_string(slot + 1) + ".";
        saveStatusTimer = 2.5f;
    }

    bool SaveGameToSlot(int slot) {
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
          << "player_xp=" << player.xp << "\n"
          << "player_xpNext=" << player.xpNext << "\n"
          << "player_gold=" << player.gold << "\n"
          << "player_rage=" << player.rage << "\n"
          << "player_igniteLevel=" << player.igniteLevel << "\n"
          << "player_frostLevel=" << player.frostLevel << "\n"
          << "player_shieldActive=" << (player.shieldActive ? 1 : 0) << "\n"
          << "relicCount=" << (int)player.relics.size() << "\n";
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

    bool LoadGameFromSlot(int slot) {
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
        std::vector<int> relicTypes;

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
            else if (k == "player_level") loadedP.level = std::max(1, iv);
            else if (k == "player_xp") loadedP.xp = std::max(0, iv);
            else if (k == "player_xpNext") loadedP.xpNext = std::max(10, iv);
            else if (k == "player_gold") loadedP.gold = std::max(0, iv);
            else if (k == "player_rage") loadedP.rage = std::max(0.0f, std::min(100.0f, fv));
            else if (k == "player_igniteLevel") loadedP.igniteLevel = std::max(0, iv);
            else if (k == "player_frostLevel") loadedP.frostLevel = std::max(0, iv);
            else if (k == "player_shieldActive") loadedP.shieldActive = (iv != 0);
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
        for (int i = 0; i < relicCount && i < (int)relicTypes.size(); ++i) {
            int rt = relicTypes[i];
            if (rt >= 0 && rt <= (int)RELIC_THORNS) loadedP.relics.push_back(MakeRelic((RelicType)rt));
        }

        player = loadedP;
        for (int i = 0; i < 4; ++i) {
            hasEquip[i] = loadedHas[i];
            equipped[i] = loadedEq[i];
        }
        floor = loadedFloor;
        enemiesSlain = loadedSlain;
        bestCombo = loadedBestCombo;
        bossesKilled = loadedBosses;
        phoenixUsed = loadedPhoenix;
        selectedClass = player.heroClass;
        comboCount = 0;
        comboTimer = 0.0f;
        damagePulse = 0.0f;
        hitStop = 0.0f;
        newRecord = false;
        (void)loadedSelClass;

        BuildFloor();
        player.pos = { 0, roomH / 2.0f - 90.0f };
        player.hpDisplay = player.hp;
        player.specialCooldown = Skill1MaxCd();
        player.skill2Cooldown = Skill2MaxCd();
        camera.target = player.pos;
        if (roomCleared) {
            enemies.clear();
            pendingSpawns.clear();
            portalOpen = true;
        }
        state = PLAYING;
        PlaySound(snd.sndPortal);
        ShowBanner("LOADED SLOT " + std::to_string(slot + 1) + " - FLOOR " + std::to_string(floor), 3.0f);
        return true;
    }

    void OpenSaveLoad(State from) {
        saveLoadReturn = from;
        saveStatusMsg.clear();
        saveStatusTimer = 0.0f;
        RefreshSaveSlots();
        state = SAVE_LOAD;
        PlaySound(snd.sndCoin);
    }

    void UpdateSaveLoad(float dt) {
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
                    if (CheckCollisionPointRec(m, saveBtn)) { SaveGameToSlot(i); return; }
                    if (info.exists) {
                        Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 94.0f, 36.0f };
                        Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 50.0f, 54.0f, 36.0f };
                        if (CheckCollisionPointRec(m, loadBtn)) { LoadGameFromSlot(i); return; }
                        if (CheckCollisionPointRec(m, delBtn)) { DeleteSaveSlot(i); return; }
                    }
                } else if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 94.0f, 40.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 54.0f, 54.0f, 40.0f };
                    if (CheckCollisionPointRec(m, loadBtn)) { LoadGameFromSlot(i); return; }
                    if (CheckCollisionPointRec(m, delBtn)) { DeleteSaveSlot(i); return; }
                }
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            if (saveLoadReturn == PAUSED) SaveGameToSlot(saveSlotSel);
            else LoadGameFromSlot(saveSlotSel);
        } else if (IsKeyPressed(KEY_L) && saveLoadReturn == PAUSED) {
            LoadGameFromSlot(saveSlotSel);
        } else if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X)) {
            DeleteSaveSlot(saveSlotSel);
        }
    }

    void DrawSaveLoadScreen() {
        float t = (float)GetTime();
        DrawRectangleGradientV(0, 0, 1280, 720, Fade({ 8, 12, 22, 255 }, 0.94f), Fade({ 14, 18, 32, 255 }, 0.92f));
        const char* hdr = (saveLoadReturn == PAUSED) ? "SAVE & LOAD GAME (4 SLOTS)" : "LOAD SAVED GAME (4 SLOTS)";
        OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 38) / 2, 26, 38, GOLD, { 70, 40, 10, 255 });

        const int cardW = 280, gap = 18, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 98, cardH = 505;
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 4; ++i) {
            const SaveSlotInfo& info = saveSlots[i];
            int cx = x0 + i * (cardW + gap);
            bool sel = (saveSlotSel == i);
            Rectangle r = { (float)cx, (float)cy, (float)cardW, (float)cardH };
            const ClassDef& cd = kClasses[std::max(0, std::min(3, info.heroClass))];
            Color accent = info.exists ? cd.color : Color{ 95, 110, 140, 255 };

            DrawRectangleRec(r, sel ? Color{ 26, 34, 56, 252 } : Color{ 16, 20, 34, 240 });
            DrawRectangle(cx, cy, cardW, 42, Fade(accent, sel ? 0.35f : 0.18f));
            DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : Fade(accent, 0.6f));
            DrawText(TextFormat("[%d] SLOT %d", i + 1, i + 1), cx + 14, cy + 11, 20, sel ? WHITE : accent);
            const char* tag = info.exists ? "SAVED" : "EMPTY";
            DrawText(tag, cx + cardW - 14 - MeasureText(tag, 14), cy + 14, 14, info.exists ? Color{ 120, 235, 150, 255 } : GRAY);

            if (info.exists) {
                Vector2 prev = { cx + cardW / 2.0f, (float)cy + 122.0f };
                DrawCircleV(prev, 44.0f, Fade(accent, sel ? 0.18f : 0.09f));
                DrawHeroModel(prev, info.heroClass, info.weaponType, { 0.9f, sinf(t * 2.0f + i) * 0.2f }, false, t * (sel ? 5.0f : 0.0f), sel, 0.35f, false, false, 0.0f, 0.0f);

                int iy = cy + 190;
                DrawText(cd.name, cx + cardW / 2 - MeasureText(cd.name, 20) / 2, iy, 20, cd.color);
                DrawRectangle(cx + 16, iy + 32, cardW - 32, 138, { 12, 16, 28, 225 });
                DrawRectangleLines(cx + 16, iy + 32, cardW - 32, 138, Fade(accent, 0.35f));
                DrawText(TextFormat("Floor %d   -   Level %d", info.floor, info.level), cx + 26, iy + 42, 17, GOLD);
                DrawText(TextFormat("HP: %d / %d", info.hp, info.maxHp), cx + 26, iy + 68, 16, { 245, 110, 120, 255 });
                DrawText(TextFormat("Gold: %d   Slain: %d", info.gold, info.enemiesSlain), cx + 26, iy + 92, 16, RAYWHITE);
                std::string wTxt = "Weapon: " + info.weaponName;
                if ((int)wTxt.size() > 26) wTxt = wTxt.substr(0, 24) + "..";
                DrawText(wTxt.c_str(), cx + 26, iy + 116, 15, SKYBLUE);
                DrawText(info.timestamp.c_str(), cx + 26, iy + 142, 13, { 150, 165, 195, 255 });
            } else {
                DrawText("EMPTY SLOT", cx + cardW / 2 - MeasureText("EMPTY SLOT", 20) / 2, cy + 196, 20, { 110, 122, 150, 255 });
                const char* sub = (saveLoadReturn == PAUSED) ? "Click SAVE HERE below" : "No saved run yet";
                DrawText(sub, cx + cardW / 2 - MeasureText(sub, 15) / 2, cy + 230, 15, { 130, 145, 175, 255 });
            }

            if (saveLoadReturn == PAUSED) {
                Rectangle saveBtn = { r.x + 16.0f, r.y + r.height - 96.0f, r.width - 32.0f, 38.0f };
                bool hovS = CheckCollisionPointRec(m, saveBtn);
                Color sCol = info.exists ? Color{ 160, 102, 24, 255 } : Color{ 34, 106, 62, 255 };
                DrawRectangleRec(saveBtn, hovS ? Shade(sCol, 1.3f) : sCol);
                DrawRectangleLinesEx(saveBtn, 2.0f, hovS ? WHITE : (info.exists ? GOLD : Color{ 110, 235, 145, 255 }));
                const char* sLbl = info.exists ? "OVERWRITE SAVE" : "SAVE HERE";
                DrawText(sLbl, (int)(saveBtn.x + saveBtn.width / 2) - MeasureText(sLbl, 17) / 2, (int)saveBtn.y + 10, 17, WHITE);

                if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 94.0f, 36.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 50.0f, 54.0f, 36.0f };
                    bool hovL = CheckCollisionPointRec(m, loadBtn);
                    bool hovD = CheckCollisionPointRec(m, delBtn);
                    DrawRectangleRec(loadBtn, hovL ? Color{ 48, 104, 165, 255 } : Color{ 34, 76, 124, 255 });
                    DrawRectangleLinesEx(loadBtn, 1.6f, hovL ? WHITE : SKYBLUE);
                    DrawText("LOAD", (int)(loadBtn.x + loadBtn.width / 2) - MeasureText("LOAD", 17) / 2, (int)loadBtn.y + 9, 17, WHITE);
                    DrawRectangleRec(delBtn, hovD ? Color{ 155, 50, 62, 255 } : Color{ 110, 36, 46, 255 });
                    DrawRectangleLinesEx(delBtn, 1.6f, hovD ? WHITE : Color{ 240, 120, 130, 255 });
                    DrawText("DEL", (int)(delBtn.x + delBtn.width / 2) - MeasureText("DEL", 15) / 2, (int)delBtn.y + 10, 15, WHITE);
                } else {
                    Rectangle noBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 32.0f, 36.0f };
                    DrawRectangleRec(noBtn, { 28, 34, 50, 220 });
                    DrawRectangleLinesEx(noBtn, 1.2f, { 60, 70, 95, 255 });
                    DrawText("NO SAVE TO LOAD", (int)(noBtn.x + noBtn.width / 2) - MeasureText("NO SAVE TO LOAD", 14) / 2, (int)noBtn.y + 11, 14, { 110, 120, 145, 255 });
                }
            } else {
                if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 94.0f, 40.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 54.0f, 54.0f, 40.0f };
                    bool hovL = CheckCollisionPointRec(m, loadBtn);
                    bool hovD = CheckCollisionPointRec(m, delBtn);
                    DrawRectangleRec(loadBtn, hovL ? Color{ 44, 130, 82, 255 } : Color{ 32, 98, 60, 255 });
                    DrawRectangleLinesEx(loadBtn, 2.0f, hovL ? WHITE : Color{ 120, 240, 160, 255 });
                    DrawText("LOAD GAME", (int)(loadBtn.x + loadBtn.width / 2) - MeasureText("LOAD GAME", 18) / 2, (int)loadBtn.y + 11, 18, WHITE);
                    DrawRectangleRec(delBtn, hovD ? Color{ 155, 50, 62, 255 } : Color{ 110, 36, 46, 255 });
                    DrawRectangleLinesEx(delBtn, 1.6f, hovD ? WHITE : Color{ 240, 120, 130, 255 });
                    DrawText("DEL", (int)(delBtn.x + delBtn.width / 2) - MeasureText("DEL", 15) / 2, (int)delBtn.y + 12, 15, WHITE);
                } else {
                    Rectangle noBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 32.0f, 40.0f };
                    DrawRectangleRec(noBtn, { 28, 34, 50, 220 });
                    DrawRectangleLinesEx(noBtn, 1.2f, { 60, 70, 95, 255 });
                    DrawText("EMPTY", (int)(noBtn.x + noBtn.width / 2) - MeasureText("EMPTY", 16) / 2, (int)noBtn.y + 12, 16, { 110, 120, 145, 255 });
                }
            }
        }

        if (saveStatusTimer > 0.0f && !saveStatusMsg.empty()) {
            DrawText(saveStatusMsg.c_str(), 1280 / 2 - MeasureText(saveStatusMsg.c_str(), 24) / 2, 626, 24, GOLD);
        }
        const char* foot = (saveLoadReturn == PAUSED)
            ? "1-4 / A-D: select slot     Enter: save/overwrite     L: load     Del/X: delete     Esc: back"
            : "1-4 / A-D: select slot     Enter / Click: load game     Del/X: delete     Esc: back";
        DrawText(foot, 1280 / 2 - MeasureText(foot, 16) / 2, 678, 16, { 150, 165, 200, 255 });
    }

    // ── Màn hình tiêu đề / tạm dừng / hướng dẫn ──
    static void OutlinedText(const char* txt, int x, int y, int size, Color col, Color outline) {
        for (int ox = -2; ox <= 2; ox += 2)
            for (int oy = -2; oy <= 2; oy += 2)
                if (ox != 0 || oy != 0) DrawText(txt, x + ox, y + oy, size, outline);
        DrawText(txt, x, y, size, col);
    }

    void DrawHelpPanel() {
        int w = 860, h = 612, x = 1280 / 2 - w / 2, y = 54;
        DrawRectangle(x, y, w, h, { 14, 18, 30, 245 });
        DrawRectangleLines(x, y, w, h, GOLD);
        DrawText("CONTROLS", x + w / 2 - MeasureText("CONTROLS", 30) / 2, y + 16, 30, GOLD);
        const char* rows[][2] = {
            { "W A S D / Arrows", "Move" },
            { "Mouse", "Aim weapon & skills" },
            { "Left Click / Space / J", "Basic Attack (8 weapon types)" },
            { "Right Click / Shift / K", "Dash (brief invulnerability)" },
            { "Q / U", "Class Skill 1" },
            { "E / L", "Class Skill 2" },
            { "R / Middle Click", "Ultimate Rage Skill (at 100 Rage)" },
            { "1 / 2 / 3", "Pick an upgrade on level-up" },
            { "I", "Equipment, stats & full skill kit" },
            { "F", "Trade with the merchant (every 3rd floor)" },
            { "Gamepad", "Sticks, RT/A atk, B dash, X Skill 1, Y Skill 2, LB Ult" },
            { "O / F2 / F11", "Settings / Windowed Fullscreen" },
            { "S", "Sanctuary: permanent upgrades (title / game over)" },
            { "F5 / L", "Save & Load Game (4 slots with overwrite)" },
            { "M / Esc / F1", "Radar / Pause & Back / Controls" },
        };
        for (int i = 0; i < 15; ++i) {
            DrawText(rows[i][0], x + 36, y + 70 + i * 32, 18, SKYBLUE);
            DrawText(rows[i][1], x + 340, y + 70 + i * 32, 18, RAYWHITE);
        }
        DrawText("Clear a room, enter the portal. Hit enemies to build 100 Rage for [R] Ultimate!", x + 36, y + h - 42, 16, { 190, 200, 220, 255 });
    }

    void DrawTitleScreen() {
        float t = (float)GetTime();
        DrawRectangleGradientV(0, 0, 1280, 720, Fade(BLACK, 0.86f), Fade(BLACK, 0.70f));
        if (showHelp) { DrawHelpPanel(); return; }
        const char* title = "GEOMETRIC DUNGEON";
        int tw = MeasureText(title, 72);
        int ty = 100 + (int)(sinf(t * 1.6f) * 5.0f);
        DrawText(title, 1280 / 2 - tw / 2 + 4, ty + 5, 72, Fade(BLACK, 0.8f));
        OutlinedText(title, 1280 / 2 - tw / 2, ty, 72, GOLD, { 120, 60, 10, 255 });
        const char* sub = "4 Classes  -  8 Weapons  -  Rage Ultimates";
        DrawText(sub, 1280 / 2 - MeasureText(sub, 22) / 2, ty + 88, 22, { 170, 190, 230, 255 });

        std::string rec = "Best Record: Floor " + std::to_string(bestFloor);
        DrawText(rec.c_str(), 1280 / 2 - MeasureText(rec.c_str(), 22) / 2, 292, 22, SKYBLUE);

        const ClassDef& cd = kClasses[std::max(0, std::min(3, selectedClass))];
        std::string heroTxt = std::string("Hero: ") + cd.name;
        DrawText(heroTxt.c_str(), 1280 / 2 - MeasureText(heroTxt.c_str(), 20) / 2, 324, 20, cd.color);

        const char* diffName = settings.difficulty == 0 ? "Easy" : (settings.difficulty == 1 ? "Normal" : "Hard");
        std::string prog = std::string(diffName) + "     Souls: " + std::to_string(souls) + "     Runs: " + std::to_string(runs);
        DrawText(prog.c_str(), 1280 / 2 - MeasureText(prog.c_str(), 20) / 2, 354, 20, { 190, 150, 255, 255 });
#ifndef __ANDROID__
        const char* hint = "Enter: New Game    L: Load Save    S: Sanctuary    O: Settings    F1: Controls";
        DrawText(hint, 1280 / 2 - MeasureText(hint, 18) / 2, 628, 18, { 130, 145, 180, 255 });
#endif
    }

    void DrawPauseScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.72f));
        if (showHelp) { DrawHelpPanel(); return; }
        const char* t1 = "PAUSED";
        OutlinedText(t1, 1280 / 2 - MeasureText(t1, 56) / 2, 108, 56, WHITE, { 20, 24, 40, 255 });
        std::string info = std::string(kClasses[player.heroClass].name) + "   Floor " + std::to_string(floor) + "   Level " + std::to_string(player.level) + "   Slain " + std::to_string(enemiesSlain);
        DrawText(info.c_str(), 1280 / 2 - MeasureText(info.c_str(), 20) / 2, 170, 20, SKYBLUE);
    }

    // ── Drawing ──
    void Draw() {
        if (settings.lighting) RenderLightmap();
        BeginTextureMode(screenRT);
        BiomeColors biome = Biome();
        ClearBackground(biome.bg);

        BeginMode2D(camera);

        // 1. Tường gạch, sàn đá vát cạnh, bóng viền, decal, đuốc, cột
        int halfW = (int)roomW / 2;
        int halfH = (int)roomH / 2;
        int tileSize = 48;
        DrawWalls(biome);
        for (int y = -halfH; y < halfH; y += tileSize) {
            for (int x = -halfW; x < halfW; x += tileSize) {
                int tx = x / tileSize, ty = y / tileSize;
                bool alt = ((tx + ty) & 1) == 0;
                Color tileCol = alt ? biome.tileA : biome.tileB;
                int th = ((tx * 73856093) ^ (ty * 19349663)) & 15;
                tileCol = Shade(tileCol, 0.96f + (th & 3) * 0.025f);
                DrawRectangle(x, y, tileSize, tileSize, tileCol);
                DrawRectangle(x, y, tileSize, 2, Fade(WHITE, 0.04f));
                DrawRectangle(x, y, 2, tileSize, Fade(WHITE, 0.03f));
                DrawRectangleLines(x, y, tileSize, tileSize, Fade(BLACK, 0.16f));
                if (th == 0) DrawRectangle(x + 12, y + 16, 6, 4, Fade(BLACK, 0.12f));
                else if (th == 7) DrawRectangle(x + 28, y + 30, 5, 5, Fade(WHITE, 0.04f));
            }
        }
        // Bóng đổ từ tường xuống sàn
        DrawRectangleGradientV(-halfW, -halfH, (int)roomW, 46, Fade(BLACK, 0.50f), Fade(BLACK, 0.0f));
        DrawRectangleGradientV(-halfW, halfH - 46, (int)roomW, 46, Fade(BLACK, 0.0f), Fade(BLACK, 0.50f));
        DrawRectangleGradientH(-halfW, -halfH, 46, (int)roomH, Fade(BLACK, 0.50f), Fade(BLACK, 0.0f));
        DrawRectangleGradientH(halfW - 46, -halfH, 46, (int)roomH, Fade(BLACK, 0.0f), Fade(BLACK, 0.50f));
        DrawRectangleLinesEx({ -roomW / 2.0f, -roomH / 2.0f, roomW, roomH }, 3.0f, Shade(biome.wall, 1.25f));

        DrawDecals(biome);

        // Bụi lơ lửng
        for (const auto& m : motes) {
            float a2 = 0.10f + 0.16f * (0.5f + 0.5f * sinf((float)GetTime() * 1.6f + m.phase));
            DrawCircleV(m.pos, m.size, Fade(biome.mote, a2));
        }

        for (const auto& tp : torches) DrawTorch(tp);
        for (const auto& pl : pillars) DrawPillar(pl, biome);

        for (const auto& ch : chests) DrawChest(ch);
        if (shopFloor) DrawMerchant();

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
                DrawText("BLOOD SHRINE", (int)sx - 52, (int)sy + 34, 14, { 255, 120, 130, 255 });
            } else {
                DrawCircle((int)sx, (int)sy - 42, 8, { 64, 50, 56, 255 });
            }
        }

        // Exit Portal (vòng ma pháp đa tầng xoay)
        if (portalOpen) {
            float tm = (float)GetTime();
            float pulse = sinf(tm * 6.0f) * 5.0f;
            BeginBlendMode(BLEND_ADDITIVE);
            CircleGrad(portalPos.x, portalPos.y, 58.0f + pulse, Fade(SKYBLUE, 0.45f), Fade(SKYBLUE, 0.0f));
            EndBlendMode();
            DrawRing(portalPos, 32.0f, 36.0f, tm * 70.0f, tm * 70.0f + 270.0f, 24, { 90, 225, 255, 210 });
            DrawRing(portalPos, 23.0f, 27.0f, -tm * 110.0f, -tm * 110.0f + 260.0f, 20, { 170, 245, 255, 230 });
            for (int i = 0; i < 8; ++i) {
                float a = tm * 2.2f + i * (PI / 4.0f);
                DrawCircleV({ portalPos.x + cosf(a) * 34.0f, portalPos.y + sinf(a) * 34.0f }, 3.0f, WHITE);
            }
            DrawCircle((int)portalPos.x, (int)portalPos.y, 19.0f, { 70, 210, 255, 220 });
            DrawCircle((int)portalPos.x, (int)portalPos.y, 11.0f, WHITE);
            DrawText("PORTAL", (int)portalPos.x - 26, (int)portalPos.y + 40, 14, SKYBLUE);
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
            } else if (ge.kind == 2) { // Hố đen Mage
                float alpha = std::min(1.0f, ge.timer);
                DrawCircleV(ge.pos, ge.radius, Fade({ 110, 45, 200, 255 }, 0.12f * alpha));
                DrawRing(ge.pos, ge.radius - 5.0f, ge.radius, -tm * 80.0f, -tm * 80.0f + 310.0f, 36, Fade({ 180, 95, 255, 255 }, 0.75f * alpha));
                DrawRing(ge.pos, 45.0f, 54.0f, tm * 160.0f, tm * 160.0f + 260.0f, 20, Fade({ 120, 225, 255, 255 }, 0.8f * alpha));
                DrawCircleV(ge.pos, 26.0f, Fade({ 15, 8, 30, 255 }, 0.9f * alpha));
            } else if (ge.kind == 3) { // Vùng Mưa Tên Ranger
                float alpha = std::min(1.0f, ge.timer);
                DrawCircleV(ge.pos, ge.radius, Fade({ 255, 145, 45, 255 }, 0.08f * alpha));
                DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius, Fade({ 255, 175, 65, 255 }, 0.65f * alpha));
            } else if (ge.kind == 4) { // Thiên thạch rơi
                float rem = std::max(0.0f, ge.timer / ge.maxTimer);
                DrawCircleV(ge.pos, ge.radius, Fade(ge.color, 0.12f));
                DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius * (1.0f - rem * 0.5f), Fade(ge.color, 0.85f));
                Vector2 skyPos = { ge.pos.x + rem * 110.0f, ge.pos.y - rem * 240.0f };
                DrawLineEx(skyPos, { skyPos.x + 28.0f, skyPos.y - 58.0f }, 10.0f, Fade(ORANGE, 0.7f));
                DrawCircleV(skyPos, 15.0f, { 185, 95, 255, 255 });
                DrawCircleV(skyPos, 8.0f, WHITE);
            } else if (ge.kind == 5) { // Mũi tên Phượng Hoàng rơi
                float rem = std::max(0.0f, ge.timer / ge.maxTimer);
                DrawCircleV(ge.pos, ge.radius, Fade(ORANGE, 0.10f));
                DrawCircleLines((int)ge.pos.x, (int)ge.pos.y, ge.radius * (1.0f - rem * 0.5f), Fade(GOLD, 0.8f));
                Vector2 skyPos = { ge.pos.x - rem * 45.0f, ge.pos.y - rem * 210.0f };
                DrawLineEx(skyPos, { skyPos.x - 14.0f, skyPos.y - 42.0f }, 5.0f, { 255, 130, 40, 255 });
                DrawLineEx(skyPos, { skyPos.x - 10.0f, skyPos.y - 30.0f }, 2.5f, YELLOW);
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
            Color gc = Fade(kClasses[player.heroClass].color, a);
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
            } else if (sl.kind == 12) { // Blade Storm
                DrawRing(sl.pos, 110.0f, 178.0f, sl.angle, sl.angle + 135.0f, 22, Fade({ 255, 65, 130, 255 }, 0.75f * k));
                DrawRing(sl.pos, 110.0f, 178.0f, sl.angle + 180.0f, sl.angle + 315.0f, 22, Fade({ 190, 80, 255, 255 }, 0.75f * k));
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

            // Health bar
            float barW = e.radius * 2.2f;
            float pct = std::max(0.0f, e.hp / e.maxHp);
            DrawRectangle((int)(e.pos.x - barW / 2), (int)(e.pos.y - e.radius - 12), (int)barW, 5, BLACK);
            DrawRectangle((int)(e.pos.x - barW / 2), (int)(e.pos.y - e.radius - 12), (int)(barW * pct), 5, RED);
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
            } else { // Tinh thể năng lượng định hướng
                Vector2 tail = Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 2.4f));
                Tri(tail, Vector2Add(p.pos, Vector2Scale(perp, p.radius * 0.9f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 0.9f)), Fade(p.color, 0.42f));
                Vector2 head = Vector2Add(p.pos, Vector2Scale(dir, p.radius * 1.35f));
                QuadOut(head, Vector2Add(p.pos, Vector2Scale(perp, p.radius * 0.85f)),
                        Vector2Subtract(p.pos, Vector2Scale(dir, p.radius * 0.95f)), Vector2Subtract(p.pos, Vector2Scale(perp, p.radius * 0.85f)),
                        p.color, WHITE, 1.2f);
            }
        }

        // Player
        DrawEllipse((int)player.pos.x, (int)player.pos.y + 16, 20, 8, { 0, 0, 0, 100 }); // Shadow
        DrawPlayerSprite();

        DrawItemDrops();

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

        // Vignette + viền đỏ khi bị đau
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
        const ClassDef& cd = kClasses[std::max(0, std::min(3, player.heroClass))];
        // Top-Left: Class Badge, HP (with damage lag), XP, Rage, Gold
        DrawRectangle(20, 14, 320, 22, { 18, 22, 34, 225 });
        DrawRectangleLines(20, 14, 320, 22, Fade(cd.color, 0.75f));
        DrawText(cd.name, 28, 17, 15, cd.color);
        std::string clsRight = "LVL " + std::to_string(player.level);
        DrawText(clsRight.c_str(), 332 - MeasureText(clsRight.c_str(), 15), 17, 15, GOLD);

        DrawRectangle(20, 40, 320, 24, { 35, 38, 48, 220 });
        float lagPct = std::max(0.0f, std::min(1.0f, player.hpDisplay / std::max(1.0f, player.maxHp)));
        float hpPct = std::max(0.0f, std::min(1.0f, player.hp / std::max(1.0f, player.maxHp)));
        DrawRectangle(22, 42, (int)(316 * lagPct), 20, { 245, 220, 220, 210 });
        DrawRectangle(22, 42, (int)(316 * hpPct), 20, { 225, 48, 62, 255 });
        std::string hpTxt = "HP " + std::to_string((int)player.hp) + " / " + std::to_string((int)player.maxHp);
        DrawText(hpTxt.c_str(), 116, 44, 16, WHITE);

        DrawRectangle(20, 68, 320, 14, { 35, 38, 48, 220 });
        float xpPct = std::min(1.0f, (float)player.xp / std::max(1, player.xpNext));
        DrawRectangle(22, 70, (int)(316 * xpPct), 10, SKYBLUE);
        std::string xpTxt = "XP " + std::to_string(player.xp) + "/" + std::to_string(player.xpNext);
        DrawText(xpTxt.c_str(), 134, 69, 12, WHITE);

        // Thanh Nộ (Rage Bar)
        DrawRectangle(20, 86, 320, 16, { 35, 26, 22, 225 });
        float ragePct = std::min(1.0f, player.rage / player.maxRage);
        bool rageReady = (player.rage >= player.maxRage);
        Color rageCol = rageReady ? ((int)(GetTime() * 8.0f) % 2 == 0 ? GOLD : ORANGE) : Color{ 235, 110, 35, 255 };
        DrawRectangle(22, 88, (int)(316 * ragePct), 12, rageCol);
        DrawRectangleLines(20, 86, 320, 16, rageReady ? GOLD : Color{ 90, 60, 40, 255 });
        std::string rageTxt = rageReady ? "RAGE 100/100  [R] ULTIMATE READY!" : ("RAGE " + std::to_string((int)player.rage) + " / 100");
        DrawText(rageTxt.c_str(), 180 - MeasureText(rageTxt.c_str(), 12) / 2, 88, 12, WHITE);

        std::string goldTxt = "Gold: " + std::to_string(player.gold);
        DrawText(goldTxt.c_str(), 24, 108, 18, GOLD);

        // Relics Row
        DrawText("Relics:", 24, 132, 15, SKYBLUE);
        if (player.relics.empty()) {
            DrawText("None", 24 + MeasureText("Relics: ", 15), 132, 15, SKYBLUE);
        } else {
            int rx = 24 + MeasureText("Relics: ", 15);
            for (const auto& r : player.relics) {
                DrawCircle(rx + 6, 139, 5.5f, r.color);
                DrawCircleLines(rx + 6, 139, 5.5f, WHITE);
                DrawText(r.name.c_str(), rx + 16, 132, 15, r.color);
                rx += 16 + MeasureText(r.name.c_str(), 15) + 12;
            }
        }

        // Top-Center: Floor & Combo
        std::string floorStr = "FLOOR " + std::to_string(floor);
        DrawText(floorStr.c_str(), 1280 / 2 - MeasureText(floorStr.c_str(), 32) / 2, 20, 32, RAYWHITE);

        if (comboCount > 1) {
            std::string comboStr = "COMBO x" + std::to_string(comboCount) + "!";
            Color cCol = comboCount >= 20 ? RED : (comboCount >= 10 ? ORANGE : YELLOW);
            DrawText(comboStr.c_str(), 1280 - 240, 160, 26, cCol);
        }

        // Trang bị đang đeo (HUD)
        for (int i = 0; i < 4; ++i) {
            int hx = 24 + i * 36, hy = 156;
            Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 60, 68, 90, 255 };
            DrawRectangle(hx, hy, 30, 30, { 14, 18, 30, 210 });
            DrawRectangleLines(hx, hy, 30, 30, rc);
            DrawItemIcon(i, { hx + 15.0f, hy + 15.0f }, 17.0f, hasEquip[i] ? rc : Color{ 70, 78, 100, 255 }, hasEquip[i] ? equipped[i].weaponType : 0);
        }
        DrawText("[I] Gear & Skills", 24 + 4 * 36 + 4, 164, 14, { 150, 165, 200, 255 });

        if (touchMode && state == PLAYING) DrawTouchControls();

        // Thanh máu boss
        for (const auto& e : enemies) {
            if (e.type == BOSS && e.hp > 0) {
                int bw = 460, bx = 1280 / 2 - bw / 2, by = 66;
                float pct = std::max(0.0f, e.hp / e.maxHp);
                DrawRectangle(bx - 3, by - 3, bw + 6, 22, { 10, 12, 20, 230 });
                DrawRectangle(bx, by, (int)(bw * pct), 16, (e.bossKind == 1) ? (e.enraged ? Color{ 255, 90, 30, 255 } : Color{ 255, 150, 40, 255 }) : (e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 190, 60, 200, 255 }));
                DrawRectangleLines(bx - 3, by - 3, bw + 6, 22, GOLD);
                const char* nm = (e.bossKind == 1) ? (e.enraged ? "THE MOLTEN COLOSSUS - ENRAGED" : "THE MOLTEN COLOSSUS") : (e.enraged ? "THE VOID SOVEREIGN - ENRAGED" : "THE VOID SOVEREIGN");
                DrawText(nm, 1280 / 2 - MeasureText(nm, 14) / 2, by + 1, 14, WHITE);
                break;
            }
        }

        // Mini-Map Radar (Top-Right)
        if (showMiniMap) {
            int mapX = 1280 - 180;
            int mapY = 20;
            int mapW = 160;
            int mapH = 110;
            DrawRectangle(mapX, mapY, mapW, mapH, { 15, 20, 30, 220 });
            DrawRectangleLines(mapX, mapY, mapW, mapH, { 70, 90, 130, 255 });
            DrawText("RADAR [M]", mapX + 48, mapY + 4, 11, SKYBLUE);

            float sX = (mapW - 16) / roomW;
            float sY = (mapH - 16) / roomH;
            float midX = mapX + mapW / 2.0f;
            float midY = mapY + mapH / 2.0f;

            // Player dot
            DrawCircle((int)(midX + player.pos.x * sX), (int)(midY + player.pos.y * sY), 3.5f, GREEN);
            // Portal dot
            if (portalOpen) DrawCircle((int)(midX + portalPos.x * sX), (int)(midY + portalPos.y * sY), 4.5f, SKYBLUE);
            // Enemy dots
            for (const auto& e : enemies) {
                if (e.hp <= 0) continue;
                DrawCircle((int)(midX + e.pos.x * sX), (int)(midY + e.pos.y * sY), e.type == BOSS ? 5.0f : 2.5f, e.type == BOSS ? GOLD : RED);
            }
        }

        // Thanh 5 ô kỹ năng ở giữa đáy màn hình (ẩn khi dùng cảm ứng vì đã có nút trên màn hình)
        if (!touchMode) {
            const int slotW = 66, slotH = 66, gap = 10;
            const int dockX0 = 1280 / 2 - (slotW * 5 + gap * 4) / 2;
            const int dockY = 720 - 80;
            struct SlotUI { const char* key; int iconKind; Color col; float cd; float maxCd; bool isUlt; bool ready; };
            SlotUI slots[5] = {
                { "PSV", 0, cd.color, 0.0f, 1.0f, false, true },
                { "RMB", -1, YELLOW, player.dashTimer, player.dashCooldown, false, player.dashTimer <= 0.0f },
                { "Q", 1, SKYBLUE, player.specialTimer, std::max(0.1f, player.specialCooldown), false, player.specialTimer <= 0.0f },
                { "E", 2, { 110, 235, 175, 255 }, player.skill2Timer, std::max(0.1f, player.skill2Cooldown), false, player.skill2Timer <= 0.0f },
                { "R", 3, ORANGE, 0.0f, 1.0f, true, rageReady }
            };
            for (int s = 0; s < 5; ++s) {
                int sx = dockX0 + s * (slotW + gap);
                const SlotUI& ui = slots[s];
                DrawRectangle(sx, dockY, slotW, slotH, { 18, 22, 34, 230 });
                if (ui.isUlt) {
                    int fillH = (int)(slotH * ragePct);
                    DrawRectangle(sx, dockY + slotH - fillH, slotW, fillH, Fade(ui.ready ? GOLD : ORANGE, ui.ready ? 0.35f : 0.22f));
                }
                Color border = ui.ready ? (ui.isUlt ? ((int)(GetTime() * 8.0f) % 2 == 0 ? GOLD : WHITE) : ui.col) : Color{ 60, 68, 90, 255 };
                DrawRectangleLinesEx({ (float)sx, (float)dockY, (float)slotW, (float)slotH }, ui.ready && ui.isUlt ? 2.5f : 1.5f, border);
                DrawSkillIcon(player.heroClass, ui.iconKind, { sx + slotW / 2.0f, dockY + 28.0f }, 36.0f, ui.ready ? ui.col : GRAY);
                if (ui.cd > 0.0f) {
                    float pct = std::min(1.0f, ui.cd / std::max(0.01f, ui.maxCd));
                    DrawRectangle(sx, dockY + (int)((1.0f - pct) * slotH), slotW, (int)(pct * slotH), Fade(BLACK, 0.72f));
                    const char* cdt = TextFormat("%.1f", ui.cd);
                    DrawText(cdt, sx + slotW / 2 - MeasureText(cdt, 18) / 2, dockY + 20, 18, WHITE);
                } else if (ui.isUlt && !ui.ready) {
                    const char* rpt = TextFormat("%d%%", (int)player.rage);
                    DrawText(rpt, sx + slotW / 2 - MeasureText(rpt, 13) / 2, dockY + 46, 13, GOLD);
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

        // ── Level Up Modal ──
        if (state == LEVEL_UP) {
            DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.8f));
            DrawText("LEVEL UP! CHOOSE AN UPGRADE", 1280 / 2 - 250, 150, 34, GOLD);

            float cardW = 240;
            float startX = 1280 / 2 - (cardW * 3 + 40) / 2;

            for (size_t i = 0; i < levelChoices.size(); ++i) {
                float cx = startX + i * (cardW + 20);
                float cy = 240;
                DrawRectangle((int)cx, (int)cy, (int)cardW, 280, { 25, 32, 45, 245 });
                DrawRectangleLines((int)cx, (int)cy, (int)cardW, 280, SKYBLUE);

                std::string title = UpgradeTitle(levelChoices[i]);
                std::string desc = UpgradeDesc(levelChoices[i]);

                DrawText(title.c_str(), (int)cx + 20, (int)cy + 30, 22, GOLD);
                DrawText(desc.c_str(), (int)cx + 20, (int)cy + 85, 17, RAYWHITE);

                std::string btn = touchMode ? std::string("TAP TO CHOOSE") : "PRESS [" + std::to_string(i + 1) + "]";
                DrawRectangle((int)cx + 30, (int)cy + 210, 180, 42, DARKBLUE);
                DrawText(btn.c_str(), (int)cx + 120 - MeasureText(btn.c_str(), 18) / 2, (int)cy + 222, 18, WHITE);
            }
        }

        // ── Game Over Modal ──
        if (state == GAME_OVER) {
            DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.88f));
            DrawText("YOU HAVE FALLEN", 1280 / 2 - 180, 160, 44, RED);

            std::string st0 = std::string("Class: ") + kClasses[player.heroClass].name;
            std::string st1 = "Floor Reached: " + std::to_string(floor);
            std::string st2 = "Final Level: " + std::to_string(player.level);
            std::string st3 = "Enemies Slain: " + std::to_string(enemiesSlain);
            std::string st4 = "Gold Accumulated: " + std::to_string(player.gold);
            std::string st5 = "Best Record: Floor " + std::to_string(bestFloor) + (newRecord ? "  (NEW RECORD!)" : "");

            DrawText(st0.c_str(), 1280 / 2 - 140, 226, 22, kClasses[player.heroClass].color);
            DrawText(st1.c_str(), 1280 / 2 - 140, 260, 22, RAYWHITE);
            DrawText(st2.c_str(), 1280 / 2 - 140, 295, 22, RAYWHITE);
            DrawText(st3.c_str(), 1280 / 2 - 140, 330, 22, RAYWHITE);
            DrawText(st4.c_str(), 1280 / 2 - 140, 365, 22, GOLD);
            DrawText(st5.c_str(), 1280 / 2 - 140, 400, 22, newRecord ? GREEN : SKYBLUE);
            if (bestCombo >= 2) {
                std::string st6 = "Best Combo: x" + std::to_string(bestCombo);
                DrawText(st6.c_str(), 1280 / 2 - 140, 428, 22, ORANGE);
            }

            std::string st7 = "Souls earned: +" + std::to_string(lastRunSouls) + "   (total " + std::to_string(souls) + ")";
            DrawText(st7.c_str(), 1280 / 2 - 140, 458, 22, { 190, 225, 255, 255 });

            // nút PLAY AGAIN / CHANGE HERO / SANCTUARY được vẽ bởi DrawButtons()
        }

        if (state == INVENTORY) DrawInventory();
        if (state == SETTINGS) DrawSettingsScreen();
        if (state == SHOP) DrawShopScreen();
        if (state == META) DrawMetaScreen();
        if (state == CLASS_SELECT) DrawClassSelectScreen();
        if (state == SAVE_LOAD) DrawSaveLoadScreen();
        if (state == TITLE) DrawTitleScreen();
        if (state == PAUSED) DrawPauseScreen();

        DrawButtons();

        if (settings.showFps) {
            std::string f = "FPS: " + std::to_string(GetFPS());
            DrawText(f.c_str(), 1280 - 12 - MeasureText(f.c_str(), 18), 720 - 26, 18, { 120, 255, 140, 255 });
        }

        EndTextureMode();
        Present();
    }
};

int main() {
#ifndef __ANDROID__
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
#endif
    InitWindow(1280, 720, "Geometric Dungeon - Standalone Roguelike RPG");
#ifndef __ANDROID__
    SetWindowMinSize(640, 360);
#endif
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // Esc dùng để tạm dừng, không thoát ngay

    Game game;
    game.Init();

    while (!WindowShouldClose() && !game.quitRequested) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;
        game.Update(dt);
        game.Draw();
    }

    game.StopRumble();
    game.snd.Close();
    CloseWindow();
    return 0;
}
