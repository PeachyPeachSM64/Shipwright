#include "randomizer_check_tracker.h"
#include "randomizer_entrance_tracker.h"
#include "randomizer_item_tracker.h"
#include "randomizerTypes.h"
#include "soh/OTRGlobals.h"
#include "soh/cvar_prefixes.h"
#include "soh/SaveManager.h"
#include "soh/ResourceManagerHelpers.h"
#include "soh/SohGui/UIWidgets.hpp"
#include "soh/SohGui/SohGui.hpp"
#include "dungeon.h"
#include "entrance.h"
#include "location_access.h"
#include "3drando/fill.hpp"
#include "soh/Enhancements/debugger/performanceTimer.h"

#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <libultraship/libultraship.h>
#include "location.h"
#include "item_location.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "z64item.h"
#include "randomizerTypes.h"
#include "fishsanity.h"

extern "C" {
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;
}
extern "C" GetItemEntry ItemTable_RetrieveEntry(s16 modIndex, s16 getItemID);

extern std::vector<ItemTrackerItem> dungeonRewardStones;
extern std::vector<ItemTrackerItem> dungeonRewardMedallions;
extern std::vector<ItemTrackerItem> songItems;
extern std::vector<ItemTrackerItem> equipmentItems;

using json = nlohmann::json;

namespace CheckTracker {

// settings
bool showShops;
bool showOverworldTokens;
bool showDungeonTokens;
bool showBeans;
bool showScrubs;
bool showMajorScrubs;
bool showMerchants;
bool showSongs;
bool showBeehives;
bool showCows;
bool showOverworldFreestanding;
bool showDungeonFreestanding;
bool showAdultTrade;
bool showKokiriSword;
bool showMasterSword;
bool showHyruleLoach;
bool showWeirdEgg;
bool showGerudoCard;
bool showOverworldPots;
bool showDungeonPots;
bool showOverworldGrass;
bool showDungeonGrass;
bool showOverworldCrates;
bool showDungeonCrates;
bool showFrogSongRupees;
bool showFairies;
bool showStartingMapsCompasses;
bool showKeysanity;
bool showGerudoFortressKeys;
bool showBossKeysanity;
bool showGanonBossKey;
bool showOcarinas;
bool show100SkullReward;
bool showLinksPocket;
bool fortressFast;
bool fortressNormal;

u8 fishsanityMode;
u8 fishsanityPondCount;
bool fishsanityAgeSplit;

// persistent during gameplay
bool initialized;
bool doAreaScroll;
bool previousShowHidden = false;
bool hideShopUnshuffledChecks = false;
bool alwaysShowGS = false;

static bool presetLoaded = false;
static ImVec2 presetPos;
static ImVec2 presetSize;

std::map<uint32_t, RandomizerCheck> startingShopItem = {
    { SCENE_KOKIRI_SHOP, RC_KF_SHOP_ITEM_1 },
    { SCENE_BAZAAR, RC_MARKET_BAZAAR_ITEM_1 },
    { SCENE_POTION_SHOP_MARKET, RC_MARKET_POTION_SHOP_ITEM_1 },
    { SCENE_BOMBCHU_SHOP, RC_MARKET_BOMBCHU_SHOP_ITEM_1 },
    { SCENE_POTION_SHOP_KAKARIKO, RC_KAK_POTION_SHOP_ITEM_1 },
    { SCENE_ZORA_SHOP, RC_ZD_SHOP_ITEM_1 },
    { SCENE_GORON_SHOP, RC_GC_SHOP_ITEM_1 },
};

std::map<SceneID, RandomizerCheckArea> DungeonRCAreasBySceneID = {
    { SCENE_DEKU_TREE, RCAREA_DEKU_TREE },
    { SCENE_DODONGOS_CAVERN, RCAREA_DODONGOS_CAVERN },
    { SCENE_JABU_JABU, RCAREA_JABU_JABUS_BELLY },
    { SCENE_FOREST_TEMPLE, RCAREA_FOREST_TEMPLE },
    { SCENE_FIRE_TEMPLE, RCAREA_FIRE_TEMPLE },
    { SCENE_WATER_TEMPLE, RCAREA_WATER_TEMPLE },
    { SCENE_SHADOW_TEMPLE, RCAREA_SHADOW_TEMPLE },
    { SCENE_SPIRIT_TEMPLE, RCAREA_SPIRIT_TEMPLE },
    { SCENE_BOTTOM_OF_THE_WELL, RCAREA_BOTTOM_OF_THE_WELL },
    { SCENE_ICE_CAVERN, RCAREA_ICE_CAVERN },
    { SCENE_GERUDO_TRAINING_GROUND, RCAREA_GERUDO_TRAINING_GROUND },
    { SCENE_INSIDE_GANONS_CASTLE, RCAREA_GANONS_CASTLE },
};

// Dungeon entrances with obvious visual differences between MQ and vanilla qualifying as spoiling on sight
std::vector<uint32_t> spoilingEntrances = {
    ENTR_DEKU_TREE_ENTRANCE,
    ENTR_DODONGOS_CAVERN_BOSS_DOOR,
    ENTR_JABU_JABU_ENTRANCE,
    ENTR_JABU_JABU_BOSS_DOOR,
    ENTR_FOREST_TEMPLE_ENTRANCE,
    ENTR_FIRE_TEMPLE_ENTRANCE,
    ENTR_FIRE_TEMPLE_BOSS_DOOR,
    ENTR_WATER_TEMPLE_BOSS_DOOR,
    ENTR_SPIRIT_TEMPLE_ENTRANCE,
    ENTR_SHADOW_TEMPLE_BOSS_DOOR,
    ENTR_ICE_CAVERN_ENTRANCE,
    ENTR_GERUDO_TRAINING_GROUND_ENTRANCE,
    ENTR_INSIDE_GANONS_CASTLE_ENTRANCE,
};

std::map<RandomizerCheckArea, std::vector<RandomizerCheck>> checksByArea;
bool areasFullyChecked[RCAREA_INVALID];
u32 areasSpoiled = 0;
bool showVOrMQ;
s16 areaChecksGotten[RCAREA_INVALID]; //|     "Kokiri Forest (4/9)"
s16 areaChecksAvailable[RCAREA_INVALID];
s16 areaCheckTotals[RCAREA_INVALID];
uint16_t totalChecks = 0;
uint16_t totalChecksAvailable = 0;
uint16_t totalChecksGotten = 0;
bool optCollapseAll; // A bool that will collapse all checks once
bool optExpandAll;   // A bool that will expand all checks once
RandomizerCheck lastLocationChecked = RC_UNKNOWN_CHECK;
RandomizerCheckArea previousArea = RCAREA_INVALID;
RandomizerCheckArea currentArea = RCAREA_INVALID;
OSContPad* trackerButtonsPressed;
std::unordered_map<RandomizerCheck, std::string> checkNameOverrides;

bool ShouldShowCheck(RandomizerCheck rc);
bool UpdateFilters();
void BeginFloatWindows(std::string UniqueName, bool& open, ImGuiWindowFlags flags = 0);
bool CompareChecks(RandomizerCheck, RandomizerCheck);
bool CheckByArea(RandomizerCheckArea);
void DrawLocation(RandomizerCheck);
void EndFloatWindows();
void LoadSettings();
void RainbowTick();
void UpdateAreas(RandomizerCheckArea area);
void UpdateInventoryChecks();
void UpdateOrdering(RandomizerCheckArea);
int sectionId;

bool hideUnchecked = false;
bool hideScummed = false;
bool hideSeen = false;
bool hideSkipped = false;
bool hideSaved = false;
bool hideCollected = false;
bool showHidden = true;
bool mystery = false;
bool showLogicTooltip = false;
bool enableAvailableChecks = false;
bool onlyShowAvailable = false;

SceneID DungeonSceneLookupByArea(RandomizerCheckArea area) {
    switch (area) {
        case RCAREA_DEKU_TREE:
            return SCENE_DEKU_TREE;
        case RCAREA_DODONGOS_CAVERN:
            return SCENE_DODONGOS_CAVERN;
        case RCAREA_JABU_JABUS_BELLY:
            return SCENE_JABU_JABU;
        case RCAREA_FOREST_TEMPLE:
            return SCENE_FOREST_TEMPLE;
        case RCAREA_FIRE_TEMPLE:
            return SCENE_FIRE_TEMPLE;
        case RCAREA_WATER_TEMPLE:
            return SCENE_WATER_TEMPLE;
        case RCAREA_SPIRIT_TEMPLE:
            return SCENE_SPIRIT_TEMPLE;
        case RCAREA_SHADOW_TEMPLE:
            return SCENE_SHADOW_TEMPLE;
        case RCAREA_BOTTOM_OF_THE_WELL:
            return SCENE_BOTTOM_OF_THE_WELL;
        case RCAREA_ICE_CAVERN:
            return SCENE_ICE_CAVERN;
        case RCAREA_GERUDO_TRAINING_GROUND:
            return SCENE_GERUDO_TRAINING_GROUND;
        case RCAREA_GANONS_CASTLE:
            return SCENE_INSIDE_GANONS_CASTLE;
        default:
            return SCENE_ID_MAX;
    }
}

Color_RGBA8 Color_Bg_Default = { 0, 0, 0, 255 };                          // Black
Color_RGBA8 Color_Main_Default = { 255, 255, 255, 255 };                  // White
Color_RGBA8 Color_Area_Incomplete_Extra_Default = { 255, 255, 255, 255 }; // White
Color_RGBA8 Color_Area_Complete_Extra_Default = { 255, 255, 255, 255 };   // White
Color_RGBA8 Color_Unchecked_Extra_Default = { 255, 255, 255, 255 };       // White
Color_RGBA8 Color_Skipped_Main_Default = { 160, 160, 160, 255 };          // Grey
Color_RGBA8 Color_Skipped_Extra_Default = { 160, 160, 160, 255 };         // Grey
Color_RGBA8 Color_Seen_Extra_Default = { 255, 255, 255, 255 };            // TODO
Color_RGBA8 Color_Hinted_Extra_Default = { 255, 255, 255, 255 };          // TODO
Color_RGBA8 Color_Collected_Extra_Default = { 242, 101, 34, 255 };        // Orange
Color_RGBA8 Color_Scummed_Extra_Default = { 0, 174, 239, 255 };           // Blue
Color_RGBA8 Color_Saved_Extra_Default = { 0, 185, 0, 255 };               // Green

Color_RGBA8 Color_Background = { 0, 0, 0, 255 };

Color_RGBA8 Color_Area_Incomplete_Main = { 255, 255, 255, 255 };  // White
Color_RGBA8 Color_Area_Incomplete_Extra = { 255, 255, 255, 255 }; // White
Color_RGBA8 Color_Area_Complete_Main = { 255, 255, 255, 255 };    // White
Color_RGBA8 Color_Area_Complete_Extra = { 255, 255, 255, 255 };   // White
Color_RGBA8 Color_Unchecked_Main = { 255, 255, 255, 255 };        // White
Color_RGBA8 Color_Unchecked_Extra = { 255, 255, 255, 255 };       // Useless
Color_RGBA8 Color_Skipped_Main = { 160, 160, 160, 255 };          // Grey
Color_RGBA8 Color_Skipped_Extra = { 160, 160, 160, 255 };         // Grey
Color_RGBA8 Color_Seen_Main = { 255, 255, 255, 255 };             // TODO
Color_RGBA8 Color_Seen_Extra = { 160, 160, 160, 255 };            // TODO
Color_RGBA8 Color_Hinted_Main = { 255, 255, 255, 255 };           // TODO
Color_RGBA8 Color_Hinted_Extra = { 255, 255, 255, 255 };          // TODO
Color_RGBA8 Color_Collected_Main = { 255, 255, 255, 255 };        // White
Color_RGBA8 Color_Collected_Extra = { 242, 101, 34, 255 };        // Orange
Color_RGBA8 Color_Scummed_Main = { 255, 255, 255, 255 };          // White
Color_RGBA8 Color_Scummed_Extra = { 0, 174, 239, 255 };           // Blue
Color_RGBA8 Color_Saved_Main = { 255, 255, 255, 255 };            // White
Color_RGBA8 Color_Saved_Extra = { 0, 185, 0, 255 };               // Green

std::vector<uint32_t> buttons = { BTN_A, BTN_B, BTN_CUP,   BTN_CDOWN, BTN_CLEFT, BTN_CRIGHT, BTN_L,
                                  BTN_Z, BTN_R, BTN_START, BTN_DUP,   BTN_DDOWN, BTN_DLEFT,  BTN_DRIGHT };
static ImGuiTextFilter checkSearch;
static bool recalculateAvailable = false;
std::array<bool, RCAREA_INVALID> filterAreasHidden = { 0 };
std::array<bool, RC_MAX> filterChecksHidden = { 0 };

void TrySetAreas() {
    if (checksByArea.empty()) {
        for (int i = RCAREA_KOKIRI_FOREST; i < RCAREA_INVALID; i++) {
            checksByArea.emplace(static_cast<RandomizerCheckArea>(i), std::vector<RandomizerCheck>());
        }
    }
}

void CalculateTotals() {
    totalChecks = 0;
    totalChecksAvailable = 0;
    totalChecksGotten = 0;

    for (uint8_t i = 0; i < RCAREA_INVALID; i++) {
        totalChecks += areaCheckTotals[i];
        totalChecksAvailable += areaChecksAvailable[i];
        totalChecksGotten += areaChecksGotten[i];
    }
}

uint16_t GetTotalChecks() {
    return totalChecks;
}

uint16_t GetTotalChecksGotten() {
    return totalChecksGotten;
}

bool IsCheckHidden(RandomizerCheck rc) {
    Rando::ItemLocation* itemLocation = OTRGlobals::Instance->gRandoContext->GetItemLocation(rc);
    RandomizerCheckStatus status = itemLocation->GetCheckStatus();
    bool available = itemLocation->IsAvailable();
    bool skipped = itemLocation->GetIsSkipped();
    bool obtained = itemLocation->HasObtained();
    bool seen = status == RCSHOW_SEEN || status == RCSHOW_IDENTIFIED;
    bool scummed = status == RCSHOW_SCUMMED;
    bool unchecked = status == RCSHOW_UNCHECKED;

    return !showHidden &&
           ((skipped && hideSkipped) || (seen && hideSeen) || (scummed && hideScummed) || (unchecked && hideUnchecked));
}

void RecalculateAreaTotals(RandomizerCheckArea rcArea) {
    areaChecksGotten[rcArea] = 0;
    areaChecksAvailable[rcArea] = 0;
    areaCheckTotals[rcArea] = 0;
    for (auto rc : checksByArea.at(rcArea)) {
        if (!IsVisibleInCheckTracker(rc)) {
            continue;
        }
        areaCheckTotals[rcArea]++;

        Rando::ItemLocation* itemLoc = OTRGlobals::Instance->gRandoContext->GetItemLocation(rc);

        if (itemLoc->GetIsSkipped() || itemLoc->HasObtained()) {
            areaChecksGotten[rcArea]++;
        }

        if (itemLoc->IsAvailable() && !IsCheckHidden(rc)) {
            areaChecksAvailable[rcArea]++;
        }
    }
    CalculateTotals();
}

std::map<RandomizerGet, RandomizerCheckArea> MapRGtoRandomizerCheckArea = {
    { RG_DEKU_TREE_MAP, RCAREA_DEKU_TREE },
    { RG_DODONGOS_CAVERN_MAP, RCAREA_DODONGOS_CAVERN },
    { RG_JABU_JABUS_BELLY_MAP, RCAREA_JABU_JABUS_BELLY },
    { RG_FOREST_TEMPLE_MAP, RCAREA_FOREST_TEMPLE },
    { RG_FIRE_TEMPLE_MAP, RCAREA_FIRE_TEMPLE },
    { RG_WATER_TEMPLE_MAP, RCAREA_WATER_TEMPLE },
    { RG_SPIRIT_TEMPLE_MAP, RCAREA_SPIRIT_TEMPLE },
    { RG_SHADOW_TEMPLE_MAP, RCAREA_SHADOW_TEMPLE },
    { RG_BOTTOM_OF_THE_WELL_MAP, RCAREA_BOTTOM_OF_THE_WELL },
    { RG_ICE_CAVERN_MAP, RCAREA_ICE_CAVERN }
};

void SpoilAreaFromCheck(RandomizerCheck rc) {
    Rando::Location* loc = Rando::StaticData::GetLocation(rc);
    Rando::ItemLocation* itemLoc = Rando::Context::GetInstance()->GetItemLocation(rc);
    if (itemLoc->GetPlacedItem().GetItemType() == ItemType::ITEMTYPE_MAP) {
        RandomizerCheckArea area = MapRGtoRandomizerCheckArea[itemLoc->GetPlacedRandomizerGet()];
        if (!IsAreaSpoiled(area)) {
            SetAreaSpoiled(area);
        }
    }
    if (!IsAreaSpoiled(loc->GetArea())) {
        SetAreaSpoiled(loc->GetArea());
    }
}

void RecalculateAllAreaTotals() {
    for (auto& [rcArea, checks] : checksByArea) {
        if (rcArea == RCAREA_INVALID) {
            return;
        }
        RecalculateAreaTotals(rcArea);
    }
}

void SetCheckCollected(RandomizerCheck rc) {
    OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->SetCheckStatus(RCSHOW_COLLECTED);
    Rando::Location* loc = Rando::StaticData::GetLocation(rc);
    if (IsVisibleInCheckTracker(rc)) {
        if (!OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->GetIsSkipped()) {
            areaChecksGotten[loc->GetArea()]++;
            areaChecksAvailable[loc->GetArea()]--;
        } else {
            OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->SetIsSkipped(false);
        }
    }
    SaveManager::Instance->SaveSection(gSaveContext.fileNum, sectionId, true);

    if (!IsAreaSpoiled(loc->GetArea())) {
        SetAreaSpoiled(loc->GetArea());
    }

    doAreaScroll = true;
    UpdateOrdering(loc->GetArea());
    UpdateInventoryChecks();
}

bool IsAreaScene(SceneID sceneNum) {
    switch (sceneNum) {
        case SCENE_HYRULE_FIELD:
        case SCENE_KAKARIKO_VILLAGE:
        case SCENE_GRAVEYARD:
        case SCENE_ZORAS_RIVER:
        case SCENE_KOKIRI_FOREST:
        case SCENE_SACRED_FOREST_MEADOW:
        case SCENE_LAKE_HYLIA:
        case SCENE_ZORAS_DOMAIN:
        case SCENE_ZORAS_FOUNTAIN:
        case SCENE_GERUDO_VALLEY:
        case SCENE_LOST_WOODS:
        case SCENE_DESERT_COLOSSUS:
        case SCENE_GERUDOS_FORTRESS:
        case SCENE_HAUNTED_WASTELAND:
        case SCENE_HYRULE_CASTLE:
        case SCENE_DEATH_MOUNTAIN_TRAIL:
        case SCENE_DEATH_MOUNTAIN_CRATER:
        case SCENE_GORON_CITY:
        case SCENE_LON_LON_RANCH:
        case SCENE_DEKU_TREE:
        case SCENE_DODONGOS_CAVERN:
        case SCENE_JABU_JABU:
        case SCENE_FOREST_TEMPLE:
        case SCENE_FIRE_TEMPLE:
        case SCENE_WATER_TEMPLE:
        case SCENE_SPIRIT_TEMPLE:
        case SCENE_SHADOW_TEMPLE:
        case SCENE_BOTTOM_OF_THE_WELL:
        case SCENE_ICE_CAVERN:
        case SCENE_GERUDO_TRAINING_GROUND:
        case SCENE_GANONS_TOWER:
        case SCENE_INSIDE_GANONS_CASTLE:
        case SCENE_BACK_ALLEY_DAY:
        case SCENE_BACK_ALLEY_NIGHT:
        case SCENE_MARKET_DAY:
        case SCENE_MARKET_NIGHT:
        case SCENE_MARKET_RUINS:
            return true;
        default:
            return false;
    }
}

RandomizerCheckArea AreaFromEntranceGroup[] = {
    RCAREA_INVALID,          RCAREA_KOKIRI_FOREST, RCAREA_LOST_WOODS,           RCAREA_SACRED_FOREST_MEADOW,
    RCAREA_KAKARIKO_VILLAGE, RCAREA_GRAVEYARD,     RCAREA_DEATH_MOUNTAIN_TRAIL, RCAREA_DEATH_MOUNTAIN_CRATER,
    RCAREA_GORON_CITY,       RCAREA_ZORAS_RIVER,   RCAREA_ZORAS_DOMAIN,         RCAREA_ZORAS_FOUNTAIN,
    RCAREA_HYRULE_FIELD,     RCAREA_LON_LON_RANCH, RCAREA_LAKE_HYLIA,           RCAREA_GERUDO_VALLEY,
    RCAREA_GERUDO_FORTRESS,  RCAREA_WASTELAND,     RCAREA_DESERT_COLOSSUS,      RCAREA_MARKET,
    RCAREA_HYRULE_CASTLE,
};

RandomizerCheckArea GetCheckArea() {
    auto scene = static_cast<SceneID>(gPlayState->sceneNum);
    bool grottoScene = (scene == SCENE_GROTTOS || scene == SCENE_FAIRYS_FOUNTAIN);
    const EntranceData* ent =
        GetEntranceData(grottoScene ? ENTRANCE_GROTTO_EXIT_START + GetCurrentGrottoId() : gSaveContext.entranceIndex);
    RandomizerCheckArea area = RCAREA_INVALID;
    if (ent != nullptr && !IsAreaScene(scene) && ent->type != ENTRANCE_TYPE_DUNGEON) {
        if (ent->source == "Desert Colossus" || ent->destination == "Desert Colossus") {
            area = RCAREA_DESERT_COLOSSUS;
        } else {
            area = AreaFromEntranceGroup[ent->dstGroup];
        }
    }
    if (area == RCAREA_INVALID) {
        if (grottoScene && (GetCurrentGrottoId() == -1) &&
            (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_GROTTO_ENTRANCES) == RO_GENERIC_OFF)) {
            area = previousArea;
        } else {
            area = RandomizerCheckObjects::GetRCAreaBySceneID(scene);
        }
    }
    return area;
}

