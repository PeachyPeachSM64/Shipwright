#include "soh/Enhancements/randomizer/location_access.h"
#include "soh/Enhancements/randomizer/entrance.h"
#include "soh/Enhancements/randomizer/dungeon.h"

using namespace Rando;

void RegionTable_Init_ForestTemple() {
    // clang-format off
    // Vanilla/MQ Decider
    areaTable[RR_FOREST_TEMPLE_ENTRYWAY] = Region("Forest Temple Entryway", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_FIRST_ROOM, []{return ctx->GetDungeon(FOREST_TEMPLE)->IsVanilla();}),
        Entrance(RR_FOREST_TEMPLE_MQ_LOBBY,   []{return ctx->GetDungeon(FOREST_TEMPLE)->IsMQ();}),
        Entrance(RR_SACRED_FOREST_MEADOW,     []{return true;}),
    });

#pragma region Vanilla

    areaTable[RR_FOREST_TEMPLE_FIRST_ROOM] = Region("Forest Temple First Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_FIRST_ROOM_CHEST, true),
        LOCATION(RC_FOREST_TEMPLE_GS_FIRST_ROOM,    (logic->IsAdult && logic->CanUse(RG_BOMB_BAG)) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_BOOMERANG) || logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->CanUse(RG_BOMBCHU_5) || logic->CanUse(RG_DINS_FIRE) || (ctx->GetTrickOption(RT_FOREST_FIRST_GS) && (logic->CanJumpslashExceptHammer() || (logic->IsChild && logic->CanUse(RG_BOMB_BAG))))),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_ENTRYWAY,       []{return true;}),
        Entrance(RR_FOREST_TEMPLE_SOUTH_CORRIDOR, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_SOUTH_CORRIDOR] = Region("Forest Temple South Corridor", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_FIRST_ROOM, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_LOBBY,      []{return logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
    });

    areaTable[RR_FOREST_TEMPLE_LOBBY] = Region("Forest Temple Lobby", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleMeg, []{return logic->ForestTempleJoelle && logic->ForestTempleBeth && logic->ForestTempleAmy && logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_GS_LOBBY,    logic->HookshotOrBoomerang()),
        LOCATION(RC_FOREST_TEMPLE_LOBBY_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_LOBBY_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_LOBBY_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_LOBBY_POT_4, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_LOBBY_POT_5, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_LOBBY_POT_6, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_SOUTH_CORRIDOR,    []{return true;}),
        Entrance(RR_FOREST_TEMPLE_NORTH_CORRIDOR,    []{return true;}),
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_LOWER, []{return logic->CanUse(RG_SONG_OF_TIME) || logic->IsChild;}),
        Entrance(RR_FOREST_TEMPLE_NE_OUTDOORS_LOWER, []{return logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_FAIRY_SLINGSHOT);}),
        Entrance(RR_FOREST_TEMPLE_WEST_CORRIDOR,     []{return logic->SmallKeys(RR_FOREST_TEMPLE, 1, 5);}),
        Entrance(RR_FOREST_TEMPLE_EAST_CORRIDOR,     []{return false;}),
        Entrance(RR_FOREST_TEMPLE_BOSS_REGION,       []{return logic->ForestTempleMeg;}),
        Entrance(RR_FOREST_TEMPLE_BOSS_ENTRYWAY,     []{return false;}),
    });

    areaTable[RR_FOREST_TEMPLE_NORTH_CORRIDOR] = Region("Forest Temple North Corridor", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_LOBBY,         []{return true;}),
        Entrance(RR_FOREST_TEMPLE_LOWER_STALFOS, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_LOWER_STALFOS] = Region("Forest Temple Lower Stalfos", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->FairyPot, []{return true;}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_FIRST_STALFOS_CHEST, logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 2)),
        LOCATION(RC_FOREST_TEMPLE_LOWER_STALFOS_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_LOWER_STALFOS_POT_2, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NORTH_CORRIDOR, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_NW_OUTDOORS_LOWER] = Region("Forest Temple NW Outdoors Lower", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->DekuBabaSticks, []{return logic->CanGetDekuBabaSticks();}),
        EventAccess(&logic->DekuBabaNuts,   []{return logic->CanGetDekuBabaNuts();}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_GS_LEVEL_ISLAND_COURTYARD, logic->CanUse(RG_LONGSHOT) || Here(RR_FOREST_TEMPLE_NW_OUTDOORS_UPPER, []{return logic->HookshotOrBoomerang();})),
        LOCATION(RC_FOREST_TEMPLE_COURTYARD_RIGHT_HEART,     logic->CanUse(RG_BOOMERANG) && ctx->GetTrickOption(RT_FOREST_OUTDOORS_HEARTS_BOOMERANG)),
        LOCATION(RC_FOREST_TEMPLE_COURTYARD_LEFT_HEART,      logic->CanUse(RG_BOOMERANG) && ctx->GetTrickOption(RT_FOREST_OUTDOORS_HEARTS_BOOMERANG)),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_LOBBY,             []{return logic->CanUse(RG_SONG_OF_TIME);}),
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_UPPER, []{return ctx->GetTrickOption(RT_HOVER_BOOST_SIMPLE) && ctx->GetTrickOption(RT_DAMAGE_BOOST_SIMPLE) && logic->HasExplosives() && logic->CanUse(RG_HOVER_BOOTS);}),
        Entrance(RR_FOREST_TEMPLE_MAP_ROOM,          []{return true;}),
        Entrance(RR_FOREST_TEMPLE_SEWER,             []{return logic->HasItem(RG_GOLDEN_SCALE) || logic->CanUse(RG_IRON_BOOTS) || HasAccessTo(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER);}),
        Entrance(RR_FOREST_TEMPLE_BOSS_ENTRYWAY,     []{return false;}),
    });

    areaTable[RR_FOREST_TEMPLE_NW_OUTDOORS_UPPER] = Region("Forest Temple NW Outdoors Upper", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->DekuBabaSticks, []{return logic->CanGetDekuBabaSticks();}),
        EventAccess(&logic->DekuBabaNuts,   []{return logic->CanGetDekuBabaNuts();}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_COURTYARD_RIGHT_HEART, true),
        LOCATION(RC_FOREST_TEMPLE_COURTYARD_LEFT_HEART,  true),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_LOWER,    []{return true;}),
        Entrance(RR_FOREST_TEMPLE_BELOW_BOSS_KEY_CHEST, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_FLOORMASTER_ROOM,     []{return true;}),
        Entrance(RR_FOREST_TEMPLE_BLOCK_PUSH_ROOM,      []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_NE_OUTDOORS_LOWER] = Region("Forest Temple NE Outdoors Lower", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->DekuBabaSticks, []{return logic->CanGetDekuBabaSticks();}),
        EventAccess(&logic->DekuBabaNuts,   []{return logic->CanGetDekuBabaNuts();}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_RAISED_ISLAND_COURTYARD_CHEST, logic->CanUse(RG_HOOKSHOT) || HasAccessTo(RR_FOREST_TEMPLE_FALLING_ROOM) || (HasAccessTo(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER) && logic->IsAdult && ctx->GetTrickOption(RT_FOREST_OUTDOORS_LEDGE) && logic->CanUse(RG_HOVER_BOOTS))),
        LOCATION(RC_FOREST_TEMPLE_GS_RAISED_ISLAND_COURTYARD,    logic->CanUse(RG_HOOKSHOT) || (ctx->GetTrickOption(RT_FOREST_OUTDOORS_EAST_GS) && logic->CanUse(RG_BOOMERANG)) || Here(RR_FOREST_TEMPLE_FALLING_ROOM, []{return logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->CanUse(RG_DINS_FIRE) || logic->HasExplosives();})),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_LOBBY,             []{return true;}),
        Entrance(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER, []{return logic->CanUse(RG_LONGSHOT) || (ctx->GetTrickOption(RT_FOREST_VINES) && logic->CanUse(RG_HOOKSHOT));}),
        Entrance(RR_FOREST_TEMPLE_SEWER,             []{return logic->HasItem(RG_GOLDEN_SCALE) || logic->CanUse(RG_IRON_BOOTS) || HasAccessTo(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER);}),
        Entrance(RR_FOREST_TEMPLE_FALLING_ROOM,      []{return false;}),
    });

    areaTable[RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER] = Region("Forest Temple NE Outdoors Upper", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->DekuBabaSticks, []{return logic->CanGetDekuBabaSticks();}),
        EventAccess(&logic->DekuBabaNuts,   []{return logic->CanGetDekuBabaNuts();}),
    }, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NE_OUTDOORS_LOWER, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_MAP_ROOM,          []{return true;}),
        Entrance(RR_FOREST_TEMPLE_FALLING_ROOM,      []{return ctx->GetTrickOption(RT_FOREST_DOORFRAME) && logic->CanJumpslashExceptHammer() && logic->CanUse(RG_HOVER_BOOTS) && logic->CanUse(RG_SCARECROW);}),
    });

    areaTable[RR_FOREST_TEMPLE_MAP_ROOM] = Region("Forest Temple Map Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MAP_CHEST, logic->CanKillEnemy(RE_BLUE_BUBBLE)),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_LOWER, []{return Here(RR_FOREST_TEMPLE_MAP_ROOM, []{return logic->CanKillEnemy(RE_BLUE_BUBBLE);});}),
        Entrance(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER, []{return Here(RR_FOREST_TEMPLE_MAP_ROOM, []{return logic->CanKillEnemy(RE_BLUE_BUBBLE);});}),
    });

    areaTable[RR_FOREST_TEMPLE_SEWER] = Region("Forest Temple Sewer", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_WELL_CHEST,      HasAccessTo(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER)),
        LOCATION(RC_FOREST_TEMPLE_WELL_WEST_HEART, HasAccessTo(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER) || (logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8)),
        LOCATION(RC_FOREST_TEMPLE_WELL_EAST_HEART, HasAccessTo(RR_FOREST_TEMPLE_NE_OUTDOORS_UPPER) || (logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8)),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_LOWER, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_NE_OUTDOORS_LOWER, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_BELOW_BOSS_KEY_CHEST] = Region("Forest Temple Below Boss Key Chest", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_UPPER, []{return Here(RR_FOREST_TEMPLE_BELOW_BOSS_KEY_CHEST, []{return logic->CanKillEnemy(RE_BLUE_BUBBLE);});}),
    });

    areaTable[RR_FOREST_TEMPLE_FLOORMASTER_ROOM] = Region("Forest Temple Floormaster Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_FLOORMASTER_CHEST, logic->CanDamage()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_UPPER, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_WEST_CORRIDOR] = Region("Forest Temple West Corridor", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_LOBBY,           []{return logic->SmallKeys(RR_FOREST_TEMPLE, 1, 5);}),
        Entrance(RR_FOREST_TEMPLE_BLOCK_PUSH_ROOM, []{return logic->CanAttack() || logic->CanUse(RG_NUTS);}),
    });

    areaTable[RR_FOREST_TEMPLE_BLOCK_PUSH_ROOM] = Region("Forest Temple Block Push Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_EYE_SWITCH_CHEST, logic->HasItem(RG_GORONS_BRACELET) && (logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_FAIRY_SLINGSHOT))),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_WEST_CORRIDOR,            []{return true;}),
        Entrance(RR_FOREST_TEMPLE_NW_OUTDOORS_UPPER,        []{return logic->CanUse(RG_HOVER_BOOTS) || (ctx->GetTrickOption(RT_FOREST_OUTSIDE_BACKDOOR) && logic->CanJumpslashExceptHammer() && logic->HasItem(RG_GORONS_BRACELET));}),
        Entrance(RR_FOREST_TEMPLE_NW_CORRIDOR_TWISTED,      []{return logic->IsAdult && logic->HasItem(RG_GORONS_BRACELET) && logic->SmallKeys(RR_FOREST_TEMPLE, 2);}),
        Entrance(RR_FOREST_TEMPLE_NW_CORRIDOR_STRAIGHTENED, []{return logic->IsAdult && (logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_FAIRY_SLINGSHOT)) && logic->HasItem(RG_GORONS_BRACELET) && logic->SmallKeys(RR_FOREST_TEMPLE, 2);}),
    });

    areaTable[RR_FOREST_TEMPLE_NW_CORRIDOR_TWISTED] = Region("Forest Temple NW Corridor Twisted", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_BLOCK_PUSH_ROOM, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 2);}),
        Entrance(RR_FOREST_TEMPLE_RED_POE_ROOM,    []{return logic->SmallKeys(RR_FOREST_TEMPLE, 3);}),
    });

    areaTable[RR_FOREST_TEMPLE_NW_CORRIDOR_STRAIGHTENED] = Region("Forest Temple NW Corridor Straightened", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_BOSS_KEY_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_BELOW_BOSS_KEY_CHEST, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_BLOCK_PUSH_ROOM,      []{return logic->SmallKeys(RR_FOREST_TEMPLE, 2);}),
    });

    areaTable[RR_FOREST_TEMPLE_RED_POE_ROOM] = Region("Forest Temple Red Poe Room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleJoelle, []{return logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_RED_POE_CHEST, logic->ForestTempleJoelle),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NW_CORRIDOR_TWISTED, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 3);}),
        Entrance(RR_FOREST_TEMPLE_UPPER_STALFOS,       []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_UPPER_STALFOS] = Region("Forest Temple Upper Stalfos", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_BOW_CHEST,           logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 3)),
        LOCATION(RC_FOREST_TEMPLE_UPPER_STALFOS_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_UPPER_STALFOS_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_UPPER_STALFOS_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_UPPER_STALFOS_POT_4, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_RED_POE_ROOM,  []{return logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 3);}),
        Entrance(RR_FOREST_TEMPLE_BLUE_POE_ROOM, []{return logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 3);}),
    });

    areaTable[RR_FOREST_TEMPLE_BLUE_POE_ROOM] = Region("Forest Temple Blue Poe Room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleBeth, []{return logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_BLUE_POE_CHEST, logic->ForestTempleBeth),
        LOCATION(RC_FOREST_TEMPLE_BLUE_POE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_BLUE_POE_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_BLUE_POE_POT_3, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_UPPER_STALFOS,            []{return true;}),
        Entrance(RR_FOREST_TEMPLE_NE_CORRIDOR_STRAIGHTENED, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 4);}),
    });

    areaTable[RR_FOREST_TEMPLE_NE_CORRIDOR_STRAIGHTENED] = Region("Forest Temple NE Corridor Straightened", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_BLUE_POE_ROOM,   []{return logic->SmallKeys(RR_FOREST_TEMPLE, 4);}),
        Entrance(RR_FOREST_TEMPLE_FROZEN_EYE_ROOM, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 5);}),
    });

    areaTable[RR_FOREST_TEMPLE_NE_CORRIDOR_TWISTED] = Region("Forest Temple NE Corridor Twisted", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_FROZEN_EYE_ROOM, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 5);}),
        Entrance(RR_FOREST_TEMPLE_FALLING_ROOM,    []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_FROZEN_EYE_ROOM] = Region("Forest Temple Frozen Eye Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_FROZEN_EYE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_FROZEN_EYE_POT_2, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NE_CORRIDOR_STRAIGHTENED, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 5);}),
        Entrance(RR_FOREST_TEMPLE_NE_CORRIDOR_TWISTED,      []{return logic->SmallKeys(RR_FOREST_TEMPLE, 5) && (logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_DINS_FIRE));}),
    });

    areaTable[RR_FOREST_TEMPLE_FALLING_ROOM] = Region("Forest Temple Falling Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_FALLING_CEILING_ROOM_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_NE_OUTDOORS_LOWER, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_GREEN_POE_ROOM,    []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_GREEN_POE_ROOM] = Region("Forest Temple Green Poe Room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleAmy, []{return logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_GREEN_POE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_GREEN_POE_POT_2, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_FALLING_ROOM,  []{return true;}),
        Entrance(RR_FOREST_TEMPLE_EAST_CORRIDOR, []{return logic->ForestTempleAmy;}),
    });

    areaTable[RR_FOREST_TEMPLE_EAST_CORRIDOR] = Region("Forest Temple East Corridor", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_LOBBY,          []{return logic->CanAttack() || logic->CanUse(RG_NUTS);}),
        Entrance(RR_FOREST_TEMPLE_GREEN_POE_ROOM, []{return logic->CanAttack() || logic->CanUse(RG_NUTS);}),
    });

    areaTable[RR_FOREST_TEMPLE_BOSS_REGION] = Region("Forest Temple Boss Region", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_BASEMENT_CHEST, true),
        LOCATION(RC_FOREST_TEMPLE_GS_BASEMENT,    logic->HookshotOrBoomerang()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_LOBBY,         []{return true;}),
        Entrance(RR_FOREST_TEMPLE_BOSS_ENTRYWAY, []{return true;}),
    });

