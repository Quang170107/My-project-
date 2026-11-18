#pragma once
#include "fonts_audio.hpp"

enum EnemyType { CHASER, RANGER, BRUTE, BOSS, BOMBER, CHARGER, SPLITTER, SUMMONER };
enum RelicType { RELIC_LIGHTNING, RELIC_TOXIC, RELIC_FROST, RELIC_AEGIS, RELIC_CHALICE, RELIC_COMBUSTION, RELIC_PHOENIX, RELIC_MIDAS, RELIC_BERSERK, RELIC_THORNS };
enum UpgradeType { UP_HP, UP_DMG, UP_SPEED, UP_ATTACK_RATE, UP_DASH, UP_LIFESTEAL, UP_ARMOR, UP_CRIT, UP_NOVA, UP_IGNITE, UP_FROST };
enum HeroClass { CLASS_KNIGHT = 0, CLASS_ASSASSIN = 1, CLASS_MAGE = 2, CLASS_RANGER = 3, CLASS_TEMPLAR = 4 };
static constexpr int kNumClasses = 5;
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

static const ClassDef kClasses[kNumClasses] = {
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
    },
    {
        "Royal Lancemaster", "Imperial Halberdier", { 245, 185, 55, 255 }, { 255, 95, 115, 255 }, WP_SPEAR,
        112.0f, 290.0f, 27.0f, 2.65f, 1.8f, 0.10f, 5.0f, 6.8f,
        "Imperial Mandate", "Every 3rd thrust fires a piercing Golden Dragon Wave (1.45x) & heals 5 HP",
        "Dragon Piercer", "Dash-thrust 115px (2.25x DMG + Stagger) & launch 3 golden lance beams",
        "Royal Banner", "1.95x Banner Slam (Stun 1.2s) + 3.5s Command Aura (+4 ARM, +30% ATK SPD, Shield)",
        "Sovereign's Phalanx", "3.8x Imperial Impact + 4.0s storm of Golden Halberds (1.35x AoE) & Overdrive"
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

static const SubclassDef kSubclasses[kNumClasses][2] = {
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
    },
    { // TEMPLAR / ROYAL LANCEMASTER (Class weapon: Spear WP_SPEAR)
        {
            CLASS_TEMPLAR, 1, "Grand Inquisitor", "Dai Phan Quan - Thanh Thuong Phan Quyet",
            { 255, 215, 85, 255 }, { 115, 235, 255, 255 }, WP_SPEAR, "Sunpiercer Halberd",
            34.0f, 8.0f, 2.8f, 15.0f, 0.35f, 0.08f, 0.06f, 7.2f, 9.2f,
            "Inquisitor's Verdict", "Every 2nd thrust fires a burning Sun-Lance Wave & heals 6 HP",
            "Solar Impale", "Dash-impale 130px (2.55x DMG + Stun 1.4s) & fire 3 Sun-Lance Waves",
            "Sanctified Standard", "2.2x Holy Banner Nova + 4.5s Barrier (+6 ARM, +35% ATK SPD, Shield)",
            "Javelins of Light", "Hurl 5 piercing Golden Javelins (2.15x DMG, Stun 1.1s, Burn) & heal 10% HP",
            "Cross of Judgment", "2.65x Golden Cross blast (Burn + +45 Stagger) & +20 Rage",
            "Heavensfall Phalanx", "4.3x Solar Halberd Impact + 4.5s rain of Golden Spears & Sanctuary"
        },
        {
            CLASS_TEMPLAR, 2, "Dragonlord Monarch", "Long De Ba Vuong - Phuong Thien Long Kich",
            { 245, 75, 95, 255 }, { 255, 210, 75, 255 }, WP_SPEAR, "Dragonfang Halberd",
            28.0f, 11.0f, 1.8f, 25.0f, 0.50f, 0.10f, 0.08f, 7.0f, 9.0f,
            "Dragonblood Sovereignty", "Every attack fires a Dragon Crescent Wave (+15% Crit DMG) & +3 Rage",
            "Wyrmfang Charge", "Dragon leap 135px (2.65x AoE + Burn), pull foes & heal 10% Max HP",
            "Emperor's Roar", "2.3x Dragon Shockwave (Stun 1.5s) + 4s Haste/Shield & +25 Rage",
            "Eight-Direction Sweep", "2.4x 360-deg Halberd cleave firing 8 piercing Dragon Waves outward",
            "Imperial Starfall", "Summon 6 Dragon Pillars (2.3x AoE + Burn) & reset Dash cooldown",
            "Dynasty Annihilation", "4.5x Dragon Cataclysm + 4.2s Crimson-Gold Vortex & Overdrive"
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

static const ClassTextVi kClassesVi[kNumClasses] = {
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
    },
    {
        "Long Kỵ Thương Vương", "Thánh Thương Hoàng Gia", "Thánh Thương",
        "Thánh Chỉ Hoàng Gia", "Đòn đâm thứ 3 phóng Long Thương Khí xuyên thấu (1.45x ST) & hồi 5 HP",
        "Long Thương Xung Kích", "Lướt đâm 115px (2.25x ST + Phá Giáp) & phóng 3 luồng thương hoàng kim",
        "Hoàng Kỳ Chiêu Quân", "Cắm Hoàng Kỳ nổ 1.95x (Choáng 1.2s) + 3.5s Giáp Thánh (+4 Giáp, +30% Tốc)",
        "", "", "", "",
        "Thiên Quân Vạn Mã", "3.8x Đại Thương Giáng Thế + 4.0s mưa Thánh Kích (1.35x AoE) & Bùng Nổ"
    }
};

static const ClassTextVi kSubclassesVi[kNumClasses][2] = {
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
    },
    { // ROYAL LANCEMASTER
        {
            "Đại Phán Quan", "Đại Phán Quan - Thánh Thương Phán Quyết", "Thần Kích Nhật Quang",
            "Phán Quyết Ánh Sáng", "Đòn đâm thứ 2 phóng Thánh Thương Khí thiêu đốt & hồi 6 HP",
            "Nhật Quang Xung Kích", "Lướt đâm 130px (2.55x ST + Choáng 1.4s) & phóng 3 luồng thương lửa",
            "Thánh Kỳ Bảo Hộ", "Nổ 2.2x ST + Giáp Thánh 4.5s (+6 Giáp, +35% Tốc Đánh, Khiên)",
            "Mưa Lao Ánh Sáng", "Ném 5 Lao Thần xuyên thấu (2.15x ST, Choáng 1.1s, Đốt) & hồi 10% Máu",
            "Thập Tự Phán Quyết", "2.65x Vụ nổ Thập Tự Hoàng Kim (Đốt + +45 Phá Giáp) & +20 Nộ",
            "Thiên Phạt Đại Trận", "4.3x Thần Kích Giáng Thế + 4.5s mưa Thánh Thương & Thánh Vực"
        },
        {
            "Long Đế Bá Vương", "Long Đế Bá Vương - Phương Thiên Long Kích", "Phương Thiên Long Kích",
            "Long Huyết Đế Vương", "Mỗi đòn đâm phóng Long Trăng Khí (+15% Chí Mạng) & +3 Nộ",
            "Bá Long Xung Trận", "Nhảy 135px nổ Long Viêm 2.65x AoE, hút quái & hồi 10% Máu Tối Đa",
            "Long Khiếu Chấn Thiên", "2.3x Sóng Long Uy (Choáng 1.5s) + 4s Tăng Tốc/Khiên & +25 Nộ",
            "Bát Phương Quét Ngang", "Quét kích 360° (2.4x ST) phóng 8 luồng Long Khí xuyên thấu",
            "Tinh Tú Giáng Trần", "Triệu 6 Cột Lửa Long Viêm (2.3x AoE + Đốt) & hồi ngay Dash",
            "Hoàng Triều Hủy Diệt", "4.5x Đại Địa Chấn Rồng + 4.2s Xoáy Đỏ Hoàng Kim & Bùng Nổ"
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
    int bossKind = 0;          // 0 = Void Sovereign, 1 = Molten Colossus, 2 = Frost Dragon Archon
    bool isGoldenGoblin = false; // Yêu Tinh Vàng chạy trốn rơi kho báu
    float reactionCooldown = 0.0f; // Hồi chiêu phản ứng nguyên tố trên mục tiêu
    float escapeTimer = 18.0f; // Thời gian trước khi Yêu Tinh Vàng biến mất
    float stagger = 0.0f;           // Điểm Phá Giáp tích lũy (0..100) cho Elite / Warden / Boss
    float maxStagger = 100.0f;
    float staggerBrokenTimer = 0.0f; // > 0: Đang bị Phá Giáp (+35% sát thương nhận vào)
    bool isKingBoss = false;        // Vua Hoàng Triều trong trận Nổi Dậy Chiếm Ngai Vàng
    int kingClass = 0;              // Class ngẫu nhiên (0..4) mà Vua sử dụng
    int kingPowerScore = 100;       // Uy lực của Vua (scale theo sức mạnh nhân vật)
    bool isNationalGeneral = false; // Tướng Tổng Tư Lệnh Quốc Gia trên Chiến Trường
    int generalNation = 0;
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
    int childOwner = -1;   // >= 0: đạn do con (đang rèn luyện) bắn ra
};

struct GroundEffect {
    int kind = 0; // 0: Bẫy nổ Ranger, 1: Thánh vực Knight, 2: Hố đen Mage, 3: Vùng Mưa Tên Ranger, 4: Thiên thạch rơi, 5: Mũi tên Phượng Hoàng rơi, 6: Sét Thiên Lôi giáng xuống
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
    int fClamped = std::max(1, floor);
    // Giới hạn phẩm chất theo tầng: Tầng 1-2 tối đa Rare (1); Tầng 3-5 tối đa Epic (2); Tầng 6+ mở khóa Legendary (3)
    float legChance  = (fClamped >= 6) ? std::min(0.06f, 0.015f + (fClamped - 6) * 0.003f) : 0.0f;
    float epicChance = (fClamped >= 3) ? std::min(0.16f, 0.05f + (fClamped - 3) * 0.008f) : 0.0f;
    float rareChance = std::min(0.36f, 0.18f + (fClamped - 1) * 0.012f);
    it.rarity = (roll < legChance) ? 3 : (roll < legChance + epicChance ? 2 : (roll < legChance + epicChance + rareChance ? 1 : 0));
    float rm = it.rarity == 3 ? 2.15f : (it.rarity == 2 ? 1.60f : (it.rarity == 1 ? 1.25f : 1.0f));
    float m = rm * (1.0f + (fClamped - 1) * 0.085f);
    switch (it.slot) {
        case SLOT_WEAPON:
            it.damage = roundf(3.5f * m);
            it.attackRate = std::min(0.90f, R1(0.10f * m));
            it.crit = std::min(0.14f, roundf(0.015f * m * 100.0f) / 100.0f);
            break;
        case SLOT_ARMOR:
            it.armor = R1(0.85f * m);
            it.maxHp = roundf(7.5f * m);
            break;
        case SLOT_BOOTS:
            it.moveSpeed = std::min(1.8f, R1(0.22f * m));
            it.maxHp = roundf(4.5f * m);
            break;
        default:
            it.lifesteal = std::min(0.10f * lsScale, roundf(0.014f * m * lsScale * 1000.0f) / 1000.0f);
            it.attackRate = std::min(0.75f, R1(0.12f * m));
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
    bool cursed = false; // rương nguyền rủa: triệu hồi tinh anh, thưởng Linh Thú & đồ xịn
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

// ── Thời gian & Tuổi tác: 1 năm trong game = 3 phút (180 giây) chơi thực tế ──
static constexpr float kRealSecondsPerYear = 180.0f;
static constexpr int   kChildTrainAge      = 6;    // đủ tuổi theo cha xuống Hầm Ngục rèn luyện
static constexpr int   kChildAdultAge      = 16;   // trưởng thành: có thể kế thừa gia tộc
static constexpr int   kHeroLifespan       = 80;   // tuổi thọ tối đa của nhân vật chính
static constexpr int   kChildMaxLevel      = 30;
static constexpr float kChildInjuryTime    = kRealSecondsPerYear * 0.5f; // nửa năm dưỡng thương

struct ChildInfo {
    std::string name;
    int heroClass = 0;
    float bonusHp = 0.0f;
    float bonusDmg = 0.0f;
    float bonusArmor = 0.0f;
    float bonusSpeed = 0.0f;
    // Tuổi tác & rèn luyện (được lưu)
    float age = 0.0f;          // tuổi (năm), tăng theo thời gian thực
    int level = 1;             // cấp rèn luyện trong Hầm Ngục
    int xp = 0;
    bool inParty = false;      // được mang theo vào Hầm Ngục
    float injuredTimer = 0.0f; // giây thực còn lại để hồi phục sau khi trọng thương
    // Trạng thái chiến đấu (không lưu)
    bool active = false;
    Vector2 pos = { 0.0f, 0.0f };
    Vector2 aim = { 1.0f, 0.0f };
    float hp = 1.0f;
    float maxHp = 1.0f;
    float atkTimer = 0.0f;
    float invuln = 0.0f;
    float walkTimer = 0.0f;
    float swing = 0.0f;
    bool moving = false;
    bool facingLeft = false;
};

static int ChildXpNext(int lvl) { return 15 + std::max(1, lvl) * 8; }

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

// ── Tước Vị Quý Tộc Chuẩn Vương Quốc Anh (0..8) ────────────────────────────
struct PeerageDef {
    int rank;
    const char* nameEn;
    const char* nameVi;
    const char* styleEn;
    const char* styleVi;
    int goldCost;
    int meritReq;
    int dailyGold;
    float hpBonus;
    float dmgBonus;
    float armorBonus;
    Color color;
};

static const PeerageDef kPeerageDefs[9] = {
    { 0, "Commoner",       "Thường Dân",        "Citizen",            "Công Dân",           0,   0,    0,   0.0f,  0.0f,  0.0f, { 180, 190, 210, 255 } },
    { 1, "Knight",         "Hiệp Sĩ",           "Sir / Dame",         "Tước Hiệp Sĩ",    1500,   0,   60,  35.0f,  6.0f,  1.5f, { 130, 225, 255, 255 } },
    { 2, "Baronet",        "Chuẩn Nam Tước",    "Baronet",            "Chuẩn Nam Tước",   500,  20,  110,  60.0f, 11.0f,  2.2f, { 115, 245, 175, 255 } },
    { 3, "Baron",          "Nam Tước",          "Right Hon. Baron",   "Nam Tước Lãnh Chúa", 800, 50,  175,  90.0f, 17.0f,  3.0f, { 105, 230, 255, 255 } },
    { 4, "Viscount",       "Tử Tước",           "Viscount",           "Tử Tước",         1200,  90,  250, 125.0f, 24.0f,  4.0f, { 185, 145, 255, 255 } },
    { 5, "Earl",           "Bá Tước",           "Right Hon. Earl",    "Bá Tước",         1700, 140,  340, 165.0f, 32.0f,  5.2f, { 225, 120, 255, 255 } },
    { 6, "Marquess",       "Hầu Tước",          "Most Hon. Marquess", "Hầu Tước",        2300, 210,  450, 215.0f, 42.0f,  6.5f, { 255, 155,  75, 255 } },
    { 7, "Duke",           "Công Tước",         "His Grace Duke",     "Đại Công Tước",   3000, 300,  600, 275.0f, 55.0f,  8.0f, { 255, 205,  75, 255 } },
    { 8, "Sovereign King", "Quốc Vương Tối Cao","His Majesty King",   "Bệ Hạ Quốc Vương",   0,   0, 1000, 380.0f, 80.0f, 11.0f, { 255, 230,  90, 255 } }
};

// ── Quốc Gia, Bản Đồ Thế Giới & Quân Đồng Minh ───────────────────────────────
struct NationState {
    const char* nameEn;
    const char* nameVi;
    const char* capitalEn;
    const char* capitalVi;
    Vector2 mapPos;
    Color color;
    int militaryPower = 300;
    int territories = 3;
    int status = 0; // 0: Hòa bình, 1: Đồng minh, 2: Đang chiến tranh, 3: Đã chinh phạt
    float warProgress = 0.0f; // 0..100%
};

struct AlliedUnit {
    Vector2 pos = { 0, 0 };
    Vector2 aim = { 1, 0 };
    float hp = 200.0f;
    float maxHp = 200.0f;
    float damage = 25.0f;
    float atkTimer = 0.0f;
    float walkTimer = 0.0f;
    float swing = 0.0f;
    int heroClass = 0;
    bool facingLeft = false;
};

// ── Game State ─────────────────────────────────────────────────────────────
