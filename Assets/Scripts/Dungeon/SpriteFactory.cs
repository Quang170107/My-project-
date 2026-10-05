using System.Collections.Generic;
using UnityEngine;

namespace SimpleRPG
{
    public static class SpriteFactory
    {
        private static readonly Dictionary<string, Sprite> _cache = new Dictionary<string, Sprite>();

        public static Sprite GetSprite(string key)
        {
            if (_cache.TryGetValue(key, out var sprite) && sprite != null)
                return sprite;

            sprite = GenerateSprite(key);
            if (sprite != null)
                _cache[key] = sprite;

            return sprite;
        }

        private static Sprite GenerateSprite(string key)
        {
            switch (key)
            {
                case "player":
                    return CreatePlayerSprite();
                case "sword":
                    return CreateSwordSprite();
                case "chaser":
                    return CreateChaserSprite();
                case "ranger":
                    return CreateRangerSprite();
                case "brute":
                    return CreateBruteSprite();
                case "boss":
                    return CreateBossSprite();
                case "projectile_player":
                    return CreateCircle(32, new Color(0.2f, 0.9f, 1f), Color.white, 2);
                case "projectile_enemy":
                    return CreateCircle(32, new Color(1f, 0.25f, 0.25f), new Color(1f, 0.8f, 0.4f), 2);
                case "gem_xp":
                    return CreateDiamond(32, new Color(0.15f, 0.85f, 1f), Color.white, 2);
                case "coin":
                    return CreateCoinSprite();
                case "potion_hp":
                    return CreatePotionSprite();
                case "chest":
                    return CreateChestSprite();
                case "portal":
                    return CreatePortalSprite();
                case "floor_tile":
                    return CreateFloorTileSprite();
                case "wall_tile":
                    return CreateWallTileSprite();
                case "pillar":
                    return CreatePillarSprite();
                case "slash":
                    return CreateSlashSprite();
                case "ui_bar":
                    return CreateRoundedRect(64, 16, 4, Color.white, Color.clear, 0);
                default:
                    return CreateCircle(32, Color.white, Color.gray, 2);
            }
        }

        public static Sprite CreatePlayerSprite()
        {
            // Crisp hero crest: Rounded diamond with cyan/blue gradient and silver rim
            int size = 64;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            tex.filterMode = FilterMode.Bilinear;
            var colors = new Color32[size * size];

            Vector2 center = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float radius = size * 0.44f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float dx = Mathf.Abs(x - center.x);
                    float dy = Mathf.Abs(y - center.y);
                    // Diamond metric + slight curve
                    float d = (dx + dy) / 1.35f;
                    float circleD = Vector2.Distance(new Vector2(x, y), center);
                    float blendD = Mathf.Lerp(d, circleD, 0.35f);

                    if (blendD <= radius)
                    {
                        if (blendD >= radius - 3.5f)
                        {
                            // Shiny border
                            colors[y * size + x] = new Color32(240, 245, 255, 255);
                        }
                        else
                        {
                            // Heroic cyan-blue fill
                            float t = (float)y / size;
                            byte r = (byte)Mathf.Lerp(30, 70, t);
                            byte g = (byte)Mathf.Lerp(140, 210, t);
                            byte b = (byte)Mathf.Lerp(230, 255, t);
                            colors[y * size + x] = new Color32(r, g, b, 255);
                        }
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }

            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateSwordSprite()
        {
            int w = 24, h = 64;
            var tex = new Texture2D(w, h, TextureFormat.RGBA32, false);
            tex.filterMode = FilterMode.Point;
            var colors = new Color32[w * h];

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    Color32 col = Color.clear;
                    int mid = w / 2;
                    int dx = Mathf.Abs(x - mid);

                    // Blade
                    if (y >= 16 && y < h - 4)
                    {
                        int halfBladeWidth = y > h - 10 ? (h - y) / 2 : 4;
                        if (dx <= halfBladeWidth)
                        {
                            col = (dx == 0) ? new Color32(255, 255, 255, 255) : new Color32(200, 225, 255, 255);
                        }
                    }
                    // Crossguard
                    else if (y >= 12 && y < 16)
                    {
                        if (dx <= 9)
                            col = new Color32(255, 205, 50, 255);
                    }
                    // Handle & Pommel
                    else if (y >= 2 && y < 12)
                    {
                        if (dx <= 2)
                            col = new Color32(110, 65, 40, 255);
                        if (y <= 4 && dx <= 3)
                            col = new Color32(255, 205, 50, 255);
                    }

                    colors[y * w + x] = col;
                }
            }

