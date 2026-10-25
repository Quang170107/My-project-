#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <random>

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
enum EnemyType { CHASER, RANGER, BRUTE, BOSS, BOMBER, CHARGER };
enum RelicType { RELIC_LIGHTNING, RELIC_TOXIC, RELIC_FROST, RELIC_AEGIS, RELIC_CHALICE, RELIC_COMBUSTION };
enum UpgradeType { UP_HP, UP_DMG, UP_SPEED, UP_ATTACK_RATE, UP_DASH, UP_LIFESTEAL, UP_ARMOR, UP_CRIT, UP_NOVA, UP_IGNITE, UP_FROST };

struct Relic {
    RelicType type;
    std::string name;
    std::string symbol;
    Color color;
};

struct Player {
    Vector2 pos;
    Vector2 vel;
    float maxHp = 100.0f;
    float hp = 100.0f;
    float speed = 280.0f;
    float baseDmg = 25.0f;
    float attackRate = 2.5f;
    float attackTimer = 0.0f;
    float dashTimer = 0.0f;
    float dashCooldown = 1.2f;
    bool isDashing = false;
    float dashDur = 0.2f;
    Vector2 dashDir;
    float specialTimer = 0.0f;
    float specialCooldown = 6.0f;
    float critChance = 0.05f;
    float lifesteal = 0.0f;
    float armor = 0.0f;
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
    float hitFlash = 0.0f;   // nháy trắng khi trúng đòn
    int chargeState = 0;     // Charger: 0 tiến lại, 1 báo hiệu, 2 lao tới, 3 choáng
    float chargeTimer = 0.0f;
    float chargeCooldown = 1.2f;
    Vector2 chargeDir = { 1, 0 };
};

struct Projectile {
    Vector2 pos;
    Vector2 vel;
    float damage;
    bool fromPlayer;
    float life;
    float radius;
    Color color;
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
    float timer = 0.15f;
    float maxTimer = 0.15f;
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

static float Rand01() { return (float)rand() / (float)RAND_MAX; }

// ── Trang bị (4 ô: vũ khí, giáp, giày, nhẫn) ─────────────────────────────
enum EquipSlot { SLOT_WEAPON, SLOT_ARMOR, SLOT_BOOTS, SLOT_RING };

struct EquipItem {
    std::string name;
    int slot = 0;
    int rarity = 0; // 0 thường, 1 hiếm, 2 sử thi, 3 huyền thoại
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

static EquipItem GenerateItem(int floor) {
    static const char* names[4][4] = {
        { "Sword", "Blade", "Axe", "Spear" }, { "Plate", "Mail", "Vest", "Cuirass" },
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
    it.name = std::string(prefix[it.rarity][rand() % 4]) + " " + names[it.slot][rand() % 4];
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

// ── Cài đặt (lưu vào settings.cfg cạnh file exe) ─────────────────────────
struct Settings {
    int resW = 1280, resH = 720;
    bool fullscreen = false;
    bool vsync = true;
    int fpsIndex = 1;     // chỉ số trong kFpsOptions
    bool showFps = false;
    bool vibration = true; // rung màn hình
    bool lighting = true;
    int particles = 2;    // 0 thấp, 1 vừa, 2 cao
    bool vignette = true;
    int volume = 10;      // 0..10
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

    enum State { PLAYING, LEVEL_UP, GAME_OVER, TITLE, PAUSED, INVENTORY, SETTINGS, SHOP };
    State state = PLAYING;
    bool quitRequested = false;
    bool showHelp = false;
    State prevState = PLAYING;
    EquipItem equipped[4];
    bool hasEquip[4] = { false, false, false, false };
    std::vector<ItemDrop> itemDrops;
    Settings settings;
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

    void Init() {
        snd.Init();
        camera.zoom = 1.0f;
        camera.offset = { 1280 / 2.0f, 720 / 2.0f };
        InitMotes();
        lightmap = LoadRenderTexture(1280, 720);
        screenRT = LoadRenderTexture(1280, 720);
        SetTextureFilter(screenRT.texture, TEXTURE_FILTER_BILINEAR);
        LoadSettings();
        BuildResList();
        ApplySettings();
        RestartRun();
        state = TITLE;
    }

    void RestartRun() {
        player = Player();
        player.pos = { 0, 240 };
        floor = 1;
        enemiesSlain = 0;
        comboCount = 0;
        bestCombo = 0;
        damagePulse = 0.0f;
        hitStop = 0.0f;
        particles.clear();
        for (int i = 0; i < 4; ++i) hasEquip[i] = false;
        itemDrops.clear();
        newRecord = false;
        state = PLAYING;
        BuildFloor();
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
            case UP_NOVA: return "-0.8s Nova Cooldown\nSpam Nova";
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
    }

    void BuildFloor() {
        enemies.clear();
        projectiles.clear();
        loots.clear();
        slashes.clear();
        novas.clear();
        ghosts.clear();
        pillars.clear();
        particles.clear();
        itemDrops.clear();
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
        eliteChance = (floorMod == MOD_ELITE) ? 0.55f : 0.20f;

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
            int spitters = std::min(8, std::max(0, (floor - 1) * 2));
            int brutes = std::min(5, std::max(0, floor - 2));
            int chargers = (floor >= 2) ? std::min(3, 1 + (floor - 2) / 3) : 0;
            int bombers = (floor >= 4) ? std::min(4, 1 + (floor - 4) / 2) : 0;
            switch (floorMod) {
                case MOD_SWARM:
                    chasers = std::min(16, (int)roundf((3 + floor) * 1.8f));
                    spitters = std::min(8, std::max(0, floor - 1));
                    brutes = 0; bombers = 0; chargers = std::min(chargers, 1);
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
            int total = chasers + spitters + brutes + chargers + bombers + ((floor == 4 || floor == 8) ? 1 : 0);
            while (total > maxTotal && (chasers > 2 || spitters > 2)) {
                if (chasers > 2) chasers--; else spitters--;
                total--;
            }

            for (int i = 0; i < chasers; ++i) SpawnEnemy(CHASER, GetRandomSpawnPos());
            for (int i = 0; i < spitters; ++i) SpawnEnemy(RANGER, GetRandomSpawnPos());
            for (int i = 0; i < brutes; ++i) SpawnEnemy(BRUTE, GetRandomSpawnPos());
            for (int i = 0; i < chargers; ++i) SpawnEnemy(CHARGER, GetRandomSpawnPos());
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
        w.maxHp = 180.0f * (1.0f + (floor - 1) * 0.15f);
        w.hp = w.maxHp;
        w.speed = 85.0f;
        w.contactDmg = 20.0f * (1.0f + (floor - 1) * 0.10f);
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

    void SpawnEnemy(EnemyType type, Vector2 pos) {
        Enemy e;
        e.type = type;
        e.pos = pos;
        e.vel = { 0, 0 };
        float multHp = 1.0f + (floor - 1) * 0.15f;
        float multDmg = 1.0f + (floor - 1) * 0.10f;

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
        } else if (type == BOMBER) {
            e.maxHp = 22.0f * multHp;
            e.speed = 190.0f;
            e.contactDmg = 6.0f * multDmg;
            e.radius = 17.0f;
        } else if (type == BOSS) {
            e.maxHp = 260.0f * multHp;
            e.speed = 115.0f;
            e.contactDmg = 20.0f * multDmg;
            e.radius = 42.0f;
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

        enemies.push_back(e);
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

    void AddRelic(RelicType t) {
        if (HasRelic(t)) return;
        Relic r;
        r.type = t;
        if (t == RELIC_LIGHTNING) { r.name = "Thunder Conduit"; r.symbol = "⚡"; r.color = SKYBLUE; }
        else if (t == RELIC_TOXIC) { r.name = "Toxic Fang"; r.symbol = "☣"; r.color = LIME; }
        else if (t == RELIC_FROST) { r.name = "Frost Core"; r.symbol = "❄"; r.color = RAYWHITE; }
        else if (t == RELIC_AEGIS) { r.name = "Divine Aegis"; r.symbol = "🛡"; r.color = GOLD; player.shieldActive = true; }
        else if (t == RELIC_CHALICE) { r.name = "Blood Chalice"; r.symbol = "🩸"; r.color = RED; }
        else if (t == RELIC_COMBUSTION) { r.name = "Combustion Sigil"; r.symbol = "💥"; r.color = ORANGE; }
        player.relics.push_back(r);
        PlaySound(snd.sndLevelUp);
        ShowBanner("RELIC ACQUIRED: " + r.name + "!");
    }

    void Update(float dt) {
        if (bannerTimer > 0) bannerTimer -= dt;
        if (screenShakeTimer > 0) screenShakeTimer -= dt;

        // Hit-stop: khựng khung hình rất ngắn khi chí mạng
        if (hitStop > 0) { hitStop -= dt; return; }

        if (state == TITLE) {
            UpdateEffects(dt);
            bannerTimer = 3.0f; // giữ thông báo tầng 1 cho đến khi bắt đầu chơi
            if (IsKeyPressed(KEY_O) || IsKeyPressed(KEY_F2)) { settingsReturn = TITLE; setSel = 0; state = SETTINGS; return; }
            if (IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H)) showHelp = !showHelp;
            else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                showHelp = false;
                state = PLAYING;
                PlaySound(snd.sndPotion);
            }
            if (IsKeyPressed(KEY_ESCAPE)) quitRequested = true;
            return;
        }
        if (state == PAUSED) {
            if (IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_H)) showHelp = !showHelp;
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER)) { showHelp = false; state = PLAYING; }
            if (IsKeyPressed(KEY_R)) { showHelp = false; RestartRun(); }
            if (IsKeyPressed(KEY_I)) { prevState = PAUSED; state = INVENTORY; }
            if (IsKeyPressed(KEY_O)) { settingsReturn = PAUSED; setSel = 0; state = SETTINGS; }
            if (IsKeyPressed(KEY_Q)) quitRequested = true;
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
        if (state == INVENTORY) {
            if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) state = prevState;
            return;
        }
        if (state == PLAYING && IsKeyPressed(KEY_I)) {
            prevState = PLAYING;
            state = INVENTORY;
            return;
        }
        if (state == PLAYING && (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P))) {
            state = PAUSED;
            return;
        }

        UpdateEffects(dt);

        if (state == LEVEL_UP) {
            // Number keys 1, 2, 3 to choose
            if (IsKeyPressed(KEY_ONE) && levelChoices.size() >= 1) ChooseUpgrade(levelChoices[0]);
            if (IsKeyPressed(KEY_TWO) && levelChoices.size() >= 2) ChooseUpgrade(levelChoices[1]);
            if (IsKeyPressed(KEY_THREE) && levelChoices.size() >= 3) ChooseUpgrade(levelChoices[2]);
            return;
        }

        if (state == GAME_OVER) {
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                RestartRun();
            }
            return;
        }