bool vector_contains_scene(std::vector<SceneID> vec, const int16_t scene) {
    return std::any_of(vec.begin(), vec.end(), [&](const auto& x) { return x == scene; });
}

std::vector<SceneID> skipScenes = {
    SCENE_GANON_BOSS,
    SCENE_GANONS_TOWER_COLLAPSE_EXTERIOR,
    SCENE_GANON_BOSS,
    SCENE_INSIDE_GANONS_CASTLE_COLLAPSE,
    SCENE_GANONS_TOWER_COLLAPSE_INTERIOR,
};

void ClearAreaChecksAndTotals() {
    for (auto& [rcArea, vec] : checksByArea) {
        vec.clear();
        areaChecksGotten[rcArea] = 0;
        areaChecksAvailable[rcArea] = 0;
        areaCheckTotals[rcArea] = 0;
    }
    totalChecks = 0;
    totalChecksGotten = 0;
    totalChecksAvailable = 0;
}

void SetShopSeen(uint32_t sceneNum, bool prices) {
    RandomizerCheck start = startingShopItem.find(sceneNum)->second;
    if (sceneNum == SCENE_POTION_SHOP_KAKARIKO && !LINK_IS_ADULT) {
        return;
    }
    if (GetCheckArea() == RCAREA_KAKARIKO_VILLAGE && sceneNum == SCENE_BAZAAR) {
        start = RC_KAK_BAZAAR_ITEM_1;
    }
    bool statusChanged = false;
    for (int i = start; i < start + 8; i++) {
        if (OTRGlobals::Instance->gRandoContext->GetItemLocation(i)->GetCheckStatus() == RCSHOW_UNCHECKED) {
            OTRGlobals::Instance->gRandoContext->GetItemLocation(i)->SetCheckStatus(RCSHOW_SEEN);
            statusChanged = true;
        }
    }
    if (statusChanged) {
        SaveManager::Instance->SaveSection(gSaveContext.fileNum, sectionId, true);
    }
}

void CheckTrackerLoadGame(int32_t fileNum) {
    LoadSettings();
    TrySetAreas();
    for (auto& entry : Rando::StaticData::GetLocationTable()) {
        RandomizerCheck rc = entry.GetRandomizerCheck();
        if (rc == RC_UNKNOWN_CHECK || rc == RC_MAX || rc == RC_LINKS_POCKET ||
            !Rando::StaticData::GetLocation(rc) != RC_UNKNOWN_CHECK) {
            continue;
        }

        Rando::Location* entry2 = Rando::StaticData::GetLocation(rc);
        Rando::ItemLocation* loc = OTRGlobals::Instance->gRandoContext->GetItemLocation(rc);

        checksByArea.find(entry2->GetArea())->second.push_back(entry2->GetRandomizerCheck());
        if (IsVisibleInCheckTracker(entry2->GetRandomizerCheck())) {
            areaCheckTotals[entry2->GetArea()]++;
            if (loc->GetCheckStatus() == RCSHOW_SAVED || loc->GetIsSkipped()) {
                areaChecksGotten[entry2->GetArea()]++;
            }
            if (loc->IsAvailable()) {
                areaChecksAvailable[entry2->GetArea()]++;
            }
        }

        if (areaChecksGotten[entry2->GetArea()] != 0 || RandomizerCheckObjects::AreaIsOverworld(entry2->GetArea()) ||
            loc->GetCheckStatus() == RCSHOW_SCUMMED) {
            areasSpoiled |= (1 << entry2->GetArea());
        }

        // Create check name overrides for child pond fish if age split is disabled
        if (fishsanityMode != RO_FISHSANITY_OFF && fishsanityMode != RO_FISHSANITY_OVERWORLD &&
            entry.GetRCType() == RCTYPE_FISH && entry.GetScene() == SCENE_FISHING_POND &&
            entry.GetActorParams() != 116 && !fishsanityAgeSplit) {
            if (entry.GetShortName().starts_with("Child")) {
                checkNameOverrides[rc] = entry.GetShortName().substr(6);
            }
        }
    }
    for (int i = RCAREA_KOKIRI_FOREST; i < RCAREA_INVALID; i++) {
        if (!IsAreaSpoiled(static_cast<RandomizerCheckArea>(i)) &&
            (RandomizerCheckObjects::AreaIsOverworld(static_cast<RandomizerCheckArea>(i)) || !IS_RANDO ||
             OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_RANDOM) == RO_MQ_DUNGEONS_NONE ||
             (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_RANDOM) ==
                  RO_MQ_DUNGEONS_SELECTION &&
              OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(
                  static_cast<RandomizerSettingKey>(RSK_MQ_DEKU_TREE + (i - RCAREA_DEKU_TREE))) != RO_MQ_SET_RANDOM) ||
             (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_SET) == RO_GENERIC_ON &&
              OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(
                  static_cast<RandomizerSettingKey>(RSK_MQ_DEKU_TREE + (i - RCAREA_DEKU_TREE))) != RO_MQ_SET_RANDOM) ||
             (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_RANDOM) ==
                  RO_MQ_DUNGEONS_SET_NUMBER &&
              (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_COUNT) == 12 ||
               OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_COUNT) == 0)))) {
            SetAreaSpoiled(static_cast<RandomizerCheckArea>(i));
        }
    }
    if (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_LINKS_POCKET) != RO_LINKS_POCKET_NOTHING &&
        IS_RANDO) {
        uint8_t startingAge = OTRGlobals::Instance->gRandoContext->GetOption(RSK_SELECTED_STARTING_AGE).Get();
        RandomizerCheckArea startingArea;
        switch (startingAge) {
            case RO_AGE_CHILD:
                startingArea = RCAREA_KOKIRI_FOREST;
                break;
            case RO_AGE_ADULT:
                startingArea = RCAREA_MARKET;
                break;
            default:
                startingArea = RCAREA_KOKIRI_FOREST;
                break;
        }

        checksByArea.find(startingArea)->second.push_back(RC_LINKS_POCKET);
        areaChecksGotten[startingArea]++;
        areaCheckTotals[startingArea]++;
    }

    showVOrMQ =
        (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_RANDOM) ==
             RO_MQ_DUNGEONS_RANDOM_NUMBER ||
         (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_RANDOM) == RO_MQ_DUNGEONS_SET_NUMBER &&
          OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_MQ_DUNGEON_COUNT) < 12));
    initialized = true;
    UpdateAllOrdering();
    UpdateInventoryChecks();
    UpdateFilters();

    RegionTable_Init();

    if (Rando::Context::GetInstance()->GetOption(RSK_SHUFFLE_ENTRANCES).Get()) {
        Rando::Context::GetInstance()->GetEntranceShuffler()->ApplyEntranceOverrides();
    }

    RecalculateAvailableChecks();
}

