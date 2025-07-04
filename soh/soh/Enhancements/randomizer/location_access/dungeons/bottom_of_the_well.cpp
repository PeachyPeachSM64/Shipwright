#include "soh/Enhancements/randomizer/location_access.h"
#include "soh/Enhancements/randomizer/entrance.h"
#include "soh/Enhancements/randomizer/dungeon.h"

using namespace Rando;

void RegionTable_Init_BottomOfTheWell() {
    // clang-format off
    // Vanilla/MQ Decider
    areaTable[RR_BOTTOM_OF_THE_WELL_ENTRYWAY] = Region("Bottom of the Well Entryway", SCENE_BOTTOM_OF_THE_WELL, {}, {}, {
        //Exits
        //Technically involves an fake wall, but passing it lensless is intended in vanilla and it is well telegraphed
        Entrance(RR_BOTTOM_OF_THE_WELL_PERIMETER,    []{return ctx->GetDungeon(Rando::BOTTOM_OF_THE_WELL)->IsVanilla() && logic->IsChild && logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return ctx->GetDungeon(Rando::BOTTOM_OF_THE_WELL)->IsMQ()      && logic->IsChild;}),
        Entrance(RR_KAK_WELL,                        []{return true;}),
    });

#pragma region Vanilla

    areaTable[RR_BOTTOM_OF_THE_WELL_PERIMETER] = Region("Bottom of the Well Perimeter", SCENE_BOTTOM_OF_THE_WELL, {
        //Events
        EventAccess(&logic->StickPot,               []{return true;}),
        EventAccess(&logic->NutPot,                 []{return true;}),
        EventAccess(&logic->LoweredWaterInsideBotw, []{return logic->CanUse(RG_ZELDAS_LULLABY);}),
    }, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_FRONT_CENTER_BOMBABLE_CHEST,  logic->HasExplosives()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_UNDERWATER_FRONT_CHEST,       logic->LoweredWaterInsideBotw),
        LOCATION(RC_BOTTOM_OF_THE_WELL_UNDERWATER_LEFT_CHEST,        logic->LoweredWaterInsideBotw),
        LOCATION(RC_BOTTOM_OF_THE_WELL_NEAR_ENTRANCE_POT_1,          logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_NEAR_ENTRANCE_POT_2,          logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_UNDERWATER_POT,               (logic->CanBreakPots() && logic->LoweredWaterInsideBotw) || logic->CanUse(RG_BOOMERANG)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_ENTRYWAY,          []{return logic->IsChild && logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_BEHIND_FAKE_WALLS, []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_SOUTHWEST_ROOM,    []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_KEESE_BEAMOS_ROOM, []{return logic->IsChild && logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 3);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_COFFIN_ROOM,       []{return logic->LoweredWaterInsideBotw || logic->HasItem(RG_BRONZE_SCALE);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_DEAD_HAND_ROOM,    []{return logic->LoweredWaterInsideBotw && logic->IsChild;}),
        //Falling down into basement requires nothing, but falling down somewhere specific requires lens or lens trick
        //kinda questionable given several drops are blocked by rocks, but that's how it was handled before and on N64
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT,          []{return true;}),
    });

    //This region combines the Middle with the perimeter's hidden areas. If a warp puts link into the middle without crossing the perimeter or using lens, it will need it's own region
    areaTable[RR_BOTTOM_OF_THE_WELL_BEHIND_FAKE_WALLS] = Region("Bottom of the Well Behind Fake Walls", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_FRONT_LEFT_FAKE_WALL_CHEST,   true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_RIGHT_BOTTOM_FAKE_WALL_CHEST, true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_COMPASS_CHEST,                true),
        //You can just barely pass the spider on the right side without damage or items, but it's probably tight enough to count as as a trick
        LOCATION(RC_BOTTOM_OF_THE_WELL_CENTER_SKULLTULA_CHEST,       logic->CanPassEnemy(RE_BIG_SKULLTULA) || logic->TakeDamage()),
        //Not technically behind a wall, but still logically needs lens due to pits
        LOCATION(RC_BOTTOM_OF_THE_WELL_BACK_LEFT_BOMBABLE_CHEST,     logic->HasExplosives()),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_PERIMETER,         []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_INNER_ROOMS,       []{return logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 3);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT,          []{return true;}),
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM, []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
    });

    //This area can be reached without lens in logic from basement, but that could require silver rupees if they are shuffled.
    areaTable[RR_BOTTOM_OF_THE_WELL_SOUTHWEST_ROOM] = Region("Bottom of the Well Southwest Room", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_LEFT_SIDE_POT_1, logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_LEFT_SIDE_POT_2, logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_LEFT_SIDE_POT_3, logic->CanBreakPots()),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_PERIMETER, []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
    });

    //Passing through this area needs lens, but entering doesn't, so that the fire keese can be killed without crossing the pits if enemy drops are ever shuffled
    areaTable[RR_BOTTOM_OF_THE_WELL_KEESE_BEAMOS_ROOM] = Region("Bottom of the Well Keese-Beamos Room", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_FIRE_KEESE_CHEST, ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_FIRE_KEESE_POT_1, logic->CanBreakPots() && (ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH))),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_PERIMETER,                    []{return logic->IsChild && logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 3) && (ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH));}),
        Entrance(RR_BOTTOM_OF_THE_WELL_LIKE_LIKE_CAGE,               []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
        //not sure if this lens check is needed, these holes are a bit too easy to find, but it matches existing logic
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT_USEFUL_BOMB_FLOWERS, []{return ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_LIKE_LIKE_CAGE] = Region("Bottom of the Well Like-Like Cage", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_LIKE_LIKE_CHEST,   true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_GS_LIKE_LIKE_CAGE, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA, ED_BOOMERANG)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_KEESE_BEAMOS_ROOM, []{return true;}),
    });

    //If the player can voidwarp into one of these rooms they will need splitting up, and Fake walls will need specifying into middle and the rest moved to perimeter
    areaTable[RR_BOTTOM_OF_THE_WELL_INNER_ROOMS] = Region("Bottom of the Well Inner Rooms", SCENE_BOTTOM_OF_THE_WELL, {
        //Events
        EventAccess(&logic->DekuBabaSticks, []{return logic->CanGetDekuBabaSticks();}),
        EventAccess(&logic->DekuBabaNuts,   []{return logic->CanGetDekuBabaNuts();}),
    }, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_GS_WEST_INNER_ROOM, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA, ED_BOOMERANG)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_GS_EAST_INNER_ROOM, logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA, ED_BOOMERANG)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_BEHIND_FAKE_WALLS, []{return logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 3);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_COFFIN_ROOM] = Region("Bottom of the Well Coffin Room", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_FREESTANDING_KEY,                logic->HasFireSourceWithTorch() || logic->CanUse(RG_FAIRY_BOW)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_COFFIN_ROOM_FRONT_LEFT_HEART,    true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_COFFIN_ROOM_MIDDLE_RIGHT_HEART,  logic->HasFireSourceWithTorch() || logic->CanUse(RG_FAIRY_BOW)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_PERIMETER, []{return logic->LoweredWaterInsideBotw || logic->HasItem(RG_BRONZE_SCALE);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_DEAD_HAND_ROOM] = Region("Bottom of the Well Dead Hand Room", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_LENS_OF_TRUTH_CHEST, logic->CanKillEnemy(RE_DEAD_HAND)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_INVISIBLE_CHEST,     (ctx->GetTrickOption(RT_LENS_BOTW) || logic->CanUse(RG_LENS_OF_TRUTH))),
    }, {
        //Exits
        //This assumes we spawned in dead hand's room, if whatever trick made this relevant instead puts us in the previous room, remove the kill Dead Hand check.
        Entrance(RR_BOTTOM_OF_THE_WELL_PERIMETER, []{return logic->IsChild && logic->CanKillEnemy(RE_DEAD_HAND);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_BASEMENT] = Region("Bottom of the Well Basement", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_MAP_CHEST,                     logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_1,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_2,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_3,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_4,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_5,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_6,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_7,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_8,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_9,                logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_10,               logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_11,               logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_POT_12,               logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_SUN_FAIRY,            logic->CanUse(RG_SUNS_SONG)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_GRASS_1,              logic->CanCutShrubs()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_GRASS_2,              logic->CanCutShrubs()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_GRASS_3,              logic->CanCutShrubs()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_1, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_2, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_3, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_4, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_5, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_6, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_7, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_8, logic->CanCutShrubs() && logic->BlastOrSmash()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_BEHIND_ROCKS_GRASS_9, logic->CanCutShrubs() && logic->BlastOrSmash()),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_SOUTHWEST_ROOM,               []{return logic->IsChild && logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
        //It's possible to abuse boulder's limited range of collision detection to detonate the flowers through the boulder with bow, but this is a glitch
        //the exact range is just past the furthest away plank in the green goo section
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT_USEFUL_BOMB_FLOWERS, []{return Here(RR_BOTTOM_OF_THE_WELL_BASEMENT, []{return logic->BlastOrSmash() || logic->CanUse(RG_DINS_FIRE) || (logic->CanUse(RG_STICKS) && ctx->GetTrickOption(RT_BOTW_BASEMENT));});}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_BASEMENT_USEFUL_BOMB_FLOWERS] = Region("Bottom of the Well Basement Useful Bomb Flowers", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        //Assumes RR_BOTTOM_OF_THE_WELL_BASEMENT access
        LOCATION(RC_BOTTOM_OF_THE_WELL_MAP_CHEST, logic->HasItem(RG_GORONS_BRACELET)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT, []{return logic->CanDetonateUprightBombFlower();}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM] = Region("Bottom of the Well Basement Platform", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM_LEFT_RUPEE,        true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM_BACK_LEFT_RUPEE,   true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM_MIDDLE_RUPEE,      true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM_BACK_RIGHT_RUPEE,  true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_BASEMENT_PLATFORM_RIGHT_RUPEE,       true),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_BASEMENT, []{return true;}),
    });

