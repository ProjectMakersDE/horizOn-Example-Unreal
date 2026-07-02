# Unreal Editor Setup: Seagull Storm

All game logic, UI, input and styling are pure C++ and need no editor work. What C++
cannot create are **binary assets** (`.umap`, imported `.uasset` files). This document
lists every editor step required to make the project fully playable, in order. The
`.gitignore` no longer excludes `*.uasset`/`*.umap`, so commit the results of each step.

The game is fully null-guarded: it runs after step 1 alone (silent, with invisible
actors), and each further step adds audio and visuals.

**Before you start:** set your horizOn API key first (see [README.md](README.md) steps
2-3). `Config/DefaultGame.ini` still ships `ApiKey=YOUR_API_KEY`; without a real key
every auth call fails and the screen flow stalls at the Title screen, which shows a
"Sign-in failed" status instead of advancing to the Hub.

---

## 1. Create the map: `Content/Maps/MainMap.umap`

`Config/DefaultEngine.ini` already wires `GameDefaultMap=/Game/Maps/MainMap`,
`GlobalDefaultGameMode=SeagullStormGameMode` and `GameInstanceClass=SeagullGameInstance`,
so the map itself can stay completely empty: no Level Blueprint, no placed actors.

1. Open `SeagullStorm.uproject` with Unreal Engine 5.5 (let it compile the C++ module).
2. **File > New Level... > Empty Level**.
3. **File > Save Current Level As...**, create the folder `Maps` under `Content`, name the
   level `MainMap`, save.

**Expected result:** Pressing **Play** boots into the Title screen (dark background,
seagull logo, "SEAGULL STORM", name input, five auth buttons — Guest, Google, Apple,
Email, Create Account; Google/Apple are desktop stubs, see the README deviation). With a
configured API key the whole screen flow (Title -> Hub -> Run -> Game Over) works;
gameplay actors are still invisible and the game is silent until steps 2-4 are done.

## 2. Import the 14 audio files

`SeagullAudioManager::Initialize()` loads these by path, so the asset names must equal the
file stems (the default when importing).

1. In the Content Browser create the folders `Audio/Music` and `Audio/SFX` under `Content`.
2. Drag `Content/Audio/Music/music_menu.ogg`, `music_battle.ogg`, `music_boss.ogg`
   (from the repo folder on disk) into `/Game/Audio/Music`.
3. Drag the 11 SFX files (`sfx_feather.ogg`, `sfx_screech.ogg`, `sfx_dive.ogg`,
   `sfx_gust.ogg`, `sfx_player_hit.ogg`, `sfx_enemy_hit.ogg`, `sfx_enemy_attack.ogg`,
   `sfx_pickup_xp.ogg`, `sfx_levelup.ogg`, `sfx_upgrade_select.ogg`, `sfx_game_over.ogg`)
   into `/Game/Audio/SFX`.
4. Save all (**Ctrl+Shift+S**).

**Expected result:** The output log shows
`AudioManager initialized — loaded 14/14 audio assets` on the next Play. Menu music
plays on the title screen, battle/boss music during runs, SFX on every trigger.

## 3. Import the sprite sheets with pixel-art settings

1. Create the folder `Textures` under `Content` in the Content Browser and drag in
   `seagull.png`, `enemies.png`, `weapons.png`, `pickups.png` from
   `Content/Textures/` on disk. (`tilemap.png` and `ui.png` are currently not consumed
   by the Unreal code; the arena uses colored planes and the UI uses flat color
   styling, importing them is optional.)
2. Select all imported textures, right-click > **Sprite Actions > Apply Paper2D Texture
   Settings**. Then verify in each texture:
   - Compression Settings: `UserInterface2D (RGBA)`
   - Mip Gen Settings: `NoMipmaps`
   - Filter: `Nearest`
   - sRGB: enabled
3. Save all.

**Expected result:** Textures render crisp (no smoothing) when zoomed in the viewer.

## 4. Extract sprites and create the Flipbooks

The C++ constructors load flipbooks by these **exact** names from `/Game/Flipbooks`.
Create the folder `Flipbooks` under `Content` first. Sprite asset names do not matter;
only the flipbook names do.

Extraction (right-click a texture > **Sprite Actions > Extract Sprites**, mode `Grid`):

- `seagull.png` (192x128): cell 32x32 -> 4 rows: row 0 = Idle (4 frames), row 1 = Walk
  (6), row 2 = Hurt (2), row 3 = Death (4). Empty cells produce no sprites.
- `enemies.png` (256x320): two passes. Pass 1: cell 32x32, grid height 192 (rows 0-5).
  Pass 2: cell 64x64, Y offset 192 (boss rows). Rows: 0 Crab Walk (4), 1 Crab Death (3),
  2 Jellyfish Idle (4), 3 Jellyfish Death (3), 4 Pirate Walk (4), 5 Pirate Death (3),
  then Boss Idle (4) and Boss Death (4) at 64x64.
- `weapons.png` (128x128): cell 32x32 -> row 0 Feather (4), row 1 Screech (4),
  row 2 Dive (4), row 3 Gust (4).
- `pickups.png` (64x48): cell 16x16 -> row 0 XP Shell (4), row 1 Coin (4),
  row 2 Health (2).

Then select the frames of one animation (in sheet order, left to right), right-click >
**Create Flipbook**, move it to `/Game/Flipbooks`, rename exactly, and set
`Frames Per Second` (loop behavior is controlled at runtime by the code):

| Flipbook name | Source | Row | Frames | FPS |
|---|---|---|---|---|
| `FB_Seagull_Idle` | seagull.png | 0 | 4 | 8 |
| `FB_Seagull_Walk` | seagull.png | 1 | 6 | 10 |
| `FB_Seagull_Hurt` | seagull.png | 2 | 2 | 10 |
| `FB_Seagull_Death` | seagull.png | 3 | 4 | 8 |
| `FB_Crab_Walk` | enemies.png | 0 | 4 | 8 |
| `FB_Crab_Death` | enemies.png | 1 | 3 | 8 |
| `FB_Jellyfish_Idle` | enemies.png | 2 | 4 | 8 |
| `FB_Jellyfish_Death` | enemies.png | 3 | 3 | 8 |
| `FB_Pirate_Walk` | enemies.png | 4 | 4 | 10 |
| `FB_Pirate_Death` | enemies.png | 5 | 3 | 8 |
| `FB_Boss_Idle` | enemies.png | 6 (64x64) | 4 | 6 |
| `FB_Boss_Death` | enemies.png | 7 (64x64) | 4 | 6 |
| `FB_Feather` | weapons.png | 0 | 4 | 12 |
| `FB_XPShell` | pickups.png | 0 | 4 | 8 |
| `FB_Coin` | pickups.png | 1 | 4 | 8 |

Optional (defined in the asset plan, not yet consumed by code): `FB_Screech`
(weapons row 1), `FB_Dive` (row 2), `FB_Gust` (row 3), `FB_Health` (pickups row 2).

**Expected result:** On the next Play the seagull is visible and switches Idle/Walk
while moving, flashes Hurt on damage and plays Death on dying; crabs/jellyfish/pirates/
boss are visible, walk-animated and play their death animation before despawning;
feather projectiles and XP shells are visible.

## 5. Commit the assets

```
git add Content/Maps Content/Audio Content/Textures Content/Flipbooks
git commit -m "feat(assets): editor-created map, audio imports, sprites and flipbooks"
```

**Expected result:** `git status` is clean; a fresh clone opens fully playable.
