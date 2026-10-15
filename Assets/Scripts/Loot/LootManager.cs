using UnityEngine;

namespace SimpleRPG
{
    public class LootManager : MonoBehaviour
    {
        public static LootManager Instance { get; private set; }

        private void Awake()
        {
            if (Instance == null)
            {
                Instance = this;
            }
            else
            {
                Destroy(gameObject);
            }
        }

        public void SpawnXP(Vector3 pos, int amount)
        {
            var go = new GameObject("XP_Gem");
            go.transform.position = pos;
            var loot = go.AddComponent<LootItem>();
            loot.lootType = LootType.XP;
            loot.value = amount;
            loot.SetupVisual();
        }

        public void SpawnCoin(Vector3 pos, int amount)
        {
            var go = new GameObject("Gold_Coin");
            go.transform.position = pos;
            var loot = go.AddComponent<LootItem>();
            loot.lootType = LootType.Coin;
            loot.value = amount;
            loot.SetupVisual();
        }

        public void SpawnPotion(Vector3 pos, float heal)
        {
            var go = new GameObject("HP_Potion");
            go.transform.position = pos;
            var loot = go.AddComponent<LootItem>();
            loot.lootType = LootType.Potion;
            loot.healAmount = heal;
            loot.SetupVisual();
        }

        public void SpawnChest(Vector3 pos)
        {
            var go = new GameObject("Treasure_Chest");
            go.transform.position = pos;

            // Gắn vào Loot_Container để rương chưa mở được dọn khi sang tầng mới
            // (trước đây rương bị bỏ lại và chồng thêm ở mỗi tầng)
            var dm = DungeonManager.Instance;
            if (dm != null && dm.lootContainer != null)
            {
                go.transform.SetParent(dm.lootContainer, true);
            }
            var loot = go.AddComponent<LootItem>();
            loot.lootType = LootType.Chest;
            loot.SetupVisual();
        }
    }
}