void CheckTrackerShopSlotChange(uint8_t cursorSlot, int16_t basePrice) {
    if (gPlayState->sceneNum == SCENE_HAPPY_MASK_SHOP) { // Happy Mask Shop is not used in rando, so is not tracked
        return;
    }

    auto slot = startingShopItem.find(gPlayState->sceneNum)->second + cursorSlot;
    if (GetCheckArea() == RCAREA_KAKARIKO_VILLAGE && gPlayState->sceneNum == SCENE_BAZAAR) {
        slot = RC_KAK_BAZAAR_ITEM_1 + cursorSlot;
    }
    auto status = OTRGlobals::Instance->gRandoContext->GetItemLocation(slot)->GetCheckStatus();
    if (status == RCSHOW_SEEN) {
        OTRGlobals::Instance->gRandoContext->GetItemLocation(slot)->SetCheckStatus(RCSHOW_IDENTIFIED);
        SaveManager::Instance->SaveSection(gSaveContext.fileNum, sectionId, true);
        RecalculateAvailableChecks();
    }
}

void CheckTrackerTransition(uint32_t sceneNum) {
    if (!GameInteractor::IsSaveLoaded()) {
        return;
    }
    doAreaScroll = true;
    previousArea = currentArea;
    currentArea = GetCheckArea();
    switch (sceneNum) {
        case SCENE_KOKIRI_SHOP:
        case SCENE_BAZAAR:
        case SCENE_POTION_SHOP_MARKET:
        case SCENE_BOMBCHU_SHOP:
        case SCENE_POTION_SHOP_KAKARIKO:
        case SCENE_GORON_SHOP:
        case SCENE_ZORA_SHOP:
            SetShopSeen(sceneNum, false);
            break;
    }
    if (!IsAreaSpoiled(currentArea) && (RandomizerCheckObjects::AreaIsOverworld(currentArea) ||
                                        std::find(spoilingEntrances.begin(), spoilingEntrances.end(),
                                                  gPlayState->nextEntranceIndex) != spoilingEntrances.end())) {
        SetAreaSpoiled(currentArea);
    }
}

void CheckTrackerItemReceive(GetItemEntry giEntry) {
    if (!GameInteractor::IsSaveLoaded() || vector_contains_scene(skipScenes, gPlayState->sceneNum)) {
        return;
    }
    auto scene = static_cast<SceneID>(gPlayState->sceneNum);
    // Vanilla special item checks
    if (!IS_RANDO) {
        if (giEntry.itemId == ITEM_SHIELD_DEKU) {
            SetCheckCollected(RC_KF_SHOP_ITEM_1);
            return;
        } else if (giEntry.itemId == ITEM_KOKIRI_EMERALD) {
            SetCheckCollected(RC_QUEEN_GOHMA);
            return;
        } else if (giEntry.itemId == ITEM_GORON_RUBY) {
            SetCheckCollected(RC_KING_DODONGO);
            return;
        } else if (giEntry.itemId == ITEM_ZORA_SAPPHIRE) {
            SetCheckCollected(RC_BARINADE);
            return;
        } else if (giEntry.itemId == ITEM_MEDALLION_FOREST) {
            SetCheckCollected(RC_PHANTOM_GANON);
            return;
        } else if (giEntry.itemId == ITEM_MEDALLION_FIRE) {
            SetCheckCollected(RC_VOLVAGIA);
            return;
        } else if (giEntry.itemId == ITEM_MEDALLION_WATER) {
            SetCheckCollected(RC_MORPHA);
            return;
        } else if (giEntry.itemId == ITEM_MEDALLION_SHADOW) {
            SetCheckCollected(RC_BONGO_BONGO);
            return;
        } else if (giEntry.itemId == ITEM_MEDALLION_SPIRIT) {
            SetCheckCollected(RC_TWINROVA);
            return;
        } else if (giEntry.itemId == ITEM_MEDALLION_LIGHT) {
            SetCheckCollected(RC_GIFT_FROM_RAURU);
            return;
        } else if (giEntry.itemId == ITEM_SONG_EPONA) {
            SetCheckCollected(RC_SONG_FROM_MALON);
            return;
        } else if (giEntry.itemId == ITEM_SONG_SARIA) {
            SetCheckCollected(RC_SONG_FROM_SARIA);
            return;
        } else if (giEntry.itemId == ITEM_BEAN) {
            SetCheckCollected(RC_ZR_MAGIC_BEAN_SALESMAN);
            return;
        } else if (giEntry.itemId == ITEM_BRACELET) {
            SetCheckCollected(RC_GC_DARUNIAS_JOY);
            return;
        } /* else if (giEntry.itemId == ITEM_SONG_SUN) {
             SetCheckCollected(RC_SONG_FROM_ROYAL_FAMILYS_TOMB);
             return;
         } else if (giEntry.itemId == ITEM_SONG_TIME) {
             SetCheckCollected(RC_SONG_FROM_OCARINA_OF_TIME);
             return;
         } else if (giEntry.itemId == ITEM_SONG_STORMS) {
             SetCheckCollected(RC_SONG_FROM_WINDMILL);
             return;
         } else if (giEntry.itemId == ITEM_SONG_MINUET) {
             SetCheckCollected(RC_SHEIK_IN_FOREST);
             return;
         } else if (giEntry.itemId == ITEM_SONG_BOLERO) {
             SetCheckCollected(RC_SHEIK_IN_CRATER);
             return;
         } else if (giEntry.itemId == ITEM_SONG_SERENADE) {
             SetCheckCollected(RC_SHEIK_IN_ICE_CAVERN);
             return;
         } else if (giEntry.itemId == ITEM_SONG_NOCTURNE) {
             SetCheckCollected(RC_SHEIK_IN_KAKARIKO);
             return;
         } else if (giEntry.itemId == ITEM_SONG_REQUIEM) {
             SetCheckCollected(RC_SHEIK_AT_COLOSSUS);
             return;
         } else if (giEntry.itemId == ITEM_SONG_PRELUDE) {
             SetCheckCollected(RC_SHEIK_AT_TEMPLE);
             return;
         }*/
    }
}

void CheckTrackerSceneFlagSet(int16_t sceneNum, int16_t flagType, int32_t flag) {
    if (IS_RANDO) {
        return;
    }

    if (flagType != FLAG_SCENE_TREASURE && flagType != FLAG_SCENE_COLLECTIBLE) {
        return;
    }
    if (sceneNum == SCENE_GRAVEYARD && flag == 0x19 &&
        flagType == FLAG_SCENE_COLLECTIBLE) { // Gravedigging tour special case
        SetCheckCollected(RC_GRAVEYARD_DAMPE_GRAVEDIGGING_TOUR);
        return;
    }
    for (auto& loc : Rando::StaticData::GetLocationTable()) {
        if (!IsVisibleInCheckTracker(loc.GetRandomizerCheck())) {
            continue;
        }
        SpoilerCollectionCheckType checkMatchType = flagType == FLAG_SCENE_TREASURE
                                                        ? SpoilerCollectionCheckType::SPOILER_CHK_CHEST
                                                        : SpoilerCollectionCheckType::SPOILER_CHK_COLLECTABLE;
        Rando::SpoilerCollectionCheck scCheck = loc.GetCollectionCheck();
        if (scCheck.scene == sceneNum && scCheck.flag == flag && scCheck.type == checkMatchType) {
            SetCheckCollected(loc.GetRandomizerCheck());
            return;
        }
    }
}

void CheckTrackerFlagSet(int16_t flagType, int32_t flag) {
    if (IS_RANDO) {
        return;
    }

    SpoilerCollectionCheckType checkMatchType = SpoilerCollectionCheckType::SPOILER_CHK_NONE;
    switch (flagType) {
        case FLAG_GS_TOKEN:
            checkMatchType = SpoilerCollectionCheckType::SPOILER_CHK_GOLD_SKULLTULA;
            break;
        case FLAG_EVENT_CHECK_INF:
            if ((flag == EVENTCHKINF_CARPENTERS_FREE(0) || flag == EVENTCHKINF_CARPENTERS_FREE(1) ||
                 flag == EVENTCHKINF_CARPENTERS_FREE(2) || flag == EVENTCHKINF_CARPENTERS_FREE(3)) &&
                GET_EVENTCHKINF_CARPENTERS_FREE_ALL()) {
                SetCheckCollected(RC_GF_GERUDO_MEMBERSHIP_CARD);
                return;
            }
            checkMatchType = SpoilerCollectionCheckType::SPOILER_CHK_EVENT_CHK_INF;
            break;
        case FLAG_INF_TABLE:
            if (flag == INFTABLE_190) {
                SetCheckCollected(RC_GF_HBA_1000_POINTS);
                return;
            } else if (flag == INFTABLE_11E) {
                SetCheckCollected(RC_GC_ROLLING_GORON_AS_CHILD);
                return;
            } else if (flag == INFTABLE_GORON_CITY_DOORS_UNLOCKED) {
                SetCheckCollected(RC_GC_ROLLING_GORON_AS_ADULT);
                return;
            } else if (flag == INFTABLE_139) {
                SetCheckCollected(RC_ZD_KING_ZORA_THAWED);
                return;
            } else if (flag == INFTABLE_191) {
                SetCheckCollected(RC_MARKET_LOST_DOG);
                return;
            }
            if (!IS_RANDO) {
                if (flag == INFTABLE_BOUGHT_STICK_UPGRADE) {
                    SetCheckCollected(RC_LW_DEKU_SCRUB_NEAR_BRIDGE);
                    return;
                } else if (flag == INFTABLE_BOUGHT_NUT_UPGRADE) {
                    SetCheckCollected(RC_LW_DEKU_SCRUB_GROTTO_FRONT);
                    return;
                }
            }
            break;
        case FLAG_ITEM_GET_INF:
            if (!IS_RANDO) {
                if (flag == ITEMGETINF_OBTAINED_STICK_UPGRADE_FROM_STAGE) {
                    SetCheckCollected(RC_DEKU_THEATER_SKULL_MASK);
                    return;
                } else if (flag == ITEMGETINF_OBTAINED_NUT_UPGRADE_FROM_STAGE) {
                    SetCheckCollected(RC_DEKU_THEATER_MASK_OF_TRUTH);
                    return;
                } else if (flag == ITEMGETINF_DEKU_SCRUB_HEART_PIECE) {
                    SetCheckCollected(RC_HF_DEKU_SCRUB_GROTTO);
                    return;
                }
            }
            checkMatchType = SpoilerCollectionCheckType::SPOILER_CHK_ITEM_GET_INF;
            break;
        case FLAG_RANDOMIZER_INF:
            checkMatchType = SpoilerCollectionCheckType::SPOILER_CHK_RANDOMIZER_INF;
            break;
    }
    if (checkMatchType == SpoilerCollectionCheckType::SPOILER_CHK_NONE) {
        return;
    }
    for (auto& loc : Rando::StaticData::GetLocationTable()) {
        if ((!IS_RANDO && ((loc.GetQuest() == RCQUEST_MQ && !IS_MASTER_QUEST) ||
                           (loc.GetQuest() == RCQUEST_VANILLA && IS_MASTER_QUEST))) ||
            (IS_RANDO &&
             !(OTRGlobals::Instance->gRandoContext->GetDungeons()->GetDungeonFromScene(loc.GetScene()) == nullptr) &&
             ((OTRGlobals::Instance->gRandoContext->GetDungeons()->GetDungeonFromScene(loc.GetScene())->IsMQ() &&
               loc.GetQuest() == RCQUEST_VANILLA) ||
              OTRGlobals::Instance->gRandoContext->GetDungeons()->GetDungeonFromScene(loc.GetScene())->IsVanilla() &&
                  loc.GetQuest() == RCQUEST_MQ))) {
            continue;
        }
        Rando::SpoilerCollectionCheck scCheck = loc.GetCollectionCheck();
        SpoilerCollectionCheckType scCheckType = scCheck.type;
        if (checkMatchType == SpoilerCollectionCheckType::SPOILER_CHK_RANDOMIZER_INF &&
            scCheckType == SpoilerCollectionCheckType::SPOILER_CHK_RANDOMIZER_INF) {
            if (flag == OTRGlobals::Instance->gRandomizer->GetRandomizerInfFromCheck(loc.GetRandomizerCheck())) {
                SetCheckCollected(loc.GetRandomizerCheck());
                return;
            }
            continue;
        }
        int16_t checkFlag = scCheck.flag;
        if (checkMatchType == SpoilerCollectionCheckType::SPOILER_CHK_GOLD_SKULLTULA) {
            checkFlag = loc.GetActorParams();
        }
        if (checkFlag == flag && scCheck.type == checkMatchType) {
            SetCheckCollected(loc.GetRandomizerCheck());
            return;
        }
    }
}

