#include "randomizer_item_tracker.h"
#include "soh/util.h"
#include "soh/OTRGlobals.h"
#include "soh/cvar_prefixes.h"
#include "soh/SaveManager.h"
#include "soh/ResourceManagerHelpers.h"
#include "soh/SohGui/UIWidgets.hpp"
#include "soh/SohGui/SohGui.hpp"
#include "randomizerTypes.h"

#include <map>
#include <string>
#include <vector>
#include <libultraship/libultraship.h>
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "randomizer_check_tracker.h"
#include <algorithm>

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;

#include "textures/icon_item_static/icon_item_static.h"
#include "textures/icon_item_24_static/icon_item_24_static.h"
}

void DrawEquip(ItemTrackerItem item);
void DrawItem(ItemTrackerItem item);
void DrawDungeonItem(ItemTrackerItem item);
void DrawBottle(ItemTrackerItem item);
void DrawQuest(ItemTrackerItem item);
void DrawSong(ItemTrackerItem item);

int itemTrackerSectionId;

using namespace UIWidgets;

bool shouldUpdateVectors = true;

std::vector<ItemTrackerItem> mainWindowItems = {};

std::vector<ItemTrackerItem> inventoryItems = {
    ITEM_TRACKER_ITEM(ITEM_STICK, 0, DrawItem),       ITEM_TRACKER_ITEM(ITEM_NUT, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_BOMB, 0, DrawItem),        ITEM_TRACKER_ITEM(ITEM_BOW, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_ARROW_FIRE, 0, DrawItem),  ITEM_TRACKER_ITEM(ITEM_DINS_FIRE, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_SLINGSHOT, 0, DrawItem),   ITEM_TRACKER_ITEM(ITEM_OCARINA_FAIRY, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_BOMBCHU, 0, DrawItem),     ITEM_TRACKER_ITEM(ITEM_HOOKSHOT, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_ARROW_ICE, 0, DrawItem),   ITEM_TRACKER_ITEM(ITEM_FARORES_WIND, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_BOOMERANG, 0, DrawItem),   ITEM_TRACKER_ITEM(ITEM_LENS, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_BEAN, 0, DrawItem),        ITEM_TRACKER_ITEM(ITEM_HAMMER, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_ARROW_LIGHT, 0, DrawItem), ITEM_TRACKER_ITEM(ITEM_NAYRUS_LOVE, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_BOTTLE, 0, DrawBottle),    ITEM_TRACKER_ITEM(ITEM_BOTTLE, 1, DrawBottle),
    ITEM_TRACKER_ITEM(ITEM_BOTTLE, 2, DrawBottle),    ITEM_TRACKER_ITEM(ITEM_BOTTLE, 3, DrawBottle),
    ITEM_TRACKER_ITEM(ITEM_POCKET_EGG, 0, DrawItem),  ITEM_TRACKER_ITEM(ITEM_MASK_KEATON, 0, DrawItem),
};

std::vector<ItemTrackerItem> equipmentItems = {
    ITEM_TRACKER_ITEM(ITEM_SWORD_KOKIRI, 1 << 0, DrawEquip),  ITEM_TRACKER_ITEM(ITEM_SWORD_MASTER, 1 << 1, DrawEquip),
    ITEM_TRACKER_ITEM(ITEM_SWORD_BGS, 1 << 2, DrawEquip),     ITEM_TRACKER_ITEM(ITEM_TUNIC_KOKIRI, 1 << 8, DrawEquip),
    ITEM_TRACKER_ITEM(ITEM_TUNIC_GORON, 1 << 9, DrawEquip),   ITEM_TRACKER_ITEM(ITEM_TUNIC_ZORA, 1 << 10, DrawEquip),
    ITEM_TRACKER_ITEM(ITEM_SHIELD_DEKU, 1 << 4, DrawEquip),   ITEM_TRACKER_ITEM(ITEM_SHIELD_HYLIAN, 1 << 5, DrawEquip),
    ITEM_TRACKER_ITEM(ITEM_SHIELD_MIRROR, 1 << 6, DrawEquip), ITEM_TRACKER_ITEM(ITEM_BOOTS_KOKIRI, 1 << 12, DrawEquip),
    ITEM_TRACKER_ITEM(ITEM_BOOTS_IRON, 1 << 13, DrawEquip),   ITEM_TRACKER_ITEM(ITEM_BOOTS_HOVER, 1 << 14, DrawEquip),
};

