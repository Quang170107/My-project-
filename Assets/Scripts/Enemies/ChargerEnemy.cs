using System.Collections;
using UnityEngine;

namespace SimpleRPG
{
    /// <summary>
    /// Quái húc: ngắm (đỏ dần để báo hiệu), lao thẳng một đường rất nhanh theo hướng đã khóa,
    /// rồi choáng một lúc - đó là cơ hội để đánh. Người chơi né bằng dash hoặc bước sang bên.
    /// </summary>
    public class ChargerEnemy : EnemyBase
    {
        private static readonly Color BaseTint = new Color(1f, 0.72f, 0.35f);

        private float _chargeCooldown = 1.5f;

        protected override void Awake()
        {
            base.Awake();
            enemyName = "Horned Charger";
            maxHealth = 55f;
            currentHealth = maxHealth;
            moveSpeed = 2.2f;
            contactDamage = 16f;
            xpDrop = 22;

            spriteRenderer.sprite = SpriteFactory.GetSprite("brute");
            spriteRenderer.color = BaseTint;
            transform.localScale = Vector3.one * 0.85f;

            var col = GetComponent<CircleCollider2D>();
            col.radius = 0.4f;
        }

        protected override void UpdateAI()
        {
            if (_chargeCooldown > 0f) _chargeCooldown -= Time.deltaTime;

            Vector2 toPlayer = (Vector2)playerTransform.position - (Vector2)transform.position;
            float dist = toPlayer.magnitude;
            Vector2 dir = toPlayer.normalized;

            if (_chargeCooldown <= 0f && dist > 2.5f && dist < 9f)
            {
                StartCoroutine(ChargeRoutine(dir));
                return;
            }

            SetVelocity(Vector2.Lerp(GetVelocity(), dir * moveSpeed, 8f * Time.deltaTime));
            spriteRenderer.flipX = dir.x < 0;
        }

        private IEnumerator ChargeRoutine(Vector2 dir)
        {
            isAttacking = true;
            SetVelocity(Vector2.zero);
            spriteRenderer.flipX = dir.x < 0;

            // 1. Báo hiệu: đỏ dần trong 0.7s, hướng lao bị khóa tại thời điểm này
            const float windup = 0.7f;
            for (float t = 0f; t < windup; t += Time.deltaTime)
            {
                if (!IsAlive) yield break;
                spriteRenderer.color = Color.Lerp(BaseTint, Color.red, t / windup);
                yield return null;
            }

            // 2. Lao thẳng
            const float dash = 0.55f;
            for (float t = 0f; t < dash; t += Time.deltaTime)
            {
                if (!IsAlive) yield break;
                SetVelocity(dir * 11f);
                yield return null;
            }

            // 3. Choáng: đứng yên, xám đi
            SetVelocity(Vector2.zero);
            spriteRenderer.color = new Color(0.6f, 0.6f, 0.7f);
            yield return new WaitForSeconds(0.9f);

            if (!IsAlive) yield break;
            spriteRenderer.color = BaseTint;
            _chargeCooldown = 2.2f;
            isAttacking = false;
        }

        private Vector2 GetVelocity()
        {
#if UNITY_6000_0_OR_NEWER
            return rb.linearVelocity;
#else
            return rb.velocity;
#endif
        }

        private void SetVelocity(Vector2 v)
        {
#if UNITY_6000_0_OR_NEWER
            rb.linearVelocity = v;
#else
            rb.velocity = v;
#endif
        }
    }
}
