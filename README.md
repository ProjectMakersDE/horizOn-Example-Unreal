# horizOn Example — Unreal Engine

> **Status: Under Construction**
> This project is actively being developed. Screenshots and a playable demo will be added soon.

**Seagull Storm** is a mini Vampire Survivors-style roguelike built with Unreal Engine 5. It serves as a comprehensive example project demonstrating all 9 [horizOn](https://horizon.pm) SDK features in a real, playable game.

## Features Demonstrated

| # | horizOn Feature | In-Game Usage |
|---|----------------|---------------|
| 1 | **Authentication** | Guest, Google, Email sign-in/sign-up on title screen |
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
2. Open the `.uproject` file with **Unreal Engine 5.4+**

### Step 2 — Create a horizOn Account and API Key

1. Go to [horizon.pm](https://horizon.pm) and create a free account
2. Open the **Dashboard** and create a new project
3. Navigate to **Settings > API Keys** and generate an API key
4. Download the config JSON file — it contains your `apiKey` and `backendUrl`

### Step 3 — Import the Config into the SDK

The horizOn SDK is already included as a plugin at `Plugins/HorizonSDK/`.

1. Make sure the plugin is enabled in **Edit > Plugins > horizOn SDK**
2. Go to **Edit > Project Settings > horizOn** and paste your API key and backend URL
3. Alternatively, place the downloaded config JSON at `Config/horizon_config.json`

### Step 4 — Set Up Remote Config (Optional)

The game works out of the box with built-in defaults. To customize the game balance, set up Remote Config variables in the horizOn Dashboard under **Remote Config**. See the [Remote Config Reference](#remote-config-reference) below for all available keys.

### Step 5 — Run

Press **Play** in the Unreal editor.

## Remote Config Reference

All values are optional — the game ships with sensible built-in defaults. Set these in the horizOn Dashboard under **Remote Config** to customize the game balance without updating the client.

### General

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `run_duration_seconds` | float | `180.0` | Duration of a survival run in seconds before the boss spawns |
| `boss_wave_enabled` | bool | `true` | Whether a boss wave spawns when the timer runs out |
| `coin_divisor` | int | `10` | Score is divided by this value to calculate coins earned |
| `xp_level_curve` | float | `1.4` | XP-to-next-level scaling exponent (higher = steeper curve) |

### Wave Spawning

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `wave_interval_seconds` | float | `15.0` | Seconds between enemy waves |
| `wave_enemy_count_base` | int | `5` | Number of enemies in the first wave |
| `wave_enemy_count_growth` | float | `1.3` | Enemy count multiplier per wave (e.g. 1.3 = +30% each wave) |
| `wave_boss_hp` | float | `500.0` | Boss hit points (overrides `enemy_boss_hp`) |

### Enemy Stats

Each enemy type (`crab`, `jellyfish`, `pirate`) has four config keys following the pattern `enemy_{type}_{stat}`:

| Key Pattern | Type | Default | Description |
|-------------|------|---------|-------------|
| `enemy_{type}_hp` | int | `30` | Hit points |
| `enemy_{type}_speed` | float | `40.0` | Movement speed (units/sec) |
| `enemy_{type}_damage` | int | `10` | Melee attack damage |
| `enemy_{type}_xp` | int | `10` | XP dropped on death |

**Example keys:** `enemy_crab_hp`, `enemy_jellyfish_speed`, `enemy_pirate_damage`

### Weapon Stats

Each weapon type (`feather`, `screech`, `dive`, `gust`) has config keys following the pattern `weapon_{type}_{stat}`:

| Key Pattern | Type | Default | Description |
|-------------|------|---------|-------------|
| `weapon_{type}_damage` | float | `20.0` | Base damage per hit |
| `weapon_{type}_cooldown` | float | `1.0` | Seconds between attacks |
| `weapon_{type}_projectiles` | int | `1` | Number of projectiles (feather only) |
| `weapon_{type}_radius` | float | `80.0` | AoE radius (screech only) |
| `weapon_{type}_range` | float | `120.0` | Dash range (dive only) |
| `weapon_{type}_knockback` | float | `60.0` | Knockback force (gust only) |

**Example keys:** `weapon_feather_damage`, `weapon_screech_cooldown`, `weapon_dive_range`

### Upgrade System

Each upgrade type (`speed`, `damage`, `hp`, `magnet`) has three config keys:

| Key Pattern | Type | Default | Description |
|-------------|------|---------|-------------|
| `upgrade_{type}_max` | int | `5` | Maximum upgrade level |
| `upgrade_{type}_costs` | JSON array | `[10, 25, 50, 100, 200]` | Coin cost per level (array index = level) |
| `upgrade_{type}_values` | JSON array | *(see below)* | Stat value at each level (array index = level) |

**Default upgrade values:**

| Upgrade | Values (level 0–5) |
|---------|---------------------|
| `speed` | `[1.0, 1.1, 1.2, 1.3, 1.4, 1.5]` (multiplier) |
| `damage` | `[1.0, 1.15, 1.3, 1.5, 1.75, 2.0]` (multiplier) |
| `hp` | `[100, 120, 140, 170, 200, 250]` (max HP) |
| `magnet` | `[50, 65, 80, 100, 120, 150]` (pickup radius) |

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
Plugins/
  HorizonSDK/              # horizOn SDK plugin (auto-updated)
Source/
  SeagullStorm/
    Core/                   # GameMode, GameState, GameInstance
    Player/                 # PlayerPawn, PlayerController
    Enemies/                # EnemyBase, Crab, Jellyfish, Pirate, Boss
    Weapons/                # WeaponBase, Feather, Screech, Dive, Gust
    Spawning/               # WaveSpawner, XPShell
    UI/                     # Widget classes for all screens
    Horizon/                # HorizonManager facade
    Audio/                  # AudioManager
    Data/                   # GameData, RunState structs
SeagullStorm.uproject       # Project file
```

## Requirements

- [Unreal Engine 5.4+](https://www.unrealengine.com/)
- [horizOn Account](https://horizon.pm) (free tier works)
- [horizOn SDK for Unreal](https://github.com/ProjectMakersDE/horizOn-SDK-Unreal)

## Related Projects

- [horizOn-SDK-Unreal](https://github.com/ProjectMakersDE/horizOn-SDK-Unreal) — The SDK this example uses
- [horizOn-Example-Godot](https://github.com/ProjectMakersDE/horizOn-Example-Godot) — Same game in Godot
- [horizOn-Example-Unity](https://github.com/ProjectMakersDE/horizOn-Example-Unity) — Same game in Unity

## License

MIT