std::vector<ItemTrackerItem> miscItems = {
    ITEM_TRACKER_ITEM(ITEM_BRACELET, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_SCALE_SILVER, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_WALLET_ADULT, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_HEART_CONTAINER, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_HEART_PIECE, 0, DrawItem),
    ITEM_TRACKER_ITEM(ITEM_MAGIC_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM(QUEST_GERUDO_CARD, 1 << 22, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_SKULL_TOKEN, 1 << 23, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_STONE_OF_AGONY, 1 << 21, DrawQuest),
};

std::vector<ItemTrackerItem> dungeonRewardStones = {
    ITEM_TRACKER_ITEM(QUEST_KOKIRI_EMERALD, 1 << 18, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_GORON_RUBY, 1 << 19, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_ZORA_SAPPHIRE, 1 << 20, DrawQuest),
};

std::vector<ItemTrackerItem> dungeonRewardMedallions = {
    ITEM_TRACKER_ITEM(QUEST_MEDALLION_FOREST, 1 << 0, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_MEDALLION_FIRE, 1 << 1, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_MEDALLION_WATER, 1 << 2, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_MEDALLION_SPIRIT, 1 << 3, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_MEDALLION_SHADOW, 1 << 4, DrawQuest),
    ITEM_TRACKER_ITEM(QUEST_MEDALLION_LIGHT, 1 << 5, DrawQuest),
};

std::vector<ItemTrackerItem> dungeonRewards = {};

std::vector<ItemTrackerItem> songItems = {
    ITEM_TRACKER_ITEM(QUEST_SONG_LULLABY, 0, DrawSong),  ITEM_TRACKER_ITEM(QUEST_SONG_EPONA, 0, DrawSong),
    ITEM_TRACKER_ITEM(QUEST_SONG_SARIA, 0, DrawSong),    ITEM_TRACKER_ITEM(QUEST_SONG_SUN, 0, DrawSong),
    ITEM_TRACKER_ITEM(QUEST_SONG_TIME, 0, DrawSong),     ITEM_TRACKER_ITEM(QUEST_SONG_STORMS, 0, DrawSong),
    ITEM_TRACKER_ITEM(QUEST_SONG_MINUET, 0, DrawSong),   ITEM_TRACKER_ITEM(QUEST_SONG_BOLERO, 0, DrawSong),
    ITEM_TRACKER_ITEM(QUEST_SONG_SERENADE, 0, DrawSong), ITEM_TRACKER_ITEM(QUEST_SONG_REQUIEM, 0, DrawSong),
    ITEM_TRACKER_ITEM(QUEST_SONG_NOCTURNE, 0, DrawSong), ITEM_TRACKER_ITEM(QUEST_SONG_PRELUDE, 0, DrawSong),
};

std::vector<ItemTrackerItem> gregItems = {
    ITEM_TRACKER_ITEM(ITEM_RUPEE_GREEN, 0, DrawItem),
};

std::vector<ItemTrackerItem> triforcePieces = {
    ITEM_TRACKER_ITEM(RG_TRIFORCE_PIECE, 0, DrawItem),
};

std::vector<ItemTrackerItem> bossSoulItems = {
    ITEM_TRACKER_ITEM(RG_GOHMA_SOUL, 0, DrawItem),       ITEM_TRACKER_ITEM(RG_KING_DODONGO_SOUL, 0, DrawItem),
    ITEM_TRACKER_ITEM(RG_BARINADE_SOUL, 0, DrawItem),    ITEM_TRACKER_ITEM(RG_PHANTOM_GANON_SOUL, 0, DrawItem),
    ITEM_TRACKER_ITEM(RG_VOLVAGIA_SOUL, 0, DrawItem),    ITEM_TRACKER_ITEM(RG_MORPHA_SOUL, 0, DrawItem),
    ITEM_TRACKER_ITEM(RG_BONGO_BONGO_SOUL, 0, DrawItem), ITEM_TRACKER_ITEM(RG_TWINROVA_SOUL, 0, DrawItem),
    ITEM_TRACKER_ITEM(RG_GANON_SOUL, 0, DrawItem),
};

std::vector<ItemTrackerItem> ocarinaButtonItems = {
    // Hack for right now, just gonna draw ocarina buttons as ocarinas.
    // Will replace with other macro once we have a custom texture
    ITEM_TRACKER_ITEM_CUSTOM(RG_OCARINA_A_BUTTON, ITEM_OCARINA_TIME, ITEM_OCARINA_TIME, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_OCARINA_C_UP_BUTTON, ITEM_OCARINA_TIME, ITEM_OCARINA_TIME, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_OCARINA_C_DOWN_BUTTON, ITEM_OCARINA_TIME, ITEM_OCARINA_TIME, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_OCARINA_C_LEFT_BUTTON, ITEM_OCARINA_TIME, ITEM_OCARINA_TIME, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_OCARINA_C_RIGHT_BUTTON, ITEM_OCARINA_TIME, ITEM_OCARINA_TIME, 0, DrawItem),
};

std::vector<ItemTrackerItem> overworldKeyItems = {
    // Hack for right now, just gonna overworld keys as dungeon keys.
    // Will replace with other macro once we have a custom texture
    ITEM_TRACKER_ITEM_CUSTOM(RG_GUARD_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_MARKET_BAZAAR_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_MARKET_POTION_SHOP_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_MASK_SHOP_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_MARKET_SHOOTING_GALLERY_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_BOMBCHU_BOWLING_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_TREASURE_CHEST_GAME_BUILDING_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_BOMBCHU_SHOP_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_RICHARDS_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_ALLEY_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_KAK_BAZAAR_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_KAK_POTION_SHOP_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_BOSS_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_GRANNYS_POTION_SHOP_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_SKULLTULA_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_IMPAS_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_WINDMILL_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_KAK_SHOOTING_GALLERY_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_DAMPES_HUT_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_TALONS_HOUSE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_STABLES_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_BACK_TOWER_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_HYLIA_LAB_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
    ITEM_TRACKER_ITEM_CUSTOM(RG_FISHING_HOLE_KEY, ITEM_KEY_SMALL, ITEM_KEY_SMALL, 0, DrawItem),
};

std::vector<ItemTrackerItem> fishingPoleItems = { ITEM_TRACKER_ITEM(ITEM_FISHING_POLE, 0, DrawItem) };

std::vector<ItemTrackerDungeon> itemTrackerDungeonsWithMapsHorizontal = {
    { SCENE_DEKU_TREE, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_DODONGOS_CAVERN, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_JABU_JABU, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_FOREST_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_FIRE_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_WATER_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_SPIRIT_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_SHADOW_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_INSIDE_GANONS_CASTLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_BOTTOM_OF_THE_WELL, { ITEM_KEY_SMALL, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_ICE_CAVERN, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_GERUDO_TRAINING_GROUND, { ITEM_KEY_SMALL } },
};

std::vector<ItemTrackerDungeon> itemTrackerDungeonsHorizontal = {
    { SCENE_FOREST_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_FIRE_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_WATER_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_SPIRIT_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_SHADOW_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_INSIDE_GANONS_CASTLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_BOTTOM_OF_THE_WELL, { ITEM_KEY_SMALL } },
    { SCENE_GERUDO_TRAINING_GROUND, { ITEM_KEY_SMALL } },
};

std::vector<ItemTrackerDungeon> itemTrackerDungeonsWithMapsCompact = {
    { SCENE_FOREST_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_FIRE_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_WATER_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_SPIRIT_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_SHADOW_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_BOTTOM_OF_THE_WELL, { ITEM_KEY_SMALL, ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_DEKU_TREE, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_DODONGOS_CAVERN, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_JABU_JABU, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_ICE_CAVERN, { ITEM_DUNGEON_MAP, ITEM_COMPASS } },
    { SCENE_INSIDE_GANONS_CASTLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_GERUDO_TRAINING_GROUND, { ITEM_KEY_SMALL } },
};

std::vector<ItemTrackerDungeon> itemTrackerDungeonsCompact = {
    { SCENE_FOREST_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_FIRE_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_WATER_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_SPIRIT_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_SHADOW_TEMPLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_INSIDE_GANONS_CASTLE, { ITEM_KEY_SMALL, ITEM_KEY_BOSS } },
    { SCENE_BOTTOM_OF_THE_WELL, { ITEM_KEY_SMALL } },
    { SCENE_GERUDO_TRAINING_GROUND, { ITEM_KEY_SMALL } },
    { SCENE_THIEVES_HIDEOUT, { ITEM_KEY_SMALL } },
};

std::map<uint16_t, std::string> itemTrackerDungeonShortNames = {
    { SCENE_FOREST_TEMPLE, "FRST" },   { SCENE_FIRE_TEMPLE, "FIRE" },           { SCENE_WATER_TEMPLE, "WATR" },
    { SCENE_SPIRIT_TEMPLE, "SPRT" },   { SCENE_SHADOW_TEMPLE, "SHDW" },         { SCENE_BOTTOM_OF_THE_WELL, "BOTW" },
    { SCENE_DEKU_TREE, "DEKU" },       { SCENE_DODONGOS_CAVERN, "DCVN" },       { SCENE_JABU_JABU, "JABU" },
    { SCENE_ICE_CAVERN, "ICE" },       { SCENE_INSIDE_GANONS_CASTLE, "GANON" }, { SCENE_GERUDO_TRAINING_GROUND, "GTG" },
    { SCENE_THIEVES_HIDEOUT, "HIDE" },
};

std::map<uint16_t, std::string> itemTrackerBossShortNames = {
    { RG_GOHMA_SOUL, "GOHMA" },       { RG_KING_DODONGO_SOUL, "KD" }, { RG_BARINADE_SOUL, "BARI" },
    { RG_PHANTOM_GANON_SOUL, "PG" },  { RG_VOLVAGIA_SOUL, "VOLV" },   { RG_MORPHA_SOUL, "MORPH" },
    { RG_BONGO_BONGO_SOUL, "BONGO" }, { RG_TWINROVA_SOUL, "TWIN" },   { RG_GANON_SOUL, "GANON" },
};

std::map<uint16_t, std::string> itemTrackerOcarinaButtonShortNames = {
    { RG_OCARINA_A_BUTTON, "A" },        { RG_OCARINA_C_UP_BUTTON, "C-U" },    { RG_OCARINA_C_DOWN_BUTTON, "C-D" },
    { RG_OCARINA_C_LEFT_BUTTON, "C-L" }, { RG_OCARINA_C_RIGHT_BUTTON, "C-R" },
};

std::map<uint16_t, std::string> itemTrackerOverworldKeyShortNames = {
    { RG_GUARD_HOUSE_KEY, "GUARD" },
    { RG_MARKET_BAZAAR_KEY, "MKBAZ" },
    { RG_MARKET_POTION_SHOP_KEY, "MKPOT" },
    { RG_MASK_SHOP_KEY, "MASK" },
    { RG_MARKET_SHOOTING_GALLERY_KEY, "MKSHO" },
    { RG_BOMBCHU_BOWLING_KEY, "BOWL" },
    { RG_TREASURE_CHEST_GAME_BUILDING_KEY, "TREASU" },
    { RG_BOMBCHU_SHOP_KEY, "CHUSHO" },
    { RG_RICHARDS_HOUSE_KEY, "RICH" },
    { RG_ALLEY_HOUSE_KEY, "ALLEY" },
    { RG_KAK_BAZAAR_KEY, "KAKBAZ" },
    { RG_KAK_POTION_SHOP_KEY, "KAKPO" },
    { RG_BOSS_HOUSE_KEY, "BOSS" },
    { RG_GRANNYS_POTION_SHOP_KEY, "GRANNY" },
    { RG_SKULLTULA_HOUSE_KEY, "SKULL" },
    { RG_IMPAS_HOUSE_KEY, "IMPAS" },
    { RG_WINDMILL_KEY, "WIND" },
    { RG_KAK_SHOOTING_GALLERY_KEY, "KAKSHO" },
    { RG_DAMPES_HUT_KEY, "DAMPES" },
    { RG_TALONS_HOUSE_KEY, "TALONS" },
    { RG_STABLES_KEY, "STABLE" },
    { RG_BACK_TOWER_KEY, "TOWER" },
    { RG_HYLIA_LAB_KEY, "LAB" },
    { RG_FISHING_HOLE_KEY, "FISH" },
};

std::vector<ItemTrackerItem> dungeonItems = {};

std::unordered_map<uint32_t, ItemTrackerItem> actualItemTrackerItemMap = {
    { ITEM_BOTTLE, ITEM_TRACKER_ITEM(ITEM_BOTTLE, 0, DrawItem) },
    { ITEM_BIG_POE, ITEM_TRACKER_ITEM(ITEM_BIG_POE, 0, DrawItem) },
    { ITEM_BLUE_FIRE, ITEM_TRACKER_ITEM(ITEM_BLUE_FIRE, 0, DrawItem) },
    { ITEM_BUG, ITEM_TRACKER_ITEM(ITEM_BUG, 0, DrawItem) },
    { ITEM_FAIRY, ITEM_TRACKER_ITEM(ITEM_FAIRY, 0, DrawItem) },
    { ITEM_FISH, ITEM_TRACKER_ITEM(ITEM_FISH, 0, DrawItem) },
    { ITEM_POTION_GREEN, ITEM_TRACKER_ITEM(ITEM_POTION_GREEN, 0, DrawItem) },
    { ITEM_POE, ITEM_TRACKER_ITEM(ITEM_POE, 0, DrawItem) },
    { ITEM_POTION_RED, ITEM_TRACKER_ITEM(ITEM_POTION_RED, 0, DrawItem) },
    { ITEM_POTION_BLUE, ITEM_TRACKER_ITEM(ITEM_POTION_BLUE, 0, DrawItem) },
    { ITEM_MILK_BOTTLE, ITEM_TRACKER_ITEM(ITEM_MILK_BOTTLE, 0, DrawItem) },
    { ITEM_MILK_HALF, ITEM_TRACKER_ITEM(ITEM_MILK_HALF, 0, DrawItem) },
    { ITEM_LETTER_RUTO, ITEM_TRACKER_ITEM(ITEM_LETTER_RUTO, 0, DrawItem) },

    { ITEM_HOOKSHOT, ITEM_TRACKER_ITEM(ITEM_HOOKSHOT, 0, DrawItem) },
    { ITEM_LONGSHOT, ITEM_TRACKER_ITEM(ITEM_LONGSHOT, 0, DrawItem) },

    { ITEM_OCARINA_FAIRY, ITEM_TRACKER_ITEM(ITEM_OCARINA_FAIRY, 0, DrawItem) },
    { ITEM_OCARINA_TIME, ITEM_TRACKER_ITEM(ITEM_OCARINA_TIME, 0, DrawItem) },

    { ITEM_MAGIC_SMALL, ITEM_TRACKER_ITEM(ITEM_MAGIC_SMALL, 0, DrawItem) },
    { ITEM_MAGIC_LARGE, ITEM_TRACKER_ITEM(ITEM_MAGIC_LARGE, 0, DrawItem) },

    { ITEM_WALLET_ADULT, ITEM_TRACKER_ITEM(ITEM_WALLET_ADULT, 0, DrawItem) },
    { ITEM_WALLET_GIANT, ITEM_TRACKER_ITEM(ITEM_WALLET_GIANT, 0, DrawItem) },

    { ITEM_BRACELET, ITEM_TRACKER_ITEM(ITEM_BRACELET, 0, DrawItem) },
    { ITEM_GAUNTLETS_SILVER, ITEM_TRACKER_ITEM(ITEM_GAUNTLETS_SILVER, 0, DrawItem) },
    { ITEM_GAUNTLETS_GOLD, ITEM_TRACKER_ITEM(ITEM_GAUNTLETS_GOLD, 0, DrawItem) },

    { ITEM_SCALE_SILVER, ITEM_TRACKER_ITEM(ITEM_SCALE_SILVER, 0, DrawItem) },
    { ITEM_SCALE_GOLDEN, ITEM_TRACKER_ITEM(ITEM_SCALE_GOLDEN, 0, DrawItem) },

    { ITEM_WEIRD_EGG, ITEM_TRACKER_ITEM(ITEM_WEIRD_EGG, 0, DrawItem) },
    { ITEM_CHICKEN, ITEM_TRACKER_ITEM(ITEM_CHICKEN, 0, DrawItem) },
    { ITEM_LETTER_ZELDA, ITEM_TRACKER_ITEM(ITEM_LETTER_ZELDA, 0, DrawItem) },
    { ITEM_MASK_KEATON, ITEM_TRACKER_ITEM(ITEM_MASK_KEATON, 0, DrawItem) },
    { ITEM_MASK_SKULL, ITEM_TRACKER_ITEM(ITEM_MASK_SKULL, 0, DrawItem) },
    { ITEM_MASK_SPOOKY, ITEM_TRACKER_ITEM(ITEM_MASK_SPOOKY, 0, DrawItem) },
    { ITEM_MASK_BUNNY, ITEM_TRACKER_ITEM(ITEM_MASK_BUNNY, 0, DrawItem) },
    { ITEM_MASK_GORON, ITEM_TRACKER_ITEM(ITEM_MASK_GORON, 0, DrawItem) },
    { ITEM_MASK_ZORA, ITEM_TRACKER_ITEM(ITEM_MASK_ZORA, 0, DrawItem) },
    { ITEM_MASK_GERUDO, ITEM_TRACKER_ITEM(ITEM_MASK_GERUDO, 0, DrawItem) },
    { ITEM_MASK_TRUTH, ITEM_TRACKER_ITEM(ITEM_MASK_TRUTH, 0, DrawItem) },
    { ITEM_SOLD_OUT, ITEM_TRACKER_ITEM(ITEM_SOLD_OUT, 0, DrawItem) },

    { ITEM_POCKET_EGG, ITEM_TRACKER_ITEM(ITEM_POCKET_EGG, 0, DrawItem) },
    { ITEM_POCKET_CUCCO, ITEM_TRACKER_ITEM(ITEM_POCKET_CUCCO, 0, DrawItem) },
    { ITEM_COJIRO, ITEM_TRACKER_ITEM(ITEM_COJIRO, 0, DrawItem) },
    { ITEM_ODD_MUSHROOM, ITEM_TRACKER_ITEM(ITEM_ODD_MUSHROOM, 0, DrawItem) },
    { ITEM_ODD_POTION, ITEM_TRACKER_ITEM(ITEM_ODD_POTION, 0, DrawItem) },
    { ITEM_SAW, ITEM_TRACKER_ITEM(ITEM_SAW, 0, DrawItem) },
    { ITEM_SWORD_BROKEN, ITEM_TRACKER_ITEM(ITEM_SWORD_BROKEN, 0, DrawItem) },
    { ITEM_PRESCRIPTION, ITEM_TRACKER_ITEM(ITEM_PRESCRIPTION, 0, DrawItem) },
    { ITEM_FROG, ITEM_TRACKER_ITEM(ITEM_FROG, 0, DrawItem) },
    { ITEM_EYEDROPS, ITEM_TRACKER_ITEM(ITEM_EYEDROPS, 0, DrawItem) },
    { ITEM_CLAIM_CHECK, ITEM_TRACKER_ITEM(ITEM_CLAIM_CHECK, 0, DrawItem) },
};

std::vector<uint32_t> buttonMap = {
    BTN_A, BTN_B, BTN_CUP,   BTN_CDOWN, BTN_CLEFT, BTN_CRIGHT, BTN_L,
    BTN_Z, BTN_R, BTN_START, BTN_DUP,   BTN_DDOWN, BTN_DLEFT,  BTN_DRIGHT,
};

typedef enum {
    ITEM_TRACKER_NUMBER_NONE,
    ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY,
    ITEM_TRACKER_NUMBER_CURRENT_AMMO_ONLY,
    ITEM_TRACKER_NUMBER_CAPACITY,
    ITEM_TRACKER_NUMBER_AMMO,
} ItemTrackerNumberOption;

typedef enum {
    KEYS_COLLECTED_MAX,
    KEYS_CURRENT_COLLECTED_MAX,
    KEYS_CURRENT_MAX,
} ItemTrackerKeysNumberOption;

typedef enum {
    TRIFORCE_PIECE_COLLECTED_REQUIRED,
    TRIFORCE_PIECE_COLLECTED_REQUIRED_MAX,
} ItemTrackerTriforcePieceNumberOption;

typedef enum {
    SECTION_DISPLAY_HIDDEN,
    SECTION_DISPLAY_MAIN_WINDOW,
    SECTION_DISPLAY_SEPARATE,
} ItemTrackerDisplayType;

typedef enum {
    SECTION_DISPLAY_EXTENDED_HIDDEN,
    SECTION_DISPLAY_EXTENDED_MAIN_WINDOW,
    SECTION_DISPLAY_EXTENDED_MISC_WINDOW,
    SECTION_DISPLAY_EXTENDED_SEPARATE
} ItemTrackerExtendedDisplayType;

typedef enum {
    SECTION_DISPLAY_MINIMAL_HIDDEN,
    SECTION_DISPLAY_MINIMAL_SEPARATE,
} ItemTrackerMinimalDisplayType;

struct ItemTrackerNumbers {
    int currentCapacity;
    int maxCapacity;
    int currentAmmo;
};

static ImVector<char> itemTrackerNotes;
uint32_t notesIdleFrames = 0;
bool notesNeedSave = false;
const uint32_t notesMaxIdleFrames = 40; // two seconds of game time, since OnGameFrameUpdate is used to tick

static bool presetLoaded = false;
static std::unordered_map<std::string, ImVec2> presetPos;
static std::unordered_map<std::string, ImVec2> presetSize;

void ItemTrackerOnFrame() {
    if (notesNeedSave && notesIdleFrames <= notesMaxIdleFrames) {
        notesIdleFrames++;
    }
}

bool IsValidSaveFile() {
    bool validSave = gSaveContext.fileNum >= 0 && gSaveContext.fileNum <= 2;
    return validSave;
}

bool HasSong(ItemTrackerItem item) {
    return GameInteractor::IsSaveLoaded() ? ((1 << item.id) & gSaveContext.inventory.questItems) : false;
}

bool HasQuestItem(ItemTrackerItem item) {
    return GameInteractor::IsSaveLoaded() ? (item.data & gSaveContext.inventory.questItems) : false;
}

bool HasEquipment(ItemTrackerItem item) {
    return GameInteractor::IsSaveLoaded() ? (item.data & gSaveContext.inventory.equipment) : false;
}

void ItemTracker_LoadFromPreset(nlohmann::json trackerInfo) {
    presetLoaded = true;
    for (auto window : itemTrackerWindowIDs) {
        if (trackerInfo.contains(window)) {
            presetPos[window] = { trackerInfo[window]["pos"]["x"], trackerInfo[window]["pos"]["y"] };
            presetSize[window] = { trackerInfo[window]["size"]["width"], trackerInfo[window]["size"]["height"] };
        }
    }
}

ItemTrackerNumbers GetItemCurrentAndMax(ItemTrackerItem item) {
    ItemTrackerNumbers result;
    result.currentCapacity = 0;
    result.maxCapacity = 0;
    result.currentAmmo = 0;

    switch (item.id) {
        case ITEM_STICK:
            result.currentCapacity = CUR_CAPACITY(UPG_STICKS);
            result.maxCapacity = 30;
            result.currentAmmo = AMMO(ITEM_STICK);
            break;
        case ITEM_NUT:
            result.currentCapacity = CUR_CAPACITY(UPG_NUTS);
            result.maxCapacity = 40;
            result.currentAmmo = AMMO(ITEM_NUT);
            break;
        case ITEM_BOMB:
            result.currentCapacity = CUR_CAPACITY(UPG_BOMB_BAG);
            result.maxCapacity = 40;
            result.currentAmmo = AMMO(ITEM_BOMB);
            break;
        case ITEM_BOW:
            result.currentCapacity = CUR_CAPACITY(UPG_QUIVER);
            result.maxCapacity = 50;
            result.currentAmmo = AMMO(ITEM_BOW);
            break;
        case ITEM_SLINGSHOT:
            result.currentCapacity = CUR_CAPACITY(UPG_BULLET_BAG);
            result.maxCapacity = 50;
            result.currentAmmo = AMMO(ITEM_SLINGSHOT);
            break;
        case ITEM_WALLET_ADULT:
        case ITEM_WALLET_GIANT:
            result.currentCapacity =
                IS_RANDO && !Flags_GetRandomizerInf(RAND_INF_HAS_WALLET) ? 0 : CUR_CAPACITY(UPG_WALLET);
            result.maxCapacity =
                OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_INCLUDE_TYCOON_WALLET) ? 999 : 500;
            result.currentAmmo = gSaveContext.rupees;
            break;
        case ITEM_BOMBCHU:
            result.currentCapacity = INV_CONTENT(ITEM_BOMBCHU) == ITEM_BOMBCHU ? 50 : 0;
            result.maxCapacity = 50;
            result.currentAmmo = AMMO(ITEM_BOMBCHU);
            break;
        case ITEM_BEAN:
            result.currentCapacity = INV_CONTENT(ITEM_BEAN) == ITEM_BEAN ? 10 : 0;
            result.maxCapacity = 10;
            result.currentAmmo = AMMO(ITEM_BEAN);
            break;
        case QUEST_SKULL_TOKEN:
            result.maxCapacity = result.currentCapacity = 100;
            result.currentAmmo = gSaveContext.inventory.gsTokens;
            break;
        case ITEM_HEART_CONTAINER:
            result.maxCapacity = result.currentCapacity = 8;
            result.currentAmmo = gSaveContext.ship.stats.heartContainers;
            break;
        case ITEM_HEART_PIECE:
            result.maxCapacity = result.currentCapacity = 36;
            result.currentAmmo = gSaveContext.ship.stats.heartPieces;
            break;
        case ITEM_KEY_SMALL:
            // Though the ammo/capacity naming doesn't really make sense for keys, we are
            // hijacking the same system to display key counts as there are enough similarities
            result.currentAmmo = MAX(gSaveContext.inventory.dungeonKeys[item.data], 0);
            result.currentCapacity = gSaveContext.ship.stats.dungeonKeys[item.data];
            switch (item.data) {
                case SCENE_FOREST_TEMPLE:
                    result.maxCapacity = FOREST_TEMPLE_SMALL_KEY_MAX;
                    break;
                case SCENE_FIRE_TEMPLE:
                    result.maxCapacity = FIRE_TEMPLE_SMALL_KEY_MAX;
                    break;
                case SCENE_WATER_TEMPLE:
                    result.maxCapacity = WATER_TEMPLE_SMALL_KEY_MAX;
                    break;
                case SCENE_SPIRIT_TEMPLE:
                    result.maxCapacity = SPIRIT_TEMPLE_SMALL_KEY_MAX;
                    break;
                case SCENE_SHADOW_TEMPLE:
                    result.maxCapacity = SHADOW_TEMPLE_SMALL_KEY_MAX;
                    break;
                case SCENE_BOTTOM_OF_THE_WELL:
                    result.maxCapacity = BOTTOM_OF_THE_WELL_SMALL_KEY_MAX;
                    break;
                case SCENE_GERUDO_TRAINING_GROUND:
                    result.maxCapacity = GERUDO_TRAINING_GROUND_SMALL_KEY_MAX;
                    break;
                case SCENE_THIEVES_HIDEOUT:
                    result.maxCapacity = GERUDO_FORTRESS_SMALL_KEY_MAX;
                    break;
                case SCENE_INSIDE_GANONS_CASTLE:
                    result.maxCapacity = GANONS_CASTLE_SMALL_KEY_MAX;
                    break;
            }
            break;
    }

    return result;
}

#define IM_COL_WHITE IM_COL32(255, 255, 255, 255)
#define IM_COL_RED IM_COL32(255, 0, 0, 255)
#define IM_COL_GREEN IM_COL32(0, 255, 0, 255)
#define IM_COL_GRAY IM_COL32(155, 155, 155, 255)
#define IM_COL_PURPLE IM_COL32(180, 90, 200, 255)
#define IM_COL_LIGHT_YELLOW IM_COL32(255, 255, 130, 255)

void DrawItemCount(ItemTrackerItem item, bool hideMax) {
    if (!GameInteractor::IsSaveLoaded()) {
        return;
    }
    int iconSize = CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36);
    int textSize = CVarGetInteger(CVAR_TRACKER_ITEM("TextSize"), 13);
    ItemTrackerNumbers currentAndMax = GetItemCurrentAndMax(item);
    ImVec2 p = ImGui::GetCursorScreenPos();
    int32_t trackerNumberDisplayMode =
        CVarGetInteger(CVAR_TRACKER_ITEM("ItemCountType"), ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY);
    int32_t trackerKeyNumberDisplayMode = CVarGetInteger(CVAR_TRACKER_ITEM("KeyCounts"), KEYS_COLLECTED_MAX);
    float textScalingFactor = static_cast<float>(iconSize) / 36.0f;
    uint32_t actualItemId = INV_CONTENT(item.id);
    bool hasItem = actualItemId != ITEM_NONE;

    if (CVarGetInteger(CVAR_TRACKER_ITEM("HookshotIdentifier"), 0)) {
        if ((actualItemId == ITEM_HOOKSHOT || actualItemId == ITEM_LONGSHOT) && hasItem) {

            // Calculate the scaled position for the text
            ImVec2 textPos =
                ImVec2(p.x + (iconSize / 2) -
                           (ImGui::CalcTextSize(item.id == ITEM_HOOKSHOT ? "H" : "L").x * textScalingFactor / 2) +
                           8 * textScalingFactor,
                       p.y - 22 * textScalingFactor);

            ImGui::SetCursorScreenPos(textPos);
            ImGui::SetWindowFontScale(textScalingFactor);

            ImGui::Text(item.id == ITEM_HOOKSHOT ? "H" : "L");
            ImGui::SetWindowFontScale(1.0f); // Reset font scale to the original state
        }
    }

    ImGui::SetWindowFontScale(textSize / 13.0f);

    if (item.id == ITEM_KEY_SMALL && IsValidSaveFile()) {
        std::string currentString = "";
        std::string maxString = hideMax ? "???" : std::to_string(currentAndMax.maxCapacity);
        ImU32 currentColor = IM_COL_WHITE;
        ImU32 maxColor = IM_COL_GREEN;
        // "Collected / Max", "Current / Collected / Max", "Current / Max"
        if (trackerKeyNumberDisplayMode == KEYS_CURRENT_COLLECTED_MAX ||
            trackerKeyNumberDisplayMode == KEYS_CURRENT_MAX) {
            currentString += std::to_string(currentAndMax.currentAmmo);
            currentString += "/";
        }
        if (trackerKeyNumberDisplayMode == KEYS_COLLECTED_MAX ||
            trackerKeyNumberDisplayMode == KEYS_CURRENT_COLLECTED_MAX) {
            currentString += std::to_string(currentAndMax.currentCapacity);
            currentString += "/";
        }

        ImGui::SetCursorScreenPos(
            ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize((currentString + maxString).c_str()).x / 2), p.y - 14));
        ImGui::PushStyleColor(ImGuiCol_Text, currentColor);
        ImGui::Text("%s", currentString.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, maxColor);
        ImGui::Text("%s", maxString.c_str());
        ImGui::PopStyleColor();
    } else if (currentAndMax.currentCapacity > 0 && trackerNumberDisplayMode != ITEM_TRACKER_NUMBER_NONE &&
               IsValidSaveFile()) {
        std::string currentString = "";
        std::string maxString = "";
        ImU32 currentColor = IM_COL_WHITE;
        ImU32 maxColor = item.id == QUEST_SKULL_TOKEN ? IM_COL_RED : IM_COL_GREEN;

        bool shouldAlignToLeft = CVarGetInteger(CVAR_TRACKER_ITEM("ItemCountAlignLeft"), 0) &&
                                 trackerNumberDisplayMode != ITEM_TRACKER_NUMBER_CAPACITY &&
                                 trackerNumberDisplayMode != ITEM_TRACKER_NUMBER_AMMO;

        bool shouldDisplayAmmo = trackerNumberDisplayMode == ITEM_TRACKER_NUMBER_AMMO ||
                                 trackerNumberDisplayMode == ITEM_TRACKER_NUMBER_CURRENT_AMMO_ONLY ||
                                 // These items have a static capacity, so display ammo instead
                                 item.id == ITEM_BOMBCHU || item.id == ITEM_BEAN || item.id == QUEST_SKULL_TOKEN ||
                                 item.id == ITEM_HEART_CONTAINER || item.id == ITEM_HEART_PIECE;

        bool shouldDisplayMax = !(trackerNumberDisplayMode == ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY ||
                                  trackerNumberDisplayMode == ITEM_TRACKER_NUMBER_CURRENT_AMMO_ONLY);

        if (shouldDisplayAmmo) {
            currentString = std::to_string(currentAndMax.currentAmmo);
            if (currentAndMax.currentAmmo >= currentAndMax.currentCapacity) {
                if (item.id == QUEST_SKULL_TOKEN) {
                    currentColor = IM_COL_RED;
                } else {
                    currentColor = IM_COL_GREEN;
                }
            }
            if (shouldDisplayMax) {
                currentString += "/";
                maxString = std::to_string(currentAndMax.currentCapacity);
            }
            if (currentAndMax.currentAmmo <= 0) {
                currentColor = IM_COL_GRAY;
            }
        } else {
            currentString = std::to_string(currentAndMax.currentCapacity);
            if (currentAndMax.currentCapacity >= currentAndMax.maxCapacity) {
                currentColor = IM_COL_GREEN;
            } else if (shouldDisplayMax) {
                currentString += "/";
                maxString = std::to_string(currentAndMax.maxCapacity);
            }
        }

        float x = shouldAlignToLeft
                      ? p.x
                      : p.x + (iconSize / 2) - (ImGui::CalcTextSize((currentString + maxString).c_str()).x / 2);

        ImGui::SetCursorScreenPos(ImVec2(x, p.y - 14));
        ImGui::PushStyleColor(ImGuiCol_Text, currentColor);
        ImGui::Text("%s", currentString.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, maxColor);
        ImGui::Text("%s", maxString.c_str());
        ImGui::PopStyleColor();
    } else if (item.id == RG_TRIFORCE_PIECE && IS_RANDO &&
               OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_TRIFORCE_HUNT) && IsValidSaveFile()) {
        std::string currentString = "";
        std::string requiredString = "";
        std::string maxString = "";
        uint8_t piecesRequired =
            (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_TRIFORCE_HUNT_PIECES_REQUIRED) + 1);
        uint8_t piecesTotal =
            (OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_TRIFORCE_HUNT_PIECES_TOTAL) + 1);
        ImU32 currentColor = gSaveContext.ship.quest.data.randomizer.triforcePiecesCollected >= piecesRequired
                                 ? IM_COL_GREEN
                                 : IM_COL_WHITE;
        ImU32 maxColor = IM_COL_GREEN;
        int32_t trackerTriforcePieceNumberDisplayMode =
            CVarGetInteger(CVAR_TRACKER_ITEM("TriforcePieceCounts"), TRIFORCE_PIECE_COLLECTED_REQUIRED_MAX);

        currentString += std::to_string(gSaveContext.ship.quest.data.randomizer.triforcePiecesCollected);
        currentString += "/";
        // gItemTrackerTriforcePieceTrack
        if (trackerTriforcePieceNumberDisplayMode == TRIFORCE_PIECE_COLLECTED_REQUIRED_MAX) {
            currentString += std::to_string(piecesRequired);
            currentString += "/";
            maxString += std::to_string(piecesTotal);
        } else if (trackerTriforcePieceNumberDisplayMode == TRIFORCE_PIECE_COLLECTED_REQUIRED) {
            maxString += std::to_string(piecesRequired);
        }

        ImGui::SetCursorScreenPos(
            ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize((currentString + maxString).c_str()).x / 2), p.y - 14));
        ImGui::PushStyleColor(ImGuiCol_Text, currentColor);
        ImGui::Text("%s", currentString.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, maxColor);
        ImGui::Text("%s", maxString.c_str());
        ImGui::PopStyleColor();
    } else {
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y - 14));
        ImGui::Text("");
    }
}

