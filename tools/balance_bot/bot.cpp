#define main game_main_unused
#include "../../StandaloneApp/main.cpp"
#undef main
#include <map>
#include <numeric>

static const char* typeName(int t) {
    static const char* n[8] = { "Slime", "Spitter", "Brute", "Boss", "Bomber", "Charger", "Splitter", "Summoner" };
    return n[t];
}

struct RunStats {
    int floorReached = 1;
    double seconds = 0;
    int kills = 0;
    std::string cause = "?";
    bool stuck = false;
    std::map<int, double> floorTime;
    std::map<int, double> hpLost; // theo tỉ lệ máu tối đa
};

static void ChooseUpgradeBot(Game& g) {
    static const UpgradeType prio[] = { UP_DMG, UP_ATTACK_RATE, UP_HP, UP_CRIT, UP_LIFESTEAL, UP_ARMOR, UP_IGNITE, UP_NOVA, UP_SPEED, UP_DASH, UP_FROST };
    bool low = g.player.hp < g.player.maxHp * 0.5f;
    int best = 0, bestScore = 1 << 30;
    for (int i = 0; i < (int)g.levelChoices.size(); ++i) {
        UpgradeType u = g.levelChoices[i];
        int sc = 100;
        for (int k = 0; k < 11; ++k) if (prio[k] == u) sc = k;
        if (low && (u == UP_HP || u == UP_LIFESTEAL || u == UP_ARMOR)) sc -= 5;
        if (sc < bestScore) { bestScore = sc; best = i; }
    }
    g.ChooseUpgrade(g.levelChoices[best]);
}

