# horizOn Example — Unreal Engine

> **Status: Under Construction**
> This project is actively being developed. Screenshots and a playable demo will be added soon.

**Seagull Storm** is a mini Vampire Survivors-style roguelike built with Unreal Engine 5. It serves as a comprehensive example project demonstrating all 9 [horizOn](https://horizon.pm) SDK features in a real, playable game.

## Features Demonstrated

| # | horizOn Feature | In-Game Usage |
|---|----------------|---------------|
| 1 | **Authentication** | Guest and Email sign-in/sign-up on title screen (Google/Apple buttons are shown but unavailable on desktop, see below) |
| 2 | **Leaderboards** | Score submission, Top 10 display, player rank |
| 3 | **Cloud Save** | Persistent coins, upgrades, highscore across sessions |
| 4 | **Remote Config** | All game balancing (enemies, weapons, upgrades, wave timing) |
| 5 | **News** | In-game news feed in hub and pause menu |
| 6 | **Gift Codes** | Code redemption for coin rewards |
| 7 | **Feedback** | Bug reports and feature requests from in-game |
| 8 | **User Logs** | Aggregated run summary logged at game over |
| 9 | **Crash Reporting** | Session tracking, breadcrumbs, exception capture |

## About the Game

You play as a seagull on a beach, surviving waves of crabs, jellyfish, and pirate seagulls. Auto-attack with upgradeable weapons, collect XP shells to level up, and try to survive the final boss — a giant octopus.

- **Genre:** Vampire Survivors-style auto-attack roguelike
- **Session Length:** 3–5 minutes
- **Art Style:** Pixel art (32x32 sprites), placeholder graphics included
- **Font:** Press Start 2P

## Getting Started

### Step 1 — Clone and Open

1. Clone this repository
2. Open the `.uproject` file with **Unreal Engine 5.5**
3. Follow [EDITOR_SETUP.md](EDITOR_SETUP.md) to create the editor-only assets
   (map, audio imports, sprites/flipbooks); all code is C++ and needs no editor work

### Step 2 — Create a horizOn Account and API Key

1. Go to [horizon.pm](https://horizon.pm) and create a free account
2. Open the **Dashboard** and create a new project
3. Navigate to **Settings > API Keys** and generate an API key
4. Copy the API key

### Step 3 — Configure the SDK

The horizOn SDK is already included as a plugin at `Plugins/HorizonSDK/`. It is
configured through `Config/DefaultGame.ini` (section `[/Script/HorizonSDK.HorizonConfig]`).

1. Make sure the plugin is enabled in **Edit > Plugins > horizOn SDK**
2. Go to **Edit > Project Settings > horizOn SDK** and paste your API key
   (this writes it into `Config/DefaultGame.ini`)
3. Alternatively, edit `Config/DefaultGame.ini` directly and replace
   `ApiKey=YOUR_API_KEY` with your key; the regional `Hosts` list is already set up

### Step 4 — Set Up Remote Config (Optional)

The game works out of the box with built-in defaults. To customize the game balance, set up Remote Config variables in the horizOn Dashboard under **Remote Config**. See the [Remote Config Reference](#remote-config-reference) below for all available keys.

### Step 5 — Run

Press **Play** in the Unreal editor.

### Design Deviation: Quit from the Pause Menu

The shared design doc says "Quit -> returns to Hub, counts as death for scoring".
This implementation deliberately deviates: **Quit Run** ends the run via the normal
end-of-run path (`EndRun(false)`): the score still counts and is submitted, the run
does not count as a death, and the player lands on the Game Over screen instead of
jumping straight to the Hub. This keeps every run's score submission consistent.

### Design Deviation: Title-Screen Auth Buttons

The design layout shows four auth buttons (Guest, Google, Email, Create Account);
this implementation shows five. A **Sign In with Apple** button is added because the
Unreal SDK ships a drop-in Apple flow (native sheet on iOS). On desktop platforms
both the Google and the Apple button report "not available on this platform" on the
status line: the SDK's Google flow requires a pre-obtained OAuth authorization code
(there is no desktop browser flow), and the Apple flow no-ops outside iOS.

### Design Deviation: Hub Leaderboard Shows Top 10 Only

The design's Main Hub layout shows the player's own rank row below the top list.
This implementation shows only the Top 10 in the hub; the player's own rank is
fetched and shown on the Game Over screen instead (right after the score submit),
which keeps the hub at its fixed request budget. The Run HUD also omits the design's
percentage label on the XP bar.

## Remote Config Reference

All values are optional — the game ships with sensible built-in defaults. Set these in the horizOn Dashboard under **Remote Config** to customize the game balance without updating the client.

### General

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `run_duration_seconds` | int | `180` | Duration of a survival run in seconds before the boss spawns |
| `boss_wave_enabled` | bool | `true` | Whether a boss wave spawns when the timer runs out |
| `coin_divisor` | int | `10` | Score is divided by this value to calculate coins earned |
| `xp_per_kill_base` | int | `10` | Base XP unit for the level-up curve (the first level-up needs 5x this value) |
| `xp_level_curve` | float | `1.4` | XP-to-next-level scaling exponent (higher = steeper curve) |

### Wave Spawning

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `wave_interval_seconds` | float | `15.0` | Seconds between enemy waves |
| `wave_enemy_count_base` | int | `5` | Number of enemies in the first wave |
| `wave_enemy_count_growth` | float | `1.3` | Enemy count multiplier per wave (e.g. 1.3 = +30% each wave) |
| `wave_boss_hp` | int | `500` | Boss hit points |

### Enemy Stats

Each enemy type (`crab`, `jellyfish`, `pirate`) has four config keys following the pattern `enemy_{type}_{stat}` with per-type defaults:

| Type | `enemy_{type}_hp` (int) | `enemy_{type}_speed` (float) | `enemy_{type}_damage` (int) | `enemy_{type}_xp` (int) |
|------|------------------------|------------------------------|-----------------------------|--------------------------|
| `crab` | `30` | `40.0` | `10` | `10` |
| `jellyfish` | `50` | `60.0` | `15` | `20` |
| `pirate` | `40` | `100.0` | `20` | `25` |

`hp` = hit points, `speed` = movement speed (units/sec), `damage` = melee attack damage, `xp` = XP dropped on death. The boss has no `enemy_boss_*` keys; its hit points come from `wave_boss_hp` above.

**Example keys:** `enemy_crab_hp`, `enemy_jellyfish_speed`, `enemy_pirate_damage`

### Weapon Stats

Each weapon type (`feather`, `screech`, `dive`, `gust`) has `weapon_{type}_{stat}` keys; not every stat exists for every weapon:

| Weapon | `_damage` (float) | `_cooldown` (float, sec) | Extra key | Extra default |
|--------|-------------------|--------------------------|-----------|---------------|
| `feather` | `20.0` | `0.8` | `weapon_feather_projectiles` (int) | `1` |
| `screech` | `15.0` | `2.0` | `weapon_screech_radius` (float, AoE radius) | `80.0` |
| `dive` | `50.0` | `3.0` | `weapon_dive_range` (float, dash range) | `120.0` |
| `gust` | `10.0` | `2.5` | `weapon_gust_knockback` (float, force) | `60.0` |

**Example keys:** `weapon_feather_damage`, `weapon_screech_cooldown`, `weapon_dive_range`

### Upgrade System

Each upgrade type (`speed`, `damage`, `hp`, `magnet`) has three config keys — `upgrade_{type}_max` (int), `upgrade_{type}_costs` (JSON array, coin cost to buy the NEXT level, index = current level) and `upgrade_{type}_values` (JSON array, stat value at each level, index = level):

| Upgrade | Max | Costs | Values |
|---------|-----|-------|--------|
| `speed` | `4` | `[100, 300, 600, 1000]` | `[1.0, 1.1, 1.2, 1.35, 1.5]` (multiplier) |
| `damage` | `4` | `[100, 300, 600, 1000]` | `[1.0, 1.15, 1.3, 1.5, 1.8]` (multiplier) |
| `hp` | `3` | `[150, 400, 800]` | `[100, 130, 170, 220]` (max HP) |
| `magnet` | `2` | `[200, 500]` | `[50, 80, 120]` (pickup radius) |

**Example keys:** `upgrade_speed_max`, `upgrade_damage_costs`, `upgrade_hp_values`

### Level-Up Choices

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `levelup_choices` | int | `3` | Number of choices shown on level up |
| `levelup_pool` | JSON array | *(built-in pool)* | Pool of available upgrades with weighted random selection |

**`levelup_pool` format** — each entry is an object with `id`, `type`, and `weight`:
```json
[
  {"id": "feather_dmg",   "type": "weapon_upgrade", "weight": 3},
  {"id": "feather_speed", "type": "weapon_upgrade", "weight": 2},
  {"id": "screech_new",   "type": "weapon_new",     "weight": 1},
  {"id": "dive_new",      "type": "weapon_new",     "weight": 1},
  {"id": "gust_new",      "type": "weapon_new",     "weight": 1},
  {"id": "move_speed",    "type": "stat_boost",     "weight": 2},
  {"id": "max_hp",        "type": "stat_boost",     "weight": 2},
  {"id": "xp_magnet",     "type": "stat_boost",     "weight": 1}
]
```

## Project Structure

```
Config/                     # Engine and game configuration
Content/                    # Assets (sprites, fonts, audio)
EDITOR_SETUP.md             # Editor-only asset creation steps (map, imports, flipbooks)
Plugins/
  HorizonSDK/              # horizOn SDK plugin (auto-updated)
Source/
  SeagullStorm/
    Core/                   # GameMode, GameState, GameInstance, PlayerController, types
    Player/                 # PlayerPawn, Health & XP components
    Enemies/                # EnemyBase, Crab, Jellyfish, Pirate, Boss, spawner, poison zone
    Weapons/                # WeaponBase, Feather, Screech, Dive, Gust, projectile
    Pickups/                # XP shell, coin
    UI/                     # Widget classes for all screens + shared widget styles
    Horizon/                # HorizonManager facade
    Audio/                  # AudioManager
    Map/                    # Procedural arena generator
    Data/                   # SaveData, ConfigCache
SeagullStorm.uproject       # Project file
```

## Requirements

- [Unreal Engine 5.5](https://www.unrealengine.com/)
- [horizOn Account](https://horizon.pm) (free tier works)
- [horizOn SDK for Unreal](https://github.com/ProjectMakersDE/horizOn-SDK-Unreal)

## Related Projects

- [horizOn-SDK-Unreal](https://github.com/ProjectMakersDE/horizOn-SDK-Unreal) — The SDK this example uses
- [horizOn-Example-Godot](https://github.com/ProjectMakersDE/horizOn-Example-Godot) — Same game in Godot
- [horizOn-Example-Unity](https://github.com/ProjectMakersDE/horizOn-Example-Unity) — Same game in Unity

## License

MIT