void DrawEquip(ItemTrackerItem item) {
    bool hasEquip = HasEquipment(item);
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                     hasEquip && IsValidSaveFile() ? item.name : item.nameFaded),
                 ImVec2(iconSize, iconSize), ImVec2(0.0f, 0.0f), ImVec2(1, 1));

    Tooltip(SohUtils::GetItemName(item.id).c_str());
}

void DrawQuest(ItemTrackerItem item) {
    bool hasQuestItem = HasQuestItem(item);
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    ImGui::BeginGroup();
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                     hasQuestItem && IsValidSaveFile() ? item.name : item.nameFaded),
                 ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1));

    if (item.id == QUEST_SKULL_TOKEN) {
        DrawItemCount(item, false);
    }

    ImGui::EndGroup();

    Tooltip(SohUtils::GetQuestItemName(item.id).c_str());
};

void DrawItem(ItemTrackerItem item) {

    uint32_t actualItemId = GameInteractor::IsSaveLoaded() ? INV_CONTENT(item.id) : ITEM_NONE;
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    bool hasItem = actualItemId != ITEM_NONE;
    std::string itemName = "";

    // Hack fix as RG_MARKET_SHOOTING_GALLERY_KEY is RandomizerGet #255 which collides
    // with ITEM_NONE (ItemId #255) due to the lack of a modid to separate them
    if (item.name != "ITEM_KEY_SMALL" && item.id == ITEM_NONE) {
        return;
    }

    switch (item.id) {
        case ITEM_HEART_CONTAINER:
            actualItemId = item.id;
            hasItem = gSaveContext.ship.stats.heartContainers > 0;
            break;
        case ITEM_HEART_PIECE:
            actualItemId = item.id;
            hasItem = gSaveContext.ship.stats.heartPieces > 0;
            break;
        case ITEM_MAGIC_SMALL:
        case ITEM_MAGIC_LARGE:
            actualItemId = gSaveContext.magicLevel == 2 ? ITEM_MAGIC_LARGE : ITEM_MAGIC_SMALL;
            hasItem = gSaveContext.magicLevel > 0;
            break;
        case ITEM_WALLET_ADULT:
        case ITEM_WALLET_GIANT:
            actualItemId = CUR_UPG_VALUE(UPG_WALLET) == 2 ? ITEM_WALLET_GIANT : ITEM_WALLET_ADULT;
            hasItem = !IS_RANDO || Flags_GetRandomizerInf(RAND_INF_HAS_WALLET);
            break;
        case ITEM_BRACELET:
        case ITEM_GAUNTLETS_SILVER:
        case ITEM_GAUNTLETS_GOLD:
            actualItemId = CUR_UPG_VALUE(UPG_STRENGTH) == 3   ? ITEM_GAUNTLETS_GOLD
                           : CUR_UPG_VALUE(UPG_STRENGTH) == 2 ? ITEM_GAUNTLETS_SILVER
                                                              : ITEM_BRACELET;
            hasItem = CUR_UPG_VALUE(UPG_STRENGTH) > 0;
            break;
        case ITEM_SCALE_SILVER:
        case ITEM_SCALE_GOLDEN:
            actualItemId = CUR_UPG_VALUE(UPG_SCALE) == 2 ? ITEM_SCALE_GOLDEN : ITEM_SCALE_SILVER;
            hasItem = CUR_UPG_VALUE(UPG_SCALE) > 0;
            break;
        case ITEM_RUPEE_GREEN:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_GREG_FOUND);
            break;
        case RG_TRIFORCE_PIECE:
            actualItemId = item.id;
            hasItem = IS_RANDO && OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_TRIFORCE_HUNT);
            itemName = "Triforce Piece";
            break;
        case RG_GOHMA_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_GOHMA_SOUL);
            itemName = "Gohma's Soul";
            break;
        case RG_KING_DODONGO_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_KING_DODONGO_SOUL);
            itemName = "King Dodongo's Soul";
            break;
        case RG_BARINADE_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_BARINADE_SOUL);
            itemName = "Barinade's Soul";
            break;
        case RG_PHANTOM_GANON_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_PHANTOM_GANON_SOUL);
            itemName = "Phantom Ganon's Soul";
            break;
        case RG_VOLVAGIA_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_VOLVAGIA_SOUL);
            itemName = "Volvagia's Soul";
            break;
        case RG_MORPHA_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_MORPHA_SOUL);
            itemName = "Morpha's Soul";
            break;
        case RG_BONGO_BONGO_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_BONGO_BONGO_SOUL);
            itemName = "Bongo Bongo's Soul";
            break;
        case RG_TWINROVA_SOUL:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_TWINROVA_SOUL);
            itemName = "Twinrova's Soul";
            break;
        case RG_GANON_SOUL:
            actualItemId = item.id;
            hasItem = OTRGlobals::Instance->gRandomizer->GetRandoSettingValue(RSK_SHUFFLE_BOSS_SOULS) ==
                              RO_BOSS_SOULS_ON_PLUS_GANON
                          ? Flags_GetRandomizerInf(RAND_INF_GANON_SOUL)
                          : true;
            itemName = "Ganon's Soul";
            break;

        case RG_OCARINA_A_BUTTON:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_HAS_OCARINA_A);
            itemName = "Ocarina A Button";
            break;
        case RG_OCARINA_C_UP_BUTTON:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_HAS_OCARINA_C_UP);
            itemName = "Ocarina C Up Button";
            break;
        case RG_OCARINA_C_DOWN_BUTTON:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_HAS_OCARINA_C_DOWN);
            itemName = "Ocarina C Down Button";
            break;
        case RG_OCARINA_C_LEFT_BUTTON:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_HAS_OCARINA_C_LEFT);
            itemName = "Ocarina C Left Button";
            break;
        case RG_OCARINA_C_RIGHT_BUTTON:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_HAS_OCARINA_C_RIGHT);
            itemName = "Ocarina C Right Button";
            break;
        case ITEM_FISHING_POLE:
            actualItemId = item.id;
            hasItem = IS_RANDO && Flags_GetRandomizerInf(RAND_INF_FISHING_POLE_FOUND);
            itemName = "Fishing Pole";
            break;

        case RG_GUARD_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_GUARD_HOUSE_KEY_OBTAINED);
            itemName = "Guard House Key";
            break;
        case RG_MARKET_BAZAAR_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_MARKET_BAZAAR_KEY_OBTAINED);
            itemName = "Market Bazaar Key";
            break;
        case RG_MARKET_POTION_SHOP_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_MARKET_POTION_SHOP_KEY_OBTAINED);
            itemName = "Market Potion Shop Key";
            break;
        case RG_MASK_SHOP_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_MASK_SHOP_KEY_OBTAINED);
            itemName = "Mask Shop Key";
            break;
        case RG_MARKET_SHOOTING_GALLERY_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_MARKET_SHOOTING_GALLERY_KEY_OBTAINED);
            itemName = "Market Shooting Gallery Key";
            break;
        case RG_BOMBCHU_BOWLING_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_BOMBCHU_BOWLING_KEY_OBTAINED);
            itemName = "Bombchu Bowling Key";
            break;
        case RG_TREASURE_CHEST_GAME_BUILDING_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_TREASURE_CHEST_GAME_BUILDING_KEY_OBTAINED);
            itemName = "Treasure Chest Game Building Key";
            break;
        case RG_BOMBCHU_SHOP_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_BOMBCHU_SHOP_KEY_OBTAINED);
            itemName = "Bombchu Shop Key";
            break;
        case RG_RICHARDS_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_RICHARDS_HOUSE_KEY_OBTAINED);
            itemName = "Richards House Key";
            break;
        case RG_ALLEY_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_ALLEY_HOUSE_KEY_OBTAINED);
            itemName = "Alley House Key";
            break;
        case RG_KAK_BAZAAR_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_KAK_BAZAAR_KEY_OBTAINED);
            itemName = "Kak Bazaar Key";
            break;
        case RG_KAK_POTION_SHOP_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_KAK_POTION_SHOP_KEY_OBTAINED);
            itemName = "Kak Potion Shop Key";
            break;
        case RG_BOSS_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_BOSS_HOUSE_KEY_OBTAINED);
            itemName = "Boss House Key";
            break;
        case RG_GRANNYS_POTION_SHOP_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_GRANNYS_POTION_SHOP_KEY_OBTAINED);
            itemName = "Granny's Potion Shop Key";
            break;
        case RG_SKULLTULA_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_SKULLTULA_HOUSE_KEY_OBTAINED);
            itemName = "Skulltula House Key";
            break;
        case RG_IMPAS_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_IMPAS_HOUSE_KEY_OBTAINED);
            itemName = "Impa's House Key";
            break;
        case RG_WINDMILL_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_WINDMILL_KEY_OBTAINED);
            itemName = "Windmill Key";
            break;
        case RG_KAK_SHOOTING_GALLERY_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_KAK_SHOOTING_GALLERY_KEY_OBTAINED);
            itemName = "Kak Shooting Gallery Key";
            break;
        case RG_DAMPES_HUT_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_DAMPES_HUT_KEY_OBTAINED);
            itemName = "Dampé's Hut Key";
            break;
        case RG_TALONS_HOUSE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_TALONS_HOUSE_KEY_OBTAINED);
            itemName = "Talon's House Key";
            break;
        case RG_STABLES_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_STABLES_KEY_OBTAINED);
            itemName = "Stables Key";
            break;
        case RG_BACK_TOWER_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_BACK_TOWER_KEY_OBTAINED);
            itemName = "Back Tower Key";
            break;
        case RG_HYLIA_LAB_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_HYLIA_LAB_KEY_OBTAINED);
            itemName = "Hylia Lab Key";
            break;
        case RG_FISHING_HOLE_KEY:
            actualItemId = item.id;
            hasItem = Flags_GetRandomizerInf(RAND_INF_FISHING_HOLE_KEY_OBTAINED);
            itemName = "Fishing Hole Key";
            break;
    }

    if (GameInteractor::IsSaveLoaded() &&
        (hasItem && item.id != actualItemId &&
         actualItemTrackerItemMap.find(actualItemId) != actualItemTrackerItemMap.end())) {
        item = actualItemTrackerItemMap[actualItemId];
    }

    ImGui::BeginGroup();

    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                     hasItem && IsValidSaveFile() ? item.name : item.nameFaded),
                 ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1));

    DrawItemCount(item, false);

    if (item.id >= RG_GOHMA_SOUL && item.id <= RG_GANON_SOUL) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        std::string bossName = itemTrackerBossShortNames[item.id];
        ImGui::SetCursorScreenPos(
            ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize(bossName.c_str()).x / 2), p.y - (iconSize + 13)));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL_WHITE);
        ImGui::Text("%s", bossName.c_str());
        ImGui::PopStyleColor();
    }

    if (item.id >= RG_OCARINA_A_BUTTON && item.id <= RG_OCARINA_C_RIGHT_BUTTON) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        std::string ocarinaButtonName = itemTrackerOcarinaButtonShortNames[item.id];
        ImGui::SetCursorScreenPos(ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize(ocarinaButtonName.c_str()).x / 2),
                                         p.y - (iconSize + 13)));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL_WHITE);
        ImGui::Text("%s", ocarinaButtonName.c_str());
        ImGui::PopStyleColor();
    }

    if (item.id >= RG_GUARD_HOUSE_KEY && item.id <= RG_FISHING_HOLE_KEY) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        std::string overworldKeyName = itemTrackerOverworldKeyShortNames[item.id];
        ImGui::SetCursorScreenPos(ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize(overworldKeyName.c_str()).x / 2),
                                         p.y - (iconSize + 13)));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL_WHITE);
        ImGui::Text("%s", overworldKeyName.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndGroup();

    if (itemName == "") {
        itemName = SohUtils::GetItemName(item.id);
    }

    Tooltip(itemName.c_str());
}