void InitTrackerData(bool isDebug) {
    TrySetAreas();
    areasSpoiled = 0;
}

void SaveTrackerData(SaveContext* saveContext, int sectionID, bool fullSave) {
    bool updateOrdering = false;
    std::vector<RandomizerCheck> checkCount;
    for (int i = RC_UNKNOWN_CHECK; i < RC_MAX; i++) {
        if (OTRGlobals::Instance->gRandoContext->GetItemLocation(i)->GetCheckStatus() != RCSHOW_UNCHECKED ||
            OTRGlobals::Instance->gRandoContext->GetItemLocation(i)->GetIsSkipped())
            checkCount.push_back(static_cast<RandomizerCheck>(i));
    }
    SaveManager::Instance->SaveArray("checkStatus", checkCount.size(), [&](size_t i) {
        RandomizerCheck check = checkCount.at(i);
        RandomizerCheckStatus savedStatus =
            OTRGlobals::Instance->gRandoContext->GetItemLocation(check)->GetCheckStatus();
        bool isSkipped = OTRGlobals::Instance->gRandoContext->GetItemLocation(check)->GetIsSkipped();
        if (savedStatus == RCSHOW_COLLECTED) {
            if (fullSave) {
                OTRGlobals::Instance->gRandoContext->GetItemLocation(check)->SetCheckStatus(RCSHOW_SAVED);
                savedStatus = RCSHOW_SAVED;
                updateOrdering = true;
            } else {
                savedStatus = RCSHOW_SCUMMED;
            }
        }
        if (savedStatus != RCSHOW_UNCHECKED || isSkipped) {
            SaveManager::Instance->SaveStruct("", [&]() {
                SaveManager::Instance->SaveData("randomizerCheck", check);
                SaveManager::Instance->SaveData("status", savedStatus);
                SaveManager::Instance->SaveData("skipped", isSkipped);
            });
        }
    });
    SaveManager::Instance->SaveData("areasSpoiled", areasSpoiled);
    if (updateOrdering) {
        UpdateAllOrdering();
        UpdateAllAreas();
    }
}

void SaveFile(SaveContext* saveContext, int sectionID, bool fullSave) {
    SaveTrackerData(saveContext, sectionID, fullSave);
    if (fullSave) {
        recalculateAvailable = true;
    }
}

void LoadFile() {
    SaveManager::Instance->LoadArray("checkStatus", RC_MAX, [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&]() {
            RandomizerCheckStatus status;
            bool skipped;
            RandomizerCheck rc;
            SaveManager::Instance->LoadData("randomizerCheck", rc, RC_UNKNOWN_CHECK);
            SaveManager::Instance->LoadData("status", status, RCSHOW_UNCHECKED);
            SaveManager::Instance->LoadData("skipped", skipped, false);
            OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->SetCheckStatus(status);
            OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->SetIsSkipped(skipped);
        });
    });
    SaveManager::Instance->LoadData("areasSpoiled", areasSpoiled, (uint32_t)0);
    UpdateAllOrdering();
    UpdateAllAreas();
}

void Teardown() {
    initialized = false;
    ClearAreaChecksAndTotals();
    checksByArea.clear();
    areasSpoiled = 0;
    filterAreasHidden = { 0 };
    filterChecksHidden = { 0 };

    lastLocationChecked = RC_UNKNOWN_CHECK;
}

bool IsAreaSpoiled(RandomizerCheckArea rcArea) {
    return areasSpoiled & (1 << rcArea);
}

void SetAreaSpoiled(RandomizerCheckArea rcArea) {
    areasSpoiled |= (1 << rcArea);
    SaveManager::Instance->SaveSection(gSaveContext.fileNum, sectionId, true);
}

void CheckTrackerWindow::DrawElement() {
    Color_Background = CVarGetColor(CVAR_TRACKER_CHECK("BgColor.Value"), Color_Bg_Default);
    Color_Area_Incomplete_Main = CVarGetColor(CVAR_TRACKER_CHECK("AreaIncomplete.MainColor.Value"), Color_Main_Default);
    Color_Area_Incomplete_Extra =
        CVarGetColor(CVAR_TRACKER_CHECK("AreaIncomplete.ExtraColor.Value"), Color_Area_Incomplete_Extra_Default);
    Color_Area_Complete_Main = CVarGetColor(CVAR_TRACKER_CHECK("AreaComplete.MainColor.Value"), Color_Main_Default);
    Color_Area_Complete_Extra =
        CVarGetColor(CVAR_TRACKER_CHECK("AreaComplete.ExtraColor.Value"), Color_Area_Complete_Extra_Default);
    Color_Unchecked_Main = CVarGetColor(CVAR_TRACKER_CHECK("Unchecked.MainColor.Value"), Color_Main_Default);
    Color_Unchecked_Extra =
        CVarGetColor(CVAR_TRACKER_CHECK("Unchecked.ExtraColor.Value"), Color_Unchecked_Extra_Default);
    Color_Skipped_Main = CVarGetColor(CVAR_TRACKER_CHECK("Skipped.MainColor.Value"), Color_Main_Default);
    Color_Skipped_Extra = CVarGetColor(CVAR_TRACKER_CHECK("Skipped.ExtraColor.Value"), Color_Skipped_Extra_Default);
    Color_Seen_Main = CVarGetColor(CVAR_TRACKER_CHECK("Seen.MainColor.Value"), Color_Main_Default);
    Color_Seen_Extra = CVarGetColor(CVAR_TRACKER_CHECK("Seen.ExtraColor.Value"), Color_Seen_Extra_Default);
    Color_Hinted_Main = CVarGetColor(CVAR_TRACKER_CHECK("Hinted.MainColor.Value"), Color_Main_Default);
    Color_Hinted_Extra = CVarGetColor(CVAR_TRACKER_CHECK("Hinted.ExtraColor.Value"), Color_Hinted_Extra_Default);
    Color_Collected_Main = CVarGetColor(CVAR_TRACKER_CHECK("Collected.MainColor.Value"), Color_Main_Default);
    Color_Collected_Extra =
        CVarGetColor(CVAR_TRACKER_CHECK("Collected.ExtraColor.Value"), Color_Collected_Extra_Default);
    Color_Scummed_Main = CVarGetColor(CVAR_TRACKER_CHECK("Scummed.MainColor.Value"), Color_Main_Default);
    Color_Scummed_Extra = CVarGetColor(CVAR_TRACKER_CHECK("Scummed.ExtraColor.Value"), Color_Scummed_Extra_Default);
    Color_Saved_Main = CVarGetColor(CVAR_TRACKER_CHECK("Saved.MainColor.Value"), Color_Main_Default);
    Color_Saved_Extra = CVarGetColor(CVAR_TRACKER_CHECK("Saved.ExtraColor.Value"), Color_Saved_Extra_Default);
    hideUnchecked = CVarGetInteger(CVAR_TRACKER_CHECK("Unchecked.Hide"), 0);
    hideScummed = CVarGetInteger(CVAR_TRACKER_CHECK("Scummed.Hide"), 0);
    hideSeen = CVarGetInteger(CVAR_TRACKER_CHECK("Seen.Hide"), 0);
    hideSkipped = CVarGetInteger(CVAR_TRACKER_CHECK("Skipped.Hide"), 0);
    hideSaved = CVarGetInteger(CVAR_TRACKER_CHECK("Saved.Hide"), 0);
    hideCollected = CVarGetInteger(CVAR_TRACKER_CHECK("Collected.Hide"), 0);
    showHidden = CVarGetInteger(CVAR_TRACKER_CHECK("ShowHidden"), 0);
    mystery = CVarGetInteger(CVAR_RANDOMIZER_ENHANCEMENT("MysteriousShuffle"), 0);
    showLogicTooltip = CVarGetInteger(CVAR_TRACKER_CHECK("ShowLogic"), 0);
    enableAvailableChecks = CVarGetInteger(CVAR_TRACKER_CHECK("EnableAvailableChecks"), 0);
    onlyShowAvailable = CVarGetInteger(CVAR_TRACKER_CHECK("OnlyShowAvailable"), 0);

    hideShopUnshuffledChecks = CVarGetInteger(CVAR_TRACKER_CHECK("HideUnshuffledShopChecks"), 0);
    alwaysShowGS = CVarGetInteger(CVAR_TRACKER_CHECK("AlwaysShowGSLocs"), 0);
    if (CVarGetInteger(CVAR_TRACKER_CHECK("WindowType"), TRACKER_WINDOW_WINDOW) == TRACKER_WINDOW_FLOATING) {
        if (CVarGetInteger(CVAR_TRACKER_CHECK("ShowOnlyPaused"), 0) &&
            (gPlayState == nullptr || gPlayState->pauseCtx.state == 0)) {
            return;
        }

        if (CVarGetInteger(CVAR_TRACKER_CHECK("DisplayType"), TRACKER_DISPLAY_ALWAYS) == TRACKER_DISPLAY_COMBO_BUTTON) {
            int comboButton1Mask = buttons[CVarGetInteger(CVAR_TRACKER_CHECK("ComboButton1"), TRACKER_COMBO_BUTTON_L)];
            int comboButton2Mask = buttons[CVarGetInteger(CVAR_TRACKER_CHECK("ComboButton2"), TRACKER_COMBO_BUTTON_R)];
            OSContPad* trackerButtonsPressed =
                std::dynamic_pointer_cast<LUS::ControlDeck>(Ship::Context::GetInstance()->GetControlDeck())->GetPads();
            bool comboButtonsHeld = trackerButtonsPressed != nullptr &&
                                    trackerButtonsPressed[0].button & comboButton1Mask &&
                                    trackerButtonsPressed[0].button & comboButton2Mask;
            if (!comboButtonsHeld) {
                return;
            }
        }
    }

    if (presetLoaded) {
        ImGui::SetNextWindowSize(presetSize);
        ImGui::SetNextWindowPos(presetPos);
        presetLoaded = false;
    } else {
        ImGui::SetNextWindowSize(ImVec2(400, 540), ImGuiCond_FirstUseEver);
    }
    BeginFloatWindows("Check Tracker", mIsVisible, ImGuiWindowFlags_NoScrollbar);

    if (!GameInteractor::IsSaveLoaded() || !initialized) {
        ImGui::Text("Waiting for file load..."); // TODO Language
        EndFloatWindows();
        return;
    }

    SceneID sceneId = SCENE_ID_MAX;
    if (gPlayState != nullptr) {
        sceneId = (SceneID)gPlayState->sceneNum;
    }

    // Quick Options
#ifdef __WIIU__
    float headerHeight = 40.0f;
#else
    float headerHeight = 20.0f;
#endif
    ImVec2 size = ImGui::GetContentRegionMax();
    size.y -= headerHeight;
    if (!ImGui::BeginTable("Check Tracker", 1, 0, size)) {
        EndFloatWindows();
        return;
    }

    ImGui::TableNextRow(0, headerHeight);
    ImGui::TableNextColumn();
    if (UIWidgets::CVarCheckbox(
            "Show Hidden Items", CVAR_TRACKER_CHECK("ShowHidden"),
            UIWidgets::CheckboxOptions(
                { { .tooltip = "When active, items will show hidden checks by default when updated to this state." } })
                .Color(THEME_COLOR))) {
        doAreaScroll = true;
        showHidden = CVarGetInteger(CVAR_TRACKER_CHECK("ShowHidden"), 0);
        RecalculateAllAreaTotals();
    }
    if (enableAvailableChecks) {
        if (UIWidgets::CVarCheckbox(
                "Only Show Available Checks", CVAR_TRACKER_CHECK("OnlyShowAvailable"),
                UIWidgets::CheckboxOptions({ { .tooltip = "When active, unavailable checks will be hidden." } })
                    .Color(THEME_COLOR))) {
            doAreaScroll = true;
            RecalculateAllAreaTotals();
        }
    }
    UIWidgets::PaddedSeparator();
    if (UIWidgets::Button("Expand All", UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(UIWidgets::Sizes::Inline))) {
        optCollapseAll = false;
        optExpandAll = true;
        doAreaScroll = true;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Collapse All",
                          UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(UIWidgets::Sizes::Inline))) {
        optExpandAll = false;
        optCollapseAll = true;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Clear", UIWidgets::ButtonOptions({ { .tooltip = "Clear the search field" } })
                                       .Color(THEME_COLOR)
                                       .Size(UIWidgets::Sizes::Inline))) {
        checkSearch.Clear();
        UpdateFilters();
        doAreaScroll = true;
    }
    UIWidgets::PushStyleCombobox(THEME_COLOR);
    if (checkSearch.Draw()) {
        UpdateFilters();
    }
    UIWidgets::PopStyleCombobox();

    ImGui::Separator();

    std::ostringstream totalChecksSS;
    totalChecksSS << "Total Checks: ";
    if (enableAvailableChecks) {
        totalChecksSS << totalChecksAvailable << " Available / ";
    }
    totalChecksSS << totalChecksGotten << " Checked / " << totalChecks << " Total";
    ImGui::Text(totalChecksSS.str().c_str());

    UIWidgets::PaddedSeparator();

    // Checks Section Lead-in
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    size = ImGui::GetContentRegionAvail();
    if (!ImGui::BeginTable("CheckTracker##Checks", 1, ImGuiTableFlags_ScrollY, size)) {
        ImGui::EndTable();
        EndFloatWindows();
        return;
    }
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    // Prep for loop
    RainbowTick();
    bool doDraw = false;
    bool thisAreaFullyChecked = false;
    bool mqSpoilers = CVarGetInteger(CVAR_TRACKER_CHECK("MQSpoilers"), 0);
    bool hideIncomplete = CVarGetInteger(CVAR_TRACKER_CHECK("AreaIncomplete.Hide"), 0);
    bool hideComplete = CVarGetInteger(CVAR_TRACKER_CHECK("AreaComplete.Hide"), 0);
    bool collapseLogic;
    bool doingCollapseOrExpand = optExpandAll || optCollapseAll;
    bool isThisAreaSpoiled;
    RandomizerCheckArea lastArea = RCAREA_INVALID;
    Color_RGBA8 mainColor;
    Color_RGBA8 extraColor;
    std::string stemp;

    bool shouldHideFilteredAreas = CVarGetInteger(CVAR_TRACKER_CHECK("HideFilteredAreas"), 1);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
    for (auto& [rcArea, checks] : checksByArea) {
        RandomizerCheckArea thisArea = currentArea;

        thisAreaFullyChecked = (areaChecksGotten[rcArea] == areaCheckTotals[rcArea]);
        // Last Area needs to be cleaned up
        if (lastArea != RCAREA_INVALID && doDraw) {
            UIWidgets::PaddedSeparator();
        }
        lastArea = rcArea;
        if (previousShowHidden != showHidden) {
            previousShowHidden = showHidden;
            doAreaScroll = true;
        }
        if ((shouldHideFilteredAreas && filterAreasHidden[rcArea]) ||
            (!showHidden && ((hideComplete && thisAreaFullyChecked) || (hideIncomplete && !thisAreaFullyChecked))) ||
            (enableAvailableChecks && onlyShowAvailable && areaChecksAvailable[rcArea] == 0)) {
            doDraw = false;
        } else {
            // Get the colour for the area
            if (thisAreaFullyChecked) {
                mainColor = Color_Area_Complete_Main;
                extraColor = Color_Area_Complete_Extra;
            } else {
                mainColor = Color_Area_Incomplete_Main;
                extraColor = Color_Area_Incomplete_Extra;
            }

            // Draw the area
            collapseLogic = !thisAreaFullyChecked;
            if (doingCollapseOrExpand) {
                if (optExpandAll) {
                    collapseLogic = true;
                } else if (optCollapseAll) {
                    collapseLogic = false;
                }
            }
            stemp = RandomizerCheckObjects::GetRCAreaName(rcArea) + "##TreeNode";
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(mainColor.r / 255.0f, mainColor.g / 255.0f,
                                                        mainColor.b / 255.0f, mainColor.a / 255.0f));
            if (doingCollapseOrExpand) {
                ImGui::SetNextItemOpen(collapseLogic, ImGuiCond_Always);
            } else {
                ImGui::SetNextItemOpen(!thisAreaFullyChecked, ImGuiCond_Once);
            }
            doDraw = ImGui::TreeNode(stemp.c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(extraColor.r / 255.0f, extraColor.g / 255.0f,
                                                        extraColor.b / 255.0f, extraColor.a / 255.0f));

            isThisAreaSpoiled = IsAreaSpoiled(rcArea) || mqSpoilers;

            if (isThisAreaSpoiled) {
                std::ostringstream areaTotalsSS;
                std::ostringstream areaTotalsTooltipSS;

                areaTotalsSS << "(";
                if (enableAvailableChecks) {
                    areaTotalsSS << static_cast<uint16_t>(areaChecksAvailable[rcArea]) << " / ";
                    areaTotalsTooltipSS << "Available / ";
                }
                areaTotalsSS << static_cast<uint16_t>(areaChecksGotten[rcArea]) << " / "
                             << static_cast<uint16_t>(areaCheckTotals[rcArea]) << ")";
                areaTotalsTooltipSS << "Checked / Total";

                if (showVOrMQ && RandomizerCheckObjects::AreaIsDungeon(rcArea)) {
                    if (OTRGlobals::Instance->gRandoContext->GetDungeons()
                            ->GetDungeonFromScene(DungeonSceneLookupByArea(rcArea))
                            ->IsMQ()) {
                        areaTotalsSS << " - MQ";
                    } else {
                        areaTotalsSS << " - Vanilla";
                    }
                }

                ImGui::Text(areaTotalsSS.str().c_str());
                UIWidgets::Tooltip(areaTotalsTooltipSS.str().c_str());
            } else {
                ImGui::Text("???");
            }

            ImGui::PopStyleColor();

            // Keep areas loaded between transitions
            if (thisArea == rcArea && doAreaScroll) {
                ImGui::SetScrollHereY(0.0f);
                doAreaScroll = false;
            }
            for (auto rc : checks) {
                if (doDraw && isThisAreaSpoiled && !filterChecksHidden[rc]) {
                    DrawLocation(rc);
                }
            }

            if (doDraw) {
                ImGui::TreePop();
            }
        }
    }
    ImGui::PopStyleVar();

    ImGui::EndTable(); // Checks Lead-out
    ImGui::EndTable(); // Quick Options Lead-out
    EndFloatWindows();
    if (doingCollapseOrExpand) {
        optCollapseAll = false;
        optExpandAll = false;
    }
}