        // Toggle radar with M
        if (IsKeyPressed(KEY_M)) showMiniMap = !showMiniMap;

        // Player Timers
        if (player.dashTimer > 0) player.dashTimer -= dt;
        if (player.specialTimer > 0) player.specialTimer -= dt;
        if (player.attackTimer > 0) player.attackTimer -= dt;
        if (player.invulnTimer > 0) player.invulnTimer -= dt;
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
        if (fabs(aimDir.x) > 0.1f) player.facingLeft = (aimDir.x < 0);
        aimDirDraw = aimDir;

        if (player.isDashing) {
            player.pos = Vector2Add(player.pos, Vector2Scale(player.dashDir, player.speed * 2.8f * dt));
            if (player.dashTimer <= player.dashCooldown - player.dashDur) {
                player.isDashing = false;
            }
        } else {
            player.pos = Vector2Add(player.pos, Vector2Scale(move, player.speed * dt));
            if (Vector2Length(move) > 0.1f) player.walkTimer += dt * 10.0f;
        }

        // Bounds Clamp
        float boundW = roomW / 2.0f - 30;
        float boundH = roomH / 2.0f - 30;
        player.pos.x = Clamp(player.pos.x, -boundW, boundW);
        player.pos.y = Clamp(player.pos.y, -boundH, boundH);

        // Dash Action
        if ((IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_K)) && player.dashTimer <= 0) {
            player.isDashing = true;
            player.dashTimer = player.dashCooldown;
            player.dashDir = (Vector2Length(move) > 0.1f) ? move : aimDir;
            PlaySound(snd.sndDash);
            ghosts.push_back({ player.pos, player.facingLeft, 0.22f, 0.22f });
        }

