using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Runtime.ConstrainedExecution;

#if ENABLE_INPUT_SYSTEM
using UnityEngine.InputSystem.UI;
#endif

namespace SimpleRPG
{
    public class GameUI : MonoBehaviour
    {
        public static GameUI Instance { get; private set; }

        private Font _font;
        private Canvas _canvas;

        // Top HUD
        private Image _hpFill;
        private Text _hpText;
        private Image _xpFill;
        private Text _xpText;
        private Text _goldText;
        private Text _floorText;
        private Text _enemyText;

        // Skill cooldowns
        private Image _dashCdFill;
        private Image _specialCdFill;

        // Banner
        private GameObject _bannerRoot;
        private Text _bannerText;
        private float _bannerTimer = 0f;

        // Upgrade Modal
        private GameObject _upgradeModalRoot;
        private readonly List<GameObject> _upgradeCards = new List<GameObject>();

        // Game Over Modal
        private GameObject _gameOverRoot;
        private Text _gameOverStatsText;

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

            _font = Resources.GetBuiltinResource<Font>("LegacyRuntime.ttf");
            if (_font == null)
            {
                _font = Font.CreateDynamicFontFromOSFont("Arial", 20);
            }

            BuildUI();
        }

        private void BuildUI()
        {
            // Canvas setup
            _canvas = gameObject.AddComponent<Canvas>();
            _canvas.renderMode = RenderMode.ScreenSpaceOverlay;
            _canvas.sortingOrder = 50;

            var scaler = gameObject.AddComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.matchWidthOrHeight = 0.5f;

            gameObject.AddComponent<GraphicRaycaster>();

            // Ensure EventSystem
            EnsureEventSystem();

            // Build HUD sections
            BuildTopHUD();
            BuildSkillBarHUD();
            BuildBannerUI();
            BuildUpgradeModal();
            BuildGameOverModal();
        }

        private void EnsureEventSystem()
        {
#if UNITY_6000_0_OR_NEWER
            var es = FindAnyObjectByType<UnityEngine.EventSystems.EventSystem>();
#else
            var es = FindObjectOfType<UnityEngine.EventSystems.EventSystem>();
#endif
            if (es == null)
            {
                var esGo = new GameObject("EventSystem");
                es = esGo.AddComponent<UnityEngine.EventSystems.EventSystem>();
#if ENABLE_INPUT_SYSTEM
                var uiMod = esGo.AddComponent<InputSystemUIInputModule>();
                uiMod.AssignDefaultActions();
#else
                esGo.AddComponent<UnityEngine.EventSystems.StandaloneInputModule>();
#endif
            }
            else
            {
#if ENABLE_INPUT_SYSTEM
                var uiMod = es.GetComponent<InputSystemUIInputModule>();
                if (uiMod != null && uiMod.actionsAsset == null)
                {
                    uiMod.AssignDefaultActions();
                }
#endif
            }
        }