bool UpdateFilters() {
    for (auto& [rcArea, checks] : checksByArea) {
        filterAreasHidden[rcArea] = !checkSearch.PassFilter(RandomizerCheckObjects::GetRCAreaName(rcArea).c_str());
        for (auto check : checks) {
            if (ShouldShowCheck(check)) {
                filterAreasHidden[rcArea] = false;
                filterChecksHidden[check] = false;
            } else {
                filterChecksHidden[check] = true;
            }
        }
    }

    return true;
}

bool ShouldShowCheck(RandomizerCheck check) {
    auto itemLoc = Rando::Context::GetInstance()->GetItemLocation(check);
    std::string search = (Rando::StaticData::GetLocation(check)->GetShortName() + " " +
                          Rando::StaticData::GetLocation(check)->GetName() + " " +
                          RandomizerCheckObjects::GetRCAreaName(Rando::StaticData::GetLocation(check)->GetArea()));
    if (itemLoc->HasObtained() || itemLoc->GetCheckStatus() == RCSHOW_SCUMMED ||
        (!mystery && (itemLoc->GetCheckStatus() == RCSHOW_IDENTIFIED || itemLoc->GetCheckStatus() == RCSHOW_SEEN) &&
         itemLoc->GetPlacedRandomizerGet() != RG_ICE_TRAP)) {
        search += " " + itemLoc->GetPlacedItemName().GetForLanguage(gSaveContext.language);
    } else if (itemLoc->GetCheckStatus() == RCSHOW_IDENTIFIED && !mystery) {
        search +=
            OTRGlobals::Instance->gRandoContext->overrides[check].GetTrickName().GetForLanguage(gSaveContext.language);
    } else if (itemLoc->GetCheckStatus() == RCSHOW_SEEN && !mystery) {
        search += Rando::StaticData::RetrieveItem(OTRGlobals::Instance->gRandoContext->overrides[check].LooksLike())
                      .GetName()
                      .GetForLanguage(gSaveContext.language);
    }
    return (IsVisibleInCheckTracker(check) &&
            (checkSearch.Filters.Size == 0 || checkSearch.PassFilter(search.c_str())));
}

// Windowing stuff
void BeginFloatWindows(std::string UniqueName, bool& open, ImGuiWindowFlags flags) {
    ImGuiWindowFlags windowFlags = flags;

    if (windowFlags == 0) {
        windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoFocusOnAppearing;
    }

    if (CVarGetInteger(CVAR_TRACKER_CHECK("WindowType"), TRACKER_WINDOW_WINDOW) == TRACKER_WINDOW_FLOATING) {
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        windowFlags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar |
                       ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

        if (!CVarGetInteger(CVAR_TRACKER_CHECK("Draggable"), 1)) {
            windowFlags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
        }
    }
    auto maybeParent = ImGui::GetCurrentWindow();
    ImGuiWindow* window = ImGui::FindWindowByName(UniqueName.c_str());
    if (window != NULL && window->DockTabIsVisible && window->ParentWindow != NULL &&
        std::string(window->ParentWindow->Name).compare(0, strlen("Main - Deck"), "Main - Deck") == 0) {
        Color_Background.a = 255;
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, VecFromRGBA8(Color_Background));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::Begin(UniqueName.c_str(), &open, windowFlags);
}
void EndFloatWindows() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::End();
}

void LoadSettings() {
    // If in randomzer, then get the setting and check if in general we should be showing the settings
    // If in vanilla, _try_ to show items that at least are needed for 100%

    showShops =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHOPSANITY) != RO_SHOPSANITY_OFF : false;
    showBeans = IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_MERCHANTS) ==
                                   RO_SHUFFLE_MERCHANTS_BEANS_ONLY ||
                               OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_MERCHANTS) ==
                                   RO_SHUFFLE_MERCHANTS_ALL
                         : true;
    showScrubs =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_SCRUBS) == RO_SCRUBS_ALL : false;
    showMajorScrubs =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_SCRUBS) != RO_SCRUBS_OFF : false;
    showMerchants = IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_MERCHANTS) ==
                                       RO_SHUFFLE_MERCHANTS_ALL_BUT_BEANS ||
                                   OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_MERCHANTS) ==
                                       RO_SHUFFLE_MERCHANTS_ALL
                             : true;
    showSongs = IS_RANDO
                    ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_SONGS) != RO_SONG_SHUFFLE_OFF
                    : false;
    showBeehives = IS_RANDO
                       ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_BEEHIVES) == RO_GENERIC_YES
                       : false;
    showCows =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_COWS) == RO_GENERIC_YES : false;
    showAdultTrade =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_ADULT_TRADE) == RO_GENERIC_YES
                 : true;
    showKokiriSword =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_KOKIRI_SWORD) == RO_GENERIC_YES
                 : true;
    showMasterSword =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_MASTER_SWORD) == RO_GENERIC_YES
                 : true;
    showHyruleLoach =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_FISHSANITY) == RO_FISHSANITY_HYRULE_LOACH
                 : false;
    showWeirdEgg =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_WEIRD_EGG) == RO_GENERIC_YES
                 : true;
    showGerudoCard = IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(
                                    RSK_SHUFFLE_GERUDO_MEMBERSHIP_CARD) == RO_GENERIC_YES
                              : true;
    showFrogSongRupees =
        IS_RANDO
            ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_FROG_SONG_RUPEES) == RO_GENERIC_YES
            : false;
    showFairies = IS_RANDO
                      ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_FAIRIES) == RO_GENERIC_YES
                      : false;
    showStartingMapsCompasses = IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(
                                               RSK_SHUFFLE_MAPANDCOMPASS) != RO_DUNGEON_ITEM_LOC_VANILLA
                                         : false;
    showKeysanity =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_KEYSANITY) != RO_DUNGEON_ITEM_LOC_VANILLA
                 : false;
    showBossKeysanity = IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_BOSS_KEYSANITY) !=
                                       RO_DUNGEON_ITEM_LOC_VANILLA
                                 : false;
    showGerudoFortressKeys =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_GERUDO_KEYS) != RO_GERUDO_KEYS_VANILLA
                 : false;
    showGanonBossKey = IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_GANONS_BOSS_KEY) !=
                                      RO_GANON_BOSS_KEY_VANILLA
                                : false;
    showOcarinas = IS_RANDO
                       ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_OCARINA) == RO_GENERIC_YES
                       : false;
    show100SkullReward =
        IS_RANDO ? OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_100_GS_REWARD) == RO_GENERIC_YES
                 : false;
    showLinksPocket =
        IS_RANDO ? // don't show Link's Pocket if not randomizer, or if rando and pocket is disabled
            OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_LINKS_POCKET) != RO_LINKS_POCKET_NOTHING
                 : false;

    if (IS_RANDO) {
        switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_TOKENS)) {
            case RO_TOKENSANITY_ALL:
                showOverworldTokens = true;
                showDungeonTokens = true;
                break;
            case RO_TOKENSANITY_OVERWORLD:
                showOverworldTokens = true;
                showDungeonTokens = false;
                break;
            case RO_TOKENSANITY_DUNGEONS:
                showOverworldTokens = false;
                showDungeonTokens = true;
                break;
            default:
                showOverworldTokens = false;
                showDungeonTokens = false;
                break;
        }

        switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_POTS)) {
            case RO_SHUFFLE_POTS_ALL:
                showOverworldPots = true;
                showDungeonPots = true;
                break;
            case RO_SHUFFLE_POTS_OVERWORLD:
                showOverworldPots = true;
                showDungeonPots = false;
                break;
            case RO_SHUFFLE_POTS_DUNGEONS:
                showOverworldPots = false;
                showDungeonPots = true;
                break;
            default:
                showOverworldPots = false;
                showDungeonPots = false;
                break;
        }

        switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_GRASS)) {
            case RO_SHUFFLE_GRASS_ALL:
                showOverworldGrass = true;
                showDungeonGrass = true;
                break;
            case RO_SHUFFLE_GRASS_OVERWORLD:
                showOverworldGrass = true;
                showDungeonGrass = false;
                break;
            case RO_SHUFFLE_GRASS_DUNGEONS:
                showOverworldGrass = false;
                showDungeonGrass = true;
                break;
            default:
                showOverworldGrass = false;
                showDungeonGrass = false;
                break;
        }

        switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_CRATES)) {
            case RO_SHUFFLE_CRATES_ALL:
                showOverworldCrates = true;
                showDungeonCrates = true;
                break;
            case RO_SHUFFLE_CRATES_OVERWORLD:
                showOverworldCrates = true;
                showDungeonCrates = false;
                break;
            case RO_SHUFFLE_CRATES_DUNGEONS:
                showOverworldCrates = false;
                showDungeonCrates = true;
                break;
            default:
                showOverworldCrates = false;
                showDungeonCrates = false;
                break;
        }
    } else { // Vanilla
        showOverworldTokens = true;
        showDungeonTokens = true;
        showOverworldPots = false;
        showDungeonPots = false;
        showOverworldGrass = false;
        showDungeonGrass = false;
        showOverworldCrates = false;
        showDungeonCrates = false;
    }

    fortressFast = false;
    fortressNormal = false;
    switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_GERUDO_FORTRESS)) {
        case RO_GF_CARPENTERS_FREE:
            showGerudoFortressKeys = false;
            showGerudoCard = false;
            break;
        case RO_GF_CARPENTERS_FAST:
            fortressFast = true;
            break;
        case RO_GF_CARPENTERS_NORMAL:
            fortressNormal = true;
            break;
    }

    fishsanityMode = OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_FISHSANITY);
    fishsanityPondCount = OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_FISHSANITY_POND_COUNT);
    fishsanityAgeSplit = OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_FISHSANITY_AGE_SPLIT);

    if (IS_RANDO) {
        switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_FREESTANDING)) {
            case RO_SHUFFLE_FREESTANDING_ALL:
                showOverworldFreestanding = true;
                showDungeonFreestanding = true;
                break;
            case RO_SHUFFLE_FREESTANDING_OVERWORLD:
                showOverworldFreestanding = true;
                showDungeonFreestanding = false;
                break;
            case RO_SHUFFLE_FREESTANDING_DUNGEONS:
                showOverworldFreestanding = false;
                showDungeonFreestanding = true;
                break;
            default:
                showOverworldFreestanding = false;
                showDungeonFreestanding = false;
                break;
        }
    } else { // Vanilla
        showOverworldFreestanding = false;
        showDungeonFreestanding = true;
    }

    switch (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_GANONS_BOSS_KEY)) {
        case RO_GANON_BOSS_KEY_LACS_STONES:
            Rando::Context::GetInstance()->LACSCondition(RO_LACS_STONES);
            break;
        case RO_GANON_BOSS_KEY_LACS_MEDALLIONS:
            Rando::Context::GetInstance()->LACSCondition(RO_LACS_MEDALLIONS);
            break;
        case RO_GANON_BOSS_KEY_LACS_REWARDS:
            Rando::Context::GetInstance()->LACSCondition(RO_LACS_REWARDS);
            break;
        case RO_GANON_BOSS_KEY_LACS_DUNGEONS:
            Rando::Context::GetInstance()->LACSCondition(RO_LACS_DUNGEONS);
            break;
        case RO_GANON_BOSS_KEY_LACS_TOKENS:
            Rando::Context::GetInstance()->LACSCondition(RO_LACS_TOKENS);
            break;
        default:
            Rando::Context::GetInstance()->LACSCondition(RO_LACS_VANILLA);
            break;
    }
}

