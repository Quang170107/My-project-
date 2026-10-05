using System.Collections.Generic;
using UnityEngine;

namespace SimpleRPG
{
    /// <summary>
    /// Tạo sprite pixel-art 12x12 cho từng ô trang bị bằng code (không cần file ảnh).
    /// Ảnh là xám + viền đen, màu độ hiếm được nhân lên bằng SpriteRenderer.color.
    /// '.' = trong suốt, 'o' = viền, '#' = sáng, '+' = trung bình.
    /// </summary>
    public static class EquipmentIcons
    {
        private static readonly Dictionary<EquipSlot, Sprite> _cache = new Dictionary<EquipSlot, Sprite>();

        private static readonly string[] SwordArt =
        {
            ".....oo.....",
            "....o##o....",
            "....o##o....",
            "....o##o....",
            "....o##o....",
            "....o##o....",
            "..oooooooo..",
            "..o++++++o..",
            "..oooooooo..",
            ".....o++o...",
            ".....o++o...",
            "....oooooo..",
        };

        private static readonly string[] ArmorArt =
        {
            "..oo....oo..",
            ".o##oooo##o.",
            ".o########o.",
            ".o########o.",
            "..o##++##o..",
            "..o##++##o..",
            "..o##++##o..",
            "..o######o..",
            "..o######o..",
            "..o##oo##o..",
            "..o#o..o#o..",
            "..ooo..ooo..",
        };

        private static readonly string[] BootsArt =
        {
            "............",
            "..oooo......",
            "..o##o......",
            "..o##o......",
            "..o##o......",
            "..o##o......",
            "..o##oooo...",
            "..o######o..",
            "..o#######o.",
            "..oooooooooo",
            "............",
            "............",
        };

        private static readonly string[] RingArt =
        {
            "............",
            "....oooo....",
            "...o####o...",
            "....oooo....",
            "..oo....oo..",
            ".o#o....o#o.",
            ".o#o....o#o.",
            ".o#o....o#o.",
            "..o##..##o..",
            "...o####o...",
            "....oooo....",
            "............",
        };

        public static Sprite Get(EquipSlot slot)
        {
            if (_cache.TryGetValue(slot, out var cached) && cached != null) return cached;

            string[] art;
            switch (slot)
            {
                case EquipSlot.Weapon: art = SwordArt; break;
                case EquipSlot.Armor: art = ArmorArt; break;
                case EquipSlot.Boots: art = BootsArt; break;
                default: art = RingArt; break;
            }

            var sprite = Build(art);
            _cache[slot] = sprite;
            return sprite;
        }

        private static Sprite Build(string[] art)
        {
            int h = art.Length;
            int w = art[0].Length;

            var tex = new Texture2D(w, h, TextureFormat.RGBA32, false);
            tex.filterMode = FilterMode.Point;
            tex.wrapMode = TextureWrapMode.Clamp;

            var clear = new Color(0, 0, 0, 0);
            var outline = new Color(0.12f, 0.12f, 0.12f, 1f);
            var light = Color.white;
            var mid = new Color(0.65f, 0.65f, 0.65f, 1f);

            for (int row = 0; row < h; row++)
            {
                for (int x = 0; x < w; x++)
                {
                    Color c;
                    switch (art[row][x])
                    {
                        case 'o': c = outline; break;
                        case '#': c = light; break;
                        case '+': c = mid; break;
                        default: c = clear; break;
                    }
                    // hàng 0 của chuỗi là hàng trên cùng của ảnh
                    tex.SetPixel(x, h - 1 - row, c);
                }
            }
            tex.Apply();

            // pixelsPerUnit = 12 -> sprite rộng đúng 1 đơn vị
            return Sprite.Create(tex, new Rect(0, 0, w, h), new Vector2(0.5f, 0.5f), w);
        }
    }
}