static int gSkill = 2; // 2 = giỏi, 1 = trung bình, 0 = vụng về
static void BotControl(Game& g) {
    auto& c = g.ctl;
    c.active = true; c.external = true; c.attack = c.dash = c.nova = false; c.move = { 0, 0 };
    if (g.state == Game::LEVEL_UP) { ChooseUpgradeBot(g); return; }
    if (g.state != Game::PLAYING) return;
    Vector2 P = g.player.pos;
    float hpFrac = g.player.hp / g.player.maxHp;

    Enemy* near = nullptr; float nd = 1e9f; int cnt190 = 0;
    for (auto& e : g.enemies) {
        if (e.hp <= 0) continue;
        float d = Vector2Distance(P, e.pos);
        if (d < nd) { nd = d; near = &e; }
        if (d < 190.0f) cnt190++;
    }
    Vector2 avoid = { 0, 0 };
    for (auto& pr : g.projectiles) {
        if (pr.fromPlayer) continue;
        float d = Vector2Distance(P, pr.pos);
        if (d < 120.0f && d > 0.1f) avoid = Vector2Add(avoid, Vector2Scale(Vector2Normalize(Vector2Subtract(P, pr.pos)), (120.0f - d) / 120.0f * 1.6f));
    }
    // né vòng nổ của Bomber đang cháy ngòi và vùng báo hiệu đập đất
    for (auto& e : g.enemies) {
        if (e.hp <= 0) continue;
        float d = Vector2Distance(P, e.pos);
        if (e.type == BOMBER && e.fusing && d < 150.0f) avoid = Vector2Add(avoid, Vector2Scale(Vector2Normalize(Vector2Subtract(P, e.pos)), 2.5f));
        if (e.type == BOSS && e.bossKind == 1 && e.chargeState == 1 && d < 230.0f) avoid = Vector2Add(avoid, Vector2Scale(Vector2Normalize(Vector2Subtract(P, e.pos)), 2.5f));
        if (((e.type == CHARGER && e.chargeState == 1) || (e.type == BOSS && e.bossKind == 1 && e.chargeState == 3))) {
            // thoát khỏi làn lao: bước vuông góc
            Vector2 perp = { -e.chargeDir.y, e.chargeDir.x };
            Vector2 rel = Vector2Subtract(P, e.pos);
            float along = Vector2DotProduct(rel, e.chargeDir);
            float side = Vector2DotProduct(rel, perp);
            if (along > 0 && fabsf(side) < 70.0f) avoid = Vector2Add(avoid, Vector2Scale(perp, (side >= 0 ? 1.0f : -1.0f) * 2.2f));
        }
    }

    Vector2 target = P;
    bool haveTarget = false;
    bool anyAlive = near != nullptr || !g.pendingSpawns.empty();

    if (near) {
        Vector2 toE = Vector2Subtract(near->pos, P);
        Vector2 dirE = nd > 0.1f ? Vector2Normalize(toE) : Vector2{ 1, 0 };
        c.aim = dirE;
        bool dangerous = (near->type == BOSS || near->type == BRUTE || near->isWarden);
        float keep = 55.0f;
        if (g.player.attackRate > 0) {
            int wt = g.hasEquip[SLOT_WEAPON] ? g.equipped[SLOT_WEAPON].weaponType : 0;
            keep = (wt == 3) ? 95.0f : (wt == 2 ? 70.0f : 55.0f);
        }
        if (hpFrac < 0.35f && nd < 170.0f) {
            c.move = Vector2Negate(dirE); // rút lui
            if (nd < 80.0f && g.player.dashTimer <= 0) c.dash = true;
        } else if (nd > keep) {
            c.move = dirE;
        } else {
            // đứng đánh, hơi đi vòng
            Vector2 perp = { -dirE.y, dirE.x };
            c.move = Vector2Scale(perp, 0.5f);
            if (dangerous) c.move = Vector2Add(c.move, Vector2Scale(dirE, -0.3f));
        }
        c.attack = nd < keep + 35.0f;
        if (g.player.specialTimer <= 0 && (cnt190 >= 3 || (nd < 190.0f && hpFrac < 0.4f))) c.nova = true;
        if (nd < 60.0f && hpFrac < 0.5f && g.player.dashTimer <= 0 && near->type == CHARGER) c.dash = true;
        haveTarget = true;
    }
    if (!anyAlive) {
        // dọn xong: nhặt rương, đồ, rồi vào cửa hàng / cổng
        float best = 1e9f;
        for (auto& ch : g.chests) if (!ch.opened) { float d = Vector2Distance(P, ch.pos); if (d < best) { best = d; target = ch.pos; haveTarget = true; } }
        if (!haveTarget) for (auto& d : g.itemDrops) { float dd = Vector2Distance(P, d.pos); if (dd < best) { best = dd; target = d.pos; haveTarget = true; } }
        if (!haveTarget && g.shopFloor && g.player.gold >= 25) {
            bool anyLeft = false; for (int i = 0; i < 5; ++i) if (!g.shopSlots[i].sold) anyLeft = true;
            if (anyLeft) {
                target = { g.merchantPos.x, g.merchantPos.y + 70.0f };
                haveTarget = true;
                if (Vector2Distance(P, g.merchantPos) < 125.0f) {
                    // mua: bình khi máu < 70%, trang bị nếu mua nổi
                    for (int i = 0; i < 4; ++i) if (!g.shopSlots[i].sold && g.player.gold >= g.shopSlots[i].price) g.BuyShopSlot(i);
                    if (hpFrac < 0.7f) g.BuyShopSlot(4);
                    for (int i = 0; i < 5; ++i) g.shopSlots[i].sold = true; // xong, đi tiếp
                }
            }
        }
        if (!haveTarget && g.portalOpen) { target = g.portalPos; haveTarget = true; }
        if (haveTarget) {
            Vector2 d = Vector2Subtract(target, P);
            if (Vector2Length(d) > 6.0f) c.move = Vector2Normalize(d);
            c.aim = c.move;
        }
    }
    // đi vòng qua cột đá: đẩy ra khỏi cột và trượt theo tiếp tuyến cùng chiều đang muốn đi
    {
        Vector2 want = c.move;
        for (auto& pl : g.pillars) {
            Vector2 rel = Vector2Subtract(P, pl);
            float d = Vector2Length(rel);
            if (d < 100.0f && d > 0.1f) {
                Vector2 radial = Vector2Scale(rel, 1.0f / d);
                Vector2 tang = { -radial.y, radial.x };
                if (Vector2DotProduct(tang, want) < 0) tang = Vector2Negate(tang);
                float k = (100.0f - d) / 100.0f;
                avoid = Vector2Add(avoid, Vector2Add(Vector2Scale(radial, 0.9f * k), Vector2Scale(tang, 1.3f * k)));
            }
        }
    }
    // gỡ kẹt: nếu gần như không nhích sau 2s mà đang cần đi tới đâu đó, đi lang thang ngẫu nhiên 1,2s
    static Vector2 chkPos = { 0, 0 }; static int chkTimer = 0, wander = 0; static Vector2 wanderDir = { 1, 0 }; static int chkFloor = -1;
    if (chkFloor != g.floor) { chkFloor = g.floor; chkTimer = 0; wander = 0; chkPos = P; }
    if (++chkTimer >= 120) {
        chkTimer = 0;
        bool wantsMove = Vector2Length(c.move) > 0.2f && (!near || nd > 110.0f);
        if (wantsMove && Vector2Distance(P, chkPos) < 30.0f) { wander = 75; float a = Rand01() * 6.2832f; wanderDir = { cosf(a), sinf(a) }; }
        chkPos = P;
    }
    if (wander > 0) { --wander; c.move = wanderDir; }
    // gộp né đạn
    c.move = Vector2Add(c.move, Vector2Scale(avoid, gSkill == 2 ? 1.0f : (gSkill == 1 ? 0.55f : 0.25f)));
    if (Vector2Length(c.move) > 1.0f) c.move = Vector2Normalize(c.move);
    if (gSkill < 2) {
        // người chơi kém hơn: không lướt/Nova đúng lúc, ngắm lệch, bỏ lỡ nhịp đánh
        if (gSkill == 0) { c.dash = false; c.nova = false; }
        else if (Rand01() < 0.5f) { c.dash = false; c.nova = false; }
        float err = (gSkill == 0) ? 0.35f : 0.18f;
        float a = atan2f(c.aim.y, c.aim.x) + (Rand01() - 0.5f) * 2.0f * err;
        c.aim = { cosf(a), sinf(a) };
        if (Rand01() < (gSkill == 0 ? 0.35f : 0.15f)) c.attack = false;
    }
}