bool IsCheckShuffled(RandomizerCheck rc) {
    Rando::Location* loc = Rando::StaticData::GetLocation(rc);
    if (loc->GetRCType() == RCTYPE_SHOP) {
        auto identity = OTRGlobals::Instance->gRandomizer->IdentifyShopItem(loc->GetScene(), loc->GetActorParams() + 1);
    }
    if (IS_RANDO && OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_LOGIC_RULES) != RO_LOGIC_VANILLA) {
        return (loc->GetArea() != RCAREA_INVALID) &&        // don't show Invalid locations
               (loc->GetRCType() != RCTYPE_GOSSIP_STONE) && // TODO: Don't show hints until tracker supports them
               (loc->GetRCType() != RCTYPE_STATIC_HINT) &&  // TODO: Don't show hints until tracker supports them
               (loc->GetRCType() !=
                RCTYPE_CHEST_GAME) && // don't show non final reward chest game checks until we support shuffling them
               (rc != RC_HC_ZELDAS_LETTER) && // don't show zeldas letter until we support shuffling it
               (rc != RC_LINKS_POCKET || showLinksPocket) &&
               OTRGlobals::Instance->gRandoContext->IsQuestOfLocationActive(rc) &&
               (loc->GetRCType() != RCTYPE_SHOP ||
                (showShops &&
                 OTRGlobals::Instance->gRandomizer->IdentifyShopItem(loc->GetScene(), loc->GetActorParams() + 1)
                         .enGirlAShopItem == 50)) &&
               (rc != RC_TRIFORCE_COMPLETED) && (rc != RC_GANON) &&
               (loc->GetRCType() != RCTYPE_SCRUB || showScrubs ||
                (showMajorScrubs && (rc == RC_LW_DEKU_SCRUB_NEAR_BRIDGE || // The 3 scrubs that are always randomized
                                     rc == RC_HF_DEKU_SCRUB_GROTTO || rc == RC_LW_DEKU_SCRUB_GROTTO_FRONT))) &&
               (loc->GetRCType() != RCTYPE_MERCHANT || showMerchants) &&
               (loc->GetRCType() != RCTYPE_SONG_LOCATION || showSongs) &&
               (loc->GetRCType() != RCTYPE_BEEHIVE || showBeehives) &&
               (loc->GetRCType() != RCTYPE_OCARINA || showOcarinas) &&
               (loc->GetRCType() != RCTYPE_SKULL_TOKEN || alwaysShowGS ||
                (showOverworldTokens && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea())) ||
                (showDungeonTokens && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_POT ||
                (showOverworldPots && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea())) ||
                (showDungeonPots && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_GRASS ||
                (showOverworldGrass && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea())) ||
                (showDungeonGrass && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_CRATE ||
                (showOverworldCrates && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea())) ||
                (showDungeonCrates && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_NLCRATE ||
                (showOverworldCrates && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea()) &&
                 OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_LOGIC_RULES) == RO_LOGIC_NO_LOGIC) ||
                (showDungeonCrates && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_SMALL_CRATE ||
                (showOverworldCrates && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea())) ||
                (showDungeonCrates && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_COW || showCows) &&
               (loc->GetRCType() != RCTYPE_FISH ||
                OTRGlobals::Instance->gRandoContext->GetFishsanity()->GetFishLocationIncluded(loc)) &&
               (loc->GetRCType() != RCTYPE_FREESTANDING ||
                (showOverworldFreestanding && RandomizerCheckObjects::AreaIsOverworld(loc->GetArea())) ||
                (showDungeonFreestanding && RandomizerCheckObjects::AreaIsDungeon(loc->GetArea()))) &&
               (loc->GetRCType() != RCTYPE_ADULT_TRADE || showAdultTrade ||
                rc == RC_KAK_ANJU_AS_ADULT ||  // adult trade checks that are always shuffled
                rc == RC_DMT_TRADE_CLAIM_CHECK // even when shuffle adult trade is off
                ) &&
               (rc != RC_KF_KOKIRI_SWORD_CHEST || showKokiriSword) && (rc != RC_TOT_MASTER_SWORD || showMasterSword) &&
               (rc != RC_LH_HYRULE_LOACH || showHyruleLoach) && (rc != RC_ZR_MAGIC_BEAN_SALESMAN || showBeans) &&
               (rc != RC_HC_MALON_EGG || showWeirdEgg) &&
               (loc->GetRCType() != RCTYPE_FROG_SONG || showFrogSongRupees) &&
               ((loc->GetRCType() != RCTYPE_MAP && loc->GetRCType() != RCTYPE_COMPASS) || showStartingMapsCompasses) &&
               (loc->GetRCType() != RCTYPE_FAIRY || showFairies) &&
               (loc->GetRCType() != RCTYPE_SMALL_KEY || showKeysanity) &&
               (loc->GetRCType() != RCTYPE_BOSS_KEY || showBossKeysanity) &&
               (loc->GetRCType() != RCTYPE_GANON_BOSS_KEY || showGanonBossKey) &&
               (rc != RC_KAK_100_GOLD_SKULLTULA_REWARD || show100SkullReward) &&
               (loc->GetRCType() != RCTYPE_GF_KEY && rc != RC_GF_GERUDO_MEMBERSHIP_CARD ||
                (showGerudoCard && rc == RC_GF_GERUDO_MEMBERSHIP_CARD) ||
                (fortressNormal && showGerudoFortressKeys && loc->GetRCType() == RCTYPE_GF_KEY) ||
                (fortressFast && showGerudoFortressKeys && rc == RC_GF_NORTH_F1_CARPENTER));
    } else if (loc->IsVanillaCompletion()) {
        return (OTRGlobals::Instance->gRandoContext->IsQuestOfLocationActive(rc) || rc == RC_GIFT_FROM_RAURU) &&
               rc != RC_LINKS_POCKET;
    }
    return false;
}

bool IsVisibleInCheckTracker(RandomizerCheck rc) {
    auto loc = Rando::StaticData::GetLocation(rc);
    if (IS_RANDO) {
        return IsCheckShuffled(rc) ||
               (alwaysShowGS && loc->GetRCType() == RCTYPE_SKULL_TOKEN &&
                OTRGlobals::Instance->gRandoContext->IsQuestOfLocationActive(rc)) ||
               (loc->GetRCType() == RCTYPE_SHOP && showShops && !hideShopUnshuffledChecks);
    } else {
        return loc->IsVanillaCompletion() &&
               (!loc->IsDungeon() || (loc->IsDungeon() && loc->GetQuest() == gSaveContext.ship.quest.id));
    }
}

void UpdateInventoryChecks() {
    // For all the areas with maps, if you have one, spoil the area
    for (auto [scene, area] : DungeonRCAreasBySceneID) {
        if (CHECK_DUNGEON_ITEM(DUNGEON_MAP, scene)) {
            SetAreaSpoiled(area);
        }
    }
}

void UpdateAreaFullyChecked(RandomizerCheckArea area) {
}

void UpdateAllAreas() {
    // Sort the entire thing
    for (int i = 0; i < RCAREA_INVALID; i++) {
        UpdateAreas(static_cast<RandomizerCheckArea>(i));
    }
}

void UpdateAreas(RandomizerCheckArea area) {
    if (checksByArea.contains(area)) {
        areasFullyChecked[area] = areaChecksGotten[area] == checksByArea.find(area)->second.size();
    }
}

void UpdateAllOrdering() {
    // Sort the entire thing
    for (int i = 0; i < RCAREA_INVALID; i++) {
        UpdateOrdering(static_cast<RandomizerCheckArea>(i));
    }
}

void UpdateOrdering(RandomizerCheckArea rcArea) {
    // Sort a single area
    if (checksByArea.contains(rcArea)) {
        std::sort(checksByArea.find(rcArea)->second.begin(), checksByArea.find(rcArea)->second.end(), CompareChecks);
    }
    RecalculateAllAreaTotals();
    CalculateTotals();
}

bool IsEoDCheck(RandomizerCheckType type) {
    return type == RCTYPE_BOSS_HEART_OR_OTHER_REWARD || type == RCTYPE_DUNGEON_REWARD;
}

bool CompareChecks(RandomizerCheck i, RandomizerCheck j) {
    Rando::Location* x = Rando::StaticData::GetLocation(i);
    Rando::Location* y = Rando::StaticData::GetLocation(j);
    auto itemI = OTRGlobals::Instance->gRandoContext->GetItemLocation(i);
    auto itemJ = OTRGlobals::Instance->gRandoContext->GetItemLocation(j);
    bool iCollected = itemI->HasObtained();
    bool iSaved = itemI->GetCheckStatus() == RCSHOW_SAVED;
    bool jCollected = itemJ->HasObtained();
    bool jSaved = itemJ->GetCheckStatus() == RCSHOW_SAVED;

    if (!iCollected && jCollected) {
        return true;
    } else if (iCollected && !jCollected) {
        return false;
    }

    if (!iSaved && jSaved) {
        return true;
    } else if (iSaved && !jSaved) {
        return false;
    }

    if (!itemI->GetIsSkipped() && itemJ->GetIsSkipped()) {
        return true;
    } else if (itemI->GetIsSkipped() && !itemJ->GetIsSkipped()) {
        return false;
    }

    if (!IsEoDCheck(x->GetRCType()) && IsEoDCheck(y->GetRCType())) {
        return true;
    } else if (IsEoDCheck(x->GetRCType()) && !IsEoDCheck(y->GetRCType())) {
        return false;
    }

    if (i < j) {
        return true;
    } else if (i > j) {
        return false;
    }

    return false;
}

bool IsHeartPiece(GetItemID giid) {
    return giid == GI_HEART_PIECE || giid == GI_HEART_PIECE_WIN;
}