void DrawBottle(ItemTrackerItem item) {
    uint32_t actualItemId =
        GameInteractor::IsSaveLoaded() ? (gSaveContext.inventory.items[SLOT(item.id) + item.data]) : false;
    bool hasItem = actualItemId != ITEM_NONE;

    if (GameInteractor::IsSaveLoaded() &&
        (hasItem && item.id != actualItemId &&
         actualItemTrackerItemMap.find(actualItemId) != actualItemTrackerItemMap.end())) {
        item = actualItemTrackerItemMap[actualItemId];
    }

    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                     hasItem && IsValidSaveFile() ? item.name : item.nameFaded),
                 ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1));

    Tooltip(SohUtils::GetItemName(item.id).c_str());
};

void DrawDungeonItem(ItemTrackerItem item) {
    uint32_t itemId = item.id;
    ImU32 dungeonColor = IM_COL_WHITE;
    uint32_t bitMask = 1 << (item.id - ITEM_KEY_BOSS); // Bitset starts at ITEM_KEY_BOSS == 0. the rest are sequential
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    bool hasItem = GameInteractor::IsSaveLoaded() ? (bitMask & gSaveContext.inventory.dungeonItems[item.data]) : false;
    bool hasSmallKey = GameInteractor::IsSaveLoaded() ? ((gSaveContext.inventory.dungeonKeys[item.data]) >= 0) : false;
    ImGui::BeginGroup();
    if (itemId == ITEM_KEY_SMALL) {
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                         hasSmallKey && IsValidSaveFile() ? item.name : item.nameFaded),
                     ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1));
    } else {
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                         hasItem && IsValidSaveFile() ? item.name : item.nameFaded),
                     ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1));
    }

    if (CheckTracker::IsAreaSpoiled(RandomizerCheckObjects::GetRCAreaBySceneID(static_cast<SceneID>(item.data))) &&
        GameInteractor::IsSaveLoaded()) {
        dungeonColor = (ResourceMgr_IsSceneMasterQuest(item.data) ? IM_COL_PURPLE : IM_COL_LIGHT_YELLOW);
    }

    if (itemId == ITEM_KEY_SMALL) {
        DrawItemCount(item, !CheckTracker::IsAreaSpoiled(
                                RandomizerCheckObjects::GetRCAreaBySceneID(static_cast<SceneID>(item.data))));

        ImVec2 p = ImGui::GetCursorScreenPos();
        // offset puts the text at the correct level. for some reason, if the save is loaded, the margin is 3 pixels
        // higher only for small keys, so we use 16 then. Otherwise, 13 is where everything else is
        int offset = GameInteractor::IsSaveLoaded() ? 16 : 13;
        std::string dungeonName = itemTrackerDungeonShortNames[item.data];
        ImGui::SetCursorScreenPos(
            ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize(dungeonName.c_str()).x / 2), p.y - (iconSize + offset)));
        ImGui::PushStyleColor(ImGuiCol_Text, dungeonColor);
        ImGui::Text("%s", dungeonName.c_str());
        ImGui::PopStyleColor();
    }

    if (itemId == ITEM_DUNGEON_MAP && (item.data == SCENE_DEKU_TREE || item.data == SCENE_DODONGOS_CAVERN ||
                                       item.data == SCENE_JABU_JABU || item.data == SCENE_ICE_CAVERN)) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        std::string dungeonName = itemTrackerDungeonShortNames[item.data];
        ImGui::SetCursorScreenPos(
            ImVec2(p.x + (iconSize / 2) - (ImGui::CalcTextSize(dungeonName.c_str()).x / 2), p.y - (iconSize + 13)));
        ImGui::PushStyleColor(ImGuiCol_Text, dungeonColor);
        ImGui::Text("%s", dungeonName.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::EndGroup();

    Tooltip(SohUtils::GetItemName(item.id).c_str());
}

