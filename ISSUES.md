# Seagull Storm Unreal — Open Issues

These issues were found during a design-doc audit and must be fixed before the project is considered complete.

---

## General Issues (shared across all engines)

### G1: Google OAuth is a stub
**Expected:** Functional Google OAuth flow using the horizOn SDK's OAuth mechanism.
**Actual:** `OnGoogleClicked()` only logs `"Google sign-in not available in this example"`. No OAuth flow.
**Acceptance:** The Google button must trigger the SDK's Google OAuth methods. If OAuth is not available on the platform, show a proper in-game error message (not just a UE_LOG).

### G2: Pause Menu News makes a new request mid-run
**Expected:** News loaded once at hub, cached and reused in pause menu. Zero requests during run.
**Actual:** `SeagullNewsPanel.cpp` calls `HM->LoadNews()` every time it opens from pause menu.
**Acceptance:** Cache the news data loaded at hub entry on `USeagullGameInstance`. The pause menu news panel must read from this cache — zero network requests during the run.

### G3: Remote Config must only be loaded once per session
**Expected:** `getAllConfigs()` called once, cached for entire app session.
**Actual:** Currently OK (flag-based guard). But `StartCrashCapture` is called per hub visit (see UE8).
**Acceptance:** Verify config is loaded exactly once. No re-fetch on hub re-entry after a run.

---

## Unreal-Specific Issues

### UE1: All Unreal binary assets missing [CRITICAL]
**Expected:** `.umap` level files and `.uasset` files for the project to open in the editor.
**Actual:** Zero `.umap` and zero `.uasset` files exist. `Content/Maps/MainMap` referenced in DefaultEngine.ini does not exist.
**Acceptance:** This requires the Unreal Editor and cannot be fixed by code alone. Document clearly what must be created in the editor. At minimum, create the MainMap and basic level Blueprint.

### UE2: Audio assets never loaded — all USoundWave* are nullptr [CRITICAL]
**Expected:** 14 audio files loaded and assigned to AudioManager properties.
**Actual:** `SeagullAudioManager` has 14 `USoundWave*` fields that are never assigned. No `LoadObject` or `ConstructorHelpers` calls exist. All audio silently fails.
**Acceptance:** Add asset loading in `SeagullAudioManager::Initialize()` using `LoadObject<USoundWave>()` with `/Game/Audio/...` paths. All 14 sounds must be loadable at runtime. Alternatively, mark them as `UPROPERTY(EditDefaultsOnly)` for Blueprint assignment.

### UE3: PaperFlipbook assets missing [CRITICAL]
**Expected:** Paper2D Flipbooks for all character animations.
**Actual:** Only raw PNG files exist in `Content/Textures/`. No Paper2D Sprites or Flipbooks.
**Acceptance:** This requires the Unreal Editor. Document the required Flipbooks: Player (Idle, Walk, Hurt, Death), each enemy type (Walk, Death), Boss (Idle, Death), weapon effects, pickups.

### UE4: Pause Menu News re-fetches mid-run [MEDIUM]
**Expected:** Zero requests during run.
**Actual:** `SeagullNewsPanel.cpp` calls `HM->LoadNews()` from pause.
**Acceptance:** Add a `TArray<FHorizonNewsEntry> CachedNews` to `USeagullGameInstance`. Populate at hub load. Read from cache in pause menu news panel.

### UE5: Gift Code reward hardcoded to 500 coins [MEDIUM]
**Expected:** Reward parsed from server response.
**Actual:** `SeagullGiftCodePanel.cpp` line 57: `GI->SaveData.Coins += 500` regardless of server response.
**Acceptance:** Parse the `GiftData` string returned by `RedeemGiftCode` to extract the actual coin reward. Apply the parsed amount, not a hardcoded 500.

### UE6: Missing breadcrumb for hub upgrade purchases [LOW]
**Expected:** `recordBreadcrumb("user_action", "bought_speed_3")` on each hub upgrade purchase.
**Actual:** `SeagullMainHub.cpp` `TryBuyUpgrade()` has no breadcrumb call.
**Acceptance:** Add `HM->RecordBreadcrumb(TEXT("user_action"), FString::Printf(TEXT("bought_%s_%d"), *Key, NewLevel))` after a successful upgrade purchase.

### UE7: Music crossfade not implemented [LOW]
**Expected:** 0.5s crossfade between music tracks.
**Actual:** `CrossfadeToTrack()` instantly switches tracks. `MusicCrossfadeDuration = 0.5f` is defined but never used.
**Acceptance:** Implement actual crossfade using `UAudioComponent::FadeOut(0.5f)` on the current track and a timed `FadeIn` on the new track.

### UE8: StartCrashCapture called on every hub visit [LOW]
**Expected:** `registerSession()` called once at app start.
**Actual:** `HM->StartCrashCapture()` is in `LoadHubData()` which fires on every hub construction.
**Acceptance:** Move `StartCrashCapture()` to `SeagullStormGameMode::BeginPlay()` after session restore. Guard with a `bCrashCaptureStarted` flag.

### UE9: Jellyfish poison zone not implemented [LOW]
**Expected:** Jellyfish "leaves poison zone" on death.
**Actual:** `SeagullEnemy_Jellyfish.cpp` `OnDeath()` has only a comment: "Could spawn poison zone here in future."
**Acceptance:** Spawn a damage-over-time zone actor on jellyfish death. The zone should persist for 2-3 seconds and deal damage to the player if they walk through it.

### UE10: Gamepad/Joystick input missing [LOW]
**Expected:** "WASD / Joystick" movement per design doc.
**Actual:** `SeagullPlayerController.cpp` only maps WASD keyboard keys.
**Acceptance:** Add gamepad left-stick bindings to the Enhanced Input mapping context.

### UE11: Feedback category FEATURE vs FEATURE_REQUEST [LOW]
**Expected:** Category `FEATURE_REQUEST`.
**Actual:** `SeagullFeedbackForm.cpp` uses `FEATURE`.
**Acceptance:** Change to `FEATURE_REQUEST` to match the design doc and other engine implementations.

### UE12: Magnet level missing from User Log string [LOW]
**Expected:** Log includes all 4 upgrade levels: `speed:3,dmg:1,hp:2,magnet:0`.
**Actual:** Log only includes speed, dmg, hp — magnet is omitted.
**Acceptance:** Add magnet upgrade level to the log format string.

### UE13: Default Map MainMap.umap does not exist [CRITICAL]
**Expected:** A playable level at `Content/Maps/MainMap`.
**Actual:** File does not exist. `DefaultEngine.ini` references it.
**Acceptance:** Create the map in Unreal Editor (see UE1).