void DrawLocation(RandomizerCheck rc) {
    Color_RGBA8 mainColor;
    Color_RGBA8 extraColor;
    std::string txt;
    Rando::Location* loc = Rando::StaticData::GetLocation(rc);
    Rando::ItemLocation* itemLoc = OTRGlobals::Instance->gRandoContext->GetItemLocation(rc);
    RandomizerCheckStatus status = itemLoc->GetCheckStatus();
    bool skipped = itemLoc->GetIsSkipped();
    bool available = itemLoc->IsAvailable();

    if (enableAvailableChecks && onlyShowAvailable && !available) {
        return;
    }

    if (status == RCSHOW_COLLECTED) {
        if (!showHidden && hideCollected) {
            return;
        }
        mainColor =
            !IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID()) && !IS_RANDO
                ? Color_Collected_Extra
                : Color_Collected_Main;
        extraColor = Color_Collected_Extra;
    } else if (status == RCSHOW_SAVED) {
        if (!showHidden && hideSaved) {
            return;
        }
        mainColor =
            !IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID()) && !IS_RANDO
                ? Color_Saved_Extra
                : Color_Saved_Main;
        extraColor = Color_Saved_Extra;
    } else if (skipped) {
        if (!showHidden && hideSkipped) {
            return;
        }
        mainColor =
            !IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID()) && !IS_RANDO
                ? Color_Skipped_Extra
                : Color_Skipped_Main;
        extraColor = Color_Skipped_Extra;
    } else if (status == RCSHOW_SEEN || status == RCSHOW_IDENTIFIED) {
        if (!showHidden && hideSeen) {
            return;
        }
        mainColor =
            !IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID()) && !IS_RANDO
                ? Color_Seen_Extra
                : Color_Seen_Main;
        extraColor = Color_Seen_Extra;
    } else if (status == RCSHOW_SCUMMED) {
        if (!showHidden && hideScummed) {
            return;
        }
        mainColor =
            !IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID()) && !IS_RANDO
                ? Color_Scummed_Extra
                : Color_Scummed_Main;
        extraColor = Color_Scummed_Extra;
    } else if (status == RCSHOW_UNCHECKED) {
        if (!showHidden && hideUnchecked) {
            return;
        }
        mainColor =
            !IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID()) && !IS_RANDO
                ? Color_Unchecked_Extra
                : Color_Unchecked_Main;
        extraColor = Color_Unchecked_Extra;
    }

    // Main Text
    if (checkNameOverrides.contains(loc->GetRandomizerCheck())) {
        txt = checkNameOverrides[loc->GetRandomizerCheck()];
    } else {
        txt = loc->GetShortName();
    }

    if (lastLocationChecked == loc->GetRandomizerCheck()) {
        txt = "* " + txt;
    }

    // Draw button - for Skipped/Seen/Scummed/Unchecked only
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 3.0f });
    float sz = ImGui::GetFrameHeight();
    if (status == RCSHOW_UNCHECKED || status == RCSHOW_SEEN || status == RCSHOW_IDENTIFIED ||
        status == RCSHOW_SCUMMED || skipped) {
        if (UIWidgets::StateButton(std::to_string(rc).c_str(), skipped ? ICON_FA_PLUS : ICON_FA_TIMES, ImVec2(sz, sz),
                                   UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
            if (skipped) {
                OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->SetIsSkipped(false);
                areaChecksGotten[loc->GetArea()]--;
                totalChecksGotten--;
                if (available) {
                    areaChecksAvailable[loc->GetArea()]++;
                    totalChecksAvailable++;
                }
            } else {
                OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->SetIsSkipped(true);
                areaChecksGotten[loc->GetArea()]++;
                totalChecksGotten++;
                if (available) {
                    areaChecksAvailable[loc->GetArea()]--;
                    totalChecksAvailable--;
                }
            }
            UpdateOrdering(loc->GetArea());
            UpdateInventoryChecks();
            SaveManager::Instance->SaveSection(gSaveContext.fileNum, sectionId, true);
        }
    } else {
        ImGui::Dummy(ImVec2(sz, sz));
    }
    ImGui::PopStyleVar();

    ImGui::SameLine();

    // Draw
    ImVec4 styleColor(mainColor.r / 255.0f, mainColor.g / 255.0f, mainColor.b / 255.0f, mainColor.a / 255.0f);
    if (enableAvailableChecks) {
        if (itemLoc->HasObtained()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 0));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, styleColor);
        }
        ImGui::Text("%s", available ? ICON_FA_UNLOCK : ICON_FA_LOCK);
        ImGui::PopStyleColor();
        ImGui::SameLine();
    }

    ImGui::PushStyleColor(ImGuiCol_Text, styleColor);
    ImGui::Text("%s", txt.c_str());
    ImGui::PopStyleColor();

    // Draw the extra info
    txt = "";

    if (status != RCSHOW_UNCHECKED) {
        switch (status) {
            case RCSHOW_SAVED:
            case RCSHOW_COLLECTED:
            case RCSHOW_SCUMMED:
                if (IS_RANDO) {
                    txt = itemLoc->GetPlacedItem().GetName().GetForLanguage(gSaveContext.language);
                } else {
                    if (IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID())) {
                        if (gSaveContext.language == LANGUAGE_ENG || gSaveContext.language == LANGUAGE_GER ||
                            gSaveContext.language == LANGUAGE_JPN) {
                            txt = Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetName().english;
                        } else if (gSaveContext.language == LANGUAGE_FRA) {
                            txt = Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetName().french;
                        }
                    }
                }
                break;
            case RCSHOW_IDENTIFIED:
            case RCSHOW_SEEN:
                if (IS_RANDO) {
                    if (itemLoc->GetPlacedRandomizerGet() == RG_ICE_TRAP && !mystery) {
                        if (status == RCSHOW_IDENTIFIED) {
                            txt = OTRGlobals::Instance->gRandoContext->overrides[rc].GetTrickName().GetForLanguage(
                                gSaveContext.language);
                        } else {
                            txt = Rando::StaticData::RetrieveItem(
                                      OTRGlobals::Instance->gRandoContext->overrides[rc].LooksLike())
                                      .GetName()
                                      .GetForLanguage(gSaveContext.language);
                        }
                    } else if (!mystery) {
                        txt = itemLoc->GetPlacedItem().GetName().GetForLanguage(gSaveContext.language);
                    }
                    if (IsVisibleInCheckTracker(rc) && status == RCSHOW_IDENTIFIED && !mystery) {
                        auto price = OTRGlobals::Instance->gRandoContext->GetItemLocation(rc)->GetPrice();
                        if (price) {
                            txt += fmt::format(" - {}", price);
                        }
                    }
                } else {
                    if (IsHeartPiece((GetItemID)Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetItemID())) {
                        if (gSaveContext.language == LANGUAGE_ENG || gSaveContext.language == LANGUAGE_GER ||
                            gSaveContext.language == LANGUAGE_JPN) {
                            txt = Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetName().english;
                        } else if (gSaveContext.language == LANGUAGE_FRA) {
                            txt = Rando::StaticData::RetrieveItem(loc->GetVanillaItem()).GetName().french;
                        }
                    }
                }
                break;
        }
    }
    if (txt == "" && skipped) {
        txt = "Skipped"; // TODO language
    }

    if (txt != "") {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(extraColor.r / 255.0f, extraColor.g / 255.0f, extraColor.b / 255.0f,
                                                    extraColor.a / 255.0f));
        ImGui::SameLine();
        ImGui::Text(" (%s)", txt.c_str());
        ImGui::PopStyleColor();
    }

    if (showLogicTooltip) {
        for (auto& locationInRegion : areaTable[itemLoc->GetParentRegionKey()].locations) {
            if (locationInRegion.GetLocation() == rc) {
                std::string conditionStr = locationInRegion.GetConditionStr();
                if (conditionStr != "true") {
                    UIWidgets::Tooltip(conditionStr.c_str());
                }
                break;
            }
        }
    }
}

static std::set<std::string> rainbowCVars = {
    CVAR_TRACKER_CHECK("AreaIncomplete.MainColor"), CVAR_TRACKER_CHECK("AreaIncomplete.ExtraColor"),
    CVAR_TRACKER_CHECK("AreaComplete.MainColor"),   CVAR_TRACKER_CHECK("AreaComplete.ExtraColor"),
    CVAR_TRACKER_CHECK("Unchecked.MainColor"),      CVAR_TRACKER_CHECK("Unchecked.ExtraColor"),
    CVAR_TRACKER_CHECK("Skipped.MainColor"),        CVAR_TRACKER_CHECK("Skipped.ExtraColor"),
    CVAR_TRACKER_CHECK("Seen.MainColor"),           CVAR_TRACKER_CHECK("Seen.ExtraColor"),
    CVAR_TRACKER_CHECK("Hinted.MainColor"),         CVAR_TRACKER_CHECK("Hinted.ExtraColor"),
    CVAR_TRACKER_CHECK("Collected.MainColor"),      CVAR_TRACKER_CHECK("Collected.ExtraColor"),
    CVAR_TRACKER_CHECK("Scummed.MainColor"),        CVAR_TRACKER_CHECK("Scummed.ExtraColor"),
    CVAR_TRACKER_CHECK("Saved.MainColor"),          CVAR_TRACKER_CHECK("Saved.ExtraColor"),
};

int hue = 0;
void RainbowTick() {
    float freqHue = hue * 2 * M_PIf / (360 * CVarGetFloat(CVAR_COSMETIC("RainbowSpeed"), 0.6f));
    for (auto& cvar : rainbowCVars) {
        if (CVarGetInteger((cvar + ".Rainbow").c_str(), 0) == 0) {
            continue;
        }

        Color_RGBA8 newColor;
        newColor.r = static_cast<uint8_t>(sin(freqHue + 0) * 127) + 128;
        newColor.g = static_cast<uint8_t>(sin(freqHue + (2 * M_PI / 3)) * 127) + 128;
        newColor.b = static_cast<uint8_t>(sin(freqHue + (4 * M_PI / 3)) * 127) + 128;
        newColor.a = 255;

        CVarSetColor((cvar + ".Value").c_str(), newColor);
    }

    hue++;
    hue %= 360;
}

void ImGuiDrawTwoColorPickerSection(const char* text, const char* cvarMainName, const char* cvarExtraName,
                                    Color_RGBA8& main_color, Color_RGBA8& extra_color, Color_RGBA8& main_default_color,
                                    Color_RGBA8& extra_default_color, const char* cvarHideName, const char* tooltip,
                                    UIWidgets::Colors theme) {
    Color_RGBA8 cvarMainColor = CVarGetColor(cvarMainName, main_default_color);
    Color_RGBA8 cvarExtraColor = CVarGetColor(cvarExtraName, extra_default_color);
    main_color = cvarMainColor;
    extra_color = cvarExtraColor;

    UIWidgets::PushStyleCombobox(theme);
    if (ImGui::CollapsingHeader(text)) {
        if (*cvarHideName != '\0') {
            std::string label = cvarHideName;
            label += "##Hidden";
            ImGui::PushID(label.c_str());
            UIWidgets::CVarCheckbox(
                "Hidden", cvarHideName,
                UIWidgets::CheckboxOptions(
                    { { .tooltip = "When active, checks will hide by default when updated to this state. Can "
                                   "be overridden with the \"Show Hidden Items\" option." } })
                    .Color(theme));
            ImGui::PopID();
        }
        std::string mainLabel = "Name##" + std::string(cvarMainName);
        if (UIWidgets::CVarColorPicker(mainLabel.c_str(), cvarMainName, main_default_color, false,
                                       UIWidgets::ColorPickerRandomButton | UIWidgets::ColorPickerResetButton |
                                           UIWidgets::ColorPickerRainbowCheck,
                                       theme)) {
            main_color = CVarGetColor(cvarMainName, main_default_color);
        }

        std::string extraLabel = "Details##" + std::string(cvarExtraName);
        if (UIWidgets::CVarColorPicker(extraLabel.c_str(), cvarExtraName, extra_default_color, false,
                                       UIWidgets::ColorPickerRandomButton | UIWidgets::ColorPickerResetButton |
                                           UIWidgets::ColorPickerRainbowCheck,
                                       theme)) {
            extra_color = CVarGetColor(cvarExtraName, extra_default_color);
        }
    }
    if (tooltip != NULL && strlen(tooltip) != 0) {
        ImGui::SameLine();
        ImGui::Text(" ?");
        UIWidgets::Tooltip(tooltip);
    }
    UIWidgets::PopStyleCombobox();
}

void RecalculateAvailableChecks(RandomizerRegion startingRegion /* = RR_ROOT */) {
    if (!enableAvailableChecks) {
        return;
    }

    ResetPerformanceTimer(PT_RECALCULATE_AVAILABLE_CHECKS);
    StartPerformanceTimer(PT_RECALCULATE_AVAILABLE_CHECKS);

    const auto& ctx = Rando::Context::GetInstance();

    std::vector<RandomizerCheck> targetLocations;
    targetLocations.reserve(RC_MAX);
    for (auto& location : Rando::StaticData::GetLocationTable()) {
        RandomizerCheck rc = location.GetRandomizerCheck();
        Rando::ItemLocation* itemLocation = ctx->GetItemLocation(rc);
        itemLocation->SetAvailable(false);
        if (!itemLocation->HasObtained()) {
            targetLocations.emplace_back(rc);
        }
    }

    std::vector<RandomizerCheck> availableChecks = ReachabilitySearch(targetLocations, RG_NONE, true, startingRegion);
    for (auto& rc : availableChecks) {
        const auto& itemLocation = ctx->GetItemLocation(rc);
        itemLocation->SetAvailable(true);
    }

    totalChecksAvailable = 0;
    for (auto& [rcArea, vec] : checksByArea) {
        areaChecksAvailable[rcArea] = 0;
        for (auto& rc : vec) {
            Rando::ItemLocation* itemLocation = ctx->GetItemLocation(rc);
            if (itemLocation->IsAvailable() && IsVisibleInCheckTracker(rc) && !IsCheckHidden(rc)) {
                areaChecksAvailable[rcArea]++;
            }
        }
        totalChecksAvailable += areaChecksAvailable[rcArea];
    }

    StopPerformanceTimer(PT_RECALCULATE_AVAILABLE_CHECKS);
    SPDLOG_INFO("Recalculate Available Checks Time: {}ms",
                GetPerformanceTimer(PT_RECALCULATE_AVAILABLE_CHECKS).count());
}

