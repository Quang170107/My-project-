using UnityEngine;

namespace SimpleRPG
{
    /// <summary>
    /// Hiệu ứng hình ảnh dựng hoàn toàn bằng code (không cần asset): vầng sáng mềm, bóng đổ,
    /// hạt bay theo chủ đề màn chơi, vết bẩn trên sàn và tia lửa khi đánh trúng.
    /// </summary>
    public static class VisualFX
    {
        private static Sprite _softCircle;
        private static Sprite _vignette;
        private static Material _particleMat;

        /// <summary>Hình tròn trắng mờ dần ra rìa, kích thước 1x1 đơn vị.</summary>
        public static Sprite SoftCircle
        {
            get
            {
                if (_softCircle == null)
                {
                    _softCircle = BuildRadial(64, nd => Mathf.Pow(Mathf.Clamp01(1f - nd), 1.6f));
                }
                return _softCircle;
            }
        }

        /// <summary>Gradient tối dần về rìa khung hình, kích thước 1x1 đơn vị.</summary>
        public static Sprite Vignette
        {
            get
            {
                if (_vignette == null)
                {
                    _vignette = BuildRadial(128, nd => 0.85f * Mathf.SmoothStep(0f, 1f, Mathf.Clamp01((nd - 0.45f) / 0.75f)));
                }
                return _vignette;
            }
        }

        private static Material ParticleMaterial
        {
            get
            {
                if (_particleMat == null)
                {
                    var shader = Shader.Find("Sprites/Default");
                    if (shader == null) return null;
                    _particleMat = new Material(shader);
                    _particleMat.mainTexture = SoftCircle.texture;
                }
                return _particleMat;
            }
        }

        private static Sprite BuildRadial(int size, System.Func<float, float> alphaOfNormalizedDistance)
        {
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            tex.wrapMode = TextureWrapMode.Clamp;
            tex.filterMode = FilterMode.Bilinear;

            var px = new Color32[size * size];
            float half = (size - 1) / 2f;
            var center = new Vector2(half, half);

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float nd = Vector2.Distance(new Vector2(x, y), center) / half;
                    byte a = (byte)Mathf.RoundToInt(Mathf.Clamp01(alphaOfNormalizedDistance(nd)) * 255f);
                    px[y * size + x] = new Color32(255, 255, 255, a);
                }
            }