        // Melee Slash Attack
        if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_J)) && player.attackTimer <= 0 && !player.isDashing) {
            player.attackTimer = 1.0f / player.attackRate;
            PlaySound(snd.sndSlash);

            SlashVisual sv;
            sv.pos = Vector2Add(player.pos, Vector2Scale(aimDir, 35.0f));
            sv.angle = atan2f(aimDir.y, aimDir.x) * RAD2DEG;
            slashes.push_back(sv);

            // Hit query in front
            Vector2 hitCenter = Vector2Add(player.pos, Vector2Scale(aimDir, 45.0f));
            float hitRadius = 75.0f;
            bool hitAny = false;

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(hitCenter, e.pos) <= hitRadius + e.radius) {
                    hitAny = true;
                    bool isCrit = ((float)rand() / RAND_MAX < player.critChance);
                    float dmg = player.baseDmg * (isCrit ? 2.0f : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, isCrit ? 240.0f : 160.0f, isCrit);
                }
            }
            if (hitAny) {
                PlaySound(snd.sndHit);
                Shake(0.12f, 3.5f);
            }
        }

        // Special Nova Pulse (Q / E / U)
        if ((IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_U)) && player.specialTimer <= 0) {
            player.specialTimer = player.specialCooldown;
            PlaySound(snd.sndSpecial);
            Shake(0.25f, 6.0f);
            Burst(player.pos, SKYBLUE, 34, 380.0f, 0.6f);

            NovaVisual nv;
            nv.pos = player.pos;
            nv.radius = 0;
            nv.maxRadius = 190.0f;
            nv.timer = 0.35f;
            nv.maxTimer = 0.35f;
            nv.color = SKYBLUE;
            novas.push_back(nv);

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                if (Vector2Distance(player.pos, e.pos) <= nv.maxRadius) {
                    bool isCrit = ((float)rand() / RAND_MAX < player.critChance);
                    float dmg = player.baseDmg * 1.8f * (isCrit ? 2.0f : 1.0f);
                    Vector2 kb = Vector2Normalize(Vector2Subtract(e.pos, player.pos));
                    DamageEnemy(e, dmg, kb, 280.0f, isCrit);
                }
            }
        }

        // ── Enemies AI & Update ──
        for (auto& e : enemies) {
            if (e.hp <= 0) continue;

            if (e.hitFlash > 0) e.hitFlash -= dt;

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
            Vector2 dir = Vector2Normalize(toPlayer);
            if (fabs(dir.x) > 0.05f) e.facingLeft = (dir.x < 0);

            if (e.type == CHASER) {
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
            bool hitPillar = ResolvePillars(e.pos, e.radius * 0.85f);
            if (hitPillar && e.type == CHARGER && e.chargeState == 2) { // đâm vào cột: choáng
                e.chargeState = 3;
                e.chargeTimer = 1.2f;
                Shake(0.15f, 4.0f);
                Burst(e.pos, { 200, 190, 170, 255 }, 10, 180.0f, 0.4f);
            }
            if (Vector2Distance(e.pos, player.pos) < e.radius + 18.0f) {
                float contactMul = (e.type == CHARGER && e.chargeState == 2) ? 1.8f : 1.0f;
                DamagePlayer(e.contactDmg * contactMul);
            }
        }

        // ── Projectiles ──
        for (size_t i = 0; i < projectiles.size();) {
            auto& p = projectiles[i];
            p.pos = Vector2Add(p.pos, Vector2Scale(p.vel, dt));
            p.life -= dt;

            bool hit = false;
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
            }
            if (hit || p.life <= 0 || fabs(p.pos.x) > roomW / 2.0f || fabs(p.pos.y) > roomH / 2.0f) {
                projectiles.erase(projectiles.begin() + i);
            } else {
                ++i;
            }
        }

        // ── Loot collection ──
        for (size_t i = 0; i < loots.size();) {
            auto& l = loots[i];
            float d = Vector2Distance(l.pos, player.pos);
            if (d < 160.0f) {
                Vector2 pull = Vector2Normalize(Vector2Subtract(player.pos, l.pos));
                l.pos = Vector2Add(l.pos, Vector2Scale(pull, 420.0f * dt));
            }
            if (d < 24.0f) {
                if (l.type == 0) { // XP
                    player.xp += l.value;
                    Burst(l.pos, SKYBLUE, 3, 100.0f, 0.3f);
                    PlaySound(snd.sndCoin);
                    while (player.xp >= player.xpNext) {
                        player.xp -= player.xpNext;
                        player.level++;
                        player.xpNext = (int)(player.xpNext * 1.45f);
                        TriggerLevelUp();
                    }
                } else if (l.type == 1) { // Gold
                    player.gold += l.value;
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
            if (md < 130.0f && IsKeyPressed(KEY_F)) {
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
        bool anyAlive = false;
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

    void DamageEnemy(Enemy& e, float dmg, Vector2 kb, float kbForce, bool isCrit) {
        if (e.type == CHARGER && e.chargeState == 3) dmg *= 1.5f; // choáng sau cú lao: dễ trúng đòn
        e.hp -= dmg;
        e.pos = Vector2Add(e.pos, Vector2Scale(kb, kbForce * 0.05f));
        SpawnPopup(e.pos, (isCrit ? "!" : "") + std::to_string((int)dmg), isCrit ? YELLOW : WHITE);
        e.hitFlash = 0.09f;
        Burst(e.pos, isCrit ? Color{ 255, 217, 51, 255 } : WHITE, isCrit ? 9 : 4, isCrit ? 230.0f : 160.0f, isCrit ? 0.4f : 0.28f);
        if (isCrit) hitStop = 0.035f;

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
        if (e.type == BOSS) hitStop = 0.12f;
        loots.push_back({ 0, e.pos, (e.type == BOSS ? 150 : (e.isWarden ? 90 : (e.isElite ? 35 : 15))), 20.0f });
        {
            float dropChance = (e.type == BOSS || e.isWarden) ? 1.0f : (e.isElite ? 0.30f : 0.12f);
            if (Rand01() < dropChance) SpawnItemDrop(e.pos);
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
            std::vector<RelicType> pool = { RELIC_LIGHTNING, RELIC_TOXIC, RELIC_FROST, RELIC_AEGIS, RELIC_CHALICE, RELIC_COMBUSTION };
            for (auto r : pool) {
                if (!HasRelic(r)) { AddRelic(r); break; }
            }
        }
    }

    void DamagePlayer(float dmg) {
        if (player.isInvuln || player.hp <= 0) return;

        // Divine Aegis barrier check
        if (HasRelic(RELIC_AEGIS) && player.shieldActive) {
            player.shieldActive = false;
            player.shieldTimer = 0.0f;
            PlaySound(snd.sndShield);
            SpawnPopup(player.pos, "BLOCKED!", GOLD);
            Shake(0.12f, 3.0f);
            return;
        }

        float effDmg = std::max(1.0f, dmg * 100.0f / (100.0f + player.armor * 10.0f));
        player.hp = std::max(0.0f, player.hp - effDmg);
        comboCount = 0;
        damagePulse = std::min(1.0f, 0.45f + effDmg / 60.0f);
        player.invulnTimer = 0.45f;
        PlaySound(snd.sndHurt);
        Shake(0.18f, 5.0f);
        SpawnPopup(player.pos, std::to_string((int)effDmg), RED);

        if (player.hp <= 0) {
            state = GAME_OVER;
            PlaySound(snd.sndDeath);
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
        else if (up == UP_NOVA) { player.specialCooldown = std::max(2.5f, player.specialCooldown - 0.8f); }
        else if (up == UP_IGNITE) { player.igniteLevel++; }
        else if (up == UP_FROST) { player.frostLevel++; }
    }

    // ── Vẽ nhân vật & quái bằng hình học có đổ bóng ──
    static void Tri(Vector2 a, Vector2 b, Vector2 c, Color col) {
        DrawTriangle(a, b, c, col);
        DrawTriangle(a, c, b, col); // vẽ cả hai chiều để không bị culling
    }

    static Color Shade(Color c, float k) { // k < 1: tối hơn, k > 1: sáng hơn
        return { (unsigned char)std::min(255.0f, c.r * k), (unsigned char)std::min(255.0f, c.g * k),
                 (unsigned char)std::min(255.0f, c.b * k), c.a };
    }

    // Thân dạng blob: viền tối, mảng tối ở đáy, thân sáng, vệt highlight
    static void Blob(Vector2 c, float rx, float ry, Color base) {
        DrawEllipse((int)c.x, (int)c.y, rx + 2.5f, ry + 2.5f, Shade(base, 0.25f));
        DrawEllipse((int)c.x, (int)c.y, rx, ry, Shade(base, 0.55f));
        DrawEllipse((int)c.x, (int)(c.y - ry * 0.10f), rx * 0.96f, ry * 0.88f, base);
        DrawEllipse((int)(c.x - rx * 0.30f), (int)(c.y - ry * 0.42f), rx * 0.34f, ry * 0.20f, Fade(Shade(base, 1.7f), 0.75f));
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

        Color base = { 90, 210, 90, 255 };
        if (e.type == RANGER) base = { 228, 72, 88, 255 };
        else if (e.type == BRUTE) base = e.isWarden ? Color{ 214, 156, 62, 255 } : Color{ 146, 92, 206, 255 };
        else if (e.type == BOSS) base = e.enraged ? Color{ 150, 40, 70, 255 } : Color{ 88, 46, 142, 255 };
        else if (e.type == BOMBER) base = { 255, 140, 38, 255 };
        else if (e.type == CHARGER) base = { 205, 120, 66, 255 };

        // Bomber cháy ngòi: phồng lên, nháy đỏ, vòng cảnh báo bán kính nổ
        if (e.type == BOMBER && e.fusing) {
            float f = e.fuseTimer / 0.75f;
            r = e.radius * (1.0f + 0.35f * f);
            bool blink = sinf(f * f * 40.0f) > 0.0f;
            DrawCircle((int)c.x, (int)c.y, 120.0f, Fade({ 255, 60, 30, 255 }, blink ? 0.22f : 0.10f));
            DrawCircleLines((int)c.x, (int)c.y, 120.0f, Fade({ 255, 90, 40, 255 }, blink ? 1.0f : 0.55f));
            if (blink) base = { 255, 60, 50, 255 };
        }
        if (e.chillTimer > 0) base = LerpColor(base, SKYBLUE, 0.6f);
        if (e.hitFlash > 0) base = LerpColor(base, WHITE, 0.8f);

        // Hào quang elite / boss
        if (e.isElite) {
            float k = 0.5f + 0.5f * sinf(t * 5.0f + c.x);
            DrawCircle((int)c.x, (int)c.y, r + 10.0f, Fade(GOLD, 0.07f + 0.07f * k));
            DrawCircleLines((int)c.x, (int)c.y, r + 10.0f, Fade(GOLD, 0.45f + 0.4f * k));
        }
        if (e.type == BOSS) {
            float k = 0.5f + 0.5f * sinf(t * 3.0f);
            Color aura = e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 170, 80, 255, 255 };
            DrawCircle((int)c.x, (int)c.y, r * 1.55f, Fade(aura, 0.07f + 0.05f * k));
            DrawCircle((int)c.x, (int)c.y, r * 1.25f, Fade(aura, 0.10f + 0.05f * k));
            for (int i = 0; i < 4; ++i) {
                float a = t * (e.enraged ? 2.4f : 1.5f) + i * (PI / 2.0f);
                Vector2 o = { c.x + cosf(a) * r * 1.4f, c.y + sinf(a) * r * 1.4f * 0.8f };
                DrawCircleV(o, 9.0f, Fade(aura, 0.25f));
                DrawCircleV(o, 5.5f, { 255, 220, 120, 255 });
                DrawCircleV(o, 2.5f, WHITE);
            }
        }

        if (e.type == CHASER) {
            // Slime nhảy: co giãn theo nhịp nhảy
            float hop = std::max(0.2f, sinf(e.bounceTimer));
            float rx = r * (1.0f + 0.14f * (1.0f - hop));
            float ry = r * (0.80f + 0.22f * hop);
            Vector2 bc = { c.x, c.y + (r - ry) - hop * 5.0f };
            Blob(bc, rx, ry, base);
            DrawCircle((int)(bc.x - rx * 0.55f), (int)(bc.y + ry * 0.92f), 3.0f, Shade(base, 0.75f)); // giọt nhầy
            DrawCircle((int)(bc.x + rx * 0.45f), (int)(bc.y + ry * 0.98f), 2.2f, Shade(base, 0.75f));
            Eye({ bc.x - rx * 0.36f, bc.y - ry * 0.12f }, rx * 0.21f, ry * 0.27f, look, { 30, 120, 40, 255 });
            Eye({ bc.x + rx * 0.36f, bc.y - ry * 0.12f }, rx * 0.21f, ry * 0.27f, look, { 30, 120, 40, 255 });
            DrawEllipse((int)bc.x, (int)(bc.y + ry * 0.38f), rx * 0.20f, ry * 0.09f, { 30, 60, 30, 255 });
        } else if (e.type == RANGER) {
            float charge = std::min(1.0f, e.attackTimer / 2.2f);
            Color dark = Shade(base, 0.5f);
            Tri({ c.x - r * 0.75f, c.y - r * 0.30f }, { c.x - r * 0.30f, c.y - r * 0.85f }, { c.x - r * 1.05f, c.y - r * 1.20f }, dark); // tai nhọn
            Tri({ c.x + r * 0.75f, c.y - r * 0.30f }, { c.x + r * 0.30f, c.y - r * 0.85f }, { c.x + r * 1.05f, c.y - r * 1.20f }, dark);
            Blob(c, r, r * 0.95f, base);
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 ec = { c.x + s2 * r * 0.38f, c.y - r * 0.18f };
                DrawCircleV(ec, r * 0.42f, Fade({ 255, 220, 60, 255 }, 0.22f)); // mắt phát sáng
                Eye(ec, r * 0.22f, r * 0.26f, look, { 255, 190, 40, 255 });
                DrawLineEx({ ec.x - s2 * r * 0.28f, ec.y - r * 0.34f }, { ec.x + s2 * r * 0.18f, ec.y - r * 0.20f }, 3.0f, Shade(base, 0.3f)); // lông mày
            }
            // Miệng mở rộng dần và phát sáng khi sắp bắn
            DrawEllipse((int)c.x, (int)(c.y + r * 0.40f), r * (0.20f + 0.16f * charge), r * (0.10f + 0.18f * charge), { 60, 10, 20, 255 });
            if (charge > 0.55f)
                DrawCircleV({ c.x, c.y + r * 0.40f }, r * 0.14f * charge, Fade({ 255, 170, 50, 255 }, 0.5f + 0.5f * charge));
        } else if (e.type == BRUTE) {
            Color steel = e.isWarden ? Color{ 236, 192, 84, 255 } : Color{ 152, 158, 180, 255 };
            Color horn = e.isWarden ? Color{ 255, 240, 190, 255 } : Color{ 232, 226, 210, 255 };
            float hs = e.isWarden ? 1.35f : 1.0f;
            Tri({ c.x - r * 0.72f, c.y - r * 0.62f }, { c.x - r * 0.40f, c.y - r * 0.92f }, { c.x - r * 1.00f * hs, c.y - r * 1.38f * hs }, horn); // sừng
            Tri({ c.x + r * 0.72f, c.y - r * 0.62f }, { c.x + r * 0.40f, c.y - r * 0.92f }, { c.x + r * 1.00f * hs, c.y - r * 1.38f * hs }, horn);
            Blob(c, r, r * 0.96f, base);
            DrawCircleSector({ c.x, c.y - r * 0.08f }, r * 1.0f, 180.0f, 360.0f, 28, steel);     // mũ giáp
            DrawCircleSectorLines({ c.x, c.y - r * 0.08f }, r * 1.0f, 180.0f, 360.0f, 28, Shade(steel, 0.45f));
            DrawLineEx({ c.x - r, c.y - r * 0.08f }, { c.x + r, c.y - r * 0.08f }, 4.0f, Shade(steel, 0.5f));
            DrawEllipse((int)(c.x - r * 0.35f), (int)(c.y - r * 0.55f), r * 0.2f, r * 0.10f, Fade(WHITE, 0.45f)); // ánh kim loại
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 ec = { c.x + s2 * r * 0.36f, c.y + r * 0.16f };
                Eye(ec, r * 0.17f, r * 0.17f, look, { 255, 90, 60, 255 });
                DrawLineEx({ ec.x - s2 * r * 0.26f, ec.y - r * 0.28f }, { ec.x + s2 * r * 0.14f, ec.y - r * 0.12f }, 3.5f, { 20, 12, 30, 255 }); // lông mày giận dữ
            }
            Tri({ c.x - r * 0.34f, c.y + r * 0.50f }, { c.x - r * 0.14f, c.y + r * 0.50f }, { c.x - r * 0.24f, c.y + r * 0.78f }, horn); // răng nanh
            Tri({ c.x + r * 0.34f, c.y + r * 0.50f }, { c.x + r * 0.14f, c.y + r * 0.50f }, { c.x + r * 0.24f, c.y + r * 0.78f }, horn);
            // Báo hiệu đập đất
            if (e.attackTimer > 2.8f && Vector2Distance(c, player.pos) < 150.0f) {
                float k = (e.attackTimer - 2.8f) / 0.7f;
                DrawCircleLines((int)c.x, (int)c.y, 140.0f * std::min(1.0f, k), Fade({ 200, 120, 255, 255 }, 0.8f));
            }
        } else if (e.type == BOSS) {
            Blob(c, r, r * 0.98f, base);
            for (int i = -2; i <= 2; ++i) // vết nứt phát sáng
                DrawLineEx({ c.x + i * r * 0.28f, c.y + r * 0.2f }, { c.x + i * r * 0.40f, c.y + r * 0.75f }, 2.0f, Fade({ 220, 140, 255, 255 }, 0.6f));
            Color eyeC = e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 255, 210, 80, 255 };
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 ec = { c.x + s2 * r * 0.38f, c.y - r * 0.12f };
                DrawCircleV(ec, r * 0.30f, Fade(eyeC, 0.25f));
                Eye(ec, r * 0.18f, r * 0.22f, look, eyeC);
                DrawLineEx({ ec.x - s2 * r * 0.30f, ec.y - r * 0.32f }, { ec.x + s2 * r * 0.16f, ec.y - r * 0.16f }, 4.0f, { 20, 10, 30, 255 });
            }
            for (int i = 0; i < 6; ++i) { // miệng răng cưa
                float x0 = c.x - r * 0.42f + i * r * 0.168f;
                Tri({ x0, c.y + r * 0.42f }, { x0 + r * 0.168f, c.y + r * 0.42f }, { x0 + r * 0.084f, c.y + r * 0.62f }, WHITE);
            }
            // Vương miện
            float cx = c.x, cy = c.y - r * 0.98f - 2.0f;
            DrawRectangle((int)cx - 16, (int)cy - 7, 32, 8, GOLD);
            Tri({ cx - 16, cy - 7 }, { cx - 10, cy - 7 }, { cx - 16, cy - 20 }, GOLD);
            Tri({ cx - 7, cy - 7 }, { cx + 7, cy - 7 }, { cx, cy - 26 }, GOLD);
            Tri({ cx + 10, cy - 7 }, { cx + 16, cy - 7 }, { cx + 16, cy - 20 }, GOLD);
            DrawCircle((int)cx, (int)cy - 3, 2.5f, RED);
        } else if (e.type == CHARGER) {
            // Báo hiệu hướng lao: vệt đỏ nhấp nháy
            if (e.chargeState == 1) {
                float k = 0.5f + 0.5f * sinf(t * 30.0f);
                Vector2 end = { c.x + e.chargeDir.x * 460.0f, c.y + e.chargeDir.y * 460.0f };
                DrawLineEx(c, end, r * 1.9f, Fade({ 255, 60, 40, 255 }, 0.10f + 0.10f * k));
                DrawLineEx(c, end, 3.0f, Fade({ 255, 90, 60, 255 }, 0.6f + 0.4f * k));
                OutlinedText("!", (int)c.x - 5, (int)(c.y - r - 34.0f), 30, { 255, 220, 60, 255 }, { 80, 10, 10, 255 });
            }
            Vector2 fwd = (e.chargeState >= 1 && e.chargeState <= 2) ? e.chargeDir : look;
            float sq = (e.chargeState == 2) ? 1.18f : (e.chargeState == 1 ? 0.92f : 1.0f);
            if (e.chargeState == 2) // vệt tốc độ phía sau
                for (int i = 1; i <= 4; ++i)
                    DrawCircleV(Vector2Subtract(c, Vector2Scale(fwd, i * r * 0.7f)), r * (1.0f - i * 0.18f), Fade(base, 0.22f - i * 0.04f));
            Vector2 bc = { c.x + (e.chargeState == 1 ? sinf(t * 60.0f) * 2.0f : 0.0f), c.y };
            Blob(bc, r * sq, r * (2.0f - sq) * 0.95f, base);
            // Sừng / ngà hướng về phía trước
            Vector2 perp = { -fwd.y, fwd.x };
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 b0 = { bc.x + fwd.x * r * 0.55f + perp.x * s2 * r * 0.45f, bc.y + fwd.y * r * 0.55f + perp.y * s2 * r * 0.45f };
                Vector2 b1 = { b0.x + perp.x * s2 * r * 0.12f, b0.y + perp.y * s2 * r * 0.12f };
                Vector2 tip2 = { b0.x + fwd.x * r * 0.75f + perp.x * s2 * r * 0.28f, b0.y + fwd.y * r * 0.75f + perp.y * s2 * r * 0.28f };
                Tri({ b0.x - perp.x * s2 * 3.0f, b0.y - perp.y * s2 * 3.0f }, { b1.x + perp.x * s2 * 3.0f, b1.y + perp.y * s2 * 3.0f }, tip2, { 240, 232, 210, 255 });
            }
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 ec = { c.x + s2 * r * 0.36f, c.y - r * 0.12f };
                Eye(ec, r * 0.19f, r * 0.22f, (e.chargeState == 3) ? Vector2{ 0, 0 } : fwd, e.chargeState == 3 ? Color{ 200, 200, 80, 255 } : Color{ 255, 70, 40, 255 });
                DrawLineEx({ ec.x - s2 * r * 0.30f, ec.y - r * 0.32f }, { ec.x + s2 * r * 0.16f, ec.y - r * 0.14f }, 3.0f, { 60, 20, 10, 255 });
            }
            // Choáng: sao quay quanh đầu
            if (e.chargeState == 3)
                for (int i = 0; i < 3; ++i) {
                    float a3 = t * 6.0f + i * 2.094f;
                    DrawCircleV({ c.x + cosf(a3) * r * 0.8f, c.y - r - 8.0f + sinf(a3) * 5.0f }, 4.0f, { 255, 230, 80, 255 });
                }
        } else if (e.type == BOMBER) {
            Blob(c, r, r * 0.95f, base);
            for (int s2 = -1; s2 <= 1; s2 += 2) {
                Vector2 ec = { c.x + s2 * r * 0.36f, c.y - r * 0.08f };
                Eye(ec, r * 0.20f, r * 0.24f, look, { 120, 40, 20, 255 });
                DrawLineEx({ ec.x - s2 * r * 0.30f, ec.y - r * 0.34f }, { ec.x + s2 * r * 0.14f, ec.y - r * 0.16f }, 3.0f, { 70, 20, 10, 255 });
            }
            DrawLineEx({ c.x - r * 0.40f, c.y + r * 0.42f }, { c.x + r * 0.40f, c.y + r * 0.42f }, 3.0f, { 70, 20, 10, 255 }); // miệng
            for (int i = 0; i < 4; ++i)
                DrawRectangle((int)(c.x - r * 0.36f + i * r * 0.2f), (int)(c.y + r * 0.42f), 4, 5, WHITE);
            Vector2 top = { c.x, c.y - r * 0.95f };
            Vector2 tip = { c.x + 6.0f, c.y - r * 0.95f - 12.0f };
            DrawLineEx(top, tip, 3.5f, { 110, 70, 40, 255 });
            float tw = 0.6f + 0.4f * sinf(t * 25.0f);
            DrawCircleV(tip, 6.0f * tw + 2.0f, Fade({ 255, 200, 60, 255 }, 0.4f));
            DrawCircleV(tip, 3.2f * tw + 1.2f, { 255, 235, 90, 255 });
            DrawCircleV(tip, 1.6f, WHITE);
        }

        // Viền trạng thái
        if (e.slowTimer > 0) {
            DrawCircleLines((int)c.x, (int)c.y, r + 4.0f, { 120, 200, 255, 255 });
            DrawCircleLines((int)c.x, (int)c.y, r + 5.0f, { 120, 200, 255, 160 });
        }
        if (e.burnTimer > 0) {
            float fl = 0.6f + 0.4f * sinf(t * 30.0f + c.x);
            DrawCircleLines((int)c.x, (int)c.y, r + 7.0f, Fade({ 255, 130, 30, 255 }, fl));
            DrawCircleV({ c.x - r * 0.5f, c.y - r - 4.0f }, 3.0f + 2.0f * fl, Fade({ 255, 170, 40, 255 }, 0.9f));
        }
    }

    void DrawPlayerSprite() {
        const float t = (float)GetTime();
        const Vector2 p = player.pos;
        const float face = player.facingLeft ? -1.0f : 1.0f;
        const float back = -face;
        const bool moving = player.walkTimer > 0.01f && (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)
                              || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT));
        float bob = moving ? fabsf(sinf(player.walkTimer)) * -2.5f : sinf(t * 3.0f) * 1.2f;
        float sway = sinf(t * 8.0f) * 2.5f + (moving ? 4.0f : 0.0f);
        Vector2 body = { p.x, p.y + bob };

        // Áo choàng (sau lưng)
        Color cape = { 44, 74, 168, 255 };
        Vector2 A = { body.x + back * 6.0f, body.y - 11.0f }, B = { body.x + back * 6.0f, body.y + 11.0f };
        Vector2 C = { body.x + back * (31.0f + sway), body.y + 20.0f + sway * 0.4f }, D = { body.x + back * (31.0f + sway), body.y - 4.0f };
        Tri(A, B, C, cape); Tri(A, C, D, cape);
        DrawLineEx(A, D, 2.0f, Shade(cape, 0.55f));
        DrawLineEx(B, C, 2.0f, Shade(cape, 0.55f));

        // Ủng bước chân
        float step = moving ? sinf(player.walkTimer) * 4.0f : 0.0f;
        DrawEllipse((int)(p.x - 7.0f), (int)(p.y + 16.0f + step * 0.5f), 6.0f, 4.0f, { 52, 46, 64, 255 });
        DrawEllipse((int)(p.x + 7.0f), (int)(p.y + 16.0f - step * 0.5f), 6.0f, 4.0f, { 52, 46, 64, 255 });

        // Lông vũ mũ
        Tri({ body.x, body.y - 16.0f }, { body.x + back * (30.0f + sway * 0.5f), body.y - 26.0f + sway * 0.3f }, { body.x + back * 9.0f, body.y - 8.0f }, { 224, 52, 66, 255 });
        DrawLineEx({ body.x, body.y - 16.0f }, { body.x + back * (30.0f + sway * 0.5f), body.y - 26.0f + sway * 0.3f }, 2.0f, { 140, 24, 36, 255 });

        // Thân giáp
        Blob(body, 17.0f, 17.5f, { 222, 228, 244, 255 });
        // Mũ giáp nửa trên
        DrawCircleSector({ body.x, body.y - 1.0f }, 17.0f, 180.0f, 360.0f, 28, { 176, 188, 214, 255 });
        DrawLineEx({ body.x - 17.0f, body.y - 1.0f }, { body.x + 17.0f, body.y - 1.0f }, 2.0f, { 98, 110, 140, 255 });
        DrawEllipse((int)(body.x - 6.0f), (int)(body.y - 11.0f), 5.0f, 2.2f, Fade(WHITE, 0.7f));
        // Kính ngắm phát sáng hướng về phía nhìn
        DrawEllipse((int)(body.x + face * 5.5f), (int)(body.y + 2.0f), 9.0f, 3.6f, { 18, 24, 48, 255 });
        DrawEllipse((int)(body.x + face * 8.0f), (int)(body.y + 2.0f), 4.5f, 1.8f, { 90, 230, 255, 255 });
        DrawCircleV({ body.x + face * 3.0f, body.y + 2.0f }, 4.0f, Fade({ 90, 230, 255, 255 }, 0.18f));

        // Kiếm vung theo hướng ngắm
        float a = atan2f(aimDirDraw.y, aimDirDraw.x);
        float sinceAtk = (1.0f / player.attackRate) - player.attackTimer;
        float off = (player.attackTimer > 0.0f && sinceAtk < 0.16f) ? Lerp(-1.25f, 1.25f, sinceAtk / 0.16f) : 0.5f;
        float sa = a + off;
        Vector2 dir = { cosf(sa), sinf(sa) };
        Vector2 hand = { body.x + dir.x * 16.0f, body.y + dir.y * 16.0f };
        Vector2 tip = { hand.x + dir.x * 27.0f, hand.y + dir.y * 27.0f };
        DrawLineEx(hand, tip, 6.0f, { 90, 100, 130, 255 });
        DrawLineEx(hand, tip, 4.0f, { 232, 238, 250, 255 });
        DrawLineEx(hand, tip, 1.5f, WHITE);
        Vector2 perp = { -dir.y, dir.x };
        DrawLineEx({ hand.x + perp.x * 7.0f, hand.y + perp.y * 7.0f }, { hand.x - perp.x * 7.0f, hand.y - perp.y * 7.0f }, 4.0f, GOLD);
        DrawCircleV(hand, 4.2f, { 140, 150, 176, 255 });
        DrawCircleV(hand, 4.2f, Fade(WHITE, 0.0f));

        // Nhấp nháy khi bất tử tạm thời
        if (player.isInvuln) {
            float k = 0.5f + 0.5f * sinf(t * 30.0f);
            DrawCircleLines((int)body.x, (int)body.y, 24.0f, Fade(SKYBLUE, 0.4f + 0.5f * k));
        }
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
                if (c.vault) { // kho báu: chắc chắn có trang bị hiếm trở lên
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
            DrawCircleGradient((int)p.x, (int)p.y, 60.0f * s, Fade(band, 0.22f + 0.08f * sinf(t * 3.0f)), Fade(band, 0.0f));
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
            DrawCircleGradient((int)p.x, (int)(p.y - 4), 34.0f * s * (1.0f - c.anim * 0.5f), Fade({ 255, 230, 120, 255 }, 0.5f * (1.0f - c.anim)), Fade({ 255, 230, 120, 255 }, 0.0f));
            EndBlendMode();
        }
    }

    // ── Cửa hàng ──
    void StockShop() {
        for (int i = 0; i < 4; ++i) {
            shopSlots[i] = ShopSlot();
            shopSlots[i].item = GenerateItem(floor);
            shopSlots[i].price = ItemPrice(shopSlots[i].item);
        }
        shopSlots[4] = ShopSlot();
        shopSlots[4].potion = true;
        shopSlots[4].heal = 40.0f + floor * 5.0f;
        shopSlots[4].price = 15 + floor * 3;
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
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_F) || IsKeyPressed(KEY_BACKSPACE)) { state = PLAYING; return; }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) shopSel = (shopSel + 1) % 5;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) shopSel = (shopSel + 4) % 5;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) BuyShopSlot(shopSel);
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
                DrawItemIcon(s.item.slot, ic, 54.0f, rc);
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
        DrawEllipse((int)p.x, (int)p.y + 26, 34.0f, 10.0f, Fade(BLACK, 0.4f));
        // Bàn hàng nhỏ với đống vàng
        DrawRectangle((int)p.x + 34, (int)p.y + 4, 46, 26, { 120, 78, 44, 255 });
        DrawRectangle((int)p.x + 34, (int)p.y + 4, 46, 5, { 160, 108, 60, 255 });
        for (int i = 0; i < 5; ++i) DrawCircleV({ p.x + 42.0f + i * 8.0f, p.y + 2.0f - (i % 2) * 3.0f }, 5.0f, GOLD);
        // Áo choàng + mũ trùm
        Tri({ p.x - 28, p.y + 24 + bob }, { p.x + 28, p.y + 24 + bob }, { p.x, p.y - 28 + bob }, { 92, 52, 150, 255 });
        Tri({ p.x - 20, p.y + 22 + bob }, { p.x + 20, p.y + 22 + bob }, { p.x, p.y - 22 + bob }, { 122, 74, 190, 255 });
        DrawCircleV({ p.x, p.y - 24.0f + bob }, 17.0f, { 70, 38, 120, 255 });
        DrawCircleV({ p.x, p.y - 22.0f + bob }, 13.0f, { 20, 12, 36, 255 });
        DrawCircleV({ p.x - 5.0f, p.y - 22.0f + bob }, 2.5f, { 255, 220, 90, 255 });
        DrawCircleV({ p.x + 5.0f, p.y - 22.0f + bob }, 2.5f, { 255, 220, 90, 255 });
        // Đèn lồng
        Vector2 lan = { p.x - 44.0f + sinf(t * 2.5f) * 3.0f, p.y - 2.0f };
        DrawLineEx({ p.x - 30.0f, p.y - 12.0f + bob }, { lan.x, lan.y - 8.0f }, 2.0f, { 90, 70, 50, 255 });
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient((int)lan.x, (int)lan.y, 46.0f, Fade({ 255, 190, 80, 255 }, 0.4f), Fade({ 255, 190, 80, 255 }, 0.0f));
        EndBlendMode();
        DrawRectangle((int)lan.x - 5, (int)lan.y - 6, 10, 13, { 255, 214, 110, 255 });
        DrawRectangleLines((int)lan.x - 5, (int)lan.y - 6, 10, 13, { 90, 60, 30, 255 });
        // Biển hiệu nổi
        OutlinedText("SHOP", (int)p.x - MeasureText("SHOP", 20) / 2, (int)(p.y - 66.0f + bob), 20, GOLD, { 70, 36, 8, 255 });
        if (Vector2Distance(p, player.pos) < 130.0f) {
            const char* prompt = "[F] Trade";
            OutlinedText(prompt, (int)p.x - MeasureText(prompt, 20) / 2, (int)p.y + 40, 20, WHITE, { 20, 24, 40, 255 });
        }
    }

    // ── Cài đặt ──
    void LoadSettings() {
        if (!FileExists("settings.cfg")) return;
        char* txt = LoadFileText("settings.cfg");
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
            else if (k == "fullscreen") settings.fullscreen = v != 0;
            else if (k == "vsync") settings.vsync = v != 0;
            else if (k == "fpsIndex") settings.fpsIndex = std::max(0, std::min(4, v));
            else if (k == "showFps") settings.showFps = v != 0;
            else if (k == "vibration") settings.vibration = v != 0;
            else if (k == "lighting") settings.lighting = v != 0;
            else if (k == "particles") settings.particles = std::max(0, std::min(2, v));
            else if (k == "vignette") settings.vignette = v != 0;
            else if (k == "volume") settings.volume = std::max(0, std::min(10, v));
            else if (k == "best") bestFloor = std::max(1, v);
        }
    }

    void SaveSettings() {
        std::ostringstream o;
        o << "resW=" << settings.resW << "\nresH=" << settings.resH << "\nfullscreen=" << settings.fullscreen
          << "\nvsync=" << settings.vsync << "\nfpsIndex=" << settings.fpsIndex << "\nshowFps=" << settings.showFps
          << "\nvibration=" << settings.vibration << "\nlighting=" << settings.lighting << "\nparticles=" << settings.particles
          << "\nvignette=" << settings.vignette << "\nvolume=" << settings.volume << "\nbest=" << bestFloor << "\n";
        std::string s = o.str();
        SaveFileText("settings.cfg", (char*)s.c_str());
    }

    void BuildResList() {
        resList.clear();
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
        if (settings.fullscreen != IsWindowFullscreen()) ToggleFullscreen();
        if (!settings.fullscreen) SetWindowSize(settings.resW, settings.resH);
        if (settings.vsync) SetWindowState(FLAG_VSYNC_HINT); else ClearWindowState(FLAG_VSYNC_HINT);
        SetTargetFPS(kFpsOptions[settings.fpsIndex]);
        SetMasterVolume(settings.volume / 10.0f);
    }

    void ChangeSetting(int row, int d) {
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
            case 1: settings.fullscreen = !settings.fullscreen; break;
            case 2: settings.vsync = !settings.vsync; break;
            case 3: settings.fpsIndex = (settings.fpsIndex + d + 5) % 5; break;
            case 4: settings.showFps = !settings.showFps; break;
            case 5: settings.vibration = !settings.vibration; break;
            case 6: settings.lighting = !settings.lighting; break;
            case 7: settings.particles = (settings.particles + d + 3) % 3; break;
            case 8: settings.vignette = !settings.vignette; break;
            case 9: settings.volume = std::max(0, std::min(10, settings.volume + d)); break;
        }
    }

    std::string SettingValue(int row) {
        switch (row) {
            case 0: return settings.fullscreen ? "Native (fullscreen)" : std::to_string(settings.resW) + " x " + std::to_string(settings.resH);
            case 1: return settings.fullscreen ? "ON" : "OFF";
            case 2: return settings.vsync ? "ON" : "OFF";
            case 3: return kFpsOptions[settings.fpsIndex] == 0 ? "Unlimited" : std::to_string(kFpsOptions[settings.fpsIndex]);
            case 4: return settings.showFps ? "ON" : "OFF";
            case 5: return settings.vibration ? "ON" : "OFF";
            case 6: return settings.lighting ? "ON" : "OFF";
            case 7: return settings.particles == 0 ? "Low" : (settings.particles == 1 ? "Medium" : "High");
            case 8: return settings.vignette ? "ON" : "OFF";
            case 9: return std::to_string(settings.volume * 10) + "%";
        }
        return "";
    }

    void UpdateSettingsMenu() {
        const int rows = 11; // 10 mục + Back
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) setSel = (setSel + 1) % rows;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) setSel = (setSel + rows - 1) % rows;
        int delta = 0;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) delta = 1;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) delta = -1;

        Vector2 m = GetMousePosition();
        for (int i = 0; i < rows; ++i) {
            Rectangle r = { 330.0f, 150.0f + i * 40.0f, 620.0f, 36.0f };
            if (CheckCollisionPointRec(m, r)) {
                if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) setSel = i;
                if (setSel == i) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) delta = 1;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) delta = -1;
                }
            }
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) { state = settingsReturn; return; }
        if (delta != 0) {
            if (setSel == 10) { state = settingsReturn; return; }
            ChangeSetting(setSel, delta);
            SaveSettings();
            ApplySettings();
        }
    }

    void DrawSettingsScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.86f));
        const char* title = "SETTINGS";
        OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 70, 48, GOLD, { 90, 50, 10, 255 });
        static const char* labels[10] = { "Resolution", "Fullscreen", "VSync", "FPS Limit", "Show FPS", "Vibration (screen shake)", "Lighting", "Particles", "Vignette", "Volume" };
        for (int i = 0; i < 11; ++i) {
            Rectangle r = { 330.0f, 150.0f + i * 40.0f, 620.0f, 36.0f };
            bool sel = (setSel == i);
            DrawRectangleRec(r, sel ? Color{ 40, 52, 84, 255 } : Color{ 20, 26, 42, 255 });
            if (sel) DrawRectangleLinesEx(r, 2.0f, GOLD);
            if (i < 10) {
                DrawText(labels[i], (int)r.x + 16, (int)r.y + 8, 20, sel ? WHITE : Color{ 190, 205, 235, 255 });
                std::string v = SettingValue(i);
                Color vc = (v == "ON") ? Color{ 120, 235, 140, 255 } : (v == "OFF" ? Color{ 255, 130, 130, 255 } : Color{ 255, 215, 90, 255 });
                int vw = MeasureText(v.c_str(), 20);
                DrawText(v.c_str(), (int)(r.x + r.width) - 40 - vw, (int)r.y + 8, 20, vc);
                if (sel) {
                    DrawText("<", (int)(r.x + r.width) - 40 - vw - 22, (int)r.y + 8, 20, GOLD);
                    DrawText(">", (int)(r.x + r.width) - 28, (int)r.y + 8, 20, GOLD);
                }
                if (i == 9) { // thanh âm lượng
                    for (int b = 0; b < 10; ++b)
                        DrawRectangle((int)r.x + 220 + b * 12, (int)r.y + 11, 9, 14, b < settings.volume ? Color{ 255, 215, 90, 255 } : Color{ 50, 58, 84, 255 });
                }
            } else {
                const char* back = "Back";
                DrawText(back, (int)(r.x + r.width / 2) - MeasureText(back, 22) / 2, (int)r.y + 7, 22, sel ? WHITE : Color{ 190, 205, 235, 255 });
            }
        }
        DrawText("W/S or mouse: select     A/D, Enter or click: change     Esc: back", 1280 / 2 - MeasureText("W/S or mouse: select     A/D, Enter or click: change     Esc: back", 16) / 2, 612, 16, { 150, 165, 200, 255 });
        DrawText("Resolution applies in windowed mode; fullscreen uses your desktop resolution. Settings are saved automatically.", 1280 / 2 - MeasureText("Resolution applies in windowed mode; fullscreen uses your desktop resolution. Settings are saved automatically.", 14) / 2, 638, 14, { 120, 135, 165, 255 });
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

    static void DrawItemIcon(int slot, Vector2 c, float s, Color col) {
        Color dark = Shade(col, 0.45f);
        switch (slot) {
            case SLOT_WEAPON: {
                Vector2 a = { c.x - s * 0.42f, c.y + s * 0.42f }, b = { c.x + s * 0.46f, c.y - s * 0.46f };
                DrawLineEx(a, b, s * 0.20f, dark);
                DrawLineEx(a, b, s * 0.12f, col);
                DrawLineEx({ c.x - s * 0.12f, c.y - s * 0.12f - 0.0f }, { c.x + s * 0.05f, c.y + s * 0.05f }, 1.0f, WHITE);
                Vector2 g = { c.x - s * 0.18f, c.y + s * 0.18f };
                DrawLineEx({ g.x - s * 0.2f, g.y - s * 0.2f }, { g.x + s * 0.2f, g.y + s * 0.2f }, s * 0.12f, GOLD);
                DrawCircleV({ c.x - s * 0.48f, c.y + s * 0.48f }, s * 0.10f, GOLD);
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
            DrawCircleGradient((int)d.pos.x, (int)(d.pos.y - 6), 46.0f, Fade(rc, 0.45f), Fade(rc, 0.0f));
            EndBlendMode();
            if (d.item.rarity >= 2)
                DrawRectangleGradientV((int)d.pos.x - 5, (int)d.pos.y - 100, 10, 100, Fade(rc, 0.0f), Fade(rc, d.item.rarity == 3 ? 0.45f : 0.28f));
            DrawCircleV({ d.pos.x, d.pos.y - 8.0f + bob }, 17.0f, { 18, 22, 34, 230 });
            DrawCircleLines((int)d.pos.x, (int)(d.pos.y - 8.0f + bob), 17.0f, rc);
            DrawItemIcon(d.item.slot, { d.pos.x, d.pos.y - 8.0f + bob }, 22.0f, rc);
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
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.82f));
        int px = 90, py = 70, pw = 1100, ph = 590;
        DrawRectangle(px, py, pw, ph, { 14, 18, 30, 248 });
        DrawRectangleLines(px, py, pw, ph, GOLD);
        const char* title = "EQUIPMENT";
        DrawText(title, 1280 / 2 - MeasureText(title, 34) / 2, py + 16, 34, GOLD);

        static const char* slotNames[4] = { "WEAPON", "ARMOR", "BOOTS", "RING" };
        int cw = 250, gap = 14, cx0 = px + (pw - (cw * 4 + gap * 3)) / 2, cy = py + 70, ch = 250;
        for (int i = 0; i < 4; ++i) {
            int cx = cx0 + i * (cw + gap);
            Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 70, 80, 105, 255 };
            DrawRectangle(cx, cy, cw, ch, { 22, 28, 44, 255 });
            DrawRectangleLines(cx, cy, cw, ch, rc);
            DrawRectangle(cx, cy, cw, 28, Fade(rc, 0.25f));
            DrawText(slotNames[i], cx + 12, cy + 6, 18, rc);
            if (hasEquip[i]) {
                DrawCircleV({ (float)cx + cw / 2.0f, (float)cy + 76 }, 36.0f, Fade(rc, 0.18f));
                DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 76 }, 58.0f, rc);
                DrawText(equipped[i].name.c_str(), cx + 12, cy + 124, 18, rc);
                DrawText(equipped[i].RarityName(), cx + 12, cy + 146, 14, Fade(rc, 0.8f));
                auto lines = equipped[i].StatLines();
                for (size_t k = 0; k < lines.size(); ++k) DrawText(lines[k].c_str(), cx + 12, cy + 170 + (int)k * 20, 16, RAYWHITE);
            } else {
                DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 76 }, 58.0f, { 60, 68, 90, 255 });
                DrawText("- Empty -", cx + cw / 2 - MeasureText("- Empty -", 18) / 2, cy + 140, 18, { 110, 120, 145, 255 });
            }
        }

        // Chỉ số nhân vật
        int sy = cy + ch + 24;
        DrawText("CHARACTER", px + 36, sy, 22, SKYBLUE);
        char buf[96];
        auto stat = [&](int col, int row, const char* label, const std::string& val) {
            DrawText(label, px + 36 + col * 330, sy + 36 + row * 28, 18, { 170, 185, 215, 255 });
            DrawText(val.c_str(), px + 36 + col * 330 + 160, sy + 36 + row * 28, 18, WHITE);
        };
        snprintf(buf, sizeof(buf), "%d / %d", (int)player.hp, (int)player.maxHp); stat(0, 0, "Health", buf);
        snprintf(buf, sizeof(buf), "%.0f", player.baseDmg); stat(0, 1, "Damage", buf);
        snprintf(buf, sizeof(buf), "%.1f", player.armor); stat(0, 2, "Armor", buf);
        snprintf(buf, sizeof(buf), "%.0f", player.speed); stat(1, 0, "Move Speed", buf);
        snprintf(buf, sizeof(buf), "%.2f /s", player.attackRate); stat(1, 1, "Attack Speed", buf);
        snprintf(buf, sizeof(buf), "%d%%", (int)roundf(std::min(0.6f, player.critChance) * 100)); stat(1, 2, "Crit Chance", buf);
        snprintf(buf, sizeof(buf), "%d%%", (int)roundf(std::min(0.3f, player.lifesteal) * 100)); stat(2, 0, "Lifesteal", buf);
        snprintf(buf, sizeof(buf), "%.1fs", player.dashCooldown); stat(2, 1, "Dash Cooldown", buf);
        snprintf(buf, sizeof(buf), "%.1fs", player.specialCooldown); stat(2, 2, "Nova Cooldown", buf);
        std::string extra = "Ember Edge Lv" + std::to_string(player.igniteLevel) + "   Frost Touch Lv" + std::to_string(player.frostLevel) + "   Relics: " + std::to_string(player.relics.size());
        DrawText(extra.c_str(), px + 36 + 780 - MeasureText(extra.c_str(), 16) , sy + 2, 16, { 150, 165, 200, 255 });
        DrawText("Gear drops from enemies. Better gear is equipped automatically; worse gear is sold for gold.", px + 36, py + ph - 52, 16, { 170, 185, 215, 255 });
        DrawText("Press I or Esc to close", px + 36, py + ph - 28, 16, { 120, 135, 165, 255 });
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
        Color stone = Shade(biome.wall, 1.15f);
        DrawEllipse((int)p.x + 3, (int)p.y + 12, 31.0f, 14.0f, Fade(BLACK, 0.38f));
        DrawCircleV(p, 29.0f, Shade(stone, 0.35f));
        DrawCircleV(p, 27.0f, Shade(stone, 0.70f));
        DrawCircleV({ p.x - 2.0f, p.y - 3.0f }, 23.0f, stone);
        DrawCircleV({ p.x - 3.0f, p.y - 5.0f }, 15.0f, Shade(stone, 1.20f));
        DrawRing(p, 19.0f, 21.0f, 200.0f, 270.0f, 10, Fade(WHITE, 0.35f));
        DrawLineEx({ p.x + 4.0f, p.y - 8.0f }, { p.x + 10.0f, p.y + 2.0f }, 2.0f, Fade(BLACK, 0.35f));
        DrawLineEx({ p.x + 10.0f, p.y + 2.0f }, { p.x + 6.0f, p.y + 12.0f }, 2.0f, Fade(BLACK, 0.35f));
        if (bi == 1) DrawCircleV({ p.x + 13.0f, p.y + 10.0f }, 7.0f, Fade({ 90, 170, 80, 255 }, 0.55f));
        if (bi == 2) DrawLineEx({ p.x - 9.0f, p.y + 6.0f }, { p.x + 2.0f, p.y + 15.0f }, 2.5f, { 255, 130, 40, 255 });
        if (bi == 3) DrawCircleSector({ p.x - 3.0f, p.y - 5.0f }, 14.0f, 180.0f, 360.0f, 14, Fade({ 235, 248, 255, 255 }, 0.6f));
    }

    void DrawTorch(Vector2 t) {
        float tm = (float)GetTime();
        float fl = 0.78f + 0.22f * sinf(tm * 13.0f + t.x * 0.1f) + 0.08f * sinf(tm * 29.0f + t.y);
        DrawRectangle((int)t.x - 3, (int)t.y - 2, 6, 15, { 70, 50, 36, 255 });
        DrawRectangle((int)t.x - 6, (int)t.y - 4, 12, 5, { 120, 88, 54, 255 });
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient((int)t.x, (int)t.y - 8, 58.0f, Fade({ 255, 150, 50, 255 }, 0.32f * fl), Fade({ 255, 150, 50, 255 }, 0.0f));
        EndBlendMode();
        DrawCircleV({ t.x, t.y - 8.0f }, 7.5f * fl, { 255, 120, 30, 255 });
        DrawCircleV({ t.x, t.y - 9.0f }, 4.8f * fl, { 255, 200, 70, 255 });
        DrawCircleV({ t.x, t.y - 10.0f }, 2.2f, { 255, 250, 205, 255 });
    }

    void AddLight(Vector2 wp, float radius, Color col, float inten) {
        Vector2 sp = GetWorldToScreen2D(wp, camera);
        if (sp.x < -radius || sp.x > 1280 + radius || sp.y < -radius || sp.y > 720 + radius) return;
        DrawCircleGradient((int)sp.x, (int)sp.y, radius, Fade(col, inten), Fade(col, 0.0f));
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
        for (const auto& pr : projectiles) AddLight(pr.pos, 80.0f, pr.color, 0.7f);
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
            else if (e.type == BOSS) AddLight(e.pos, 260.0f, e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 170, 90, 255, 255 }, 0.5f);
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

    // ── Màn hình tiêu đề / tạm dừng / hướng dẫn ──
    static void OutlinedText(const char* txt, int x, int y, int size, Color col, Color outline) {
        for (int ox = -2; ox <= 2; ox += 2)
            for (int oy = -2; oy <= 2; oy += 2)
                if (ox != 0 || oy != 0) DrawText(txt, x + ox, y + oy, size, outline);
        DrawText(txt, x, y, size, col);
    }

    void DrawHelpPanel() {
        int w = 760, h = 524, x = 1280 / 2 - w / 2, y = 100;
        DrawRectangle(x, y, w, h, { 14, 18, 30, 245 });
        DrawRectangleLines(x, y, w, h, GOLD);
        DrawText("CONTROLS", x + w / 2 - MeasureText("CONTROLS", 30) / 2, y + 18, 30, GOLD);
        const char* rows[][2] = {
            { "W A S D / Arrows", "Move" },
            { "Mouse", "Aim your sword" },
            { "Left Click / Space / J", "Slash (hold to keep attacking)" },
            { "Right Click / Shift / K", "Dash (brief invulnerability)" },
            { "Q / E / U", "Nova shockwave" },
            { "1 / 2 / 3", "Pick an upgrade on level-up" },
            { "I", "Open the equipment screen" },
            { "F", "Trade with the merchant (every 3rd floor)" },
            { "O / F2", "Settings (title and pause menu)" },
            { "M", "Toggle the radar" },
            { "Esc / P", "Pause" },
            { "F1 / H", "Show or hide this help" },
        };
        for (int i = 0; i < 12; ++i) {
            DrawText(rows[i][0], x + 36, y + 76 + i * 34, 20, SKYBLUE);
            DrawText(rows[i][1], x + 340, y + 76 + i * 34, 20, RAYWHITE);
        }
        DrawText("Clear a room, enter the portal. Blood Shrines trade HP for upgrades.", x + 36, y + h - 40, 16, { 190, 200, 220, 255 });
    }

    void DrawTitleScreen() {
        float t = (float)GetTime();
        DrawRectangleGradientV(0, 0, 1280, 720, Fade(BLACK, 0.86f), Fade(BLACK, 0.70f));
        if (showHelp) { DrawHelpPanel(); return; }
        const char* title = "GEOMETRIC DUNGEON";
        int tw = MeasureText(title, 72);
        int ty = 120 + (int)(sinf(t * 1.6f) * 5.0f);
        DrawText(title, 1280 / 2 - tw / 2 + 4, ty + 5, 72, Fade(BLACK, 0.8f));
        OutlinedText(title, 1280 / 2 - tw / 2, ty, 72, GOLD, { 120, 60, 10, 255 });
        const char* sub = "a tiny roguelike RPG";
        DrawText(sub, 1280 / 2 - MeasureText(sub, 24) / 2, ty + 88, 24, { 170, 190, 230, 255 });

        std::string rec = "Best Record: Floor " + std::to_string(bestFloor);
        DrawText(rec.c_str(), 1280 / 2 - MeasureText(rec.c_str(), 22) / 2, 330, 22, SKYBLUE);

        if (((int)(t * 2.0f)) % 2 == 0) {
            const char* start = "PRESS ENTER OR CLICK TO START";
            OutlinedText(start, 1280 / 2 - MeasureText(start, 30) / 2, 410, 30, WHITE, { 20, 24, 40, 255 });
        }
        const char* hint = "O - Settings      F1 - Controls      Esc - Quit";
        DrawText(hint, 1280 / 2 - MeasureText(hint, 20) / 2, 470, 20, { 150, 165, 200, 255 });
        if (showHelp) DrawHelpPanel();
    }

    void DrawPauseScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.72f));
        const char* t1 = "PAUSED";
        OutlinedText(t1, 1280 / 2 - MeasureText(t1, 60) / 2, 190, 60, WHITE, { 20, 24, 40, 255 });
        const char* opts[] = { "ESC / P / ENTER   Resume", "I   Equipment", "O   Settings", "R   Restart run", "F1   Controls", "Q   Quit game" };
        for (int i = 0; i < 6; ++i)
            DrawText(opts[i], 1280 / 2 - 150, 250 + i * 40, 26, i == 5 ? Color{ 255, 130, 130, 255 } : RAYWHITE);
        std::string info = "Floor " + std::to_string(floor) + "   Level " + std::to_string(player.level) + "   Slain " + std::to_string(enemiesSlain);
        DrawText(info.c_str(), 1280 / 2 - MeasureText(info.c_str(), 20) / 2, 530, 20, SKYBLUE);
        if (showHelp) DrawHelpPanel();
    }

    // ── Drawing ──
    void Draw() {
        if (settings.lighting) RenderLightmap();
        BeginTextureMode(screenRT);
        BiomeColors biome = Biome();
        ClearBackground(biome.bg);

        BeginMode2D(camera);

        // 1. Tường gạch, sàn caro, bóng viền, decal, đuốc, cột
        int halfW = (int)roomW / 2;
        int halfH = (int)roomH / 2;
        int tileSize = 48;
        DrawWalls(biome);
        for (int y = -halfH; y < halfH; y += tileSize) {
            for (int x = -halfW; x < halfW; x += tileSize) {
                bool alt = (((x / tileSize) + (y / tileSize)) & 1) == 0;
                Color tileCol = alt ? biome.tileA : biome.tileB;
                DrawRectangle(x, y, tileSize, tileSize, tileCol);
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

        // Exit Portal
        if (portalOpen) {
            float pulse = sinf(GetTime() * 6.0f) * 6.0f;
            DrawCircle((int)portalPos.x, (int)portalPos.y, 34 + pulse, { 50, 200, 255, 60 });
            DrawCircle((int)portalPos.x, (int)portalPos.y, 26, { 100, 240, 255, 255 });
            DrawCircle((int)portalPos.x, (int)portalPos.y, 16, WHITE);
            DrawText("PORTAL", (int)portalPos.x - 26, (int)portalPos.y + 36, 14, SKYBLUE);
        }

        // Loots
        for (const auto& l : loots) {
            DrawCircle((int)l.pos.x, (int)l.pos.y, 8, l.type == 0 ? SKYBLUE : (l.type == 1 ? GOLD : RED));
            DrawCircleLines((int)l.pos.x, (int)l.pos.y, 8, WHITE);
        }

        // Ghost Trails
        for (const auto& g : ghosts) {
            float a = (g.timer / g.maxTimer) * 0.4f;
            DrawCircle((int)g.pos.x, (int)g.pos.y, 20, Fade(SKYBLUE, a));
        }

        // Slashes: vệt cung kiếm
        for (const auto& sl : slashes) {
            float rad = sl.angle * DEG2RAD;
            float k = sl.timer / sl.maxTimer;
            Vector2 centre = Vector2Subtract(sl.pos, { cosf(rad) * 35.0f, sinf(rad) * 35.0f });
            DrawRing(centre, 36.0f, 52.0f, sl.angle - 60.0f, sl.angle + 60.0f, 24, Fade(WHITE, 0.85f * k));
            DrawRing(centre, 30.0f, 38.0f, sl.angle - 45.0f, sl.angle + 45.0f, 20, Fade({ 120, 220, 255, 255 }, 0.7f * k));
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

        // Projectiles
        for (const auto& p : projectiles) {
            DrawCircleV(p.pos, p.radius * 2.0f, Fade(p.color, 0.22f));
            DrawCircleV(p.pos, p.radius, p.color);
            DrawCircleLines((int)p.pos.x, (int)p.pos.y, p.radius, WHITE);
        }

        // Player
        DrawEllipse((int)player.pos.x, (int)player.pos.y + 16, 20, 8, { 0, 0, 0, 100 }); // Shadow
        DrawPlayerSprite();

        // Aegis Barrier Glow
        if (player.shieldActive) {
            DrawCircleLines((int)player.pos.x, (int)player.pos.y, 28, Fade(GOLD, 0.8f));
        }

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
        // Top-Left: HP, XP, Gold
        DrawRectangle(20, 20, 320, 28, { 40, 45, 55, 200 });
        float hpPct = std::max(0.0f, player.hp / player.maxHp);
        DrawRectangle(22, 22, (int)(316 * hpPct), 24, { 230, 50, 60, 255 });
        std::string hpTxt = "HP: " + std::to_string((int)player.hp) + " / " + std::to_string((int)player.maxHp);
        DrawText(hpTxt.c_str(), 110, 25, 18, WHITE);

        DrawRectangle(20, 56, 320, 18, { 40, 45, 55, 200 });
        float xpPct = (float)player.xp / player.xpNext;
        DrawRectangle(22, 58, (int)(316 * xpPct), 14, SKYBLUE);
        std::string xpTxt = "LVL " + std::to_string(player.level) + " - XP: " + std::to_string(player.xp) + "/" + std::to_string(player.xpNext);
        DrawText(xpTxt.c_str(), 110, 58, 14, WHITE);

        std::string goldTxt = "Gold: " + std::to_string(player.gold);
        DrawText(goldTxt.c_str(), 24, 84, 20, GOLD);

        // Relics Row
        DrawText("Relics:", 24, 110, 16, SKYBLUE);
        if (player.relics.empty()) {
            DrawText("None", 24 + MeasureText("Relics: ", 16), 110, 16, SKYBLUE);
        } else {
            int rx = 24 + MeasureText("Relics: ", 16);
            for (const auto& r : player.relics) {
                DrawCircle(rx + 6, 118, 6, r.color);
                DrawCircleLines(rx + 6, 118, 6, WHITE);
                DrawText(r.name.c_str(), rx + 16, 110, 16, r.color);
                rx += 16 + MeasureText(r.name.c_str(), 16) + 14;
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
            int hx = 24 + i * 36, hy = 138;
            Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 60, 68, 90, 255 };
            DrawRectangle(hx, hy, 30, 30, { 14, 18, 30, 210 });
            DrawRectangleLines(hx, hy, 30, 30, rc);
            DrawItemIcon(i, { hx + 15.0f, hy + 15.0f }, 17.0f, hasEquip[i] ? rc : Color{ 70, 78, 100, 255 });
        }
        DrawText("[I] Gear", 24 + 4 * 36 + 4, 146, 14, { 150, 165, 200, 255 });

        // Thanh máu boss
        for (const auto& e : enemies) {
            if (e.type == BOSS && e.hp > 0) {
                int bw = 460, bx = 1280 / 2 - bw / 2, by = 66;
                float pct = std::max(0.0f, e.hp / e.maxHp);
                DrawRectangle(bx - 3, by - 3, bw + 6, 22, { 10, 12, 20, 230 });
                DrawRectangle(bx, by, (int)(bw * pct), 16, e.enraged ? Color{ 255, 70, 50, 255 } : Color{ 190, 60, 200, 255 });
                DrawRectangleLines(bx - 3, by - 3, bw + 6, 22, GOLD);
                const char* nm = e.enraged ? "THE VOID SOVEREIGN - ENRAGED" : "THE VOID SOVEREIGN";
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

        // Skill Icons (Bottom Center)
        DrawRectangle(1280 / 2 - 60, 720 - 70, 50, 50, { 30, 35, 48, 220 });
        DrawText("DASH", 1280 / 2 - 54, 720 - 64, 12, YELLOW);
        DrawText("RMB", 1280 / 2 - 50, 720 - 36, 12, GRAY);
        if (player.dashTimer > 0) {
            float pct = player.dashTimer / player.dashCooldown;
            DrawRectangle(1280 / 2 - 60, (int)(720 - 70 + (1.0f - pct) * 50), 50, (int)(pct * 50), Fade(BLACK, 0.7f));
        }

        DrawRectangle(1280 / 2 + 10, 720 - 70, 50, 50, { 30, 35, 48, 220 });
        DrawText("NOVA", 1280 / 2 + 16, 720 - 64, 12, SKYBLUE);
        DrawText("Q / E", 1280 / 2 + 18, 720 - 36, 12, GRAY);
        if (player.specialTimer > 0) {
            float pct = player.specialTimer / player.specialCooldown;
            DrawRectangle(1280 / 2 + 10, (int)(720 - 70 + (1.0f - pct) * 50), 50, (int)(pct * 50), Fade(BLACK, 0.7f));
        }

        // Banner Toast
        if (bannerTimer > 0 && state != TITLE && state != PAUSED) {
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

                std::string btn = "PRESS [" + std::to_string(i + 1) + "]";
                DrawRectangle((int)cx + 30, (int)cy + 210, 180, 42, DARKBLUE);
                DrawText(btn.c_str(), (int)cx + 55, (int)cy + 222, 18, WHITE);
            }
        }

        // ── Game Over Modal ──
        if (state == GAME_OVER) {
            DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.88f));
            DrawText("YOU HAVE FALLEN", 1280 / 2 - 180, 180, 44, RED);

            std::string st1 = "Floor Reached: " + std::to_string(floor);
            std::string st2 = "Final Level: " + std::to_string(player.level);
            std::string st3 = "Enemies Slain: " + std::to_string(enemiesSlain);
            std::string st4 = "Gold Accumulated: " + std::to_string(player.gold);
            std::string st5 = "Best Record: Floor " + std::to_string(bestFloor) + (newRecord ? "  (NEW RECORD!)" : "");

            DrawText(st1.c_str(), 1280 / 2 - 140, 260, 22, RAYWHITE);
            DrawText(st2.c_str(), 1280 / 2 - 140, 295, 22, RAYWHITE);
            DrawText(st3.c_str(), 1280 / 2 - 140, 330, 22, RAYWHITE);
            DrawText(st4.c_str(), 1280 / 2 - 140, 365, 22, GOLD);
            DrawText(st5.c_str(), 1280 / 2 - 140, 400, 22, newRecord ? GREEN : SKYBLUE);
            if (bestCombo >= 2) {
                std::string st6 = "Best Combo: x" + std::to_string(bestCombo);
                DrawText(st6.c_str(), 1280 / 2 - 140, 428, 22, ORANGE);
            }

            DrawRectangle(1280 / 2 - 130, 460, 260, 52, MAROON);
            DrawText("PLAY AGAIN (R)", 1280 / 2 - 75, 476, 22, WHITE);
        }

        if (state == INVENTORY) DrawInventory();
        if (state == SETTINGS) DrawSettingsScreen();
        if (state == SHOP) DrawShopScreen();
        if (state == TITLE) DrawTitleScreen();
        if (state == PAUSED) DrawPauseScreen();

        if (settings.showFps) {
            std::string f = "FPS: " + std::to_string(GetFPS());
            DrawText(f.c_str(), 1280 - 12 - MeasureText(f.c_str(), 18), 720 - 26, 18, { 120, 255, 140, 255 });
        }

        EndTextureMode();
        Present();
    }
};

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Geometric Dungeon - Standalone Roguelike RPG");
    SetWindowMinSize(640, 360);
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

    game.snd.Close();
    CloseWindow();
    return 0;
}
