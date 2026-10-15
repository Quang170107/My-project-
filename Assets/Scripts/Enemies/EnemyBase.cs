using System.Collections;
using UnityEngine;

namespace SimpleRPG
{
    [RequireComponent(typeof(Rigidbody2D), typeof(CircleCollider2D))]
    public abstract class EnemyBase : MonoBehaviour
    {
        [Header("Enemy Stats")]
        public string enemyName = "Enemy";
        public float maxHealth = 40f;
        public float currentHealth;
        public float moveSpeed = 3f;
        public float contactDamage = 10f;
        public int xpDrop = 15;
        public int coinDropChance = 50; // percentage
        public int potionDropChance = 15; // percentage

        public bool IsAlive => currentHealth > 0;

        protected Rigidbody2D rb;
        protected SpriteRenderer spriteRenderer;
        protected Transform playerTransform;
        protected bool isAttacking = false;

        // Mini health bar
        private Transform _healthBarRoot;
        private Transform _healthBarFill;

        protected virtual void Awake()
        {
            rb = GetComponent<Rigidbody2D>();
            rb.gravityScale = 0f;
            rb.freezeRotation = true;
#if UNITY_6000_0_OR_NEWER
            rb.linearDamping = 4f;
#else
            rb.drag = 4f;
#endif

            spriteRenderer = GetComponent<SpriteRenderer>();
            if (spriteRenderer == null)
            {
                spriteRenderer = gameObject.AddComponent<SpriteRenderer>();
            }

            // Quái mặc định ở order 0 trùng với sàn; đặt cao hơn vết bẩn/dung nham nhưng thấp hơn tường
            if (spriteRenderer.sortingOrder == 0) spriteRenderer.sortingOrder = 6;

            currentHealth = maxHealth;
            CreateMiniHealthBar();
        }

        protected virtual void Start()
        {
            if (PlayerController.Instance != null)
            {
                playerTransform = PlayerController.Instance.transform;
            }
            PositionHealthBar();
        }

        private void PositionHealthBar()
        {
            if (_healthBarRoot == null || spriteRenderer == null || spriteRenderer.sprite == null)
                return;
            float top = spriteRenderer.sprite.bounds.max.y + 0.16f;
            _healthBarRoot.localPosition = new Vector3(0f, top, 0f);
        }

        private void CreateMiniHealthBar()
        {
            var barGo = new GameObject("HealthBar");
            barGo.transform.SetParent(transform, false);
            barGo.transform.localPosition = new Vector3(0, 0.7f, 0);
            _healthBarRoot = barGo.transform;

            // Background
            var bgGo = new GameObject("BG");
            bgGo.transform.SetParent(barGo.transform, false);
            var bgSr = bgGo.AddComponent<SpriteRenderer>();
            bgSr.sprite = SpriteFactory.CreateRoundedRect(36, 6, 2, new Color(0.1f, 0.1f, 0.1f, 0.8f), Color.clear, 0);
            bgSr.sortingOrder = 20;

            // Fill
            var fillGo = new GameObject("Fill");
            fillGo.transform.SetParent(barGo.transform, false);
            var fillSr = fillGo.AddComponent<SpriteRenderer>();
            fillSr.sprite = SpriteFactory.CreateRoundedRect(34, 4, 1, new Color(0.9f, 0.2f, 0.25f, 1f), Color.clear, 0);
            fillSr.sortingOrder = 21;
            _healthBarFill = fillGo.transform;

            _healthBarRoot.gameObject.SetActive(false); // only show when damaged
        }

        private GameObject _shadowGo;
        private bool _shadowAdded = false;

        /// <summary>Màu tia lửa khi quái bị đánh trúng hoặc chết.</summary>
        protected virtual Color HitFxColor { get { return new Color(0.9f, 0.3f, 0.25f); } }

        private void AddShadow()
        {
            _shadowAdded = true;
            if (spriteRenderer == null || spriteRenderer.sprite == null) return;

            var b = spriteRenderer.sprite.bounds;
            float width = Mathf.Max(0.6f, b.size.x * 0.85f);
            _shadowGo = VisualFX.AddShadow(transform, width, b.min.y + 0.06f, Mathf.Max(0, spriteRenderer.sortingOrder - 1));
        }

