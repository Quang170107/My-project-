#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <random>
#include <ctime>

// ── Phông chữ Unicode đậm sắc nét đa kích thước hỗ trợ đầy đủ Tiếng Việt ──
static const int kFontBuckets[7] = { 15, 18, 21, 24, 28, 34, 46 };
static Font gUiFonts[7] = { { 0 } };
static bool gFontLoaded = false;
static bool gDrawingOutlinePass = false;

static void AppendUtf8(std::string& dst, int cp) {
    if (cp <= 0x7F) {
        dst.push_back((char)cp);
    } else if (cp <= 0x7FF) {
        dst.push_back((char)(0xC0 | ((cp >> 6) & 0x1F)));
        dst.push_back((char)(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        dst.push_back((char)(0xE0 | ((cp >> 12) & 0x0F)));
        dst.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        dst.push_back((char)(0x80 | (cp & 0x3F)));
    } else {
        dst.push_back((char)(0xF0 | ((cp >> 18) & 0x07)));
        dst.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
        dst.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        dst.push_back((char)(0x80 | (cp & 0x3F)));
    }
}

static void PopUtf8Char(std::string& s) {
    while (!s.empty() && ((unsigned char)s.back() & 0xC0) == 0x80) s.pop_back();
    if (!s.empty()) s.pop_back();
}

static void InitGameFont() {
    if (gFontLoaded) return;
    std::vector<int> cps;
    for (int c = 0x0020; c <= 0x007E; ++c) cps.push_back(c);
    for (int c = 0x00A0; c <= 0x024F; ++c) cps.push_back(c);
    for (int c = 0x1E00; c <= 0x1EFF; ++c) cps.push_back(c);
    for (int c = 0x2010; c <= 0x203A; ++c) cps.push_back(c);
    cps.push_back(0x20AB);
    static const char* kFontPaths[] = {
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/tahomabd.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/system/fonts/Roboto-Bold.ttf",
        "/system/fonts/NotoSans-Bold.ttf",
        "/system/fonts/Roboto-Regular.ttf",
        "/system/fonts/NotoSans-Regular.ttf"
    };
    const char* chosen = nullptr;
    for (const char* path : kFontPaths) {
        if (FileExists(path)) { chosen = path; break; }
    }
    if (!chosen) return;
    for (int i = 0; i < 7; ++i) {
        Font f = LoadFontEx(chosen, kFontBuckets[i], cps.data(), (int)cps.size());
        if (f.texture.id > 0 && f.baseSize > 0) {
            gUiFonts[i] = f;
            SetTextureFilter(gUiFonts[i].texture, TEXTURE_FILTER_POINT);
        }
    }
    gFontLoaded = (gUiFonts[0].texture.id > 0 && gUiFonts[0].baseSize > 0);
}

static void CloseGameFont() {
    for (int i = 0; i < 7; ++i) {
        if (gUiFonts[i].texture.id > 0 && gUiFonts[i].baseSize > 0) {
            UnloadFont(gUiFonts[i]);
            gUiFonts[i] = { 0 };
        }
    }
    gFontLoaded = false;
}

static int EffectiveFontSize(int fontSize) {
    if (fontSize <= 11) return fontSize + 3;
    if (fontSize <= 15) return fontSize + 2;
    if (fontSize <= 20) return fontSize + 1;
    return fontSize;
}

static const Font& PickGameFont(int effSize) {
    int best = 0;
    int bestDiff = 9999;
    for (int i = 0; i < 7; ++i) {
        if (gUiFonts[i].baseSize <= 0) continue;
        int d = abs(gUiFonts[i].baseSize - effSize);
        if (d < bestDiff) { bestDiff = d; best = i; }
    }
    return gUiFonts[best];
}

static void GameDrawText(const char* text, int posX, int posY, int fontSize, Color color) {
    if (!text) return;
    int eff = EffectiveFontSize(fontSize);
    if (gFontLoaded) {
        const Font& f = PickGameFont(eff);
        const float sp = (eff >= 28) ? 0.8f : 0.4f;
        if (!gDrawingOutlinePass && ((int)color.r + (int)color.g + (int)color.b) > 75 && color.a > 60) {
            Color sh = { 6, 8, 16, (unsigned char)std::min(235, (int)color.a) };
            DrawTextEx(f, text, { (float)(posX + 1), (float)(posY + 1) }, (float)eff, sp, sh);
            DrawTextEx(f, text, { (float)posX, (float)(posY + 1) }, (float)eff, sp, sh);
        }
        DrawTextEx(f, text, { (float)posX, (float)posY }, (float)eff, sp, color);
    } else {
        ::DrawText(text, posX, posY, fontSize, color);
    }
}

static int GameMeasureText(const char* text, int fontSize) {
    if (!text) return 0;
    int eff = EffectiveFontSize(fontSize);
    if (gFontLoaded) {
        const Font& f = PickGameFont(eff);
        const float sp = (eff >= 28) ? 0.8f : 0.4f;
        return (int)ceilf(MeasureTextEx(f, text, (float)eff, sp).x);
    }
    return ::MeasureText(text, fontSize);
}

#define DrawText GameDrawText
#define MeasureText GameMeasureText

// ── Audio Procedural Synthesizer ───────────────────────────────────────────
struct SoundManager {
    Sound sndSlash;
    Sound sndHit;
    Sound sndDeath;
    Sound sndShoot;
    Sound sndDash;
    Sound sndCoin;
    Sound sndPotion;
    Sound sndLevelUp;
    Sound sndPortal;
    Sound sndHurt;
    Sound sndSpecial;
    Sound sndShield;
    Sound sndFreeze;

    void Init() {
        InitAudioDevice();
        sndSlash   = GenTone(0.12f, 440, 150, 3);
        sndHit     = GenTone(0.08f, 220, 80, 1);
        sndDeath   = GenTone(0.25f, 300, 50, 4);
        sndShoot   = GenTone(0.14f, 880, 440, 0);
        sndDash    = GenTone(0.15f, 600, 200, 3);
        sndHurt    = GenTone(0.12f, 180, 90, 2);
        sndSpecial = GenTone(0.35f, 180, 40, 0);
        sndFreeze  = GenTone(0.18f, 1200, 600, 0);

        sndCoin    = GenChime({ 988.0f, 1318.0f }, 0.08f);
        sndPotion  = GenChime({ 523.0f, 659.0f, 784.0f }, 0.07f);
        sndLevelUp = GenChime({ 523.0f, 659.0f, 784.0f, 1046.0f }, 0.12f);
        sndPortal  = GenChime({ 440.0f, 554.0f, 659.0f, 880.0f }, 0.15f);
        sndShield  = GenChime({ 880.0f, 1320.0f }, 0.09f);
    }

    void Close() {
        UnloadSound(sndSlash);
        UnloadSound(sndHit);
        UnloadSound(sndDeath);
        UnloadSound(sndShoot);
        UnloadSound(sndDash);
        UnloadSound(sndCoin);
        UnloadSound(sndPotion);
        UnloadSound(sndLevelUp);
        UnloadSound(sndPortal);
        UnloadSound(sndHurt);
        UnloadSound(sndSpecial);
        UnloadSound(sndShield);
        UnloadSound(sndFreeze);
        CloseAudioDevice();
    }

private:
    Sound GenTone(float duration, float startFreq, float endFreq, int type) {
        int sampleRate = 44100;
        int sampleCount = (int)(duration * sampleRate);
        short* samples = (short*)malloc(sampleCount * sizeof(short));
        float phase = 0.0f;

        for (int i = 0; i < sampleCount; ++i) {
            float t = (float)i / sampleCount;
            float freq = startFreq + (endFreq - startFreq) * t;
            float env = 1.0f - t;
            float s = 0.0f;
            if (type == 0) s = sinf(phase);
            else if (type == 1) s = (sinf(phase) >= 0 ? 0.6f : -0.6f);
            else if (type == 2) s = (2.0f * (phase / (2.0f * PI) - floorf(phase / (2.0f * PI) + 0.5f))) * 0.7f;
            else if (type == 3) s = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.5f;
            else if (type == 4) s = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * (1.0f - t * 0.8f);

            phase += (2.0f * PI * freq) / sampleRate;
            samples[i] = (short)(s * env * 24000.0f);
        }

        Wave wave = { 0 };
        wave.frameCount = sampleCount;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples;
        Sound snd = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return snd;
    }

    Sound GenChime(const std::vector<float>& freqs, float noteDur) {
        int sampleRate = 44100;
        int noteSamples = (int)(noteDur * sampleRate);
        int totalSamples = (int)(noteSamples * freqs.size());
        short* samples = (short*)malloc(totalSamples * sizeof(short));

        for (size_t n = 0; n < freqs.size(); ++n) {
            float freq = freqs[n];
            float phase = 0.0f;
            for (int i = 0; i < noteSamples; ++i) {
                int idx = (int)(n * noteSamples + i);
                float t = (float)i / noteSamples;
                float env = powf(1.0f - t, 1.5f);
                float s = sinf(phase) * 0.7f;
                phase += (2.0f * PI * freq) / sampleRate;
                samples[idx] = (short)(s * env * 22000.0f);
            }
        }

        Wave wave = { 0 };
        wave.frameCount = totalSamples;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples;
        Sound snd = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return snd;
    }
};

// ── Game Data Structures ───────────────────────────────────────────────────
