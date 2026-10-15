# Geometric Dungeon: 2D Roguelike RPG

Welcome to **Geometric Dungeon**, a fast-paced 2D Roguelike Action RPG built natively in Unity 6!

---

## 🎮 How to Play

### Option 1: Just Hit Play!
Simply click the **Play** button (`▶`) at the top of the Unity Editor in `SampleScene`.
The game automatically bootstraps the camera, player, audio, dungeon room, UI, shop, inventory, and monsters!

### Option 2: Pre-populate Scene in Edit Mode
In Unity's top menu bar, click:
> **RPG Game** -> **Setup Playable Scene**

This places all Managers, UI, and Player directly into the hierarchy for Edit Mode inspection.

---

## 🕹️ Controls

| Action | Primary Input | Secondary / Alternative | Description |
| :--- | :--- | :--- | :--- |
| **Move** | `W` `A` `S` `D` | Arrow Keys / Left Gamepad Stick | Smooth 8-directional movement with walk squash |
| **Aim** | Mouse Cursor | Facing Direction | Weapon rotates towards aim, knight flips facing direction |
| **Melee Attack** | Left Mouse Button (`LMB`) | `Space` / `J` | 120° slash arc, knocks back enemies |
| **Dash / Dodge Roll** | Right Mouse Button (`RMB`) | `Left Shift` / `K` | Speed burst + Invulnerability Frames + Ghost trail |
| **Special Skill (Nova)** | `Q` or `E` | `U` | 360° radial pulse pushing & damaging all nearby foes |
| **Inventory (Kho đồ)** | `I` | `Esc` to close | View 4 equipped slots, stats breakdown & pause game |
| **Settings (Cài đặt)** | `Esc` (Settings Button in UI) | - | Resolution, VSync, Screen Shake toggle, Window mode |
| **Open Chest / Collect** | Walk near / Touch | - | XP gems & coins are magnetically drawn to you |
| **Restart Game** | `R` (on Game Over screen) | Click "Play Again" / Click anywhere | Restarts a fresh dungeon run |

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

### 3. Equipment & Inventory System (Kho đồ)
- **4 Equipment Slots**: Weapon, Armor, Boots, Ring.
- **4 Rarity Tiers**: Common (White), Rare (Blue), Epic (Purple), Legendary (Orange).
- **Dynamic Weapons**: Equipping Swords, Blades, Axes, or Spears dynamically updates the weapon sprite and scale held in the player's hands!
- **Auto-Equip & Auto-Sell**: Walking over equipment drops automatically equips them if they are stronger than your current gear, or sells them for bonus gold if weaker!

### 4. Merchant Shop (Cửa hàng Thương nhân)
- Spawns every 3 floors (Floors 3, 6, 9...).
- Rest area with a floating Merchant NPC.
- Sells 4 level-scaled equipment pieces + instant Health Potions.
- Spend your accumulated Gold to power up before upcoming Boss floors!

### 5. Level-Up Upgrade Cards
Every time you level up, the game pauses and presents **3 random upgrade cards**:
- **Titan's Heart**: +25 Max Health & instant heal
- **Sharpened Edge**: +8 Sword Damage
- **Swift Wind**: +15% Move Speed
- **Flurry**: +30% Attack Speed
- **Blink Step**: -0.2s Dash Cooldown
- **Vampiric Touch**: +8% Lifesteal on all melee attacks
- **Iron Aegis**: +2 Flat Damage Reduction Armor

### 6. Audiovisual Juice & Polish
- Procedural retro sound effects synthesized directly in code (zero missing audio files!).
- Floating damage numbers with color coding (white for enemy hit, red for player hurt, green for healing, yellow for gold).
- Hit flashes, camera screen shake, and squash-and-stretch walk bobbing.
- Dash afterimages / ghost trail effects.

---

## 📁 Project Architecture