        protected virtual void Update()
        {
            if (!IsAlive) return;

            if (!_shadowAdded) AddShadow();

            if (playerTransform == null && PlayerController.Instance != null)
            {
                playerTransform = PlayerController.Instance.transform;
            }

            if (playerTransform != null && !isAttacking)
            {
                UpdateAI();
            }
        }

        protected abstract void UpdateAI();

        public virtual void TakeDamage(float damage, Vector2 knockbackDir, float knockbackForce = 5f)
        {
            if (!IsAlive) return;

            currentHealth = Mathf.Max(0, currentHealth - damage);

            // Floating damage text
            DamageNumberManager.Instance?.SpawnNumber(transform.position + Vector3.up * 0.3f, Mathf.RoundToInt(damage), Color.white);

            // Update mini health bar
            if (_healthBarRoot != null)
            {
                _healthBarRoot.gameObject.SetActive(true);
                float pct = currentHealth / maxHealth;
                _healthBarFill.localScale = new Vector3(Mathf.Clamp01(pct), 1f, 1f);
            }

            // Knockback
            if (rb != null && knockbackForce > 0f)
            {
                rb.AddForce(knockbackDir.normalized * knockbackForce, ForceMode2D.Impulse);
            }

            // Hit flash
            StartCoroutine(FlashRoutine());
            VisualFX.Burst(transform.position, HitFxColor, 5, 3f, 0.13f);

            if (currentHealth <= 0)
            {
                Die();
            }
        }

        private IEnumerator FlashRoutine()
        {
            Color orig = spriteRenderer.color;
            spriteRenderer.color = Color.white * 2f;
            yield return new WaitForSeconds(0.08f);
            spriteRenderer.color = orig;
        }

        protected virtual void Die()
        {
            AudioManager.Instance?.PlaySound("enemy_death");

            // Drop loot
            DropLoot();

            // Notify DungeonManager
            DungeonManager.Instance?.OnEnemyDefeated(this);

            // Death poof animation
            VisualFX.Burst(transform.position, HitFxColor, 16, 5.5f, 0.22f);
            if (_shadowGo != null) _shadowGo.SetActive(false);
            StartCoroutine(DeathRoutine());
        }

        private void DropLoot()
        {
            // Always drop XP gem
            LootManager.Instance?.SpawnXP(transform.position, xpDrop);

            // Chance of coin
            if (Random.Range(0, 100) < coinDropChance)
            {
                LootManager.Instance?.SpawnCoin(transform.position + (Vector3)Random.insideUnitCircle * 0.4f, Random.Range(1, 4));
            }

            // Chance of health potion
            if (Random.Range(0, 100) < potionDropChance)
            {
                LootManager.Instance?.SpawnPotion(transform.position + (Vector3)Random.insideUnitCircle * 0.4f, 35f);
            }
            // Chance of equipment (boss luôn rơi)
            int floor = DungeonManager.Instance != null ? DungeonManager.Instance.currentFloor : 1;
            bool isBoss = this is BossEnemy;
            EquipmentPickup.TryDrop(transform.position + (Vector3)Random.insideUnitCircle * 0.4f, 0.12f, floor, isBoss);
        }

        private IEnumerator DeathRoutine()
        {
            GetComponent<Collider2D>().enabled = false;
            float elapsed = 0f;
            float duration = 0.25f;
            Vector3 initScale = transform.localScale;

            while (elapsed < duration)
            {
                elapsed += Time.deltaTime;
                float t = elapsed / duration;
                transform.localScale = Vector3.Lerp(initScale, Vector3.zero, t);
                spriteRenderer.color = new Color(1f, 1f, 1f, 1f - t);
                yield return null;
            }

            Destroy(gameObject);
        }

        private void OnCollisionStay2D(Collision2D collision)
        {
            if (!IsAlive) return;

            if (collision.gameObject.CompareTag("Player"))
            {
                var player = collision.gameObject.GetComponent<PlayerController>();
                if (player != null)
                {
                    Vector2 dir = ((Vector2)player.transform.position - (Vector2)transform.position).normalized;
                    player.TakeDamage(contactDamage, dir, 5f);
                }
            }
        }
    }
}
