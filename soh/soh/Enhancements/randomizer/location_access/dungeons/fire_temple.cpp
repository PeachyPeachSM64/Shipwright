#include "soh/Enhancements/randomizer/location_access.h"
#include "soh/Enhancements/randomizer/entrance.h"
#include "soh/Enhancements/randomizer/dungeon.h"

using namespace Rando;

void RegionTable_Init_FireTemple() {
    // clang-format off
    // Vanilla/MQ Decider
    areaTable[RR_FIRE_TEMPLE_ENTRYWAY] = Region("Fire Temple Entryway", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRST_ROOM,          []{return ctx->GetDungeon(FIRE_TEMPLE)->IsVanilla();}),
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER, []{return ctx->GetDungeon(FIRE_TEMPLE)->IsMQ();}),
        Entrance(RR_DMC_CENTRAL_LOCAL,               []{return true;}),
    });

#pragma region Vanilla

    areaTable[RR_FIRE_TEMPLE_FIRST_ROOM] = Region("Fire Temple First Room", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_ENTRYWAY,       []{return true;}),
        Entrance(RR_FIRE_TEMPLE_NEAR_BOSS_ROOM, []{return logic->FireTimer() >= 24;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_ENEMIES,   []{return Here(RR_FIRE_TEMPLE_FIRST_ROOM, []{return logic->CanUse(RG_MEGATON_HAMMER);}) && (logic->SmallKeys(RR_FIRE_TEMPLE, 8) || !logic->IsFireLoopLocked);}),
        Entrance(RR_FIRE_TEMPLE_LOOP_EXIT,      []{return true;}),
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM,  []{return logic->SmallKeys(RR_FIRE_TEMPLE, 2) && logic->FireTimer() >= 24;}),
    });

    areaTable[RR_FIRE_TEMPLE_NEAR_BOSS_ROOM] = Region("Fire Temple Near Boss Room", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->FairyPot, []{return logic->CanUse(RG_HOVER_BOOTS) || logic->CanUse(RG_HOOKSHOT);}),
    }, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_NEAR_BOSS_CHEST, true),
        LOCATION(RC_FIRE_TEMPLE_NEAR_BOSS_POT_1, (logic->CanBreakPots() && (logic->CanUse(RG_HOVER_BOOTS) || logic->CanUse(RG_HOOKSHOT)))),
        LOCATION(RC_FIRE_TEMPLE_NEAR_BOSS_POT_2, (logic->CanBreakPots() && (logic->CanUse(RG_HOVER_BOOTS) || logic->CanUse(RG_HOOKSHOT)))),
        LOCATION(RC_FIRE_TEMPLE_NEAR_BOSS_POT_3, (logic->CanBreakPots() && (logic->CanUse(RG_HOVER_BOOTS) || logic->CanUse(RG_HOOKSHOT)))),
        LOCATION(RC_FIRE_TEMPLE_NEAR_BOSS_POT_4, (logic->CanBreakPots() && (logic->CanUse(RG_HOVER_BOOTS) || logic->CanUse(RG_HOOKSHOT)))),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRST_ROOM,    []{return true;}),
        Entrance(RR_FIRE_TEMPLE_BOSS_ENTRYWAY, []{return logic->IsAdult && (ctx->GetTrickOption(RT_FIRE_BOSS_DOOR_JUMP) || Here(RR_FIRE_TEMPLE_FIRE_MAZE_UPPER, []{return logic->CanUse(RG_MEGATON_HAMMER);}) || logic->CanUse(RG_HOVER_BOOTS));}),
    });

    areaTable[RR_FIRE_TEMPLE_LOOP_ENEMIES] = Region("Fire Temple Loop Enemies", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRST_ROOM, []{return logic->SmallKeys(RR_FIRE_TEMPLE, 8) || !logic->IsFireLoopLocked;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_TILES, []{return Here(RR_FIRE_TEMPLE_LOOP_ENEMIES, []{return logic->CanKillEnemy(RE_TORCH_SLUG) && logic->CanKillEnemy(RE_FIRE_KEESE);});}),
    });

    areaTable[RR_FIRE_TEMPLE_LOOP_TILES] = Region("Fire Temple Loop Tiles", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_GS_BOSS_KEY_LOOP, logic->CanAttack()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_LOOP_ENEMIES,      []{return true;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_FLARE_DANCER, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_LOOP_FLARE_DANCER] = Region("Fire Temple Loop Flare Dancer", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_FLARE_DANCER_CHEST, (logic->HasExplosives() || logic->CanUse(RG_MEGATON_HAMMER)) && logic->IsAdult),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_LOOP_TILES,         []{return true;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_HAMMER_SWITCH, []{return Here(RR_FIRE_TEMPLE_LOOP_FLARE_DANCER, []{return logic->CanKillEnemy(RE_FLARE_DANCER);});}),
    });

    areaTable[RR_FIRE_TEMPLE_LOOP_HAMMER_SWITCH] = Region("Fire Temple Loop Hammer Switch", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->FireLoopSwitch, []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    }, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_LOOP_FLARE_DANCER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_GORON_ROOM,   []{return logic->FireLoopSwitch;}),
    });

    areaTable[RR_FIRE_TEMPLE_LOOP_GORON_ROOM] = Region("Fire Temple Loop Goron Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BOSS_KEY_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_LOOP_HAMMER_SWITCH, []{return logic->FireLoopSwitch;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_EXIT,          []{return logic->FireLoopSwitch;}),
    });

    areaTable[RR_FIRE_TEMPLE_LOOP_EXIT] = Region("Fire Temple Loop Exit", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRST_ROOM,      []{return true;}),
        Entrance(RR_FIRE_TEMPLE_LOOP_GORON_ROOM, []{return logic->FireLoopSwitch;}),
    });

    areaTable[RR_FIRE_TEMPLE_BIG_LAVA_ROOM] = Region("Fire Temple Big Lava Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BIG_LAVA_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_BIG_LAVA_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_BIG_LAVA_POT_3, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRST_ROOM,                []{return logic->SmallKeys(RR_FIRE_TEMPLE, 2);}),
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM_NORTH_GORON, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM_NORTH_TILES, []{return logic->IsAdult && (logic->CanUse(RG_SONG_OF_TIME) || ctx->GetTrickOption(RT_FIRE_SOT));}),
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM_SOUTH_GORON, []{return logic->IsAdult && logic->HasExplosives();}),
        Entrance(RR_FIRE_TEMPLE_FIRE_PILLAR_ROOM,          []{return logic->SmallKeys(RR_FIRE_TEMPLE, 3);}),
    });

    areaTable[RR_FIRE_TEMPLE_BIG_LAVA_ROOM_NORTH_GORON] = Region("Fire Temple Big Lava Room North Goron", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BIG_LAVA_ROOM_LOWER_OPEN_DOOR_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_BIG_LAVA_ROOM_NORTH_TILES] = Region("Fire Temple Big Lava Room North Tiles", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //RANDOTODO check if child can reach
        LOCATION(RC_FIRE_TEMPLE_GS_SONG_OF_TIME_ROOM, (logic->IsAdult && logic->CanAttack()) || logic->HookshotOrBoomerang()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_BIG_LAVA_ROOM_SOUTH_GORON] = Region("Fire Temple Big Lava Room South Goron", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BIG_LAVA_ROOM_BLOCKED_DOOR_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_FIRE_PILLAR_ROOM] = Region("Fire Temple Fire Pillar Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_FIRE_PILLAR_LEFT_HEART,  logic->FireTimer() >= 56),
        LOCATION(RC_FIRE_TEMPLE_FIRE_PILLAR_RIGHT_HEART, logic->FireTimer() >= 56),
        LOCATION(RC_FIRE_TEMPLE_FIRE_PILLAR_BACK_HEART,  logic->FireTimer() >= 56),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM, []{return logic->SmallKeys(RR_FIRE_TEMPLE, 3);}),
        Entrance(RR_FIRE_TEMPLE_SHORTCUT_ROOM, []{return logic->FireTimer() >= 56 && logic->SmallKeys(RR_FIRE_TEMPLE, 4);}),
    });

    areaTable[RR_FIRE_TEMPLE_SHORTCUT_ROOM] = Region("Fire Temple Shortcut Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BOULDER_MAZE_SHORTCUT_CHEST, Here(RR_FIRE_TEMPLE_SHORTCUT_CLIMB, []{return true;})),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRE_PILLAR_ROOM,   []{return logic->SmallKeys(RR_FIRE_TEMPLE, 4);}),
        Entrance(RR_FIRE_TEMPLE_SHORTCUT_CLIMB,     []{return Here(RR_FIRE_TEMPLE_SHORTCUT_CLIMB, []{return true;});}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER, []{return logic->IsAdult && (logic->HasItem(RG_GORONS_BRACELET) || ctx->GetTrickOption(RT_FIRE_STRENGTH)) && (logic->HasExplosives() || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_FAIRY_SLINGSHOT));}),
    });

    areaTable[RR_FIRE_TEMPLE_SHORTCUT_CLIMB] = Region("Fire Temple Shortcut Climb", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_SHORTCUT_ROOM,      []{return true;}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_UPPER, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER] = Region("Fire Temple Boulder Maze Lower", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BOULDER_MAZE_LOWER_CHEST, true),
        LOCATION(RC_FIRE_TEMPLE_GS_BOULDER_MAZE,          logic->HasExplosives() && (logic->IsAdult || logic->HookshotOrBoomerang())),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_SHORTCUT_ROOM,                []{return true;}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER_SIDE_ROOM, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_EAST_CENTRAL_ROOM,            []{return logic->SmallKeys(RR_FIRE_TEMPLE, 5, 7);}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_UPPER,           []{return false;}),
    });

    areaTable[RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER_SIDE_ROOM] = Region("Fire Temple Boulder Maze Lower Side Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BOULDER_MAZE_SIDE_ROOM_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_EAST_CENTRAL_ROOM] = Region("Fire Temple East Central Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_EAST_CENTRAL_LEFT_HEART,   true),
        LOCATION(RC_FIRE_TEMPLE_EAST_CENTRAL_RIGHT_HEART,  true),
        LOCATION(RC_FIRE_TEMPLE_EAST_CENTRAL_MIDDLE_HEART, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BIG_LAVA_ROOM,      []{return logic->TakeDamage();}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER, []{return logic->SmallKeys(RR_FIRE_TEMPLE, 5, 8);}),
        Entrance(RR_FIRE_TEMPLE_FIRE_WALL_CHASE,    []{return logic->SmallKeys(RR_FIRE_TEMPLE, 6, 8);}),
        Entrance(RR_FIRE_TEMPLE_MAP_AREA,           []{return logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->CanUse(RG_FAIRY_BOW);}),
    });

    areaTable[RR_FIRE_TEMPLE_FIRE_WALL_CHASE] = Region("Fire Temple Fire Wall Chase", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_FIRE_WALL_EAST_HEART, logic->FireTimer() >= 24 && (logic->IsAdult || logic->CanUse(RG_BOOMERANG))),
        LOCATION(RC_FIRE_TEMPLE_FIRE_WALL_WEST_HEART, logic->FireTimer() >= 24 && (logic->IsAdult || logic->CanUse(RG_BOOMERANG))),
        LOCATION(RC_FIRE_TEMPLE_FIRE_WALL_EXIT_HEART, logic->FireTimer() >= 24),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_EAST_CENTRAL_ROOM,  []{return logic->FireTimer() >= 24 && logic->SmallKeys(RR_FIRE_TEMPLE, 6, 8);}),
        Entrance(RR_FIRE_TEMPLE_MAP_AREA,           []{return logic->IsAdult;}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_UPPER, []{return logic->FireTimer() >= 24 && logic->IsAdult;}),
        Entrance(RR_FIRE_TEMPLE_CORRIDOR,           []{return logic->FireTimer() >= 24 && logic->IsAdult && logic->SmallKeys(RR_FIRE_TEMPLE, 7);}),
    });

    areaTable[RR_FIRE_TEMPLE_MAP_AREA] = Region("Fire Temple Map Region", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MAP_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_EAST_CENTRAL_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_BOULDER_MAZE_UPPER] = Region("Fire Temple Boulder Maze Upper", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_BOULDER_MAZE_UPPER_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_SHORTCUT_CLIMB,     []{return logic->HasExplosives();}),
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_LOWER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_FIRE_WALL_CHASE,    []{return true;}),
        Entrance(RR_FIRE_TEMPLE_SCARECROW_ROOM,     []{return logic->CanUse(RG_SCARECROW) || (ctx->GetTrickOption(RT_FIRE_SCARECROW) && logic->IsAdult && logic->CanUse(RG_LONGSHOT));}),
    });

    areaTable[RR_FIRE_TEMPLE_SCARECROW_ROOM] = Region("Fire Temple Scarecrow Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_GS_SCARECROW_CLIMB, logic->CanJumpslashExceptHammer() || logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->CanUse(RG_BOOMERANG) || logic->HasExplosives() || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_DINS_FIRE)),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BOULDER_MAZE_UPPER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_EAST_PEAK,          []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_EAST_PEAK] = Region("Fire Temple East Peak", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_SCARECROW_CHEST,  true),
        LOCATION(RC_FIRE_TEMPLE_GS_SCARECROW_TOP, logic->CanUseProjectile()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_SCARECROW_ROOM,    []{return true;}),
        Entrance(RR_FIRE_TEMPLE_EAST_CENTRAL_ROOM, []{return logic->TakeDamage();}),
    });

    areaTable[RR_FIRE_TEMPLE_CORRIDOR] = Region("Fire Temple Corridor", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRE_WALL_CHASE, []{return logic->SmallKeys(RR_FIRE_TEMPLE, 7);}),
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_ROOM,  []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_FIRE_MAZE_ROOM] = Region("Fire Temple Fire Maze Room", SCENE_FIRE_TEMPLE, {}, {
        // Locations
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_LEFT_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_LEFT_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_LEFT_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_LEFT_POT_4, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_CORRIDOR,            []{return true;}),
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_UPPER,     []{return logic->CanUse(RG_HOVER_BOOTS);}),
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_SIDE_ROOM, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_WEST_CENTRAL_LOWER,  []{return logic->SmallKeys(RR_FIRE_TEMPLE, 8);}),
        Entrance(RR_FIRE_TEMPLE_LATE_FIRE_MAZE,      []{return ctx->GetTrickOption(RT_FIRE_FLAME_MAZE) || false;}),
    });

    areaTable[RR_FIRE_TEMPLE_FIRE_MAZE_UPPER] = Region("Fire Temple Fire Maze Upper", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_NEAR_BOSS_ROOM,     []{return logic->CanUse(RG_MEGATON_HAMMER);}),
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_ROOM,     []{return true;}),
        Entrance(RR_FIRE_TEMPLE_WEST_CENTRAL_UPPER, []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    });

    areaTable[RR_FIRE_TEMPLE_FIRE_MAZE_SIDE_ROOM] = Region("Fire Temple Fire Maze Side Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_COMPASS_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_WEST_CENTRAL_LOWER] = Region("Fire Temple West Central Lower", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_HIGHEST_GORON_CHEST, Here(RR_FIRE_TEMPLE_WEST_CENTRAL_UPPER, []{return (logic->CanUse(RG_SONG_OF_TIME) || ctx->GetTrickOption(RT_RUSTED_SWITCHES)) && logic->CanUse(RG_MEGATON_HAMMER);})),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_ROOM,     []{return logic->SmallKeys(RR_FIRE_TEMPLE, 8);}),
        Entrance(RR_FIRE_TEMPLE_WEST_CENTRAL_UPPER, []{return logic->IsAdult && logic->CanUse(RG_SONG_OF_TIME);}),
        Entrance(RR_FIRE_TEMPLE_LATE_FIRE_MAZE,     []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_WEST_CENTRAL_UPPER] = Region("Fire Temple West Central Upper", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_BOSS_ENTRYWAY,      []{return false;}),
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_UPPER,    []{return true;}),
        Entrance(RR_FIRE_TEMPLE_WEST_CENTRAL_LOWER, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_LATE_FIRE_MAZE] = Region("Fire Temple Late Fire Maze", SCENE_FIRE_TEMPLE, {}, {
        // Locations
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_RIGHT_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_RIGHT_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_RIGHT_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_FLAME_MAZE_RIGHT_POT_4, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_ROOM,     []{return false;}),
        Entrance(RR_FIRE_TEMPLE_WEST_CENTRAL_LOWER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_UPPER_FLARE_DANCER, []{return logic->HasExplosives();}),
    });

    areaTable[RR_FIRE_TEMPLE_UPPER_FLARE_DANCER] = Region("Fire Temple Upper Flare Dancer", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_LATE_FIRE_MAZE, []{return Here(RR_FIRE_TEMPLE_UPPER_FLARE_DANCER, []{return logic->CanKillEnemy(RE_FLARE_DANCER);});}),
        Entrance(RR_FIRE_TEMPLE_WEST_CLIMB,     []{return Here(RR_FIRE_TEMPLE_UPPER_FLARE_DANCER, []{return logic->CanKillEnemy(RE_FLARE_DANCER);});}),
    });

    areaTable[RR_FIRE_TEMPLE_WEST_CLIMB] = Region("Fire Temple West Climb", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_UPPER_FLARE_DANCER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_WEST_PEAK,          []{return logic->CanUseProjectile();}),
    });

    areaTable[RR_FIRE_TEMPLE_WEST_PEAK] = Region("Fire Temple West Peak", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MEGATON_HAMMER_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_WEST_CENTRAL_UPPER, []{return logic->TakeDamage();}),
        Entrance(RR_FIRE_TEMPLE_WEST_CLIMB,         []{return true;}),
        Entrance(RR_FIRE_TEMPLE_HAMMER_RETURN_PATH, []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    });

    areaTable[RR_FIRE_TEMPLE_HAMMER_RETURN_PATH] = Region("Fire Temple Hammer Return Path", SCENE_FIRE_TEMPLE, {}, {
        // Locations
        LOCATION(RC_FIRE_TEMPLE_AFTER_HAMMER_SMALL_CRATE_1, logic->CanBreakSmallCrates()),
        LOCATION(RC_FIRE_TEMPLE_AFTER_HAMMER_SMALL_CRATE_2, logic->CanBreakSmallCrates()),
    },
    {
        //Exits
        Entrance(RR_FIRE_TEMPLE_ABOVE_FIRE_MAZE, []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    });

    areaTable[RR_FIRE_TEMPLE_ABOVE_FIRE_MAZE] = Region("Fire Temple Above Fire Maze", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_HAMMER_RETURN_PATH, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_FIRE_MAZE_UPPER,    []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    });