void DrawSong(ItemTrackerItem item) {
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool hasSong = HasSong(item);
    ImGui::SetCursorScreenPos(ImVec2(p.x + 6, p.y));
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                     hasSong && IsValidSaveFile() ? item.name : item.nameFaded),
                 ImVec2(iconSize / 1.5f, iconSize), ImVec2(0, 0), ImVec2(1, 1));
    Tooltip(SohUtils::GetQuestItemName(item.id).c_str());
}

void DrawNotes(bool resizeable = false) {
    ImGui::BeginGroup();
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    int iconSpacing = CVarGetInteger(CVAR_TRACKER_ITEM("IconSpacing"), 12);

    struct ItemTrackerNotes {
        static int TrackerNotesResizeCallback(ImGuiInputTextCallbackData* data) {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                ImVector<char>* itemTrackerNotes = (ImVector<char>*)data->UserData;
                IM_ASSERT(itemTrackerNotes->begin() == data->Buf);
                itemTrackerNotes->resize(
                    data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
                data->Buf = itemTrackerNotes->begin();
            }
            return 0;
        }
        static bool TrackerNotesInputTextMultiline(const char* label, ImVector<char>* itemTrackerNotes,
                                                   const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0) {
            IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
            return ImGui::InputTextMultiline(label, itemTrackerNotes->begin(), (size_t)itemTrackerNotes->size(), size,
                                             flags | ImGuiInputTextFlags_CallbackResize,
                                             ItemTrackerNotes::TrackerNotesResizeCallback, (void*)itemTrackerNotes);
        }
    };
    ImVec2 size = resizeable ? ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y)
                             : ImVec2(((iconSize + iconSpacing) * 6) - 8.0f, 200.0f);
    if (GameInteractor::IsSaveLoaded()) {
        if (ItemTrackerNotes::TrackerNotesInputTextMultiline("##ItemTrackerNotes", &itemTrackerNotes, size,
                                                             ImGuiInputTextFlags_AllowTabInput)) {
            notesNeedSave = true;
            notesIdleFrames = 0;
        }
        if ((ImGui::IsItemDeactivatedAfterEdit() || (notesNeedSave && notesIdleFrames > notesMaxIdleFrames)) &&
            IsValidSaveFile()) {
            notesNeedSave = false;
            SaveManager::Instance->SaveSection(gSaveContext.fileNum, itemTrackerSectionId, true);
        }
    }
    ImGui::EndGroup();
}

