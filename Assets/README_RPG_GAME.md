# Geometric Dungeon: 2D Roguelike RPG

Welcome to **Geometric Dungeon**, a fast-paced 2D Roguelike Action RPG built natively in Unity 6!

---

## 🎮 How to Play

### Option 1: Just Hit Play!
Simply click the **Play** button (`▶`) at the top of the Unity Editor in `SampleScene`.
The game automatically bootstraps the camera, player, audio, dungeon room, UI, and monsters!

### Option 2: Pre-populate Scene in Edit Mode
In Unity's top menu bar, click:
> **RPG Game** -> **Setup Playable Scene**

This places the Player, DungeonManager, UI, and Managers into the hierarchy for Edit Mode inspection.

---

## 🕹️ Controls

| Action | Primary Input | Secondary / Alternative |
| :--- | :--- | :--- |
| **Move** | `W` `A` `S` `D` | Arrow Keys / Left Gamepad Stick |
| **Aim** | Mouse Cursor | Facing Direction |
| **Melee Attack (Sword)** | Left Mouse Button (`LMB`) | `Space` / `J` |
| **Dash / Dodge Roll** | Right Mouse Button (`RMB`) | `Left Shift` / `K` (gives Invulnerability Frames) |
| **Special Skill (Nova)** | `Q` or `E` | `U` (360° Knockback & Damage Shockwave) |
| **Open Chest / Collect** | Walk near / Touch | Automatic magnetic pickup for XP & Coins |
| **Restart Game** | `R` (on Game Over screen) | Click "Play Again" button |

---

## ⚔️ Gameplay Mechanics

### 1. Floor Progression & The Portal
- You start in a fortified chamber on **Floor 1**.
- Each room contains waves of monsters, obstacles for tactical cover, and treasure chests.
- Defeat all monsters in the chamber to **open the Exit Portal**.
- Stepping onto the glowing portal teleports you deeper into the dungeon!

### 2. Monsters & Archetypes
- 🟢 **Slime Chaser** (lime slime): Bouncy, fast, lunges when in striking distance.
- 🔴 **Crimson Spitter** (horned red slime): Kites away, keeps distance, and fires aimed energy darts.
- 🟣 **Armored Slime** (purple slime with rock plates): High health, slow march, telegraphs a massive AOE ground stomp!
- 👑 **The Void Sovereign** (crowned boss slime on Floor 5, 10...): Giant slime that unleashes 10-way bullet spirals, charging rushes, and leaves behind massive loot chests!

### 3. Loot & Upgrades
- 🔷 **XP Gems**: Magnetically attracted to the hero; fill your level gauge.
- 🪙 **Gold Coins**: Currency collected on each run.
- 🧪 **Health Potions**: Restores 35-50 HP.
- 📦 **Treasure Chests**: Found in chambers and after bosses; open for bonus loot and gold.

### 4. Level-Up Cards
Every time you level up, the game pauses and presents **3 random upgrade cards**:
- **Titan's Heart**: +25 Max Health & instant heal
- **Sharpened Edge**: +8 Sword Damage
- **Swift Wind**: +15% Move Speed
- **Flurry**: +30% Attack Speed
- **Blink Step**: -0.2s Dash Cooldown
- **Vampiric Touch**: +8% Lifesteal on all melee attacks
- **Iron Aegis**: +2 Flat Damage Reduction Armor

### 5. Audiovisual Juice & Polish
- Procedural retro sound effects synthesized directly in code (zero missing audio files!).
- Floating damage numbers with color coding (white for enemy hit, red for player hurt, green for healing, yellow for gold).
- Hit flashes, camera screen shake, and squash-and-stretch walk bobbing.
- Dash afterimages / ghost trail effects.

---

## 📁 Project Architecture

- `Assets/Scripts/Player/`: `PlayerController.cs`, `CameraFollow.cs`, `InputHelper.cs`
- `Assets/Scripts/Combat/`: `CombatStats.cs`, `Projectile.cs`
- `Assets/Scripts/Enemies/`: `EnemyBase.cs`, `ChaserEnemy.cs`, `RangerEnemy.cs`, `BruteEnemy.cs`, `BossEnemy.cs`
- `Assets/Scripts/Loot/`: `LootItem.cs`, `LootManager.cs`
- `Assets/Scripts/Dungeon/`: `DungeonManager.cs`, `ExitPortal.cs`, `SpriteFactory.cs`, `GameBootstrap.cs`
- `Assets/Scripts/UI/`: `GameUI.cs`, `DamageNumberManager.cs`
- `Assets/Scripts/Audio/`: `AudioManager.cs`
- `Assets/Scripts/Editor/`: `RPGEditorSetup.cs`
