# Seagull Storm Unreal: Issue Status

These issues were found during a design-doc audit. All code-side issues are resolved;
the only remaining work is editor-only binary asset creation, documented step by step
in [EDITOR_SETUP.md](EDITOR_SETUP.md).

---

## General Issues (shared across all engines)

### G1: Google OAuth is a stub: RESOLVED
`OnGoogleClicked()` no longer fires a doomed `SignInGoogle("", "")` network call
(the SDK requires a pre-obtained OAuth authorization code and has no code-acquisition
flow). It now surfaces a proper in-game error on the title screen's `StatusText`
("Google sign-in is not available on this platform"), which renders since the widget
trees are now constructed in C++ (`SeagullTitleScreen.cpp`).

### G2: Pause Menu News makes a new request mid-run: RESOLVED
News is loaded once at hub entry, cached on `USeagullGameInstance`
(`bNewsLoaded`/`CachedNews`), and the pause-menu news panel renders exclusively from
that cache (`SeagullNewsPanel.cpp`). Zero network requests during a run.

### G3: Remote Config must only be loaded once per session: RESOLVED
`LoadAllConfigs` is guarded by `GI->bConfigLoaded` and never re-fetched on hub
re-entry; the flag only resets on sign-out. `StartCrashCapture` moved to
`SeagullStormGameMode::BeginPlay()` behind `bCrashCaptureStarted` (see UE8).

---

## Unreal-Specific Issues

### UE1: All Unreal binary assets missing: EDITOR-ONLY REMAINDER
Everything creatable in code now exists: the full UMG widget trees for all 11 screens
are constructed in pure C++ (`Source/SeagullStorm/UI/`, shared styling in
`SeagullWidgetStyles.h/.cpp`), so no `WBP_` assets are needed at all. The `.gitignore`
lines that blocked committing `*.uasset`/`*.umap` were removed. Remaining: create
`Content/Maps/MainMap.umap` and import audio/textures in the Unreal Editor;
see [EDITOR_SETUP.md](EDITOR_SETUP.md) steps 1-3.

### UE2: Audio assets never loaded: RESOLVED (code) / import pending (editor)
Code side done: `SeagullAudioManager::Initialize()` loads all 14 sounds via
`LoadObject<USoundWave>` from `/Game/Audio/...` paths and logs a loaded count. The 14
`USoundWave*` properties stay plain `UPROPERTY()` (GC-referencing only): the audio
manager is created with `NewObject` at runtime and has no Blueprint/archetype editing
surface, so `EditDefaultsOnly`/Blueprint override would have no effect (documented in
`SeagullAudioManager.h`). Every sound has a live trigger site. Remaining: import the 14
OGG files in the editor at the matching paths; see
[EDITOR_SETUP.md](EDITOR_SETUP.md) step 2.

### UE3: PaperFlipbook assets missing: RESOLVED (code) / flipbooks pending (editor)
Code side done: player pawn, all four enemy types, XP/coin pickups and the feather
projectile load canonical `/Game/Flipbooks/FB_*` flipbooks in their constructors
(null-guarded), and the animation state switching is implemented: player Idle/Walk by
velocity plus Hurt flash and Death hooks, enemies switch to their death flipbook and
play it before despawning. Remaining: extract sprites and create the `FB_*` assets in
the editor; exact names, frame counts and FPS are tabled in
[EDITOR_SETUP.md](EDITOR_SETUP.md) step 4.

### UE4: Pause Menu News re-fetches mid-run: RESOLVED
Same fix as G2 (`USeagullGameInstance::CachedNews`).

### UE5: Gift Code reward hardcoded to 500 coins: RESOLVED
`SeagullGiftCodePanel.cpp` parses the server's `GiftData` JSON and applies the
`coins` field; 500 is only the fallback when parsing yields nothing.

### UE6: Missing breadcrumb for hub upgrade purchases: RESOLVED
`TryBuyUpgrade()` records `bought_<key>_<level>` as a `user_action` breadcrumb.

### UE7: Music crossfade not implemented: RESOLVED
`CrossfadeToTrack()` fades the current track out over `MusicCrossfadeDuration`
(0.5s) and fades the new track in after the crossfade timer.

### UE8: StartCrashCapture called on every hub visit: RESOLVED
Sole call site is `SeagullStormGameMode::BeginPlay()` guarded by
`bCrashCaptureStarted`, once per app start.

### UE9: Jellyfish poison zone not implemented: RESOLVED
`ASeagullPoisonZone` spawns at the jellyfish death location: 2.5s lifetime,
5 damage per 0.5s tick to overlapping player pawns.

### UE10: Gamepad/Joystick input missing: RESOLVED
Gamepad left stick (`Gamepad_LeftX`/`Gamepad_LeftY` with swizzle) drives the Move
action and `Gamepad_Special_Right` pauses, all in the runtime-built Enhanced Input
mapping context.

### UE11: Feedback category FEATURE vs FEATURE_REQUEST: RESOLVED
The feedback form offers `BUG` / `FEATURE_REQUEST` / `GENERAL`.

### UE12: Magnet level missing from User Log string: RESOLVED
The game-over user log includes `magnet:%d` alongside speed/dmg/hp.

### UE13: Default Map MainMap.umap does not exist: EDITOR-ONLY REMAINDER
Blocked-by-.gitignore part fixed (ignore lines removed). Remaining: create the empty
level and save it as `Content/Maps/MainMap`; see
[EDITOR_SETUP.md](EDITOR_SETUP.md) step 1. GameMode/GameInstance are already wired
project-wide via `DefaultEngine.ini`, so an empty level suffices.

---

## Additional fixes beyond the original audit list

- **Widget trees (was the biggest functional blocker):** all 11 widget classes in
  `Source/SeagullStorm/UI/` construct their full UMG trees in
  `NativeOnInitialized()` via `WidgetTree->ConstructWidget<>`, assigning every
  `BindWidgetOptional` pointer. Shared Press Start 2P font (loaded from the shipped
  TTF at runtime), palette colors and button styling live in `SeagullWidgetStyles`.
- **Title logo:** `seagull_logo.png` is imported at runtime
  (`FImageUtils::ImportFileAsTexture2D`) into a `UImage` above the title text.
- **Crash custom key `score`:** set alongside the existing `wave` (wave spawn) and
  `level` (level-up) custom keys, per plan section 6.9.
- **Map tile tint:** the dynamic tile material sets both `BaseColor` and `Color`
  vector parameters so the engine BasicShapeMaterial tints correctly.
- **Sign-out reset:** also clears the news cache (`bNewsLoaded`/`CachedNews`) in
  addition to save/config flags.
- **Pause-quit design deviation** (Quit ends the run via the score-submitting
  Game Over path instead of silently returning to the Hub) is documented in the
  README.