int main(int argc, char** argv) {
    int runs = argc > 1 ? atoi(argv[1]) : 30;
    int metaLevel = argc > 2 ? atoi(argv[2]) : 0; // 0 = không có nâng cấp Thánh địa
    gSkill = argc > 3 ? atoi(argv[3]) : 2;
    int diff = argc > 4 ? atoi(argv[4]) : 1;
    remove("settings.cfg");
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(1280, 720, "bot");
    Game g; g.Init(); SetTargetFPS(0);
    g.settings.difficulty = diff;
    std::vector<RunStats> all;
    std::map<std::string, int> causes;
    const double dt = 1.0 / 60.0;
    for (int run = 0; run < runs; ++run) {
        srand(1000 + run * 7919);
        for (int i = 0; i < 8; ++i) g.meta[i] = std::min(metaLevel, kMeta[i].maxLevel);
        g.RestartRun();
        g.state = Game::PLAYING;
        g.bannerTimer = 0;
        RunStats rs;
        int lastFloor = g.floor; double floorClock = 0; float prevHp = g.player.hp;
        std::string cause = "?";
        const int maxFrames = 60 * 60 * 45;
        for (int f = 0; f < maxFrames; ++f) {
            // ảnh chụp nguyên nhân tử vong: kẻ địch/đạn gần nhất trước khi cập nhật
            { float md = 1e9f; std::string who = "contact";
              for (auto& e : g.enemies) if (e.hp > 0) { float d = Vector2Distance(g.player.pos, e.pos) - e.radius; if (d < md) { md = d; who = typeName(e.type); if (e.type == BOSS) who = e.bossKind ? "Molten Colossus" : "Void Sovereign"; if (e.isWarden) who = "Warden"; } }
              for (auto& pr : g.projectiles) if (!pr.fromPlayer && Vector2Distance(g.player.pos, pr.pos) < md) { md = Vector2Distance(g.player.pos, pr.pos); who = "projectile"; }
              cause = who; }
            BotControl(g);
            float hpBefore = g.player.hp;
            g.Update((float)dt);
            if (g.player.hp < hpBefore) rs.hpLost[g.floor] += (hpBefore - g.player.hp) / g.player.maxHp;
            rs.seconds += dt; floorClock += dt;
            rs.floorTime[g.floor] += dt;
            if (g.floor != lastFloor) { lastFloor = g.floor; floorClock = 0; }
            if (g.state == Game::GAME_OVER) { rs.cause = cause; break; }
            if (floorClock > 150.0) {
                rs.stuck = true; rs.cause = "STUCK";
                int alive = 0; for (auto& e : g.enemies) if (e.hp > 0) alive++;
                printf("  STUCK diag: floor %d layout %d mod %d shop %d portalOpen %d alive %d pending %zu pos (%.0f,%.0f) chests unopened %d itemDrops %zu loots %zu state %d\n", g.floor, g.layout, g.floorMod, g.shopFloor, g.portalOpen, alive, g.pendingSpawns.size(), g.player.pos.x, g.player.pos.y, (int)std::count_if(g.chests.begin(), g.chests.end(), [](const Chest& c){ return !c.opened; }), g.itemDrops.size(), g.loots.size(), (int)g.state);
                for (auto& e : g.enemies) if (e.hp > 0) printf("    enemy type %s at (%.0f,%.0f) hp %.0f minion %d\n", typeName(e.type), e.pos.x, e.pos.y, e.hp, e.isMinion);
                break;
            }
            if (g.floor >= 30) { rs.cause = "cleared 30"; break; }
        }
        rs.floorReached = g.floor;
        rs.kills = g.enemiesSlain;
        all.push_back(rs);
        causes[rs.cause]++;
        printf("run %2d: floor %2d, %5.0fs, kills %3d, lvl %2d, hp %.0f, cause %s\n", run, rs.floorReached, rs.seconds, rs.kills, g.player.level, g.player.maxHp, rs.cause.c_str());
        fflush(stdout);
    }
    printf("\n=== summary over %d runs (meta level %d, skill %d, difficulty %d) ===\n", runs, metaLevel, gSkill, diff);
    double avgFloor = 0; for (auto& r : all) avgFloor += r.floorReached; avgFloor /= all.size();
    printf("avg floor reached: %.1f\n", avgFloor);
    printf("floor | reached%% | died here%% | avg time(s) | avg hp lost (x max hp) | shop?\n");
    for (int f = 1; f <= 30; ++f) {
        int reached = 0, died = 0; double t = 0, hl = 0; int n = 0;
        for (auto& r : all) { if (r.floorReached >= f) reached++; if (r.floorReached == f && r.cause != "cleared 30") died++; if (r.floorTime.count(f)) { t += r.floorTime.at(f); hl += r.hpLost.count(f) ? r.hpLost.at(f) : 0; n++; } }
        if (reached == 0) break;
        printf("%4d  | %5.0f%%   | %5.0f%%     | %6.0f      | %6.2f                 | %s\n", f, 100.0 * reached / all.size(), 100.0 * died / all.size(), n ? t / n : 0.0, n ? hl / n : 0.0, IsShopFloor(f) ? "shop" : (f % 5 == 0 ? "BOSS" : ""));
    }
    printf("causes of death:");
    for (auto& kv : causes) printf("  %s=%d", kv.first.c_str(), kv.second);
    printf("\n");
    CloseWindow();
    return 0;
}
