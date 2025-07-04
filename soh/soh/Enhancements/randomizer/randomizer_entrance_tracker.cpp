#include "randomizer_entrance_tracker.h"
#include "soh/OTRGlobals.h"
#include "soh/cvar_prefixes.h"
#include "soh/SohGui/SohGui.hpp"

#include <map>
#include <string>
#include <vector>
#include <libultraship/libultraship.h>

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;

#include "soh/Enhancements/randomizer/randomizer_entrance.h"
#include "soh/Enhancements/randomizer/randomizer_grotto.h"
#include "soh/Enhancements/randomizer/randomizerTypes.h"
}

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "entrance.h"

#define COLOR_ORANGE IM_COL32(230, 159, 0, 255)
#define COLOR_GREEN IM_COL32(0, 158, 115, 255)
#define COLOR_GRAY IM_COL32(155, 155, 155, 255)

EntranceOverride srcListSortedByArea[ENTRANCE_OVERRIDES_MAX_COUNT] = { 0 };
EntranceOverride destListSortedByArea[ENTRANCE_OVERRIDES_MAX_COUNT] = { 0 };
EntranceOverride srcListSortedByType[ENTRANCE_OVERRIDES_MAX_COUNT] = { 0 };
EntranceOverride destListSortedByType[ENTRANCE_OVERRIDES_MAX_COUNT] = { 0 };
EntranceTrackingData gEntranceTrackingData = { 0 };

static const EntranceOverride emptyOverride = { 0 };

static s16 lastEntranceIndex = -1;
static s16 currentGrottoId = -1;
static s16 lastSceneOrEntranceDetected = -1;

static bool presetLoaded = false;
static ImVec2 presetPos;
static ImVec2 presetSize;

static std::string spoilerEntranceGroupNames[] = {
    "Spawns/Warp Songs/Owls",
    "Kokiri Forest",
    "Lost Woods",
    "Sacred Forest Meadow",
    "Kakariko Village",
    "Graveyard",
    "Death Mountain Trail",
    "Death Mountain Crater",
    "Goron City",
    "Zora's River",
    "Zora's Domain",
    "Zora's Fountain",
    "Hyrule Field",
    "Lon Lon Ranch",
    "Lake Hylia",
    "Gerudo Valley",
    "Gerudo Fortress",
    "Haunted Wasteland",
    "Desert Colossus",
    "Market",
    "Hyrule Castle",
};

static std::string groupTypeNames[] = {
    "One Way", "Overworld", "Interior", "Grotto", "Dungeon",
};

