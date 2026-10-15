using UnityEngine;

namespace SimpleRPG
{
    /// <summary>
    /// Vũng dung nham: gây sát thương theo nhịp cho người chơi đang đứng trên nó.
    /// Sát thương vẫn đi qua i-frame/armor của người chơi nên dash qua vũng là an toàn.
    /// </summary>
    [RequireComponent(typeof(BoxCollider2D))]
    public class HazardZone : MonoBehaviour
    {
        public float damage = 6f;
        public float tickInterval = 0.6f;

        private float _nextTick = 0f;
        private SpriteRenderer _sr;
        private float _phase;

        private void Awake()
        {
            GetComponent<BoxCollider2D>().isTrigger = true;
            _sr = GetComponent<SpriteRenderer>();
            _phase = Random.Range(0f, 6.28f);
        }

        // Nhịp sáng của dung nham
        private void Update()
        {
            if (_sr == null) return;
            float k = 0.5f + 0.5f * Mathf.Sin(Time.time * 3f + _phase);
            _sr.color = Color.Lerp(new Color(0.85f, 0.2f, 0.05f, 0.85f), new Color(1f, 0.55f, 0.1f, 0.95f), k);
        }

        private void OnTriggerStay2D(Collider2D other)
        {
            if (!other.CompareTag("Player")) return;
            if (Time.time < _nextTick) return;

            var player = other.GetComponent<PlayerController>();
            if (player == null) return;

            _nextTick = Time.time + tickInterval;
            player.TakeDamage(damage, Vector2.zero, 0f);
        }
    }
}