#pragma endregion

#pragma region MQ

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER] = Region("Bottom of the Well MQ Perimeter", SCENE_BOTTOM_OF_THE_WELL, {
        //Events
        //technically obsolete due to a wonder item fairy which only needs a projectile, but we don't have an event var for it yet
        EventAccess(&logic->FairyPot,               []{return Here(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return logic->BlastOrSmash();}) && logic->CanHitEyeTargets();}),
        //It is possible to hit the water switch with a pot from RR_BOTTOM_OF_THE_WELL_MQ_MIDDLE, however the hitbox for making it activate is very unintuitive
        //You have to throw the pot from further back to hit the switch from the front instead of the top, trying to hit the "fingers" directly
        //This unintuitiveness means it should be a trick. ZL is needed to get a clear path to carry the pot
        EventAccess(&logic->LoweredWaterInsideBotw, []{return logic->CanJumpslash() || logic->CanUseProjectile();}),
    }, {
        //Locations
        //Implies CanBreakPots()
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_OUTER_LOBBY_POT,  Here(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return logic->BlastOrSmash();}) && logic->CanHitEyeTargets()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_BOMB_LEFT_HEART,  logic->HasExplosives()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_BOMB_RIGHT_HEART, logic->HasExplosives()),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_ENTRYWAY,            []{return logic->IsChild;}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_WEST_ROOM_SWITCH, []{return Here(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return logic->BlastOrSmash();}) && logic->CanPassEnemy(RE_BIG_SKULLTULA);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_COFFIN_ROOM,      []{return (logic->LoweredWaterInsideBotw || logic->HasItem(RG_BRONZE_SCALE)) && logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 2);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_LOCKED_CAGE,      []{return logic->IsChild && logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 2) && logic->CanUseProjectile();}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_ROOM,   []{return logic->IsChild && logic->LoweredWaterInsideBotw;}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_MIDDLE,           []{return logic->CanUse(RG_ZELDAS_LULLABY);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT,         []{return true;}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_WEST_ROOM_SWITCH] = Region("Bottom of the Well MQ West Room Switch", SCENE_BOTTOM_OF_THE_WELL, {
        //Events
        EventAccess(&logic->OpenedWestRoomMQBotw, []{return true;}),
    }, {}, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return logic->BlastOrSmash() && (logic->CanPassEnemy(RE_BIG_SKULLTULA) || ctx->GetTrickOption(RT_BOTW_MQ_PITS));}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_MIDDLE,    []{return (bool)ctx->GetTrickOption(RT_BOTW_MQ_PITS);}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT,  []{return true;}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_COFFIN_ROOM] = Region("Bottom of the Well MQ Coffin Room", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_GS_COFFIN_ROOM,                 logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_COFFIN_ROOM_FRONT_RIGHT_HEART,  logic->HasFireSourceWithTorch() || logic->CanUse(RG_FAIRY_BOW)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_COFFIN_ROOM_MIDDLE_LEFT_HEART,  logic->HasFireSourceWithTorch() || logic->CanUse(RG_FAIRY_BOW)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER,  []{return (logic->LoweredWaterInsideBotw || logic->HasItem(RG_BRONZE_SCALE)) && logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 2);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_LOCKED_CAGE] = Region("Bottom of the Well MQ Locked Cage", SCENE_BOTTOM_OF_THE_WELL, {
        //Events
        EventAccess(&logic->OpenedMiddleHoleMQBotw, []{return logic->HasExplosives();}),
    }, {}, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return logic->IsChild && logic->SmallKeys(RR_BOTTOM_OF_THE_WELL, 2);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_ROOM] = Region("Bottom of the Well MQ Dead Hand Room", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_COMPASS_CHEST,              logic->CanKillEnemy(RE_DEAD_HAND)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_FREESTANDING_KEY, logic->HasExplosives() || (ctx->GetTrickOption(RT_BOTW_MQ_DEADHAND_KEY) && logic->CanUse(RG_BOOMERANG))),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_GRASS_1,          logic->CanCutShrubs()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_GRASS_2,          logic->CanCutShrubs()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_GRASS_3,          logic->CanCutShrubs()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_DEAD_HAND_GRASS_4,          logic->CanCutShrubs()),
    }, {
        //Exits
        //This assumes we spawned in dead hand's room, if whatever trick made this relevant instead puts us in the previous room, remove the kill Dead Hand check.
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return logic->IsChild && logic->CanKillEnemy(RE_DEAD_HAND);}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_MIDDLE] = Region("Bottom of the Well MQ Middle", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_MAP_CHEST,                        true),
        //This location technically involves an invisible platform, but it's intended to do lensless in vanilla and is clearly signposted by pots.
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_EAST_INNER_ROOM_FREESTANDING_KEY, true),
        //The enemies in this room are invisible and crowd around the player, being awkward to deal with blind unless you already know how.
        //the right wall is safe, and can be followed to get behind the grave which you can then pull easily assuming you can tank invisible keese
        //Using a deku nut however stuns everything easily. and if you have a melee weapon you can kill the skull through the grave then grab the drop
        //though it can be hard to tell where the safe direct path to the grave is without lens.
        //Also you get cheap shotted on entry sometimes.
        //An MQ lens trick is recommended here, and a review of this room for OHKO logic what that is added is advised.
        //In the meantime I assume damage taken or the easy answer (nuts)
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_GS_WEST_INNER_ROOM,    logic->OpenedWestRoomMQBotw && (logic->TakeDamage() || logic->CanUse(RG_NUTS)) && logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_INNER_LOBBY_POT_1,     logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_INNER_LOBBY_POT_2,     logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_INNER_LOBBY_POT_3,     logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_EAST_INNER_ROOM_POT_1, logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_EAST_INNER_ROOM_POT_2, logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_EAST_INNER_ROOM_POT_3, logic->CanBreakPots()),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_CELL_SUN_FAIRY,        logic->CanUse(RG_SUNS_SONG)),
    }, {
        //Exits
        //If a relevant trick causes you to be able to warp into here without going through PERIMETER, a new eventAccess will be needed for lowering the gates with ZL
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT_SWITCH_PLATFORM, []{return logic->OpenedMiddleHoleMQBotw;}),
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT,                 []{return true;}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT] = Region("Bottom of the Well MQ Basement", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        //behind invisible big skulltulas, but with navi spotting it's easy to avoid them, or at worst, tank your way through as they do not block the path
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_GS_BASEMENT,                  logic->CanGetEnemyDrop(RE_GOLD_SKULLTULA)),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_BASEMENT_HALLWAY_FRONT_HEART, true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_BASEMENT_HALLWAY_LEFT_HEART,  true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_BASEMENT_HALLWAY_RIGHT_HEART, true),
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_BASEMENT_SUN_FAIRY,           logic->CanUse(RG_SUNS_SONG)),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_PERIMETER, []{return true;}),
    });

    areaTable[RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT_SWITCH_PLATFORM] = Region("Bottom of the Well MQ Basement Switch Platform", SCENE_BOTTOM_OF_THE_WELL, {}, {
        //Locations
        //Assumes RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT access
        //it is technically possible to get the chest before you get screamed at without rolling, but hard enough to be a trick if that is the requirement for something to be logical
        //With some kind of movement tech it's much easier, easy enough to be default logic, as the redeads don't lock on immediately in addition to the extra speed
        //leaving with no requirements for now but up for discussion.
        LOCATION(RC_BOTTOM_OF_THE_WELL_MQ_LENS_OF_TRUTH_CHEST, true),
    }, {
        //Exits
        Entrance(RR_BOTTOM_OF_THE_WELL_MQ_BASEMENT,  []{return true;}),
    });

#pragma endregion
    // clang-format on
}
