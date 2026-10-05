using UnityEngine;
using UnityEngine.UI;

namespace SimpleRPG
{
    public class DamageNumberManager : MonoBehaviour
    {
        public static DamageNumberManager Instance { get; private set; }

        private Font _defaultFont;

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

            _defaultFont = Resources.GetBuiltinResource<Font>("LegacyRuntime.ttf");
            if (_defaultFont == null)
            {
                _defaultFont = Font.CreateDynamicFontFromOSFont("Arial", 24);
            }
        }

        public void SpawnNumber(Vector3 worldPos, int amount, Color color, string prefix = "")
        {
            var go = new GameObject("DamagePopup");
            go.transform.position = worldPos + new Vector3(Random.Range(-0.25f, 0.25f), Random.Range(-0.1f, 0.1f), 0f);

            var canvas = go.AddComponent<Canvas>();
            canvas.renderMode = RenderMode.WorldSpace;
            canvas.sortingOrder = 100;

            var rect = go.GetComponent<RectTransform>();
            rect.sizeDelta = new Vector2(2f, 1f);
            rect.localScale = Vector3.one * 0.02f;

            var textGo = new GameObject("Text");
            textGo.transform.SetParent(go.transform, false);

            var text = textGo.AddComponent<Text>();
            text.text = prefix + amount;
            text.font = _defaultFont;
            text.fontSize = 44;
            text.fontStyle = FontStyle.Bold;
            text.alignment = TextAnchor.MiddleCenter;
            text.color = color;
            text.raycastTarget = false;

            var outline = textGo.AddComponent<Outline>();
            outline.effectColor = new Color(0f, 0f, 0f, 0.85f);
            outline.effectDistance = new Vector2(2f, -2f);

            var textRect = textGo.GetComponent<RectTransform>();
            textRect.sizeDelta = new Vector2(200f, 60f);

            var anim = go.AddComponent<FloatingTextAnimation>();
            anim.Initialize(text);
        }
    }

    public class FloatingTextAnimation : MonoBehaviour
    {
        private Text _text;
        private float _elapsed = 0f;
        private const float Duration = 0.65f;
        private Vector3 _startPos;
        private Vector3 _targetPos;
        private Color _startColor;

        public void Initialize(Text text)
        {
            _text = text;
            _startPos = transform.position;
            _targetPos = _startPos + Vector3.up * 0.9f;
            _startColor = text.color;
        }

        private void Update()
        {
            _elapsed += Time.deltaTime;
            float t = _elapsed / Duration;

            if (t >= 1f)
            {
                Destroy(gameObject);
                return;
            }

            // Ease out float
            transform.position = Vector3.Lerp(_startPos, _targetPos, Mathf.Sin(t * Mathf.PI * 0.5f));

            // Punch scale at start then settle
            float scale = (t < 0.2f) ? Mathf.Lerp(0.015f, 0.025f, t / 0.2f) : Mathf.Lerp(0.025f, 0.018f, (t - 0.2f) / 0.8f);
            transform.localScale = Vector3.one * scale;

            // Fade out
            if (t > 0.5f)
            {
                float alpha = 1f - ((t - 0.5f) / 0.5f);
                _text.color = new Color(_startColor.r, _startColor.g, _startColor.b, alpha);
            }
        }
    }
}