        private void BuildTopHUD()
        {
            var hudTopLeft = CreateUIObject("TopLeftHUD", transform, new Vector2(0, 1), new Vector2(0, 1), new Vector2(0, 1), new Vector2(20, -20), new Vector2(360, 140));
            Color lostColor = new Color(0.35f, 0.35f, 0.35f, 1f); // xám

            // HP Bar Background
            var hpBg = CreateImage("HP_BG", hudTopLeft.transform, lostColor, new Vector2(0, 1), new Vector2(0, 1), new Vector2(0, 1), new Vector2(0, 0), new Vector2(320, 28));
            var hpFillGo = CreateImage("HP_Fill", hpBg.transform, new Color(0.92f, 0.22f, 0.25f, 1f), new Vector2(0, 0), new Vector2(0, 1), new Vector2(0, 0.5f), new Vector2(2, 0), new Vector2(316, 24));
            _hpFill = hpFillGo.GetComponent<Image>();

            var hpTextGo = CreateText("HP_Text", hpBg.transform, "HP: 100 / 100", 18, Color.white, TextAnchor.MiddleCenter);
            _hpText = hpTextGo.GetComponent<Text>();
            hpTextGo.GetComponent<RectTransform>().sizeDelta = new Vector2(320, 28);

            // XP Bar Background
            var xpBg = CreateImage("XP_BG", hudTopLeft.transform, lostColor, new Vector2(0, 1), new Vector2(0, 1), new Vector2(0, 1), new Vector2(0, -36), new Vector2(320, 20));
            var xpFillGo = CreateImage("XP_Fill", xpBg.transform, new Color(0.15f, 0.85f, 1f, 1f), new Vector2(0, 0), new Vector2(0, 1), new Vector2(0, 0.5f), new Vector2(2, 0), new Vector2(316, 16));
            _xpFill = xpFillGo.GetComponent<Image>();
            SetBar(_xpFill, 0f, 1f);

            var xpTextGo = CreateText("XP_Text", xpBg.transform, "LVL 1 - XP 0 / 50", 14, Color.white, TextAnchor.MiddleCenter);
            _xpText = xpTextGo.GetComponent<Text>();
            xpTextGo.GetComponent<RectTransform>().sizeDelta = new Vector2(320, 20);

            // Gold Display
            var goldGo = CreateText("GoldText", hudTopLeft.transform, "Gold: 0", 22, new Color(1f, 0.85f, 0.2f), TextAnchor.MiddleLeft);
            var goldRect = goldGo.GetComponent<RectTransform>();
            goldRect.anchorMin = new Vector2(0, 1);
            goldRect.anchorMax = new Vector2(0, 1);
            goldRect.pivot = new Vector2(0, 1);
            goldRect.anchoredPosition = new Vector2(5, -68);
            goldRect.sizeDelta = new Vector2(200, 30);
            _goldText = goldGo.GetComponent<Text>();

            // Top-Center: Floor & Enemy count
            var topCenter = CreateUIObject("TopCenterHUD", transform, new Vector2(0.5f, 1), new Vector2(0.5f, 1), new Vector2(0.5f, 1), new Vector2(0, -20), new Vector2(500, 90));

            var floorGo = CreateText("FloorText", topCenter.transform, "FLOOR 1", 34, new Color(1f, 0.95f, 0.8f), TextAnchor.MiddleCenter);
            var floorRect = floorGo.GetComponent<RectTransform>();
            floorRect.anchoredPosition = new Vector2(0, 0);
            floorRect.sizeDelta = new Vector2(400, 45);
            _floorText = floorGo.GetComponent<Text>();
            var outline = floorGo.AddComponent<Outline>();
            outline.effectColor = Color.black;
            outline.effectDistance = new Vector2(2, -2);

            var enemyGo = CreateText("EnemyText", topCenter.transform, "Enemies Remaining: 0", 20, new Color(0.85f, 0.85f, 0.85f), TextAnchor.MiddleCenter);
            var enemyRect = enemyGo.GetComponent<RectTransform>();
            enemyRect.anchoredPosition = new Vector2(0, -42);
            enemyRect.sizeDelta = new Vector2(400, 30);
            _enemyText = enemyGo.GetComponent<Text>();
        }
        private void SetBar(Image fill, float current, float max)
        {
            if (fill == null) return;
            float ratio = max > 0f ? Mathf.Clamp01(current / max) : 0f;
            fill.rectTransform.localScale = new Vector3(ratio, 1f, 1f);
        }

        private void BuildSkillBarHUD()
        {
            // Bottom Center Skill HUD
            var skillBar = CreateUIObject("SkillBar", transform, new Vector2(0.5f, 0), new Vector2(0.5f, 0), new Vector2(0.5f, 0), new Vector2(0, 30), new Vector2(340, 80));

            // Melee indicator
            CreateSkillIcon(skillBar.transform, new Vector2(-100, 0), "LMB", "Attack", Color.white);

            // Dash indicator with cooldown overlay
            var dashIcon = CreateSkillIcon(skillBar.transform, new Vector2(0, 0), "Shift", "Dash", new Color(0.2f, 0.85f, 1f), out _dashCdFill);

            // Special Nova indicator with cooldown overlay
            var specialIcon = CreateSkillIcon(skillBar.transform, new Vector2(100, 0), "Q / E", "Nova", new Color(1f, 0.7f, 0.2f), out _specialCdFill);
        }