void CheckTracker_LoadFromPreset(nlohmann::json info) {
    presetLoaded = true;
    presetPos = { info["pos"]["x"], info["pos"]["y"] };
    presetSize = { info["size"]["width"], info["size"]["height"] };
}

void CheckTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
}

static std::unordered_map<int32_t, const char*> windowType = { { TRACKER_WINDOW_FLOATING, "Floating" },
                                                               { TRACKER_WINDOW_WINDOW, "Window" } };
static std::unordered_map<int32_t, const char*> displayType = { { 0, "Always" }, { 1, "Combo Button Hold" } };
static std::unordered_map<int32_t, const char*> buttonStrings = {
    { TRACKER_COMBO_BUTTON_A, "A Button" },    { TRACKER_COMBO_BUTTON_B, "B Button" },
    { TRACKER_COMBO_BUTTON_C_UP, "C-Up" },     { TRACKER_COMBO_BUTTON_C_DOWN, "C-Down" },
    { TRACKER_COMBO_BUTTON_C_LEFT, "C-Left" }, { TRACKER_COMBO_BUTTON_C_RIGHT, "C-Right" },
    { TRACKER_COMBO_BUTTON_L, "L Button" },    { TRACKER_COMBO_BUTTON_Z, "Z Button" },
    { TRACKER_COMBO_BUTTON_R, "R Button" },    { TRACKER_COMBO_BUTTON_START, "Start" },
    { TRACKER_COMBO_BUTTON_D_UP, "D-Up" },     { TRACKER_COMBO_BUTTON_D_DOWN, "D-Down" },
    { TRACKER_COMBO_BUTTON_D_LEFT, "D-Left" }, { TRACKER_COMBO_BUTTON_D_RIGHT, "D-Right" }
};

void CheckTrackerSettingsWindow::DrawElement() {
    if (recalculateAvailable) {
        recalculateAvailable = false;
        RecalculateAvailableChecks();
    }
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 8.0f, 8.0f });
    if (ImGui::BeginTable("CheckTrackerSettingsTable", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV)) {
        ImGui::TableSetupColumn("General settings", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::TableSetupColumn("Section settings", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        UIWidgets::CVarColorPicker("BG Color", CVAR_TRACKER_CHECK("BgColor"), Color_Bg_Default, true,
                                   UIWidgets::ColorPickerResetButton | UIWidgets::ColorPickerRandomButton, THEME_COLOR);
        ImGui::PopItemWidth();

        UIWidgets::CVarCombobox("Window Type", CVAR_TRACKER_CHECK("WindowType"), windowType,
                                UIWidgets::ComboboxOptions()
                                    .LabelPosition(UIWidgets::LabelPositions::Far)
                                    .ComponentAlignment(UIWidgets::ComponentAlignments::Right)
                                    .Color(THEME_COLOR)
                                    .DefaultIndex(TRACKER_WINDOW_WINDOW));

        if (CVarGetInteger(CVAR_TRACKER_CHECK("WindowType"), TRACKER_WINDOW_WINDOW) == TRACKER_WINDOW_FLOATING) {
            UIWidgets::CVarCheckbox("Enable Dragging", CVAR_TRACKER_CHECK("Draggable"),
                                    UIWidgets::CheckboxOptions().Color(THEME_COLOR));
            UIWidgets::CVarCheckbox("Only enable while paused", CVAR_TRACKER_CHECK("ShowOnlyPaused"),
                                    UIWidgets::CheckboxOptions().Color(THEME_COLOR));
            UIWidgets::CVarCombobox("Display Mode", CVAR_TRACKER_CHECK("DisplayType"), displayType,
                                    UIWidgets::ComboboxOptions()
                                        .LabelPosition(UIWidgets::LabelPositions::Far)
                                        .ComponentAlignment(UIWidgets::ComponentAlignments::Right)
                                        .Color(THEME_COLOR)
                                        .DefaultIndex(0));
            if (CVarGetInteger(CVAR_TRACKER_CHECK("DisplayType"), TRACKER_DISPLAY_ALWAYS) ==
                TRACKER_DISPLAY_COMBO_BUTTON) {
                UIWidgets::CVarCombobox("Combo Button 1", CVAR_TRACKER_CHECK("ComboButton1"), buttonStrings,
                                        UIWidgets::ComboboxOptions()
                                            .LabelPosition(UIWidgets::LabelPositions::Far)
                                            .ComponentAlignment(UIWidgets::ComponentAlignments::Right)
                                            .Color(THEME_COLOR)
                                            .DefaultIndex(TRACKER_COMBO_BUTTON_L));
                UIWidgets::CVarCombobox("Combo Button 2", CVAR_TRACKER_CHECK("ComboButton2"), buttonStrings,
                                        UIWidgets::ComboboxOptions()
                                            .LabelPosition(UIWidgets::LabelPositions::Far)
                                            .ComponentAlignment(UIWidgets::ComponentAlignments::Right)
                                            .Color(THEME_COLOR)
                                            .DefaultIndex(TRACKER_COMBO_BUTTON_L));
            }
        }
        ImGui::BeginDisabled(CVarGetInteger(CVAR_SETTING("DisableChanges"), 0));
        UIWidgets::CVarCheckbox("Vanilla/MQ Dungeon Spoilers", CVAR_TRACKER_CHECK("MQSpoilers"),
                                UIWidgets::CheckboxOptions()
                                    .Tooltip("If enabled, Vanilla/MQ dungeons will show on the tracker immediately. "
                                             "Otherwise, Vanilla/MQ dungeon locations must be unlocked.")
                                    .Color(THEME_COLOR));
        ImGui::EndDisabled();
        if (UIWidgets::CVarCheckbox(
                "Hide unshuffled shop item checks", CVAR_TRACKER_CHECK("HideUnshuffledShopChecks"),
                UIWidgets::CheckboxOptions()
                    .Tooltip("If enabled, will prevent the tracker from displaying slots with non-shop-item shuffles.")
                    .Color(THEME_COLOR))) {
            hideShopUnshuffledChecks = CVarGetInteger(CVAR_TRACKER_CHECK("HideUnshuffledShopChecks"), 0);
            UpdateFilters();
        }
        if (UIWidgets::CVarCheckbox(
                "Always show gold skulltulas", CVAR_TRACKER_CHECK("AlwaysShowGSLocs"),
                UIWidgets::CheckboxOptions()
                    .Tooltip("If enabled, will show GS locations in the tracker regardless of tokensanity settings.")
                    .Color(THEME_COLOR))) {
            alwaysShowGS = !alwaysShowGS;
            UpdateFilters();
        }
        UIWidgets::CVarCheckbox("Show Logic", CVAR_TRACKER_CHECK("ShowLogic"),
                                UIWidgets::CheckboxOptions()
                                    .Tooltip("If enabled, will show a check's logic when hovering over it.")
                                    .Color(THEME_COLOR));
        ImGui::BeginDisabled(CVarGetInteger(CVAR_SETTING("DisableChanges"), 0));
        if (UIWidgets::CVarCheckbox("Enable Available Checks", CVAR_TRACKER_CHECK("EnableAvailableChecks"),
                                    UIWidgets::CheckboxOptions()
                                        .Tooltip("If enabled, will show the checks that are available to be collected "
                                                 "with your current progress.")
                                        .Color(THEME_COLOR))) {
            enableAvailableChecks = CVarGetInteger(CVAR_TRACKER_CHECK("EnableAvailableChecks"), 0);

            if (GameInteractor::IsSaveLoaded(true)) {
                RecalculateAvailableChecks();
            }
        }
        ImGui::EndDisabled();

        // Filtering settings
        UIWidgets::PaddedSeparator();
        UIWidgets::CVarCheckbox(
            "Filter Empty Areas", CVAR_TRACKER_CHECK("HideFilteredAreas"),
            UIWidgets::CheckboxOptions()
                .Tooltip("If enabled, will hide area headers that have no locations matching filter")
                .Color(THEME_COLOR)
                .DefaultValue(true));

        ImGui::TableNextColumn();

        CheckTracker::ImGuiDrawTwoColorPickerSection("Area Incomplete", CVAR_TRACKER_CHECK("AreaIncomplete.MainColor"),
                                                     CVAR_TRACKER_CHECK("AreaIncomplete.ExtraColor"),
                                                     Color_Area_Incomplete_Main, Color_Area_Incomplete_Extra,
                                                     Color_Main_Default, Color_Area_Incomplete_Extra_Default,
                                                     CVAR_TRACKER_CHECK("AreaIncomplete.Hide"), "", THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection("Area Complete", CVAR_TRACKER_CHECK("AreaComplete.MainColor"),
                                                     CVAR_TRACKER_CHECK("AreaComplete.ExtraColor"),
                                                     Color_Area_Complete_Main, Color_Area_Complete_Extra,
                                                     Color_Main_Default, Color_Area_Complete_Extra_Default,
                                                     CVAR_TRACKER_CHECK("AreaComplete.Hide"), "", THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection(
            "Unchecked", CVAR_TRACKER_CHECK("Unchecked.MainColor"), CVAR_TRACKER_CHECK("Unchecked.ExtraColor"),
            Color_Unchecked_Main, Color_Unchecked_Extra, Color_Main_Default, Color_Unchecked_Extra_Default,
            CVAR_TRACKER_CHECK("Unchecked.Hide"), "Checks you have not interacted with at all.", THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection(
            "Skipped", CVAR_TRACKER_CHECK("Skipped.MainColor"), CVAR_TRACKER_CHECK("Skipped.ExtraColor"),
            Color_Skipped_Main, Color_Skipped_Extra, Color_Main_Default, Color_Skipped_Extra_Default,
            CVAR_TRACKER_CHECK("Skipped.Hide"), "", THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection(
            "Seen", CVAR_TRACKER_CHECK("Seen.MainColor"), CVAR_TRACKER_CHECK("Seen.ExtraColor"), Color_Seen_Main,
            Color_Seen_Extra, Color_Main_Default, Color_Seen_Extra_Default, CVAR_TRACKER_CHECK("Seen.Hide"),
            "Used for shops. Shows item names for shop slots when walking in, and prices when highlighting them in buy "
            "mode.",
            THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection(
            "Scummed", CVAR_TRACKER_CHECK("Scummed.MainColor"), CVAR_TRACKER_CHECK("Scummed.ExtraColor"),
            Color_Scummed_Main, Color_Scummed_Extra, Color_Main_Default, Color_Scummed_Extra_Default,
            CVAR_TRACKER_CHECK("Scummed.Hide"),
            "Checks you collect, but then reload before saving so you no longer have them.", THEME_COLOR);
        // CheckTracker::ImGuiDrawTwoColorPickerSection("Hinted (WIP)",     CVAR_TRACKER_CHECK("Hinted.MainColor"),
        // CVAR_TRACKER_CHECK("Hinted.ExtraColor"),          Color_Hinted_Main,            Color_Hinted_Extra,
        // Color_Main_Default, Color_Hinted_Extra_Default,          CVAR_TRACKER_CHECK("Hinted.Hide"),         "",
        // THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection(
            "Collected", CVAR_TRACKER_CHECK("Collected.MainColor"), CVAR_TRACKER_CHECK("Collected.ExtraColor"),
            Color_Collected_Main, Color_Collected_Extra, Color_Main_Default, Color_Collected_Extra_Default,
            CVAR_TRACKER_CHECK("Collected.Hide"), "Checks you have collected without saving or reloading yet.",
            THEME_COLOR);
        CheckTracker::ImGuiDrawTwoColorPickerSection(
            "Saved", CVAR_TRACKER_CHECK("Saved.MainColor"), CVAR_TRACKER_CHECK("Saved.ExtraColor"), Color_Saved_Main,
            Color_Saved_Extra, Color_Main_Default, Color_Saved_Extra_Default, CVAR_TRACKER_CHECK("Saved.Hide"),
            "Checks that you saved the game while having collected.", THEME_COLOR);

        ImGui::PopStyleVar(1);
    }
    ImGui::EndTable();
}

void CheckTrackerWindow::InitElement() {
    SaveManager::Instance->AddInitFunction(InitTrackerData);
    sectionId = SaveManager::Instance->AddSaveFunction("trackerData", 1, SaveFile, true, SECTION_PARENT_NONE);
    SaveManager::Instance->AddLoadFunction("trackerData", 1, LoadFile);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(CheckTrackerLoadGame);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnExitGame>([](uint32_t fileNum) { Teardown(); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnItemReceive>(CheckTrackerItemReceive);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnTransitionEnd>(CheckTrackerTransition);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnShopSlotChange>(CheckTrackerShopSlotChange);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneFlagSet>(CheckTrackerSceneFlagSet);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnFlagSet>(CheckTrackerFlagSet);
}

void CheckTrackerWindow::UpdateElement() {
}
} // namespace CheckTracker