// Entrance data for the tracker taken from the 3ds rando entrance tracker, and supplemented with scene/spawn info and
// meta search tags ENTR_HYRULE_FIELD_10 and ENTR_POTION_SHOP_KAKARIKO_1 have been repurposed for entrance randomizer
const EntranceData entranceData[] = {
    // clang-format off
    //index,                reverse, scenes (and spawns),     source name,   destination name, source group,           destination group,      type,                 metaTag, oneExit
    { ENTR_LINKS_HOUSE_CHILD_SPAWN,   -1,      SINGLE_SCENE_INFO(SCENE_LINKS_HOUSE), "Child Spawn", "Link's House",   ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_HYRULE_FIELD_10, -1,      SINGLE_SCENE_INFO(SCENE_TEMPLE_OF_TIME), "Adult Spawn", "Temple of Time", ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},

    { ENTR_SACRED_FOREST_MEADOW_WARP_PAD,  -1, {{ -1 }}, "Minuet of Forest",   "SFM Warp Pad",              ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_DEATH_MOUNTAIN_CRATER_WARP_PAD, -1, {{ -1 }}, "Bolero of Fire",     "DMC Warp Pad",              ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_LAKE_HYLIA_WARP_PAD,            -1, {{ -1 }}, "Serenade of Water",  "Lake Hylia Warp Pad",       ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_DESERT_COLOSSUS_WARP_PAD,       -1, {{ -1 }}, "Requiem of Spirit",  "Desert Colossus Warp Pad",  ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_GRAVEYARD_WARP_PAD,             -1, {{ -1 }}, "Nocturne of Shadow", "Graveyard Warp Pad",        ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_TEMPLE_OF_TIME_WARP_PAD,        -1, {{ -1 }}, "Prelude of Light",   "Temple of Time Warp Pad",   ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},

    { ENTR_KAKARIKO_VILLAGE_OWL_DROP, -1, SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "DMT Owl Flight", "Kakariko Village Owl Drop", ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},
    { ENTR_HYRULE_FIELD_OWL_DROP,     -1, SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),           "LH Owl Flight",  "Hyrule Field Owl Drop",     ENTRANCE_GROUP_ONE_WAY, ENTRANCE_GROUP_ONE_WAY, ENTRANCE_TYPE_ONE_WAY},

    // Kokiri Forest
    { ENTR_LOST_WOODS_BRIDGE_EAST_EXIT,              ENTR_KOKIRI_FOREST_LOWER_EXIT,                 SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "Kokiri Forest Lower Exit",     "Lost Woods Bridge East Exit", ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTR_LOST_WOODS_SOUTH_EXIT,                    ENTR_KOKIRI_FOREST_UPPER_EXIT,                 SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "Kokiri Forest Upper Exit",     "Lost Woods South Exit",       ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTR_LINKS_HOUSE_1,                            ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE,        SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Link's House Entry",        "Link's House",                ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_MIDOS_HOUSE_0,                            ENTR_KOKIRI_FOREST_OUTSIDE_MIDOS_HOUSE,        SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Mido's House Entry",        "Mido's House",                ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_SARIAS_HOUSE_0,                           ENTR_KOKIRI_FOREST_OUTSIDE_SARIAS_HOUSE,       SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Saria's House Entry",       "Saria's House",               ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_TWINS_HOUSE_0,                            ENTR_KOKIRI_FOREST_OUTSIDE_TWINS_HOUSE,        SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF House of Twins Entry",      "House of Twins",              ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_KNOW_IT_ALL_BROS_HOUSE_0,                 ENTR_KOKIRI_FOREST_OUTSIDE_KNOW_IT_ALL_HOUSE,  SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Know-It-All House Entry",   "Know-It-All House",           ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_KOKIRI_SHOP_0,                            ENTR_KOKIRI_FOREST_OUTSIDE_SHOP,               SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Shop Entry",                "Kokiri Shop",                 ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_KF_STORMS_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_KF_STORMS_OFFSET), SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Storms Grotto Entry",       "KF Storms Grotto",            ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTR_DEKU_TREE_ENTRANCE,                       ENTR_KOKIRI_FOREST_OUTSIDE_DEKU_TREE,          SINGLE_SCENE_INFO(SCENE_KOKIRI_FOREST),          "KF Outside Deku Tree",         "Deku Tree Entrance",          ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE,        ENTR_LINKS_HOUSE_1,                            SINGLE_SCENE_INFO(SCENE_LINKS_HOUSE),            "Link's House",                 "KF Link's House Entry",       ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  ""},
    { ENTR_KOKIRI_FOREST_OUTSIDE_MIDOS_HOUSE,        ENTR_MIDOS_HOUSE_0,                            SINGLE_SCENE_INFO(SCENE_MIDOS_HOUSE),            "Mido's House",                 "KF Mido's House Entry",       ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  ""},
    { ENTR_KOKIRI_FOREST_OUTSIDE_SARIAS_HOUSE,       ENTR_SARIAS_HOUSE_0,                           SINGLE_SCENE_INFO(SCENE_SARIAS_HOUSE),           "Saria's House",                "KF Saria's House Entry",      ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  ""},
    { ENTR_KOKIRI_FOREST_OUTSIDE_TWINS_HOUSE,        ENTR_TWINS_HOUSE_0,                            SINGLE_SCENE_INFO(SCENE_TWINS_HOUSE),            "House of Twins",               "KF House of Twins Entry",     ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  ""},
    { ENTR_KOKIRI_FOREST_OUTSIDE_KNOW_IT_ALL_HOUSE,  ENTR_KNOW_IT_ALL_BROS_HOUSE_0,                 SINGLE_SCENE_INFO(SCENE_KNOW_IT_ALL_BROS_HOUSE), "Know-It-All House",            "KF Know-It-All House Entry",  ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  ""},
    { ENTR_KOKIRI_FOREST_OUTSIDE_SHOP,               ENTR_KOKIRI_SHOP_0,                            SINGLE_SCENE_INFO(SCENE_KOKIRI_SHOP),            "Kokiri Shop",                  "KF Shop Entry",               ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_INTERIOR,  ""},
    { ENTRANCE_GROTTO_EXIT(GROTTO_KF_STORMS_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_KF_STORMS_OFFSET), {{ SCENE_GROTTOS, 0x00 }},                       "KF Storms Grotto",             "KF Storms Grotto Entry",      ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_GROTTO,    "chest"},
    { ENTR_KOKIRI_FOREST_OUTSIDE_DEKU_TREE,          ENTR_DEKU_TREE_ENTRANCE,                       SINGLE_SCENE_INFO(SCENE_DEKU_TREE),              "Deku Tree Entrance",           "KF Outside Deku Tree",        ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_DUNGEON,   ""},
    { ENTR_DEKU_TREE_BOSS_ENTRANCE,                  ENTR_DEKU_TREE_BOSS_DOOR,                      SINGLE_SCENE_INFO(SCENE_DEKU_TREE),              "Deku Tree Boss Door",          "Gohma",                       ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_DEKU_TREE_BOSS_DOOR,                      ENTR_DEKU_TREE_BOSS_ENTRANCE,                  SINGLE_SCENE_INFO(SCENE_DEKU_TREE_BOSS),         "Gohma",                        "Deku Tree Boss Door",         ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_KOKIRI_FOREST_DEKU_TREE_BLUE_WARP,        -1,                                            SINGLE_SCENE_INFO(SCENE_DEKU_TREE_BOSS),         "Gohma Blue Warp",              "Deku Tree Blue Warp",         ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_ONE_WAY,   "bw", 1},

    // Lost Woods
    { ENTR_KOKIRI_FOREST_LOWER_EXIT,                         ENTR_LOST_WOODS_BRIDGE_EAST_EXIT,                      SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "Lost Woods Bridge East Exit",    "Kokiri Forest Lower Exit",         ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTR_HYRULE_FIELD_WOODED_EXIT,                         ENTR_LOST_WOODS_BRIDGE_WEST_EXIT,                      SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "Lost Woods Bridge West Exit",    "Hyrule Field Wooded Exit",         ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_OVERWORLD, "lw,hf"},
    { ENTR_KOKIRI_FOREST_UPPER_EXIT,                         ENTR_LOST_WOODS_SOUTH_EXIT,                            SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "Lost Woods South Exit",          "Kokiri Forest Upper Exit",         ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_KOKIRI_FOREST, ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTR_GORON_CITY_TUNNEL_SHORTCUT,                       ENTR_LOST_WOODS_TUNNEL_SHORTCUT,                       SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "Lost Woods Tunnel Shortcut",     "Goron City Tunnel Shortcut",       ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_GORON_CITY,    ENTRANCE_TYPE_OVERWORLD, "lw,gc"},
    { ENTR_ZORAS_RIVER_UNDERWATER_SHORTCUT,                  ENTR_LOST_WOODS_UNDERWATER_SHORTCUT,                   SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "Lost Woods Underwater Shortcut", "Zora's River Underwater Shortcut", ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_ZORAS_RIVER,   ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTR_SACRED_FOREST_MEADOW_SOUTH_EXIT,                  ENTR_LOST_WOODS_NORTH_EXIT,                            SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "Lost Woods North Exit",          "Sacred Forest Meadow South Exit",  ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_SFM,           ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTRANCE_GROTTO_LOAD(GROTTO_LW_NEAR_SHORTCUTS_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_LW_NEAR_SHORTCUTS_OFFSET), SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "LW Tunnel Grotto Entry",         "LW Tunnel Grotto",                 ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_GROTTO,    "lw,chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_LW_SCRUBS_OFFSET),         ENTRANCE_GROTTO_EXIT(GROTTO_LW_SCRUBS_OFFSET),         SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "LW North Grotto Entry",          "LW Scrubs Grotto",                 ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_GROTTO,    "lw", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_LW_DEKU_THEATRE_OFFSET),   ENTRANCE_GROTTO_EXIT(GROTTO_LW_DEKU_THEATRE_OFFSET),   SINGLE_SCENE_INFO(SCENE_LOST_WOODS), "LW Meadow Grotto Entry",         "Deku Theater",                     ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_GROTTO,    "lw,mask,stage", 1},
    { ENTRANCE_GROTTO_EXIT(GROTTO_LW_NEAR_SHORTCUTS_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_LW_NEAR_SHORTCUTS_OFFSET), {{ SCENE_GROTTOS, 0x00 }},           "LW Tunnel Grotto",               "LW Tunnel Grotto Entry",           ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_GROTTO,    "lw,chest"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_LW_SCRUBS_OFFSET),         ENTRANCE_GROTTO_LOAD(GROTTO_LW_SCRUBS_OFFSET),         {{ SCENE_GROTTOS, 0x07 }},           "LW Scrubs Grotto",               "LW North Grotto Entry",            ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_GROTTO,    "lw"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_LW_DEKU_THEATRE_OFFSET),   ENTRANCE_GROTTO_LOAD(GROTTO_LW_DEKU_THEATRE_OFFSET),   {{ SCENE_GROTTOS, 0x0C }},           "Deku Theater",                   "LW Meadow Grotto Entry",           ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_GROTTO,    "lw,mask,stage"},

    // Sacred Forest Meadow
    { ENTR_LOST_WOODS_NORTH_EXIT,                        ENTR_SACRED_FOREST_MEADOW_SOUTH_EXIT,           SINGLE_SCENE_INFO(SCENE_SACRED_FOREST_MEADOW), "Sacred Forest Meadow South Exit",            "Lost Woods North Exit",                      ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_LOST_WOODS, ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTRANCE_GROTTO_LOAD(GROTTO_SFM_WOLFOS_OFFSET),    ENTRANCE_GROTTO_EXIT(GROTTO_SFM_WOLFOS_OFFSET), SINGLE_SCENE_INFO(SCENE_SACRED_FOREST_MEADOW), "SFM Wolfos Grotto Entry",                    "SFM Wolfos Grotto",                          ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_SFM_FAIRY_OFFSET),     ENTRANCE_GROTTO_EXIT(GROTTO_SFM_FAIRY_OFFSET),  SINGLE_SCENE_INFO(SCENE_SACRED_FOREST_MEADOW), "SFM Fairy Grotto Entry",                     "SFM Fairy Grotto",                           ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_GROTTO,    "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_SFM_STORMS_OFFSET),    ENTRANCE_GROTTO_EXIT(GROTTO_SFM_STORMS_OFFSET), SINGLE_SCENE_INFO(SCENE_SACRED_FOREST_MEADOW), "SFM Storms Grotto Entry",                    "SFM Deku Scrub Grotto",                      ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTR_FOREST_TEMPLE_ENTRANCE,                       ENTR_SACRED_FOREST_MEADOW_OUTSIDE_TEMPLE,       SINGLE_SCENE_INFO(SCENE_SACRED_FOREST_MEADOW), "Sacred Forest Meadow Outside Forest Temple", "Forest Temple Entrance",                     ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTRANCE_GROTTO_EXIT(GROTTO_SFM_WOLFOS_OFFSET),    ENTRANCE_GROTTO_LOAD(GROTTO_SFM_WOLFOS_OFFSET), {{ SCENE_GROTTOS, 0x08 }},                     "SFM Wolfos Grotto",                          "SFM Wolfos Grotto Entry",                    ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_GROTTO},
    { ENTRANCE_GROTTO_EXIT(GROTTO_SFM_FAIRY_OFFSET),     ENTRANCE_GROTTO_LOAD(GROTTO_SFM_FAIRY_OFFSET),  {{ SCENE_FAIRYS_FOUNTAIN, 0x00 }},             "SFM Fairy Grotto",                           "SFM Fairy Grotto Entry",                     ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_GROTTO},
    { ENTRANCE_GROTTO_EXIT(GROTTO_SFM_STORMS_OFFSET),    ENTRANCE_GROTTO_LOAD(GROTTO_SFM_STORMS_OFFSET), {{ SCENE_GROTTOS, 0x0A }},                     "SFM Deku Scrub Grotto",                      "SFM Storms Grotto Entry",                    ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_GROTTO,    "scrubs"},
    { ENTR_SACRED_FOREST_MEADOW_OUTSIDE_TEMPLE,          ENTR_FOREST_TEMPLE_ENTRANCE,                    SINGLE_SCENE_INFO(SCENE_FOREST_TEMPLE),        "Forest Temple Entrance",                     "Sacred Forest Meadow Outside Forest Temple", ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_DUNGEON},
    { ENTR_FOREST_TEMPLE_BOSS_ENTRANCE,                  ENTR_FOREST_TEMPLE_BOSS_DOOR,                   SINGLE_SCENE_INFO(SCENE_FOREST_TEMPLE),        "Forest Temple Boss Door",                    "Phantom Ganon",                              ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_DUNGEON, "", 1},
    { ENTR_FOREST_TEMPLE_BOSS_DOOR,                      ENTR_FOREST_TEMPLE_BOSS_ENTRANCE,               SINGLE_SCENE_INFO(SCENE_FOREST_TEMPLE_BOSS),   "Phantom Ganon",                              "Forest Temple Boss Door",                    ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_DUNGEON, "", 1},
    { ENTR_SACRED_FOREST_MEADOW_FOREST_TEMPLE_BLUE_WARP, -1,                                             SINGLE_SCENE_INFO(SCENE_FOREST_TEMPLE_BOSS),   "Phantom Ganon Blue Warp",                    "Forest Temple Blue Warp",                    ENTRANCE_GROUP_SFM, ENTRANCE_GROUP_SFM,        ENTRANCE_TYPE_ONE_WAY, "bw", 1},

    // Kakariko Village
    { ENTR_HYRULE_FIELD_STAIRS_EXIT,                    ENTR_KAKARIKO_VILLAGE_FRONT_GATE,                 SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kakariko Front Gate",            "Hyrule Field Stairs Exit",         ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_HYRULE_FIELD,         ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_GRAVEYARD_ENTRANCE,                          ENTR_KAKARIKO_VILLAGE_SOUTHEAST_EXIT,             SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kakariko Southeast Exit",        "Graveyard Entrance",               ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_GRAVEYARD,            ENTRANCE_TYPE_OVERWORLD},
    { ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT,            ENTR_KAKARIKO_VILLAGE_GUARD_GATE,                 SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kakariko Guard Gate Exit",       "Death Mountain Trail Bottom Exit", ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_TYPE_OVERWORLD},
    { ENTR_KAKARIKO_CENTER_GUEST_HOUSE_0,               ENTR_KAKARIKO_VILLAGE_OUTSIDE_CENTER_GUEST_HOUSE, SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Boss House Entry",           "Carpenter Boss House",             ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_HOUSE_OF_SKULLTULA_0,                        ENTR_KAKARIKO_VILLAGE_OUTSIDE_SKULKLTULA_HOUSE,   SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Skulltula House Entry",      "House of Skulltula",               ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_IMPAS_HOUSE_FRONT,                           ENTR_KAKARIKO_VILLAGE_OUTSIDE_IMPAS_HOUSE_FRONT,  SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Impa's House Front Entry",   "Impa's House Front",               ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_IMPAS_HOUSE_BACK,                            ENTR_KAKARIKO_VILLAGE_OUTSIDE_IMPAS_HOUSE_BACK,   SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Impa's House Back Entry",    "Impa's House Back",                ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "cow", 1},
    { ENTR_WINDMILL_AND_DAMPES_GRAVE_WINDMILL,          ENTR_KAKARIKO_VILLAGE_OUTSIDE_WINDMILL,           SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Windmill Entry",             "Windmill",                         ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_SHOOTING_GALLERY_0,                          ENTR_KAKARIKO_VILLAGE_OUTSIDE_SHOOTING_GALLERY,   SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Shooting Gallery Entry",     "Kak Shooting Gallery",             ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "adult", 1},
    { ENTR_POTION_SHOP_GRANNY_0,                        ENTR_KAKARIKO_VILLAGE_OUTSIDE_SHOP_GRANNY,        SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Granny's Potion Shop Entry", "Granny's Potion Shop",             ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_BAZAAR_0,                                    ENTR_KAKARIKO_VILLAGE_OUTSIDE_BAZAAR,             SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Bazaar Entry",               "Kak Bazaar",                       ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "shop", 1},
    { ENTR_POTION_SHOP_KAKARIKO_FRONT,                  ENTR_KAKARIKO_VILLAGE_OUTSIDE_POTION_SHOP_FRONT,  SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Potion Shop Front Entry",    "Kak Potion Shop Front",            ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_POTION_SHOP_KAKARIKO_BACK,                   ENTR_KAKARIKO_VILLAGE_OUTSIDE_POTION_SHOP_BACK,   SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Potion Shop Back Entry",     "Kak Potion Shop Back",             ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_KAK_OPEN_OFFSET),     ENTRANCE_GROTTO_EXIT(GROTTO_KAK_OPEN_OFFSET),     SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Open Grotto Entry",          "Kak Open Grotto",                  ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_KAK_REDEAD_OFFSET),   ENTRANCE_GROTTO_EXIT(GROTTO_KAK_REDEAD_OFFSET),   SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kak Center Grotto Entry",        "Kak Redead Grotto",                ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTR_BOTTOM_OF_THE_WELL_ENTRANCE,                 ENTR_KAKARIKO_VILLAGE_OUTSIDE_BOTTOM_OF_THE_WELL, SINGLE_SCENE_INFO(SCENE_KAKARIKO_VILLAGE),            "Kakariko Outside the Well",      "Bottom of the Well Entrance",      ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_DUNGEON,   "botw", 1},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_CENTER_GUEST_HOUSE, ENTR_KAKARIKO_CENTER_GUEST_HOUSE_0,               SINGLE_SCENE_INFO(SCENE_KAKARIKO_CENTER_GUEST_HOUSE), "Carpenter Boss House",           "Kak Boss House Entry",             ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_SKULKLTULA_HOUSE,   ENTR_HOUSE_OF_SKULLTULA_0,                        SINGLE_SCENE_INFO(SCENE_HOUSE_OF_SKULLTULA),          "House of Skulltula",             "Kak Skulltula House Entry",        ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_IMPAS_HOUSE_FRONT,  ENTR_IMPAS_HOUSE_FRONT,                           SINGLE_SCENE_INFO(SCENE_IMPAS_HOUSE),                 "Impa's House Front",             "Kak Impa's House Front Entry",     ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_IMPAS_HOUSE_BACK,   ENTR_IMPAS_HOUSE_BACK,                            SINGLE_SCENE_INFO(SCENE_IMPAS_HOUSE),                 "Impa's House Back",              "Kak Impa's House Back Entry",      ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "cow"},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_WINDMILL,           ENTR_WINDMILL_AND_DAMPES_GRAVE_WINDMILL,          SINGLE_SCENE_INFO(SCENE_WINDMILL_AND_DAMPES_GRAVE),   "Windmill",                       "Kak Windmill Entry",               ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_SHOOTING_GALLERY,   ENTR_SHOOTING_GALLERY_0,                          {{ SCENE_SHOOTING_GALLERY, 0x00 }},                   "Kak Shooting Gallery",           "Kak Shooting Gallery Entry",       ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_SHOP_GRANNY,        ENTR_POTION_SHOP_GRANNY_0,                        SINGLE_SCENE_INFO(SCENE_POTION_SHOP_GRANNY),          "Granny's Potion Shop",           "Kak Granny's Potion Shop Entry",   ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_BAZAAR,             ENTR_BAZAAR_0,                                    {{ SCENE_BAZAAR, 0x00 }},                             "Kak Bazaar",                     "Kak Bazaar Entry",                 ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR,  "shop"},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_POTION_SHOP_FRONT,  ENTR_POTION_SHOP_KAKARIKO_FRONT,                  SINGLE_SCENE_INFO(SCENE_POTION_SHOP_KAKARIKO),        "Kak Potion Shop Front",          "Kak Potion Shop Front Entry",      ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_POTION_SHOP_BACK,   ENTR_POTION_SHOP_KAKARIKO_BACK,                   SINGLE_SCENE_INFO(SCENE_POTION_SHOP_KAKARIKO),        "Kak Potion Shop Back",           "Kak Potion Shop Back Entry",       ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_INTERIOR},
    { ENTRANCE_GROTTO_EXIT(GROTTO_KAK_OPEN_OFFSET),     ENTRANCE_GROTTO_LOAD(GROTTO_KAK_OPEN_OFFSET),     {{ SCENE_GROTTOS, 0x00 }},                            "Kak Open Grotto",                "Kak Open Grotto Entry",            ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_GROTTO,    "chest"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_KAK_REDEAD_OFFSET),   ENTRANCE_GROTTO_LOAD(GROTTO_KAK_REDEAD_OFFSET),   {{ SCENE_GROTTOS, 0x03 }},                            "Kak Redead Grotto",              "Kak Center Grotto Entry",          ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_GROTTO,    "chest"},
    { ENTR_KAKARIKO_VILLAGE_OUTSIDE_BOTTOM_OF_THE_WELL, ENTR_BOTTOM_OF_THE_WELL_ENTRANCE,                 SINGLE_SCENE_INFO(SCENE_BOTTOM_OF_THE_WELL),          "Bottom of the Well Entrance",    "Kakariko Outside the Well",        ENTRANCE_GROUP_KAKARIKO, ENTRANCE_GROUP_KAKARIKO,             ENTRANCE_TYPE_DUNGEON,   "botw"},

    // The Graveyard
    { ENTR_KAKARIKO_VILLAGE_SOUTHEAST_EXIT,   ENTR_GRAVEYARD_ENTRANCE,               SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "Graveyard Entrance",           "Kakariko Southeast Exit",      ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_KAKARIKO,  ENTRANCE_TYPE_OVERWORLD},
    { ENTR_GRAVEKEEPERS_HUT_0,                ENTR_GRAVEYARD_OUTSIDE_DAMPES_HUT,     SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "GY Dampe's Hut Entry",         "Dampe's Hut",                  ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_INTERIOR, "", 1},
    { ENTR_GRAVE_WITH_FAIRYS_FOUNTAIN_0,      ENTR_GRAVEYARD_SHIELD_GRAVE_EXIT,      SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "GY Near-Hut Grave Entry",      "Shield Grave",                 ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO,   "", 1},
    { ENTR_REDEAD_GRAVE_0,                    ENTR_GRAVEYARD_HEART_PIECE_GRAVE_EXIT, SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "GY Near-Tomb Grave Entry",     "Heart Piece Grave",            ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO,   "", 1},
    { ENTR_ROYAL_FAMILYS_TOMB_0,              ENTR_GRAVEYARD_ROYAL_TOMB_EXIT,        SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "GY Royal Family's Tomb Entry", "Royal Family's Tomb",          ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO,   "", 1},
    { ENTR_WINDMILL_AND_DAMPES_GRAVE_GRAVE,   ENTR_GRAVEYARD_DAMPES_GRAVE_EXIT,      SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "GY Near-Ledge Grave Entry",    "Dampe's Grave",                ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO,   "race", 1},
    { ENTR_SHADOW_TEMPLE_ENTRANCE,            ENTR_GRAVEYARD_OUTSIDE_TEMPLE,         SINGLE_SCENE_INFO(SCENE_GRAVEYARD),                  "Graveyard Outside Temple",     "Shadow Temple Entrance",       ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_DUNGEON,  "", 1},
    { ENTR_GRAVEYARD_OUTSIDE_DAMPES_HUT,      ENTR_GRAVEKEEPERS_HUT_0,               SINGLE_SCENE_INFO(SCENE_GRAVEKEEPERS_HUT),           "Dampe's Hut",                  "GY Dampe's Hut Entry",         ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_INTERIOR},
    { ENTR_GRAVEYARD_SHIELD_GRAVE_EXIT,       ENTR_GRAVE_WITH_FAIRYS_FOUNTAIN_0,     SINGLE_SCENE_INFO(SCENE_GRAVE_WITH_FAIRYS_FOUNTAIN), "Shield Grave",                 "GY Near-Hut Grave Entry",      ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO},
    { ENTR_GRAVEYARD_HEART_PIECE_GRAVE_EXIT,  ENTR_REDEAD_GRAVE_0,                   SINGLE_SCENE_INFO(SCENE_REDEAD_GRAVE),               "Heart Piece Grave",            "GY Near-Tomb Grave Entry",     ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO},
    { ENTR_GRAVEYARD_ROYAL_TOMB_EXIT,         ENTR_ROYAL_FAMILYS_TOMB_0,             SINGLE_SCENE_INFO(SCENE_ROYAL_FAMILYS_TOMB),         "Royal Family's Tomb",          "GY Royal Family's Tomb Entry", ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO},
    { ENTR_GRAVEYARD_DAMPES_GRAVE_EXIT,       ENTR_WINDMILL_AND_DAMPES_GRAVE_GRAVE,  SINGLE_SCENE_INFO(SCENE_WINDMILL_AND_DAMPES_GRAVE),  "Dampe's Grave",                "GY Near-Ledge Grave Entry",    ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_GROTTO,   "race"},
    { ENTR_GRAVEYARD_OUTSIDE_TEMPLE,          ENTR_SHADOW_TEMPLE_ENTRANCE,           SINGLE_SCENE_INFO(SCENE_SHADOW_TEMPLE),              "Shadow Temple Entrance",       "Graveyard Outside Temple",     ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_DUNGEON},
    { ENTR_SHADOW_TEMPLE_BOSS_ENTRANCE,       ENTR_SHADOW_TEMPLE_BOSS_DOOR,          SINGLE_SCENE_INFO(SCENE_SHADOW_TEMPLE),              "Shadow Temple Boss Door",      "Bongo-Bongo",                  ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_DUNGEON, "", 1},
    { ENTR_SHADOW_TEMPLE_BOSS_DOOR,           ENTR_SHADOW_TEMPLE_BOSS_ENTRANCE,      SINGLE_SCENE_INFO(SCENE_SHADOW_TEMPLE_BOSS),         "Bongo-Bongo",                  "Shadow Temple Boss Door",      ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_DUNGEON, "", 1},
    { ENTR_GRAVEYARD_SHADOW_TEMPLE_BLUE_WARP, -1,                                    SINGLE_SCENE_INFO(SCENE_SHADOW_TEMPLE_BOSS),         "Bongo-Bongo Blue Warp",        "Shadow Temple Blue Warp",      ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_GROUP_GRAVEYARD, ENTRANCE_TYPE_ONE_WAY, "bw", 1},

    // Death Mountain Trail
    { ENTR_GORON_CITY_UPPER_EXIT,                        ENTR_DEATH_MOUNTAIN_TRAIL_GC_EXIT,                 SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "Death Mountain Trail Middle Exit",              "Goron City Upper Exit",                         ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_GORON_CITY,            ENTRANCE_TYPE_OVERWORLD, "gc"},
    { ENTR_KAKARIKO_VILLAGE_GUARD_GATE,                  ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT,             SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "Death Mountain Trail Bottom Exit",              "Kakariko Guard Gate Exit",                      ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_KAKARIKO,              ENTRANCE_TYPE_OVERWORLD},
    { ENTR_DEATH_MOUNTAIN_CRATER_UPPER_EXIT,             ENTR_DEATH_MOUNTAIN_TRAIL_SUMMIT_EXIT,             SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "Death Mountain Trail Top Exit",                 "Death Mountain Crater Upper Exit",              ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_OVERWORLD},
    { ENTR_GREAT_FAIRYS_FOUNTAIN_MAGIC_DMT,              ENTR_DEATH_MOUNTAIN_TRAIL_GREAT_FAIRY_EXIT,        SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "DMT Great Fairy Entry",                         "DMT Great Fairy Fountain",                      ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_DMT_STORMS_OFFSET),    ENTRANCE_GROTTO_EXIT(GROTTO_DMT_STORMS_OFFSET),    SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "DMT Rock Circle Grotto Entry",                  "DMT Storms Grotto",                             ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_DMT_COW_OFFSET),       ENTRANCE_GROTTO_EXIT(GROTTO_DMT_COW_OFFSET),       SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "DMT Boulder Grotto Entry",                      "DMT Cow Grotto",                                ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_GROTTO,    "", 1},
    { ENTR_DODONGOS_CAVERN_ENTRANCE,                     ENTR_DEATH_MOUNTAIN_TRAIL_OUTSIDE_DODONGOS_CAVERN, SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_TRAIL), "Death Mountain Trail Outside Dodongo's Cavern", "Dodongo's Cavern Entrance",                     ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_DUNGEON,   "dc", 1},
    { ENTR_DEATH_MOUNTAIN_TRAIL_GREAT_FAIRY_EXIT,        ENTR_GREAT_FAIRYS_FOUNTAIN_MAGIC_DMT,              {{ SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC, 0x00 }}, "DMT Great Fairy Fountain",                      "DMT Great Fairy Entry",                         ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_INTERIOR},
    { ENTRANCE_GROTTO_EXIT(GROTTO_DMT_STORMS_OFFSET),    ENTRANCE_GROTTO_LOAD(GROTTO_DMT_STORMS_OFFSET),    {{ SCENE_GROTTOS, 0x00 }},                     "DMT Storms Grotto",                             "DMT Rock Circle Grotto Entry",                  ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_GROTTO,    "chest"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_DMT_COW_OFFSET),       ENTRANCE_GROTTO_LOAD(GROTTO_DMT_COW_OFFSET),       {{ SCENE_GROTTOS, 0x0D }},                     "DMT Cow Grotto",                                "DMT Boulder Grotto Entry",                      ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_GROTTO},
    { ENTR_DEATH_MOUNTAIN_TRAIL_OUTSIDE_DODONGOS_CAVERN, ENTR_DODONGOS_CAVERN_ENTRANCE,                     SINGLE_SCENE_INFO(SCENE_DODONGOS_CAVERN),      "Dodongo's Cavern Entrance",                     "Death Mountain Trail Outside Dodongo's Cavern", ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_DUNGEON,   "dc"},
    { ENTR_DODONGOS_CAVERN_BOSS_ENTRANCE,                ENTR_DODONGOS_CAVERN_BOSS_DOOR,                    SINGLE_SCENE_INFO(SCENE_DODONGOS_CAVERN),      "Dodongo's Cavern Boss Door",                    "King Dodongo",                                  ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_DUNGEON,   "dc", 1},
    { ENTR_DODONGOS_CAVERN_BOSS_DOOR,                    ENTR_DODONGOS_CAVERN_BOSS_ENTRANCE,                SINGLE_SCENE_INFO(SCENE_DODONGOS_CAVERN_BOSS), "King Dodongo",                                  "Dodongo's Cavern Boss Door",                    ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_DUNGEON,   "dc", 1},
    { ENTR_DEATH_MOUNTAIN_TRAIL_DODONGO_BLUE_WARP,       -1,                                                SINGLE_SCENE_INFO(SCENE_DODONGOS_CAVERN_BOSS), "King Dodongo Blue Warp",                        "Dodongo's Cavern Blue Warp",                    ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_ONE_WAY,   "dc,bw", 1},

    // Death Mountain Crater
    { ENTR_GORON_CITY_DARUNIA_ROOM_EXIT,                ENTR_DEATH_MOUNTAIN_CRATER_GC_EXIT,             SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_CRATER), "Death Mountain Crater Bridge Exit",    "Goron City Darunia's Room Backdoor",   ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_GORON_CITY,            ENTRANCE_TYPE_OVERWORLD, "gc"},
    { ENTR_DEATH_MOUNTAIN_TRAIL_SUMMIT_EXIT,            ENTR_DEATH_MOUNTAIN_CRATER_UPPER_EXIT,          SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_CRATER), "Death Mountain Crater Upper Exit",     "Death Mountain Trail Top Exit",        ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_OVERWORLD},
    { ENTR_GREAT_FAIRYS_FOUNTAIN_MAGIC_DMC,             ENTR_DEATH_MOUNTAIN_CRATER_GREAT_FAIRY_EXIT,    SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_CRATER), "DMC Great Fairy Entry",                "DMC Great Fairy Fountain",             ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_DMC_UPPER_OFFSET),    ENTRANCE_GROTTO_EXIT(GROTTO_DMC_UPPER_OFFSET),  SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_CRATER), "DMC Upper Grotto Entry",               "DMC Upper Grotto",                     ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_DMC_HAMMER_OFFSET),   ENTRANCE_GROTTO_EXIT(GROTTO_DMC_HAMMER_OFFSET), SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_CRATER), "DMC Hammer Grotto Entry",              "DMC Scrubs Grotto",                    ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTR_FIRE_TEMPLE_ENTRANCE,                        ENTR_DEATH_MOUNTAIN_CRATER_OUTSIDE_TEMPLE,      SINGLE_SCENE_INFO(SCENE_DEATH_MOUNTAIN_CRATER), "Death Mountain Crater Outside Temple", "Fire Temple Entrance",                 ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_DEATH_MOUNTAIN_CRATER_GREAT_FAIRY_EXIT,      ENTR_GREAT_FAIRYS_FOUNTAIN_MAGIC_DMC,           {{ SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC, 0x01 }},  "DMC Great Fairy Fountain",             "DMC Great Fairy Entry",                ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_INTERIOR},
    { ENTRANCE_GROTTO_EXIT(GROTTO_DMC_UPPER_OFFSET),    ENTRANCE_GROTTO_LOAD(GROTTO_DMC_UPPER_OFFSET),  {{ SCENE_GROTTOS, 0x00 }},                      "DMC Upper Grotto",                     "DMC Upper Grotto Entry",               ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_GROTTO,    "chest"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_DMC_HAMMER_OFFSET),   ENTRANCE_GROTTO_LOAD(GROTTO_DMC_HAMMER_OFFSET), {{ SCENE_GROTTOS, 0x04 }},                      "DMC Scrubs Grotto",                    "DMC Hammer Grotto Entry",              ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_GROTTO,    "scrubs"},
    { ENTR_DEATH_MOUNTAIN_CRATER_OUTSIDE_TEMPLE,        ENTR_FIRE_TEMPLE_ENTRANCE,                      SINGLE_SCENE_INFO(SCENE_FIRE_TEMPLE),           "Fire Temple Entrance",                 "Death Mountain Crater Outside Temple", ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_DUNGEON},
    { ENTR_FIRE_TEMPLE_BOSS_ENTRANCE,                   ENTR_FIRE_TEMPLE_BOSS_DOOR,                     SINGLE_SCENE_INFO(SCENE_FIRE_TEMPLE),           "Fire Temple Boss Door",                "Volvagia",                             ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_FIRE_TEMPLE_BOSS_DOOR,                       ENTR_FIRE_TEMPLE_BOSS_ENTRANCE,                 SINGLE_SCENE_INFO(SCENE_FIRE_TEMPLE_BOSS),      "Volvagia",                             "Fire Temple Boss Door",                ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_DEATH_MOUNTAIN_CRATER_FIRE_TEMPLE_BLUE_WARP, -1,                                             SINGLE_SCENE_INFO(SCENE_FIRE_TEMPLE_BOSS),      "Volvagia Blue Warp",                   "Fire Temple Blue Warp",                ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_ONE_WAY,   "bw", 1},

    // Goron City
    { ENTR_DEATH_MOUNTAIN_TRAIL_GC_EXIT,              ENTR_GORON_CITY_UPPER_EXIT,                     SINGLE_SCENE_INFO(SCENE_GORON_CITY), "Goron City Upper Exit",              "Death Mountain Trail Middle Exit",  ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_DEATH_MOUNTAIN_TRAIL,  ENTRANCE_TYPE_OVERWORLD, "gc"},
    { ENTR_DEATH_MOUNTAIN_CRATER_GC_EXIT,             ENTR_GORON_CITY_DARUNIA_ROOM_EXIT,              SINGLE_SCENE_INFO(SCENE_GORON_CITY), "Goron City Darunia's Room Backdoor", "Death Mountain Crater Bridge Exit", ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_DEATH_MOUNTAIN_CRATER, ENTRANCE_TYPE_OVERWORLD, "gc"},
    { ENTR_LOST_WOODS_TUNNEL_SHORTCUT,                ENTR_GORON_CITY_TUNNEL_SHORTCUT,                SINGLE_SCENE_INFO(SCENE_GORON_CITY), "Goron City Tunnel Shortcut",         "Lost Woods Tunnel Shortcut",        ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_LOST_WOODS,            ENTRANCE_TYPE_OVERWORLD, "gc,lw"},
    { ENTR_GORON_SHOP_0,                              ENTR_GORON_CITY_OUTSIDE_SHOP,                   SINGLE_SCENE_INFO(SCENE_GORON_CITY), "GC Shop Entry",                      "Goron Shop",                        ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_GORON_CITY,            ENTRANCE_TYPE_INTERIOR,  "gc", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_GORON_CITY_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_GORON_CITY_OFFSET), SINGLE_SCENE_INFO(SCENE_GORON_CITY), "GC Lava Grotto Entry",               "GC Scrubs Grotto",                  ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_GORON_CITY,            ENTRANCE_TYPE_GROTTO,    "gc,scrubs", 1},
    { ENTR_GORON_CITY_OUTSIDE_SHOP,                   ENTR_GORON_SHOP_0,                              SINGLE_SCENE_INFO(SCENE_GORON_SHOP), "Goron Shop",                         "GC Shop Entry",                     ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_GORON_CITY,            ENTRANCE_TYPE_INTERIOR,  "gc"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_GORON_CITY_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_GORON_CITY_OFFSET), {{ SCENE_GROTTOS, 0x04 }},           "GC Scrubs Grotto",                   "GC Lava Grotto Entry",              ENTRANCE_GROUP_GORON_CITY, ENTRANCE_GROUP_GORON_CITY,            ENTRANCE_TYPE_GROTTO,    "gc,scrubs"},

    // Zora's River
    { ENTR_HYRULE_FIELD_RIVER_EXIT,                  ENTR_ZORAS_RIVER_WEST_EXIT,                    SINGLE_SCENE_INFO(SCENE_ZORAS_RIVER), "Zora's River Lower Exit",          "Hyrule Field River Exit",        ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_LOST_WOODS_UNDERWATER_SHORTCUT,           ENTR_ZORAS_RIVER_UNDERWATER_SHORTCUT,          SINGLE_SCENE_INFO(SCENE_ZORAS_RIVER), "Zora's River Underwater Shortcut", "Lost Woods Underwater Shortcut", ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_LOST_WOODS,   ENTRANCE_TYPE_OVERWORLD, "lw"},
    { ENTR_ZORAS_DOMAIN_ENTRANCE,                    ENTR_ZORAS_RIVER_WATERFALL_EXIT,               SINGLE_SCENE_INFO(SCENE_ZORAS_RIVER), "Zora's River Waterfall Exit",      "Zora's Domain Entrance",         ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_TYPE_OVERWORLD},
    { ENTRANCE_GROTTO_LOAD(GROTTO_ZR_STORMS_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_ZR_STORMS_OFFSET), SINGLE_SCENE_INFO(SCENE_ZORAS_RIVER), "ZR Rock Circle Grotto Entry",      "ZR Deku SCrub Grotto",           ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_RIVER,  ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_ZR_FAIRY_OFFSET),  ENTRANCE_GROTTO_EXIT(GROTTO_ZR_FAIRY_OFFSET),  SINGLE_SCENE_INFO(SCENE_ZORAS_RIVER), "ZR Raised Boulder Grotto Entry",   "ZR Fairy Grotto",                ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_RIVER,  ENTRANCE_TYPE_GROTTO,    "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_ZR_OPEN_OFFSET),   ENTRANCE_GROTTO_EXIT(GROTTO_ZR_OPEN_OFFSET),   SINGLE_SCENE_INFO(SCENE_ZORAS_RIVER), "ZR Raised Open Grotto Entry",      "ZR Open Grotto",                 ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_RIVER,  ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_EXIT(GROTTO_ZR_STORMS_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_ZR_STORMS_OFFSET), {{ SCENE_GROTTOS, 0x0A }},            "ZR Deku Scrub Grotto",             "ZR Rock Circle Grotto Entry",    ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_RIVER,  ENTRANCE_TYPE_GROTTO,    "scrubs"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_ZR_FAIRY_OFFSET),  ENTRANCE_GROTTO_LOAD(GROTTO_ZR_FAIRY_OFFSET),  {{ SCENE_FAIRYS_FOUNTAIN, 0x00 }},    "ZR Fairy Grotto",                  "ZR Raised Boulder Grotto Entry", ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_RIVER,  ENTRANCE_TYPE_GROTTO},
    { ENTRANCE_GROTTO_EXIT(GROTTO_ZR_OPEN_OFFSET),   ENTRANCE_GROTTO_LOAD(GROTTO_ZR_OPEN_OFFSET),   {{ SCENE_GROTTOS, 0x00 }},            "ZR Open Grotto",                   "ZR Raised Open Grotto Entry",    ENTRANCE_GROUP_ZORAS_RIVER, ENTRANCE_GROUP_ZORAS_RIVER,  ENTRANCE_TYPE_GROTTO,    "chest"},

    // Zora's Domain
    { ENTR_ZORAS_RIVER_WATERFALL_EXIT,               ENTR_ZORAS_DOMAIN_ENTRANCE,                    SINGLE_SCENE_INFO(SCENE_ZORAS_DOMAIN), "Zora's Domain Entrance",            "Zora's River Waterfall Exit",    ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_ZORAS_RIVER,    ENTRANCE_TYPE_OVERWORLD},
    { ENTR_LAKE_HYLIA_UNDERWATER_SHORTCUT,           ENTR_ZORAS_DOMAIN_UNDERWATER_SHORTCUT,         SINGLE_SCENE_INFO(SCENE_ZORAS_DOMAIN), "Zora's Domain Underwater Shortcut", "Lake Hylia Underwater Shortcut", ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_LAKE_HYLIA,     ENTRANCE_TYPE_OVERWORLD, "lh"},
    { ENTR_ZORAS_FOUNTAIN_TUNNEL_EXIT,               ENTR_ZORAS_DOMAIN_KING_ZORA_EXIT,              SINGLE_SCENE_INFO(SCENE_ZORAS_DOMAIN), "Zora's Domain Behind King Zora",    "Zora's Fountain Tunnel Exit",    ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_OVERWORLD},
    { ENTR_ZORA_SHOP_0,                              ENTR_ZORAS_DOMAIN_OUTSIDE_SHOP,                SINGLE_SCENE_INFO(SCENE_ZORAS_DOMAIN), "ZD Shop Entry",                     "Zora Shop",                      ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_ZORAS_DOMAIN,   ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_ZD_STORMS_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_ZD_STORMS_OFFSET), SINGLE_SCENE_INFO(SCENE_ZORAS_DOMAIN), "ZD Island Grotto Entry",            "ZD Fairy Grotto",                ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_ZORAS_DOMAIN,   ENTRANCE_TYPE_GROTTO,    "fairy", 1},
    { ENTR_ZORAS_DOMAIN_OUTSIDE_SHOP,                ENTR_ZORA_SHOP_0,                              SINGLE_SCENE_INFO(SCENE_ZORA_SHOP),    "Zora Shop",                         "ZD Shop Entry",                  ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_ZORAS_DOMAIN,   ENTRANCE_TYPE_INTERIOR},
    { ENTRANCE_GROTTO_EXIT(GROTTO_ZD_STORMS_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_ZD_STORMS_OFFSET), {{ SCENE_FAIRYS_FOUNTAIN, 0x00 }},     "ZD Fairy Grotto",                   "ZD Island Grotto Entry",         ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_GROUP_ZORAS_DOMAIN,   ENTRANCE_TYPE_GROTTO,    "fairy"},

    // Zora's Fountain
    { ENTR_ZORAS_DOMAIN_KING_ZORA_EXIT,             ENTR_ZORAS_FOUNTAIN_TUNNEL_EXIT,              SINGLE_SCENE_INFO(SCENE_ZORAS_FOUNTAIN),        "Zora's Fountain Tunnel Exit",        "Zora's Domain Behind King Zora",     ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_DOMAIN,   ENTRANCE_TYPE_OVERWORLD},
    { ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_FARORES_ZF, ENTR_ZORAS_FOUNTAIN_OUTSIDE_GREAT_FAIRY,      SINGLE_SCENE_INFO(SCENE_ZORAS_FOUNTAIN),        "ZF Great Fairy Entry",               "ZF Great Fairy Fountain",            ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_INTERIOR, "", 1},
    { ENTR_JABU_JABU_ENTRANCE,                      ENTR_ZORAS_FOUNTAIN_OUTSIDE_JABU_JABU,        SINGLE_SCENE_INFO(SCENE_ZORAS_FOUNTAIN),        "Zora's Fountain Outside Jabu Jabu",  "Jabu Jabu's Belly Entrance",         ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_DUNGEON,  "", 1},
    { ENTR_ICE_CAVERN_ENTRANCE,                     ENTR_ZORAS_FOUNTAIN_OUTSIDE_ICE_CAVERN,       SINGLE_SCENE_INFO(SCENE_ZORAS_FOUNTAIN),        "Zora's Fountain Outside Ice Cavern", "Ice Cavern Entrance",                ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_DUNGEON,  "", 1},
    { ENTR_ZORAS_FOUNTAIN_OUTSIDE_GREAT_FAIRY,      ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_FARORES_ZF, {{ SCENE_GREAT_FAIRYS_FOUNTAIN_SPELLS, 0x00 }}, "ZF Great Fairy Fountain",            "ZF Great Fairy Entry",               ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_INTERIOR},
    { ENTR_ZORAS_FOUNTAIN_OUTSIDE_JABU_JABU,        ENTR_JABU_JABU_ENTRANCE,                      SINGLE_SCENE_INFO(SCENE_JABU_JABU),             "Jabu Jabu's Belly Entrance",         "Zora's Fountain Outside Jabu Jabu",  ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_DUNGEON},
    { ENTR_JABU_JABU_BOSS_ENTRANCE,                 ENTR_JABU_JABU_BOSS_DOOR,                     SINGLE_SCENE_INFO(SCENE_JABU_JABU),             "Jabu Jabu's Belly Boss Door",        "Barinade",                           ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_DUNGEON, "", 1},
    { ENTR_JABU_JABU_BOSS_DOOR,                     ENTR_JABU_JABU_BOSS_ENTRANCE,                 SINGLE_SCENE_INFO(SCENE_JABU_JABU_BOSS),        "Barinade",                           "Jabu Jabu's Belly Boss Door",        ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_DUNGEON, "", 1},
    { ENTR_ZORAS_FOUNTAIN_JABU_JABU_BLUE_WARP,      -1,                                           SINGLE_SCENE_INFO(SCENE_JABU_JABU_BOSS),        "Barinade Blue Warp",                 "Jabu Jabu's Belly Blue Warp",        ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_ONE_WAY, "bw", 1},
    { ENTR_ZORAS_FOUNTAIN_OUTSIDE_ICE_CAVERN,       ENTR_ICE_CAVERN_ENTRANCE,                     SINGLE_SCENE_INFO(SCENE_ICE_CAVERN),            "Ice Cavern Entrance",                "Zora's Fountain Outside Ice Cavern", ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_GROUP_ZORAS_FOUNTAIN, ENTRANCE_TYPE_DUNGEON},

    // Hyrule Field
    { ENTR_LOST_WOODS_BRIDGE_WEST_EXIT,                    ENTR_HYRULE_FIELD_WOODED_EXIT,                       SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field Wooded Exit",            "Lost Woods Bridge West Exit",         ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_LOST_WOODS,    ENTRANCE_TYPE_OVERWORLD, "hf,lw"},
    { ENTR_MARKET_ENTRANCE_NEAR_GUARD_EXIT,                ENTR_HYRULE_FIELD_ON_BRIDGE_SPAWN,                   SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field Drawbridge Exit",        "Market Entrance South Exit",          ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_LON_LON_RANCH_ENTRANCE,                         ENTR_HYRULE_FIELD_CENTER_EXIT,                       SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field Center Exit",            "Lon Lon Ranch Entrance",              ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_OVERWORLD, "hf,llr"},
    { ENTR_KAKARIKO_VILLAGE_FRONT_GATE,                    ENTR_HYRULE_FIELD_STAIRS_EXIT,                       SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field Stairs Exit",            "Kakariko Front Gate",                 ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_KAKARIKO,      ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_ZORAS_RIVER_WEST_EXIT,                          ENTR_HYRULE_FIELD_RIVER_EXIT,                        SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field River Exit",             "Zora's River Lower Exit",             ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_ZORAS_RIVER,   ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_LAKE_HYLIA_NORTH_EXIT,                          ENTR_HYRULE_FIELD_FENCE_EXIT,                        SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field Fence Exit",             "Lake Hylia North Exit",               ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_LAKE_HYLIA,    ENTRANCE_TYPE_OVERWORLD, "hf,lh"},
    { ENTR_GERUDO_VALLEY_EAST_EXIT,                        ENTR_HYRULE_FIELD_ROCKY_PATH,                        SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "Hyrule Field Rocky Path",             "Gerudo Valley East Exit",             ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_GERUDO_VALLEY, ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_NEAR_MARKET_OFFSET),  ENTRANCE_GROTTO_EXIT(GROTTO_HF_NEAR_MARKET_OFFSET),  SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF Near Market Boulder Grotto Entry", "HF Near Market Boulder Grotto",       ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_NEAR_KAK_OFFSET),     ENTRANCE_GROTTO_EXIT(GROTTO_HF_NEAR_KAK_OFFSET),     SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF Stone Bridge Tree Grotto Entry",   "HF Stone Bridge Tree Grotto",         ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "spider", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_TEKTITE_OFFSET),      ENTRANCE_GROTTO_EXIT(GROTTO_HF_TEKTITE_OFFSET),      SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF Northwest Tree Grotto Entry",      "HF Tektite Grotto",                   ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "water", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_FAIRY_OFFSET),        ENTRANCE_GROTTO_EXIT(GROTTO_HF_FAIRY_OFFSET),        SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF Northwest Boulder Grotto Entry",   "HF Fairy Grotto",                     ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_COW_OFFSET),          ENTRANCE_GROTTO_EXIT(GROTTO_HF_COW_OFFSET),          SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF West Rock Circle Grotto Entry",    "HF Cow Grotto",                       ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "webbed", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_OPEN_OFFSET),         ENTRANCE_GROTTO_EXIT(GROTTO_HF_OPEN_OFFSET),         SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF South Open Grotto Entry",          "HF Open Grotto",                      ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_INSIDE_FENCE_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_HF_INSIDE_FENCE_OFFSET), SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF Fenced Grotto Entry",              "HF Fenced Scrub Grotto",              ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HF_SOUTHEAST_OFFSET),    ENTRANCE_GROTTO_EXIT(GROTTO_HF_SOUTHEAST_OFFSET),    SINGLE_SCENE_INFO(SCENE_HYRULE_FIELD), "HF Southeast Boulder Grotto Entry",   "HF Southeast Grotto",                 ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "chest", 1},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_NEAR_MARKET_OFFSET),  ENTRANCE_GROTTO_LOAD(GROTTO_HF_NEAR_MARKET_OFFSET),  {{ SCENE_GROTTOS, 0x00 }},             "HF Near Market Boulder Grotto",       "HF Near Market Boulder Grotto Entry", ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_NEAR_KAK_OFFSET),     ENTRANCE_GROTTO_LOAD(GROTTO_HF_NEAR_KAK_OFFSET),     {{ SCENE_GROTTOS, 0x01 }},             "HF Stone Bridge Tree Grotto",         "HF Stone Bridge Tree Grotto Entry",   ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "spider"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_TEKTITE_OFFSET),      ENTRANCE_GROTTO_LOAD(GROTTO_HF_TEKTITE_OFFSET),      {{ SCENE_GROTTOS, 0x0B }},             "HF Tektite Grotto",                   "HF Northwest Tree Grotto Entry",      ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "water"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_FAIRY_OFFSET),        ENTRANCE_GROTTO_LOAD(GROTTO_HF_FAIRY_OFFSET),        {{ SCENE_FAIRYS_FOUNTAIN, 0x00 }},     "HF Fairy Grotto",                     "HF Northwest Boulder Grotto Entry",   ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_COW_OFFSET),          ENTRANCE_GROTTO_LOAD(GROTTO_HF_COW_OFFSET),          {{ SCENE_GROTTOS, 0x05 }},             "HF Cow Grotto",                       "HF West Rock Circle Grotto Entry",    ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "webbed"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_OPEN_OFFSET),         ENTRANCE_GROTTO_LOAD(GROTTO_HF_OPEN_OFFSET),         {{ SCENE_GROTTOS, 0x00 }},             "HF Open Grotto",                      "HF South Open Grotto Entry",          ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "chest"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_INSIDE_FENCE_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_HF_INSIDE_FENCE_OFFSET), {{ SCENE_GROTTOS, 0x02 }},             "HF Fenced Scrub Grotto",              "HF Fenced Grotto Entry",              ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "srubs"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HF_SOUTHEAST_OFFSET),    ENTRANCE_GROTTO_LOAD(GROTTO_HF_SOUTHEAST_OFFSET),    {{ SCENE_GROTTOS, 0x00 }},             "HF Southeast Grotto",                 "HF Southeast Boulder Grotto Entry",   ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_GROTTO,    "chest"},

    // Lon Lon Ranch
    { ENTR_HYRULE_FIELD_CENTER_EXIT,           ENTR_LON_LON_RANCH_ENTRANCE,             SINGLE_SCENE_INFO(SCENE_LON_LON_RANCH), "Lon Lon Ranch Entrance",  "Hyrule Field Center Exit", ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_LON_LON_BUILDINGS_TALONS_HOUSE,     ENTR_LON_LON_RANCH_OUTSIDE_TALONS_HOUSE, SINGLE_SCENE_INFO(SCENE_LON_LON_RANCH), "LLR Talon's House Entry", "Talon's House",            ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_INTERIOR,  "llr", 1},
    { ENTR_STABLE_0,                           ENTR_LON_LON_RANCH_OUTSIDE_STABLES,      SINGLE_SCENE_INFO(SCENE_LON_LON_RANCH), "LLR Stables Entry",       "LLR Stables",              ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_INTERIOR,  "cow", 1},
    { ENTR_LON_LON_BUILDINGS_TOWER,            ENTR_LON_LON_RANCH_OUTSIDE_TOWER,        SINGLE_SCENE_INFO(SCENE_LON_LON_RANCH), "LLR Tower Entry",         "LLR Tower",                ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_INTERIOR,  "cow", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_LLR_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_LLR_OFFSET), SINGLE_SCENE_INFO(SCENE_LON_LON_RANCH), "LLR Grotto Entry",        "LLR Deku Scrub Grotto",    ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTR_LON_LON_RANCH_OUTSIDE_TALONS_HOUSE, ENTR_LON_LON_BUILDINGS_TALONS_HOUSE,     {{ SCENE_LON_LON_BUILDINGS, 0x00 }},    "Talon's House",           "LLR Talon's House Entry",  ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_INTERIOR,  "llr"},
    { ENTR_LON_LON_RANCH_OUTSIDE_STABLES,      ENTR_STABLE_0,                           SINGLE_SCENE_INFO(SCENE_STABLE),        "LLR Stables",             "LLR Stables Entry",        ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_INTERIOR,  "cow"},
    { ENTR_LON_LON_RANCH_OUTSIDE_TOWER,        ENTR_LON_LON_BUILDINGS_TOWER,            {{ SCENE_LON_LON_BUILDINGS, 0x01 }},    "LLR Tower",               "LLR Tower Entry",          ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_INTERIOR,  "cow"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_LLR_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_LLR_OFFSET), {{ SCENE_GROTTOS, 0x04 }},              "LLR Deku Scrub Grotto",   "LLR Grotto Entry",         ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_GROUP_LON_LON_RANCH, ENTRANCE_TYPE_GROTTO,    "scrubs"},

    // Lake Hylia
    { ENTR_HYRULE_FIELD_FENCE_EXIT,           ENTR_LAKE_HYLIA_NORTH_EXIT,             SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),          "Lake Hylia North Exit",          "Hyrule Field Fence Exit",           ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_HYRULE_FIELD, ENTRANCE_TYPE_OVERWORLD, "lh"},
    { ENTR_ZORAS_DOMAIN_UNDERWATER_SHORTCUT,  ENTR_LAKE_HYLIA_UNDERWATER_SHORTCUT,    SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),          "Lake Hylia Underwater Shortcut", "Zora's Domain Underwater Shortcut", ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_ZORAS_DOMAIN, ENTRANCE_TYPE_OVERWORLD, "lh"},
    { ENTR_LAKESIDE_LABORATORY_0,             ENTR_LAKE_HYLIA_OUTSIDE_LAB,            SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),          "LH Lab Entry",                   "LH Lab",                            ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_INTERIOR,  "lh", 1},
    { ENTR_FISHING_POND_0,                    ENTR_LAKE_HYLIA_OUTSIDE_FISHING_POND,   SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),          "LH Fishing Pond Entry",          "Fishing Pond",                      ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_INTERIOR,  "lh", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_LH_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_LH_OFFSET), SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),          "LH Grave Grotto Entry",          "LH Deku Scrub Grotto",              ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTR_WATER_TEMPLE_ENTRANCE,             ENTR_LAKE_HYLIA_OUTSIDE_TEMPLE,         SINGLE_SCENE_INFO(SCENE_LAKE_HYLIA),          "Lake Hylia Outside Temple",      "Water Temple Entrance",             ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_DUNGEON,   "lh", 1},
    { ENTR_LAKE_HYLIA_OUTSIDE_LAB,            ENTR_LAKESIDE_LABORATORY_0,             SINGLE_SCENE_INFO(SCENE_LAKESIDE_LABORATORY), "LH Lab",                         "LH Lab Entry",                      ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_INTERIOR,  "lh"},
    { ENTR_LAKE_HYLIA_OUTSIDE_FISHING_POND,   ENTR_FISHING_POND_0,                    SINGLE_SCENE_INFO(SCENE_FISHING_POND),        "Fishing Pond",                   "LH Fishing Pond Entry",             ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_INTERIOR,  "lh"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_LH_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_LH_OFFSET), {{ SCENE_GROTTOS, 0x04 }},                    "LH Deku Scrub Grotto",           "LH Grave Grotto Entry",             ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_GROTTO,    "lh,scrubs"},
    { ENTR_LAKE_HYLIA_OUTSIDE_TEMPLE,         ENTR_WATER_TEMPLE_ENTRANCE,             SINGLE_SCENE_INFO(SCENE_WATER_TEMPLE),        "Water Temple Entrance",          "Lake Hylia Outside Temple",         ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_DUNGEON,   "lh"},
    { ENTR_WATER_TEMPLE_BOSS_ENTRANCE,        ENTR_WATER_TEMPLE_BOSS_DOOR,            SINGLE_SCENE_INFO(SCENE_WATER_TEMPLE),        "Water Temple Boss Door",         "Morpha",                            ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_DUNGEON,   "lh", 1},
    { ENTR_WATER_TEMPLE_BOSS_DOOR,            ENTR_WATER_TEMPLE_BOSS_ENTRANCE,        SINGLE_SCENE_INFO(SCENE_WATER_TEMPLE_BOSS),   "Morpha",                         "Water Temple Boss Door",            ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_DUNGEON,   "lh", 1},
    { ENTR_LAKE_HYLIA_WATER_TEMPLE_BLUE_WARP, -1,                                     SINGLE_SCENE_INFO(SCENE_WATER_TEMPLE_BOSS),   "Morpha Blue Warp",               "Water Temple Blue Warp",            ENTRANCE_GROUP_LAKE_HYLIA, ENTRANCE_GROUP_LAKE_HYLIA,   ENTRANCE_TYPE_ONE_WAY,   "lh,bw", 1},

    // Gerudo Area
    { ENTR_HYRULE_FIELD_ROCKY_PATH,                         ENTR_GERUDO_VALLEY_EAST_EXIT,                         SINGLE_SCENE_INFO(SCENE_GERUDO_VALLEY),          "Gerudo Valley East Exit",         "Hyrule Field Rocky Path",         ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_HYRULE_FIELD,      ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_GERUDOS_FORTRESS_EAST_EXIT,                      ENTR_GERUDO_VALLEY_WEST_EXIT,                         SINGLE_SCENE_INFO(SCENE_GERUDO_VALLEY),          "Gerudo Valley West Exit",         "Gerudo Fortress East Exit",       ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_FORTRESS,   ENTRANCE_TYPE_OVERWORLD, ""},
    { ENTR_LAKE_HYLIA_RIVER_EXIT,                           -1,                                                   SINGLE_SCENE_INFO(SCENE_GERUDO_VALLEY),          "Gerudo Valley River Exit",        "Lake Hylia River Exit",           ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_LAKE_HYLIA,        ENTRANCE_TYPE_OVERWORLD, "lh"},
    { ENTR_CARPENTERS_TENT_0,                               ENTR_GERUDO_VALLEY_OUTSIDE_TENT,                      SINGLE_SCENE_INFO(SCENE_GERUDO_VALLEY),          "GV Carpenters' Tent Entry",       "Carpenters' Tent",                ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_GV_OCTOROK_OFFSET),       ENTRANCE_GROTTO_EXIT(GROTTO_GV_OCTOROK_OFFSET),       SINGLE_SCENE_INFO(SCENE_GERUDO_VALLEY),          "GV Silver Rock Grotto Entry",     "GV Octorok Grotto",               ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_GROTTO,    "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_GV_STORMS_OFFSET),        ENTRANCE_GROTTO_EXIT(GROTTO_GV_STORMS_OFFSET),        SINGLE_SCENE_INFO(SCENE_GERUDO_VALLEY),          "GV Behind Tent Grotto Entry",     "GV Deku Scrub Grotto",            ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_GROTTO,    "scrubs", 1},
    { ENTR_GERUDO_VALLEY_OUTSIDE_TENT,                      ENTR_CARPENTERS_TENT_0,                               SINGLE_SCENE_INFO(SCENE_CARPENTERS_TENT),        "Carpenters' Tent",                "GV Carpenters' Tent Entry",       ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_INTERIOR},
    { ENTRANCE_GROTTO_EXIT(GROTTO_GV_OCTOROK_OFFSET),       ENTRANCE_GROTTO_LOAD(GROTTO_GV_OCTOROK_OFFSET),       {{ SCENE_GROTTOS, 0x06 }},                       "GV Octorok Grotto",               "GV Silver Rock Grotto Entry",     ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_GROTTO},
    { ENTRANCE_GROTTO_EXIT(GROTTO_GV_STORMS_OFFSET),        ENTRANCE_GROTTO_LOAD(GROTTO_GV_STORMS_OFFSET),        {{ SCENE_GROTTOS, 0x0A }},                       "GV Deku Scrub Grotto",            "GV Behind Tent Grotto Entry",     ENTRANCE_GROUP_GERUDO_VALLEY,   ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_GROTTO,    "scrubs"},
    { ENTR_GERUDO_VALLEY_WEST_EXIT,                         ENTR_GERUDOS_FORTRESS_EAST_EXIT,                      SINGLE_SCENE_INFO(SCENE_GERUDOS_FORTRESS),       "Gerudo Fortress East Exit",       "Gerudo Valley West Exit",         ENTRANCE_GROUP_GERUDO_FORTRESS, ENTRANCE_GROUP_GERUDO_VALLEY,     ENTRANCE_TYPE_OVERWORLD, ""},
    { ENTR_HAUNTED_WASTELAND_EAST_EXIT,                     ENTR_GERUDOS_FORTRESS_GATE_EXIT,                      SINGLE_SCENE_INFO(SCENE_GERUDOS_FORTRESS),       "Gerudo Fortress Gate Exit",       "Haunted Wasteland East Exit",     ENTRANCE_GROUP_GERUDO_FORTRESS, ENTRANCE_GROUP_HAUNTED_WASTELAND, ENTRANCE_TYPE_OVERWORLD, ""},
    { ENTRANCE_GROTTO_LOAD(GROTTO_GF_STORMS_OFFSET),        ENTRANCE_GROTTO_EXIT(GROTTO_GF_STORMS_OFFSET),        SINGLE_SCENE_INFO(SCENE_GERUDOS_FORTRESS),       "GF Storms Grotto Entry",          "GF Fairy Grotto",                 ENTRANCE_GROUP_GERUDO_FORTRESS, ENTRANCE_GROUP_GERUDO_FORTRESS,   ENTRANCE_TYPE_GROTTO,    "", 1},
    { ENTR_GERUDO_TRAINING_GROUND_ENTRANCE,                 ENTR_GERUDOS_FORTRESS_OUTSIDE_GERUDO_TRAINING_GROUND, SINGLE_SCENE_INFO(SCENE_GERUDOS_FORTRESS),       "GF Outside Training Ground",      "Gerudo Training Ground Entrance", ENTRANCE_GROUP_GERUDO_FORTRESS, ENTRANCE_GROUP_GERUDO_FORTRESS,   ENTRANCE_TYPE_DUNGEON,   "gtg", 1},
    { ENTRANCE_GROTTO_EXIT(GROTTO_GF_STORMS_OFFSET),        ENTRANCE_GROTTO_LOAD(GROTTO_GF_STORMS_OFFSET),        {{ SCENE_FAIRYS_FOUNTAIN, 0x00 }},               "GF Fairy Grotto",                 "GF Storms Grotto Entry",          ENTRANCE_GROUP_GERUDO_FORTRESS, ENTRANCE_GROUP_GERUDO_FORTRESS,   ENTRANCE_TYPE_GROTTO,    ""},
    { ENTR_GERUDOS_FORTRESS_OUTSIDE_GERUDO_TRAINING_GROUND, ENTR_GERUDO_TRAINING_GROUND_ENTRANCE,                 SINGLE_SCENE_INFO(SCENE_GERUDO_TRAINING_GROUND), "Gerudo Training Ground Entrance", "GF Outside Training Ground",      ENTRANCE_GROUP_GERUDO_FORTRESS, ENTRANCE_GROUP_GERUDO_FORTRESS,   ENTRANCE_TYPE_DUNGEON,   "gtg"},

    // The Wasteland
    { ENTR_GERUDOS_FORTRESS_GATE_EXIT,                   ENTR_HAUNTED_WASTELAND_EAST_EXIT,                  SINGLE_SCENE_INFO(SCENE_HAUNTED_WASTELAND),     "Haunted Wasteland East Exit",   "Gerudo Fortress Gate Exit",     ENTRANCE_GROUP_HAUNTED_WASTELAND, ENTRANCE_GROUP_GERUDO_FORTRESS,   ENTRANCE_TYPE_OVERWORLD, "hw,gf"},
    { ENTR_DESERT_COLOSSUS_EAST_EXIT,                    ENTR_HAUNTED_WASTELAND_WEST_EXIT,                  SINGLE_SCENE_INFO(SCENE_HAUNTED_WASTELAND),     "Haunted Wasteland West Exit",   "Desert Colossus East Exit",     ENTRANCE_GROUP_HAUNTED_WASTELAND, ENTRANCE_GROUP_HAUNTED_WASTELAND, ENTRANCE_TYPE_OVERWORLD, "dc,hw"},
    { ENTR_HAUNTED_WASTELAND_WEST_EXIT,                  ENTR_DESERT_COLOSSUS_EAST_EXIT,                    SINGLE_SCENE_INFO(SCENE_DESERT_COLOSSUS),       "Desert Colossus East Exit",     "Haunted Wasteland West Exit",   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_HAUNTED_WASTELAND, ENTRANCE_TYPE_OVERWORLD, "dc,hw"},
    { ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_NAYRUS_COLOSSUS, ENTR_DESERT_COLOSSUS_GREAT_FAIRY_EXIT,             SINGLE_SCENE_INFO(SCENE_DESERT_COLOSSUS),       "Colossus Great Fairy Entry",    "Colossus Great Fairy Fountain", ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_INTERIOR,  "dc", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_COLOSSUS_OFFSET),      ENTRANCE_GROTTO_EXIT(GROTTO_COLOSSUS_OFFSET),      SINGLE_SCENE_INFO(SCENE_DESERT_COLOSSUS),       "Colossus Grotto Entry",         "Colossus Deku Scrub Grotto",    ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_GROTTO,    "dc,scrubs", 1},
    { ENTR_SPIRIT_TEMPLE_ENTRANCE,                       ENTR_DESERT_COLOSSUS_OUTSIDE_TEMPLE,               SINGLE_SCENE_INFO(SCENE_DESERT_COLOSSUS),       "Colossus Outside Temple",       "Spirit Temple Entrance",        ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_DUNGEON,   "dc", 1},
    { ENTR_DESERT_COLOSSUS_GREAT_FAIRY_EXIT,             ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_NAYRUS_COLOSSUS, {{ SCENE_GREAT_FAIRYS_FOUNTAIN_SPELLS, 0x02 }}, "Colossus Great Fairy Fountain", "Colossus Great Fairy Entry",    ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_INTERIOR,  "dc"},
    { ENTRANCE_GROTTO_EXIT(GROTTO_COLOSSUS_OFFSET),      ENTRANCE_GROTTO_LOAD(GROTTO_COLOSSUS_OFFSET),      {{ SCENE_GROTTOS, 0x0A }},                      "Colossus Deku Scrub Grotto",    "Colossus Grotto Entry",         ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_GROTTO,    "dc,scrubs"},
    { ENTR_DESERT_COLOSSUS_OUTSIDE_TEMPLE,               ENTR_SPIRIT_TEMPLE_ENTRANCE,                       SINGLE_SCENE_INFO(SCENE_SPIRIT_TEMPLE),         "Spirit Temple Entrance",        "Colossus Outside Temple",       ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_DUNGEON,   "dc"},
    { ENTR_SPIRIT_TEMPLE_BOSS_ENTRANCE,                  ENTR_SPIRIT_TEMPLE_BOSS_DOOR,                      SINGLE_SCENE_INFO(SCENE_SPIRIT_TEMPLE),         "Spirit Temple Boss Door",       "Twinrova",                      ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_SPIRIT_TEMPLE_BOSS_DOOR,                      ENTR_SPIRIT_TEMPLE_BOSS_ENTRANCE,                  SINGLE_SCENE_INFO(SCENE_SPIRIT_TEMPLE_BOSS),    "Twinrova",                      "Spirit Temple Boss Door",       ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_DUNGEON,   "", 1},
    { ENTR_DESERT_COLOSSUS_SPIRIT_TEMPLE_BLUE_WARP,      -1,                                                SINGLE_SCENE_INFO(SCENE_SPIRIT_TEMPLE_BOSS),    "Twinrova Blue Warp",            "Spirit Temple Blue Warp",       ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_GROUP_DESERT_COLOSSUS,   ENTRANCE_TYPE_ONE_WAY,   "bw", 1},

    // Market
    { ENTR_HYRULE_FIELD_ON_BRIDGE_SPAWN,                  ENTR_MARKET_ENTRANCE_NEAR_GUARD_EXIT,               {SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_DAY), SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_RUINS)},                                                    "Market Entrance South Exit",       "Hyrule Field Drawbridge Exit",     ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_HYRULE_FIELD,  ENTRANCE_TYPE_OVERWORLD, "hf"},
    { ENTR_MARKET_SOUTH_EXIT,                             ENTR_MARKET_ENTRANCE_NORTH_EXIT,                    {SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_DAY), SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_RUINS)},                                                    "Market Entrance North Exit",       "Market South Exit",                ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_OVERWORLD},
    { ENTR_MARKET_GUARD_HOUSE_0,                          ENTR_MARKET_ENTRANCE_OUTSIDE_GUARD_HOUSE,           {SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_DAY), SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_ENTRANCE_RUINS)},                                                    "MK Entrance Guard House Entry",    "Guard House",                      ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "pots,poe", 1},
    { ENTR_MARKET_ENTRANCE_NORTH_EXIT,                    ENTR_MARKET_SOUTH_EXIT,                             {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "Market South Exit",                "Market Entrance North Exit",       ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_OVERWORLD},
    { ENTR_CASTLE_GROUNDS_SOUTH_EXIT,                     ENTR_MARKET_DAY_CASTLE_EXIT,                        {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "Market Castle Exit",               "Castle Grounds South Exit",        ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_OVERWORLD, "outside ganon's castle"},
    { ENTR_TEMPLE_OF_TIME_EXTERIOR_DAY_GOSSIP_STONE_EXIT, ENTR_MARKET_DAY_TEMPLE_EXIT,                        {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "Market Temple Exit",               "ToT Courtyard Gossip Stones Exit", ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_OVERWORLD},
    { ENTR_SHOOTING_GALLERY_1,                            ENTR_MARKET_DAY_OUTSIDE_SHOOTING_GALLERY,           {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Shooting Gallery Entry",        "MK Shooting Gallery",              ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "child", 1},
    { ENTR_BOMBCHU_BOWLING_ALLEY_0,                       ENTR_MARKET_DAY_OUTSIDE_BOMBCHU_BOWLING,            {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Bombchu Bowling Entry",         "Bombchu Bowling",                  ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_TREASURE_BOX_SHOP_0,                           ENTR_MARKET_DAY_OUTSIDE_TREASURE_BOX_SHOP,          {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Treasure Chest Game Entry",     "Treasure Chest Game",              ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_BACK_ALLEY_MAN_IN_GREEN_HOUSE,                 ENTR_BACK_ALLEY_DAY_OUTSIDE_MAN_IN_GREEN_HOUSE,     {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Man-in-Green House Entry",      "Man-in-Green's House",             ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_HAPPY_MASK_SHOP_0,                             ENTR_MARKET_DAY_OUTSIDE_HAPPY_MASK_SHOP,            {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Mask Shop Entry",               "Mask Shop",                        ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_BAZAAR_1,                                      ENTR_MARKET_DAY_OUTSIDE_BAZAAR,                     {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Bazaar Entry",                  "MK Bazaar",                        ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "shop", 1},
    { ENTR_POTION_SHOP_MARKET_0,                          ENTR_MARKET_DAY_OUTSIDE_POTION_SHOP,                {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Potion Shop Entry",             "MK Potion Shop",                   ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_BOMBCHU_SHOP_1,                                ENTR_BACK_ALLEY_DAY_OUTSIDE_BOMBCHU_SHOP,           {SCENE_NO_SPAWN(SCENE_MARKET_DAY), SCENE_NO_SPAWN(SCENE_MARKET_NIGHT), SCENE_NO_SPAWN(SCENE_MARKET_RUINS), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_DAY), SCENE_NO_SPAWN(SCENE_BACK_ALLEY_NIGHT)}, "MK Bombchu Shop Entry",            "Bombchu Shop",                     ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTR_MARKET_ENTRANCE_OUTSIDE_GUARD_HOUSE,           ENTR_MARKET_GUARD_HOUSE_0,                          {{ SCENE_MARKET_GUARD_HOUSE }},                                                                                                                                                           "Guard House",                      "MK Entrance Guard House Entry",    ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "pots,poe"},
    { ENTR_MARKET_DAY_OUTSIDE_SHOOTING_GALLERY,           ENTR_SHOOTING_GALLERY_1,                            {{ SCENE_SHOOTING_GALLERY, 0x01 }},                                                                                                                                                       "MK Shooting Gallery",              "MK Shooting Gallery Entry",        ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_MARKET_DAY_OUTSIDE_BOMBCHU_BOWLING,            ENTR_BOMBCHU_BOWLING_ALLEY_0,                       SINGLE_SCENE_INFO(SCENE_BOMBCHU_BOWLING_ALLEY),                                                                                                                                           "Bombchu Bowling",                  "MK Bombchu Bowling Entry",         ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_MARKET_DAY_OUTSIDE_TREASURE_BOX_SHOP,          ENTR_TREASURE_BOX_SHOP_0,                           SINGLE_SCENE_INFO(SCENE_TREASURE_BOX_SHOP),                                                                                                                                               "Treasure Chest Game",              "MK Treasure Chest Game Entry",     ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_BACK_ALLEY_DAY_OUTSIDE_MAN_IN_GREEN_HOUSE,     ENTR_BACK_ALLEY_MAN_IN_GREEN_HOUSE,                 SINGLE_SCENE_INFO(SCENE_BACK_ALLEY_HOUSE),                                                                                                                                                "Man-in-Green's House",             "MK Man-in-Green House Entry",      ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_MARKET_DAY_OUTSIDE_HAPPY_MASK_SHOP,            ENTR_HAPPY_MASK_SHOP_0,                             SINGLE_SCENE_INFO(SCENE_HAPPY_MASK_SHOP),                                                                                                                                                 "Mask Shop",                        "MK Mask Shop Entry",               ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_MARKET_DAY_OUTSIDE_BAZAAR,                     ENTR_BAZAAR_1,                                      {{ SCENE_BAZAAR, 0x01 }},                                                                                                                                                                 "MK Bazaar",                        "MK Bazaar Entry",                  ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "shop"},
    { ENTR_MARKET_DAY_OUTSIDE_POTION_SHOP,                ENTR_POTION_SHOP_MARKET_0,                          SINGLE_SCENE_INFO(SCENE_POTION_SHOP_MARKET),                                                                                                                                              "MK Potion Shop",                   "MK Potion Shop Entry",             ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_BACK_ALLEY_DAY_OUTSIDE_BOMBCHU_SHOP,           ENTR_BOMBCHU_SHOP_1,                                SINGLE_SCENE_INFO(SCENE_BOMBCHU_SHOP),                                                                                                                                                    "Bombchu Shop",                     "MK Bombchu Shop Entry",            ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR},
    { ENTR_MARKET_DAY_TEMPLE_EXIT,                        ENTR_TEMPLE_OF_TIME_EXTERIOR_DAY_GOSSIP_STONE_EXIT, {SCENE_NO_SPAWN(SCENE_TEMPLE_OF_TIME_EXTERIOR_DAY), SCENE_NO_SPAWN(SCENE_TEMPLE_OF_TIME_EXTERIOR_NIGHT), SCENE_NO_SPAWN(SCENE_TEMPLE_OF_TIME_EXTERIOR_RUINS)},                            "ToT Courtyard Gossip Stones Exit", "Market Temple Exit",               ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_OVERWORLD, "tot"},
    { ENTR_TEMPLE_OF_TIME_ENTRANCE,                       ENTR_TEMPLE_OF_TIME_EXTERIOR_DAY_OUTSIDE_TEMPLE,    {SCENE_NO_SPAWN(SCENE_TEMPLE_OF_TIME_EXTERIOR_DAY), SCENE_NO_SPAWN(SCENE_TEMPLE_OF_TIME_EXTERIOR_NIGHT), SCENE_NO_SPAWN(SCENE_TEMPLE_OF_TIME_EXTERIOR_RUINS)},                            "ToT Courtyard Temple Entry",       "Temple of Time Entrance",          ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "tot", 1},
    { ENTR_TEMPLE_OF_TIME_EXTERIOR_DAY_OUTSIDE_TEMPLE,    ENTR_TEMPLE_OF_TIME_ENTRANCE,                       SINGLE_SCENE_INFO(SCENE_TEMPLE_OF_TIME),                                                                                                                                                  "Temple of Time Entrance",          "ToT Courtyard Temple Entry",       ENTRANCE_GROUP_MARKET, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_INTERIOR,  "tot"},

    // Hyrule Castle
    { ENTR_MARKET_DAY_CASTLE_EXIT,                   ENTR_CASTLE_GROUNDS_SOUTH_EXIT,                {SCENE_NO_SPAWN(SCENE_HYRULE_CASTLE), SCENE_NO_SPAWN(SCENE_OUTSIDE_GANONS_CASTLE)}, "Castle Grounds South Exit",      "Market Castle Exit",             ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_MARKET,        ENTRANCE_TYPE_OVERWORLD, "outside ganon's castle"},
    { ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_DINS_HC,     ENTR_CASTLE_GROUNDS_GREAT_FAIRY_EXIT,          SINGLE_SCENE_INFO(SCENE_HYRULE_CASTLE),                                             "HC Boulder Crawlspace",          "HC Great Fairy Fountain",        ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_INTERIOR,  "", 1},
    { ENTRANCE_GROTTO_LOAD(GROTTO_HC_STORMS_OFFSET), ENTRANCE_GROTTO_EXIT(GROTTO_HC_STORMS_OFFSET), SINGLE_SCENE_INFO(SCENE_HYRULE_CASTLE),                                             "HC Storms Grotto Entry",         "HC Storms Grotto",               ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_GROTTO,    "bombable", 1},
    { ENTR_CASTLE_GROUNDS_GREAT_FAIRY_EXIT,          ENTR_GREAT_FAIRYS_FOUNTAIN_SPELLS_DINS_HC,     {{ SCENE_GREAT_FAIRYS_FOUNTAIN_SPELLS, 0x01 }},                                     "HC Great Fairy Fountain",        "HC Boulder Crawlspace",          ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_INTERIOR},
    { ENTRANCE_GROTTO_EXIT(GROTTO_HC_STORMS_OFFSET), ENTRANCE_GROTTO_LOAD(GROTTO_HC_STORMS_OFFSET), {{ SCENE_GROTTOS, 0x09 }},                                                          "HC Storms Grotto",               "HC Storms Grotto Entry",         ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_GROTTO,    "bombable"},
    { ENTR_GREAT_FAIRYS_FOUNTAIN_MAGIC_OGC_DD,       ENTR_POTION_SHOP_KAKARIKO_1,                   SINGLE_SCENE_INFO(SCENE_OUTSIDE_GANONS_CASTLE),                                     "OGC Behind Pillar",              "OGC Great Fairy Fountain",       ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_INTERIOR,  "outside ganon's castle", 1},
    { ENTR_INSIDE_GANONS_CASTLE_ENTRANCE,            ENTR_CASTLE_GROUNDS_RAINBOW_BRIDGE_EXIT,       SINGLE_SCENE_INFO(SCENE_OUTSIDE_GANONS_CASTLE),                                     "OGC Rainbow Bridge Exit",        "Inside Ganon's Castle Entrance", ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_DUNGEON,   "outside ganon's castle,gc", 1},
    { ENTR_POTION_SHOP_KAKARIKO_1,                   ENTR_GREAT_FAIRYS_FOUNTAIN_MAGIC_OGC_DD,       {{ SCENE_GREAT_FAIRYS_FOUNTAIN_MAGIC, 0x02 }},                                      "OGC Great Fairy Fountain",       "OGC Behind Pillar",              ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_INTERIOR,  "outside ganon's castle"},
    { ENTR_CASTLE_GROUNDS_RAINBOW_BRIDGE_EXIT,       ENTR_INSIDE_GANONS_CASTLE_ENTRANCE,            SINGLE_SCENE_INFO(SCENE_INSIDE_GANONS_CASTLE),                                      "Inside Ganon's Castle Entrance", "OGC Rainbow Bridge Exit",        ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_GROUP_HYRULE_CASTLE, ENTRANCE_TYPE_DUNGEON,   "outside ganon's castle,gc"}

    // clang-format on
};

// Check if Link is in the area and return that scene/entrance for tracking
int16_t LinkIsInArea(const EntranceData* entrance) {
    bool result = false;

    if (gPlayState == nullptr) {
        return -1;
    }

    // Handle detecting the current grotto
    if ((gPlayState->sceneNum == SCENE_FAIRYS_FOUNTAIN || gPlayState->sceneNum == SCENE_GROTTOS) &&
        entrance->type == ENTRANCE_TYPE_GROTTO) {
        if (entrance->index == (ENTRANCE_GROTTO_EXIT_START + currentGrottoId)) {
            // Return the grotto entrance for tracking
            return entrance->index;
        } else {
            return -1;
        }
    }

    // Otherwise check all scenes/spawns
    // Not all areas require a spawn position to differeniate between another area
    for (auto info : entrance->scenes) {
        // When a spawn position is specified, check that combination
        if (info.spawn != -1) {
            result = Entrance_SceneAndSpawnAre(info.scene, info.spawn);
        } else { // Otherwise just check the current scene
            result = gPlayState->sceneNum == info.scene;
        }

        // Return the scene for tracking
        if (result) {
            return info.scene;
        }
    }

    return -1;
}

bool IsEntranceDiscovered(s16 index) {
    bool isDiscovered = Entrance_GetIsEntranceDiscovered(index);
    if (!isDiscovered) {
        // If the pair included one of the hyrule field <-> zora's river entrances,
        // the randomizer will have also overridden the water-based entrances, so check those too
        if ((index == ENTR_ZORAS_RIVER_WEST_EXIT && Entrance_GetIsEntranceDiscovered(ENTR_ZORAS_RIVER_3)) ||
            (index == ENTR_ZORAS_RIVER_3 && Entrance_GetIsEntranceDiscovered(ENTR_ZORAS_RIVER_WEST_EXIT))) {
            isDiscovered = true;
        } else if ((index == ENTR_HYRULE_FIELD_RIVER_EXIT && Entrance_GetIsEntranceDiscovered(ENTR_HYRULE_FIELD_14)) ||
                   (index == ENTR_HYRULE_FIELD_14 && Entrance_GetIsEntranceDiscovered(ENTR_HYRULE_FIELD_RIVER_EXIT))) {
            isDiscovered = true;
        }
    }
    return isDiscovered;
}

const EntranceData* GetEntranceData(s16 index) {
    for (size_t i = 0; i < ARRAY_COUNT(entranceData); i++) {
        if (index == entranceData[i].index) {
            return &entranceData[i];
        }
    }
    // Shouldn't be reached
    return nullptr;
}

void EntranceTracker_LoadFromPreset(nlohmann::json info) {
    presetLoaded = true;
    presetPos = { info["pos"]["x"], info["pos"]["y"] };
    presetSize = { info["size"]["width"], info["size"]["height"] };
}

// Used for verifying the names on both sides of entrance pairs match. Keeping for ease of use for further name changes
// later
// TODO: Figure out how to remove the need for duplicate entrance names so this is no longer necessary
void CheckEntranceNames() {
    SPDLOG_ERROR("Checking entrance names:");
    for (size_t i = 0; i < ARRAY_COUNT(entranceData); i++) {
        auto entrance = &entranceData[i];
        auto reverse = GetEntranceData(entrance->reverseIndex);
        if (entrance != nullptr && reverse != nullptr) {
            if (entrance->source != reverse->destination) {
                SPDLOG_ERROR("{}({}) -> {}({})", entrance->source, entrance->index, reverse->destination,
                             reverse->reverseIndex);
            }
        }
    }
}

void SortEntranceListByType(EntranceOverride* entranceList, u8 byDest) {
    EntranceOverride tempList[ENTRANCE_OVERRIDES_MAX_COUNT] = { 0 };

    for (size_t i = 0; i < ENTRANCE_OVERRIDES_MAX_COUNT; i++) {
        tempList[i] = entranceList[i];
    }

    size_t idx = 0;

    for (size_t k = 0; k < ENTRANCE_TYPE_COUNT; k++) {
        for (size_t i = 0; i < ARRAY_COUNT(entranceData); i++) {
            for (size_t j = 0; j < ENTRANCE_OVERRIDES_MAX_COUNT; j++) {
                if (Entrance_EntranceIsNull(&tempList[j])) {
                    break;
                }

                size_t entranceIndex = byDest ? tempList[j].override : tempList[j].index;

                if (entranceData[i].type == k && entranceIndex == entranceData[i].index) {
                    entranceList[idx] = tempList[j];
                    idx++;
                    break;
                }
            }
        }
    }
}

void SortEntranceListByArea(EntranceOverride* entranceList, u8 byDest) {
    auto entranceCtx = Rando::Context::GetInstance()->GetEntranceShuffler();
    EntranceOverride tempList[ENTRANCE_OVERRIDES_MAX_COUNT] = { 0 };

    // Store to temp
    for (size_t i = 0; i < ENTRANCE_OVERRIDES_MAX_COUNT; i++) {
        tempList[i] = entranceList[i];
        // Don't include one-way indexes in the tempList if we're sorting by destination
        // so that we keep them at the beginning.
        if (byDest) {
            if (GetEntranceData(tempList[i].index)->srcGroup == ENTRANCE_GROUP_ONE_WAY) {
                tempList[i] = emptyOverride;
            }
        }
    }

    size_t idx = 0;
    // Sort Source List based on entranceData order
    if (!byDest) {
        for (size_t i = 0; i < ARRAY_COUNT(entranceData); i++) {
            for (size_t j = 0; j < ENTRANCE_OVERRIDES_MAX_COUNT; j++) {
                if (Entrance_EntranceIsNull(&tempList[j])) {
                    break;
                }
                if (tempList[j].index == entranceData[i].index) {
                    entranceList[idx] = tempList[j];
                    idx++;
                    break;
                }
            }
        }

    } else {
        // Increment the idx by however many one-way entrances are shuffled since these
        // will still be displayed at the beginning
        idx += gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_AREA][ENTRANCE_GROUP_ONE_WAY];

        // Sort the rest of the Destination List by matching destination strings with source strings when possible
        // and otherwise by group
        for (size_t group = ENTRANCE_GROUP_KOKIRI_FOREST; group < SPOILER_ENTRANCE_GROUP_COUNT; group++) {
            for (size_t i = 0; i < ENTRANCE_OVERRIDES_MAX_COUNT; i++) {
                if (Entrance_EntranceIsNull(&entranceCtx->entranceOverrides[i])) {
                    continue;
                }
                const EntranceData* curEntrance = GetEntranceData(entranceCtx->entranceOverrides[i].index);
                if (curEntrance->srcGroup != group) {
                    continue;
                }
                // First, search the list for the matching reverse entrance if it exists
                for (size_t j = 0; j < ENTRANCE_OVERRIDES_MAX_COUNT; j++) {
                    const EntranceData* curOverride = GetEntranceData(tempList[j].override);
                    if (Entrance_EntranceIsNull(&tempList[j]) || curOverride->dstGroup != group) {
                        continue;
                    }

                    if (curEntrance->reverseIndex == curOverride->index) {
                        entranceList[idx] = tempList[j];
                        // "Remove" this entrance from the tempList by setting it's values to zero
                        tempList[j] = emptyOverride;
                        idx++;
                        break;
                    }
                }
            }
            // Then find any remaining entrances in the same group and add them to the end
            for (size_t i = 0; i < ENTRANCE_OVERRIDES_MAX_COUNT; i++) {
                if (Entrance_EntranceIsNull(&tempList[i])) {
                    continue;
                }
                const EntranceData* curOverride = GetEntranceData(tempList[i].override);
                if (curOverride->dstGroup == group) {
                    entranceList[idx] = tempList[i];
                    tempList[i] = emptyOverride;
                    idx++;
                }
            }
        }
    }
}

s16 GetLastEntranceOverride() {
    return lastEntranceIndex;
}

s16 GetCurrentGrottoId() {
    return currentGrottoId;
}

void SetCurrentGrottoIDForTracker(s16 entranceIndex) {
    currentGrottoId = entranceIndex;
}

void SetLastEntranceOverrideForTracker(s16 entranceIndex) {
    lastEntranceIndex = entranceIndex;
}

void ClearEntranceTrackingData() {
    currentGrottoId = -1;
    lastEntranceIndex = -1;
    lastSceneOrEntranceDetected = -1;
    gEntranceTrackingData = { 0 };
}

void InitEntranceTrackingData() {
    auto entranceCtx = Rando::Context::GetInstance()->GetEntranceShuffler();
    gEntranceTrackingData = { 0 };

    // Check if entrance randomization is disabled
    if (!OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_ENTRANCES)) {
        return;
    }

    // Set total and group counts
    for (size_t i = 0; i < ENTRANCE_OVERRIDES_MAX_COUNT; i++) {
        if (Entrance_EntranceIsNull(&entranceCtx->entranceOverrides[i])) {
            break;
        }
        const EntranceData* index = GetEntranceData(entranceCtx->entranceOverrides[i].index);
        const EntranceData* override = GetEntranceData(entranceCtx->entranceOverrides[i].override);

        if (index->srcGroup == ENTRANCE_GROUP_ONE_WAY) {
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_AREA][ENTRANCE_GROUP_ONE_WAY]++;
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_DESTINATION_AREA][ENTRANCE_GROUP_ONE_WAY]++;
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_TYPE][ENTRANCE_TYPE_ONE_WAY]++;
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_DESTINATION_TYPE][ENTRANCE_TYPE_ONE_WAY]++;
        } else {
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_AREA][index->srcGroup]++;
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_DESTINATION_AREA][override->dstGroup]++;
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_TYPE][index->type]++;
            gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_DESTINATION_TYPE][override->type]++;
        }
        gEntranceTrackingData.EntranceCount++;
    }

    // The entrance data is sorted and grouped in a one dimensional array, so we need to track offsets
    // Set offsets for areas starting at 0
    u16 srcOffsetTotal = 0;
    u16 dstOffsetTotal = 0;
    for (size_t i = 0; i < SPOILER_ENTRANCE_GROUP_COUNT; i++) {
        // Set the offset for the current group
        gEntranceTrackingData.GroupOffsets[ENTRANCE_SOURCE_AREA][i] = srcOffsetTotal;
        gEntranceTrackingData.GroupOffsets[ENTRANCE_DESTINATION_AREA][i] = dstOffsetTotal;
        // Increment the offset by the areas entrance count
        srcOffsetTotal += gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_AREA][i];
        dstOffsetTotal += gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_DESTINATION_AREA][i];
    }
    // Set offsets for types starting at 0
    srcOffsetTotal = 0;
    dstOffsetTotal = 0;
    for (size_t i = 0; i < ENTRANCE_TYPE_COUNT; i++) {
        // Set the offset for the current group
        gEntranceTrackingData.GroupOffsets[ENTRANCE_SOURCE_TYPE][i] = srcOffsetTotal;
        gEntranceTrackingData.GroupOffsets[ENTRANCE_DESTINATION_TYPE][i] = dstOffsetTotal;
        // Increment the offset by the areas entrance count
        srcOffsetTotal += gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_SOURCE_TYPE][i];
        dstOffsetTotal += gEntranceTrackingData.GroupEntranceCounts[ENTRANCE_DESTINATION_TYPE][i];
    }

    // Sort entrances by group and type in entranceData
    for (size_t i = 0; i < ENTRANCE_OVERRIDES_MAX_COUNT; i++) {
        srcListSortedByArea[i] = entranceCtx->entranceOverrides[i];
        destListSortedByArea[i] = entranceCtx->entranceOverrides[i];
        srcListSortedByType[i] = entranceCtx->entranceOverrides[i];
        destListSortedByType[i] = entranceCtx->entranceOverrides[i];
    }
    SortEntranceListByArea(srcListSortedByArea, 0);
    SortEntranceListByArea(destListSortedByArea, 1);
    SortEntranceListByType(srcListSortedByType, 0);
    SortEntranceListByType(destListSortedByType, 1);
}

