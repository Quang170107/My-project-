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

        private void Awake()
        {
            GetComponent<BoxCollider2D>().isTrigger = true;
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