#pragma endregion

#pragma region MQ

    //potentially dangerous temp flag on the first room's torches, should be made permanent if possible
    areaTable[RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER] = Region("Fire Temple MQ First Room Lower", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_ENTRANCE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_ENTRANCE_POT_2, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_ENTRYWAY,            []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAP_ROOM_SOUTH,   []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_UPPER, []{return logic->IsAdult || logic->CanUse(RG_HOOKSHOT);}),
        Entrance(RR_FIRE_TEMPLE_MQ_STALFOS_ROOM,     []{return logic->SmallKeys(RR_FIRE_TEMPLE, 5);}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_FIRST_ROOM_UPPER] = Region("Fire Temple MQ First Room Upper", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_NEAR_BOSS_ROOM,   []{return logic->HasFireSource();}),
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_LAVA_ROOM,    []{return Here(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_UPPER, []{return logic->CanUse(RG_MEGATON_HAMMER);});}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_MAP_ROOM_SOUTH] = Region("Fire Temple MQ Map Room South", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_MAP_ROOM_SIDE_CHEST, logic->CanKillEnemy(RE_LIKE_LIKE)),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER, []{return Here(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER, []{return logic->CanKillEnemy(RE_LIKE_LIKE);});}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAP_ROOM_CAGE,    []{return logic->OpenedLowestGoronCage;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_STALFOS_ROOM] = Region("Fire Temple MQ Stalfos Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_LOOP_STALFOS_SUN_FAIRY, logic->CanUse(RG_SUNS_SONG)),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER,  []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_IRON_KNUCKLE_ROOM, []{return Here(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER, []{return logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 2);});}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_IRON_KNUCKLE_ROOM] = Region("Fire Temple MQ Iron Knuckle Room", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->FairyPot, []{return true;}),
    }, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_LOOP_KNUCKLE_SUN_FAIRY, logic->CanUse(RG_SUNS_SONG)),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_4, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_5, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_6, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_7, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_BEFORE_MINI_BOSS_POT_8, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_STALFOS_ROOM,       []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_FLARE_DANCER, []{return Here(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_LOWER, []{return logic->CanKillEnemy(RE_IRON_KNUCKLE);});}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_LOWER_FLARE_DANCER] = Region("Fire Temple MQ Lower Flare Dancer", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_MEGATON_HAMMER_CHEST, (logic->IsAdult || logic->CanUse(RG_HOOKSHOT)) && Here(RR_FIRE_TEMPLE_MQ_LOWER_FLARE_DANCER, []{return logic->CanKillEnemy(RE_FLARE_DANCER);})),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_IRON_KNUCKLE_ROOM, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAP_ROOM_NORTH,    []{return Here(RR_FIRE_TEMPLE_MQ_LOWER_FLARE_DANCER, []{return logic->CanKillEnemy(RE_FLARE_DANCER);});}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_MAP_ROOM_NORTH] = Region("Fire Temple MQ Map Room North", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->OpenedLowestGoronCage, []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    }, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_FLARE_DANCER, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAP_ROOM_CAGE,      []{return logic->OpenedLowestGoronCage;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_MAP_ROOM_CAGE] = Region("Fire Temple MQ Map Room Cage", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_MAP_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_MAP_ROOM_NORTH, []{return logic->OpenedLowestGoronCage;}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAP_ROOM_SOUTH, []{return logic->OpenedLowestGoronCage;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_NEAR_BOSS_ROOM] = Region("Fire Temple MQ Near Boss Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //If we're using the south torch as the initial torch, or using FAs, we either have to cross to the north to remove the crate, or use a trick to ignore it
        LOCATION(RC_FIRE_TEMPLE_MQ_NEAR_BOSS_CHEST,      logic->FireTimer() > 25 && ctx->GetTrickOption(RT_FIRE_MQ_NEAR_BOSS) && (logic->CanUse(RG_FIRE_ARROWS) || (logic->IsAdult && logic->CanUse(RG_DINS_FIRE) && logic->CanUse(RG_FAIRY_BOW)))),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_1, logic->FireTimer() > 25 && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_2, logic->FireTimer() > 25 && logic->CanBreakCrates()),

    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_UPPER,     []{return true;}),
        //Child cannot make it to the north side torches without a hook without specifically bunny hood speed + hover boots
        Entrance(RR_FIRE_TEMPLE_MQ_NEAR_BOSS_ROOM_NORTH, []{return logic->FireTimer() > 32 && (logic->CanUse(RG_HOOKSHOT) || (logic->IsAdult && logic->CanUse(RG_HOVER_BOOTS)));}),
        Entrance(RR_FIRE_TEMPLE_BOSS_ENTRYWAY,           []{return logic->FireTimer() >= 15 && ((logic->IsAdult && (ctx->GetTrickOption(RT_FIRE_BOSS_DOOR_JUMP) || logic->CanUse(RG_HOVER_BOOTS))) || (logic->IsAdult && logic->HitFireTemplePlatform) || (logic->HitFireTemplePlatform && logic->CanUse(RG_HOVER_BOOTS)));}),
    });

    //This room assumes tunic logic is handled on entry.
    //Covers the upper section too, as all methods to reach this can climb up somehow
    areaTable[RR_FIRE_TEMPLE_MQ_NEAR_BOSS_ROOM_NORTH] = Region("Fire Temple MQ Near Boss Room North", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //If we have FAs, we can just remove the crate and use those to light the torches.
        //otherwise, with Dins, we first light them with dins and then either use a bow shot or to cross back over to light the other torch
        //Valid ways across are adult+hovers, bunny+hovers, longshot or running through the lava and then climbing back up as adult (child can't reach the ledge).
        //The Damage logic here is for jumping down and running across the lava to get in dins range of the south torch
        //Fairies cannot be used for this as it is time sensetive, and NL is only useful with sticks as it disables other magic while in use, so it's tunic or raw damage taking ability.
        //testing tells me you take 3 ticks of lava damage, which is 12 internal damage or 3/4 of a heart at x1 damage multiplier, performing this run
        //logic->EffectiveHealth() works in half hearts for whatever reason, meaning this needs a deeper refactor to be perfect, but it should be good enough for now
        LOCATION(RC_FIRE_TEMPLE_MQ_NEAR_BOSS_CHEST,      logic->CanUse(RG_FIRE_ARROWS) || (logic->CanUse(RG_DINS_FIRE) && (logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && (logic->CanUse(RG_HOVER_BOOTS) || logic->CanUse(RG_GORON_TUNIC) || logic->EffectiveHealth() >= 2 || (logic->CanUse(RG_NAYRUS_LOVE) && logic->CanUse(RG_STICKS))))))),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_POT_1,   logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_POT_2,   logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_1, logic->FireTimer() > 25 && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_2, logic->FireTimer() > 25 && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_3, logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_4, logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_5, logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_OUTSIDE_BOSS_CRATE_6, logic->CanBreakCrates()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_NEAR_BOSS_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_BIG_LAVA_ROOM] = Region("Fire Temple MQ Big Lava Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //I'm currently assuming the oversight version of RT_FIRE_MQ_BK_CHEST for the fire timer logic
        LOCATION(RC_FIRE_TEMPLE_MQ_BIG_LAVA_ROOM_BLOCKED_DOOR_CHEST, logic->FireTimer() >= 40 && logic->HasFireSource() && logic->HasExplosives() && (logic->CanUse(RG_HOOKSHOT) || (logic->IsAdult && ctx->GetTrickOption(RT_FIRE_MQ_BLOCKED_CHEST)))),
        //implies CanGetEnemyDrop(RE_GOLD_SKULLTULA)
        LOCATION(RC_FIRE_TEMPLE_MQ_GS_BIG_LAVA_ROOM_OPEN_DOOR,       logic->FireTimer() >= 20 && logic->CanUse(RG_MEGATON_HAMMER)),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_ROOM_NORTH_POT,              logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_ROOM_HIGH_POT,               logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_ROOM_SOUTH_POT,              logic->FireTimer() >= 40 && (logic->CanUse(RG_HOOKSHOT) || (ctx->GetTrickOption(RT_FIRE_MQ_BLOCKED_CHEST) && ((logic->IsAdult && logic->CanBreakPots()) || logic->CanUse(RG_BOOMERANG))))),
    }, {
        //Exits
        // Fewer tunic requirements ends here
        Entrance(RR_FIRE_TEMPLE_MQ_FIRST_ROOM_UPPER,    []{return logic->FireTimer() >= 20;}),
        Entrance(RR_FIRE_TEMPLE_MQ_ELEVATOR_ROOM,       []{return logic->CanUse(RG_GORON_TUNIC) && logic->SmallKeys(RR_FIRE_TEMPLE, 2);}),
        Entrance(RR_FIRE_TEMPLE_MQ_TORCH_FIREWALL_ROOM, []{return logic->HasFireSource() && ((logic->CanUse(RG_FAIRY_BOW) && logic->FireTimer() >= 25) || (ctx->GetTrickOption(RT_FIRE_MQ_BK_CHEST) && logic->FireTimer() >= 50)) && (logic->CanUse(RG_HOOKSHOT) || (logic->IsAdult && ctx->GetTrickOption(RT_FIRE_SOT)));}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_TORCH_FIREWALL_ROOM] = Region("Fire Temple MQ Torch Firewall Room", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->FairyPot, []{return logic->CanUse(RG_HOOKSHOT);}),
    }, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_BOSS_KEY_CHEST,   logic->CanUse(RG_HOOKSHOT)),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_POT_1, logic->HookshotOrBoomerang()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_POT_2, logic->HookshotOrBoomerang()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_LAVA_ROOM, []{return true;}),
    });

    //This room assumes Goron Tunic until looser tunic requirements tricks are made
    areaTable[RR_FIRE_TEMPLE_MQ_ELEVATOR_ROOM] = Region("Fire Temple MQ Elevator Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_FIRE_PILLAR_LEFT_HEART,  true),
        LOCATION(RC_FIRE_TEMPLE_MQ_FIRE_PILLAR_RIGHT_HEART, true),
        LOCATION(RC_FIRE_TEMPLE_MQ_FIRE_PILLAR_LOWER_HEART, true),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_LAVA_ROOM,  []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_TORCH_ROOM, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_BIG_TORCH_ROOM] = Region("Fire Temple MQ Big Torch Room", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_MAZE,         []{return (logic->HasFireSource() && logic->CanUse(RG_HOOKSHOT)) || (ctx->GetTrickOption(RT_FIRE_MQ_CLIMB) && logic->CanUse(RG_HOVER_BOOTS));}),
        Entrance(RR_FIRE_TEMPLE_MQ_ELEVATOR_ROOM,      []{return logic->CanUse(RG_GORON_TUNIC);}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAZE_SHORTCUT_CAGE, []{return logic->OpenedUpperFireShortcut;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_LOWER_MAZE] = Region("Fire Temple MQ Lower Maze", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //Check handled on both floors
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_SIDE_ROOM_CHEST, logic->HasExplosives() && ctx->GetTrickOption(RT_FIRE_MQ_MAZE_SIDE_ROOM)),
     }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_TORCH_ROOM,        []{return true;}),
        //Explosives can also reach this room. Chus is relatively simple, they need to detonate on the first horizontal bar up from the floor while horizontally near the switch, but bombs are much harder
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_MAZE_CRATE_CAGE, []{return Here(RR_FIRE_TEMPLE_MQ_LOWER_MAZE, []{return logic->CanJumpslash();});}),
        //it's possible to make the RT_FIRE_MQ_MAZE_HOVERS as child using bunny hood jumps, but not adult as adult bonks
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE,            []{return logic->HasExplosives() && logic->CanUse(RG_MEGATON_HAMMER) && logic->CanUse(RG_HOOKSHOT);}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_LOWER_MAZE_CRATE_CAGE] = Region("Fire Temple MQ Lower Maze Crate Cage", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_LOWER_CHEST,   true),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_LOWER_CRATE_1, logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_LOWER_CRATE_2, logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_LOWER_CRATE_3, logic->CanBreakCrates()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_MAZE, []{return true;}),
        //it's possible to make the RT_FIRE_MQ_MAZE_HOVERS as child using bunny hood jumps, but not adult as adult bonks
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE, []{return logic->IsAdult && ((ctx->GetTrickOption(RT_FIRE_MQ_MAZE_HOVERS) && logic->CanUse(RG_HOVER_BOOTS)) || ctx->GetTrickOption(RT_FIRE_MQ_MAZE_JUMP));}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_UPPER_MAZE] = Region("Fire Temple MQ Upper Maze", SCENE_FIRE_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_MAZE,          []{return true;}),
        //this cage is much more lenient than the lower cage as the switch is close to the front. sling, rang and bow all hit the switch easily, though might be too unintuitive for default logic
        //This shouldn't come up in most cases anyway as most methods to get here need either a melee weapon or explosives
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE_BOX_CAGE, []{return Here(RR_FIRE_TEMPLE_MQ_UPPER_MAZE, []{return logic->CanJumpslash() || logic->HasExplosives();});}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAZE_SHORTCUT,       []{return logic->HasExplosives();}),
        //Implies RR_FIRE_TEMPLE_MQ_LOWER_MAZE access
        Entrance(RR_FIRE_TEMPLE_MQ_BURNING_BLOCK_CLIMB, []{return logic->HasExplosives() && logic->CanUse(RG_MEGATON_HAMMER) && (logic->CanUse(RG_LONGSHOT) || (logic->CanUse(RG_HOOKSHOT) && logic->CanUse(RG_SONG_OF_TIME)));}),
        Entrance(RR_FIRE_TEMPLE_MQ_HIGH_TORCH_ROOM,     []{return logic->SmallKeys(RR_FIRE_TEMPLE, 3) && logic->CanUse(RG_GORON_TUNIC);}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_UPPER_MAZE_BOX_CAGE] = Region("Fire Temple MQ Upper Maze Box Cage", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_UPPER_CHEST,           true),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_UPPER_CRATE_1,         logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_UPPER_CRATE_2,         logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_UPPER_CRATE_3,         logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_UPPER_SMALL_CRATE_1,   logic->CanBreakSmallCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_UPPER_SMALL_CRATE_2,   logic->CanBreakSmallCrates()),
        //Assumes maze access
        LOCATION(RC_FIRE_TEMPLE_MQ_LIZALFOS_MAZE_SIDE_ROOM_CHEST, logic->HasExplosives()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_MAZE_SHORTCUT] = Region("Fire Temple MQ Maze Shortcut", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->OpenedUpperFireShortcut,  []{return logic->CanUse(RG_MEGATON_HAMMER);}),
    }, {}, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE,         []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_MAZE_SHORTCUT_CAGE, []{return logic->OpenedUpperFireShortcut;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_MAZE_SHORTCUT_CAGE] = Region("Fire Temple MQ Maze Shortcut Cage", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_COMPASS_CHEST,    logic->OpenedUpperFireShortcut;),
        LOCATION(RC_FIRE_TEMPLE_MQ_SHORTCUT_CRATE_1, logic->OpenedUpperFireShortcut && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SHORTCUT_CRATE_2, logic->OpenedUpperFireShortcut && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SHORTCUT_CRATE_3, logic->OpenedUpperFireShortcut && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SHORTCUT_CRATE_4, logic->OpenedUpperFireShortcut && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SHORTCUT_CRATE_5, logic->OpenedUpperFireShortcut && logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SHORTCUT_CRATE_6, logic->OpenedUpperFireShortcut && logic->CanBreakCrates()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_MAZE_SHORTCUT,  []{return logic->OpenedUpperFireShortcut;}),
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_TORCH_ROOM, []{return logic->OpenedUpperFireShortcut;}),
    });


    areaTable[RR_FIRE_TEMPLE_MQ_BURNING_BLOCK_CLIMB] = Region("Fire Temple MQ Burning Block Climb", SCENE_FIRE_TEMPLE, {
        //Events
        //EventAccess(&WallFairy, []{return logic->CanUse(RG_HOOKSHOT);}),
    }, {
        //Locations
        //There's definitely ways to do this hammerless, but with one points on it's a trick
        LOCATION(RC_FIRE_TEMPLE_MQ_GS_SKULL_ON_FIRE,  logic->CanUse(RG_HOOKSHOT) && logic->CanUse(RG_MEGATON_HAMMER)),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE,        []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_NARROW_PATH_ROOM,  []{return logic->TakeDamage();}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_NARROW_PATH_ROOM] = Region("Fire Temple MQ Narrow Path Room", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->FairyPot, []{return true;}),
    }, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_ABOVE_LAVA_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_ABOVE_LAVA_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_ABOVE_LAVA_POT_3, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_LOWER_MAZE,    []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_BIG_LAVA_ROOM, []{return logic->TakeDamage();}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_HIGH_TORCH_ROOM] = Region("Fire Temple MQ High Torch Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_FLAME_WALL_POT_1,            logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_FLAME_WALL_POT_2,            logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_CRATE_1,          logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_CRATE_2,          logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_CRATE_3,          logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_CRATE_4,          logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_CRATE_5,          logic->CanBreakCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_SMALL_CRATE_1,    logic->CanBreakSmallCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_SMALL_CRATE_2,    logic->CanBreakSmallCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_SMALL_CRATE_3,    logic->CanBreakSmallCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_SMALL_CRATE_4,    logic->CanBreakSmallCrates()),
        LOCATION(RC_FIRE_TEMPLE_MQ_LAVA_TORCH_SMALL_CRATE_5,    logic->CanBreakSmallCrates()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_MAZE,       []{return logic->SmallKeys(RR_FIRE_TEMPLE, 3);}),
        Entrance(RR_FIRE_TEMPLE_MQ_NARROW_PATH_ROOM, []{return true;}),
        //Child has issues navigating the higher points of this room without an equip swapped hookshot
        Entrance(RR_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE,  []{return Here(RR_FIRE_TEMPLE_MQ_HIGH_TORCH_ROOM, []{return logic->CanUse(RG_FIRE_ARROWS) || (logic->CanUse(RG_FAIRY_BOW) && logic->CanUse(RG_HOOKSHOT));}) && (logic->IsAdult || logic->CanUse(RG_HOOKSHOT));}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE] = Region("Fire Temple MQ South Fire Maze", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_GS_FIRE_WALL_MAZE_CENTER, logic->HasExplosives()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE_WEST_POT, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE_EAST_POT, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_NEAR_BOSS_ROOM,      []{return logic->HitFireTemplePlatform;}),
        Entrance(RR_FIRE_TEMPLE_MQ_HIGH_TORCH_ROOM,     []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PLATFORMS, []{return logic->IsAdult || logic->CanUse(RG_SONG_OF_TIME) || logic->CanUse(RG_HOVER_BOOTS);}),
        //Hover boots get there via the platforms
        Entrance(RR_FIRE_TEMPLE_MQ_NORTH_FIRE_MAZE,     []{return (bool)ctx->GetTrickOption(RT_FIRE_MQ_FLAME_MAZE);}),
        Entrance(RR_FIRE_TEMPLE_MQ_WEST_FIRE_MAZE,      []{return logic->OpenedFireMQFireMazeDoor;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PLATFORMS] = Region("Fire Temple MQ Fire Maze Platforms", SCENE_FIRE_TEMPLE, {
        //Events
        EventAccess(&logic->HitFireTemplePlatform,    []{return logic->CanUse(RG_MEGATON_HAMMER);}),
        EventAccess(&logic->OpenedFireMQFireMazeDoor, []{return logic->CanUse(RG_MEGATON_HAMMER) && logic->CanUse(RG_HOOKSHOT);}),
    }, {}, {
        Entrance(RR_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_NORTH_FIRE_MAZE, []{return logic->CanUse(RG_SONG_OF_TIME) || logic->CanUse(RG_HOVER_BOOTS);}),
        //trick to get to RR_FIRE_TEMPLE_MQ_WEST_FIRE_MAZE with hovers + taking damage is plausible
    });

    areaTable[RR_FIRE_TEMPLE_MQ_NORTH_FIRE_MAZE] = Region("Fire Temple MQ North Fire Maze", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_GS_FIRE_WALL_MAZE_SIDE_ROOM, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
        LOCATION(RC_FIRE_TEMPLE_MQ_PAST_FIRE_MAZE_SOUTH_POT,    logic->CanUse(RG_BOOMERANG)),
        LOCATION(RC_FIRE_TEMPLE_MQ_FIRE_MAZE_NORTHMOST_POT,     logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_FIRE_MAZE_NORTHWEST_POT,     logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE, []{return logic->IsAdult || ctx->GetTrickOption(RT_FIRE_MQ_FLAME_MAZE);}),
        Entrance(RR_FIRE_TEMPLE_MQ_WEST_FIRE_MAZE,  []{return (bool)ctx->GetTrickOption(RT_FIRE_MQ_FLAME_MAZE);}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_WEST_FIRE_MAZE] = Region("Fire Temple MQ West Fire Maze", SCENE_FIRE_TEMPLE, {}, {}, {
        Entrance(RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PAST_WALL, []{return true;}),
        Entrance(RR_FIRE_TEMPLE_MQ_NORTH_FIRE_MAZE,     []{return (bool)ctx->GetTrickOption(RT_FIRE_MQ_FLAME_MAZE);}),
    });

    //this area exists for the pots in case we void warp to the top of fire somehow, because there's no way to get back the way we came
    areaTable[RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PAST_WALL] = Region("Fire Temple MQ Fire Maze Past Wall", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_PAST_FIRE_MAZE_SOUTH_POT, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_PAST_FIRE_MAZE_NORTH_POT, logic->CanBreakPots()),
        LOCATION(RC_FIRE_TEMPLE_MQ_FIRE_MAZE_NORTHWEST_POT,  logic->CanUse(RG_BOOMERANG)),
    }, {
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_FLARE_DANCER, []{return true;}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_UPPER_FLARE_DANCER] = Region("Fire Temple MQ North Fire Maze", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FIRE_TEMPLE_MQ_FREESTANDING_KEY, logic->CanKillEnemy(RE_FLARE_DANCER)),
    }, {
        Entrance(RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PAST_WALL, []{return logic->CanKillEnemy(RE_FLARE_DANCER);}),
        Entrance(RR_FIRE_TEMPLE_MQ_SCARECROW_ROOM,      []{return logic->CanKillEnemy(RE_FLARE_DANCER) && logic->SmallKeys(RR_FIRE_TEMPLE, 4);}),
    });

    areaTable[RR_FIRE_TEMPLE_MQ_SCARECROW_ROOM] = Region("Fire Temple MQ Scarecrow Room", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //This requires nothing in N64 logic, but is tight enough to need rollspam with the one-point on which is stricter than I would normally consider in logic
        //Child basically needs the scarecrow or a bunny hood though due to a worse ledge grab.
        LOCATION(RC_FIRE_TEMPLE_MQ_CHEST_ON_FIRE, logic->IsAdult || logic->CanUse(RG_SCARECROW)),
    }, {
        //The dropdown here is unusual in that it hits 1 of 3 locations: RR_FIRE_TEMPLE_MQ_SOUTH_FIRE_MAZE, RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PLATFORMS and the section of RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PLATFORMS with the hammer switch
        //Using this dropdown is in N64 logic elsewhere, but not here, probably because it requires good foreknowlege to determine where to land
        //This would be a logical method to reach the hammer switch without hookshot, but it practically requires access to the area that switch unlocks already. It could also be first child access to PLATFORMS if tricks ever enable that
        //If a practical use for this drop is found, it should be made a trick
        Entrance(RR_FIRE_TEMPLE_MQ_UPPER_FLARE_DANCER, []{return logic->SmallKeys(RR_FIRE_TEMPLE, 4);}),
        Entrance(RR_FIRE_TEMPLE_MQ_COLLAPSED_STAIRS,   []{return Here(RR_FIRE_TEMPLE_MQ_SCARECROW_ROOM, []{return logic->CanUse(RG_MEGATON_HAMMER);}) && logic->SmallKeys(RR_FIRE_TEMPLE, 5);}),
    });

    //The peg knocked down from here could have logical implications for child in the fire maze if tricks to gain height like bomb jumps exist
    areaTable[RR_FIRE_TEMPLE_MQ_COLLAPSED_STAIRS] = Region("Fire Temple MQ Collapsed Stairs", SCENE_FIRE_TEMPLE, {}, {
        //Locations
        //If someone manages to make a trick to get here from fire maze, this needs to be in a separate room as the door back is barred
        LOCATION(RC_FIRE_TEMPLE_MQ_GS_ABOVE_FIRE_MAZE, logic->CanUse(RG_HOOKSHOT)),
    }, {
        Entrance(RR_FIRE_TEMPLE_MQ_FIRE_MAZE_PLATFORMS, []{return logic->CanUse(RG_HOOKSHOT) && Here(RR_FIRE_TEMPLE_MQ_COLLAPSED_STAIRS, []{return logic->CanUse(RG_MEGATON_HAMMER);});}),
        Entrance(RR_FIRE_TEMPLE_MQ_SCARECROW_ROOM,      []{return logic->IsAdult && logic->CanUse(RG_HOOKSHOT);}),
    });

