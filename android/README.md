# Geometric Dungeon for Android

The Android build is the same C++/raylib game as the Windows exe (`StandaloneApp/main.cpp`), built into a native-activity APK. There is no Java code.

## Get the APK (no tools needed)

Every push that touches `StandaloneApp/`, `android/` or the workflow builds the APK on GitHub:

1. Open the repository's **Releases** page and download `GeometricDungeon-android.apk` from the **Android APK (latest build)** pre-release, or open the **Actions** tab, pick the latest *Android APK* run and download the `GeometricDungeon-android` artifact.
2. Copy it to your phone (or download it there directly), open it and allow **Install unknown apps** for the app you opened it with.
3. Start **Geometric Dungeon**. It runs in landscape and uses the whole screen.

The APK is signed with a debug key, so it installs directly but is not meant for the Play Store. If you later install a build signed with another key you have to uninstall the old one first.

## Controls on a phone

* **Left half of the screen:** floating virtual joystick (touch anywhere and drag).
* **Right side:** big sword button = slash (hold it; the game auto-aims at the nearest enemy), **DASH** and **NOVA** buttons with a cooldown ring.
* **Pause** and **equipment** buttons under the radar; a **TRADE** button appears next to the merchant.
* Menus are fully tappable (title, pause, game over, level-up cards, shop, Sanctuary). The Android **Back** button pauses, closes screens and quits from the title.
* A Bluetooth/USB gamepad works too (same mapping as on PC).

Settings and progress (Souls, Sanctuary upgrades, best floor) are stored in the app's private storage. Resolution / fullscreen / VSync are managed by Android and shown as "Auto". Particles default to Medium on phones.

## Build it yourself

You need Android Studio (or the command line SDK with NDK and CMake 3.22), JDK 17 and Gradle 8.9+:

```sh
git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git android/raylib
gradle -p android assembleRelease
# APK: android/app/build/outputs/apk/release/app-release.apk
```

The `main.cpp` source is shared with the desktop build, Android-specific code is behind `#ifdef __ANDROID__`.
