#include "soh/Enhancements/randomizer/location_access.h"
#include "soh/Enhancements/randomizer/entrance.h"

using namespace Rando;

void RegionTable_Init_DesertColossus() {
    // clang-format off
    areaTable[RR_DESERT_COLOSSUS] = Region("Desert Colossus", SCENE_DESERT_COLOSSUS, {
        //Events
        EventAccess(&logic->FairyPond, []{return logic->CanUse(RG_SONG_OF_STORMS);}),
        EventAccess(&logic->BugRock,   []{return true;}),
    }, {
        //Locations
        LOCATION(RC_COLOSSUS_FREESTANDING_POH,       logic->IsAdult && CanPlantBean(RR_DESERT_COLOSSUS)),
        LOCATION(RC_COLOSSUS_GS_BEAN_PATCH,          logic->CanSpawnSoilSkull() && logic->CanAttack()),
        LOCATION(RC_COLOSSUS_GS_TREE,                logic->IsAdult && logic->HookshotOrBoomerang() && logic->CanGetNightTimeGS()),
        LOCATION(RC_COLOSSUS_GS_HILL,                logic->IsAdult && ((CanPlantBean(RR_DESERT_COLOSSUS) && logic->CanAttack()) || logic->CanUse(RG_LONGSHOT) || (ctx->GetTrickOption(RT_COLOSSUS_GS) && logic->CanUse(RG_HOOKSHOT))) && logic->CanGetNightTimeGS()),
        LOCATION(RC_COLOSSUS_BEAN_SPROUT_FAIRY_1,    logic->IsChild && logic->CanUse(RG_MAGIC_BEAN) && logic->CanUse(RG_SONG_OF_STORMS)),
        LOCATION(RC_COLOSSUS_BEAN_SPROUT_FAIRY_2,    logic->IsChild && logic->CanUse(RG_MAGIC_BEAN) && logic->CanUse(RG_SONG_OF_STORMS)),
        LOCATION(RC_COLOSSUS_BEAN_SPROUT_FAIRY_3,    logic->IsChild && logic->CanUse(RG_MAGIC_BEAN) && logic->CanUse(RG_SONG_OF_STORMS)),
        LOCATION(RC_COLOSSUS_GOSSIP_STONE_FAIRY,     logic->CallGossipFairy()),
        LOCATION(RC_COLOSSUS_GOSSIP_STONE_FAIRY_BIG, logic->CanUse(RG_SONG_OF_STORMS)),
        LOCATION(RC_COLOSSUS_GOSSIP_STONE,           true),
    }, {
        //Exits
        //You can kinda get the fairies without entering the water, but it relies on them cooperating and leevers are jerks. should be a trick
        Entrance(RR_DESERT_COLOSSUS_OASIS,         []{return logic->CanUse(RG_SONG_OF_STORMS) && (logic->HasItem(RG_BRONZE_SCALE) || logic->CanUse(RG_IRON_BOOTS));}),
        Entrance(RR_COLOSSUS_GREAT_FAIRY_FOUNTAIN, []{return logic->HasExplosives();}),
        Entrance(RR_SPIRIT_TEMPLE_ENTRYWAY,        []{return true;}),
        Entrance(RR_WASTELAND_NEAR_COLOSSUS,       []{return true;}),
        Entrance(RR_COLOSSUS_GROTTO,               []{return logic->CanUse(RG_SILVER_GAUNTLETS);}),
    });

    //specifically the full oasis, after the fairies have spawned
    areaTable[RR_DESERT_COLOSSUS_OASIS] = Region("Desert Colossus Oasis", SCENE_DESERT_COLOSSUS, {
        //Events
        EventAccess(&logic->FairyPond, []{return true;}),
    }, {
        //Locations
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_1, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_2, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_3, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_4, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_5, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_6, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_7, true),
        LOCATION(RC_COLOSSUS_OASIS_FAIRY_8, true),
    }, {
        //Exits
        Entrance(RR_DESERT_COLOSSUS, []{return true;}),
    });

    areaTable[RR_DESERT_COLOSSUS_OUTSIDE_TEMPLE] = Region("Desert Colossus From Spirit Entryway", SCENE_DESERT_COLOSSUS, {}, {
        //Locations
        LOCATION(RC_SHEIK_AT_COLOSSUS, true),
    }, {
        //Exist
        Entrance(RR_DESERT_COLOSSUS, []{return true;}),
    });

    areaTable[RR_COLOSSUS_GREAT_FAIRY_FOUNTAIN] = Region("Colossus Great Fairy Fountain", SCENE_GREAT_FAIRYS_FOUNTAIN_SPELLS, {}, {
        //Locations
        LOCATION(RC_COLOSSUS_GREAT_FAIRY_REWARD, logic->CanUse(RG_ZELDAS_LULLABY)),
    }, {
        //Exits
        Entrance(RR_DESERT_COLOSSUS, []{return true;}),
    });

    areaTable[RR_COLOSSUS_GROTTO] = Region("Colossus Grotto", SCENE_GROTTOS, {}, {
        //Locations
        LOCATION(RC_COLOSSUS_DEKU_SCRUB_GROTTO_REAR,  logic->CanStunDeku()),
        LOCATION(RC_COLOSSUS_DEKU_SCRUB_GROTTO_FRONT, logic->CanStunDeku()),
        LOCATION(RC_COLOSSUS_GROTTO_BEEHIVE,          logic->CanBreakUpperBeehives()),
    }, {
        //Exits
        Entrance(RR_DESERT_COLOSSUS, []{return true;}),
    });

    // clang-format on
}
