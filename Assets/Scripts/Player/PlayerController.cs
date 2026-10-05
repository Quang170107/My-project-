using System.Collections;
using UnityEngine;

namespace SimpleRPG
{
    [RequireComponent(typeof(Rigidbody2D), typeof(CircleCollider2D), typeof(CombatStats))]
    public class PlayerController : MonoBehaviour
    {
        public static PlayerController Instance { get; private set; }

        [Header("Components")]
        public CombatStats stats;
        public Rigidbody2D rb;
        public SpriteRenderer bodyRenderer;
        public Transform weaponPivot;
        public SpriteRenderer weaponRenderer;

        [Header("State")]
        public bool isDashing = false;
        public bool isAttacking = false;
        public float dashCooldownTimer = 0f;
        public float specialCooldownTimer = 0f;
        public const float SpecialMaxCooldown = 6f;

        private Vector2 _moveInput;
        private Vector2 _aimDirection;
        private float _attackTimer = 0f;
        private Vector3 _initialScale;
        private float _walkBobTimer = 0f;

        private void Awake()
        {
            Instance = this;
            stats = GetComponent<CombatStats>();
            rb = GetComponent<Rigidbody2D>();
            _initialScale = transform.localScale;

            // Configure physics
            rb.gravityScale = 0f;
            rb.freezeRotation = true;
            rb.collisionDetectionMode = CollisionDetectionMode2D.Continuous;

            var col = GetComponent<CircleCollider2D>();
            col.radius = 0.42f;

            gameObject.tag = "Player";
        }

        private void Start()
        {
            SetupVisuals();
            stats.OnDeath += HandleDeath;
        }

        private void OnDestroy()
        {
            if (stats != null)
            {
                stats.OnDeath -= HandleDeath;
            }
        }

        private void SetupVisuals()
        {
            if (bodyRenderer == null)
            {
                bodyRenderer = gameObject.AddComponent<SpriteRenderer>();
            }
            bodyRenderer.sprite = SpriteFactory.GetSprite("player");
            bodyRenderer.sortingOrder = 10;

            if (weaponPivot == null)
            {
                var pivotGo = new GameObject("WeaponPivot");
                pivotGo.transform.SetParent(transform, false);
                weaponPivot = pivotGo.transform;

                var swordGo = new GameObject("Sword");
                swordGo.transform.SetParent(weaponPivot, false);
                swordGo.transform.localPosition = new Vector3(0.5f, 0f, 0f);
                swordGo.transform.localRotation = Quaternion.Euler(0, 0, -45f);

                weaponRenderer = swordGo.AddComponent<SpriteRenderer>();
                weaponRenderer.sprite = SpriteFactory.GetSprite("sword");
                weaponRenderer.sortingOrder = 11;
            }
        }

        private void Update()
        {
            if (stats.currentHealth <= 0) return;

            // Timers
            if (dashCooldownTimer > 0f) dashCooldownTimer -= Time.deltaTime;
            if (specialCooldownTimer > 0f) specialCooldownTimer -= Time.deltaTime;
            if (_attackTimer > 0f) _attackTimer -= Time.deltaTime;

            // Input
            _moveInput = InputHelper.GetMoveInput();
            Vector2 mouseWorld = InputHelper.GetAimWorldPosition(Camera.main);
            _aimDirection = (mouseWorld - (Vector2)transform.position).normalized;

            // Weapon aim rotation
            if (weaponPivot != null && !isAttacking)
            {
                float aimAngle = Mathf.Atan2(_aimDirection.y, _aimDirection.x) * Mathf.Rad2Deg;
                weaponPivot.rotation = Quaternion.Euler(0, 0, aimAngle);
            }

            // Attack input
            if ((InputHelper.GetAttackDown() || InputHelper.GetAttackHeld()) && _attackTimer <= 0f && !isDashing)
            {
                PerformMeleeAttack();
            }

            // Dash input
            if (InputHelper.GetDashDown() && dashCooldownTimer <= 0f && !isDashing)
            {
                StartCoroutine(PerformDash());
            }

            // Special Skill (Nova)
            if (InputHelper.GetSpecialDown() && specialCooldownTimer <= 0f)
            {
                PerformSpecialNova();
            }

            // Walk bob animation
            AnimateWalkSquash();
        }

        private void FixedUpdate()
        {
            if (stats.currentHealth <= 0)
            {
#if UNITY_6000_0_OR_NEWER
                rb.linearVelocity = Vector2.zero;
#else
                rb.velocity = Vector2.zero;
#endif
                return;
            }

            if (!isDashing)
            {
                Vector2 targetVelocity = _moveInput * stats.moveSpeed;
#if UNITY_6000_0_OR_NEWER
                rb.linearVelocity = Vector2.Lerp(rb.linearVelocity, targetVelocity, 18f * Time.fixedDeltaTime);
#else
                rb.velocity = Vector2.Lerp(rb.velocity, targetVelocity, 18f * Time.fixedDeltaTime);
#endif
            }
        }

