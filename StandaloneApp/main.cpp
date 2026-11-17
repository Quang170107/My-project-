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

// ── Phông chữ Unicode đậm sắc nét đa kích thước hỗ trợ đầy đủ Tiếng Việt ──
static const int kFontBuckets[7] = { 15, 18, 21, 24, 28, 34, 46 };
static Font gUiFonts[7] = { { 0 } };
static bool gFontLoaded = false;
static bool gDrawingOutlinePass = false;

static void AppendUtf8(std::string& dst, int cp) {
    if (cp <= 0x7F) {
        dst.push_back((char)cp);
    } else if (cp <= 0x7FF) {
        dst.push_back((char)(0xC0 | ((cp >> 6) & 0x1F)));
        dst.push_back((char)(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        dst.push_back((char)(0xE0 | ((cp >> 12) & 0x0F)));
        dst.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        dst.push_back((char)(0x80 | (cp & 0x3F)));
    } else {
        dst.push_back((char)(0xF0 | ((cp >> 18) & 0x07)));
        dst.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
        dst.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        dst.push_back((char)(0x80 | (cp & 0x3F)));
    }
}

static void PopUtf8Char(std::string& s) {
    while (!s.empty() && ((unsigned char)s.back() & 0xC0) == 0x80) s.pop_back();
    if (!s.empty()) s.pop_back();
}

static void InitGameFont() {
    if (gFontLoaded) return;
    std::vector<int> cps;
    for (int c = 0x0020; c <= 0x007E; ++c) cps.push_back(c);
    for (int c = 0x00A0; c <= 0x024F; ++c) cps.push_back(c);
    for (int c = 0x1E00; c <= 0x1EFF; ++c) cps.push_back(c);
    for (int c = 0x2010; c <= 0x203A; ++c) cps.push_back(c);
    cps.push_back(0x20AB);
    static const char* kFontPaths[] = {
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/tahomabd.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/system/fonts/Roboto-Bold.ttf",
        "/system/fonts/NotoSans-Bold.ttf",
        "/system/fonts/Roboto-Regular.ttf",
        "/system/fonts/NotoSans-Regular.ttf"
    };
    const char* chosen = nullptr;
    for (const char* path : kFontPaths) {
        if (FileExists(path)) { chosen = path; break; }
    }
    if (!chosen) return;
    for (int i = 0; i < 7; ++i) {
        Font f = LoadFontEx(chosen, kFontBuckets[i], cps.data(), (int)cps.size());
        if (f.texture.id > 0 && f.baseSize > 0) {
            gUiFonts[i] = f;
            SetTextureFilter(gUiFonts[i].texture, TEXTURE_FILTER_POINT);
        }
    }
    gFontLoaded = (gUiFonts[0].texture.id > 0 && gUiFonts[0].baseSize > 0);
}

static void CloseGameFont() {
    for (int i = 0; i < 7; ++i) {
        if (gUiFonts[i].texture.id > 0 && gUiFonts[i].baseSize > 0) {
            UnloadFont(gUiFonts[i]);
            gUiFonts[i] = { 0 };
        }
    }
    gFontLoaded = false;
}

static int EffectiveFontSize(int fontSize) {
    if (fontSize <= 11) return fontSize + 3;
    if (fontSize <= 15) return fontSize + 2;
    if (fontSize <= 20) return fontSize + 1;
    return fontSize;
}

static const Font& PickGameFont(int effSize) {
    int best = 0;
    int bestDiff = 9999;
    for (int i = 0; i < 7; ++i) {
        if (gUiFonts[i].baseSize <= 0) continue;
        int d = abs(gUiFonts[i].baseSize - effSize);
        if (d < bestDiff) { bestDiff = d; best = i; }
    }
    return gUiFonts[best];
}

static void GameDrawText(const char* text, int posX, int posY, int fontSize, Color color) {
    if (!text) return;
    int eff = EffectiveFontSize(fontSize);
    if (gFontLoaded) {
        const Font& f = PickGameFont(eff);
        const float sp = (eff >= 28) ? 0.8f : 0.4f;
        if (!gDrawingOutlinePass && ((int)color.r + (int)color.g + (int)color.b) > 75 && color.a > 60) {
            Color sh = { 6, 8, 16, (unsigned char)std::min(235, (int)color.a) };
            DrawTextEx(f, text, { (float)(posX + 1), (float)(posY + 1) }, (float)eff, sp, sh);
            DrawTextEx(f, text, { (float)posX, (float)(posY + 1) }, (float)eff, sp, sh);
        }
        DrawTextEx(f, text, { (float)posX, (float)posY }, (float)eff, sp, color);
    } else {
        ::DrawText(text, posX, posY, fontSize, color);
    }
}

static int GameMeasureText(const char* text, int fontSize) {
    if (!text) return 0;
    int eff = EffectiveFontSize(fontSize);
    if (gFontLoaded) {
        const Font& f = PickGameFont(eff);
        const float sp = (eff >= 28) ? 0.8f : 0.4f;
        return (int)ceilf(MeasureTextEx(f, text, (float)eff, sp).x);
    }
    return ::MeasureText(text, fontSize);
}

#define DrawText GameDrawText
#define MeasureText GameMeasureText

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

// ── Nhánh Chuyển Chức (Lv.5 Subclass Awakening: mỗi class rẽ 2 nhánh phụ) ──
struct SubclassDef {
    int baseClass;
    int branch; // 1 hoặc 2
    const char* name;
    const char* role;
    Color color;
    Color accent;
    int signatureWeapon;
    const char* weaponTitle;
    float bonusHp;
    float bonusDmg;
    float bonusArmor;
    float bonusSpeed;
    float bonusAtkRate;
    float bonusCrit;
    float bonusLifesteal;
    float cCooldown;
    float vCooldown;
    const char* passiveName;
    const char* passiveDesc;
    const char* qName;
    const char* qDesc;
    const char* eName;
    const char* eDesc;
    const char* cName;
    const char* cDesc;
    const char* vName;
    const char* vDesc;
    const char* rName;
    const char* rDesc;
};

static const SubclassDef kSubclasses[4][2] = {
    { // KNIGHT (Class weapon: Sword WP_SWORD)
        {
            CLASS_KNIGHT, 1, "Holy Paladin", "Thanh Ky Si - Thanh Kiem & Khien",
            { 255, 215, 90, 255 }, { 120, 230, 255, 255 }, WP_SWORD, "Sunforged Holy Sword",
            35.0f, 4.0f, 3.0f, 0.0f, 0.20f, 0.05f, 0.05f, 8.0f, 10.0f,
            "Divine Sanctuary", "-20% dmg taken; every 4th hit smites 115px AoE & heals 6 HP",
            "Crusader Grand Slam", "Lunge, slam 2.4x DMG, Stun 1.5s & spawn 2.5s healing zone",
            "Aegis of Dawn", "2.2x Holy Nova + 4.5s Barrier (+6 ARM, +30% SPD, block 1 hit)",
            "Consecrated Hammer", "Throw 3 piercing holy hammers (2.1x DMG, Stun 1.2s) & heal 8% HP",
            "Heavenly Chains", "210px radiant chain pull nova (2.3x DMG + Burn) & +15 Rage",
            "Judgment of Solaris", "4.2x Light Swords + 4.5s Sanctuary (1.1x ticks & heal)"
        },
        {
            CLASS_KNIGHT, 2, "Dread Berserker", "Cuong Chien - Huyet Kiem Cuong Bao",
            { 235, 65, 65, 255 }, { 255, 165, 50, 255 }, WP_SWORD, "Bloodreaver Greatsword",
            20.0f, 10.0f, 0.0f, 25.0f, 0.45f, 0.08f, 0.12f, 7.5f, 9.5f,
            "Blood Frenzy", "Sword swings fire Blood Waves; missing HP grants up to +40% ATK SPD",
            "Savage Cleave Leap", "Leap 110px, slam 2.5x crimson AoE, Burn & heal 10% Max HP",
            "Bloodboil Roar", "2.0x Blood Nova + Slow, +25 Rage & 4s Frenzy (+30% ATK/SPD)",
            "Crimson Eruption", "Erupt 3 blood geysers forward (2.4x AoE + Burn) & heal 12% HP",
            "Gore Whirlwind", "Fire 8 piercing Blood Waves all around (1.85x) + 2.5s Haste",
            "Cataclysmic Guillotine", "4.2x Crimson Quake + 4s Blood Vortex pulling & shredding foes"
        }
    },
    { // ASSASSIN (Class weapon: Dagger WP_DAGGER)
        {
            CLASS_ASSASSIN, 1, "Phantom Reaper", "Tu Than - Song Chu Doan Menh",
            { 195, 90, 255, 255 }, { 100, 245, 210, 255 }, WP_DAGGER, "Nightfall Soulblades",
            25.0f, 8.0f, 1.5f, 15.0f, 0.25f, 0.08f, 0.10f, 7.5f, 9.5f,
            "Soul Harvest", "Crits deal 2.6x DMG; kills launch homing Soul Wisps & heal 4 HP",
            "Death Spiral", "Hurl 6 piercing spectral blades (1.35x DMG, Poison & Chill)",
            "Reaper's Blink", "Blink 225px, 2.6x guaranteed-crit Soul Reap + Soul Shield",
            "Wraith Swarm", "Launch 6 homing Soul Wisps (1.35x DMG + Poison/Slow) & heal 18 HP",
            "Graveyard Scythe", "185px spectral scythe sweep (2.8x Crit + Freeze 1.4s) & +20 Rage",
            "Wraith Hurricane", "4.0s Blade Storm (1.05x slashes/0.18s & heal 2 HP per hit)"
        },
        {
            CLASS_ASSASSIN, 2, "Ninja Trickster", "Nhan Gia - Song Dao Sam Chop",
            { 255, 75, 150, 255 }, { 110, 235, 255, 255 }, WP_DAGGER, "Stormfang Daggers",
            15.0f, 6.0f, 0.5f, 35.0f, 0.65f, 0.12f, 0.05f, 7.0f, 9.0f,
            "Shadow Clonejutsu", "Attacks throw lightning shurikens; Dash drops explosive decoy",
            "Tempest Shurikens", "Throw 8 lightning shurikens (1.25x DMG, pierce 2 & mini-stun)",
            "Lightning Mirage", "Blink 230px, drop 2.2x explosive clone & 2.4x crit slash",
            "Caltrop Storm", "Drop 3 electric caltrop traps (1.9x AoE + Slow) + 5 shurikens",
            "Thunder Flash Slash", "Lightning dash-cut 180px (2.7x Crit + Stun 1.1s) & reset Dash",
            "Thousand Blades", "3.8s hyper-storm (+45% SPD, rapid slashes + 4-way shurikens)"
        }
    },
    { // MAGE (Class weapon: Staff WP_STAFF; Spellblade renders as Magic Sword!)
        {
            CLASS_MAGE, 1, "Spellblade (Kiem Su)", "Kiem Su - Gay Phep Hoa Kiem",
            { 85, 230, 255, 255 }, { 210, 110, 255, 255 }, WP_STAFF, "Runeblade Staff (Sword)",
            32.0f, 8.0f, 2.5f, 25.0f, 0.55f, 0.10f, 0.06f, 7.0f, 9.5f,
            "Enchanted Runeblade", "Staff channels as Magic Sword firing Moonlight Waves; Shield +25% SPD",
            "Dimensional Slash", "Dash-cut 115px (2.4x DMG) + fire 3 piercing Moonlight Waves",
            "Phantom Sword Ring", "1.9x Frost-Fire Nova (Stun 1.4s) + 3.2s orbiting Magic Swords",
            "Moonlight Cross Cut", "Cross-slash (2.3x DMG) firing 5 Moonlight Waves + Arcane Shield",
            "Astral Sword Array", "Call down 6 Astral Swords (2.1x AoE + Burn) & +18 Rage",
            "Excalibur Astral Storm", "3.8x Astral Greatsword impact + 4s Sword Rain & +35% ATK SPD"
        },
        {
            CLASS_MAGE, 2, "Archmage", "Dai Phap Su - Quyen Truong Nguyen To",
            { 165, 105, 255, 255 }, { 255, 180, 70, 255 }, WP_STAFF, "Archon Grand Staff",
            18.0f, 11.0f, 1.0f, 10.0f, 0.35f, 0.08f, 0.05f, 7.5f, 10.0f,
            "Elemental Overload", "Twin explosive staff bolts; skills fire 5 homing missiles +10 Rage",
            "Twin Meteor Comets", "Launch 2 blazing comets (2.3x AoE each + heavy Burn)",
            "Absolute Zero Nova", "215px Blizzard blast (1.9x DMG, Freeze 1.8s) + 6 ice shards",
            "Chain Tempest Orb", "3 storm orbs (1.9x AoE) + chain lightning on 6 foes (Stun 0.9s)",
            "Arcane Cataclysm", "Solar-arcane pillar (2.6x AoE + 3s burn zone) & 6 homing missiles",
            "Cosmic Singularity", "4.2s giant Void Black Hole + meteors (1.75x) & +25% HP heal"
        }
    },
    { // RANGER (Class weapon: Bow WP_BOW)
        {
            CLASS_RANGER, 1, "Sniper Deadeye", "Than Tien - Cung Than Bach Phat",
            { 95, 240, 145, 255 }, { 255, 220, 85, 255 }, WP_BOW, "Windforce Longbow",
            18.0f, 10.0f, 0.5f, 20.0f, 0.45f, 0.14f, 0.05f, 7.0f, 9.5f,
            "Hawk's Execution", "Every 2nd shot is a 1.75x piercing Wind Shot; arrows +1 pierce",
            "Starlight Barrage", "Fire a 9-arrow piercing fan (1.2x DMG each + Burn)",
            "Cluster Snare Vault", "Drop 2.8x stun-mine, vault back 140px & +35% ATK SPD 2.5s",
            "AP Railshot", "Hyper rail arrow (3.0x Crit, infinite pierce + 80px blast)",
            "Flare & Camouflage", "6 flare arrows (1.7x AoE) + 1.5s Invuln & 3.0s Haste",
            "Solaris Arrow Tempest", "4.0s storm of sun-arrows (1.35x + Burn) & +45% ATK SPD"
        },
        {
            CLASS_RANGER, 2, "Tempest Valkyrie", "Chien Than - Cung Sam Set",
            { 90, 215, 235, 255 }, { 255, 215, 90, 255 }, WP_BOW, "Thunderbolt Greatbow",
            30.0f, 8.0f, 2.5f, 25.0f, 0.35f, 0.08f, 0.08f, 7.5f, 9.5f,
            "Storm Arrow", "Every bow shot fires piercing Lightning Arrows with shockwave",
            "Thunder Volley", "Fire 7 lightning arrows (1.35x AoE, pierce 3 & 0.8s Stun)",
            "Tempest Vault", "Drop 2.5x Shock Trap, leap back & gain Storm Shield + Haste",
            "Valkyrie Spear", "Hurl 3 lightning javelins (2.1x AoE, pierce 4) + Storm Shield",
            "Eye of Hurricane", "200px shockwave (2.2x + Stun 1.1s), 8 storm arrows & +20 Rage",
            "Gungnir Skyfall", "Rain Lightning Arrows for 3.8s (1.35x AoE) + +35% ATK SPD"
        }
    }
};

struct ClassTextVi {
    const char* name;
    const char* role;
    const char* weaponTitle;
    const char* passiveName;
    const char* passiveDesc;
    const char* qName;
    const char* qDesc;
    const char* eName;
    const char* eDesc;
    const char* cName;
    const char* cDesc;
    const char* vName;
    const char* vDesc;
    const char* rName;
    const char* rDesc;
};

static const ClassTextVi kClassesVi[4] = {
    {
        "Kỵ Sĩ Tiên Phong", "Đấu Sĩ Giáp Nặng", "Kiếm Sắt",
        "Khiên Bất Khuất", "Giảm 15% sát thương; bị đánh +8 Nộ & nổ xung kích; đòn thứ 5 hồi 4 HP",
        "Xung Kích Khiên", "Lướt húc gây 1.9x sát thương & Làm choáng 1.2s",
        "Thánh Quang Xung", "Nổ 1.8x ST + Giáp Thánh 3.5s (+4 Giáp, +25% Tốc Đánh, chặn 1 đòn)",
        "", "", "", "",
        "Phán Quyết Thiên Đường", "3.5x Đại Kiếm Ánh Sáng + Choáng, vùng thánh 4s (0.85x & hồi 4% HP/s)"
    },
    {
        "Sát Thủ Bóng Đêm", "Chí Mạng Tốc Độ", "Dao Găm",
        "Động Lực Tử Thần", "Chí mạng gây 2.4x ST + Độc; hạ gục hồi 50% Dash & +25% Tốc Chạy 2s",
        "Phi Tiêu Bóng Tối", "Phóng 5 phi tiêu xuyên thấu (1.1x ST, Độc & Làm chậm)",
        "Ám Tập", "Dịch chuyển 210px, thả bom khói & chém chắc chắn chí mạng 2.2x",
        "", "", "", "",
        "Vũ Điệu Tử Thần", "3.5s Bão Kiếm (+35% Tốc Chạy, -50% ST nhận, chém 0.85x/0.2s)"
    },
    {
        "Pháp Sư Huyền Bí", "Pháp Sư Nguyên Tố", "Quyền Trượng",
        "Cộng Hưởng Ma Pháp", "Giảm 15% hồi chiêu; tung Q/E/R bắn 3 tia dẫn đường & +6 Nộ",
        "Cầu Lửa Hủy Diệt", "Cầu lửa nổ diện rộng 135px gây 2.3x ST & Thiêu đốt",
        "Vòng Băng Giá", "Vụ nổ băng 185px gây 1.5x ST, Đóng băng 1.3s & Làm chậm",
        "", "", "", "",
        "Thiên Thạch Hư Không", "3.8s Hố Đen hút quái & mưa 12 thiên thạch (1.5x ST/cái)"
    },
    {
        "Xạ Thủ Lục Bảo", "Xạ Thủ Tầm Xa", "Cung Tên",
        "Dấu Ấn Đại Bàng", "Mỗi đòn thứ 3 bắn Tên Gió xuyên thấu 1.5x ST (+10% Chí mạng)",
        "Liên Xạ Tiễn", "Bắn quạt 7 mũi tên (1.0x ST mỗi tên) đẩy lùi kẻ địch",
        "Bẫy Nổ & Nhào Lộn", "Đặt bẫy nổ (2.4x AoE + Làm chậm) & lộn ngược lùi về",
        "", "", "", "",
        "Mưa Tên Phượng Hoàng", "3.6s bão 16 mũi tên lửa (1.15x + Thiêu đốt) & +35% Tốc Đánh"
    }
};

static const ClassTextVi kSubclassesVi[4][2] = {
    { // KNIGHT
        {
            "Thánh Kỵ Sĩ Paladin", "Thánh Kỵ Sĩ - Kiếm & Khiên Thần", "Thánh Kiếm Nhật Quang",
            "Thánh Vực Bảo Hộ", "Giảm 20% ST nhận vào; đòn thứ 4 giáng thánh 115px & hồi 6 HP",
            "Thiên Phạt Đại Chấn", "Lướt đập 2.4x ST, Choáng 1.5s & tạo Thánh Vực hồi máu 2.5s",
            "Khiên Bình Minh", "Nổ 2.2x ST + Giáp Thánh 4.5s (+6 Giáp, +30% Tốc Đánh, chặn 1 đòn)",
            "Búa Thánh Phán Quyết", "Ném 3 búa thánh xuyên thấu (2.1x ST, Choáng 1.2s) & hồi 8% Máu",
            "Xiềng Xích Thiên Đường", "210px xích quang kéo quái tụ lại (2.3x ST + Đốt) & +15 Nộ",
            "Phán Quyết Solaris", "4.2x Thánh Kiếm + 4.5s Thánh Vực (1.1x ST & hồi máu liên tục)"
        },
        {
            "Cuồng Chiến Huyết Ma", "Cuồng Chiến - Huyết Kiếm Cuồng Bạo", "Trọng Kiếm Huyết Ma",
            "Huyết Cuồng Nộ", "Chém phóng Huyết Kiếm Khí; máu càng thấp tốc đánh càng tăng (+40%)",
            "Nhảy Chém Cuồng Bạo", "Nhảy 110px đập vùng máu 2.5x ST, Thiêu đốt & hồi 10% Máu Tối Đa",
            "Tiếng Gầm Sôi Máu", "Nổ máu 2.0x ST + Làm chậm, +25 Nộ & Cuồng Nộ 4s (+30% Tốc)",
            "Huyết Địa Chấn", "Bùng 3 cột huyết nham phía trước (2.4x AoE + Đốt) & hồi 12% Máu",
            "Cơn Lốc Xé Xác", "Phóng 8 Huyết Kiếm Khí tỏa tròn (1.85x ST) + Tăng Tốc 2.5s",
            "Chém Đầu Tận Thế", "4.2x Địa Chấn Đỏ + 4s Xoáy Máu hút và xé nát kẻ địch"
        }
    },
    { // ASSASSIN
        {
            "Tử Thần Hư Vô", "Tử Thần - Song Chủy Đoạt Mệnh", "Song Chủy Đoạt Hồn",
            "Thu Hoạch Linh Hồn", "Chí mạng 2.6x ST; hạ gục thả Tinh Linh truy đuổi & hồi 4 HP",
            "Vòng Xoáy Tử Thần", "Phóng 6 lưỡi dao linh hồn xuyên 3 mục tiêu (1.35x ST, Độc & Băng)",
            "Tử Thần Dịch Chuyển", "Biến 225px, chém 2.6x chắc chắn chí mạng & tạo Khiên Linh Hồn",
            "Bầy Oan Hồn", "Thả 6 Tinh Linh dẫn đường (1.35x ST + Độc/Chậm) & hồi 18 HP",
            "Lưỡi Hái Tử Địa", "Quét lưỡi hái 185px (2.8x Chí Mạng + Đóng Băng 1.4s) & +20 Nộ",
            "Cơn Lốc Oan Hồn", "4.0s Bão Đoạt Hồn (1.05x ST/0.18s & hồi 2 HP mỗi nhát trúng)"
        },
        {
            "Nhẫn Giả Lôi Ảnh", "Nhẫn Giả - Song Đao Sấm Chớp", "Song Đao Lôi Nha",
            "Ảo Ảnh Phân Thân", "Mỗi đòn đánh phóng kèm phi tiêu sét; Dash để lại bẫy phân thân nổ",
            "Bão Phi Tiêu Sấm", "Ném 8 phi tiêu sấm sét (1.25x ST, xuyên 2 & choáng nhẹ)",
            "Lôi Ảnh Huyễn Thuật", "Biến 230px, thả phân thân nổ 2.2x & chém chí mạng 2.4x",
            "Mưa Chông Điện", "Rải 3 bẫy chông điện (1.9x AoE + Chậm) & ném 5 phi tiêu sét",
            "Lôi Thiểm Nhất Trảm", "Lướt sét chém 180px (2.7x Chí Mạng + Choáng 1.1s) & hồi ngay Dash",
            "Nghìn Lưỡi Đao", "3.8s siêu bão kiếm (+45% Tốc Chạy, chém cực nhanh + 4 hướng tiêu)"
        }
    },
    { // MAGE
        {
            "Kiếm Sư Ma Pháp", "Kiếm Sư - Gậy Phép Hóa Kiếm", "Gậy Kiếm Tinh Tú (Kiếm)",
            "Ma Kiếm Phù Văn", "Cầm Gậy hóa model Kiếm Phép chém phóng Kiếm Khí Ánh Trăng nổ AoE",
            "Trảm Không Gian", "Lướt chém 115px (2.4x ST) + phóng 3 Kiếm Khí Ánh Trăng + Khiên",
            "Vòng Kiếm Hồn", "Nổ băng-lửa 1.9x (Choáng 1.4s) + 3.2s Kiếm Hồn bay chém xung quanh",
            "Thập Tự Ánh Trăng", "Chém thập tự (2.3x ST) phóng 5 Kiếm Khí Ánh Trăng + Khiên Phép",
            "Kiếm Trận Tinh Tú", "Triệu 6 Thần Kiếm giáng xuống quanh mục tiêu (2.1x AoE) & +18 Nộ",
            "Thần Kiếm Excalibur", "3.8x Đại Kiếm Tinh Tú + 4s Mưa Kiếm Phép & +35% Tốc Đánh"
        },
        {
            "Đại Pháp Sư", "Đại Pháp Sư - Quyền Trượng Nguyên Tố", "Quyền Trượng Tối Thượng",
            "Quá Tải Nguyên Tố", "Bắn song cầu phép nổ; dùng chiêu bắn 5 tia dẫn đường & +10 Nộ",
            "Song Thiên Thạch", "Phóng 2 sao chổi lửa khổng lồ (2.3x AoE mỗi quả + Thiêu đốt)",
            "Băng Giá Tuyệt Đối", "Nổ bão tuyết 215px (1.9x ST, Đóng băng 1.8s) + 6 mảnh băng",
            "Cầu Sấm Liên Hoàn", "Phóng 3 cầu sấm (1.9x AoE) + giật sét lan 6 mục tiêu (Choáng 0.9s)",
            "Cột Sáng Hủy Diệt", "Giáng Cột Quang Ám (2.6x AoE + vùng đốt 3s) & 6 tia dẫn đường",
            "Kỳ Dị Vũ Trụ", "4.2s Hố Đen Khổng Lồ + mưa thiên thạch (1.75x) & hồi 25% Máu"
        }
    },
    { // RANGER
        {
            "Thần Tiễn Bách Phát", "Thần Tiễn - Cung Thần Xuyên Phá", "Trường Cung Cuồng Phong",
            "Ưng Nhãn Hành Quyết", "Mỗi phát thứ 2 bắn Tên Gió 1.75x xuyên thấu; tên thường +1 xuyên",
            "Liên Hoàn Tinh Tú", "Bắn quạt 9 mũi tên xuyên 2 mục tiêu (1.2x ST + Thiêu đốt)",
            "Mìn Chùm & Lộn Ngược", "Đặt mìn choáng 2.8x ST, lộn lùi 140px & +35% Tốc Đánh 2.5s",
            "Tiễn Xuyên Phá", "Bắn Tên Siêu Thanh 3.0x Chí Mạng xuyên toàn bộ & nổ 80px",
            "Pháo Sáng & Ẩn Thân", "Bắn 6 tên pháo sáng (1.7x AoE) + 1.5s Bất Tử & Tăng Tốc 3s",
            "Bão Tên Thái Dương", "4.0s mưa 22 mũi tên mặt trời (1.35x + Đốt) & +45% Tốc Đánh"
        },
        {
            "Chiến Thần Sấm Sét", "Chiến Thần - Cung Sấm Sét", "Đại Cung Lôi Đình",
            "Mũi Tên Bão Táp", "Cung bắn ra Tên Sét xuyên thấu & nổ xung kích diện rộng",
            "Loạt Tên Lôi Đình", "Bắn 7 mũi tên sét nổ AoE (1.35x ST, xuyên 3 & Choáng 0.8s)",
            "Nhảy Bão Sấm", "Đặt bẫy điện 2.5x ST, lộn về & nhận Khiên Bão + Tăng Tốc",
            "Lao Sấm Valkyrie", "Phóng 3 ngọn Lao Sét xuyên 4 mục tiêu (2.1x AoE) + Khiên Sấm",
            "Tâm Bão Lôi Đình", "Sóng sét 200px (2.2x ST + Choáng 1.1s), 8 tên sét & +20 Nộ",
            "Lôi Tiễn Gungnir", "3.8s Mưa Tên Sét từ bầu trời (1.35x AoE) & +35% Tốc Đánh"
        }
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
    int subClass = 0; // 0: class goc, 1: nhanh phu A, 2: nhanh phu B
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
    float skill3Timer = 0.0f;
    float skill3Cooldown = 7.5f;
    float skill4Timer = 0.0f;
    float skill4Cooldown = 9.5f;
    float rage = 0.0f;
    float maxRage = 100.0f;
    float ultActiveTimer = 0.0f;
    float ultTickTimer = 0.0f;
    float swordOrbitTimer = 0.0f;
    float swordOrbitTick = 0.0f;
    float holyBuffTimer = 0.0f;
    float hasteTimer = 0.0f;
    int attackCounter = 0;
    int cdrUpgrades = 0;
    float critChance = 0.05f;
    float lifesteal = 0.0f;
    float armor = 2.0f;
    int level = 1;
    int titleTierClaimed = 0;
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
    float frostSlowTimer = 0.0f;
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
    int eliteAffix = 0;      // 0: none, 1: Vampiric, 2: Blink, 3: Molten, 4: Frostward
    float affixTimer = 2.5f;
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
    int kind = 0;          // 0: đạn quái, 1: cung tên, 2: cầu phép trượng, 3: phi tiêu, 4: cầu lửa, 5: tên dẫn đường, 6: kiếm khí trăng khuyết
    int pierce = 0;
    int hitMask = -1;
    float splashRadius = 0.0f;
    float splashRatio = 0.5f;
    bool applyBurn = false;
    bool applySlow = false;
    bool applyPoison = false;
    bool applyStun = false;
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

static std::string FmtPct(float frac) {
    float pct = frac * 100.0f;
    float r1 = roundf(pct * 10.0f) / 10.0f;
    char buf[32];
    if (fabsf(r1 - roundf(r1)) > 0.04f) snprintf(buf, sizeof(buf), "%.1f%%", r1);
    else snprintf(buf, sizeof(buf), "%d%%", (int)roundf(r1));
    return std::string(buf);
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
        if (lifesteal > 0) v.push_back("+" + FmtPct(lifesteal) + " Lifesteal");
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

static EquipItem GenerateItem(int floor, int forcedWeaponType = -1, float lsScale = 1.0f) {
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
            it.lifesteal = std::min(0.15f * lsScale, roundf(0.02f * m * lsScale * 1000.0f) / 1000.0f);
            it.attackRate = std::min(2.0f, R1(0.25f * m));
            break;
    }
    if (it.slot == SLOT_WEAPON) {
        int wi = (forcedWeaponType >= 0 && forcedWeaponType < 8) ? forcedWeaponType : (rand() % 8);
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

struct ArenaBarrel {
    int kind = 0; // 0: Explosive Keg, 1: Cryo Crystal, 2: Shock Conduit
    Vector2 pos = { 0, 0 };
    float hp = 20.0f;
    float maxHp = 20.0f;
    float hitFlash = 0.0f;
    bool destroyed = false;
};

struct FloorHazard {
    int kind = 0; // 0: Spike Grate, 1: Toxic Vent, 2: Magma Geyser, 3: Arcane Leyline (Buff)
    Vector2 pos = { 0, 0 };
    float radius = 48.0f;
    float timer = 0.0f;
    float cycle = 3.6f;
    float tickTimer = 0.0f;
};

struct PortalOption {
    Vector2 pos = { 0, 0 };
    int nextMod = MOD_NONE;
    int rewardKind = 0; // 0: Gear/Relic, 1: +Bonus Gold, 2: +35% Heal, 3: +40 Rage
    std::string title;
    std::string subtitle;
    Color color = SKYBLUE;
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

static const char* kMetaVi[8][2] = {
    { "Sinh Lực", "+10 Máu tối đa mỗi cấp" },
    { "Sức Mạnh", "+2 Sát thương mỗi cấp" },
    { "Nhanh Nhẹn", "+5 Tốc chạy mỗi cấp" },
    { "Phú Quý", "+10 Vàng khởi đầu mỗi cấp" },
    { "Học Giả", "+8% Kinh nghiệm nhận được mỗi cấp" },
    { "Bùa May Mắn", "+2% Tỉ lệ rơi trang bị mỗi cấp" },
    { "Mặc Cả", "-5% Giá mua tại Cửa hàng mỗi cấp" },
    { "Hơi Thở Thứ Hai", "Bắt đầu mỗi lượt chơi với 1 Cổ Vật ngẫu nhiên" }
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
    int language = 1;     // 0: English, 1: Tiếng Việt
};
static const int kFpsOptions[5] = { 30, 60, 120, 144, 0 }; // 0 = không giới hạn
static const int kResOptions[5][2] = { { 1280, 720 }, { 1366, 768 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 } };

// ── Mặt Đất, Gia Đình, Gia Phả & Quý Tộc ───────────────────────────────────
struct SpouseDef {
    const char* nameEn;
    const char* nameVi;
    const char* roleEn;
    const char* roleVi;
    int cost;
    float hpBonus;
    float dmgBonus;
    float critBonus;
    float atkSpdBonus;
    Color dressCol;
};

static const SpouseDef kSpouses[4] = {
    { "Mai Linh",    "Mai Linh",    "Village Herbalist (+HP)",       "Thầy Thuốc Làng (+Máu)",         180, 30.0f,  4.0f, 0.04f, 0.10f, { 115, 235, 165, 255 } },
    { "Van Anh",     "Vân Anh",     "Arena Swordswoman (+DMG/SPD)",  "Nữ Kiếm Sư (+Sát Thương/Tốc)",  260, 20.0f,  9.0f, 0.05f, 0.25f, { 255, 120, 145, 255 } },
    { "Thien Thanh", "Thiên Thanh", "City Arcanist (+CRIT/SPD)",     "Pháp Sư Kinh Thành (+Chí Mạng)", 320, 25.0f,  7.0f, 0.08f, 0.20f, { 130, 185, 255, 255 } },
    { "Bao Ngoc",    "Bảo Ngọc",    "Noble Lady (All Blessings)",    "Quận Chúa Cao Quý (Toàn diện)", 500, 45.0f, 12.0f, 0.08f, 0.25f, { 255, 215, 95,  255 } }
};

struct HouseDef {
    const char* nameEn;
    const char* nameVi;
    const char* descEn;
    const char* descVi;
    int cost;
    int maxKids;
    float hpBonus;
    float dmgBonus;
};

static const HouseDef kHouses[4] = {
    { "Homeless",        "Chưa Có Nhà",         "Renting an inn room. Buy a house to marry & raise heirs.",
      "Đang ở trọ. Mua nhà để cưới vợ & sinh con nối dõi.", 0, 0, 0.0f, 0.0f },
    { "Village Cottage", "Nhà Gỗ Làng Mạc",     "Unlocks Marriage & 1 Heir Slot (+20 HP, -15% Hunger)",
      "Mở Cưới Vợ & 1 Ô Sinh Con (+20 Máu, -15% Tốc Đói)", 150, 1, 20.0f, 3.0f },
    { "City Villa",      "Biệt Thự Kinh Thành", "Up to 2 Heirs (+45 HP, +6 DMG, -30% Hunger)",
      "Mở Tối Đa 2 Con (+45 Máu, +6 ST, -30% Tốc Đói)", 450, 2, 45.0f, 6.0f },
    { "Noble Estate",    "Dinh Thự Quý Tộc",    "Grand Estate for 3 Heirs (+80 HP, +12 DMG, -45% Hunger, 50% off Nobles)",
      "Phủ Đệ Hoàng Gia tối đa 3 Con (+80 Máu, +12 ST, -45% Đói; Quý Tộc giảm 50%)", 1000, 3, 80.0f, 12.0f }
};

struct ChildInfo {
    std::string name;
    int heroClass = 0;
    float bonusHp = 0.0f;
    float bonusDmg = 0.0f;
    float bonusArmor = 0.0f;
    float bonusSpeed = 0.0f;
};

static constexpr int kMaxPlayerLevel = 100;

struct LevelTitleDef {
    int minLevel;
    const char* nameEn;
    const char* nameVi;
    const char* bonusEn;
    const char* bonusVi;
    Color color;
    float hpBonus;
    float dmgBonus;
    float armorBonus;
    float critBonus;
    float atkRateBonus;
};

static const LevelTitleDef kLevelTitles[6] = {
    {   1, "Dungeon Novice",           "Tân Binh Hầm Ngục",       "Adventurer Start",                 "Khởi đầu hành trình",             { 180, 195, 220, 255 },   0.0f,  0.0f, 0.0f, 0.00f, 0.00f },
    {  20, "Elite Vanguard",           "Chiến Binh Tinh Anh",     "+30 HP  +5 DMG",                   "+30 Máu  +5 Sát Thương",          { 110, 235, 160, 255 },  30.0f,  5.0f, 0.0f, 0.00f, 0.00f },
    {  40, "Grand Warlord",            "Thống Lĩnh Chiến Trường", "+50 HP  +8 DMG  +1 ARM",           "+50 Máu  +8 ST  +1 Giáp",         {  95, 215, 255, 255 },  50.0f,  8.0f, 1.0f, 0.00f, 0.00f },
    {  60, "Dungeon Overlord",         "Bá Chủ Hầm Ngục",         "+70 HP  +12 DMG  +5% CRIT",        "+70 Máu  +12 ST  +5% CM",         { 205, 120, 255, 255 },  70.0f, 12.0f, 0.0f, 0.05f, 0.00f },
    {  80, "Immortal Demigod",         "Bán Thần Bất Diệt",       "+100 HP +16 DMG +1.5ARM +ATK",     "+100 Máu +16 ST +1.5G +Tốc",      { 255, 145,  55, 255 }, 100.0f, 16.0f, 1.5f, 0.00f, 0.25f },
    { 100, "Eternal Mythic Sovereign", "Thần Thoại Vĩnh Hằng",    "+150 HP +25 DMG +2.5ARM +8%CM",    "+150 Máu +25 ST +2.5G +8%CM",     { 255, 215,  70, 255 }, 150.0f, 25.0f, 2.5f, 0.08f, 0.35f }
};

static int XpRequiredForNextLevel(int lvl) {
    int lv = std::max(1, std::min(kMaxPlayerLevel, lvl));
    if (lv < 10) return 50 + (lv - 1) * 25 + (lv - 1) * (lv - 1) * 6;
    return std::min(4500, 760 + (lv - 10) * 60);
}

struct AncestorRecord {
    int generation = 1;
    std::string name;
    int heroClass = 0;
    int subClass = 0;
    bool wasNoble = false;
    std::string spouseName;
    int floorReached = 1;
    int levelReached = 1;
    int enemiesSlain = 0;
};

static const char* kHeirNames[10] = {
    "Minh Khôi", "Hoàng Long", "Thanh Vân", "Anh Tuấn", "Ngọc Diệp",
    "Gia Huy", "Thảo Nhi", "Bảo Lâm", "Thiên Ân", "Khánh Linh"
};

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

    enum State { PLAYING, LEVEL_UP, GAME_OVER, TITLE, PAUSED, INVENTORY, SETTINGS, SHOP, META, CLASS_SELECT, SAVE_LOAD, SURFACE_UI, FAMILY_TREE };
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
    int namingMode = 0; // 0: off, 1: New Hero, 2: Child Name, 3: New Save Name
    int namingChildIdx = -1;
    std::string nameInputBuf = "";
    std::vector<ChildInfo> children;
    std::vector<AncestorRecord> familyTree;
    float legacyHp = 0.0f;
    float legacyDmg = 0.0f;
    float legacyArmor = 0.0f;
    float legacySpeed = 0.0f;
    int surfaceBuilding = 0; // 0: Quán trọ, 1: Mua nhà, 2: Gia đình & Đời con, 3: Hoàng cung & Vua
    int surfaceSel = 0;
    int surfaceRoom = 0;   // 0: Quảng trường, 1: Quán Trọ, 2: Phường Gia Trang, 3: Tổ Ấm & Từ Đường, 4: Hoàng Cung
    Vector2 surfaceTownReturnPos = { 0.0f, 180.0f };
    State familyTreeReturn = PLAYING;

    const char* L(const char* en, const char* vi) const { return settings.language == 1 ? vi : en; }
    int TimePhase() const {
        if (timeOfDay >= 5.0f && timeOfDay < 8.0f) return 0;  // Dawn
        if (timeOfDay >= 8.0f && timeOfDay < 17.0f) return 1; // Day
        if (timeOfDay >= 17.0f && timeOfDay < 20.0f) return 2; // Dusk
        return 3; // Night
    }
    const char* TimePhaseName() const {
        switch (TimePhase()) {
            case 0: return L("Dawn", "Bình Minh");
            case 1: return L("Day", "Ban Ngày");
            case 2: return L("Dusk", "Hoàng Hôn");
            default: return L("Night", "Ban Đêm");
        }
    }
    std::string ClockString() const {
        int totMin = std::max(0, (int)floorf(timeOfDay * 60.0f)) % (24 * 60);
        int hh = totMin / 60, mm = totMin % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", hh, mm);
        return std::string(buf);
    }
    void AdvanceTime(float hours) {
        if (hours <= 0.0f) return;
        timeOfDay += hours;
        while (timeOfDay >= 24.0f) {
            timeOfDay -= 24.0f;
            dayCount++;
            if (dayCount % 3 == 0) heroAge++;
        }
    }
    void SetHeroName(const std::string& nm) {
        if (!nm.empty()) heroGivenName = nm;
    }
    void SetChildName(int idx, const std::string& nm) {
        if (idx >= 0 && idx < (int)children.size() && !nm.empty()) children[idx].name = nm;
    }
    const char* SpouseUnlockConditionText(int idx) const {
        switch (idx) {
            case 0: return L("Event: Rescue her from Dungeon Cage", "Sự kiện: Giải cứu ở Hầm Ngục");
            case 1: return TextFormat(L("Event/Quest: Rescue or Slay %d/15 Monsters", "Sự kiện/NV: Cứu hoặc Hạ %d/15 Quái"), std::min(15, spouseQuestProgress));
            case 2: return L("Event/Quest: Rescue or Conquer Trial/Boss", "Sự kiện/NV: Cứu hoặc Thắng Thử Thách/Trùm");
            default: return L("Quest: King's Commission or Royal Rescue", "NV/Sự kiện: Hoàn thành Ủy Thác Của Vua");
        }
    }
    void UnlockSpouse(int idx, const char* msgEn, const char* msgVi) {
        if (idx < 0 || idx >= 4 || spouseUnlocked[idx]) return;
        spouseUnlocked[idx] = true;
        PlaySound(snd.sndLevelUp);
        Burst(player.pos, kSpouses[idx].dressCol, 34, 290.0f, 0.85f);
        ShowBanner(L(msgEn, msgVi), 4.2f);
    }
    void RescueCaptiveSpouse() {
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
    void OpenNameModal(int mode, int childOrSlotIdx, const std::string& defName) {
        namingMode = mode;
        namingChildIdx = childOrSlotIdx;
        nameInputBuf = defName;
        StopRumble();
    }
    void ConfirmNameModal() {
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
    void UpdateNameModal() {
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
    void DrawNameModal() {
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
    const char* SpouseName(int idx) const {
        if (idx < 0 || idx >= 4) return L("None", "Chưa có");
        return settings.language == 1 ? kSpouses[idx].nameVi : kSpouses[idx].nameEn;
    }
    const char* HouseName(int tier) const {
        int t = std::max(0, std::min(3, tier));
        return settings.language == 1 ? kHouses[t].nameVi : kHouses[t].nameEn;
    }
    float HouseHungerMult() const {
        if (houseTier <= 0) return 1.0f;
        if (houseTier == 1) return 0.85f;
        if (houseTier == 2) return 0.70f;
        return 0.55f;
    }
    float HungerDamageMult() const {
        return (surfaceUnlocked && hunger >= 70.0f) ? 1.10f : 1.0f;
    }
    int MealPrice(int basePrice) const {
        return isNoble ? std::max(5, (int)roundf(basePrice * 0.70f)) : basePrice;
    }
    int HouseUpgradeCost(int targetTier) const {
        int t = std::max(1, std::min(3, targetTier));
        int base = kHouses[t].cost;
        if (!isNoble) return base;
        return (t == 3) ? base / 2 : (int)roundf(base * 0.70f);
    }
    std::string CurrentHeroDisplayName() const {
        return heroGivenName.empty() ? std::string(ClassDisplayName(player.heroClass)) : heroGivenName;
    }
    void ResetDynasty() {
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
    const ClassDef& BaseClass() const { return kClasses[std::max(0, std::min(3, player.heroClass))]; }
    const SubclassDef* ActiveSubclass() const {
        if (player.subClass != 1 && player.subClass != 2) return nullptr;
        int hc = std::max(0, std::min(3, player.heroClass));
        return &kSubclasses[hc][player.subClass - 1];
    }
    const char* ClassDisplayName(int hc) const {
        hc = std::max(0, std::min(3, hc));
        return settings.language == 1 ? kClassesVi[hc].name : kClasses[hc].name;
    }
    const char* SubDisplayName(int hc, int branch) const {
        hc = std::max(0, std::min(3, hc));
        int b = (branch == 2) ? 1 : 0;
        return settings.language == 1 ? kSubclassesVi[hc][b].name : kSubclasses[hc][b].name;
    }
    const char* HeroTitle() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].name : kSubclasses[hc][player.subClass - 1].name;
        return settings.language == 1 ? kClassesVi[hc].name : kClasses[hc].name;
    }
    static int LevelTitleTierForLevel(int lv) {
        if (lv >= 100) return 5;
        if (lv >= 80)  return 4;
        if (lv >= 60)  return 3;
        if (lv >= 40)  return 2;
        if (lv >= 20)  return 1;
        return 0;
    }
    int CurrentTitleTier() const { return LevelTitleTierForLevel(player.level); }
    const char* LevelTitleNameForLevel(int lv) const {
        int t = LevelTitleTierForLevel(lv);
        return settings.language == 1 ? kLevelTitles[t].nameVi : kLevelTitles[t].nameEn;
    }
    const char* CurrentLevelTitle() const { return LevelTitleNameForLevel(player.level); }
    Color LevelTitleColorForLevel(int lv) const { return kLevelTitles[LevelTitleTierForLevel(lv)].color; }
    Color CurrentLevelTitleColor() const { return LevelTitleColorForLevel(player.level); }
    const char* HeroRole() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].role : kSubclasses[hc][player.subClass - 1].role;
        return settings.language == 1 ? kClassesVi[hc].role : kClasses[hc].role;
    }
    Color HeroColor() const { const SubclassDef* sc = ActiveSubclass(); return sc ? sc->color : BaseClass().color; }
    Color HeroAccent() const { const SubclassDef* sc = ActiveSubclass(); return sc ? sc->accent : BaseClass().accent; }
    const char* PassiveName() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].passiveName : kSubclasses[hc][player.subClass - 1].passiveName;
        return settings.language == 1 ? kClassesVi[hc].passiveName : kClasses[hc].passiveName;
    }
    const char* PassiveDesc() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].passiveDesc : kSubclasses[hc][player.subClass - 1].passiveDesc;
        return settings.language == 1 ? kClassesVi[hc].passiveDesc : kClasses[hc].passiveDesc;
    }
    const char* QName() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].qName : kSubclasses[hc][player.subClass - 1].qName;
        return settings.language == 1 ? kClassesVi[hc].qName : kClasses[hc].qName;
    }
    const char* QDesc() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].qDesc : kSubclasses[hc][player.subClass - 1].qDesc;
        return settings.language == 1 ? kClassesVi[hc].qDesc : kClasses[hc].qDesc;
    }
    const char* EName() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].eName : kSubclasses[hc][player.subClass - 1].eName;
        return settings.language == 1 ? kClassesVi[hc].eName : kClasses[hc].eName;
    }
    const char* EDesc() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].eDesc : kSubclasses[hc][player.subClass - 1].eDesc;
        return settings.language == 1 ? kClassesVi[hc].eDesc : kClasses[hc].eDesc;
    }
    const char* CName() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].cName : kSubclasses[hc][player.subClass - 1].cName;
        return "";
    }
    const char* CDesc() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].cDesc : kSubclasses[hc][player.subClass - 1].cDesc;
        return "";
    }
    const char* VName() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].vName : kSubclasses[hc][player.subClass - 1].vName;
        return "";
    }
    const char* VDesc() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].vDesc : kSubclasses[hc][player.subClass - 1].vDesc;
        return "";
    }
    const char* RName() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].rName : kSubclasses[hc][player.subClass - 1].rName;
        return settings.language == 1 ? kClassesVi[hc].rName : kClasses[hc].rName;
    }
    const char* RDesc() const {
        int hc = std::max(0, std::min(3, player.heroClass));
        if (player.subClass == 1 || player.subClass == 2) return settings.language == 1 ? kSubclassesVi[hc][player.subClass - 1].rDesc : kSubclasses[hc][player.subClass - 1].rDesc;
        return settings.language == 1 ? kClassesVi[hc].rDesc : kClasses[hc].rDesc;
    }
    static int ClassWeaponType(int heroClass) {
        int hc = std::max(0, std::min(3, heroClass));
        return kClasses[hc].startWeapon; // Knight: Sword, Assassin: Dagger, Mage: Staff, Ranger: Bow
    }
    static int VisualWeaponType(int heroClass, int subClass, int itemWeaponType) {
        (void)itemWeaponType;
        int hc = std::max(0, std::min(3, heroClass));
        if (hc == CLASS_MAGE && subClass == 1) return WP_SWORD; // Kiem Su: dung Gay nhung model la Kiem Phep
        return ClassWeaponType(hc);
    }
    bool CanUseWeapon(int weaponType) const { return weaponType == ClassWeaponType(player.heroClass); }
    int PreferredWeaponType() const { return ClassWeaponType(player.heroClass); }
    float CritMult() const { return player.heroClass == CLASS_ASSASSIN ? (player.subClass == 1 ? 2.6f : 2.4f) : 2.0f; }

    float SkillCdrMult() const {
        float mult = std::max(0.45f, 1.0f - 0.12f * player.cdrUpgrades);
        if (player.heroClass == CLASS_MAGE) mult *= (player.subClass == 2 ? 0.78f : 0.85f);
        return mult;
    }
    float LifestealStep() const {
        return settings.difficulty == 0 ? 0.08f : (settings.difficulty == 1 ? 0.02f : 0.005f);
    }
    float LifestealCap() const {
        return settings.difficulty == 0 ? 0.30f : (settings.difficulty == 1 ? 0.10f : 0.03f);
    }
    float LifestealScale() const {
        return settings.difficulty == 0 ? 1.0f : (settings.difficulty == 1 ? 0.25f : 0.0625f);
    }
    float Skill1MaxCd() const { return kClasses[std::max(0, std::min(3, player.heroClass))].qCooldown * SkillCdrMult(); }
    float Skill2MaxCd() const { return kClasses[std::max(0, std::min(3, player.heroClass))].eCooldown * SkillCdrMult(); }
    float Skill3MaxCd() const {
        const SubclassDef* sc = ActiveSubclass();
        return (sc ? sc->cCooldown : 7.5f) * SkillCdrMult();
    }
    float Skill4MaxCd() const {
        const SubclassDef* sc = ActiveSubclass();
        return (sc ? sc->vCooldown : 9.5f) * SkillCdrMult();
    }
    float EffectiveArmor() const { return player.armor + (player.holyBuffTimer > 0.0f ? (player.subClass == 1 ? 6.0f : 4.0f) : 0.0f); }
    float EffectiveAttackRate() const {
        float r = player.attackRate;
        if (player.holyBuffTimer > 0.0f) r *= (player.heroClass == CLASS_KNIGHT && player.subClass == 1 ? 1.30f : 1.25f);
        if (player.heroClass == CLASS_KNIGHT && player.subClass == 2) {
            float missingFrac = 1.0f - Clamp(player.hp / std::max(1.0f, player.maxHp), 0.0f, 1.0f);
            r *= (1.0f + 0.40f * missingFrac);
        }
        if (player.heroClass == CLASS_MAGE && player.subClass == 1 && (player.shieldActive || player.ultActiveTimer > 0.0f || player.swordOrbitTimer > 0.0f)) r *= 1.25f;
        if (player.hasteTimer > 0.0f) {
            if (player.heroClass == CLASS_RANGER) r *= (player.subClass == 1 ? 1.45f : 1.35f);
            else if (player.heroClass == CLASS_KNIGHT || player.heroClass == CLASS_MAGE) r *= 1.30f;
        }
        if (comboCount >= 10) r *= 1.20f; // Frenzy Overdrive
        for (const auto& h : hazards) {
            if (h.kind == 3 && Vector2Distance(player.pos, h.pos) < h.radius) { r *= 1.25f; break; }
        }
        return r;
    }
    float EffectiveMoveSpeed() const {
        float s = player.speed;
        if (player.heroClass == CLASS_ASSASSIN) {
            if (player.ultActiveTimer > 0.0f) s *= (player.subClass == 2 ? 1.45f : 1.35f);
            else if (player.hasteTimer > 0.0f) s *= 1.25f;
        } else if (player.hasteTimer > 0.0f && (player.heroClass == CLASS_KNIGHT || player.heroClass == CLASS_RANGER)) {
            s *= 1.22f;
        }
        if (comboCount >= 10) s *= 1.18f; // Frenzy Overdrive
        if (player.frostSlowTimer > 0.0f) s *= 0.72f;
        if (surfaceUnlocked && hunger < 25.0f) s *= 0.90f;
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
        InitGameFont();
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
        ResetDynasty();
        roomW = 1200;
        roomH = 750;
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

    // ── Bioma: 6 quần xã phong phú đổi màu & cảnh quan mỗi 2 tầng ──
    struct BiomeColors {
        Color bg, tileA, tileB, pathCol, trimCol, accentCol, wall, mote;
        const char* nameEn;
        const char* nameVi;
    };
    int BiomeIdx() const { return ((std::max(1, floor) - 1) / 2) % 6; }
    BiomeColors SurfaceBiome() const {
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
            default: // Quảng Trường Làng Mạc & Kinh Thành (Đá vôi hoàng kim rực nắng)
                return { { 22, 28, 38, 255 }, { 62, 74, 64, 255 }, { 52, 64, 54, 255 }, { 126, 116, 98, 255 },
                         { 245, 205, 100, 255 }, { 110, 225, 255, 255 }, { 124, 112, 92, 255 }, { 255, 242, 180, 255 },
                         "ROYAL CITY SQUARE", "QUẢNG TRƯỜNG KINH THÀNH" };
        }
    }
    BiomeColors Biome() const {
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

    std::string UpgradeDesc(UpgradeType u) {
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

    static Vector2 SurfaceLandmarkPos(int idx) {
        switch (idx) {
            case 0: return { -420.0f, -180.0f }; // Quán Trọ & Chợ Đồ Ăn
            case 1: return {  420.0f, -180.0f }; // Phường Gia Trang (Mua Nhà)
            case 2: return {  420.0f,  180.0f }; // Tổ Ấm Gia Đình (Cưới Vợ / Sinh Con / An Nghỉ)
            case 3: return { -420.0f,  180.0f }; // Hoàng Cung (Quý Tộc & Ủy Thác Của Vua)
            default: return {   0.0f, -385.0f }; // Cổng Trở Lại Hầm Ngục
        }
    }

    static Vector2 SurfaceTownExitSpot(int roomIdx) {
        if (roomIdx < 1 || roomIdx > 4) return { 0.0f, 180.0f };
        Vector2 bp = SurfaceLandmarkPos(roomIdx - 1);
        return { bp.x, bp.y + 84.0f };
    }

    Vector2 SurfaceInteriorExitPos() const {
        return { 0.0f, roomH / 2.0f - 48.0f };
    }

    static Vector2 TavernStallPos(int i) {
        static const Vector2 pts[4] = {
            { -305.0f,  -65.0f },
            { -105.0f, -155.0f },
            {  105.0f, -155.0f },
            {  305.0f,  -65.0f }
        };
        return pts[std::max(0, std::min(3, i))];
    }

    static Vector2 HousingPedestalPos(int tierIdx) {
        if (tierIdx <= 1) return { -300.0f,  -55.0f };
        if (tierIdx == 2) return {    0.0f, -145.0f };
        return { 300.0f, -55.0f };
    }

    static Vector2 SpouseCandPos(int i) {
        static const Vector2 pts[4] = {
            { -355.0f, -185.0f },
            { -185.0f, -225.0f },
            { -185.0f,  -45.0f },
            { -355.0f,   15.0f }
        };
        return pts[std::max(0, std::min(3, i))];
    }

    static Vector2 HeirPosInHome(int k) {
        return { 185.0f + (k % 2) * 155.0f, -45.0f + (k / 2) * 135.0f };
    }

    void SetupSurfaceRoomEnvironment(int roomIdx, bool movePlayer) {
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

    void EnterSurfaceRoom(int roomIdx) {
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

    void LeaveSurfaceRoomToTown() {
        if (!inSurface || surfaceRoom == 0) return;
        surfaceTownReturnPos = SurfaceTownExitSpot(surfaceRoom);
        SetupSurfaceRoomEnvironment(0, true);
        PlaySound(snd.sndCoin);
        Burst(player.pos, SKYBLUE, 12, 140.0f, 0.4f);
        ShowBanner(L("RETURNED TO TOWN SQUARE", "RA QUẢNG TRƯỜNG KINH THÀNH"), 1.8f);
    }

    int SpouseCost(int idx) const {
        (void)idx;
        return 0; // Vợ không mua bằng vàng — chỉ mở khóa qua giải cứu / nhiệm vụ!
    }

    void OpenSurfaceModal(int tab) {
        surfaceBuilding = std::max(0, std::min(3, tab));
        surfaceSel = 0;
        StopRumble();
        state = SURFACE_UI;
        PlaySound(snd.sndCoin);
    }

    void OpenFamilyTree(State from) {
        familyTreeReturn = from;
        StopRumble();
        state = FAMILY_TREE;
        PlaySound(snd.sndCoin);
    }

    void EnterSurface(int nextF) {
        surfaceUnlocked = true;
        inSurface = true;
        nextDungeonFloor = std::max(nextDungeonFloor, std::max(1, nextF));
        surfaceTownReturnPos = { 0.0f, 220.0f };
        SetupSurfaceRoomEnvironment(0, true);
        PlaySound(snd.sndPortal);
        ShowBanner(L("SURFACE REACHED! Explore each building or press [1..4] to step inside!",
                     "ĐÃ LÊN MẶT ĐẤT! Bước vào từng công trình (hoặc nhấn [1..4]) để khám phá không gian riêng!"), 4.2f);
    }

    void LeaveSurfaceToDungeon() {
        inSurface = false;
        roomW = 1200.0f;
        roomH = 750.0f;
        floor = std::max(1, nextDungeonFloor);
        PlaySound(snd.sndPortal);
        BuildFloor();
    }

    void EatRation() {
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

    void BuyMeal(int kind) {
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

    void BuyHouseUpgrade(int targetTier) {
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

    void TryMarrySpouse(int idx) {
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

    void TryHaveChild() {
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

    void AcceptRoyalCommission() {
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

    void CompleteRoyalCommission() {
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

    void BuyNobleTitleDirect() {
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

    void SuccessionToChild(int childIdx) {
        if (childIdx < 0 || childIdx >= (int)children.size()) return;
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
        heroAge = 18;
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

    void BuildPortalChoices() {
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

        if (portalOpen) BuildPortalChoices();
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
            e.eliteAffix = 1 + (rand() % 4);
            e.affixTimer = 2.0f + Rand01() * 2.0f;
            e.maxHp *= (e.eliteAffix == 4 ? 1.8f * 1.15f : 1.8f);
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
        if (state == INVENTORY) {
            { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
            if (IsKeyPressed(KEY_Y)) { OpenFamilyTree(INVENTORY); return; }
            if (IsKeyPressed(KEY_I) || EscPressed() || IsKeyPressed(KEY_ENTER) || PadPressed(GAMEPAD_BUTTON_MIDDLE_LEFT) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) state = prevState;
            return;
        }
        if (state == PLAYING && IsKeyPressed(KEY_Y)) {
            OpenFamilyTree(PLAYING);
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
                if (DamagePlayer(e.contactDmg * contactMul) && e.isElite && e.eliteAffix == 1 && e.hp > 0) {
                    float vh = std::min(e.maxHp - e.hp, e.maxHp * 0.12f);
                    e.hp += vh;
                    if (vh >= 1.0f) SpawnPopup(e.pos, "+VAMP", RED);
                }
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
                        player.xp += std::max(1, (int)roundf(l.value * xpMul));
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
                    if (isNoble) gVal = (int)roundf(gVal * 1.25f);
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

    void DamageBarrel(ArenaBarrel& b, float dmg) {
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

    void TriggerBarrelsInRadius(Vector2 center, float radius, float dmg) {
        for (size_t i = 0; i < barrels.size(); ++i) {
            if (barrels[i].destroyed) continue;
            if (Vector2Distance(center, barrels[i].pos) <= radius + 22.0f) {
                DamageBarrel(barrels[i], dmg);
            }
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

    void DamageEnemy(Enemy& e, float dmg, Vector2 kb, float kbForce, bool isCrit, bool allowRage = true) {
        if (e.type == CHARGER && e.chargeState == 3) dmg *= 1.5f; // choáng sau cú lao: dễ trúng đòn
        if (e.type == BOSS && e.bossKind == 1 && e.chargeState == 5) dmg *= 1.3f; // Colossus kiệt sức: nhận thêm sát thương
        if (e.stunTimer > 0.0f) dmg *= 1.20f; // Kẻ địch đang choáng nhận thêm 20% sát thương
        if (HasRelic(RELIC_BERSERK) && player.hp < player.maxHp * 0.4f) dmg *= 1.4f;
        dmg *= HungerDamageMult();
        e.hp -= dmg;
        e.pos = Vector2Add(e.pos, Vector2Scale(kb, kbForce * 0.05f));
        SpawnPopup(e.pos, (isCrit ? "!" : "") + std::to_string((int)dmg), isCrit ? YELLOW : WHITE);
        e.hitFlash = 0.09f;
        Burst(e.pos, isCrit ? Color{ 255, 217, 51, 255 } : WHITE, isCrit ? 9 : 4, isCrit ? 230.0f : 160.0f, isCrit ? 0.4f : 0.28f);
        if (isCrit) hitStop = 0.035f;
        if (allowRage) AddRage((isCrit ? 4.0f : 2.5f) + (comboCount >= 10 ? 1.5f : 0.0f));

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

        // Combo gold payout every 5 kills
        if (comboCount % 5 == 0) {
            int cg = comboCount * 2;
            if (isNoble) cg = (int)roundf(cg * 1.25f);
            player.gold += cg;
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

    bool DamagePlayer(float dmg) {
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

        float effArmor = EffectiveArmor();
        float effDmg = dmg * 100.0f / (100.0f + effArmor * 10.0f);
        if (player.heroClass == CLASS_KNIGHT) effDmg *= (player.subClass == 1 ? 0.80f : 0.85f);
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
        return true;
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

    void CheckAndGrantLevelTitleMilestones() {
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

    void TriggerLevelUp() {
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

    void ChooseSubclass(int branch) {
        int hc = std::max(0, std::min(3, player.heroClass));
        int b = (branch == 2) ? 2 : 1;
        player.subClass = b;
        subclassChoosing = false;
        const SubclassDef& sc = kSubclasses[hc][b - 1];

        player.maxHp += sc.bonusHp;
        player.hp = std::min(player.maxHp, player.hp + player.maxHp * 0.35f + sc.bonusHp);
        player.baseDmg += sc.bonusDmg;
        player.armor += sc.bonusArmor;
        player.speed += sc.bonusSpeed;
        player.attackRate += sc.bonusAtkRate;
        player.critChance = std::min(0.6f, player.critChance + sc.bonusCrit);
        player.lifesteal = std::min(LifestealCap() + 0.05f * LifestealScale(), player.lifesteal + sc.bonusLifesteal * LifestealScale());
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
            equipped[SLOT_WEAPON].damage += 4.0f;
            player.baseDmg += 4.0f;
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

    void ChooseUpgrade(UpgradeType up) {
        if (subclassChoosing) {
            int autoBranch = (player.heroClass == CLASS_MAGE) ? 1 : ((enemiesSlain % 2) + 1);
            ChooseSubclass(autoBranch);
            return;
        }
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
        else if (up == UP_LIFESTEAL) { player.lifesteal = std::min(LifestealCap(), player.lifesteal + LifestealStep()); }
        else if (up == UP_ARMOR) { player.armor += 2.0f; }
        else if (up == UP_CRIT) { player.critChance = std::min(0.6f, player.critChance + 0.08f); }
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

    static void DrawWeaponModel(Vector2 hand, Vector2 dir, int wt, float t, bool enchanted = false) {
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
            Tri(tip, { tip.x - dir.x * 9.0f + perp.x * 5.0f, tip.y - dir.y * 9.0f + perp.y * 5.0f }, { tip.x - dir.x * 9.0f - perp.x * 5.0f, tip.y - dir.y * 9.0f - perp.y * 5.0f }, enchanted ? SKYBLUE : Color{ 215, 52, 68, 255 });
            Diamond(Vector2Add(tip, Vector2Scale(dir, 7.0f)), 4.0f, 4.0f, { 220, 232, 248, 255 }, ink);
            Tri(Vector2Add(tip, Vector2Scale(dir, 18.0f)),
                { tip.x + perp.x * 5.5f, tip.y + perp.y * 5.5f },
                { tip.x - perp.x * 5.5f, tip.y - perp.y * 5.5f }, enchanted ? Color{ 125, 240, 255, 255 } : Color{ 238, 244, 255, 255 });
            DrawLineEx(tip, Vector2Add(tip, Vector2Scale(dir, 18.0f)), 1.6f, WHITE);
        } else {
            // Sword / Dagger / Enchanted Runeblade: lưỡi đa giác vát nhọn + hào quang phép nếu Kiếm Sư
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
            DrawLineEx(baseBlade, tip, 1.6f, enchanted ? Color{ 220, 150, 255, 255 } : (isDagger ? Color{ 255, 85, 130, 255 } : WHITE));
            DrawLineEx({ hand.x + perp.x * (isDagger ? 6.0f : 9.0f), hand.y + perp.y * (isDagger ? 6.0f : 9.0f) },
                       { hand.x - perp.x * (isDagger ? 6.0f : 9.0f), hand.y - perp.y * (isDagger ? 6.0f : 9.0f) }, 4.2f, enchanted ? Color{ 165, 110, 255, 255 } : GOLD);
            DrawLineEx(Vector2Subtract(hand, Vector2Scale(dir, 6.0f)), hand, 3.6f, { 95, 60, 36, 255 });
        }
        float hdeg = atan2f(dir.y, dir.x) * RAD2DEG;
        BoxRot(hand, 7.5f, 7.5f, hdeg, enchanted ? Color{ 110, 230, 255, 255 } : Color{ 150, 160, 188, 255 }, ink, 1.3f);
    }

    static void DrawHeroModel(Vector2 p, int heroClass, int wt, Vector2 aimDir, bool facingLeft, float walkTimer, bool moving,
                              float atkSweep, bool isInvuln, bool shieldActive, float holyBuffTimer, float ultActiveTimer,
                              int subClass = 0, float swordOrbitTimer = 0.0f) {
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

        // Vũ khí theo hướng ngắm (Class quyết định model; Kiếm Sư dùng Gậy nhưng hiển thị model Kiếm Phép!)
        int visWt = VisualWeaponType(heroClass, subClass, wt);
        bool enchanted = (subClass > 0 && ((heroClass == CLASS_MAGE && subClass == 1) || (heroClass == CLASS_RANGER && subClass == 2)));
        float a = atan2f(aimDir.y, aimDir.x);
        float sa = a + atkSweep;
        Vector2 dir = { cosf(sa), sinf(sa) };
        Vector2 hand = { body.x + dir.x * 16.0f, body.y + dir.y * 16.0f };
        DrawWeaponModel(hand, dir, visWt, t, enchanted);

        if (subClass == 1 || subClass == 2) {
            const SubclassDef& sc = kSubclasses[std::max(0, std::min(3, heroClass))][subClass - 1];
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

    void DrawPlayerSprite() {
        const bool moving = player.walkTimer > 0.01f && (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)
                              || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || (ctl.active && Vector2Length(ctl.move) > 0.1f));
        int wt = hasEquip[SLOT_WEAPON] ? equipped[SLOT_WEAPON].weaponType : PreferredWeaponType();
        int visWt = VisualWeaponType(player.heroClass, player.subClass, wt);
        float effRate = EffectiveAttackRate();
        float sinceAtk = (1.0f / std::max(0.1f, effRate)) - player.attackTimer;
        float off = 0.5f;
        if (visWt == WP_BOW || visWt == WP_STAFF) off = 0.0f;
        else if (player.attackTimer > 0.0f && sinceAtk < 0.16f) off = Lerp(-1.25f, 1.25f, sinceAtk / 0.16f);
        DrawHeroModel(player.pos, player.heroClass, wt, aimDirDraw, player.facingLeft, player.walkTimer, moving, off, player.isInvuln, player.shieldActive, player.holyBuffTimer, player.ultActiveTimer, player.subClass, player.swordOrbitTimer);
    }

    // Độ khó theo tầng: tăng đều, rồi dốc hơn từ tầng 9 để theo kịp sức mạnh của người chơi (cân bằng bằng bot mô phỏng)
    float DiffHp() const { return settings.difficulty == 0 ? 0.90f : (settings.difficulty == 2 ? 1.20f : 1.0f); }
    float DiffDmg() const { return settings.difficulty == 0 ? 0.75f : (settings.difficulty == 2 ? 1.20f : 1.0f); }
    float DiffSouls() const { return settings.difficulty == 0 ? 0.75f : (settings.difficulty == 2 ? 1.5f : 1.0f); }
    float HpMult() const { return (1.0f + (floor - 1) * 0.15f + std::max(0, floor - 8) * 0.14f) * DiffHp(); }
    float DmgMult() const { return (1.0f + (floor - 1) * 0.10f + std::max(0, floor - 8) * 0.11f) * DiffDmg(); }
    float EliteChance() const { return std::min(0.45f, 0.20f + std::max(0, floor - 8) * 0.015f); }

    // ── Nút bấm trên màn hình (dùng được cho chuột lẫn chạm) ──
    enum UiAction { UI_NONE = 0, UI_START, UI_SAVE_LOAD, UI_SANCT, UI_SETTINGS, UI_HELP, UI_QUIT, UI_RESUME, UI_EQUIP, UI_RESTART, UI_BACK, UI_CLOSE_HELP, UI_CLOSE_INV, UI_CHANGE_CLASS, UI_MAIN_MENU, UI_FAMILY_TREE, UI_CONTINUE_HEIR, UI_SURFACE_RETURN };
    struct UiBtn { Rectangle r; std::string label; int action; Color col; };

    std::vector<UiBtn> BuildButtons() {
        std::vector<UiBtn> b;
        Color green = { 36, 104, 62, 255 }, teal = { 36, 92, 118, 255 }, blue = { 36, 60, 120, 255 }, purple = { 70, 50, 120, 255 }, red = { 120, 40, 50, 255 }, grey = { 44, 50, 72, 255 };
        switch (state) {
            case TITLE:
                if (showHelp) {
                    b.push_back({ { 540, 650, 200, 44 }, L("CLOSE", "ĐÓNG"), UI_CLOSE_HELP, grey });
                } else {
                    b.push_back({ { 470, 392, 340, 56 }, L("NEW GAME", "CHƠI MỚI"), UI_START, green });
                    b.push_back({ { 470, 458, 340, 48 }, L("LOAD GAME (4 SLOTS)", "TẢI GAME (4 Ô LƯU)"), UI_SAVE_LOAD, teal });
                    b.push_back({ { 470, 516, 166, 44 }, L("SANCTUARY", "THÁNH ĐIỆN"), UI_SANCT, purple });
                    b.push_back({ { 644, 516, 166, 44 }, L("SETTINGS", "CÀI ĐẶT"), UI_SETTINGS, blue });
                    b.push_back({ { 470, 570, 166, 40 }, L("CONTROLS", "ĐIỀU KHIỂN"), UI_HELP, grey });
#ifndef __ANDROID__
                    b.push_back({ { 644, 570, 166, 40 }, L("QUIT", "THOÁT"), UI_QUIT, red });
#endif
                }
                break;
            case PAUSED:
                if (showHelp) {
                    b.push_back({ { 540, 650, 200, 44 }, L("CLOSE", "ĐÓNG"), UI_CLOSE_HELP, grey });
                } else {
                    float y = 202;
                    b.push_back({ { 480, y, 320, 34 }, L("RESUME", "TIẾP TỤC"), UI_RESUME, green }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("SAVE / LOAD GAME", "LƯU / TẢI GAME"), UI_SAVE_LOAD, teal }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("EQUIPMENT & SKILLS", "TRANG BỊ & KỸ NĂNG"), UI_EQUIP, blue }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("FAMILY TREE [Y]", "CÂY GIA PHẢ [Y]"), UI_FAMILY_TREE, purple }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("CHANGE HERO", "ĐỔI TƯỚNG"), UI_CHANGE_CLASS, purple }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("SETTINGS", "CÀI ĐẶT"), UI_SETTINGS, blue }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("RESTART RUN", "CHƠI LẠI"), UI_RESTART, purple }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("CONTROLS", "ĐIỀU KHIỂN"), UI_HELP, grey }); y += 38;
                    b.push_back({ { 480, y, 320, 34 }, L("MAIN MENU", "MENU CHÍNH"), UI_MAIN_MENU, red });
                }
                break;
            case GAME_OVER:
                if (!children.empty()) {
                    b.push_back({ { 430, 444, 420, 42 }, L("CONTINUE AS HEIR (REST IN PEACE)", "TIẾP TỤC ĐỜI CON (REST IN PEACE)"), UI_CONTINUE_HEIR, green });
                }
                b.push_back({ { 510, 496, 260, 44 }, L("PLAY AGAIN", "CHƠI LẠI"), UI_RESTART, red });
                b.push_back({ { 510, 548, 260, 40 }, L("CHANGE HERO", "ĐỔI TƯỚNG"), UI_CHANGE_CLASS, blue });
                b.push_back({ { 510, 596, 260, 40 }, L("SANCTUARY", "THÁNH ĐIỆN"), UI_SANCT, purple });
                b.push_back({ { 510, 644, 260, 40 }, L("LOAD SAVE", "TẢI BẢN LƯU"), UI_SAVE_LOAD, teal });
                break;
            case INVENTORY:
                b.push_back({ { 856, 622, 174, 40 }, L("FAMILY TREE [Y]", "CÂY GIA PHẢ [Y]"), UI_FAMILY_TREE, purple });
                b.push_back({ { 1040, 622, 146, 40 }, L("CLOSE", "ĐÓNG"), UI_CLOSE_INV, grey });
                break;
            case META:
            case SHOP:
            case SAVE_LOAD:
            case SURFACE_UI:
            case FAMILY_TREE:
                b.push_back({ { 30, 28, 150, 44 }, L("< BACK", "< QUAY LẠI"), UI_BACK, grey });
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
            case UI_FAMILY_TREE: showHelp = false; OpenFamilyTree(state); break;
            case UI_CONTINUE_HEIR: showHelp = false; if (!children.empty()) SuccessionToChild(0); break;
            case UI_BACK:
                if (state == META) CloseSanctuary();
                else if (state == SHOP || state == SURFACE_UI) state = PLAYING;
                else if (state == FAMILY_TREE) state = familyTreeReturn;
                else if (state == SAVE_LOAD) state = saveLoadReturn;
                break;
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
        ctl.skill3 = false;
        ctl.skill4 = false;
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
        ctl.attack = ctl.dash = ctl.nova = ctl.skill2 = ctl.skill3 = ctl.skill4 = ctl.ult = false;
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

    // Helper phóng kiếm khí trăng khuyết (Kiếm Sư / Cuồng Chiến)
    void FireSwordWave(Vector2 origin, Vector2 dir, float dmg, float spd, int pierce, Color col, float rad = 13.0f, bool applyBurn = false) {
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

    // Đòn đánh cơ bản theo vũ khí đặc trưng từng class + hiệu ứng nhánh phụ
    void MeleeAttack(Vector2 aimDir) {
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
                float cc = std::min(0.85f, player.critChance + (eagleMark ? (player.subClass == 1 ? 0.15f : 0.10f) : 0.0f));
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

    // ── Nội tại Pháp Sư / Đại Pháp Sư: mỗi lần dùng kỹ năng bắn 3 (hoặc 5) tia ma thuật dẫn đường ──
    void TriggerArcaneResonance(Vector2 aimDir) {
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

    // ── Kỹ năng 1 [Q] ──
    void CastSkill1(Vector2 aimDir) {
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
        } else {
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
        }
    }

    // ── Kỹ năng 2 [E] ──
    void CastSkill2(Vector2 aimDir, Vector2 moveDir) {
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
        } else {
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
        }
    }

    // ── Kỹ năng Nhánh Phụ 3 [C / Z] ──
    void CastSkill3(Vector2 aimDir) {
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
        } else {
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
        }
    }

    // ── Kỹ năng Nhánh Phụ 4 [V / X] ──
    void CastSkill4(Vector2 aimDir, Vector2 moveDir) {
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
        } else {
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
        }
    }

    // ── Kỹ năng Nộ [R] ──
    void CastUltimate(Vector2 aimDir) {
        player.rage = 0.0f;
        PlaySound(snd.sndSpecial);
        PlaySound(snd.sndLevelUp);
        Shake(0.45f, 9.0f);
        hitStop = 0.06f;
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
        } else {
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
        }
    }

    void UpdateActiveSkills(float dt) {
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
                    Vector2 c = (i == 0) ? Vector2{ -330, -110 } : (i == 1 ? Vector2{ 330, -110 } : Vector2{ 0, 40 });
                    pillars.push_back({ c.x - 58.0f, c.y });
                    pillars.push_back({ c.x + 58.0f, c.y });
                    pillars.push_back({ c.x, c.y + 92.0f });
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
                    d.item = GenerateItem(floor + 1, ClassWeaponType(player.heroClass), LifestealScale());
                    for (int tries = 0; tries < 30 && d.item.rarity < 1; ++tries) d.item = GenerateItem(floor + 1, ClassWeaponType(player.heroClass), LifestealScale());
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
            shopSlots[i].item = GenerateItem(floor, ClassWeaponType(player.heroClass), LifestealScale());
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
        if (s.potion && player.hp >= player.maxHp) { ShowBanner(L("HP is already full!", "Máu đã đầy!"), 1.5f); return; }
        if (!s.potion && hasEquip[s.item.slot] && s.item.Score() <= equipped[s.item.slot].Score()) {
            ShowBanner(L("Not an upgrade - you already wear better!", "Bạn đang mặc món mạnh hơn rồi!"), 2.0f);
            return;
        }
        if (player.gold < s.price) { ShowBanner(L("Not enough Gold!", "Không đủ Vàng!"), 1.5f); PlaySound(snd.sndHurt); return; }
        player.gold -= s.price;
        if (s.potion) {
            player.hp = std::min(player.maxHp, player.hp + s.heal);
            ShowBanner(std::string(L("Healed ", "Đã hồi ")) + std::to_string((int)s.heal) + L(" HP!", " Máu!"), 2.0f);
        } else {
            int soldOld = 0;
            if (TryEquipItem(s.item, soldOld)) ShowBanner(std::string(L("EQUIPPED: ", "ĐÃ TRANG BỊ: ")) + equipped[s.item.slot].name, 2.5f);
            else { player.gold += s.item.SellValue(); ShowBanner(std::string(L("Bought & sold ", "Đã mua & bán ")) + s.item.name, 2.0f); }
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
        const char* title = L("MERCHANT'S SHOP", "CỬA HÀNG THƯƠNG NHÂN");
        OutlinedText(title, 1280 / 2 - MeasureText(title, 44) / 2, 60, 44, GOLD, { 90, 50, 10, 255 });
        std::string g = std::string(L("Gold: ", "Vàng: ")) + std::to_string(player.gold);
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
                DrawText(L("Healing Potion", "Bình Hồi Máu"), (int)r.x + 12, (int)r.y + 124, 17, rc);
                DrawText(TextFormat(L("Heals %d HP", "Hồi %d Máu"), (int)s.heal), (int)r.x + 12, (int)r.y + 156, 16, RAYWHITE);
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
                const char* soldTxt = L("SOLD", "ĐÃ BÁN");
                DrawText(soldTxt, (int)r.x + 100 - MeasureText(soldTxt, 28) / 2, (int)r.y + 150, 28, { 255, 120, 120, 255 });
            } else {
                bool afford = player.gold >= s.price;
                DrawRectangle((int)r.x + 10, (int)r.y + 296, 180, 32, afford ? Color{ 30, 80, 40, 255 } : Color{ 80, 30, 30, 255 });
                DrawRectangleLines((int)r.x + 10, (int)r.y + 296, 180, 32, afford ? Color{ 120, 235, 140, 255 } : Color{ 235, 120, 120, 255 });
                std::string pr = std::to_string(s.price) + (settings.language == 1 ? " Vàng" : " g");
                DrawText(pr.c_str(), (int)r.x + 100 - MeasureText(pr.c_str(), 20) / 2, (int)r.y + 302, 20, afford ? Color{ 255, 230, 120, 255 } : Color{ 255, 160, 160, 255 });
            }
        }
        const char* foot = L("1-5 / click / Enter: buy      A/D: select      Esc or F: leave", "1-5 / nhấp chuột / Enter: mua      A/D: chọn      Esc hoặc F: rời đi");
        DrawText(foot, 1280 / 2 - MeasureText(foot, 18) / 2, 550, 18, { 150, 165, 200, 255 });
        const char* foot2 = L("Gear you buy is equipped automatically if it beats what you wear.", "Trang bị vừa mua sẽ tự động mặc nếu mạnh hơn món hiện tại.");
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
        const char* title = L("SANCTUARY", "THÁNH ĐIỆN");
        OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 40, 48, { 190, 150, 255, 255 }, { 40, 20, 80, 255 });
        const char* sub = L("Spend Souls on upgrades that last across every run", "Dùng Linh Hồn để nâng cấp chỉ số vĩnh viễn qua mọi lượt chơi");
        DrawText(sub, 1280 / 2 - MeasureText(sub, 18) / 2, 94, 18, { 160, 150, 200, 255 });

        // Linh hồn
        std::string sv = std::string(L("Souls: ", "Linh Hồn: ")) + std::to_string(souls);
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
            DrawText(settings.language == 1 ? kMetaVi[i][0] : kMeta[i].name, (int)r.x + 50, (int)r.y + 6, 20, maxed ? Color{ 255, 215, 90, 255 } : WHITE);
            DrawText(settings.language == 1 ? kMetaVi[i][1] : kMeta[i].desc, (int)r.x + 50, (int)r.y + 29, 14, { 170, 180, 210, 255 });
            // Các ô cấp độ
            int pips = kMeta[i].maxLevel;
            for (int k = 0; k < pips; ++k)
                DrawRectangle((int)r.x + 430 + k * 16, (int)r.y + 18, 12, 14, k < meta[i] ? Color{ 190, 150, 255, 255 } : Color{ 48, 44, 80, 255 });
            // Giá
            Rectangle b = { r.x + r.width - 140.0f, r.y + 8.0f, 126.0f, 34.0f };
            if (maxed) {
                DrawRectangleRec(b, { 70, 56, 20, 255 });
                DrawRectangleLinesEx(b, 1.0f, GOLD);
                const char* maxTxt = L("MAX", "TỐI ĐA");
                DrawText(maxTxt, (int)(b.x + b.width / 2) - MeasureText(maxTxt, 18) / 2, (int)b.y + 8, 18, GOLD);
            } else {
                DrawRectangleRec(b, afford ? Color{ 30, 80, 60, 255 } : Color{ 70, 30, 40, 255 });
                DrawRectangleLinesEx(b, 1.0f, afford ? Color{ 120, 235, 170, 255 } : Color{ 235, 120, 130, 255 });
                std::string c = std::to_string(MetaCost(i));
                DrawText(c.c_str(), (int)(b.x + b.width / 2) - MeasureText(c.c_str(), 20) / 2, (int)b.y + 7, 20, afford ? Color{ 200, 255, 220, 255 } : Color{ 255, 170, 170, 255 });
            }
        }
        if (metaMsgTimer > 0) DrawText(metaMsg.c_str(), 1280 / 2 - MeasureText(metaMsg.c_str(), 22) / 2, 640, 22, GOLD);
        const char* foot = L("1-8 / click / Enter: buy      W/S: select      Esc: back", "1-8 / nhấp chuột / Enter: nâng cấp      W/S: chọn      Esc: quay lại");
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
            else if (k == "language") settings.language = std::max(0, std::min(1, v));
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
          << "\nlanguage=" << settings.language
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
            if (IsWindowFullscreen()) ToggleFullscreen();
            if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();

            int mon = GetCurrentMonitor();
            int mw = GetMonitorWidth(mon), mh = GetMonitorHeight(mon);
            Vector2 mpos = GetMonitorPosition(mon);

            if (settings.windowMode == 1) {
                // Windowed Fullscreen thực sự: không viền và cao hơn màn hình 1px để driver OpenGL không tự chiếm Exclusive Fullscreen (Alt+Tab tức thì!)
                SetWindowState(FLAG_WINDOW_UNDECORATED);
                SetWindowPosition((int)mpos.x, (int)mpos.y);
                SetWindowSize(mw, mh + 1);
            } else if (settings.windowMode == 2) {
                ClearWindowState(FLAG_WINDOW_UNDECORATED);
                SetWindowSize(mw, mh);
                if (!IsWindowFullscreen()) ToggleFullscreen();
            } else {
                ClearWindowState(FLAG_WINDOW_UNDECORATED);
                SetWindowSize(settings.resW, settings.resH);
                SetWindowPosition((int)mpos.x + std::max(0, (mw - settings.resW) / 2), (int)mpos.y + std::max(0, (mh - settings.resH) / 2));
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
            case 11: settings.language = (settings.language + d + 2) % 2; break;
        }
    }

    std::string SettingValue(int row) {
#ifdef __ANDROID__
        if (row <= 2) return L("Auto", "Tự động");
#endif
        switch (row) {
            case 0: return settings.windowMode == 1 ? L("Desktop (Borderless)", "Màn hình (Không viền)") : (settings.windowMode == 2 ? L("Native (Fullscreen)", "Gốc (Toàn màn hình)") : std::to_string(settings.resW) + " x " + std::to_string(settings.resH));
            case 1: return settings.windowMode == 0 ? L("Windowed", "Cửa sổ") : (settings.windowMode == 1 ? L("Windowed Fullscreen", "Toàn màn hình không viền") : L("Fullscreen", "Toàn màn hình"));
            case 2: return settings.vsync ? L("ON", "BẬT") : L("OFF", "TẮT");
            case 3: return kFpsOptions[settings.fpsIndex] == 0 ? L("Unlimited", "Không giới hạn") : std::to_string(kFpsOptions[settings.fpsIndex]);
            case 4: return settings.showFps ? L("ON", "BẬT") : L("OFF", "TẮT");
            case 5: return settings.vibration ? L("ON", "BẬT") : L("OFF", "TẮT");
            case 6: return settings.lighting ? L("ON", "BẬT") : L("OFF", "TẮT");
            case 7: return settings.particles == 0 ? L("Low", "Thấp") : (settings.particles == 1 ? L("Medium", "Trung bình") : L("High", "Cao"));
            case 8: return settings.vignette ? L("ON", "BẬT") : L("OFF", "TẮT");
            case 9: return std::to_string(settings.volume * 10) + "%";
            case 10: return settings.difficulty == 0 ? L("Easy", "Dễ") : (settings.difficulty == 1 ? L("Normal", "Thường") : L("Hard", "Khó"));
            case 11: return settings.language == 1 ? "Tiếng Việt" : "English";
        }
        return "";
    }

    void UpdateSettingsMenu() {
        const int rows = 13; // 12 mục + Quay lại
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) setSel = (setSel + 1) % rows;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) setSel = (setSel + rows - 1) % rows;
        int delta = 0;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) delta = 1;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) delta = -1;

        Vector2 m = GetMousePosition();
        for (int i = 0; i < rows; ++i) {
            Rectangle r = { 330.0f, 120.0f + i * 34.0f, 620.0f, 30.0f };
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
            if (setSel == 12) { state = settingsReturn; return; }
            ChangeSetting(setSel, delta);
            SaveSettings();
            ApplySettings();
        }
    }

    void DrawSettingsScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.86f));
        const char* title = L("SETTINGS", "CÀI ĐẶT");
        OutlinedText(title, 1280 / 2 - MeasureText(title, 48) / 2, 54, 48, GOLD, { 90, 50, 10, 255 });
        static const char* labelsEn[12] = { "Resolution", "Display Mode", "VSync", "FPS Limit", "Show FPS", "Vibration (shake + controller)", "Lighting", "Particles", "Vignette", "Volume", "Difficulty (next floors)", "Language" };
        static const char* labelsVi[12] = { "Độ Phân Giải", "Chế Độ Màn Hình", "Đồng Bộ Dọc (VSync)", "Giới Hạn FPS", "Hiện FPS", "Rung (màn hình + tay cầm)", "Ánh Sáng", "Hiệu Ứng Hạt", "Viền Tối Màn Hình", "Âm Lượng", "Độ Khó (tầng tiếp theo)", "Ngôn Ngữ" };
        for (int i = 0; i < 13; ++i) {
            Rectangle r = { 330.0f, 120.0f + i * 34.0f, 620.0f, 30.0f };
            bool sel = (setSel == i);
            DrawRectangleRec(r, sel ? Color{ 40, 52, 84, 255 } : Color{ 20, 26, 42, 255 });
            if (sel) DrawRectangleLinesEx(r, 2.0f, GOLD);
            if (i < 12) {
                DrawText(settings.language == 1 ? labelsVi[i] : labelsEn[i], (int)r.x + 16, (int)r.y + 5, 19, sel ? WHITE : Color{ 190, 205, 235, 255 });
                std::string v = SettingValue(i);
                bool isOn = (v == "ON" || v == "BẬT");
                bool isOff = (v == "OFF" || v == "TẮT");
                Color vc = isOn ? Color{ 120, 235, 140, 255 } : (isOff ? Color{ 255, 130, 130, 255 } : Color{ 255, 215, 90, 255 });
                if (i == 1) vc = (settings.windowMode == 1) ? Color{ 110, 235, 195, 255 } : (settings.windowMode == 2 ? Color{ 120, 235, 140, 255 } : SKYBLUE);
                int vw = MeasureText(v.c_str(), 19);
                if (i == 10) vc = settings.difficulty == 0 ? Color{ 120, 235, 140, 255 } : (settings.difficulty == 2 ? Color{ 255, 120, 110, 255 } : Color{ 255, 215, 90, 255 });
                if (i == 11) vc = Color{ 130, 220, 255, 255 };
                DrawText(v.c_str(), (int)(r.x + r.width) - 40 - vw, (int)r.y + 5, 19, vc);
                if (sel) {
                    DrawText("<", (int)(r.x + r.width) - 40 - vw - 22, (int)r.y + 5, 19, GOLD);
                    DrawText(">", (int)(r.x + r.width) - 28, (int)r.y + 5, 19, GOLD);
                }
                if (i == 9) { // thanh âm lượng
                    for (int b = 0; b < 10; ++b)
                        DrawRectangle((int)r.x + 220 + b * 12, (int)r.y + 8, 9, 14, b < settings.volume ? Color{ 255, 215, 90, 255 } : Color{ 50, 58, 84, 255 });
                }
            } else {
                const char* back = L("Back", "Quay Lại");
                DrawText(back, (int)(r.x + r.width / 2) - MeasureText(back, 21) / 2, (int)r.y + 5, 21, sel ? WHITE : Color{ 190, 205, 235, 255 });
            }
        }
        const char* f1 = L("W/S or mouse: select     A/D, Enter or click: change     Esc: back", "W/S hoặc chuột: chọn     A/D, Enter hoặc nhấp: đổi     Esc: quay lại");
        const char* f2 = L("Resolution applies in windowed mode; fullscreen uses your desktop resolution. Settings are saved automatically.", "Độ phân giải áp dụng ở chế độ Cửa sổ; Toàn màn hình dùng độ phân giải màn hình chính. Cài đặt tự lưu.");
        DrawText(f1, 1280 / 2 - MeasureText(f1, 16) / 2, 582, 16, { 150, 165, 200, 255 });
        DrawText(f2, 1280 / 2 - MeasureText(f2, 14) / 2, 610, 14, { 120, 135, 165, 255 });
    }

    // Vẽ khung 1280x720 lên cửa sổ với viền đen giữ tỉ lệ, và ánh xạ chuột về toạ độ ảo
    void Present() {
        int sw = GetScreenWidth(), sh = GetScreenHeight();
#ifndef __ANDROID__
        if (settings.windowMode == 1) {
            int mon = GetCurrentMonitor();
            sw = GetMonitorWidth(mon);
            sh = GetMonitorHeight(mon);
        }
#endif
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

    // Mặc nếu mạnh hơn món đang đeo và đúng loại vũ khí của class (món cũ bán lấy vàng).
    bool TryEquipItem(const EquipItem& it, int& soldGold) {
        soldGold = 0;
        EquipItem item = it;
        const SubclassDef* sc = ActiveSubclass();
        if (item.slot == SLOT_WEAPON) {
            int cw = ClassWeaponType(player.heroClass);
            if (!CanUseWeapon(item.weaponType)) {
                item.weaponType = cw;
                static const char* weaponNames[8] = { "Sword", "Dagger", "Axe", "Spear", "Hammer", "Scythe", "Bow", "Staff" };
                item.name = std::string(item.RarityName()) + " " + weaponNames[cw];
            }
            if (sc != nullptr) {
                item.name = std::string(item.RarityName()) + " " + sc->weaponTitle;
            }
        }
        int s = item.slot;
        if (hasEquip[s]) {
            if (item.Score() <= equipped[s].Score()) return false;
            ApplyItem(equipped[s], -1.0f);
            soldGold = equipped[s].SellValue();
            player.gold += soldGold;
        }
        equipped[s] = item;
        hasEquip[s] = true;
        ApplyItem(item, 1.0f);
        return true;
    }

    void SpawnItemDrop(Vector2 pos) {
        ItemDrop d;
        d.pos = Vector2Add(pos, { (Rand01() - 0.5f) * 30.0f, (Rand01() - 0.5f) * 30.0f });
        d.item = GenerateItem(floor, ClassWeaponType(player.heroClass), LifestealScale());
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
                    ShowBanner("EQUIPPED: " + equipped[d.item.slot].name + (sold > 0 ? "  (old gear sold +" + std::to_string(sold) + "g)" : ""), 3.0f);
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
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.84f));
        int px = 60, py = 24, pw = 1160, ph = 674;
        DrawRectangle(px, py, pw, ph, { 14, 18, 30, 248 });
        DrawRectangleLines(px, py, pw, ph, GOLD);
        std::string title = std::string(L("EQUIPMENT & SKILLS  -  ", "TRANG BỊ & KỸ NĂNG  -  ")) + HeroTitle() + "  [" + CurrentLevelTitle() + "]";
        DrawText(title.c_str(), 1280 / 2 - MeasureText(title.c_str(), 26) / 2, py + 12, 26, CurrentLevelTitleColor());
        std::string lvSub = TextFormat(L("Level %d / %d%s", "Cấp %d / %d%s"), player.level, kMaxPlayerLevel, player.level >= kMaxPlayerLevel ? L(" (MAX LEVEL)", " (CẤP TỐI ĐA)") : "");
        DrawText(lvSub.c_str(), 1280 / 2 - MeasureText(lvSub.c_str(), 14) / 2, py + 41, 14, player.level >= kMaxPlayerLevel ? GOLD : SKYBLUE);

        const char* slotNames[4] = { L("WEAPON", "VŨ KHÍ"), L("ARMOR", "ÁO GIÁP"), L("BOOTS", "GIÀY"), L("RING", "NHẪN") };
        int cw = 266, gap = 14, cx0 = px + (pw - (cw * 4 + gap * 3)) / 2, cy = py + 60, ch = 196;
        for (int i = 0; i < 4; ++i) {
            int cx = cx0 + i * (cw + gap);
            Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 70, 80, 105, 255 };
            DrawRectangle(cx, cy, cw, ch, { 22, 28, 44, 255 });
            DrawRectangleLines(cx, cy, cw, ch, rc);
            DrawRectangle(cx, cy, cw, 24, Fade(rc, 0.25f));
            DrawText(slotNames[i], cx + 12, cy + 4, 16, rc);
            if (hasEquip[i]) {
                DrawCircleV({ (float)cx + cw / 2.0f, (float)cy + 56 }, 28.0f, Fade(rc, 0.18f));
                DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 56 }, 44.0f, rc, equipped[i].weaponType);
                DrawText(equipped[i].name.c_str(), cx + 12, cy + 92, 16, rc);
                DrawText(equipped[i].RarityName(), cx + 12, cy + 111, 12, Fade(rc, 0.8f));
                auto lines = equipped[i].StatLines();
                for (size_t k = 0; k < lines.size(); ++k) DrawText(lines[k].c_str(), cx + 12, cy + 129 + (int)k * 16, 14, RAYWHITE);
            } else {
                DrawItemIcon(i, { (float)cx + cw / 2.0f, (float)cy + 56 }, 44.0f, { 60, 68, 90, 255 }, 0);
                const char* empTxt = L("- Empty -", "- Trống -");
                DrawText(empTxt, cx + cw / 2 - MeasureText(empTxt, 17) / 2, cy + 110, 17, { 110, 120, 145, 255 });
            }
        }

        // Chỉ số nhân vật
        int sy = cy + ch + 10;
        DrawText(L("CHARACTER", "CHỈ SỐ NHÂN VẬT"), px + 32, sy, 18, SKYBLUE);
        char buf[96];
        auto stat = [&](int col, int row, const char* label, const std::string& val) {
            DrawText(label, px + 32 + col * 355, sy + 24 + row * 22, 15, { 170, 185, 215, 255 });
            DrawText(val.c_str(), px + 32 + col * 355 + 155, sy + 24 + row * 22, 15, WHITE);
        };
        snprintf(buf, sizeof(buf), "%d / %d", (int)player.hp, (int)player.maxHp); stat(0, 0, L("Health", "Máu"), buf);
        snprintf(buf, sizeof(buf), "%.0f", player.baseDmg); stat(0, 1, L("Damage", "Sát Thương"), buf);
        snprintf(buf, sizeof(buf), "%.1f", EffectiveArmor()); stat(0, 2, L("Armor", "Giáp"), buf);
        snprintf(buf, sizeof(buf), "%.0f", EffectiveMoveSpeed()); stat(1, 0, L("Move Speed", "Tốc Chạy"), buf);
        snprintf(buf, sizeof(buf), "%.2f /s", EffectiveAttackRate()); stat(1, 1, L("Attack Speed", "Tốc Đánh"), buf);
        snprintf(buf, sizeof(buf), "%d%%", (int)roundf(std::min(0.75f, player.critChance) * 100)); stat(1, 2, L("Crit Chance", "Chí Mạng"), buf);
        stat(2, 0, L("Lifesteal", "Hút Máu"), FmtPct(std::min(LifestealCap() + 0.05f, player.lifesteal)));
        snprintf(buf, sizeof(buf), "%.1fs", player.dashCooldown); stat(2, 1, L("Dash Cooldown", "Hồi Lướt"), buf);
        if (player.subClass == 0) {
            snprintf(buf, sizeof(buf), "%.1fs / %.1fs", Skill1MaxCd(), Skill2MaxCd()); stat(2, 2, L("Q / E Cooldown", "Hồi Chiêu Q/E"), buf);
        } else {
            snprintf(buf, sizeof(buf), "%.0f / %.0f / %.0f / %.0fs", Skill1MaxCd(), Skill2MaxCd(), Skill3MaxCd(), Skill4MaxCd()); stat(2, 2, L("Q/E/C/V CD", "Hồi Q/E/C/V"), buf);
        }
        std::string extra = std::string(L("Rage: ", "Nộ: ")) + std::to_string((int)player.rage) + "/100   "
                          + L("Ember Lv", "Lửa Cấp ") + std::to_string(player.igniteLevel) + "   "
                          + L("Frost Lv", "Băng Cấp ") + std::to_string(player.frostLevel);
        DrawText(extra.c_str(), px + pw - 32 - MeasureText(extra.c_str(), 14), sy + 2, 14, { 150, 165, 200, 255 });
        std::string dynInfo = std::string(L("Gen ", "Đời ")) + std::to_string(generation)
                            + " | " + (isNoble ? L("NOBLE", "QUÝ TỘC") : L("Commoner", "Thường Dân"))
                            + " | " + HouseName(houseTier)
                            + " | " + L("Spouse: ", "Vợ: ") + SpouseName(hasSpouse ? spouseIdx : -1)
                            + " | " + L("Heirs: ", "Con: ") + std::to_string((int)children.size());
        DrawText(dynInfo.c_str(), px + 230, sy + 2, 13, isNoble ? GOLD : Color{ 195, 225, 195, 255 });

        // Bộ kỹ năng của Class (4) hoặc Nhánh Phụ (6)
        int ky = sy + 94;
        DrawText(player.subClass == 0 ? L("SKILLS (4)", "BỘ KỸ NĂNG (4)") : L("SUBCLASS SKILLS (6)", "BỘ KỸ NĂNG NHÁNH PHỤ (6)"), px + 32, ky, 17, HeroAccent());
        if (player.subClass == 0) {
            const char* tags[4] = { L("[PASSIVE]", "[NỘI TẠI]"), "[Q]", "[E]", L("[R] RAGE", "[R] NỘ") };
            const char* sNames[4] = { PassiveName(), QName(), EName(), RName() };
            const char* sDescs[4] = { PassiveDesc(), QDesc(), EDesc(), RDesc() };
            for (int s = 0; s < 4; ++s) {
                int rowY = ky + 22 + s * 22;
                DrawText(tags[s], px + 32, rowY, 14, s == 3 ? ORANGE : (s == 0 ? HeroColor() : SKYBLUE));
                DrawText(sNames[s], px + 126, rowY, 14, WHITE);
                DrawText(sDescs[s], px + 318, rowY, 13, { 190, 205, 230, 255 });
            }
        } else {
            const char* tags[6] = { L("[PASSIVE]", "[NỘI TẠI]"), "[Q]", "[E]", "[C]", "[V]", L("[R] RAGE", "[R] NỘ") };
            const char* sNames[6] = { PassiveName(), QName(), EName(), CName(), VName(), RName() };
            const char* sDescs[6] = { PassiveDesc(), QDesc(), EDesc(), CDesc(), VDesc(), RDesc() };
            Color sCols[6] = { HeroColor(), SKYBLUE, { 110, 235, 175, 255 }, { 245, 200, 95, 255 }, { 225, 115, 255, 255 }, ORANGE };
            for (int s = 0; s < 6; ++s) {
                int colX = (s < 3) ? (px + 32) : (px + 588);
                int rowY = ky + 20 + (s % 3) * 32;
                DrawText(tags[s], colX, rowY, 13, sCols[s]);
                DrawText(sNames[s], colX + 82, rowY, 13, WHITE);
                DrawText(sDescs[s], colX + 8, rowY + 14, 11, { 190, 205, 230, 255 });
            }
        }

        // Lộ trình Danh Hiệu theo Mốc Cấp Độ: 20 / 40 / 60 / 80 / 100 (MAX)
        int ty = ky + 118;
        DrawText(L("LEVEL MILESTONE TITLES (LV.100 MAX)", "DANH HIỆU THEO CẤP ĐỘ (CẤP 100 TỐI ĐA)"), px + 32, ty, 15, GOLD);
        int mw = 212, mgap = 8, mx0 = px + 32;
        for (int ti = 1; ti <= 5; ++ti) {
            const LevelTitleDef& td = kLevelTitles[ti];
            bool unlocked = (player.level >= td.minLevel);
            int mx = mx0 + (ti - 1) * (mw + mgap);
            int my = ty + 20;
            DrawRectangle(mx, my, mw, 50, unlocked ? Fade(td.color, 0.16f) : Color{ 18, 22, 34, 220 });
            DrawRectangleLinesEx({ (float)mx, (float)my, (float)mw, 50.0f }, unlocked ? 1.8f : 1.0f, unlocked ? td.color : Color{ 55, 64, 88, 255 });
            std::string lvTag = TextFormat(L("LV.%d%s", "CẤP %d%s"), td.minLevel, td.minLevel == 100 ? " MAX" : "");
            DrawText(lvTag.c_str(), mx + 8, my + 5, 11, unlocked ? td.color : GRAY);
            DrawText(unlocked ? L("ACTIVE", "ĐÃ MỞ") : L("LOCKED", "CHƯA MỞ"), mx + mw - 54, my + 5, 11, unlocked ? Color{ 120, 240, 155, 255 } : DARKGRAY);
            DrawText(settings.language == 1 ? td.nameVi : td.nameEn, mx + 8, my + 19, 12, unlocked ? WHITE : GRAY);
            DrawText(settings.language == 1 ? td.bonusVi : td.bonusEn, mx + 8, my + 34, 10, unlocked ? td.color : Color{ 110, 120, 145, 255 });
        }

        if (!player.relics.empty()) {
            int ry = ty + 78;
            DrawText(L("RELICS", "THÁNH TÍCH"), px + 32, ry, 14, SKYBLUE);
            int rx = px + 128;
            for (const auto& r : player.relics) {
                std::string label = r.name + " (" + RelicDesc(r.type) + ")";
                int w = MeasureText(label.c_str(), 12) + 22;
                if (rx + w > px + pw - 20) { rx = px + 128; ry += 15; }
                DrawCircle(rx + 5, ry + 6, 4.0f, r.color);
                DrawText(label.c_str(), rx + 13, ry + 1, 12, r.color);
                rx += w + 10;
            }
        }
        DrawText(L("Reach Lv.20 / 40 / 60 / 80 / 100 for permanent Titles & Stat Bonuses!   [I] or [Esc] Close",
                   "Đạt Cấp 20 / 40 / 60 / 80 / 100 để mở Danh Hiệu & Cộng Chỉ Số Vĩnh Viễn!   [I] hoặc [Esc] Đóng"),
                 px + 32, py + ph - 22, 13, { 150, 170, 205, 255 });
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

    void DrawPillar(Vector2 p, const BiomeColors& biome) {
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
            } else if (slotKind == 4) { // C: Consecrated Hammer / Crimson Eruption
                DrawRectangle((int)(c.x - s * 0.28f), (int)(c.y - s * 0.30f), (int)(s * 0.56f), (int)(s * 0.24f), col);
                DrawLineEx({ c.x, c.y - s * 0.06f }, { c.x, c.y + s * 0.38f }, 4.0f, WHITE);
            } else if (slotKind == 5) { // V: Heavenly Chains / Gore Whirlwind
                DrawCircleLines((int)c.x, (int)c.y, s * 0.34f, GOLD);
                DrawRing(c, s * 0.16f, s * 0.26f, 30.0f, 310.0f, 12, col);
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
            } else if (slotKind == 4) { // C: Wraith Swarm / Caltrop Storm
                for (int i = 0; i < 3; ++i) {
                    float a = -PI / 2.0f + i * (2.0f * PI / 3.0f);
                    DrawCircleV({ c.x + cosf(a) * s * 0.22f, c.y + sinf(a) * s * 0.22f }, 4.5f, col);
                }
            } else if (slotKind == 5) { // V: Graveyard Scythe / Thunder Slash
                DrawRing(c, s * 0.20f, s * 0.36f, -45.0f, 135.0f, 12, col);
                DrawLineEx({ c.x - s * 0.28f, c.y + s * 0.30f }, { c.x + s * 0.28f, c.y - s * 0.28f }, 3.0f, WHITE);
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
            } else if (slotKind == 4) { // C: Moonlight Cross / Chain Tempest
                DrawLineEx({ c.x - s * 0.28f, c.y - s * 0.28f }, { c.x + s * 0.28f, c.y + s * 0.28f }, 3.2f, col);
                DrawLineEx({ c.x - s * 0.28f, c.y + s * 0.28f }, { c.x + s * 0.28f, c.y - s * 0.28f }, 3.2f, WHITE);
            } else if (slotKind == 5) { // V: Astral Sword Array / Cataclysm Pillar
                DrawRectangle((int)(c.x - s * 0.12f), (int)(c.y - s * 0.36f), (int)(s * 0.24f), (int)(s * 0.72f), Fade(col, 0.75f));
                DrawCircleLines((int)c.x, (int)c.y, s * 0.36f, GOLD);
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
            } else if (slotKind == 4) { // C: Railshot / Valkyrie Spear
                DrawLineEx({ c.x - s * 0.38f, c.y + s * 0.18f }, { c.x + s * 0.34f, c.y - s * 0.18f }, 3.8f, col);
                Tri({ c.x + s * 0.40f, c.y - s * 0.22f }, { c.x + s * 0.16f, c.y - s * 0.24f }, { c.x + s * 0.26f, c.y - s * 0.02f }, WHITE);
            } else if (slotKind == 5) { // V: Flare & Camo / Eye of Hurricane
                DrawCircleLines((int)c.x, (int)c.y, s * 0.34f, col);
                DrawCircleV(c, s * 0.20f, Fade(WHITE, 0.65f));
                DrawCircleV(c, s * 0.10f, GOLD);
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
            if (!ctl.external && !IsWindowState(FLAG_WINDOW_HIDDEN) && gFontLoaded) {
                OpenNameModal(1, -1, kHeirNames[(selectedClass * 2) % 10]);
            }
        }
    }

    void DrawClassSelectScreen() {
        float t = (float)GetTime();
        DrawRectangleGradientV(0, 0, 1280, 720, Fade({ 8, 10, 18, 255 }, 0.92f), Fade({ 12, 16, 28, 255 }, 0.88f));
        const char* hdr = L("CHOOSE YOUR HERO", "CHỌN NHÂN VẬT");
        OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 42) / 2, 22, 42, GOLD, { 80, 40, 10, 255 });

        const int cardW = 285, gap = 16, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 84, cardH = 532;
        static const char* wpNamesEn[8] = { "Sword", "Dagger", "Axe", "Spear", "Hammer", "Scythe", "Bow", "Staff" };
        static const char* wpNamesVi[8] = { "Kiếm", "Dao Găm", "Rìu", "Thương", "Búa", "Lưỡi Hái", "Cung", "Quyền Trượng" };
        for (int i = 0; i < 4; ++i) {
            const ClassDef& cd = kClasses[i];
            const ClassTextVi& vi = kClassesVi[i];
            const char* cname = settings.language == 1 ? vi.name : cd.name;
            const char* crole = settings.language == 1 ? vi.role : cd.role;
            int cx = x0 + i * (cardW + gap);
            bool sel = (classSel == i);
            Rectangle r = { (float)cx, (float)cy, (float)cardW, (float)cardH };
            DrawRectangleRec(r, sel ? Color{ 26, 34, 56, 252 } : Color{ 16, 20, 34, 240 });
            DrawRectangle(cx, cy, cardW, 44, Fade(cd.color, sel ? 0.35f : 0.16f));
            DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : Fade(cd.color, 0.55f));

            DrawText(TextFormat("[%d]", i + 1), cx + 10, cy + 12, 16, GOLD);
            DrawText(cname, cx + 40, cy + 8, 20, sel ? WHITE : cd.color);
            DrawText(crole, cx + 40, cy + 28, 12, { 185, 198, 225, 255 });

            // Xem trước nhân vật sống động
            Vector2 prevPos = { cx + cardW / 2.0f, (float)cy + 102.0f };
            DrawCircleV(prevPos, 38.0f, Fade(cd.color, sel ? 0.18f : 0.08f));
            float aimA = sel ? sinf(t * 1.8f) * 0.45f : 0.25f;
            DrawHeroModel(prevPos, i, cd.startWeapon, { cosf(aimA), sinf(aimA) }, false, t * (sel ? 6.0f : 0.0f), sel, sel ? sinf(t * 5.0f) * 0.3f : 0.35f, false, false, 0.0f, 0.0f);

            // Chỉ số cơ bản + nhánh Chuyển Chức Lv.5
            int stY = cy + 146;
            DrawRectangle(cx + 10, stY, cardW - 20, 68, { 12, 15, 26, 220 });
            DrawText(TextFormat(L("HP: %d   DMG: %d   ARM: %.1f", "Máu: %d  ST: %d  Giáp: %.1f"), (int)cd.baseHp, (int)cd.baseDmg, cd.baseArmor), cx + 18, stY + 6, 13, RAYWHITE);
            DrawText(TextFormat(L("SPD: %d   ATK: %.1f/s   CRIT: %d%%", "Chạy: %d  Đánh: %.1f/s  CM: %d%%"), (int)cd.baseSpeed, cd.baseAtkRate, (int)roundf(cd.baseCrit * 100)), cx + 18, stY + 21, 13, SKYBLUE);
            DrawText(TextFormat(L("Weapon: %s", "Vũ Khí: %s"), settings.language == 1 ? wpNamesVi[cd.startWeapon] : wpNamesEn[cd.startWeapon]), cx + 18, stY + 36, 13, GOLD);
            std::string brTxt = std::string(L("Lv.5: ", "Cấp 5: ")) + SubDisplayName(i, 1) + " / " + SubDisplayName(i, 2);
            if (MeasureText(brTxt.c_str(), 11) > cardW - 28) brTxt = std::string(L("Lv.5: ", "Cấp 5: ")) + SubDisplayName(i, 1) + " | ...";
            DrawText(brTxt.c_str(), cx + 18, stY + 51, 11, cd.accent);

            // 4 kỹ năng: 1 Passive, 2 thường (Q, E), 1 Nộ (R)
            int sy = stY + 74;
            const char* tags[4] = { L("PASSIVE", "NỘI TẠI"), L("SKILL [Q]", "CHIÊU [Q]"), L("SKILL [E]", "CHIÊU [E]"), L("ULTIMATE [R]", "TUYỆT KỸ [R]") };
            const char* sn[4] = {
                settings.language == 1 ? vi.passiveName : cd.passiveName,
                settings.language == 1 ? vi.qName       : cd.qName,
                settings.language == 1 ? vi.eName       : cd.eName,
                settings.language == 1 ? vi.rName       : cd.rName
            };
            const char* sd[4] = {
                settings.language == 1 ? vi.passiveDesc : cd.passiveDesc,
                settings.language == 1 ? vi.qDesc       : cd.qDesc,
                settings.language == 1 ? vi.eDesc       : cd.eDesc,
                settings.language == 1 ? vi.rDesc       : cd.rDesc
            };
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

        Rectangle startBtn = { 1280 / 2.0f - 180.0f, 634.0f, 360.0f, 46.0f };
        DrawRectangleRec(startBtn, { 34, 88, 56, 255 });
        DrawRectangleLinesEx(startBtn, 2.0f, GOLD);
        std::string bTxt = std::string(L("PLAY AS ", "CHỌN ")) + ClassDisplayName(classSel) + " [ENTER]";
        DrawText(bTxt.c_str(), 1280 / 2 - MeasureText(bTxt.c_str(), 18) / 2, 648, 18, WHITE);
        const char* foot = L("1-4 / A-D / Click: select hero     Enter / Double-Click: start     Esc: back", "1-4 / A-D / Nhấp: chọn tướng     Enter / Nhấp đúp: bắt đầu     Esc: quay lại");
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

    void DrawSaveLoadScreen() {
        float t = (float)GetTime();
        DrawRectangleGradientV(0, 0, 1280, 720, Fade({ 8, 12, 22, 255 }, 0.94f), Fade({ 14, 18, 32, 255 }, 0.92f));
        const char* hdr = (saveLoadReturn == PAUSED) ? L("SAVE & LOAD GAME (4 SLOTS)", "LƯU & TẢI GAME (4 Ô LƯU)") : L("LOAD SAVED GAME (4 SLOTS)", "TẢI GAME ĐÃ LƯU (4 Ô LƯU)");
        OutlinedText(hdr, 1280 / 2 - MeasureText(hdr, 38) / 2, 26, 38, GOLD, { 70, 40, 10, 255 });

        const int cardW = 280, gap = 18, x0 = (1280 - (cardW * 4 + gap * 3)) / 2, cy = 98, cardH = 505;
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 4; ++i) {
            const SaveSlotInfo& info = saveSlots[i];
            int cx = x0 + i * (cardW + gap);
            bool sel = (saveSlotSel == i);
            Rectangle r = { (float)cx, (float)cy, (float)cardW, (float)cardH };
            int hc = std::max(0, std::min(3, info.heroClass));
            const ClassDef& cd = kClasses[hc];
            const SubclassDef* sc = (info.subClass == 1 || info.subClass == 2) ? &kSubclasses[hc][info.subClass - 1] : nullptr;
            const char* slotClsName = sc ? SubDisplayName(hc, info.subClass) : ClassDisplayName(hc);
            Color accent = info.exists ? (sc ? sc->color : cd.color) : Color{ 95, 110, 140, 255 };

            DrawRectangleRec(r, sel ? Color{ 26, 34, 56, 252 } : Color{ 16, 20, 34, 240 });
            DrawRectangle(cx, cy, cardW, 42, Fade(accent, sel ? 0.35f : 0.18f));
            DrawRectangleLinesEx(r, sel ? 3.0f : 1.5f, sel ? GOLD : Fade(accent, 0.6f));
            DrawText(TextFormat(L("[%d] SLOT %d", "[%d] Ô LƯU %d"), i + 1, i + 1), cx + 14, cy + 11, 20, sel ? WHITE : accent);
            const char* tag = info.exists ? L("SAVED", "ĐÃ LƯU") : L("EMPTY", "TRỐNG");
            DrawText(tag, cx + cardW - 14 - MeasureText(tag, 14), cy + 14, 14, info.exists ? Color{ 120, 235, 150, 255 } : GRAY);

            if (info.exists) {
                Vector2 prev = { cx + cardW / 2.0f, (float)cy + 122.0f };
                DrawCircleV(prev, 44.0f, Fade(accent, sel ? 0.18f : 0.09f));
                DrawHeroModel(prev, info.heroClass, info.weaponType, { 0.9f, sinf(t * 2.0f + i) * 0.2f }, false, t * (sel ? 5.0f : 0.0f), sel, 0.35f, false, false, 0.0f, 0.0f, info.subClass, 0.0f);

                int iy = cy + 190;
                std::string cardTitle = (!info.heroName.empty() ? (info.heroName + " - ") : std::string("")) + slotClsName;
                DrawText(cardTitle.c_str(), cx + cardW / 2 - MeasureText(cardTitle.c_str(), 17) / 2, iy, 17, accent);
                DrawRectangle(cx + 16, iy + 32, cardW - 32, 138, { 12, 16, 28, 225 });
                DrawRectangleLines(cx + 16, iy + 32, cardW - 32, 138, Fade(accent, 0.35f));
                DrawText(TextFormat(L("Floor %d  -  Lv %d/100", "Tầng %d  -  Cấp %d/100"), info.floor, info.level), cx + 26, iy + 38, 16, GOLD);
                DrawText(LevelTitleNameForLevel(info.level), cx + 26, iy + 58, 13, LevelTitleColorForLevel(info.level));
                DrawText(TextFormat(L("HP: %d / %d", "Máu: %d / %d"), info.hp, info.maxHp), cx + 26, iy + 78, 15, { 245, 110, 120, 255 });
                DrawText(TextFormat(L("Gold: %d   Slain: %d", "Vàng: %d   Đã hạ: %d"), info.gold, info.enemiesSlain), cx + 26, iy + 99, 15, RAYWHITE);
                std::string wn = info.weaponName;
                if ((int)wn.size() > 18) wn = wn.substr(0, 16) + "..";
                std::string wTxt = std::string(L("Weapon: ", "Vũ khí: ")) + wn;
                DrawText(wTxt.c_str(), cx + 26, iy + 120, 14, SKYBLUE);
                DrawText(info.timestamp.c_str(), cx + 26, iy + 144, 13, { 150, 165, 195, 255 });
            } else {
                const char* empSlot = L("EMPTY SLOT", "Ô LƯU TRỐNG");
                DrawText(empSlot, cx + cardW / 2 - MeasureText(empSlot, 20) / 2, cy + 196, 20, { 110, 122, 150, 255 });
                const char* sub = (saveLoadReturn == PAUSED) ? L("Click SAVE HERE below", "Nhấn LƯU TẠI ĐÂY bên dưới") : L("No saved run yet", "Chưa có dữ liệu lưu");
                DrawText(sub, cx + cardW / 2 - MeasureText(sub, 15) / 2, cy + 230, 15, { 130, 145, 175, 255 });
            }

            if (saveLoadReturn == PAUSED) {
                Rectangle saveBtn = { r.x + 16.0f, r.y + r.height - 96.0f, r.width - 32.0f, 38.0f };
                bool hovS = CheckCollisionPointRec(m, saveBtn);
                Color sCol = info.exists ? Color{ 160, 102, 24, 255 } : Color{ 34, 106, 62, 255 };
                DrawRectangleRec(saveBtn, hovS ? Shade(sCol, 1.3f) : sCol);
                DrawRectangleLinesEx(saveBtn, 2.0f, hovS ? WHITE : (info.exists ? GOLD : Color{ 110, 235, 145, 255 }));
                const char* sLbl = info.exists ? L("OVERWRITE SAVE", "GHI ĐÈ BẢN LƯU") : L("SAVE HERE", "LƯU TẠI ĐÂY");
                DrawText(sLbl, (int)(saveBtn.x + saveBtn.width / 2) - MeasureText(sLbl, 17) / 2, (int)saveBtn.y + 10, 17, WHITE);

                if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 94.0f, 36.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 50.0f, 54.0f, 36.0f };
                    bool hovL = CheckCollisionPointRec(m, loadBtn);
                    bool hovD = CheckCollisionPointRec(m, delBtn);
                    DrawRectangleRec(loadBtn, hovL ? Color{ 48, 104, 165, 255 } : Color{ 34, 76, 124, 255 });
                    DrawRectangleLinesEx(loadBtn, 1.6f, hovL ? WHITE : SKYBLUE);
                    const char* lLbl = L("LOAD", "TẢI");
                    const char* dLbl = L("DEL", "XÓA");
                    DrawText(lLbl, (int)(loadBtn.x + loadBtn.width / 2) - MeasureText(lLbl, 17) / 2, (int)loadBtn.y + 9, 17, WHITE);
                    DrawRectangleRec(delBtn, hovD ? Color{ 155, 50, 62, 255 } : Color{ 110, 36, 46, 255 });
                    DrawRectangleLinesEx(delBtn, 1.6f, hovD ? WHITE : Color{ 240, 120, 130, 255 });
                    DrawText(dLbl, (int)(delBtn.x + delBtn.width / 2) - MeasureText(dLbl, 15) / 2, (int)delBtn.y + 10, 15, WHITE);
                } else {
                    Rectangle noBtn = { r.x + 16.0f, r.y + r.height - 50.0f, r.width - 32.0f, 36.0f };
                    DrawRectangleRec(noBtn, { 28, 34, 50, 220 });
                    DrawRectangleLinesEx(noBtn, 1.2f, { 60, 70, 95, 255 });
                    const char* noLbl = L("NO SAVE TO LOAD", "CHƯA CÓ BẢN LƯU");
                    DrawText(noLbl, (int)(noBtn.x + noBtn.width / 2) - MeasureText(noLbl, 14) / 2, (int)noBtn.y + 11, 14, { 110, 120, 145, 255 });
                }
            } else {
                if (info.exists) {
                    Rectangle loadBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 94.0f, 40.0f };
                    Rectangle delBtn  = { r.x + r.width - 70.0f, r.y + r.height - 54.0f, 54.0f, 40.0f };
                    bool hovL = CheckCollisionPointRec(m, loadBtn);
                    bool hovD = CheckCollisionPointRec(m, delBtn);
                    DrawRectangleRec(loadBtn, hovL ? Color{ 44, 130, 82, 255 } : Color{ 32, 98, 60, 255 });
                    DrawRectangleLinesEx(loadBtn, 2.0f, hovL ? WHITE : Color{ 120, 240, 160, 255 });
                    const char* lgLbl = L("LOAD GAME", "TẢI GAME");
                    const char* dLbl = L("DEL", "XÓA");
                    DrawText(lgLbl, (int)(loadBtn.x + loadBtn.width / 2) - MeasureText(lgLbl, 18) / 2, (int)loadBtn.y + 11, 18, WHITE);
                    DrawRectangleRec(delBtn, hovD ? Color{ 155, 50, 62, 255 } : Color{ 110, 36, 46, 255 });
                    DrawRectangleLinesEx(delBtn, 1.6f, hovD ? WHITE : Color{ 240, 120, 130, 255 });
                    DrawText(dLbl, (int)(delBtn.x + delBtn.width / 2) - MeasureText(dLbl, 15) / 2, (int)delBtn.y + 12, 15, WHITE);
                } else {
                    Rectangle noBtn = { r.x + 16.0f, r.y + r.height - 54.0f, r.width - 32.0f, 40.0f };
                    bool hovN = CheckCollisionPointRec(m, noBtn);
                    DrawRectangleRec(noBtn, hovN ? Color{ 44, 124, 78, 255 } : Color{ 28, 72, 52, 240 });
                    DrawRectangleLinesEx(noBtn, 1.6f, hovN ? WHITE : Color{ 110, 235, 160, 255 });
                    const char* eLbl = L("CREATE NEW SAVE [N]", "TẠO BẢN LƯU MỚI [N]");
                    DrawText(eLbl, (int)(noBtn.x + noBtn.width / 2) - MeasureText(eLbl, 15) / 2, (int)noBtn.y + 12, 15, WHITE);
                }
            }
        }

        if (saveStatusTimer > 0.0f && !saveStatusMsg.empty()) {
            DrawText(saveStatusMsg.c_str(), 1280 / 2 - MeasureText(saveStatusMsg.c_str(), 24) / 2, 626, 24, GOLD);
        }
        const char* foot = (saveLoadReturn == PAUSED)
            ? L("1-4 / A-D: slot   Enter: save   N: name save   L: load   Del/X: delete   Esc: back", "1-4 / A-D: chọn ô   Enter: lưu   N: đặt tên save   L: tải   Del/X: xóa   Esc: quay lại")
            : L("1-4 / A-D: slot   Enter / Click: load / create named save   N: new name   Esc: back", "1-4 / A-D: chọn ô   Enter / Nhấp: tải hoặc tạo save có tên   Esc: quay lại");
        DrawText(foot, 1280 / 2 - MeasureText(foot, 16) / 2, 678, 16, { 150, 165, 200, 255 });
    }

    // ── Màn hình tiêu đề / tạm dừng / hướng dẫn ──
    static void OutlinedText(const char* txt, int x, int y, int size, Color col, Color outline) {
        gDrawingOutlinePass = true;
        for (int ox = -2; ox <= 2; ox += 2)
            for (int oy = -2; oy <= 2; oy += 2)
                if (ox != 0 || oy != 0) DrawText(txt, x + ox, y + oy, size, outline);
        gDrawingOutlinePass = false;
        DrawText(txt, x, y, size, col);
    }

    void DrawHelpPanel() {
        int w = 860, h = 612, x = 1280 / 2 - w / 2, y = 54;
        DrawRectangle(x, y, w, h, { 14, 18, 30, 245 });
        DrawRectangleLines(x, y, w, h, GOLD);
        const char* hdr = L("CONTROLS", "HƯỚNG DẪN ĐIỀU KHIỂN");
        DrawText(hdr, x + w / 2 - MeasureText(hdr, 30) / 2, y + 16, 30, GOLD);
        static const char* rowsEn[15][2] = {
            { "W A S D / Arrows", "Move" },
            { "Mouse", "Aim weapon & skills" },
            { "Left Click / Space / J", "Basic Attack (Class-Exclusive Weapon)" },
            { "Right Click / Shift / K", "Dash (brief invulnerability)" },
            { "Q / U  |  E / L", "Class Skills 1 & 2" },
            { "C / Z  |  V / X", "Subclass Skills 3 & 4 (unlocked at Lv.5)" },
            { "R / Middle Click", "Ultimate Rage Skill (at 100 Rage)" },
            { "1 / 2 / 3", "Pick an upgrade on level-up" },
            { "I / Y / H", "Equipment & Skills / Family Tree / Eat Ration" },
            { "F / G", "Trade / Surface Landmarks / Ascend at Floor 10+" },
            { "Gamepad", "Sticks, RT/A atk, B dash, X Skill 1, Y Skill 2, LB Ult" },
            { "O / F2 / F11", "Settings / Windowed Fullscreen" },
            { "S", "Sanctuary: permanent upgrades (title / game over)" },
            { "F5 / L", "Save & Load Game (4 slots with overwrite)" },
            { "M / Esc / F1", "Radar / Pause & Back / Controls" },
        };
        static const char* rowsVi[15][2] = {
            { "W A S D / Phím Mũi Tên", "Di chuyển nhân vật" },
            { "Chuột", "Ngắm hướng đánh & kỹ năng" },
            { "Chuột Trái / Space / J", "Tấn công cơ bản (Vũ khí đặc trưng hệ)" },
            { "Chuột Phải / Shift / K", "Lướt né đòn (miễn sát thương ngắn)" },
            { "Q / U  |  E / L", "Kỹ năng 1 & Kỹ năng 2" },
            { "C / Z  |  V / X", "Kỹ năng Nhánh Phụ 3 & 4 (mở ở Cấp 5)" },
            { "R / Chuột Giữa", "Tuyệt Kỹ Nộ Khí (khi đủ 100 Nộ)" },
            { "1 / 2 / 3", "Chọn nâng cấp hoặc nhánh thức tỉnh" },
            { "I / Y / H", "Trang bị & Kỹ năng / Cây Gia Phả / Ăn Lương Khô" },
            { "F / G", "Thương Nhân / Công Trình / Lên Mặt Đất (Tầng 10+)" },
            { "Tay Cầm", "Cần gạt, RT/A đánh, B lướt, X/Y chiêu, LB Tuyệt Kỹ" },
            { "O / F2 / F11", "Cài Đặt / Toàn Màn Hình Không Viền" },
            { "S", "Thánh Điện: nâng cấp vĩnh viễn" },
            { "F5 / L", "Lưu & Tải Game (tối đa 4 ô ghi đè)" },
            { "M / Esc / F1", "Bản đồ nhỏ / Tạm dừng & Quay lại / Hướng dẫn" },
        };
        for (int i = 0; i < 15; ++i) {
            DrawText(settings.language == 1 ? rowsVi[i][0] : rowsEn[i][0], x + 36, y + 70 + i * 32, 18, SKYBLUE);
            DrawText(settings.language == 1 ? rowsVi[i][1] : rowsEn[i][1], x + 340, y + 70 + i * 32, 18, RAYWHITE);
        }
        DrawText(L("Clear a room, enter the portal. Hit enemies to build 100 Rage for [R] Ultimate!", "Dọn sạch quái trong phòng rồi bước vào cổng. Đánh địch tích 100 Nộ để tung Tuyệt Kỹ [R]!"), x + 36, y + h - 42, 16, { 190, 200, 220, 255 });
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
        const char* sub = L("4 Classes  -  8 Weapons  -  Rage Ultimates", "4 Hệ Tướng  -  8 Vũ Khí  -  Tuyệt Kỹ Nộ Khí");
        DrawText(sub, 1280 / 2 - MeasureText(sub, 22) / 2, ty + 88, 22, { 170, 190, 230, 255 });

        std::string rec = std::string(L("Best Record: Floor ", "Kỷ Lục Cao Nhất: Tầng ")) + std::to_string(bestFloor);
        DrawText(rec.c_str(), 1280 / 2 - MeasureText(rec.c_str(), 22) / 2, 292, 22, SKYBLUE);

        int selCls = std::max(0, std::min(3, selectedClass));
        const ClassDef& cd = kClasses[selCls];
        std::string heroTxt = std::string(L("Hero: ", "Tướng Đang Chọn: ")) + ClassDisplayName(selCls);
        DrawText(heroTxt.c_str(), 1280 / 2 - MeasureText(heroTxt.c_str(), 20) / 2, 324, 20, cd.color);

        const char* diffName = settings.difficulty == 0 ? L("Easy", "Dễ") : (settings.difficulty == 1 ? L("Normal", "Thường") : L("Hard", "Khó"));
        std::string prog = std::string(diffName) + L("     Souls: ", "     Linh Hồn: ") + std::to_string(souls) + L("     Runs: ", "     Lượt Chơi: ") + std::to_string(runs);
        DrawText(prog.c_str(), 1280 / 2 - MeasureText(prog.c_str(), 20) / 2, 354, 20, { 190, 150, 255, 255 });
#ifndef __ANDROID__
        const char* hint = L("Enter: New Game    L: Load Save    S: Sanctuary    O: Settings    F1: Controls", "Enter: Chơi Mới    L: Tải Game    S: Thánh Điện    O: Cài Đặt    F1: Điều Khiển");
        DrawText(hint, 1280 / 2 - MeasureText(hint, 18) / 2, 628, 18, { 130, 145, 180, 255 });
#endif
    }

    void DrawPauseScreen() {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.72f));
        if (showHelp) { DrawHelpPanel(); return; }
        const char* t1 = L("PAUSED", "TẠM DỪNG");
        OutlinedText(t1, 1280 / 2 - MeasureText(t1, 56) / 2, 108, 56, WHITE, { 20, 24, 40, 255 });
        std::string info = std::string(HeroTitle()) + L("   Floor ", "   Tầng ") + std::to_string(floor) + L("   Level ", "   Cấp ") + std::to_string(player.level) + L("   Slain ", "   Đã Hạ ") + std::to_string(enemiesSlain);
        DrawText(info.c_str(), 1280 / 2 - MeasureText(info.c_str(), 20) / 2, 170, 20, HeroColor());
    }

    int SurfaceOptionCount() const {
        if (surfaceBuilding == 0) return 4;
        if (surfaceBuilding == 1) return 3;
        if (surfaceBuilding == 2) return hasSpouse ? (1 + (int)children.size()) : 4;
        return 2;
    }

    void TriggerSurfaceOption(int tab, int opt) {
        if (tab == 0) BuyMeal(opt);
        else if (tab == 1) BuyHouseUpgrade(opt + 1);
        else if (tab == 2) {
            if (!hasSpouse) TryMarrySpouse(opt);
            else if (opt == 0) TryHaveChild();
            else SuccessionToChild(opt - 1);
        } else if (tab == 3) {
            if (opt == 0) BuyNobleTitleDirect();
            else AcceptRoyalCommission();
        }
    }

    void UpdateSurfaceUi() {
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
            for (int i = 0; i < 4; ++i) {
                Rectangle r = { 120.0f, 186.0f + i * 96.0f, 1040.0f, 84.0f };
                if (CheckCollisionPointRec(m, r)) {
                    if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = i; TriggerSurfaceOption(0, i); return; }
                }
            }
        } else if (surfaceBuilding == 1) {
            for (int i = 0; i < 3; ++i) {
                Rectangle r = { 120.0f, 186.0f + i * 124.0f, 1040.0f, 110.0f };
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
                for (size_t i = 0; i < children.size(); ++i) {
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
            for (int i = 0; i < 2; ++i) {
                Rectangle r = { 120.0f, 196.0f + i * 190.0f, 1040.0f, 168.0f };
                if (CheckCollisionPointRec(m, r)) {
                    if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0) surfaceSel = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { surfaceSel = i; TriggerSurfaceOption(3, i); return; }
                }
            }
        }
    }

    void DrawSurfaceUi() {
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
                         + "  |  " + L("Hunger: ", "Độ No: ") + std::to_string((int)ceilf(hunger)) + "/100"
                         + "  |  " + HouseName(houseTier)
                         + "  |  " + TextFormat(L("Day %d %s (%s)  Age %d", "Ngày %d - %s (%s) - Tuổi %d"), dayCount, ClockString().c_str(), TimePhaseName(), heroAge);
        DrawText(info.c_str(), 120, 148, 15, GOLD);

        if (surfaceBuilding == 0) {
            struct MealRow { const char* nameEn; const char* nameVi; const char* descEn; const char* descVi; int baseCost; Color col; };
            static const MealRow meals[4] = {
                { "Warm Village Bread", "Bánh Mì Nóng Hổi", "+35 Hunger & +18 HP immediately", "+35 Độ No & hồi +18 Máu ngay lập tức", 15, { 245, 205, 115, 255 } },
                { "Savory Roast Platter", "Thịt Nướng Đậm Đà", "+70 Hunger & +40% Max HP", "+70 Độ No & hồi +40% Máu Tối Đa", 35, ORANGE },
                { "Royal Palace Banquet", "Yến Tiệc Thịnh Soạn", "Full 100 Hunger, Full 100% HP & +25 Rage", "Đầy 100 Độ No, Đầy 100% Máu & +25 Nộ", 70, GOLD },
                { "Dungeon Ration Pack (x2)", "Túi Lương Khô Hầm Ngục (x2)", "Stores +2 portable Rations. Press [H] anywhere to eat (+45 Hunger & +25% HP)", "Tích +2 Lương Khô mang đi. Nhấn [H] mọi lúc (+45 No & +25% Máu)", 25, { 125, 235, 155, 255 } }
            };
            for (int i = 0; i < 4; ++i) {
                Rectangle r = { 120.0f, 186.0f + i * 96.0f, 1040.0f, 84.0f };
                bool sel = (surfaceSel == i);
                int price = MealPrice(meals[i].baseCost);
                bool afford = player.gold >= price;
                DrawRectangleRec(r, sel ? Color{ 34, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
                DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(meals[i].col, 0.55f));
                DrawText(L(meals[i].nameEn, meals[i].nameVi), (int)r.x + 22, (int)r.y + 14, 22, meals[i].col);
                DrawText(L(meals[i].descEn, meals[i].descVi), (int)r.x + 22, (int)r.y + 46, 15, RAYWHITE);
                Rectangle b = { r.x + r.width - 190.0f, r.y + 20.0f, 168.0f, 44.0f };
                DrawRectangleRec(b, afford ? Color{ 34, 96, 56, 255 } : Color{ 82, 32, 38, 255 });
                DrawRectangleLinesEx(b, 1.5f, afford ? Color{ 120, 240, 150, 255 } : Color{ 240, 120, 130, 255 });
                std::string pTxt = std::to_string(price) + L(" G", " Vàng");
                DrawText(pTxt.c_str(), (int)(b.x + b.width / 2) - MeasureText(pTxt.c_str(), 19) / 2, (int)b.y + 12, 19, afford ? WHITE : Color{ 255, 175, 175, 255 });
            }
        } else if (surfaceBuilding == 1) {
            for (int t = 1; t <= 3; ++t) {
                Rectangle r = { 120.0f, 186.0f + (t - 1) * 124.0f, 1040.0f, 110.0f };
                bool sel = (surfaceSel == t - 1);
                bool owned = (houseTier >= t);
                bool nextTier = (houseTier + 1 == t);
                int price = HouseUpgradeCost(t);
                bool afford = nextTier && player.gold >= price;
                Color hc = owned ? Color{ 115, 240, 155, 255 } : (nextTier ? GOLD : GRAY);
                DrawRectangleRec(r, sel ? Color{ 32, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
                DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(hc, 0.55f));
                DrawText(HouseName(t), (int)r.x + 24, (int)r.y + 16, 24, hc);
                DrawText(L(kHouses[t].descEn, kHouses[t].descVi), (int)r.x + 24, (int)r.y + 54, 16, RAYWHITE);
                DrawText(TextFormat(L("Total Bonus: +%d Max HP   +%d Damage   Up to %d Children", "Tổng Cộng: +%d Máu Tối Đa   +%d Sát Thương   Tối đa %d Con"),
                    (int)kHouses[t].hpBonus, (int)kHouses[t].dmgBonus, kHouses[t].maxKids), (int)r.x + 24, (int)r.y + 80, 14, SKYBLUE);

                Rectangle b = { r.x + r.width - 230.0f, r.y + 30.0f, 206.0f, 48.0f };
                const char* bTxt = owned ? L("OWNED", "ĐÃ SỞ HỮU") : (nextTier ? TextFormat(L("BUY %d G", "MUA %d VÀNG"), price) : L("REQUIRES PREV TIER", "CẦN CẤP TRƯỚC"));
                Color bFill = owned ? Color{ 28, 72, 48, 255 } : (afford ? Color{ 38, 102, 62, 255 } : Color{ 64, 34, 42, 255 });
                DrawRectangleRec(b, bFill);
                DrawRectangleLinesEx(b, 1.5f, hc);
                DrawText(bTxt, (int)(b.x + b.width / 2) - MeasureText(bTxt, 17) / 2, (int)b.y + 15, 17, WHITE);
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
                        Color cc = kClasses[std::max(0, std::min(3, ch.heroClass))].color;
                        DrawRectangleRec(r, sel ? Color{ 34, 42, 68, 252 } : Color{ 20, 26, 44, 240 });
                        DrawRectangleLinesEx(r, sel ? 2.5f : 1.2f, sel ? GOLD : Fade(cc, 0.65f));
                        DrawText(TextFormat(L("Heir %d: %s  (%s)", "Con %d: %s  (%s)"), (int)i + 1, ch.name.c_str(), ClassDisplayName(ch.heroClass)), (int)r.x + 22, (int)r.y + 12, 20, cc);
                        DrawText(TextFormat(L("Inherits Gold, Gear, House, Noble & +%d HP +%d DMG +%.1f ARM +%d SPD",
                                              "Kế thừa Vàng, Đồ, Nhà, Quý Tộc & +%d Máu +%d ST +%.1f Giáp +%d Chạy"),
                            (int)ch.bonusHp, (int)ch.bonusDmg, ch.bonusArmor, (int)ch.bonusSpeed), (int)r.x + 22, (int)r.y + 44, 14, RAYWHITE);

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
            // Hoàng Cung & Vua
            Rectangle r0 = { 120.0f, 196.0f, 1040.0f, 168.0f };
            bool sel0 = (surfaceSel == 0);
            bool affordN = (!isNoble && player.gold >= 1500);
            DrawRectangleRec(r0, sel0 ? Color{ 44, 38, 28, 252 } : Color{ 24, 22, 34, 245 });
            DrawRectangleLinesEx(r0, sel0 ? 2.8f : 1.5f, sel0 ? GOLD : Color{ 195, 155, 65, 255 });
            DrawText(L("DIRECT ROYAL GRANT: BUY NOBLE TITLE (1500 GOLD)", "MUA TƯỚC VỊ QUÝ TỘC TRỰC TIẾP (1500 VÀNG)"), (int)r0.x + 26, (int)r0.y + 18, 22, GOLD);
            DrawText(L("High-gold shortcut to Nobility: +35 HP, +6 DMG, +1.5 Armor, +25% Gold drops, +30G/Floor,",
                       "Dùng lượng Vàng lớn phong tước ngay: +35 Máu, +6 ST, +1.5 Giáp, +25% Vàng rơi, +30V/Tầng,"), (int)r0.x + 26, (int)r0.y + 56, 15, RAYWHITE);
            DrawText(L("30% discount on Food/Marriage/Children & 50% discount on Noble Estate Manor.",
                       "Giảm 30% Đồ Ăn/Cưới/Sinh Con và giảm 50% giá Dinh Thự Quý Tộc."), (int)r0.x + 26, (int)r0.y + 80, 15, SKYBLUE);
            Rectangle b0 = { r0.x + r0.width - 260.0f, r0.y + 106.0f, 236.0f, 44.0f };
            DrawRectangleRec(b0, isNoble ? Color{ 38, 86, 54, 255 } : (affordN ? Color{ 138, 96, 22, 255 } : Color{ 68, 34, 38, 255 }));
            DrawRectangleLinesEx(b0, 1.5f, GOLD);
            const char* nLbl = isNoble ? L("NOBLE TITLE OWNED", "ĐÃ LÀ QUÝ TỘC") : L("BUY NOBILITY - 1500G", "MUA QUÝ TỘC - 1500V");
            DrawText(nLbl, (int)(b0.x + b0.width / 2) - MeasureText(nLbl, 16) / 2, (int)b0.y + 14, 16, WHITE);

            Rectangle r1 = { 120.0f, 386.0f, 1040.0f, 168.0f };
            bool sel1 = (surfaceSel == 1);
            bool readyQ = (royalQuestActive && royalQuestKills >= 35 && royalQuestBosses >= 1);
            DrawRectangleRec(r1, sel1 ? Color{ 32, 42, 68, 252 } : Color{ 20, 26, 44, 245 });
            DrawRectangleLinesEx(r1, sel1 ? 2.8f : 1.5f, sel1 ? GOLD : SKYBLUE);
            DrawText(L("KING'S DUNGEON SUBJUGATION COMMISSION (FREE PATH TO NOBILITY)", "ỦY THÁC THẢO PHẠT HẦM NGỤC CỦA VUA (CON ĐƯỜNG MIỄN PHÍ)"), (int)r1.x + 26, (int)r1.y + 18, 21, SKYBLUE);
            DrawText(L("Descend into the Dungeon, slay 35 Monsters and defeat 1 Boss to earn Nobility + 300G + Relic!",
                       "Xuống Hầm Ngục, tiêu diệt 35 Quái vật và 1 Trùm để Vua phong Tước Quý Tộc + 300 Vàng + Thánh Tích!"), (int)r1.x + 26, (int)r1.y + 54, 15, RAYWHITE);
            DrawText(TextFormat(L("Progress:  Monsters %d / 35    |    Bosses %d / 1", "Tiến Độ:  Quái Đã Hạ %d / 35    |    Trùm Đã Hạ %d / 1"),
                royalQuestKills, royalQuestBosses), (int)r1.x + 26, (int)r1.y + 84, 17, readyQ ? GREEN : GOLD);
            Rectangle b1 = { r1.x + r1.width - 290.0f, r1.y + 106.0f, 266.0f, 44.0f };
            Color qFill = isNoble ? Color{ 38, 86, 54, 255 } : (readyQ ? Color{ 38, 128, 66, 255 } : (royalQuestActive ? Color{ 44, 62, 102, 255 } : Color{ 118, 52, 40, 255 }));
            DrawRectangleRec(b1, qFill);
            DrawRectangleLinesEx(b1, 1.5f, isNoble || readyQ ? GOLD : SKYBLUE);
            const char* qLbl = isNoble ? L("NOBILITY ACHIEVED", "ĐÃ HOÀN THÀNH") : (readyQ ? L("CLAIM NOBILITY!", "NHẬN PHONG QUÝ TỘC!") : (royalQuestActive ? L("IN PROGRESS (ENTER DUNGEON)", "ĐANG LÀM (XUỐNG HẦM NGỤC)") : L("ACCEPT KING'S COMMISSION", "NHẬN ỦY THÁC CỦA VUA")));
            DrawText(qLbl, (int)(b1.x + b1.width / 2) - MeasureText(qLbl, 14) / 2, (int)b1.y + 15, 14, WHITE);
        }

        const char* foot = L("1-4 / Left-Right: Switch Building    Up/Down + Enter or Click: Action    Y: Family Tree    Esc: Back",
                             "1-4 / Trái-Phải: Đổi Khu Vực    Lên/Xuống + Enter hoặc Nhấp: Chọn    Y: Cây Gia Phả    Esc: Thoát");
        DrawText(foot, 1280 / 2 - MeasureText(foot, 15) / 2, 636, 15, { 165, 180, 215, 255 });
    }

    void UpdateFamilyTree() {
        { int ua = HitButton(); if (ua) { HandleUiAction(ua); return; } }
        if (EscPressed() || IsKeyPressed(KEY_Y) || IsKeyPressed(KEY_BACKSPACE) || PadPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            state = familyTreeReturn;
            return;
        }
    }

    void DrawFamilyTreeScreen() {
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
                DrawText(a.name.c_str(), cx + 12, ancY + 30, 17, kClasses[std::max(0, std::min(3, a.heroClass))].color);
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
                Color cc = kClasses[std::max(0, std::min(3, ch.heroClass))].color;
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

    static void DrawSurfacePrompt(Vector2 p, const char* title, const char* sub, Color accent, bool near) {
        int tw = MeasureText(title, 14);
        int sw = (sub && sub[0]) ? MeasureText(sub, 12) : 0;
        int bw = std::max(tw, sw) + 26;
        int bh = (sw > 0) ? 42 : 24;
        int bx = (int)p.x - bw / 2;
        int by = (int)p.y;
        DrawRectangle(bx, by, bw, bh, near ? Color{ 14, 20, 34, 245 } : Color{ 10, 14, 26, 224 });
        DrawRectangleLines(bx, by, bw, bh, near ? GOLD : Fade(accent, 0.78f));
        DrawText(title, (int)p.x - tw / 2, by + 5, 14, near ? GOLD : accent);
        if (sw > 0) DrawText(sub, (int)p.x - sw / 2, by + 23, 12, near ? WHITE : Color{ 218, 228, 246, 255 });
    }

    // npcKind: 0..3 = 4 Nàng Dâu, 4 = Đức Vua, 5 = Chủ Quán Trọ, 6 = Kiến Trúc Sư
    static void DrawDetailedNpcModel(Vector2 pos, int npcKind, Color primaryCol, Color secondaryCol, Color hairCol, bool crown, float bobPhase) {
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

    void DrawSurfaceExitDoor(Vector2 pos) {
        bool near = Vector2Distance(player.pos, pos) < 95.0f;
        DrawRectangle((int)pos.x - 42, (int)pos.y - 24, 84, 52, { 54, 42, 34, 255 });
        DrawRectangle((int)pos.x - 32, (int)pos.y - 16, 64, 40, near ? Color{ 62, 118, 165, 240 } : Color{ 34, 48, 72, 235 });
        DrawRectangleLines((int)pos.x - 34, (int)pos.y - 18, 68, 44, near ? GOLD : Color{ 165, 145, 105, 255 });
        DrawRectangle((int)pos.x - 36, (int)pos.y - 28, 72, 10, { 125, 116, 104, 255 });
        DrawSurfacePrompt({ pos.x, pos.y - 62.0f }, L("TOWN SQUARE EXIT", "CỬA RA QUẢNG TRƯỜNG"),
                          L("[F] / Walk Out   [0] Quick Exit", "[F] hoặc Đi Ra   [0] Ra Nhanh"), SKYBLUE, near);
    }

    void DrawMiniHouseModel(Vector2 p, int tier, Color roofCol, Color trimCol) {
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

    void DrawSurfaceTownSquare(const BiomeColors& biome, float tm) {
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

    void DrawSurfaceTavernRoom(const BiomeColors& biome, float tm) {
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

    void DrawSurfaceHousingRoom(const BiomeColors& biome, float tm) {
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

    void DrawSurfaceFamilyHomeRoom(const BiomeColors& biome, float tm) {
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
                DrawCircleLines((int)hp.x, (int)hp.y + 6, 30.0f, Fade(nearH ? GOLD : cc, 0.72f));
                DrawHeroModel(hp, hc, ClassWeaponType(hc), { -0.8f, 0.2f }, true, tm * 2.5f + k, true, 0.2f, false, false, 0.0f, 0.0f, 0, 0.0f);
                std::string sub = TextFormat(L("[F] REST IN PEACE (%s +%dHP)  |  [N] Rename", "[F] AN NGHỈ - KẾ THỪA  |  [N] Đổi Tên"), ClassDisplayName(hc), (int)ch.bonusHp);
                DrawSurfacePrompt({ hp.x, hp.y + 26.0f }, ch.name.c_str(), sub.c_str(), cc, nearH);
            }
        }
        DrawSurfaceExitDoor(SurfaceInteriorExitPos());
    }

    void DrawSurfacePalaceRoom(const BiomeColors& biome, float tm) {
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

    void DrawSurfaceWorld(const BiomeColors& biome) {
        float tm = (float)GetTime();
        if (surfaceRoom == 1) DrawSurfaceTavernRoom(biome, tm);
        else if (surfaceRoom == 2) DrawSurfaceHousingRoom(biome, tm);
        else if (surfaceRoom == 3) DrawSurfaceFamilyHomeRoom(biome, tm);
        else if (surfaceRoom == 4) DrawSurfacePalaceRoom(biome, tm);
        else DrawSurfaceTownSquare(biome, tm);
    }

    // ── Drawing ──
    void Draw() {
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

        // Trang bị đang đeo (HUD)
        for (int i = 0; i < 4; ++i) {
            int hx = 24 + i * 36, hy = 156 + yOff;
            Color rc = hasEquip[i] ? equipped[i].RarityColor() : Color{ 60, 68, 90, 255 };
            DrawRectangle(hx, hy, 30, 30, { 14, 18, 30, 210 });
            DrawRectangleLines(hx, hy, 30, 30, rc);
            DrawItemIcon(i, { hx + 15.0f, hy + 15.0f }, 17.0f, hasEquip[i] ? rc : Color{ 70, 78, 100, 255 }, hasEquip[i] ? equipped[i].weaponType : 0);
        }
        DrawText(L("[I] Gear & Skills   [Y] Family Tree", "[I] Đồ & Kỹ Năng   [Y] Gia Phả"), 24 + 4 * 36 + 4, 164 + yOff, 14, { 150, 165, 200, 255 });

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
    CloseGameFont();
    CloseWindow();
    return 0;
}
