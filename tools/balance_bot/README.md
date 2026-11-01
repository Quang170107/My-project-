# Balance bot

A headless bot that plays the standalone game (`StandaloneApp/main.cpp`) through the same code path as a real player (it uses the `ControlOverride` hook, so movement, attacks, dash, Nova, level-ups, chests, gear and the shop all go through the real game logic) and prints a per-floor difficulty table.

It is a tuning tool, not a human: use it to compare *relative* difficulty before and after a balance change.

## Build and run (Linux, virtual display)

```sh
# raylib 5.5 built as a static library: https://github.com/raysan5/raylib
g++ -std=c++17 -O2 tools/balance_bot/bot.cpp -I path/to/raylib/src path/to/libraylib.a \
    -lGL -lX11 -lXrandr -lXinerama -lXcursor -lXi -lpthread -ldl -lm -lrt -o bot
xvfb-run -a ./bot <runs> <sanctuary level> <skill> <difficulty>
```

* `runs` - number of simulated runs (default 30)
* `sanctuary level` - level given to every Sanctuary upgrade (0 = none, 3 = a few hours of play, 6 = lots)
* `skill` - 2 = good dodging, dash and Nova, 1 = average, 0 = clumsy (poor dodging, no dash/Nova, bad aim)
* `difficulty` - 0 easy, 1 normal, 2 hard

Runs are seeded, so the same arguments give the same result. A run ends on death, on clearing floor 30, or if the bot is stuck on a floor for 150 s (a stuck report with the enemies still alive is printed; that is how the "enemy walks out of the room" soft-lock was found).

## Reference results (30-40 runs each, no Sanctuary upgrades unless noted)

| skill | difficulty | avg floor |
| --- | --- | --- |
| clumsy | easy | 17.5 |
| clumsy | normal | 9.4 |
| clumsy | normal, Sanctuary level 3 / 6 | 19.4 / 25.2 |
| average | normal | 17.8 |
| good | normal | 18.7 |
| good | hard | 10.9 |