        private void AnimateWalkSquash()
        {
            if (_moveInput.sqrMagnitude > 0.01f)
            {
                _walkBobTimer += Time.deltaTime * stats.moveSpeed * 2.5f;
                float squash = Mathf.Sin(_walkBobTimer) * 0.08f;
                transform.localScale = new Vector3(_initialScale.x * (1f + squash), _initialScale.y * (1f - squash), _initialScale.z);
            }
            else
            {
                _walkBobTimer = 0f;
                transform.localScale = Vector3.Lerp(transform.localScale, _initialScale, 10f * Time.deltaTime);
            }
        }

        private void PerformMeleeAttack()
        {
            _attackTimer = 1f / stats.attackRate;
            StartCoroutine(AttackSwingRoutine());
        }

        private IEnumerator AttackSwingRoutine()
        {
            isAttacking = true;
            AudioManager.Instance?.PlaySound("slash");

            // Slash arc effect
            SpawnSlashEffect();

            // Swing sword through 120-degree arc
            float baseAngle = Mathf.Atan2(_aimDirection.y, _aimDirection.x) * Mathf.Rad2Deg;
            float startAngle = baseAngle + 65f;
            float endAngle = baseAngle - 65f;
            float swingDuration = 0.15f;
            float elapsed = 0f;

            // Hit detection in front arc
            Collider2D[] hits = Physics2D.OverlapCircleAll(transform.position + (Vector3)(_aimDirection * 0.8f), stats.attackRange);
            bool hitAny = false;

            foreach (var hit in hits)
            {
                if (hit.gameObject == gameObject) continue;
                var enemy = hit.GetComponent<EnemyBase>();
                if (enemy != null && enemy.IsAlive)
                {
                    hitAny = true;
                    Vector2 hitDir = ((Vector2)enemy.transform.position - (Vector2)transform.position).normalized;
                    enemy.TakeDamage(stats.baseDamage, hitDir, 6f);

                    // Lifesteal
                    if (stats.lifesteal > 0f)
                    {
                        float healAmount = stats.baseDamage * stats.lifesteal;
                        stats.Heal(healAmount);
                    }
                }
            }

            if (hitAny)
            {
                AudioManager.Instance?.PlaySound("hit");
                CameraFollow.Instance?.Shake(0.12f, 0.15f);
            }

            while (elapsed < swingDuration)
            {
                elapsed += Time.deltaTime;
                float t = elapsed / swingDuration;
                float currentAngle = Mathf.Lerp(startAngle, endAngle, Mathf.SmoothStep(0f, 1f, t));
                if (weaponPivot != null)
                {
                    weaponPivot.rotation = Quaternion.Euler(0, 0, currentAngle);
                }
                yield return null;
            }

            isAttacking = false;
        }

        private void SpawnSlashEffect()
        {
            var slashGo = new GameObject("SlashEffect");
            slashGo.transform.position = transform.position + (Vector3)(_aimDirection * 0.7f);
            float angle = Mathf.Atan2(_aimDirection.y, _aimDirection.x) * Mathf.Rad2Deg;
            slashGo.transform.rotation = Quaternion.Euler(0, 0, angle);

            var sr = slashGo.AddComponent<SpriteRenderer>();
            sr.sprite = SpriteFactory.GetSprite("slash");
            sr.sortingOrder = 12;

            var anim = slashGo.AddComponent<QuickFadeAnimation>();
            anim.duration = 0.15f;
        }

        private IEnumerator PerformDash()
        {
            isDashing = true;
            dashCooldownTimer = stats.dashCooldown;
            stats.isInvulnerable = true;
            AudioManager.Instance?.PlaySound("dash");

            Vector2 dashDir = _moveInput.sqrMagnitude > 0.01f ? _moveInput.normalized : _aimDirection;
#if UNITY_6000_0_OR_NEWER
            rb.linearVelocity = dashDir * stats.dashSpeed;
#else
            rb.velocity = dashDir * stats.dashSpeed;
#endif

            // Ghost trail effect
            StartCoroutine(SpawnGhostTrail(stats.dashDuration));

            // Translucent cyan color during i-frames
            Color origColor = bodyRenderer.color;
            bodyRenderer.color = new Color(0.4f, 1f, 1f, 0.6f);

            yield return new WaitForSeconds(stats.dashDuration);

            bodyRenderer.color = origColor;
            stats.isInvulnerable = false;
            isDashing = false;
        }

        private IEnumerator SpawnGhostTrail(float duration)
        {
            float elapsed = 0f;
            while (elapsed < duration)
            {
                var ghost = new GameObject("Ghost");
                ghost.transform.position = transform.position;
                ghost.transform.rotation = transform.rotation;
                ghost.transform.localScale = transform.localScale;

                var sr = ghost.AddComponent<SpriteRenderer>();
                sr.sprite = bodyRenderer.sprite;
                sr.color = new Color(0.2f, 0.8f, 1f, 0.45f);
                sr.sortingOrder = 9;

                var fade = ghost.AddComponent<QuickFadeAnimation>();
                fade.duration = 0.22f;

                yield return new WaitForSeconds(0.04f);
                elapsed += 0.04f;
            }
        }