            tex.SetPixels32(px);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), size);
        }

        // ───────────────────────── Sprite phụ ─────────────────────────

        /// <summary>Vầng sáng mềm (tròn, mờ dần) gắn vào một đối tượng.</summary>
        public static GameObject AddGlow(Transform parent, Color color, float scale, int sortingOrder)
        {
            var go = new GameObject("Glow");
            go.transform.SetParent(parent, false);
            go.transform.localPosition = Vector3.zero;
            go.transform.localScale = new Vector3(scale, scale, 1f);

            var sr = go.AddComponent<SpriteRenderer>();
            sr.sprite = SoftCircle;
            sr.color = color;
            sr.sortingOrder = sortingOrder;
            return go;
        }

        /// <summary>Bóng đổ hình elip dưới chân.</summary>
        public static GameObject AddShadow(Transform parent, float width, float yOffset, int sortingOrder)
        {
            var go = new GameObject("Shadow");
            go.transform.SetParent(parent, false);
            go.transform.localPosition = new Vector3(0f, yOffset, 0f);
            go.transform.localScale = new Vector3(width, width * 0.33f, 1f);

            var sr = go.AddComponent<SpriteRenderer>();
            sr.sprite = SoftCircle;
            sr.color = new Color(0f, 0f, 0f, 0.5f);
            sr.sortingOrder = sortingOrder;
            return go;
        }

        /// <summary>Vết bẩn/rêu/băng mờ rải trên sàn để bớt trống trải.</summary>
        public static void ScatterDecals(Transform parent, int themeIndex, float width, float height, int count)
        {
            Color c;
            switch (themeIndex)
            {
                case 1: c = new Color(0.10f, 0.35f, 0.12f); break; // rêu
                case 2: c = new Color(0.05f, 0.0f, 0.0f); break;   // vết cháy
                case 3: c = new Color(0.80f, 0.95f, 1.0f); break;  // băng
                default: c = new Color(0f, 0f, 0f); break;         // bụi bẩn
            }

            for (int i = 0; i < count; i++)
            {
                var go = new GameObject("Decal");
                go.transform.SetParent(parent, false);
                go.transform.localPosition = new Vector3(
                    UnityEngine.Random.Range(-width / 2f + 1.5f, width / 2f - 1.5f),
                    UnityEngine.Random.Range(-height / 2f + 1.5f, height / 2f - 1.5f), 0f);
                go.transform.localRotation = Quaternion.Euler(0f, 0f, UnityEngine.Random.Range(0f, 360f));
                go.transform.localScale = new Vector3(UnityEngine.Random.Range(1.4f, 3.4f), UnityEngine.Random.Range(0.9f, 2.2f), 1f);

                var sr = go.AddComponent<SpriteRenderer>();
                sr.sprite = SoftCircle;
                sr.color = new Color(c.r, c.g, c.b, UnityEngine.Random.Range(0.08f, 0.18f));
                sr.sortingOrder = 1;
            }
        }

        // ───────────────────────── Hạt (particles) ─────────────────────────

        private static ParticleSystem CreateSystem(string name, Transform parent, Vector3 position, int sortingOrder)
        {
            var go = new GameObject(name);
            if (parent != null) go.transform.SetParent(parent, false);
            go.transform.position = position;

            var ps = go.AddComponent<ParticleSystem>();
            // Dừng ngay để cấu hình (không thể đổi duration khi đang chạy)
            ps.Stop(true, ParticleSystemStopBehavior.StopEmittingAndClear);

            var renderer = go.GetComponent<ParticleSystemRenderer>();
            var mat = ParticleMaterial;
            if (mat != null) renderer.sharedMaterial = mat;
            renderer.sortingOrder = sortingOrder;
            return ps;
        }

        private static void FadeAndShrink(ParticleSystem ps)
        {
            var col = ps.colorOverLifetime;
            col.enabled = true;
            var g = new Gradient();
            g.SetKeys(
                new[] { new GradientColorKey(Color.white, 0f), new GradientColorKey(Color.white, 1f) },
                new[] { new GradientAlphaKey(1f, 0f), new GradientAlphaKey(0f, 1f) });
            col.color = g;

            var sz = ps.sizeOverLifetime;
            sz.enabled = true;
            sz.size = new ParticleSystem.MinMaxCurve(1f, AnimationCurve.Linear(0f, 1f, 1f, 0f));
        }

        /// <summary>Chùm tia lửa/máu bắn ra một lần rồi tự hủy.</summary>
        public static void Burst(Vector3 position, Color color, int count, float speed, float size = 0.18f)
        {
            var ps = CreateSystem("FX_Burst", null, position, 18);

            var main = ps.main;
            main.loop = false;
            main.playOnAwake = false;
            main.duration = 0.5f;
            main.startLifetime = new ParticleSystem.MinMaxCurve(0.25f, 0.55f);
            main.startSpeed = new ParticleSystem.MinMaxCurve(speed * 0.4f, speed);
            main.startSize = new ParticleSystem.MinMaxCurve(size * 0.5f, size);
            main.startColor = color;
            main.gravityModifier = 0f;
            main.simulationSpace = ParticleSystemSimulationSpace.World;
            main.stopAction = ParticleSystemStopAction.Destroy;

            var em = ps.emission;
            em.rateOverTime = 0f;
            em.SetBursts(new[] { new ParticleSystem.Burst(0f, (short)count) });

            var shape = ps.shape;
            shape.shapeType = ParticleSystemShapeType.Circle;
            shape.radius = 0.12f;

            FadeAndShrink(ps);
            ps.Play();
        }

        /// <summary>Hạt trôi trong phòng theo chủ đề: bụi, đom đóm, tàn lửa, tuyết.</summary>
        public static void CreateAmbient(Transform parent, int themeIndex, float width, float height)
        {
            Color color;
            float rate, lifetime, sizeMin, sizeMax, vxMin, vxMax, vyMin, vyMax;

            switch (themeIndex)
            {
                case 1: // đom đóm / bào tử
                    color = new Color(0.8f, 1f, 0.5f, 0.6f);
                    rate = 6f; lifetime = 6f; sizeMin = 0.10f; sizeMax = 0.18f;
                    vxMin = -0.25f; vxMax = 0.25f; vyMin = 0.05f; vyMax = 0.35f;
                    break;
                case 2: // tàn lửa bay lên
                    color = new Color(1f, 0.5f, 0.15f, 0.9f);
                    rate = 14f; lifetime = 3f; sizeMin = 0.07f; sizeMax = 0.14f;
                    vxMin = -0.3f; vxMax = 0.3f; vyMin = 0.5f; vyMax = 1.6f;
                    break;
                case 3: // tuyết rơi
                    color = new Color(0.9f, 0.95f, 1f, 0.8f);
                    rate = 14f; lifetime = 6f; sizeMin = 0.07f; sizeMax = 0.14f;
                    vxMin = -0.25f; vxMax = 0.25f; vyMin = -0.5f; vyMax = -0.1f;
                    break;
                default: // bụi lơ lửng
                    color = new Color(0.8f, 0.85f, 1f, 0.25f);
                    rate = 4f; lifetime = 7f; sizeMin = 0.06f; sizeMax = 0.12f;
                    vxMin = -0.3f; vxMax = 0.3f; vyMin = -0.3f; vyMax = 0.3f;
                    break;
            }

            var ps = CreateSystem("FX_Ambient", parent, Vector3.zero, 18);

            var main = ps.main;
            main.loop = true;
            main.prewarm = true;
            main.playOnAwake = false;
            main.startLifetime = lifetime;
            main.startSpeed = 0f;
            main.startSize = new ParticleSystem.MinMaxCurve(sizeMin, sizeMax);
            main.startColor = color;
            main.gravityModifier = 0f;
            main.maxParticles = 300;
            main.simulationSpace = ParticleSystemSimulationSpace.Local;

            var em = ps.emission;
            em.rateOverTime = rate;

            var shape = ps.shape;
            shape.shapeType = ParticleSystemShapeType.Box;
            shape.scale = new Vector3(width, height, 0.1f);

            var vel = ps.velocityOverLifetime;
            vel.enabled = true;
            vel.space = ParticleSystemSimulationSpace.Local;
            vel.x = new ParticleSystem.MinMaxCurve(vxMin, vxMax);
            vel.y = new ParticleSystem.MinMaxCurve(vyMin, vyMax);
            vel.z = new ParticleSystem.MinMaxCurve(0f, 0f);

            // Hiện dần rồi mờ dần
            var col = ps.colorOverLifetime;
            col.enabled = true;
            var g = new Gradient();
            g.SetKeys(
                new[] { new GradientColorKey(Color.white, 0f), new GradientColorKey(Color.white, 1f) },
                new[] { new GradientAlphaKey(0f, 0f), new GradientAlphaKey(1f, 0.2f), new GradientAlphaKey(1f, 0.8f), new GradientAlphaKey(0f, 1f) });
            col.color = g;

            ps.Play();
        }
    }
}