- `Assets/Scripts/Player/`: `PlayerController.cs`, `CameraFollow.cs`, `InputHelper.cs`
- `Assets/Scripts/`: `EquipmentItem.cs`, `EquipmentManager.cs`, `EquipmentIcons.cs`, `EquipmentPickup.cs`, `InventoryUI.cs`, `PlayerWeaponVisual.cs`
- `Assets/Scripts/Combat/`: `CombatStats.cs`, `Projectile.cs`
- `Assets/Scripts/Enemies/`: `EnemyBase.cs`, `ChaserEnemy.cs`, `RangerEnemy.cs`, `BruteEnemy.cs`, `BossEnemy.cs`
- `Assets/Scripts/Loot/`: `LootItem.cs`, `LootManager.cs`
- `Assets/Scripts/Dungeon/`: `DungeonManager.cs`, `DungeonObstacle.cs`, `ExitPortal.cs`, `ShopManager.cs`, `SpriteFactory.cs`, `EnemySpriteBytes.cs`, `GameBootstrap.cs`
- `Assets/Scripts/UI/`: `GameUI.cs`, `DamageNumberManager.cs`, `SettingsManager.cs`
- `Assets/Scripts/Editor/`: `RPGEditorSetup.cs`

## 🆕 Changelog (balance & polish pass)

- **Fix:** Boss floors (5, 10, 15, 30...) are no longer replaced by the Merchant Shop when they coincide with a shop floor.
- **Balance:** Armor is now a percentage reduction with diminishing returns (armor 10 = -50% damage) instead of a flat reduction that made enemies deal 1 damage.
- **Balance:** Lifesteal is capped at 30% and heals once per swing instead of once per enemy hit.
- **Balance:** Enemy counts per floor are capped (10 Chasers / 8 Spitters / 5 Armored Slimes) so late floors stay readable.
- **Fix:** Gaining several levels at once (e.g. from a boss kill) now queues the upgrade cards instead of overwriting them.
- **New:** Cover pillars are randomised every floor (4-7 pillars), keeping spawn, portal and chest areas clear. Enemies no longer spawn inside pillars.
- **New:** Boss enrages below 50% HP (faster movement, shorter attack cycle, 16-way bullet ring).
- **New:** Best floor is saved (PlayerPrefs) and shown on the Game Over screen, with a NEW RECORD marker.
- **Fix:** The shop no longer lets you buy a potion at full HP or gear that is not an upgrade (previously the gold was spent and only partly refunded).
- **Repo:** Added MIT license.

## 🗺️ Level variety update

- **4 biomes** that change every 3 floors (Ancient Crypt, Overgrown Ruins, Molten Depths, Frozen Halls): different colour palette and a different enemy mix (e.g. Ruins = more Chasers, Molten = more Spitters, Frozen = more Armored Slimes).
- **5 room layouts**, never repeated twice in a row: Scattered pillars, Pillar Ring, 2x2 Blocks, Bastion walls and an Open arena. Boss and shop floors avoid layouts that block them.
- **Floor modifiers** on normal floors: Swarm, Elite Hunt, Sniper Nest, and an enemy-free Treasure Vault with extra chests.
- Checkerboard floor shading for a bit more depth.

## ⚔️ Enemy & hazard update (branch `claude/improvements`)

- **New enemy - Horned Charger** (from floor 4): telegraphs by turning red, locks its direction, dashes in a straight line, then is stunned for ~1s. Dodge with dash or a sidestep, then punish. More common on Elite Hunt floors, absent from Swarm and Sniper Nest floors.
- **Lava pools** in *Molten Depths* (floors 7-9, 16-18...): tick damage while you stand in them (scales with floor). Dashing through is safe thanks to i-frames. They avoid the spawn, portal, chests, pillars and the boss.
- **Fix:** unopened chests were never cleaned up and stacked on top of each other from floor to floor.

## ✨ Graphics update (branch `claude/improvements`)

All effects are generated in code (no art assets needed):
- **Ambient particles per biome:** drifting dust (Crypt), fireflies (Ruins), rising embers (Molten Depths), falling snow (Frozen Halls).
- **Screen vignette** tinted by the biome, with a red pulse when the player is hurt.
- **Soft ground shadows** under every enemy, and **hit / death spark bursts**; the player gets a hurt burst too.
- **Floor decals** (grime, moss, scorch marks, frost) scattered around each room.
- **Glow:** purple halo on the open exit portal, golden halo on chests, pulsing orange halo on lava pools.
- Enemies now have an explicit sorting order, so lava and decals can never draw on top of them.