        private GameObject CreateSkillIcon(Transform parent, Vector2 pos, string keyName, string label, Color themeColor)
        {
            return CreateSkillIcon(parent, pos, keyName, label, themeColor, out _);
        }

        private GameObject CreateSkillIcon(Transform parent, Vector2 pos, string keyName, string label, Color themeColor, out Image cdOverlay)
        {
            var iconRoot = CreateImage("Skill_" + label, parent, new Color(0.12f, 0.14f, 0.2f, 0.85f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), pos, new Vector2(74, 74));

            // Border
            var border = CreateImage("Border", iconRoot.transform, themeColor, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), Vector2.zero, new Vector2(74, 74));
            var borderImg = border.GetComponent<Image>();
            borderImg.sprite = SpriteFactory.CreateRoundedRect(74, 74, 8, Color.clear, themeColor, 3);

            // Key text
            var keyGo = CreateText("Key", iconRoot.transform, keyName, 18, Color.white, TextAnchor.MiddleCenter);
            keyGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, 8);
            keyGo.GetComponent<RectTransform>().sizeDelta = new Vector2(70, 30);

            // Label
            var lblGo = CreateText("Label", iconRoot.transform, label, 12, new Color(0.8f, 0.85f, 0.9f), TextAnchor.MiddleCenter);
            lblGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -18);
            lblGo.GetComponent<RectTransform>().sizeDelta = new Vector2(70, 20);

            // Cooldown overlay
            var cdGo = CreateImage("CDOverlay", iconRoot.transform, new Color(0f, 0f, 0f, 0.7f), new Vector2(0, 0), new Vector2(1, 1), new Vector2(0.5f, 0.5f), Vector2.zero, Vector2.zero);
            var cdImg = cdGo.GetComponent<Image>();
            cdImg.type = Image.Type.Filled;
            cdImg.fillMethod = Image.FillMethod.Radial360;
            cdImg.fillAmount = 0f;
            cdOverlay = cdImg;

            return iconRoot;
        }

        private void BuildBannerUI()
        {
            _bannerRoot = CreateUIObject("Banner", transform, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0, 150), new Vector2(800, 90));
            var bg = CreateImage("BG", _bannerRoot.transform, new Color(0.08f, 0.1f, 0.15f, 0.85f), new Vector2(0, 0), new Vector2(1, 1), new Vector2(0.5f, 0.5f), Vector2.zero, Vector2.zero);

            var textGo = CreateText("Text", _bannerRoot.transform, "FLOOR CLEARED! ENTER THE PORTAL", 28, new Color(0.2f, 1f, 0.7f), TextAnchor.MiddleCenter);
            _bannerText = textGo.GetComponent<Text>();
            textGo.GetComponent<RectTransform>().sizeDelta = new Vector2(780, 80);
            var outline = textGo.AddComponent<Outline>();
            outline.effectColor = Color.black;

            _bannerRoot.SetActive(false);
        }

        private void BuildUpgradeModal()
        {
            _upgradeModalRoot = CreateUIObject("UpgradeModal", transform, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), Vector2.zero, new Vector2(1920, 1080));
            // Dim background
            CreateImage("ModalBG", _upgradeModalRoot.transform, new Color(0f, 0f, 0f, 0.75f), new Vector2(0, 0), new Vector2(1, 1), new Vector2(0.5f, 0.5f), Vector2.zero, Vector2.zero);

            // Title
            var titleGo = CreateText("Title", _upgradeModalRoot.transform, "LEVEL UP!", 46, new Color(1f, 0.85f, 0.2f), TextAnchor.MiddleCenter);
            titleGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, 220);
            titleGo.GetComponent<RectTransform>().sizeDelta = new Vector2(800, 60);

            var subGo = CreateText("Subtitle", _upgradeModalRoot.transform, "Choose an enhancement to grow stronger:", 22, Color.white, TextAnchor.MiddleCenter);
            subGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, 160);
            subGo.GetComponent<RectTransform>().sizeDelta = new Vector2(800, 40);

            // Card Container
            var cardContainer = CreateUIObject("CardContainer", _upgradeModalRoot.transform, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0, -30), new Vector2(900, 320));

            // Create 3 card slots
            float[] xOffsets = new float[] { -280f, 0f, 280f };
            for (int i = 0; i < 3; i++)
            {
                var card = CreateUpgradeCard(cardContainer.transform, new Vector2(xOffsets[i], 0));
                _upgradeCards.Add(card);
            }

            _upgradeModalRoot.SetActive(false);
        }

        private GameObject CreateUpgradeCard(Transform parent, Vector2 pos)
        {
            var cardGo = CreateImage("Card", parent, new Color(0.14f, 0.16f, 0.24f, 0.95f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), pos, new Vector2(250, 310));
            var border = CreateImage("Border", cardGo.transform, new Color(0.3f, 0.7f, 1f), new Vector2(0, 0), new Vector2(1, 1), new Vector2(0.5f, 0.5f), Vector2.zero, Vector2.zero);
            var borderImg = border.GetComponent<Image>();
            borderImg.sprite = SpriteFactory.CreateRoundedRect(250, 310, 12, Color.clear, new Color(0.3f, 0.8f, 1f), 3);

            // Icon placeholder
            var iconGo = CreateImage("Icon", cardGo.transform, Color.white, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0, 75), new Vector2(56, 56));

            // Name
            var nameGo = CreateText("Name", cardGo.transform, "Upgrade Name", 20, Color.white, TextAnchor.MiddleCenter);
            nameGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, 15);
            nameGo.GetComponent<RectTransform>().sizeDelta = new Vector2(230, 45);

            // Description
            var descGo = CreateText("Desc", cardGo.transform, "+Bonus Stats", 16, new Color(0.8f, 0.9f, 0.95f), TextAnchor.MiddleCenter);
            descGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -45);
            descGo.GetComponent<RectTransform>().sizeDelta = new Vector2(220, 60);

            // Button
            var btnGo = CreateImage("SelectButton", cardGo.transform, new Color(0.18f, 0.65f, 0.95f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0, -110), new Vector2(180, 44));
            var btn = btnGo.AddComponent<Button>();
            var btnColors = btn.colors;
            btnColors.highlightedColor = new Color(0.35f, 0.8f, 1f);
            btnColors.pressedColor = new Color(0.1f, 0.5f, 0.8f);
            btn.colors = btnColors;

            var btnText = CreateText("BtnText", btnGo.transform, "CHOOSE", 18, Color.white, TextAnchor.MiddleCenter);
            btnText.GetComponent<RectTransform>().sizeDelta = new Vector2(180, 44);

            return cardGo;
        }

        private void BuildGameOverModal()
        {
            _gameOverRoot = CreateUIObject("GameOverModal", transform, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), Vector2.zero, new Vector2(1920, 1080));
            CreateImage("BG", _gameOverRoot.transform, new Color(0.05f, 0.02f, 0.02f, 0.9f), new Vector2(0, 0), new Vector2(1, 1), new Vector2(0.5f, 0.5f), Vector2.zero, Vector2.zero);

            var titleGo = CreateText("Title", _gameOverRoot.transform, "YOU HAVE FALLEN", 52, new Color(1f, 0.25f, 0.25f), TextAnchor.MiddleCenter);
            titleGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, 160);
            titleGo.GetComponent<RectTransform>().sizeDelta = new Vector2(900, 70);

            var statsGo = CreateText("Stats", _gameOverRoot.transform, "Floor Reached: 1\nLevel: 1\nGold Collected: 0", 24, Color.white, TextAnchor.MiddleCenter);
            statsGo.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, 30);
            statsGo.GetComponent<RectTransform>().sizeDelta = new Vector2(600, 120);
            _gameOverStatsText = statsGo.GetComponent<Text>();

            // Restart Button
            var restartBtnGo = CreateImage("RestartBtn", _gameOverRoot.transform, new Color(0.85f, 0.25f, 0.3f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0, -90), new Vector2(240, 56));
            var btn = restartBtnGo.AddComponent<Button>();
            btn.onClick.AddListener(() => DungeonManager.Instance?.RestartGame());

            var btnText = CreateText("Text", restartBtnGo.transform, "PLAY AGAIN (R)", 22, Color.white, TextAnchor.MiddleCenter);
            btnText.GetComponent<RectTransform>().sizeDelta = new Vector2(240, 56);

            _gameOverRoot.SetActive(false);
        }

        private void Update()
        {
            // Banner timer
            if (_bannerTimer > 0f)
            {
                _bannerTimer -= Time.deltaTime;
                if (_bannerTimer <= 0f)
                {
                    _bannerRoot.SetActive(false);
                }
            }

            // Update cooldown rings
            if (PlayerController.Instance != null)
            {
                var player = PlayerController.Instance;
                if (_dashCdFill != null)
                {
                    float dashPct = player.dashCooldownTimer / player.stats.dashCooldown;
                    _dashCdFill.fillAmount = Mathf.Clamp01(dashPct);
                }
                if (_specialCdFill != null)
                {
                    float specPct = player.specialCooldownTimer / PlayerController.SpecialMaxCooldown;
                    _specialCdFill.fillAmount = Mathf.Clamp01(specPct);
                }
            }

            // Quick restart via R key or click
            if (_gameOverRoot != null && _gameOverRoot.activeSelf)
            {
                if (InputHelper.GetRestartDown() || InputHelper.GetAttackDown())
                {
                    DungeonManager.Instance?.RestartGame();
                }
            }
        }

        public void UpdateHealth(float current, float max)
        {
            if (_hpFill != null) SetBar(_hpFill, current , max);
            if (_hpText != null) _hpText.text = $"HP: {Mathf.CeilToInt(current)} / {Mathf.CeilToInt(max)}";
        }

        public void UpdateXP(int current, int toNext, int level)
        {
            if (_xpFill != null) SetBar(_xpFill, current, toNext);
            if (_xpText != null) _xpText.text = $"LVL {level} - XP {current} / {toNext}";
        }

        public void UpdateGold(int gold)
        {
            if (_goldText != null) _goldText.text = $"Gold: {gold}";
        }

        public void UpdateFloor(int floor)
        {
            if (_floorText != null) _floorText.text = $"FLOOR {floor}";
        }

        public void UpdateEnemiesRemaining(int count)
        {
            if (_enemyText != null)
            {
                _enemyText.text = count > 0 ? $"Enemies Remaining: {count}" : "<color=#4dff88>Room Cleared! Enter Portal</color>";
            }
        }

        public void ShowBanner(string message, float duration = 3.5f)
        {
            if (_bannerRoot != null && _bannerText != null)
            {
                _bannerText.text = message;
                _bannerRoot.SetActive(true);
                _bannerTimer = duration;
            }
        }

        public void ShowLevelUpModal(UpgradeType[] options)
        {
            Time.timeScale = 0f; // Pause gameplay
            _upgradeModalRoot.SetActive(true);

            for (int i = 0; i < _upgradeCards.Count; i++)
            {
                var card = _upgradeCards[i];
                if (i < options.Length)
                {
                    card.SetActive(true);
                    UpgradeType upType = options[i];

                    string title = GetUpgradeTitle(upType);
                    string desc = GetUpgradeDesc(upType);
                    Sprite icon = GetUpgradeIcon(upType);

                    card.transform.Find("Name").GetComponent<Text>().text = title;
                    card.transform.Find("Desc").GetComponent<Text>().text = desc;
                    card.transform.Find("Icon").GetComponent<Image>().sprite = icon;

                    var btn = card.transform.Find("SelectButton").GetComponent<Button>();
                    btn.onClick.RemoveAllListeners();
                    btn.onClick.AddListener(() =>
                    {
                        PlayerController.Instance?.stats.ApplyUpgrade(upType);
                        Time.timeScale = 1f;
                        _upgradeModalRoot.SetActive(false);
                        AudioManager.Instance?.PlaySound("potion");
                    });
                }
                else
                {
                    card.SetActive(false);
                }
            }
        }

        private string GetUpgradeTitle(UpgradeType type)
        {
            switch (type)
            {
                case UpgradeType.MaxHealth: return "Titan's Heart";
                case UpgradeType.Damage: return "Sharpened Edge";
                case UpgradeType.MoveSpeed: return "Swift Wind";
                case UpgradeType.AttackSpeed: return "Flurry";
                case UpgradeType.DashCooldown: return "Blink Step";
                case UpgradeType.Lifesteal: return "Vampiric Touch";
                case UpgradeType.Armor: return "Iron Aegis";
                default: return "Mystery Boon";
            }
        }

        private string GetUpgradeDesc(UpgradeType type)
        {
            switch (type)
            {
                case UpgradeType.MaxHealth: return "+25 Max Health\n& Instant Recovery";
                case UpgradeType.Damage: return "+8 Attack Damage\nFor all weapon swings";
                case UpgradeType.MoveSpeed: return "+15% Movement Speed\nOutrun dangers";
                case UpgradeType.AttackSpeed: return "+30% Attack Speed\nRapid sword strikes";
                case UpgradeType.DashCooldown: return "-0.2s Dash Cooldown\nGreater mobility";
                case UpgradeType.Lifesteal: return "+8% Lifesteal\nHeal on every hit";
                case UpgradeType.Armor: return "+2 Damage Reduction\nResist all hits";
                default: return "+Bonus Stats";
            }
        }

        private Sprite GetUpgradeIcon(UpgradeType type)
        {
            switch (type)
            {
                case UpgradeType.MaxHealth: return SpriteFactory.GetSprite("potion_hp");
                case UpgradeType.Damage: return SpriteFactory.GetSprite("sword");
                case UpgradeType.MoveSpeed: return SpriteFactory.GetSprite("gem_xp");
                case UpgradeType.AttackSpeed: return SpriteFactory.GetSprite("slash");
                case UpgradeType.DashCooldown: return SpriteFactory.GetSprite("player");
                case UpgradeType.Lifesteal: return SpriteFactory.GetSprite("projectile_enemy");
                case UpgradeType.Armor: return SpriteFactory.GetSprite("pillar");
                default: return SpriteFactory.GetSprite("coin");
            }
        }

        public void ShowGameOverModal(int floor, int level, int gold, int enemiesSlain)
        {
            _gameOverRoot.SetActive(true);
            if (_gameOverStatsText != null)
            {
                _gameOverStatsText.text = $"Floor Reached: {floor}\nFinal Level: {level}\nEnemies Slain: {enemiesSlain}\nGold Accumulated: {gold}";
            }
        }

        public void HideGameOverModal()
        {
            if (_gameOverRoot != null)
            {
                _gameOverRoot.SetActive(false);
            }
        }

        // Helper UI creation methods
        private GameObject CreateUIObject(string name, Transform parent, Vector2 anchorMin, Vector2 anchorMax, Vector2 pivot, Vector2 pos, Vector2 size)
        {
            var go = new GameObject(name);
            go.transform.SetParent(parent, false);
            var rect = go.AddComponent<RectTransform>();
            rect.anchorMin = anchorMin;
            rect.anchorMax = anchorMax;
            rect.pivot = pivot;
            rect.anchoredPosition = pos;
            rect.sizeDelta = size;
            return go;
        }

        private GameObject CreateImage(string name, Transform parent, Color color, Vector2 anchorMin, Vector2 anchorMax, Vector2 pivot, Vector2 pos, Vector2 size)
        {
            var go = CreateUIObject(name, parent, anchorMin, anchorMax, pivot, pos, size);
            var img = go.AddComponent<Image>();
            img.color = color;
            return go;
        }

        private GameObject CreateText(string name, Transform parent, string content, int fontSize, Color color, TextAnchor align)
        {
            var go = CreateUIObject(name, parent, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), Vector2.zero, new Vector2(200, 40));
            var txt = go.AddComponent<Text>();
            txt.font = _font;
            txt.text = content;
            txt.fontSize = fontSize;
            txt.color = color;
            txt.alignment = align;
            txt.raycastTarget = false;
            return go;
        }
    }
}