            tex.SetPixels32(colors);
            tex.Apply();
            // Pivot at handle
            return Sprite.Create(tex, new Rect(0, 0, w, h), new Vector2(0.5f, 0.2f), 32);
        }

        public static Sprite CreateChaserSprite()
        {
            // Bouncy Slime / Hexagon - Lime Green
            return CreateHexagon(48, new Color(0.25f, 0.95f, 0.35f), new Color(0.9f, 1f, 0.5f), 3);
        }

        public static Sprite CreateRangerSprite()
        {
            // Spiky Diamond / Stalker - Crimson Red
            return CreateDiamond(48, new Color(0.95f, 0.25f, 0.35f), new Color(1f, 0.7f, 0.7f), 3);
        }

        public static Sprite CreateBruteSprite()
        {
            // Heavy Armored Octagon - Deep Royal Purple
            return CreateOctagon(64, new Color(0.55f, 0.2f, 0.85f), new Color(0.85f, 0.6f, 1f), 4);
        }

        public static Sprite CreateBossSprite()
        {
            // Giant Dread Lord - Dark Crimson & Gold
            return CreateOctagon(88, new Color(0.85f, 0.1f, 0.2f), new Color(1f, 0.85f, 0.3f), 5);
        }

        public static Sprite CreateCoinSprite()
        {
            int size = 32;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.42f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float d = Vector2.Distance(new Vector2(x, y), c);
                    if (d <= r)
                    {
                        if (d >= r - 2.5f || d <= r * 0.45f)
                            colors[y * size + x] = new Color32(255, 235, 120, 255);
                        else
                            colors[y * size + x] = new Color32(245, 185, 20, 255);
                    }
                    else
                        colors[y * size + x] = Color.clear;
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreatePotionSprite()
        {
            int size = 36;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, 14f);

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float dx = Mathf.Abs(x - c.x);
                    Color32 col = Color.clear;

                    // Round flask bottom
                    float d = Vector2.Distance(new Vector2(x, y), c);
                    if (d <= 12f)
                    {
                        col = (d >= 10f) ? new Color32(255, 255, 255, 255) : new Color32(235, 45, 75, 255);
                    }
                    // Flask neck
                    else if (y >= 20 && y <= 28 && dx <= 4)
                    {
                        col = (dx >= 3) ? new Color32(255, 255, 255, 255) : new Color32(210, 230, 255, 200);
                    }
                    // Cork
                    else if (y > 28 && y <= 33 && dx <= 5)
                    {
                        col = new Color32(180, 110, 60, 255);
                    }

                    colors[y * size + x] = col;
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateChestSprite()
        {
            int size = 48;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    Color32 col = Color.clear;
                    if (x >= 6 && x < size - 6 && y >= 6 && y < size - 10)
                    {
                        bool border = (x <= 8 || x >= size - 9 || y <= 8 || y >= size - 12);
                        bool lockPlate = (Mathf.Abs(x - size / 2) <= 3 && Mathf.Abs(y - size / 2) <= 3);

                        if (border || lockPlate)
                            col = new Color32(255, 210, 50, 255); // Gold trim & lock
                        else if (y == 24)
                            col = new Color32(40, 25, 15, 255); // Seam
                        else
                            col = new Color32(140, 75, 35, 255); // Wood body
                    }
                    colors[y * size + x] = col;
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreatePortalSprite()
        {
            int size = 64;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.45f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float d = Vector2.Distance(new Vector2(x, y), c);
                    if (d <= r)
                    {
                        float t = d / r;
                        if (t > 0.8f)
                            colors[y * size + x] = new Color32(100, 240, 255, 255); // Outer glow
                        else if (t > 0.5f)
                            colors[y * size + x] = new Color32(50, 150, 255, 220); // Mid ring
                        else
                            colors[y * size + x] = new Color32(20, 40, 90, 255);  // Void center
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateFloorTileSprite()
        {
            int size = 64;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    bool border = (x == 0 || x == size - 1 || y == 0 || y == size - 1);
                    if (border)
                    {
                        colors[y * size + x] = new Color32(32, 36, 48, 255);
                    }
                    else
                    {
                        // Subtle slate stone texture
                        byte v = (byte)(46 + ((x ^ y) % 5));
                        colors[y * size + x] = new Color32(v, (byte)(v + 4), (byte)(v + 10), 255);
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateWallTileSprite()
        {
            int size = 64;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    bool border = (x == 0 || x == size - 1 || y == 0 || y == size - 1 || y == size / 2);
                    if (border)
                        colors[y * size + x] = new Color32(18, 20, 28, 255);
                    else
                        colors[y * size + x] = new Color32(75, 82, 100, 255);
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreatePillarSprite()
        {
            int size = 64;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.44f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float d = Vector2.Distance(new Vector2(x, y), c);
                    if (d <= r)
                    {
                        if (d >= r - 3)
                            colors[y * size + x] = new Color32(30, 35, 45, 255);
                        else if (d <= r * 0.4f)
                            colors[y * size + x] = new Color32(100, 220, 255, 255); // Glowing rune core
                        else
                            colors[y * size + x] = new Color32(95, 105, 125, 255);
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateSlashSprite()
        {
            int size = 64;
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2(8f, (size - 1) / 2f);

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float d = Vector2.Distance(new Vector2(x, y), c);
                    float angle = Mathf.Atan2(y - c.y, x - c.x) * Mathf.Rad2Deg;

                    if (d >= 24f && d <= 36f && angle >= -45f && angle <= 45f)
                    {
                        float alpha = 1f - (Mathf.Abs(angle) / 45f);
                        colors[y * size + x] = new Color(1f, 1f, 1f, alpha * 0.85f);
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.2f, 0.5f), 32);
        }

        public static Sprite CreateCircle(int size, Color fill, Color outline, int outlineWidth)
        {
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.46f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float d = Vector2.Distance(new Vector2(x, y), c);
                    if (d <= r)
                    {
                        if (outlineWidth > 0 && d >= r - outlineWidth)
                            colors[y * size + x] = outline;
                        else
                            colors[y * size + x] = fill;
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateDiamond(int size, Color fill, Color outline, int outlineWidth)
        {
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.46f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float d = (Mathf.Abs(x - c.x) + Mathf.Abs(y - c.y));
                    if (d <= r)
                    {
                        if (outlineWidth > 0 && d >= r - outlineWidth)
                            colors[y * size + x] = outline;
                        else
                            colors[y * size + x] = fill;
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateHexagon(int size, Color fill, Color outline, int outlineWidth)
        {
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.46f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float dx = Mathf.Abs(x - c.x) / r;
                    float dy = Mathf.Abs(y - c.y) / r;
                    // Hexagon inequality
                    float d = Mathf.Max(dx * 0.866025f + dy * 0.5f, dy);
                    if (d <= 0.866025f)
                    {
                        if (outlineWidth > 0 && d >= 0.866025f - (outlineWidth / (float)size))
                            colors[y * size + x] = outline;
                        else
                            colors[y * size + x] = fill;
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateOctagon(int size, Color fill, Color outline, int outlineWidth)
        {
            var tex = new Texture2D(size, size, TextureFormat.RGBA32, false);
            var colors = new Color32[size * size];
            Vector2 c = new Vector2((size - 1) / 2f, (size - 1) / 2f);
            float r = size * 0.46f;

            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float dx = Mathf.Abs(x - c.x);
                    float dy = Mathf.Abs(y - c.y);
                    float d = Mathf.Max(Mathf.Max(dx, dy), (dx + dy) * 0.7071f);
                    if (d <= r)
                    {
                        if (outlineWidth > 0 && d >= r - outlineWidth)
                            colors[y * size + x] = outline;
                        else
                            colors[y * size + x] = fill;
                    }
                    else
                    {
                        colors[y * size + x] = Color.clear;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, size, size), new Vector2(0.5f, 0.5f), 32);
        }

        public static Sprite CreateRoundedRect(int width, int height, int radius, Color fill, Color outline, int outlineWidth)
        {
            var tex = new Texture2D(width, height, TextureFormat.RGBA32, false);
            var colors = new Color32[width * height];

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int cornerX = (x < radius) ? radius - x : (x >= width - radius) ? x - (width - radius - 1) : 0;
                    int cornerY = (y < radius) ? radius - y : (y >= height - radius) ? y - (height - radius - 1) : 0;

                    if (cornerX > 0 && cornerY > 0 && (cornerX * cornerX + cornerY * cornerY) > radius * radius)
                    {
                        colors[y * width + x] = Color.clear;
                    }
                    else
                    {
                        if (outlineWidth > 0 && (x < outlineWidth || x >= width - outlineWidth || y < outlineWidth || y >= height - outlineWidth))
                            colors[y * width + x] = outline;
                        else
                            colors[y * width + x] = fill;
                    }
                }
            }
            tex.SetPixels32(colors);
            tex.Apply();
            return Sprite.Create(tex, new Rect(0, 0, width, height), new Vector2(0.5f, 0.5f), 32);
        }
    }
}
