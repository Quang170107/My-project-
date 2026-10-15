using UnityEngine;

namespace SimpleRPG
{
    /// <summary>
    /// Lớp tối ở rìa màn hình gắn vào camera: tạo chiều sâu, đổi sắc theo chủ đề màn chơi
    /// và nháy đỏ khi người chơi bị thương.
    /// </summary>
    public class ScreenVignette : MonoBehaviour
    {
        private static ScreenVignette _instance;

        private SpriteRenderer _sr;
        private Camera _cam;
        private Color _base = new Color(0.04f, 0.045f, 0.06f, 0.65f);
        private Color _pulseColor = Color.red;
        private float _pulse;

        public static void Attach(Camera cam)
        {
            if (cam == null || _instance != null) return;

            var go = new GameObject("ScreenVignette");
            go.transform.SetParent(cam.transform, false);
            go.transform.localPosition = new Vector3(0f, 0f, 5f);

            var vignette = go.AddComponent<ScreenVignette>();
            vignette._cam = cam;
            vignette._sr = go.AddComponent<SpriteRenderer>();
            vignette._sr.sprite = VisualFX.Vignette;
            vignette._sr.sortingOrder = 900;
            vignette._sr.color = vignette._base;
            _instance = vignette;
        }

        /// <summary>Đổi sắc rìa tối theo màu nền của chủ đề.</summary>
        public static void SetTint(Color background)
        {
            if (_instance == null) return;
            _instance._base = new Color(background.r * 0.5f, background.g * 0.5f, background.b * 0.5f, 0.65f);
        }

        /// <summary>Nháy màu ở rìa màn hình (ví dụ đỏ khi bị thương), mờ dần sau ~0.4s.</summary>
        public static void Pulse(Color color)
        {
            if (_instance == null) return;
            _instance._pulseColor = color;
            _instance._pulse = 1f;
        }

        private void LateUpdate()
        {
            if (_cam == null || _sr == null) return;

            float h = _cam.orthographicSize * 2f;
            float w = h * _cam.aspect;
            transform.localScale = new Vector3(w * 1.05f, h * 1.05f, 1f);

            _pulse = Mathf.MoveTowards(_pulse, 0f, Time.unscaledDeltaTime * 2.5f);
            Color target = new Color(_pulseColor.r, _pulseColor.g, _pulseColor.b, 1f);
            _sr.color = Color.Lerp(_base, target, _pulse);
        }
    }
}