void DrawTotalChecks() {
    uint16_t totalChecks = CheckTracker::GetTotalChecks();
    uint16_t totalChecksGotten = CheckTracker::GetTotalChecksGotten();

    ImGui::BeginGroup();
    if (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_FLOATING) {
        ImGui::SetWindowFontScale(2.5);
    } else {
        ImGui::SetWindowFontScale(1);
    }
    ImGui::Text("Checks: %d/%d", totalChecksGotten, totalChecks);
    ImGui::EndGroup();
}

// Windowing stuff
void BeginFloatingWindows(std::string UniqueName, ImGuiWindowFlags flags = 0) {
    ImGuiWindowFlags windowFlags = flags;

    if (windowFlags == 0) {
        windowFlags |=
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize;
    }

    if (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_FLOATING) {
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        windowFlags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar |
                       ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

        if (!CVarGetInteger(CVAR_TRACKER_ITEM("Draggable"), 0)) {
            windowFlags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
        }
    }
    auto color = VecFromRGBA8(CVarGetColor(CVAR_TRACKER_ITEM("BgColor.Value"), { 0, 0, 0, 0 }));
    auto maybeParent = ImGui::GetCurrentWindow();
    ImGuiWindow* window = ImGui::FindWindowByName(UniqueName.c_str());
    if (window != NULL && window->DockTabIsVisible && window->ParentWindow != NULL &&
        std::string(window->ParentWindow->Name).compare(0, strlen("Main - Deck"), "Main - Deck") == 0) {
        color.w = 1.0f;
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    if (presetLoaded && presetPos.contains(UniqueName)) {
        ImGui::SetNextWindowSize(presetSize[UniqueName]);
        ImGui::SetNextWindowPos(presetPos[UniqueName]);
        presetSize.erase(UniqueName);
        presetPos.erase(UniqueName);
    }
    ImGui::Begin(UniqueName.c_str(), nullptr, windowFlags);
}
void EndFloatingWindows() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::End();
}

/**
 * DrawItemsInRows
 * Takes in a vector of ItemTrackerItem and draws them in rows of N items
 */
void DrawItemsInRows(std::vector<ItemTrackerItem> items, int columns = 6) {
    float iconSize = static_cast<float>(CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36));
    int iconSpacing = CVarGetInteger(CVAR_TRACKER_ITEM("IconSpacing"), 12);
    int topPadding =
        (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_WINDOW) ? 20 : 0;

    for (int i = 0; i < items.size(); i++) {
        int row = i / columns;
        int column = i % columns;
        ImGui::SetCursorPos(
            ImVec2((column * (iconSize + iconSpacing) + 8.0f), (row * (iconSize + iconSpacing)) + 8.0f + topPadding));
        items[i].drawFunc(items[i]);
    }
}

/**
 * DrawItemsInACircle
 * Takes in a vector of ItemTrackerItem and draws them evenly spread across a circle
 */
void DrawItemsInACircle(std::vector<ItemTrackerItem> items) {
    int iconSize = CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36);
    int iconSpacing = CVarGetInteger(CVAR_TRACKER_ITEM("IconSpacing"), 12);

    ImVec2 max = ImGui::GetWindowContentRegionMax();
    float radius = (iconSize + iconSpacing) * 2.0f;

    for (int i = 0; i < items.size(); i++) {
        float angle = static_cast<float>(i / items.size() * 2.0f * M_PI);
        float x = (radius / 2.0f) * cos(angle) + max.x / 2.0f;
        float y = (radius / 2.0f) * sin(angle) + max.y / 2.0f;
        ImGui::SetCursorPos(ImVec2(x - (CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36) - 8) / 2.0f, y + 4));
        items[i].drawFunc(items[i]);
    }
}

/**
 * GetDungeonItemsVector
 * Loops over dungeons and creates vectors of items in the correct order
 * to then call DrawItemsInRows
 */
std::vector<ItemTrackerItem> GetDungeonItemsVector(std::vector<ItemTrackerDungeon> dungeons, int columns = 6) {
    std::vector<ItemTrackerItem> dungeonItems = {};

    int rowCount = 0;
    for (int i = 0; i < dungeons.size(); i++) {
        if (dungeons[i].items.size() > rowCount)
            rowCount = static_cast<int32_t>(dungeons[i].items.size());
    }

    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < MIN(dungeons.size(), columns); j++) {
            if (dungeons[j].items.size() > i) {
                switch (dungeons[j].items[i]) {
                    case ITEM_KEY_SMALL:
                        dungeonItems.push_back(ITEM_TRACKER_ITEM(ITEM_KEY_SMALL, dungeons[j].id, DrawDungeonItem));
                        break;
                    case ITEM_KEY_BOSS:
                        // Swap Ganon's Castle boss key to the right scene ID manually
                        if (dungeons[j].id == SCENE_INSIDE_GANONS_CASTLE) {
                            dungeonItems.push_back(
                                ITEM_TRACKER_ITEM(ITEM_KEY_BOSS, SCENE_GANONS_TOWER, DrawDungeonItem));
                        } else {
                            dungeonItems.push_back(ITEM_TRACKER_ITEM(ITEM_KEY_BOSS, dungeons[j].id, DrawDungeonItem));
                        }
                        break;
                    case ITEM_DUNGEON_MAP:
                        dungeonItems.push_back(ITEM_TRACKER_ITEM(ITEM_DUNGEON_MAP, dungeons[j].id, DrawDungeonItem));
                        break;
                    case ITEM_COMPASS:
                        dungeonItems.push_back(ITEM_TRACKER_ITEM(ITEM_COMPASS, dungeons[j].id, DrawDungeonItem));
                        break;
                }
            } else {
                dungeonItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
            }
        }
    }

    if (dungeons.size() > columns) {
        std::vector<ItemTrackerItem> nextDungeonItems =
            GetDungeonItemsVector(std::vector<ItemTrackerDungeon>(dungeons.begin() + columns, dungeons.end()), columns);
        dungeonItems.insert(dungeonItems.end(), nextDungeonItems.begin(), nextDungeonItems.end());
    }

    return dungeonItems;
}
/* ****************************************************** */

void UpdateVectors() {
    if (!shouldUpdateVectors) {
        return;
    }

    dungeonRewards.clear();
    dungeonRewards.insert(dungeonRewards.end(), dungeonRewardStones.begin(), dungeonRewardStones.end());
    dungeonRewards.insert(dungeonRewards.end(), dungeonRewardMedallions.begin(), dungeonRewardMedallions.end());

    dungeonItems.clear();
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DungeonItems.Layout"), 1) &&
        CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
        if (CVarGetInteger(CVAR_TRACKER_ITEM("DungeonItems.DisplayMaps"), 1)) {
            dungeonItems = GetDungeonItemsVector(itemTrackerDungeonsWithMapsHorizontal, 12);
            // Manually adding Thieves Hideout to an open spot so we don't get an additional row for one item
            dungeonItems[23] = ITEM_TRACKER_ITEM(ITEM_KEY_SMALL, SCENE_THIEVES_HIDEOUT, DrawDungeonItem);
        } else {
            // Manually adding Thieves Hideout to an open spot so we don't get an additional row for one item
            dungeonItems = GetDungeonItemsVector(itemTrackerDungeonsHorizontal, 8);
            dungeonItems[15] = ITEM_TRACKER_ITEM(ITEM_KEY_SMALL, SCENE_THIEVES_HIDEOUT, DrawDungeonItem);
        }
    } else {
        if (CVarGetInteger(CVAR_TRACKER_ITEM("DungeonItems.DisplayMaps"), 1)) {
            dungeonItems = GetDungeonItemsVector(itemTrackerDungeonsWithMapsCompact);
            // Manually adding Thieves Hideout to an open spot so we don't get an additional row for one item
            dungeonItems[35] = ITEM_TRACKER_ITEM(ITEM_KEY_SMALL, SCENE_THIEVES_HIDEOUT, DrawDungeonItem);
        } else {
            dungeonItems = GetDungeonItemsVector(itemTrackerDungeonsCompact);
        }
    }

    mainWindowItems.clear();
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Inventory"), SECTION_DISPLAY_MAIN_WINDOW) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        mainWindowItems.insert(mainWindowItems.end(), inventoryItems.begin(), inventoryItems.end());
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Equipment"), SECTION_DISPLAY_MAIN_WINDOW) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        mainWindowItems.insert(mainWindowItems.end(), equipmentItems.begin(), equipmentItems.end());
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Misc"), SECTION_DISPLAY_MAIN_WINDOW) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        mainWindowItems.insert(mainWindowItems.end(), miscItems.begin(), miscItems.end());
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonRewards"), SECTION_DISPLAY_MAIN_WINDOW) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        mainWindowItems.insert(mainWindowItems.end(), dungeonRewardStones.begin(), dungeonRewardStones.end());
        mainWindowItems.insert(mainWindowItems.end(), dungeonRewardMedallions.begin(), dungeonRewardMedallions.end());
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Songs"), SECTION_DISPLAY_MAIN_WINDOW) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Misc"), SECTION_DISPLAY_MAIN_WINDOW) ==
                SECTION_DISPLAY_MAIN_WINDOW &&
            CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonRewards"), SECTION_DISPLAY_MAIN_WINDOW) !=
                SECTION_DISPLAY_MAIN_WINDOW) {
            mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
            mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
            mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
        }
        mainWindowItems.insert(mainWindowItems.end(), songItems.begin(), songItems.end());
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), SECTION_DISPLAY_HIDDEN) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        mainWindowItems.insert(mainWindowItems.end(), dungeonItems.begin(), dungeonItems.end());
    }

    // if we're adding greg to the misc window,
    // and misc isn't on the main window,
    // and it doesn't already have greg, add him
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Greg"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
            SECTION_DISPLAY_EXTENDED_MISC_WINDOW &&
        CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Misc"), SECTION_DISPLAY_MAIN_WINDOW) !=
            SECTION_DISPLAY_MAIN_WINDOW) {
        if (std::none_of(miscItems.begin(), miscItems.end(),
                         [](ItemTrackerItem item) { return item.id == ITEM_RUPEE_GREEN; }))
            miscItems.insert(miscItems.end(), gregItems.begin(), gregItems.end());
    } else {
        miscItems.erase(std::remove_if(miscItems.begin(), miscItems.end(),
                                       [](ItemTrackerItem i) { return i.id == ITEM_RUPEE_GREEN; }),
                        miscItems.end());
    }

    bool newRowAdded = false;
    // if we're adding greg to the main window
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Greg"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
        SECTION_DISPLAY_EXTENDED_MAIN_WINDOW) {
        if (!newRowAdded) {
            // insert empty items until we're on a new row for greg
            while (mainWindowItems.size() % 6) {
                mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
            }
            newRowAdded = true;
        }

        // add greg
        mainWindowItems.insert(mainWindowItems.end(), gregItems.begin(), gregItems.end());
    }

    // If we're adding triforce pieces to the main window
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.TriforcePieces"), SECTION_DISPLAY_HIDDEN) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        // If Greg isn't on the main window, add empty items to place the triforce pieces on a new row.
        if (!newRowAdded) {
            while (mainWindowItems.size() % 6) {
                mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
            }
            newRowAdded = true;
        }

        // Add triforce pieces
        mainWindowItems.insert(mainWindowItems.end(), triforcePieces.begin(), triforcePieces.end());
    }

    // if misc is separate and fishing pole isn't added, add fishing pole to misc
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.FishingPole"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
            SECTION_DISPLAY_EXTENDED_MISC_WINDOW &&
        CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Misc"), SECTION_DISPLAY_MAIN_WINDOW) !=
            SECTION_DISPLAY_MAIN_WINDOW) {
        if (std::none_of(miscItems.begin(), miscItems.end(),
                         [](ItemTrackerItem item) { return item.id == ITEM_FISHING_POLE; }))
            miscItems.insert(miscItems.end(), fishingPoleItems.begin(), fishingPoleItems.end());
    } else {
        miscItems.erase(std::remove_if(miscItems.begin(), miscItems.end(),
                                       [](ItemTrackerItem i) { return i.id == ITEM_FISHING_POLE; }),
                        miscItems.end());
    }
    // add fishing pole to main window
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.FishingPole"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
        SECTION_DISPLAY_EXTENDED_MAIN_WINDOW) {
        if (!newRowAdded) {
            while (mainWindowItems.size() % 6) {
                mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
            }
            newRowAdded = true;
        }

        mainWindowItems.insert(mainWindowItems.end(), fishingPoleItems.begin(), fishingPoleItems.end());
    }

    // If we're adding boss souls to the main window...
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.BossSouls"), SECTION_DISPLAY_HIDDEN) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        //...add empty items on the main window to get the souls on their own row. (Too many to sit with Greg/Triforce
        // pieces)
        while (mainWindowItems.size() % 6) {
            mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
        }

        // Add boss souls
        mainWindowItems.insert(mainWindowItems.end(), bossSoulItems.begin(), bossSoulItems.end());
    }

    // If we're adding ocarina buttons to the main window...
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.OcarinaButtons"), SECTION_DISPLAY_HIDDEN) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        //...add empty items on the main window to get the buttons on their own row. (Too many to sit with Greg/Triforce
        // pieces/boss souls)
        while (mainWindowItems.size() % 6) {
            mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
        }

        // Add ocarina buttons
        mainWindowItems.insert(mainWindowItems.end(), ocarinaButtonItems.begin(), ocarinaButtonItems.end());
    }

    // If we're adding overworld keys to the main window...
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.OverworldKeys"), SECTION_DISPLAY_HIDDEN) ==
        SECTION_DISPLAY_MAIN_WINDOW) {
        //...add empty items on the main window to get the keys on their own row. (Too many to sit with Greg/Triforce
        // pieces/boss souls/ocarina buttons)
        while (mainWindowItems.size() % 6) {
            mainWindowItems.push_back(ITEM_TRACKER_ITEM(ITEM_NONE, 0, DrawItem));
        }

        // Add overworld keys
        mainWindowItems.insert(mainWindowItems.end(), overworldKeyItems.begin(), overworldKeyItems.end());
    }

    shouldUpdateVectors = false;
}