#pragma endregion

#pragma region MQ

    areaTable[RR_FOREST_TEMPLE_MQ_LOBBY] = Region("Forest Temple MQ Lobby", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_FIRST_ROOM_CHEST, logic->CanPassEnemy(RE_BIG_SKULLTULA, ED_SHORT_JUMPSLASH, false) || logic->CanUse(RG_HOVER_BOOTS)),
        //Implies CanPassEnemy(RE_BIG_SKULLTULA)
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_FIRST_HALLWAY, logic->HookshotOrBoomerang()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_ENTRYWAY,        []{return true;}),
        Entrance(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 1) && logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_CENTRAL_AREA] = Region("Forest Temple MQ Central Region", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleMeg, []{return logic->ForestTempleJoelle && logic->ForestTempleBeth && logic->ForestTempleAmy && logic->CanKillEnemy(RE_MEG);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_LOBBY_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_LOBBY_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_LOBBY_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_LOBBY_POT_4, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_LOBBY_POT_5, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_LOBBY_POT_6, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_WOLFOS_ROOM,        []{return logic->IsChild || logic->CanUse(RG_SONG_OF_TIME);}),
        Entrance(RR_FOREST_TEMPLE_MQ_NW_OUTDOORS,        []{return logic->CanHitEyeTargets();}),
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS,        []{return logic->CanHitEyeTargets();}),
        Entrance(RR_FOREST_TEMPLE_MQ_LOWER_BLOCK_PUZZLE, []{return Here(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA, []{return logic->CanKillEnemy(RE_STALFOS);});}),
        //implies the other 3 poes
        Entrance(RR_FOREST_TEMPLE_MQ_BASEMENT,           []{return logic->ForestTempleMeg;}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_WOLFOS_ROOM] = Region("Forest Temple MQ Wolfos Room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->FairyPot,                 []{return true;}),
        EventAccess(&logic->ForestClearBelowBowChest, []{return logic->CanKillEnemy(RE_WOLFOS);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_WOLFOS_CHEST, logic->ForestClearBelowBowChest),
        LOCATION(RC_FOREST_TEMPLE_MQ_WOLFOS_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_WOLFOS_POT_2, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA, []{return logic->ForestClearBelowBowChest && (logic->IsChild || logic->CanUse(RG_SONG_OF_TIME));}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_LOWER_BLOCK_PUZZLE] = Region("Forest Temple MQ Lower Block Puzzle", SCENE_FOREST_TEMPLE, {
        //longshot is capable of hitting the switch, but some invisible collision makes the shot harder than you would think, so it may be trickworthy
        EventAccess(&logic->MQForestBlockRoomTargets, []{return (ctx->GetTrickOption(RT_FOREST_MQ_BLOCK_PUZZLE) && logic->CanUse(RG_BOMBCHU_5));}),
        //It is barely possible to get this as child with master + hovers, but it's tight without bunny speed
        EventAccess(&logic->ForestCanTwistHallway,    []{return (ctx->GetTrickOption(RT_FOREST_MQ_JS_HALLWAY_SWITCH) && logic->CanUse(RG_HOVER_BOOTS) && (logic->IsAdult && logic->CanJumpslash()) || (logic->CanUse(RG_STICKS) || logic->CanUse(RG_BIGGORON_SWORD) || (logic->MQForestBlockRoomTargets && logic->CanUse(RG_MASTER_SWORD)))) || (ctx->GetTrickOption(RT_FOREST_MQ_RANG_HALLWAY_SWITCH) && logic->CanUse(RG_BOOMERANG)) || (ctx->GetTrickOption(RT_FOREST_MQ_HOOKSHOT_HALLWAY_SWITCH) && logic->CanUse(RG_HOOKSHOT));}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_BLOCK_PUSH_ROOM, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA,        []{return Here(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA, []{return logic->CanKillEnemy(RE_STALFOS);});}),
        Entrance(RR_FOREST_TEMPLE_MQ_MIDDLE_BLOCK_PUZZLE, []{return logic->HasItem(RG_GORONS_BRACELET) || (logic->MQForestBlockRoomTargets && logic->CanUse(RG_HOOKSHOT));}),
        //Assumes RR_FOREST_TEMPLE_MQ_MIDDLE_BLOCK_PUZZLE access
        Entrance(RR_FOREST_TEMPLE_MQ_UPPER_BLOCK_PUZZLE,  []{return (logic->IsAdult && logic->HasItem(RG_GORONS_BRACELET)) || (logic->MQForestBlockRoomTargets && logic->CanUse(RG_HOOKSHOT));}),
        Entrance(RR_FOREST_TEMPLE_MQ_OUTDOOR_LEDGE,       []{return logic->ForestCanTwistHallway && (logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_HOVER_BOOTS));}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_MIDDLE_BLOCK_PUZZLE] = Region("Forest Temple MQ Middle Block Puzzle", SCENE_FOREST_TEMPLE, {
        //longshot is capable of hitting the switch, but some invisible collision makes the shot more annoying than you would think, so it may be trickworthy
        EventAccess(&logic->MQForestBlockRoomTargets, []{return (logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->CanUse(RG_LONGSHOT));}),
        EventAccess(&logic->ForestCanTwistHallway,    []{return ctx->GetTrickOption(RT_FOREST_MQ_JS_HALLWAY_SWITCH) && (logic->IsAdult && logic->CanJumpslash()) || (logic->CanUse(RG_HOVER_BOOTS) && (logic->CanUse(RG_STICKS) || logic->CanUse(RG_BIGGORON_SWORD) || logic->CanUse(RG_MASTER_SWORD)));}),
    }, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_LOWER_BLOCK_PUZZLE, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_MQ_UPPER_BLOCK_PUZZLE, []{return (logic->IsAdult && logic->HasItem(RG_GORONS_BRACELET)) || (logic->MQForestBlockRoomTargets && logic->CanUse(RG_HOOKSHOT));}),
        //Hammer cannot recoil from here, but can make the jump forwards with a hammer jumpslash as adult
        Entrance(RR_FOREST_TEMPLE_MQ_OUTDOOR_LEDGE,      []{return logic->ForestCanTwistHallway && logic->CanUse(RG_HOVER_BOOTS) || (ctx->GetTrickOption(RT_FOREST_OUTSIDE_BACKDOOR) && (logic->CanJumpslashExceptHammer() || (logic->IsAdult && logic->CanUse(RG_MEGATON_HAMMER))));}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_UPPER_BLOCK_PUZZLE] = Region("Forest Temple MQ After Block Puzzle", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_BOSS_KEY_CHEST, logic->SmallKeys(RR_FOREST_TEMPLE, 3)),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_STRAIGHT_HALLWAY, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 3);}),
        Entrance(RR_FOREST_TEMPLE_MQ_JOELLE_ROOM,      []{return logic->ForestCanTwistHallway && logic->SmallKeys(RR_FOREST_TEMPLE, 4);}),
        //!QUANTUM LOGIC!
        //As there is no way in default logic to reach the other possible key use without going through RR_FOREST_TEMPLE_MQ_NW_OUTDOORS, this is logically safe for now
        //Breaks if there's any other way to RR_FOREST_TEMPLE_MQ_FALLING_ROOM than going through the eye targets in RR_FOREST_TEMPLE_MQ_CENTRAL_AREA
        //Requires a bow/sling ammo source once ammo logic is done, to avoid edge cases.
        Entrance(RR_FOREST_TEMPLE_MQ_NW_OUTDOORS,      []{return logic->SmallKeys(RR_FOREST_TEMPLE, 2) && Here(RR_FOREST_TEMPLE_MQ_UPPER_BLOCK_PUZZLE, []{return logic->CanKillEnemy(RE_FLOORMASTER);});}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_STRAIGHT_HALLWAY] = Region("Forest Temple MQ Straight Hallway", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_BOSS_KEY_CHEST, logic->SmallKeys(RR_FOREST_TEMPLE, 3)),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_FLOORMASTER_ROOM, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_FLOORMASTER_ROOM] = Region("Forest Temple MQ Floormaster Room", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_OUTDOOR_LEDGE, []{return Here(RR_FOREST_TEMPLE_MQ_FLOORMASTER_ROOM, []{return logic->CanKillEnemy(RE_FLOORMASTER);});}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_OUTDOOR_LEDGE] = Region("Forest Temple MQ Outdoor Ledge", SCENE_FOREST_TEMPLE, {
        EventAccess(&logic->ForestCanTwistHallway, []{return logic->CanHitSwitch();}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_REDEAD_CHEST,           logic->CanKillEnemy(RE_REDEAD)),
        LOCATION(RC_FOREST_TEMPLE_MQ_COURTYARD_RIGHT_HEART,  true),
        LOCATION(RC_FOREST_TEMPLE_MQ_COURTYARD_MIDDLE_HEART, true),
        LOCATION(RC_FOREST_TEMPLE_MQ_COURTYARD_LEFT_HEART,   true),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_NW_OUTDOORS, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_NW_OUTDOORS] = Region("Forest Temple MQ NW Outdoors", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_LEVEL_ISLAND_COURTYARD, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
        //the well checks are considered from both areas instead of being a region because the draining is a temp flag and the skull (as well as the chest with hook glitch) has different breath timers from each side
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_WELL,                   (logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8 && logic->CanUse(RG_HOOKSHOT))),
        LOCATION(RC_FOREST_TEMPLE_MQ_COURTYARD_RIGHT_HEART,     logic->CanUse(RG_BOOMERANG) && ctx->GetTrickOption(RT_FOREST_OUTDOORS_HEARTS_BOOMERANG)),
        LOCATION(RC_FOREST_TEMPLE_MQ_COURTYARD_MIDDLE_HEART,    logic->CanUse(RG_BOOMERANG) && ctx->GetTrickOption(RT_FOREST_OUTDOORS_HEARTS_BOOMERANG)),
        LOCATION(RC_FOREST_TEMPLE_MQ_COURTYARD_LEFT_HEART,      logic->CanUse(RG_BOOMERANG) && ctx->GetTrickOption(RT_FOREST_OUTDOORS_HEARTS_BOOMERANG)),
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_WEST_HEART,           logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8),
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_MIDDLE_HEART,         logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8),
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_EAST_HEART,           logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS,         []{return (((logic->CanUse(RG_IRON_BOOTS) || logic->CanUse(RG_LONGSHOT) || (ctx->GetTrickOption(RT_FOREST_MQ_WELL_SWIM) && logic->CanUse(RG_HOOKSHOT))) && logic->HasItem(RG_BRONZE_SCALE)) || logic->HasItem(RG_GOLDEN_SCALE)) && logic->WaterTimer() >= 16;}),
        Entrance(RR_FOREST_TEMPLE_MQ_OUTDOORS_TOP_LEDGES, []{return logic->CanUse(RG_FIRE_ARROWS);}),
    });

    //The well only coniders the eye target here because the eye target is a temp flag, making it unwieldy to use as an EventAccess to make it it's own room
    areaTable[RR_FOREST_TEMPLE_MQ_NE_OUTDOORS] = Region("Forest Temple MQ NE Outdoors", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->DekuBabaSticks, []{return logic->CanGetDekuBabaSticks();}),
        EventAccess(&logic->DekuBabaNuts,   []{return logic->CanGetDekuBabaNuts();}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_CHEST,                 logic->CanHitEyeTargets()),
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_RAISED_ISLAND_COURTYARD, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
        //implies logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_WELL,                    logic->CanHitEyeTargets() || (logic->CanUse(RG_IRON_BOOTS) && logic->CanUse(RG_HOOKSHOT))),
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_WEST_HEART,            (logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8) || logic->CanHitEyeTargets()),
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_MIDDLE_HEART,          (logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8) || logic->CanHitEyeTargets()),
        LOCATION(RC_FOREST_TEMPLE_MQ_WELL_EAST_HEART,            (logic->CanUse(RG_IRON_BOOTS) && logic->WaterTimer() >= 8) || logic->CanHitEyeTargets()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_NW_OUTDOORS,         []{return (((logic->CanUse(RG_IRON_BOOTS) || logic->CanUse(RG_LONGSHOT)) && logic->HasItem(RG_BRONZE_SCALE)) || logic->HasItem(RG_GOLDEN_SCALE)) && logic->WaterTimer() >= 16;}),
        Entrance(RR_FOREST_TEMPLE_MQ_OUTDOORS_TOP_LEDGES, []{return logic->CanUse(RG_LONGSHOT) || (logic->CanUse(RG_HOOKSHOT) && ((logic->IsAdult && logic->CanUse(RG_HOVER_BOOTS)) || logic->CanUse(RG_SONG_OF_TIME)));}),
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS_LEDGE,   []{return logic->CanUse(RG_LONGSHOT);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_OUTDOORS_TOP_LEDGES] = Region("Forest Temple MQ Outdoors Top Ledges", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_RAISED_ISLAND_COURTYARD_UPPER_CHEST, true),
        //Actually killing the skull from the doorframe with melee is annoying. Hammer swing hits low enough unaided, other swords need to crouch stab but the spot is precise based on range. kokiri sword doesn't reach at all for adult.
        LOCATION(RC_FOREST_TEMPLE_MQ_GS_RAISED_ISLAND_COURTYARD,          ((logic->IsAdult && logic->CanUse(RG_SONG_OF_TIME)) || (logic->CanUse(RG_HOVER_BOOTS) && ctx->GetTrickOption(RT_FOREST_DOORFRAME))) && logic->CanJumpslash() && (logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->BlastOrSmash() || logic->CanUse(RG_DINS_FIRE) || logic->CanUse(RG_FAIRY_BOW) || logic->HookshotOrBoomerang() || (logic->CanStandingShield() && (logic->CanUse(RG_STICKS) || logic->CanUse(RG_BIGGORON_SWORD) || logic->CanUse(RG_MASTER_SWORD) || (logic->IsChild && logic->CanUse(RG_KOKIRI_SWORD)))))),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_NW_OUTDOORS,       []{return logic->HasFireSourceWithTorch();}),
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS,       []{return true;}),
        //N64 logic doesn't check damage but I always take some so I'm adding it
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS_LEDGE, []{return ctx->GetTrickOption(RT_FOREST_OUTDOORS_LEDGE) && logic->CanUse(RG_HOVER_BOOTS) && logic->CanJumpslash() && logic->TakeDamage();}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_NE_OUTDOORS_LEDGE] = Region("Forest Temple MQ NE Outdoors Ledge", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_RAISED_ISLAND_COURTYARD_LOWER_CHEST, true),
    }, {
        //Exits
        //Skipping swim here is non-trival, needs a roll-jump. If a swim lock is added it's probably wise to copy deku baba events here
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS,  []{return true;}),
        Entrance(RR_FOREST_TEMPLE_MQ_FALLING_ROOM, []{return logic->CanUse(RG_SONG_OF_TIME);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_JOELLE_ROOM] = Region("Forest Temple MQ Joelle room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleJoelle, []{return logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_MAP_CHEST, logic->ForestTempleJoelle),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_UPPER_BLOCK_PUZZLE, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 4);}),
        Entrance(RR_FOREST_TEMPLE_MQ_3_STALFOS_ROOM,     []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_3_STALFOS_ROOM] = Region("Forest Temple MQ 3 Stalfos Room", SCENE_FOREST_TEMPLE, {
        //Events
        //technically happens in RR_FOREST_TEMPLE_MQ_WOLFOS_ROOM, but the way this room blocks the hole means it cannot be logical to do anything else there.
        EventAccess(&logic->ForestClearBelowBowChest, []{return logic->CanKillEnemy(RE_WOLFOS);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_BOW_CHEST,           logic->ForestClearBelowBowChest && logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 3)),
        LOCATION(RC_FOREST_TEMPLE_MQ_UPPER_STALFOS_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_UPPER_STALFOS_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_UPPER_STALFOS_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_UPPER_STALFOS_POT_4, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_JOELLE_ROOM, []{return logic->ForestClearBelowBowChest && logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 3);}),
        Entrance(RR_FOREST_TEMPLE_MQ_BETH_ROOM,   []{return logic->ForestClearBelowBowChest && logic->CanKillEnemy(RE_STALFOS, ED_CLOSE, true, 3);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_BETH_ROOM] = Region("Forest Temple MQ Beth Room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleBeth, []{return logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_COMPASS_CHEST,  logic->ForestTempleBeth),
        LOCATION(RC_FOREST_TEMPLE_MQ_BLUE_POE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_BLUE_POE_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_BLUE_POE_POT_3, logic->CanBreakPots()),
    }, {
        //Exits
        //!QUANTUM LOGIC!
        //This key logic assumes that you can get to falling room either by spending the 5th key here, or by wasting a key in falling room itself.
        //While being the 5th key makes this simpler in theory, if a different age can waste the key compared to reaching this room it breaks
        Entrance(RR_FOREST_TEMPLE_MQ_FALLING_ROOM,    []{return logic->SmallKeys(RR_FOREST_TEMPLE, 5) && Here(RR_FOREST_TEMPLE_MQ_BETH_ROOM, []{return logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_DINS_FIRE);});}),
        Entrance(RR_FOREST_TEMPLE_MQ_TORCH_SHOT_ROOM, []{return logic->SmallKeys(RR_FOREST_TEMPLE, 6);}),
        Entrance(RR_FOREST_TEMPLE_MQ_3_STALFOS_ROOM,  []{return true;}),
    });

    //This room exists to show the actual map layout, and for when the crates get added to logic
    areaTable[RR_FOREST_TEMPLE_MQ_TORCH_SHOT_ROOM] = Region("Forest Temple MQ Torch Shot Room", SCENE_FOREST_TEMPLE, {}, {
        // Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_FROZEN_EYE_SWITCH_SMALL_CRATE_1, logic->CanBreakSmallCrates()),
        LOCATION(RC_FOREST_TEMPLE_MQ_FROZEN_EYE_SWITCH_SMALL_CRATE_2, logic->CanBreakSmallCrates()),
        LOCATION(RC_FOREST_TEMPLE_MQ_FROZEN_EYE_SWITCH_SMALL_CRATE_3, logic->CanBreakSmallCrates()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_FALLING_ROOM, []{return logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_DINS_FIRE);}),
        Entrance(RR_FOREST_TEMPLE_MQ_BETH_ROOM,    []{return logic->SmallKeys(RR_FOREST_TEMPLE, 6);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_FALLING_ROOM] = Region("Forest Temple MQ Falling Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_FALLING_CEILING_ROOM_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_NE_OUTDOORS_LEDGE, []{return true;}),
        Entrance(RR_FOREST_TEMPLE_MQ_AMY_ROOM,          []{return logic->SmallKeys(RR_FOREST_TEMPLE, 6);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_AMY_ROOM] = Region("Forest Temple MQ Amy Room", SCENE_FOREST_TEMPLE, {
        //Events
        EventAccess(&logic->ForestTempleAmy, []{return logic->CanUse(RG_FAIRY_BOW);}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_GREEN_POE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_GREEN_POE_POT_2, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA, []{return logic->ForestTempleAmy;}),
        Entrance(RR_FOREST_TEMPLE_MQ_FALLING_ROOM, []{return true;}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_BASEMENT] = Region("Forest Temple MQ Basement", SCENE_FOREST_TEMPLE, {
        //Events
        //Implies CanHitSwitch()
        EventAccess(&logic->ForestOpenBossCorridor, []{return logic->CanHitEyeTargets();}),
    }, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_BASEMENT_CHEST, true),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_CENTRAL_AREA,      []{return logic->ForestTempleMeg;}),
        Entrance(RR_FOREST_TEMPLE_MQ_BASEMENT_POT_ROOM, []{return logic->CanPassEnemy(RE_BIG_SKULLTULA) || logic->TakeDamage();}),
        Entrance(RR_FOREST_TEMPLE_MQ_BOSS_REGION,       []{return logic->ForestOpenBossCorridor;}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_BASEMENT_POT_ROOM] = Region("Forest Temple MQ Basement Pot Room", SCENE_FOREST_TEMPLE, {}, {
        //Locations
        LOCATION(RC_FOREST_TEMPLE_MQ_BASEMENT_POT_1, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_BASEMENT_POT_2, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_BASEMENT_POT_3, logic->CanBreakPots()),
        LOCATION(RC_FOREST_TEMPLE_MQ_BASEMENT_POT_4, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_BASEMENT, []{return logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
    });

    areaTable[RR_FOREST_TEMPLE_MQ_BOSS_REGION] = Region("Forest Temple MQ Boss Region", SCENE_FOREST_TEMPLE, {}, {}, {
        //Exits
        Entrance(RR_FOREST_TEMPLE_MQ_BASEMENT,   []{return logic->ForestOpenBossCorridor;}),
        Entrance(RR_FOREST_TEMPLE_BOSS_ENTRYWAY, []{return true;}),
    });

#pragma endregion

    // Boss Room
    areaTable[RR_FOREST_TEMPLE_BOSS_ENTRYWAY] = Region("Forest Temple Boss Entryway", SCENE_FOREST_TEMPLE, {}, {}, {
        // Exits
        Entrance(RR_FOREST_TEMPLE_BOSS_REGION,    []{return ctx->GetDungeon(FOREST_TEMPLE)->IsVanilla() && false;}),
        Entrance(RR_FOREST_TEMPLE_MQ_BOSS_REGION, []{return ctx->GetDungeon(FOREST_TEMPLE)->IsMQ() && false;}),
        Entrance(RR_FOREST_TEMPLE_BOSS_ROOM,      []{return logic->HasItem(RG_FOREST_TEMPLE_BOSS_KEY);}),
    });

    areaTable[RR_FOREST_TEMPLE_BOSS_ROOM] = Region("Forest Temple Boss Room", SCENE_FOREST_TEMPLE_BOSS, {
        // Events
        EventAccess(&logic->ForestTempleClear, []{return logic->CanKillEnemy(RE_PHANTOM_GANON);}),
    }, {
        // Locations
        LOCATION(RC_FOREST_TEMPLE_PHANTOM_GANON_HEART, logic->ForestTempleClear),
        LOCATION(RC_PHANTOM_GANON,                     logic->ForestTempleClear),
    }, {
        // Exits
        Entrance(RR_FOREST_TEMPLE_BOSS_ENTRYWAY, []{return false;}),
        Entrance(RR_SACRED_FOREST_MEADOW,        []{return logic->ForestTempleClear;}, false),
    });

    // clang-format on
}
