using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace SimpleRPG
{
    public class DungeonManager : MonoBehaviour
    {
        public static DungeonManager Instance { get; private set; }

        [Header("Progression")]
        public int currentFloor = 1;
        public int totalEnemiesSlain = 0;
        public int remainingEnemies = 0;

        [Header("Room Dimensions")]
        public int roomWidth = 22;
        public int roomHeight = 14;

        [Header("Prefabs / References")]
        public ExitPortal exitPortal;
        public Transform roomContainer;
        public Transform enemyContainer;
        public Transform lootContainer;

        private readonly List<EnemyBase> _activeEnemies = new List<EnemyBase>();

        private void Awake()
        {
            if (Instance == null)
            {
                Instance = this;
            }
            else
            {
                Destroy(gameObject);
                return;
            }

            Time.timeScale = 1f;
        }

        private void Start()
        {
            InitializeContainers();
            StartGame();
        }

        private void InitializeContainers()
        {
            if (roomContainer == null)
            {
                roomContainer = new GameObject("Room_Container").transform;
            }
            if (enemyContainer == null)
            {
                enemyContainer = new GameObject("Enemy_Container").transform;
            }
            if (lootContainer == null)
            {
                lootContainer = new GameObject("Loot_Container").transform;
            }
        }

        private bool _eventsConnected = false;

        public void StartGame()
        {
            currentFloor = 1;
            totalEnemiesSlain = 0;

            // Connect player events safely
            if (PlayerController.Instance != null && !_eventsConnected)
            {
                _eventsConnected = true;
                var stats = PlayerController.Instance.stats;
                stats.OnHealthChanged += (cur, max) => GameUI.Instance?.UpdateHealth(cur, max);
                stats.OnXPChanged += (cur, next) => GameUI.Instance?.UpdateXP(cur, next, stats.level);
                stats.OnGoldChanged += (gold) => GameUI.Instance?.UpdateGold(gold);
                stats.OnLevelUp += HandleLevelUp;
            }

            if (PlayerController.Instance != null)
            {
                var stats = PlayerController.Instance.stats;
                GameUI.Instance?.UpdateHealth(stats.currentHealth, stats.maxHealth);
                GameUI.Instance?.UpdateXP(stats.currentXP, stats.xpToNextLevel, stats.level);
                GameUI.Instance?.UpdateGold(stats.gold);
            }

            BuildFloor();
        }

        private void BuildFloor()
        {
            ClearPreviousFloor();

            GameUI.Instance?.UpdateFloor(currentFloor);
            GameUI.Instance?.ShowBanner($"FLOOR {currentFloor}", 2.5f);

            // 1. Build Room Tiles and Walls
            GenerateRoomGeometry();

            // 2. Position Player
            if (PlayerController.Instance != null)
            {
                PlayerController.Instance.transform.position = new Vector3(0, -roomHeight / 2f + 2.5f, 0);
            }

            // 3. Spawn Exit Portal (Top Center)
            SpawnPortal();

            // 4. Spawn Chests
            SpawnChests();

            // 5. Spawn Enemies for Floor
            SpawnFloorEnemies();
        }

        private void ClearPreviousFloor()
        {
            _activeEnemies.Clear();

            // Destroy old room tiles, enemies, and loot
            foreach (Transform child in roomContainer) Destroy(child.gameObject);
            foreach (Transform child in enemyContainer) Destroy(child.gameObject);
            foreach (Transform child in lootContainer) Destroy(child.gameObject);
        }

        private void GenerateRoomGeometry()
        {
            int halfW = roomWidth / 2;
            int halfH = roomHeight / 2;

            Sprite floorSprite = SpriteFactory.GetSprite("floor_tile");
            Sprite wallSprite = SpriteFactory.GetSprite("wall_tile");
            Sprite pillarSprite = SpriteFactory.GetSprite("pillar");

            // Floor tiles
            for (int x = -halfW; x <= halfW; x++)
            {
                for (int y = -halfH; y <= halfH; y++)
                {
                    var tile = new GameObject($"Floor_{x}_{y}");
                    tile.transform.SetParent(roomContainer, false);
                    tile.transform.position = new Vector3(x, y, 0);

                    var sr = tile.AddComponent<SpriteRenderer>();
                    sr.sprite = floorSprite;
                    sr.sortingOrder = 0;
                }
            }

            // Outer perimeter walls with colliders
            for (int x = -halfW - 1; x <= halfW + 1; x++)
            {
                CreateWallTile(new Vector3(x, halfH + 1, 0), wallSprite);
                CreateWallTile(new Vector3(x, -halfH - 1, 0), wallSprite);
            }
            for (int y = -halfH; y <= halfH; y++)
            {
                CreateWallTile(new Vector3(-halfW - 1, y, 0), wallSprite);
                CreateWallTile(new Vector3(halfW + 1, y, 0), wallSprite);
            }

            // Tactical cover pillars
            Vector2[] pillarPositions = new Vector2[]
            {
                new Vector2(-halfW + 4, halfH - 3),
                new Vector2(halfW - 4, halfH - 3),
                new Vector2(-halfW + 4, -halfH + 4),
                new Vector2(halfW - 4, -halfH + 4),
            };

            foreach (var pos in pillarPositions)
            {
                var pillar = new GameObject("Pillar");
                pillar.transform.SetParent(roomContainer, false);
                pillar.transform.position = pos;
                pillar.AddComponent<DungeonObstacle>();

                var sr = pillar.AddComponent<SpriteRenderer>();
                sr.sprite = pillarSprite;
                sr.sortingOrder = 8;

                var col = pillar.AddComponent<CircleCollider2D>();
                col.radius = 0.5f;
            }
        }

        private void CreateWallTile(Vector3 pos, Sprite wallSprite)
        {
            var wall = new GameObject("Wall");
            wall.transform.SetParent(roomContainer, false);
            wall.transform.position = pos;
            wall.AddComponent<DungeonObstacle>();

            var sr = wall.AddComponent<SpriteRenderer>();
            sr.sprite = wallSprite;
            sr.sortingOrder = 7;

            var col = wall.AddComponent<BoxCollider2D>();
            col.size = Vector2.one;
        }

        private void SpawnPortal()
        {
            var portalGo = new GameObject("ExitPortal");
            portalGo.transform.SetParent(roomContainer, false);
            portalGo.transform.position = new Vector3(0, roomHeight / 2f - 1.8f, 0);

            exitPortal = portalGo.AddComponent<ExitPortal>();
            exitPortal.SetOpen(false);
        }

        private void SpawnChests()
        {
            int chestCount = (currentFloor % 3 == 0) ? 2 : 1;
            float halfW = roomWidth / 2f - 2.5f;
            float halfH = roomHeight / 2f - 2.5f;

            if (chestCount >= 1)
            {
                LootManager.Instance?.SpawnChest(new Vector3(-halfW, halfH, 0));
            }
            if (chestCount >= 2)
            {
                LootManager.Instance?.SpawnChest(new Vector3(halfW, halfH, 0));
            }
        }

        private void SpawnFloorEnemies()
        {
            _activeEnemies.Clear();

            bool isBossFloor = (currentFloor % 5 == 0);

            if (isBossFloor)
            {
                // Boss + minion spawns
                SpawnBoss(new Vector3(0, 2f, 0));
                for (int i = 0; i < 4; i++)
                {
                    SpawnChaser(GetRandomSpawnPos());
                }
            }
            else
            {
                // Standard scaling waves
                int chaserCount = 3 + currentFloor;
                int rangerCount = Mathf.Max(0, (currentFloor - 1) * 2);
                int bruteCount = Mathf.Max(0, (currentFloor - 2));

                for (int i = 0; i < chaserCount; i++)
                {
                    SpawnChaser(GetRandomSpawnPos());
                }
                for (int i = 0; i < rangerCount; i++)
                {
                    SpawnRanger(GetRandomSpawnPos());
                }
                for (int i = 0; i < bruteCount; i++)
                {
                    SpawnBrute(GetRandomSpawnPos());
                }
            }

            remainingEnemies = _activeEnemies.Count;
            GameUI.Instance?.UpdateEnemiesRemaining(remainingEnemies);
        }

        private Vector3 GetRandomSpawnPos()
        {
            float halfW = roomWidth / 2f - 3f;
            float halfH = roomHeight / 2f - 3f;
            // Ensure enemy spawns away from player spawn point
            Vector3 pos;
            int tries = 0;
            do
            {
                pos = new Vector3(UnityEngine.Random.Range(-halfW, halfW), UnityEngine.Random.Range(-halfH + 3f, halfH), 0);
                tries++;
            }
            while (Vector3.Distance(pos, new Vector3(0, -roomHeight / 2f + 2.5f, 0)) < 3.5f && tries < 20);

            return pos;
        }

        private void SpawnChaser(Vector3 pos)
        {
            var go = new GameObject("Slime_Chaser");
            go.transform.SetParent(enemyContainer, false);
            go.transform.position = pos;
            var enemy = go.AddComponent<ChaserEnemy>();
            ScaleEnemy(enemy);
            _activeEnemies.Add(enemy);
        }

        private void SpawnRanger(Vector3 pos)
        {
            var go = new GameObject("Crimson_Ranger");
            go.transform.SetParent(enemyContainer, false);
            go.transform.position = pos;
            var enemy = go.AddComponent<RangerEnemy>();
            ScaleEnemy(enemy);
            _activeEnemies.Add(enemy);
        }

        private void SpawnBrute(Vector3 pos)
        {
            var go = new GameObject("Iron_Brute");
            go.transform.SetParent(enemyContainer, false);
            go.transform.position = pos;
            var enemy = go.AddComponent<BruteEnemy>();
            ScaleEnemy(enemy);
            _activeEnemies.Add(enemy);
        }

        private void SpawnBoss(Vector3 pos)
        {
            var go = new GameObject("Void_Sovereign_Boss");
            go.transform.SetParent(enemyContainer, false);
            go.transform.position = pos;
            var boss = go.AddComponent<BossEnemy>();
            ScaleEnemy(boss);
            _activeEnemies.Add(boss);
        }

        private void ScaleEnemy(EnemyBase enemy)
        {
            float hpMult = 1f + (currentFloor - 1) * 0.15f;
            float dmgMult = 1f + (currentFloor - 1) * 0.1f;
            enemy.maxHealth *= hpMult;
            enemy.currentHealth = enemy.maxHealth;
            enemy.contactDamage *= dmgMult;

            // Đảm bảo quái luôn nằm trên sàn, tường và cột
            var sr = enemy.GetComponentInChildren<SpriteRenderer>();
            if (sr != null)
            {
                sr.sortingOrder = 10;
            }
        }

        public void OnEnemyDefeated(EnemyBase enemy)
        {
            _activeEnemies.Remove(enemy);
            totalEnemiesSlain++;
            remainingEnemies = Mathf.Max(0, _activeEnemies.Count);

            GameUI.Instance?.UpdateEnemiesRemaining(remainingEnemies);

            if (remainingEnemies <= 0)
            {
                OnRoomCleared();
            }
        }

        private void OnRoomCleared()
        {
            AudioManager.Instance?.PlaySound("portal");
            GameUI.Instance?.ShowBanner("ROOM CLEARED! DESCEND THROUGH THE PORTAL!", 4f);
            if (exitPortal != null)
            {
                exitPortal.SetOpen(true);
            }
        }

        public void GoToNextFloor()
        {
            currentFloor++;
            BuildFloor();
        }

        private void HandleLevelUp(int newLevel)
        {
            // Pick 3 random distinct upgrades
            var allUpgrades = (UpgradeType[])Enum.GetValues(typeof(UpgradeType));
            var shuffled = new List<UpgradeType>(allUpgrades);
            for (int i = 0; i < shuffled.Count; i++)
            {
                int r = UnityEngine.Random.Range(i, shuffled.Count);
                var temp = shuffled[i];
                shuffled[i] = shuffled[r];
                shuffled[r] = temp;
            }

            var choices = new UpgradeType[] { shuffled[0], shuffled[1], shuffled[2] };
            GameUI.Instance?.ShowLevelUpModal(choices);
        }

        public void OnPlayerDied()
        {
            int lvl = PlayerController.Instance != null ? PlayerController.Instance.stats.level : 1;
            int gold = PlayerController.Instance != null ? PlayerController.Instance.stats.gold : 0;
            GameUI.Instance?.ShowGameOverModal(currentFloor, lvl, gold, totalEnemiesSlain);
        }

        public void RestartGame()
        {
            Time.timeScale = 1f;

            GameUI.Instance?.HideGameOverModal();

            if (PlayerController.Instance == null)
            {
                GameBootstrap.SpawnPlayer();
            }
            else
            {
                PlayerController.Instance.ResetPlayer(new Vector3(0, -roomHeight / 2f + 2.5f, 0));
            }

            StartGame();
        }
    }
}
