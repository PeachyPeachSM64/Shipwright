#include "soh/Enhancements/randomizer/location_access.h"
#include "soh/Enhancements/randomizer/entrance.h"

using namespace Rando;

/*
 * This file should be split into "gerudo_fortress.cpp" (overworld) & "thieves_hideout.cpp" (dungeons)
 * when the gerudo fortress refactor is done
 */

void RegionTable_Init_GerudoFortress() {
    // clang-format off
    areaTable[RR_GERUDO_FORTRESS] = Region("Gerudo Fortress", SCENE_GERUDOS_FORTRESS, {
        //Events
        EventAccess(&logic->CarpenterRescue, []{return logic->CanFinishGerudoFortress();}),
        EventAccess(&logic->GF_GateOpen,     []{return logic->IsAdult && logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD);}),
        EventAccess(&logic->GtG_GateOpen,    []{return logic->IsAdult && logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) && logic->HasItem(RG_CHILD_WALLET);}),
    }, {
        //Locations
        LOCATION(RC_GF_CHEST,                         logic->CanUse(RG_HOVER_BOOTS) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || logic->CanUse(RG_LONGSHOT)),
        LOCATION(RC_GF_HBA_1000_POINTS,               logic->HasItem(RG_CHILD_WALLET) && logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) && logic->CanUse(RG_EPONA) && logic->CanUse(RG_FAIRY_BOW) && logic->AtDay),
        LOCATION(RC_GF_HBA_1500_POINTS,               logic->HasItem(RG_CHILD_WALLET) && logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) && logic->CanUse(RG_EPONA) && logic->CanUse(RG_FAIRY_BOW) && logic->AtDay),
        LOCATION(RC_GF_NORTH_F1_CARPENTER,            logic->CanKillEnemy(RE_GERUDO_WARRIOR)),
        LOCATION(RC_GF_NORTH_F2_CARPENTER,            (logic->CanKillEnemy(RE_GERUDO_WARRIOR)) && (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_HOVER_BOOTS) || ctx->GetTrickOption(RT_GF_KITCHEN))),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER,            logic->CanKillEnemy(RE_GERUDO_WARRIOR)),
        LOCATION(RC_GF_SOUTH_F2_CARPENTER,            logic->CanKillEnemy(RE_GERUDO_WARRIOR)),
        LOCATION(RC_GF_GERUDO_MEMBERSHIP_CARD,        logic->CanFinishGerudoFortress()),
        LOCATION(RC_GF_GS_ARCHERY_RANGE,              logic->IsAdult && logic->HookshotOrBoomerang() && logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) && logic->CanGetNightTimeGS()),
        LOCATION(RC_GF_GS_TOP_FLOOR,                  logic->IsAdult && (logic->CanJumpslashExceptHammer() || logic->CanUse(RG_FAIRY_SLINGSHOT) || logic->CanUse(RG_BOOMERANG) || logic->HasExplosives() || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_DINS_FIRE)) && (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT) || logic->CanUse(RG_HOVER_BOOTS) || ctx->GetTrickOption(RT_GF_KITCHEN) || ctx->GetTrickOption(RT_GF_JUMP)) && logic->CanGetNightTimeGS()),
        LOCATION(RC_GF_BREAK_ROOM_POT_1,              (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || (logic->CanUse(RG_HOVER_BOOTS) || (ctx->GetTrickOption(RT_GF_JUMP) && (ctx->GetTrickOption(RT_GF_KITCHEN) || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT))))) && logic->CanBreakPots()),
        LOCATION(RC_GF_BREAK_ROOM_POT_2,              (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || (logic->CanUse(RG_HOVER_BOOTS) || (ctx->GetTrickOption(RT_GF_JUMP) && (ctx->GetTrickOption(RT_GF_KITCHEN) || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT))))) && logic->CanBreakPots()),
        LOCATION(RC_GF_KITCHEN_POT_1,                 (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakPots()),
        LOCATION(RC_GF_KITCHEN_POT_2,                 (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakPots()),
        LOCATION(RC_GF_NORTH_F1_CARPENTER_POT_1,      logic->CanBreakPots()),
        LOCATION(RC_GF_NORTH_F1_CARPENTER_POT_2,      logic->CanBreakPots()),
        LOCATION(RC_GF_NORTH_F1_CARPENTER_POT_3,      logic->CanBreakPots()),
        LOCATION(RC_GF_NORTH_F2_CARPENTER_POT_1,      logic->CanBreakPots()),
        LOCATION(RC_GF_NORTH_F2_CARPENTER_POT_2,      logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_POT_1,      logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_POT_2,      logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_POT_3,      logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_CELL_POT_1, logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_CELL_POT_2, logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_CELL_POT_3, logic->CanBreakPots()),
        LOCATION(RC_GF_SOUTH_F1_CARPENTER_CELL_POT_4, logic->CanBreakPots()),
        LOCATION(RC_GF_ABOVE_JAIL_CRATE,              (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || (logic->CanUse(RG_HOOKSHOT) && ctx->GetTrickOption(RT_GF_JUMP))) && logic->CanBreakCrates()),
        LOCATION(RC_GF_OUTSIDE_CENTER_CRATE_1,        logic->CanBreakCrates()),
        LOCATION(RC_GF_OUTSIDE_CENTER_CRATE_2,        logic->CanBreakCrates()),
        LOCATION(RC_GF_OUTSIDE_CENTER_CRATE_3,        logic->CanBreakCrates()),
        LOCATION(RC_GF_OUTSIDE_CENTER_CRATE_4,        logic->CanBreakCrates()),
        LOCATION(RC_GF_OUTSIDE_LEFT_CRATE_1,          (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_OUTSIDE_LEFT_CRATE_2,          (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_1,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_2,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_3,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_4,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_5,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_6,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RANGE_CRATE_7,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_START_CRATE_1,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_START_CRATE_2,         (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_LEFT_END_CRATE_1,      (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_LEFT_END_CRATE_2,      (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && (logic->IsAdult || (logic->BlastOrSmash() || logic->HookshotOrBoomerang() || logic->CanUse(RG_HOVER_BOOTS)))),
        LOCATION(RC_GF_ARCHERY_LEFT_END_CHILD_CRATE,  logic->IsChild && logic->HasExplosives() && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RIGHT_END_CRATE_1,     (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_ARCHERY_RIGHT_END_CRATE_2,     (logic->IsChild || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_KITCHEN_CRATE_1,               (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_KITCHEN_CRATE_2,               (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_KITCHEN_CRATE_3,               (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_KITCHEN_CRATE_4,               (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_KITCHEN_CRATE_5,               (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_BREAK_ROOM_CRATE_1,            (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || (logic->CanUse(RG_HOVER_BOOTS) || (ctx->GetTrickOption(RT_GF_JUMP) && (ctx->GetTrickOption(RT_GF_KITCHEN) || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT))))) && logic->CanBreakCrates()),
        LOCATION(RC_GF_BREAK_ROOM_CRATE_2,            (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || (logic->CanUse(RG_HOVER_BOOTS) || (ctx->GetTrickOption(RT_GF_JUMP) && (ctx->GetTrickOption(RT_GF_KITCHEN) || logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT))))) && logic->CanBreakCrates()),
        LOCATION(RC_GF_BREAK_ROOM_CRATE_3,            (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || ((logic->CanUse(RG_HOVER_BOOTS) || ctx->GetTrickOption(RT_GF_JUMP)) && (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW)))) && logic->CanBreakCrates()),
        LOCATION(RC_GF_BREAK_ROOM_CRATE_4,            (logic->CanUse(RG_LONGSHOT) || (logic->IsAdult && logic->CanUse(RG_SCARECROW)) || ((logic->CanUse(RG_HOVER_BOOTS) || ctx->GetTrickOption(RT_GF_JUMP)) && (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW)))) && logic->CanBreakCrates()),
        LOCATION(RC_GF_NORTH_F1_CARPENTER_CRATE,      (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_NORTH_F3_CARPENTER_CRATE,      (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanBreakCrates()),
        LOCATION(RC_GF_SOUTH_F2_CARPENTER_CRATE_1,    logic->CanBreakCrates()),
        LOCATION(RC_GF_SOUTH_F2_CARPENTER_CRATE_2,    logic->CanBreakCrates()),


        //RANDOTODO doublecheck when GF isn't a blob
        LOCATION(RC_GF_KITCHEN_SUN_FAIRY,             (logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) || logic->CanUse(RG_FAIRY_BOW) || logic->CanUse(RG_HOOKSHOT)) && logic->CanUse(RG_SUNS_SONG)),
    }, {
        //Exits
        Entrance(RR_GV_FORTRESS_SIDE,                []{return true;}),
        Entrance(RR_GF_OUTSIDE_GATE,                 []{return logic->GF_GateOpen;}),
        Entrance(RR_GERUDO_TRAINING_GROUND_ENTRYWAY, []{return logic->GtG_GateOpen && (logic->IsAdult || ctx->GetOption(RSK_SHUFFLE_DUNGEON_ENTRANCES));}),
        Entrance(RR_GF_STORMS_GROTTO,                []{return logic->IsAdult && logic->CanOpenStormsGrotto();}),
    });

    areaTable[RR_GF_OUTSIDE_GATE] = Region("GF Outside Gate", SCENE_GERUDOS_FORTRESS, {
        //Events
        EventAccess(&logic->GF_GateOpen, []{return logic->IsAdult && logic->HasItem(RG_GERUDO_MEMBERSHIP_CARD) && (ctx->GetOption(RSK_SHUFFLE_GERUDO_MEMBERSHIP_CARD) || ctx->GetOption(RSK_SHUFFLE_OVERWORLD_ENTRANCES) /*|| ShuffleSpecialIndoorEntrances*/);}),
    }, {}, {
        //Exits
        Entrance(RR_GERUDO_FORTRESS,         []{return (logic->IsAdult && (logic->CanUse(RG_HOOKSHOT) || !ctx->GetOption(RSK_SHUFFLE_OVERWORLD_ENTRANCES))) || logic->GF_GateOpen;}),
        Entrance(RR_WASTELAND_NEAR_FORTRESS, []{return true;}),
    });

    areaTable[RR_GF_STORMS_GROTTO] = Region("GF Storms Grotto", SCENE_GROTTOS, {
        //Events
        EventAccess(&logic->FreeFairies, []{return true;}),
    }, {
        //Locations
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_1, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_2, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_3, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_4, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_5, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_6, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_7, true),
        LOCATION(RC_GF_FAIRY_GROTTO_FAIRY_8, true),
    }, {
        //Exits
        Entrance(RR_GERUDO_FORTRESS, []{return true;}),
    });

    // clang-format on
}