#pragma endregion

    // Boss Room
    areaTable[RR_FIRE_TEMPLE_BOSS_ENTRYWAY] = Region("Fire Temple Boss Entryway", SCENE_FIRE_TEMPLE, {}, {}, {
        // Exits
        Entrance(RR_FIRE_TEMPLE_NEAR_BOSS_ROOM,    []{return ctx->GetDungeon(FIRE_TEMPLE)->IsVanilla() && false;}),
        Entrance(RR_FIRE_TEMPLE_MQ_NEAR_BOSS_ROOM, []{return ctx->GetDungeon(FIRE_TEMPLE)->IsMQ() && false;}),
        Entrance(RR_FIRE_TEMPLE_BOSS_ROOM,         []{return logic->HasItem(RG_FIRE_TEMPLE_BOSS_KEY);}),
    });

    areaTable[RR_FIRE_TEMPLE_BOSS_ROOM] = Region("Fire Temple Boss Room", SCENE_FIRE_TEMPLE_BOSS, {
        // Events
        EventAccess(&logic->FireTempleClear, []{return logic->FireTimer() >= 64 && logic->CanKillEnemy(RE_VOLVAGIA);}),
    }, {
        // Locations
        LOCATION(RC_FIRE_TEMPLE_VOLVAGIA_HEART, logic->FireTempleClear),
        LOCATION(RC_VOLVAGIA,                   logic->FireTempleClear),
    }, {
        // Exits
        Entrance(RR_FIRE_TEMPLE_BOSS_ENTRYWAY, []{return false;}),
        Entrance(RR_DMC_CENTRAL_LOCAL,         []{return logic->FireTempleClear;}, false),
    });

    // clang-format on
}