void ItemTrackerInitFile(bool isDebug) {
    itemTrackerNotes.clear();
    itemTrackerNotes.push_back(0);
}

void ItemTrackerSaveFile(SaveContext* saveContext, int sectionID, bool fullSave) {
    SaveManager::Instance->SaveData("personalNotes",
                                    std::string(std::begin(itemTrackerNotes), std::end(itemTrackerNotes)).c_str());
}

void ItemTrackerLoadFile() {
    std::string initialTrackerNotes = "";
    SaveManager::Instance->LoadData("personalNotes", initialTrackerNotes);
    itemTrackerNotes.resize(static_cast<int>(initialTrackerNotes.length() + 1));
    if (initialTrackerNotes != "") {
        SohUtils::CopyStringToCharArray(itemTrackerNotes.Data, initialTrackerNotes.c_str(), itemTrackerNotes.size());
    } else {
        itemTrackerNotes.push_back(0);
    }
}

void ItemTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }
    ImGui::PushFont(OTRGlobals::Instance->fontMono);
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
    ImGui::PopFont();
}

void ItemTrackerWindow::DrawElement() {
    UpdateVectors();

    int iconSize = CVarGetInteger(CVAR_TRACKER_ITEM("IconSize"), 36);
    int iconSpacing = CVarGetInteger(CVAR_TRACKER_ITEM("IconSpacing"), 12);
    int comboButton1Mask = buttonMap[CVarGetInteger(CVAR_TRACKER_ITEM("ComboButton1"), TRACKER_COMBO_BUTTON_L)];
    int comboButton2Mask = buttonMap[CVarGetInteger(CVAR_TRACKER_ITEM("ComboButton2"), TRACKER_COMBO_BUTTON_R)];
    OSContPad* buttonsPressed =
        std::dynamic_pointer_cast<LUS::ControlDeck>(Ship::Context::GetInstance()->GetControlDeck())->GetPads();
    bool comboButtonsHeld = buttonsPressed != nullptr && buttonsPressed[0].button & comboButton1Mask &&
                            buttonsPressed[0].button & comboButton2Mask;
    bool isPaused = CVarGetInteger(CVAR_TRACKER_ITEM("ShowOnlyPaused"), 0) == 0 ||
                    gPlayState != nullptr && gPlayState->pauseCtx.state > 0;

    if (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_WINDOW ||
        isPaused &&
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Main"), TRACKER_DISPLAY_ALWAYS) == TRACKER_DISPLAY_ALWAYS
                 ? CVarGetInteger(CVAR_WINDOW("ItemTracker"), 0)
                 : comboButtonsHeld)) {
        if ((CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Inventory"), SECTION_DISPLAY_MAIN_WINDOW) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Equipment"), SECTION_DISPLAY_MAIN_WINDOW) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Misc"), SECTION_DISPLAY_MAIN_WINDOW) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonRewards"), SECTION_DISPLAY_MAIN_WINDOW) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Songs"), SECTION_DISPLAY_MAIN_WINDOW) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), SECTION_DISPLAY_HIDDEN) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Greg"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
             SECTION_DISPLAY_EXTENDED_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.TriforcePieces"), SECTION_DISPLAY_HIDDEN) ==
             SECTION_DISPLAY_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.FishingPole"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
             SECTION_DISPLAY_EXTENDED_MAIN_WINDOW) ||
            (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Notes"), SECTION_DISPLAY_HIDDEN) ==
             SECTION_DISPLAY_MAIN_WINDOW)) {
            BeginFloatingWindows("Item Tracker");
            DrawItemsInRows(mainWindowItems, 6);

            if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Notes"), SECTION_DISPLAY_HIDDEN) ==
                    SECTION_DISPLAY_MAIN_WINDOW &&
                (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_FLOATING &&
                 CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Main"), TRACKER_DISPLAY_ALWAYS) ==
                     TRACKER_DISPLAY_ALWAYS)) {
                DrawNotes();
            }
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Inventory"), SECTION_DISPLAY_MAIN_WINDOW) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Inventory Items Tracker");
            DrawItemsInRows(inventoryItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Equipment"), SECTION_DISPLAY_MAIN_WINDOW) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Equipment Items Tracker");
            DrawItemsInRows(equipmentItems, 3);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Misc"), SECTION_DISPLAY_MAIN_WINDOW) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Misc Items Tracker");
            DrawItemsInRows(miscItems, 4);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonRewards"), SECTION_DISPLAY_MAIN_WINDOW) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Dungeon Rewards Tracker");
            if (CVarGetInteger(CVAR_TRACKER_ITEM("DungeonRewardsLayout"), 0)) {
                ImGui::BeginGroup();
                DrawItemsInACircle(dungeonRewardMedallions);
                ImGui::EndGroup();
                ImGui::BeginGroup();
                DrawItemsInRows(dungeonRewardStones);
                ImGui::EndGroup();
            } else {
                DrawItemsInRows(dungeonRewards, 3);
            }
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Songs"), SECTION_DISPLAY_MAIN_WINDOW) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Songs Tracker");
            DrawItemsInRows(songItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Dungeon Items Tracker");
            if (CVarGetInteger(CVAR_TRACKER_ITEM("DungeonItems.Layout"), 1)) {
                if (CVarGetInteger(CVAR_TRACKER_ITEM("DungeonItems.DisplayMaps"), 1)) {
                    DrawItemsInRows(dungeonItems, 12);
                } else {
                    DrawItemsInRows(dungeonItems, 8);
                }
            } else {
                DrawItemsInRows(dungeonItems);
            }
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Greg"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
            SECTION_DISPLAY_EXTENDED_SEPARATE) {
            BeginFloatingWindows("Greg Tracker");
            DrawItemsInRows(gregItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.TriforcePieces"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Triforce Piece Tracker");
            DrawItemsInRows(triforcePieces);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.BossSouls"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Boss Soul Tracker");
            DrawItemsInRows(bossSoulItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.OcarinaButtons"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Ocarina Button Tracker");
            DrawItemsInRows(ocarinaButtonItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.OverworldKeys"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
            BeginFloatingWindows("Overworld Key Tracker");
            DrawItemsInRows(overworldKeyItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.FishingPole"), SECTION_DISPLAY_EXTENDED_HIDDEN) ==
            SECTION_DISPLAY_EXTENDED_SEPARATE) {
            BeginFloatingWindows("Fishing Pole Tracker");
            DrawItemsInRows(fishingPoleItems);
            EndFloatingWindows();
        }

        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Notes"), SECTION_DISPLAY_HIDDEN) ==
                SECTION_DISPLAY_SEPARATE &&
            (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_WINDOW ||
             (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_FLOATING &&
              CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Main"), TRACKER_DISPLAY_ALWAYS) !=
                  TRACKER_DISPLAY_COMBO_BUTTON))) {
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            BeginFloatingWindows("Personal Notes", ImGuiWindowFlags_NoFocusOnAppearing);
            DrawNotes(true);
            EndFloatingWindows();
        }

        if (CVarGetInteger("gTrackers.ItemTracker.TotalChecks.DisplayType", SECTION_DISPLAY_MINIMAL_HIDDEN) ==
            SECTION_DISPLAY_MINIMAL_SEPARATE) {
            ImGui::SetNextWindowSize(ImVec2(450, 300), ImGuiCond_FirstUseEver);
            BeginFloatingWindows("Total Checks");
            DrawTotalChecks();
            EndFloatingWindows();
        }
    }
    if (presetLoaded) {
        shouldUpdateVectors = true;
        presetLoaded = false;
    }
}

static std::unordered_map<int32_t, const char*> itemTrackerCapacityTrackOptions = {
    { ITEM_TRACKER_NUMBER_NONE, "No Numbers" },
    { ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY, "Current Capacity" },
    { ITEM_TRACKER_NUMBER_CURRENT_AMMO_ONLY, "Current Ammo" },
    { ITEM_TRACKER_NUMBER_CAPACITY, "Current Capacity / Max Capacity" },
    { ITEM_TRACKER_NUMBER_AMMO, "Current Ammo / Current Capacity" },
};
static std::unordered_map<int32_t, const char*> itemTrackerKeyTrackOptions = {
    { KEYS_COLLECTED_MAX, "Collected / Max" },
    { KEYS_CURRENT_COLLECTED_MAX, "Current / Collected / Max" },
    { KEYS_CURRENT_MAX, "Current / Max" },
};
static std::unordered_map<int32_t, const char*> itemTrackerTriforcePieceTrackOptions = {
    { TRIFORCE_PIECE_COLLECTED_REQUIRED, "Collected / Required" },
    { TRIFORCE_PIECE_COLLECTED_REQUIRED_MAX, "Collected / Required / Max" },
};
static std::unordered_map<int32_t, const char*> windowTypes = {
    { TRACKER_WINDOW_FLOATING, "Floating" },
    { TRACKER_WINDOW_WINDOW, "Window" },
};
static std::unordered_map<int32_t, const char*> displayModes = {
    { TRACKER_DISPLAY_ALWAYS, "Always" },
    { TRACKER_DISPLAY_COMBO_BUTTON, "Combo Button Hold" },
};
static std::unordered_map<int32_t, const char*> buttons = {
    { TRACKER_COMBO_BUTTON_A, "A" },           { TRACKER_COMBO_BUTTON_B, "B" },
    { TRACKER_COMBO_BUTTON_C_UP, "C-Up" },     { TRACKER_COMBO_BUTTON_C_DOWN, "C-Down" },
    { TRACKER_COMBO_BUTTON_C_LEFT, "C-Left" }, { TRACKER_COMBO_BUTTON_C_RIGHT, "C-Right" },
    { TRACKER_COMBO_BUTTON_L, "L" },           { TRACKER_COMBO_BUTTON_Z, "Z" },
    { TRACKER_COMBO_BUTTON_R, "R" },           { TRACKER_COMBO_BUTTON_START, "Start" },
    { TRACKER_COMBO_BUTTON_D_UP, "D-Up" },     { TRACKER_COMBO_BUTTON_D_DOWN, "D-Down" },
    { TRACKER_COMBO_BUTTON_D_LEFT, "D-Left" }, { TRACKER_COMBO_BUTTON_D_RIGHT, "D-Right" },
};
static std::unordered_map<int32_t, const char*> displayTypes = {
    { SECTION_DISPLAY_HIDDEN, "Hidden" },
    { SECTION_DISPLAY_MAIN_WINDOW, "Main Window" },
    { SECTION_DISPLAY_SEPARATE, "Separate" },
};
static std::unordered_map<int32_t, const char*> extendedDisplayTypes = {
    { SECTION_DISPLAY_EXTENDED_HIDDEN, "Hidden" },
    { SECTION_DISPLAY_EXTENDED_MAIN_WINDOW, "Main Window" },
    { SECTION_DISPLAY_EXTENDED_MISC_WINDOW, "Misc Window" },
    { SECTION_DISPLAY_EXTENDED_SEPARATE, "Separate" },
};
static std::unordered_map<int32_t, const char*> minimalDisplayTypes = {
    { SECTION_DISPLAY_MINIMAL_HIDDEN, "Hidden" }, { SECTION_DISPLAY_MINIMAL_SEPARATE, "Separate" }
};

