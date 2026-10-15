using UnityEngine;

namespace SimpleRPG
{
    [RequireComponent(typeof(CircleCollider2D))]
    public class ExitPortal : MonoBehaviour
    {
        public bool isOpen = false;

        private SpriteRenderer _sr;
        private CircleCollider2D _col;
        private float _spinSpeed = 60f;

        private void Awake()
        {
            _sr = GetComponent<SpriteRenderer>();
            if (_sr == null) _sr = gameObject.AddComponent<SpriteRenderer>();

            _col = GetComponent<CircleCollider2D>();
            _col.isTrigger = true;
            _col.radius = 0.8f;

            _sr.sprite = SpriteFactory.GetSprite("portal");
            _sr.sortingOrder = 3;

            SetOpen(false);
        }

        private void Update()
        {
            if (isOpen)
            {
                transform.Rotate(0, 0, -_spinSpeed * Time.deltaTime);
            }
        }

        private GameObject _glow;

        public void SetOpen(bool open)
        {
            isOpen = open;

            // Vầng sáng tím quanh cổng khi đã mở
            if (_glow == null) _glow = VisualFX.AddGlow(transform, new Color(0.55f, 0.45f, 1f, 0.35f), 3.2f, 2);
            _glow.SetActive(open);

            if (isOpen)
            {
                _sr.color = Color.white;
                transform.localScale = Vector3.one * 1.25f;
                AudioManager.Instance?.PlaySound("portal");
            }
            else
            {
                _sr.color = new Color(0.3f, 0.3f, 0.4f, 0.5f);
                transform.localScale = Vector3.one * 0.8f;
            }
        }

        private void OnTriggerEnter2D(Collider2D collision)
        {
            if (isOpen && collision.CompareTag("Player"))
            {
                isOpen = false;
                DungeonManager.Instance?.GoToNextFloor();
            }
        }
    }
}