        private void PerformSpecialNova()
        {
            specialCooldownTimer = SpecialMaxCooldown;
            AudioManager.Instance?.PlaySound("special");
            CameraFollow.Instance?.Shake(0.25f, 0.35f);

            // Radial shockwave visual
            var novaGo = new GameObject("NovaRing");
            novaGo.transform.position = transform.position;
            var sr = novaGo.AddComponent<SpriteRenderer>();
            sr.sprite = SpriteFactory.CreateCircle(64, Color.clear, new Color(0.3f, 0.95f, 1f, 0.9f), 4);
            sr.sortingOrder = 15;

            var expand = novaGo.AddComponent<NovaExpandAnimation>();
            expand.maxRadius = 3.5f;
            expand.duration = 0.35f;

            // Damage all enemies in 3.5m radius
            Collider2D[] hits = Physics2D.OverlapCircleAll(transform.position, 3.5f);
            foreach (var hit in hits)
            {
                var enemy = hit.GetComponent<EnemyBase>();
                if (enemy != null && enemy.IsAlive)
                {
                    Vector2 pushDir = ((Vector2)enemy.transform.position - (Vector2)transform.position).normalized;
                    enemy.TakeDamage(stats.baseDamage * 1.8f, pushDir, 12f);
                }
            }
        }

        public void TakeDamage(float damage, Vector2 knockbackDir, float knockbackForce)
        {
            if (stats.isInvulnerable || stats.currentHealth <= 0) return;

            stats.TakeDamage(damage, knockbackDir, knockbackForce);
            AudioManager.Instance?.PlaySound("hurt");
            CameraFollow.Instance?.Shake(0.18f, 0.25f);
            StartCoroutine(DamageFlashRoutine());
        }

        private IEnumerator DamageFlashRoutine()
        {
            Color orig = bodyRenderer.color;
            bodyRenderer.color = new Color(1f, 0.2f, 0.2f, 1f);
            yield return new WaitForSeconds(0.1f);
            bodyRenderer.color = orig;
        }

        private void HandleDeath()
        {
            AudioManager.Instance?.PlaySound("enemy_death");
            CameraFollow.Instance?.Shake(0.4f, 0.45f);
            bodyRenderer.color = new Color(0.4f, 0.4f, 0.4f, 0.5f);
            DungeonManager.Instance?.OnPlayerDied();
        }

        public void ResetPlayer(Vector3 spawnPos)
        {
            transform.position = spawnPos;
            transform.localScale = _initialScale;
            isDashing = false;
            isAttacking = false;
            dashCooldownTimer = 0f;
            specialCooldownTimer = 0f;
            _attackTimer = 0f;
            _walkBobTimer = 0f;

            if (bodyRenderer != null)
            {
                bodyRenderer.color = Color.white;
            }

            if (rb != null)
            {
#if UNITY_6000_0_OR_NEWER
                rb.linearVelocity = Vector2.zero;
#else
                rb.velocity = Vector2.zero;
#endif
            }

            if (stats != null)
            {
                stats.ResetStats();
            }
        }
    }

    public class QuickFadeAnimation : MonoBehaviour
    {
        public float duration = 0.2f;
        private SpriteRenderer _sr;
        private float _elapsed = 0f;
        private Color _initColor;

        private void Start()
        {
            _sr = GetComponent<SpriteRenderer>();
            _initColor = _sr != null ? _sr.color : Color.white;
        }

        private void Update()
        {
            _elapsed += Time.deltaTime;
            float t = _elapsed / duration;
            if (t >= 1f)
            {
                Destroy(gameObject);
                return;
            }

            if (_sr != null)
            {
                _sr.color = new Color(_initColor.r, _initColor.g, _initColor.b, Mathf.Lerp(_initColor.a, 0f, t));
            }
        }
    }

    public class NovaExpandAnimation : MonoBehaviour
    {
        public float maxRadius = 3.5f;
        public float duration = 0.35f;
        private float _elapsed = 0f;
        private SpriteRenderer _sr;

        private void Start()
        {
            _sr = GetComponent<SpriteRenderer>();
            transform.localScale = Vector3.zero;
        }

        private void Update()
        {
            _elapsed += Time.deltaTime;
            float t = _elapsed / duration;
            if (t >= 1f)
            {
                Destroy(gameObject);
                return;
            }

            float currentRadius = Mathf.Lerp(0f, maxRadius * 2f, Mathf.Sqrt(t));
            transform.localScale = new Vector3(currentRadius, currentRadius, 1f);

            if (_sr != null)
            {
                _sr.color = new Color(0.3f, 0.95f, 1f, 1f - t);
            }
        }
    }
}