void EntranceTrackerSettingsWindow::DrawElement() {

    ImGui::TextWrapped("The entrance tracker will only track shuffled entrances");
    UIWidgets::Spacer(0);

    ImGui::TableNextColumn();

    if (ImGui::BeginTable("entranceTrackerSubSettings", 2,
                          ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("column 1", ImGuiTableColumnFlags_WidthStretch, 150.0f);
        ImGui::TableSetupColumn("column 2", ImGuiTableColumnFlags_WidthStretch, 150.0f);

        ImGui::TableNextColumn();

        ImGui::Text("Sort By");
        UIWidgets::CVarRadioButton("To", CVAR_TRACKER_ENTRANCE("SortBy"), 0,
                                   UIWidgets::RadioButtonsOptions()
                                       .Color(THEME_COLOR)
                                       .Tooltip("Sort entrances by the original source entrance"));
        UIWidgets::CVarRadioButton(
            "From", CVAR_TRACKER_ENTRANCE("SortBy"), 1,
            UIWidgets::RadioButtonsOptions().Color(THEME_COLOR).Tooltip("Sort entrances by the overrided destination"));

        ImGui::Text("List Items");
        UIWidgets::CVarCheckbox(
            "Auto scroll", CVAR_TRACKER_ENTRANCE("AutoScroll"),
            UIWidgets::CheckboxOptions()
                .Tooltip("Automatically scroll to the first available entrance in the current scene")
                .Color(THEME_COLOR));
        ImGui::BeginDisabled(CVarGetInteger(CVAR_SETTING("DisableChanges"), 0));
        UIWidgets::CVarCheckbox("Highlight previous", CVAR_TRACKER_ENTRANCE("HighlightPrevious"),
                                UIWidgets::CheckboxOptions()
                                    .Tooltip("Highlight the previous entrance that Link came from")
                                    .Color(THEME_COLOR));
        UIWidgets::CVarCheckbox("Highlight available", CVAR_TRACKER_ENTRANCE("HighlightAvailable"),
                                UIWidgets::CheckboxOptions()
                                    .Tooltip("Highlight available entrances in the current scene")
                                    .Color(THEME_COLOR));
        ImGui::EndDisabled();
        UIWidgets::CVarCheckbox("Hide undiscovered", CVAR_TRACKER_ENTRANCE("CollapseUndiscovered"),
                                UIWidgets::CheckboxOptions()
                                    .Tooltip("Collapse undiscovered entrances towards the bottom of each group")
                                    .Color(THEME_COLOR));
        bool disableHideReverseEntrances =
            OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_DECOUPLED_ENTRANCES) == RO_GENERIC_ON;
        static const char* disableHideReverseEntrancesText =
            "This option is disabled because \"Decouple Entrances\" is enabled.";
        UIWidgets::CVarCheckbox("Hide reverse", CVAR_TRACKER_ENTRANCE("HideReverseEntrances"),
                                UIWidgets::CheckboxOptions({ { .disabled = disableHideReverseEntrances,
                                                               .disabledTooltip = disableHideReverseEntrancesText } })
                                    .Tooltip("Hide reverse entrance transitions when Decouple Entrances is off")
                                    .DefaultValue(true)
                                    .Color(THEME_COLOR));

        ImGui::TableNextColumn();

        ImGui::Text("Group By");
        UIWidgets::CVarRadioButton(
            "Area", CVAR_TRACKER_ENTRANCE("GroupBy"), 0,
            UIWidgets::RadioButtonsOptions().Color(THEME_COLOR).Tooltip("Group entrances by their area"));
        UIWidgets::CVarRadioButton(
            "Type", CVAR_TRACKER_ENTRANCE("GroupBy"), 1,
            UIWidgets::RadioButtonsOptions().Color(THEME_COLOR).Tooltip("Group entrances by their entrance type"));

        ImGui::Text("Spoiler Reveal");
        ImGui::BeginDisabled(CVarGetInteger(CVAR_SETTING("DisableChanges"), 0));
        UIWidgets::CVarCheckbox(
            "Show Source", CVAR_TRACKER_ENTRANCE("ShowFrom"),
            UIWidgets::CheckboxOptions().Tooltip("Reveal the sourcefor undiscovered entrances").Color(THEME_COLOR));
        UIWidgets::CVarCheckbox("Show Destination", CVAR_TRACKER_ENTRANCE("ShowTo"),
                                UIWidgets::CheckboxOptions()
                                    .Tooltip("Reveal the destination for undiscovered entrances")
                                    .Color(THEME_COLOR));
        ImGui::EndDisabled();
        ImGui::EndTable();
    }

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("Legend")) {
        ImGui::TextColored(ImColor(COLOR_ORANGE), "Last Entrance");
        ImGui::TextColored(ImColor(COLOR_GREEN), "Available Entrances");
        ImGui::TextColored(ImColor(COLOR_GRAY), "Undiscovered Entrances");
        ImGui::TreePop();
    }
}

void EntranceTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
}

void EntranceTrackerWindow::DrawElement() {
    if (presetLoaded) {
        ImGui::SetNextWindowSize(presetSize);
        ImGui::SetNextWindowPos(presetPos);
        presetLoaded = false;
    } else {
        ImGui::SetNextWindowSize(ImVec2(600, 375), ImGuiCond_FirstUseEver);
    }

    if (!ImGui::Begin("Entrance Tracker", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        ImGui::End();
        return;
    }

    static ImGuiTextFilter locationSearch;

    uint8_t nextTreeState = 0;
    if (UIWidgets::Button("Collapse All", UIWidgets::ButtonOptions({ { .tooltip = "Collapse all entrance groups" } })
                                              .Color(THEME_COLOR)
                                              .Size(UIWidgets::Sizes::Inline))) {
        nextTreeState = 1;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Expand All", UIWidgets::ButtonOptions({ { .tooltip = "Expand all entrance groups" } })
                                            .Color(THEME_COLOR)
                                            .Size(UIWidgets::Sizes::Inline))) {
        nextTreeState = 2;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Clear", UIWidgets::ButtonOptions({ { .tooltip = "Clear the search field" } })
                                       .Color(THEME_COLOR)
                                       .Size(UIWidgets::Sizes::Inline))) {
        locationSearch.Clear();
    }

    UIWidgets::PushStyleCombobox(THEME_COLOR);
    if (locationSearch.Draw()) {
        nextTreeState = 2;
    }
    UIWidgets::PopStyleCombobox();

    uint8_t destToggle = CVarGetInteger(CVAR_TRACKER_ENTRANCE("SortBy"), 0);
    uint8_t groupToggle = CVarGetInteger(CVAR_TRACKER_ENTRANCE("GroupBy"), 0);

    // Combine destToggle and groupToggle to get a range of 0-3
    uint8_t groupType = destToggle + (groupToggle * 2);
    size_t groupCount = groupToggle ? ENTRANCE_TYPE_COUNT : SPOILER_ENTRANCE_GROUP_COUNT;
    auto groupNames = groupToggle ? groupTypeNames : spoilerEntranceGroupNames;

    EntranceOverride* entranceList;

    switch (groupType) {
        case ENTRANCE_SOURCE_AREA:
            entranceList = srcListSortedByArea;
            break;
        case ENTRANCE_DESTINATION_AREA:
            entranceList = destListSortedByArea;
            break;
        case ENTRANCE_SOURCE_TYPE:
            entranceList = srcListSortedByType;
            break;
        case ENTRANCE_DESTINATION_TYPE:
            entranceList = destListSortedByType;
            break;
    }

    // Begin tracker list
    ImGui::BeginChild("ChildEntranceTrackerLocations", ImVec2(0, -8));
    bool showTo = CVarGetInteger(CVAR_TRACKER_ENTRANCE("ShowTo"), 0);
    bool showFrom = CVarGetInteger(CVAR_TRACKER_ENTRANCE("ShowFrom"), 0);
    bool collapsUndiscovered = CVarGetInteger(CVAR_TRACKER_ENTRANCE("CollapseUndiscovered"), 0);
    bool highlightPrevious = CVarGetInteger(CVAR_TRACKER_ENTRANCE("HighlightPrevious"), 0);
    bool highlightAvailable = CVarGetInteger(CVAR_TRACKER_ENTRANCE("HighlightAvailable"), 0);
    bool hideReverse = CVarGetInteger(CVAR_TRACKER_ENTRANCE("HideReverseEntrances"), 1);
    bool autoScrollArea = CVarGetInteger(CVAR_TRACKER_ENTRANCE("AutoScroll"), 0);
    for (size_t i = 0; i < groupCount; i++) {
        std::string groupName = groupNames[i];

        uint16_t entranceCount = gEntranceTrackingData.GroupEntranceCounts[groupType][i];
        uint16_t startIndex = gEntranceTrackingData.GroupOffsets[groupType][i];

        bool doAreaScroll = false;
        size_t undiscovered = 0;
        std::vector<EntranceOverride> displayEntrances = {};

        // Loop over entrances first for filtering
        for (size_t entranceIdx = 0; entranceIdx < entranceCount; entranceIdx++) {
            size_t trueIdx = entranceIdx + startIndex;

            EntranceOverride entrance = entranceList[trueIdx];

            const EntranceData* original = GetEntranceData(entrance.index);
            const EntranceData* override = GetEntranceData(entrance.override);

            // If entrance is a dungeon, grotto, or interior entrance, the transition into that area has oneExit set,
            // which means we can filter the return transitions as redundant if entrances are not decoupled, as this is
            // redundant information. Also checks a setting, enabled by default, for hiding them. If all of these
            // conditions are met, we skip adding this entrance to any lists. However, if entrances are decoupled, then
            // all transitions need to be displayed, so we proceed with the filtering
            if ((original->type == ENTRANCE_TYPE_DUNGEON || original->type == ENTRANCE_TYPE_GROTTO ||
                 original->type == ENTRANCE_TYPE_INTERIOR) &&
                (original->oneExit != 1 &&
                 OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_DECOUPLED_ENTRANCES) == RO_GENERIC_OFF) &&
                hideReverse == 1) {
                continue;
            }

            // RANDOTODO: Only show blue warps if bluewarp shuffle is on
            if (original->metaTag.ends_with("bw") || override->metaTag.ends_with("bw")) {
                continue;
            }

            bool isDiscovered = IsEntranceDiscovered(entrance.index);

            bool showOverride = (!destToggle ? showTo : showFrom) || isDiscovered;
            bool showOriginal = (!destToggle ? showFrom : showTo) || isDiscovered;

            const char* origSrcAreaName = spoilerEntranceGroupNames[original->srcGroup].c_str();
            const char* origTypeName = groupTypeNames[original->type].c_str();
            const char* rplcSrcAreaName = spoilerEntranceGroupNames[override->srcGroup].c_str();
            const char* rplcTypeName = groupTypeNames[override->type].c_str();

            const char* origSrcName = showOriginal ? original->source.c_str() : "";
            const char* rplcDstName = showOverride ? override->destination.c_str() : "";

            // Filter for entrances by group name, type, source/destination names, and meta tags
            if ((!locationSearch.IsActive() && (showOriginal || showOverride || !collapsUndiscovered)) ||
                ((showOriginal &&
                  (locationSearch.PassFilter(origSrcName) || locationSearch.PassFilter(origSrcAreaName) ||
                   locationSearch.PassFilter(origTypeName) || locationSearch.PassFilter(original->metaTag.c_str()))) ||
                 (showOverride &&
                  (locationSearch.PassFilter(rplcDstName) || locationSearch.PassFilter(rplcSrcAreaName) ||
                   locationSearch.PassFilter(rplcTypeName) || locationSearch.PassFilter(override->metaTag.c_str()))))) {

                // Detect if a scroll should happen and remember the scene for that scroll
                if (!doAreaScroll &&
                    (lastSceneOrEntranceDetected != LinkIsInArea(original) && LinkIsInArea(original) != -1)) {
                    lastSceneOrEntranceDetected = LinkIsInArea(original);
                    doAreaScroll = true;
                }

                displayEntrances.push_back(entrance);
            } else {
                if (!isDiscovered) {
                    undiscovered++;
                }
            }
        }

        // Then display the entrances in groups
        if (displayEntrances.size() != 0 || (!locationSearch.IsActive() && undiscovered > 0)) {
            // Handle opening/closing trees based on auto scroll or collapse/expand buttons
            if (nextTreeState == 1) {
                ImGui::SetNextItemOpen(false, ImGuiCond_None);
            } else {
                ImGui::SetNextItemOpen(true, nextTreeState == 0 && !doAreaScroll ? ImGuiCond_Once : ImGuiCond_None);
            }

            if (ImGui::TreeNode(groupName.c_str())) {
                for (auto entrance : displayEntrances) {
                    const EntranceData* original = GetEntranceData(entrance.index);
                    const EntranceData* override = GetEntranceData(entrance.override);

                    bool isDiscovered = IsEntranceDiscovered(entrance.index);

                    bool showOverride = (!destToggle ? showTo : showFrom) || isDiscovered;
                    bool showOriginal = (!destToggle ? showFrom : showTo) || isDiscovered;

                    const char* unknown = "???";

                    const char* origSrcName = showOriginal ? original->source.c_str() : unknown;
                    const char* rplcDstName = showOverride ? override->destination.c_str() : unknown;

                    uint32_t color = isDiscovered ? IM_COL32_WHITE : COLOR_GRAY;

                    // Handle highlighting and auto scroll
                    if ((original->index == lastEntranceIndex ||
                         (override->reverseIndex == lastEntranceIndex &&
                          OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_DECOUPLED_ENTRANCES) ==
                              RO_GENERIC_OFF)) &&
                        highlightPrevious) {
                        color = COLOR_ORANGE;
                    } else if (LinkIsInArea(original) != -1) {
                        if (highlightAvailable) {
                            color = COLOR_GREEN;
                        }

                        if (doAreaScroll) {
                            doAreaScroll = false;
                            if (autoScrollArea) {
                                ImGui::SetScrollHereY(0.0f);
                            }
                        }
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, color);

                    // Use a non-breaking space to keep the arrow from wrapping to a newline by itself
                    auto nbsp = u8"\u00A0";
                    ImGui::TextWrapped("%s%s-> %s", origSrcName, nbsp, rplcDstName);

                    ImGui::PopStyleColor();
                }

                // Write collapsed undiscovered info
                if (!locationSearch.IsActive() && undiscovered > 0) {
                    UIWidgets::Spacer(0);
                    ImGui::PushStyleColor(ImGuiCol_Text, COLOR_GRAY);
                    ImGui::TextWrapped("%d Undiscovered", undiscovered);
                    ImGui::PopStyleColor();
                }

                UIWidgets::Spacer(0);
                ImGui::TreePop();
            }
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void EntranceTrackerWindow::InitElement() {
    // Setup hooks for loading and clearing the entrance tracker data
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { InitEntranceTrackingData(); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnExitGame>(
        [](int32_t fileNum) { ClearEntranceTrackingData(); });
}