void ItemTrackerSettingsWindow::DrawElement() {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 8.0f, 8.0f });
    ImGui::BeginTable("itemTrackerSettingsTable", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV);
    ImGui::TableSetupColumn("General settings", ImGuiTableColumnFlags_WidthStretch, 200.0f);
    ImGui::TableSetupColumn("Section settings", ImGuiTableColumnFlags_WidthStretch, 200.0f);
    ImGui::TableHeadersRow();
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    CVarColorPicker("Background Color##gItemTrackerBgColor", CVAR_TRACKER_ITEM("BgColor"), { 0, 0, 0, 0 }, true,
                    ColorPickerRandomButton | ColorPickerResetButton, THEME_COLOR);

    ImGui::PopItemWidth();
    if (CVarCombobox("Window Type", CVAR_TRACKER_ITEM("WindowType"), windowTypes,
                     ComboboxOptions()
                         .DefaultIndex(TRACKER_WINDOW_FLOATING)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_FLOATING) {
        if (CVarCheckbox("Enable Dragging", CVAR_TRACKER_ITEM("Draggable"), CheckboxOptions().Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
        if (CVarCheckbox("Only enable while paused", CVAR_TRACKER_ITEM("ShowOnlyPaused"),
                         CheckboxOptions().Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
        if (CVarCombobox("Display Mode", CVAR_TRACKER_ITEM("DisplayType.Main"), displayModes,
                         ComboboxOptions()
                             .DefaultIndex(TRACKER_DISPLAY_ALWAYS)
                             .ComponentAlignment(ComponentAlignments::Right)
                             .LabelPosition(LabelPositions::Far)
                             .Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Main"), TRACKER_DISPLAY_ALWAYS) ==
            TRACKER_DISPLAY_COMBO_BUTTON) {
            if (CVarCombobox("Combo Button 1", CVAR_TRACKER_ITEM("ComboButton1"), buttons,
                             ComboboxOptions()
                                 .DefaultIndex(TRACKER_COMBO_BUTTON_L)
                                 .ComponentAlignment(ComponentAlignments::Right)
                                 .LabelPosition(LabelPositions::Far)
                                 .Color(THEME_COLOR))) {
                shouldUpdateVectors = true;
            }
            if (CVarCombobox("Combo Button 2", CVAR_TRACKER_ITEM("ComboButton2"), buttons,
                             ComboboxOptions()
                                 .DefaultIndex(TRACKER_COMBO_BUTTON_R)
                                 .ComponentAlignment(ComponentAlignments::Right)
                                 .LabelPosition(LabelPositions::Far)
                                 .Color(THEME_COLOR))) {
                shouldUpdateVectors = true;
            }
        }
    }
    ImGui::Separator();
    CVarSliderInt("Icon size : %dpx", CVAR_TRACKER_ITEM("IconSize"),
                  IntSliderOptions().Min(25).Max(128).DefaultValue(36).Color(THEME_COLOR));
    CVarSliderInt("Icon margins : %dpx", CVAR_TRACKER_ITEM("IconSpacing"),
                  IntSliderOptions().Min(-5).Max(50).DefaultValue(12).Color(THEME_COLOR));
    CVarSliderInt("Text size : %dpx", CVAR_TRACKER_ITEM("TextSize"),
                  IntSliderOptions().Min(1).Max(30).DefaultValue(13).Color(THEME_COLOR));

    ImGui::NewLine();
    CVarCombobox("Ammo/Capacity Tracking", CVAR_TRACKER_ITEM("ItemCountType"), itemTrackerCapacityTrackOptions,
                 ComboboxOptions()
                     .DefaultIndex(ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY)
                     .ComponentAlignment(ComponentAlignments::Left)
                     .LabelPosition(LabelPositions::Above)
                     .Color(THEME_COLOR)
                     .Tooltip("Customize what the numbers under each item are tracking."
                              "\n\nNote: items without capacity upgrades will track ammo even in capacity mode"));
    if (CVarGetInteger(CVAR_TRACKER_ITEM("ItemCountType"), ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY) ==
            ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY ||
        CVarGetInteger(CVAR_TRACKER_ITEM("ItemCountType"), ITEM_TRACKER_NUMBER_CURRENT_CAPACITY_ONLY) ==
            ITEM_TRACKER_NUMBER_CURRENT_AMMO_ONLY) {
        if (CVarCheckbox("Align count to left side", CVAR_TRACKER_ITEM("ItemCountAlignLeft"),
                         CheckboxOptions().Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
    }

    CVarCombobox("Key Count Tracking", CVAR_TRACKER_ITEM("KeyCounts"), itemTrackerKeyTrackOptions,
                 ComboboxOptions()
                     .DefaultIndex(KEYS_COLLECTED_MAX)
                     .ComponentAlignment(ComponentAlignments::Left)
                     .LabelPosition(LabelPositions::Above)
                     .Color(THEME_COLOR)
                     .Tooltip("Customize what numbers are shown for key tracking."));

    CVarCombobox("Triforce Piece Count Tracking", CVAR_TRACKER_ITEM("TriforcePieceCounts"),
                 itemTrackerTriforcePieceTrackOptions,
                 ComboboxOptions()
                     .DefaultIndex(TRIFORCE_PIECE_COLLECTED_REQUIRED_MAX)
                     .ComponentAlignment(ComponentAlignments::Left)
                     .LabelPosition(LabelPositions::Above)
                     .Color(THEME_COLOR)
                     .Tooltip("Customize what numbers are shown for triforce piece tracking."));

    ImGui::TableNextColumn();

    if (CVarCombobox("Inventory", CVAR_TRACKER_ITEM("DisplayType.Inventory"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_MAIN_WINDOW)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }
    if (CVarCombobox("Equipment", CVAR_TRACKER_ITEM("DisplayType.Equipment"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_MAIN_WINDOW)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }
    if (CVarCombobox("Misc", CVAR_TRACKER_ITEM("DisplayType.Misc"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_MAIN_WINDOW)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }
    if (CVarCombobox("Dungeon Rewards", CVAR_TRACKER_ITEM("DisplayType.DungeonRewards"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_MAIN_WINDOW)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonRewards"), SECTION_DISPLAY_MAIN_WINDOW) ==
        SECTION_DISPLAY_SEPARATE) {
        if (CVarCheckbox("Circle display", CVAR_TRACKER_ITEM("DungeonRewardsLayout"),
                         CheckboxOptions().DefaultValue(false).Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
    }
    if (CVarCombobox("Songs", CVAR_TRACKER_ITEM("DisplayType.Songs"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_MAIN_WINDOW)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }
    if (CVarCombobox("Dungeon Items", CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }
    if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), SECTION_DISPLAY_HIDDEN) !=
        SECTION_DISPLAY_HIDDEN) {
        if (CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.DungeonItems"), SECTION_DISPLAY_HIDDEN) ==
            SECTION_DISPLAY_SEPARATE) {
            if (CVarCheckbox("Horizontal display", CVAR_TRACKER_ITEM("DungeonItems.Layout"),
                             CheckboxOptions().DefaultValue(true).Color(THEME_COLOR))) {
                shouldUpdateVectors = true;
            }
        }
        if (CVarCheckbox("Maps and compasses", CVAR_TRACKER_ITEM("DungeonItems.DisplayMaps"),
                         CheckboxOptions().DefaultValue(true).Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
    }
    if (CVarCombobox("Greg", CVAR_TRACKER_ITEM("DisplayType.Greg"), extendedDisplayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_EXTENDED_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarCombobox("Triforce Pieces", CVAR_TRACKER_ITEM("DisplayType.TriforcePieces"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarCombobox("Boss Souls", CVAR_TRACKER_ITEM("DisplayType.BossSouls"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarCombobox("Ocarina Buttons", CVAR_TRACKER_ITEM("DisplayType.OcarinaButtons"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarCombobox("Overworld Keys", CVAR_TRACKER_ITEM("DisplayType.OverworldKeys"), displayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarCombobox("Fishing Pole", CVAR_TRACKER_ITEM("DisplayType.FishingPole"), extendedDisplayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_EXTENDED_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarCombobox("Total Checks", "gTrackers.ItemTracker.TotalChecks.DisplayType", minimalDisplayTypes,
                     ComboboxOptions()
                         .DefaultIndex(SECTION_DISPLAY_MINIMAL_HIDDEN)
                         .ComponentAlignment(ComponentAlignments::Right)
                         .LabelPosition(LabelPositions::Far)
                         .Color(THEME_COLOR))) {
        shouldUpdateVectors = true;
    }

    if (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_WINDOW ||
        (CVarGetInteger(CVAR_TRACKER_ITEM("WindowType"), TRACKER_WINDOW_FLOATING) == TRACKER_WINDOW_FLOATING &&
         CVarGetInteger(CVAR_TRACKER_ITEM("DisplayType.Main"), TRACKER_DISPLAY_ALWAYS) !=
             TRACKER_DISPLAY_COMBO_BUTTON)) {
        if (CVarCombobox("Personal notes", CVAR_TRACKER_ITEM("DisplayType.Notes"), displayTypes,
                         ComboboxOptions()
                             .DefaultIndex(SECTION_DISPLAY_HIDDEN)
                             .ComponentAlignment(ComponentAlignments::Right)
                             .LabelPosition(LabelPositions::Far)
                             .Color(THEME_COLOR))) {
            shouldUpdateVectors = true;
        }
    }
    CVarCheckbox("Show Hookshot Identifiers", CVAR_TRACKER_ITEM("HookshotIdentifier"),
                 CheckboxOptions()
                     .Tooltip("Shows an 'H' or an 'L' to more easiely distinguish between Hookshot and Longshot.")
                     .Color(THEME_COLOR));

    ImGui::PopStyleVar(1);
    ImGui::EndTable();
}

void ItemTrackerWindow::InitElement() {
    // Crashes when the itemTrackerNotes is empty, so add an empty character to it
    if (itemTrackerNotes.empty()) {
        itemTrackerNotes.push_back(0);
    }

    SaveManager::Instance->AddInitFunction(ItemTrackerInitFile);
    itemTrackerSectionId = SaveManager::Instance->AddSaveFunction("itemTrackerData", 1, ItemTrackerSaveFile, true, -1);
    SaveManager::Instance->AddLoadFunction("itemTrackerData", 1, ItemTrackerLoadFile);

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameFrameUpdate>(ItemTrackerOnFrame);
}
