#include "actorViewer.h"
#include "../../util.h"
#include "soh/SohGui/UIWidgets.hpp"
#include "soh/SohGui/SohGui.hpp"
#include "soh/ActorDB.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/nametag.h"
#include "soh/ShipInit.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <map>
#include <unordered_map>
#include <string>
#include <libultraship/bridge.h>
#include <libultraship/libultraship.h>
#include <spdlog/fmt/fmt.h>
#include "soh/OTRGlobals.h"
#include "soh/cvar_prefixes.h"
#include "soh/ObjectExtension/ActorListIndex.h"

extern "C" {
#include <z64.h>
#include "z64math.h"
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;

#include "textures/icon_item_static/icon_item_static.h"
#include "textures/icon_item_24_static/icon_item_24_static.h"
}

#define DEKUNUTS_FLOWER 10
#define DEBUG_ACTOR_NAMETAG_TAG "debug_actor_viewer"

#define CVAR_ACTOR_NAME_TAGS(val) CVAR_DEVELOPER_TOOLS("ActorViewer.NameTags." val)
#define CVAR_ACTOR_NAME_TAGS_ENABLED_NAME CVAR_ACTOR_NAME_TAGS("Enabled")
#define CVAR_ACTOR_NAME_TAGS_ENABLED CVarGetInteger(CVAR_ACTOR_NAME_TAGS("Enabled"), 0)

typedef struct {
    u16 id;
    u16 params;
    Vec3f pos;
    Vec3s rot;
} ActorInfo;

std::array<const char*, 12> acMapping = {
    "Switch",      "Background (Prop type 1)",
    "Player",      "Bomb",
    "NPC",         "Enemy",
    "Prop type 2", "Item/Action",
    "Misc.",       "Boss",
    "Door",        "Chest",
};

using namespace UIWidgets;

typedef enum {
    ACTORVIEWER_NAMETAGS_NONE,
    ACTORVIEWER_NAMETAGS_DESC,
    ACTORVIEWER_NAMETAGS_NAME,
    ACTORVIEWER_NAMETAGS_BOTH,
} ActorViewerNameTagsType;

const std::string GetActorDescription(u16 id) {
    return ActorDB::Instance->RetrieveEntry(id).entry.valid ? ActorDB::Instance->RetrieveEntry(id).entry.desc : "???";
}

const std::string GetActorDebugName(u16 id) {
    return ActorDB::Instance->RetrieveEntry(id).entry.valid ? ActorDB::Instance->RetrieveEntry(id).entry.name : "???";
}

template <typename T> void DrawGroupWithBorder(T&& drawFunc, std::string section) {
    // First group encapsulates the inner portion and border
    ImGui::BeginChild(std::string("##" + section).c_str(), ImVec2(0, 0),
                      ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX |
                          ImGuiChildFlags_AutoResizeY);

    // Second group encapsulates just the inner portion
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    drawFunc();
    ImGui::EndGroup();

    ImGui::EndChild();
}

void PopulateActorDropdown(int i, std::vector<Actor*>& data) {
    if (data.size() != 0) {
        data.clear();
    }
    if (gPlayState != nullptr) {
        ActorListEntry currList = gPlayState->actorCtx.actorLists[i];
        Actor* currAct = currList.head;
        if (currAct != nullptr) {
            while (currAct != nullptr) {
                data.push_back(currAct);
                currAct = currAct->next;
            }
        }
    }
}

// actors that don't use params at all
static std::vector<u16> noParamsActors = {
    ACTOR_ARMS_HOOK,
    ACTOR_ARROW_FIRE,
    ACTOR_ARROW_ICE,
    ACTOR_ARROW_LIGHT,
    ACTOR_BG_BOM_GUARD,
    ACTOR_BG_DY_YOSEIZO,
    ACTOR_BG_GATE_SHUTTER,
    ACTOR_BG_GJYO_BRIDGE,
    ACTOR_BG_HIDAN_FSLIFT,
    ACTOR_BG_HIDAN_RSEKIZOU,
    ACTOR_BG_HIDAN_SYOKU,
    ACTOR_BG_JYA_GOROIWA,
    ACTOR_BG_MIZU_UZU,
    ACTOR_BG_MORI_RAKKATENJO,
    ACTOR_BG_PUSHBOX,
    ACTOR_BG_SPOT01_FUSYA,
    ACTOR_BG_SPOT01_IDOHASHIRA,
    ACTOR_BG_SPOT01_IDOMIZU,
    ACTOR_BG_SPOT01_IDOSOKO,
    ACTOR_BG_SPOT11_OASIS,
    ACTOR_BG_SPOT15_SAKU,
    ACTOR_BG_SPOT18_FUTA,
    ACTOR_BG_TOKI_SWD,
    ACTOR_BG_TREEMOUTH,
    ACTOR_BG_VB_SIMA,
    ACTOR_BOSS_DODONGO,
    ACTOR_BOSS_FD,
    ACTOR_BOSS_GOMA,
    ACTOR_DEMO_EXT,
    ACTOR_DEMO_SHD,
    ACTOR_DEMO_TRE_LGT,
    ACTOR_DOOR_TOKI,
    ACTOR_EFC_ERUPC,
    ACTOR_EN_ANI,
    ACTOR_EN_AROW_TRAP,
    ACTOR_EN_BIRD,
    ACTOR_EN_BLKOBJ,
    ACTOR_EN_BOM_BOWL_MAN,
    ACTOR_EN_BOM_BOWL_PIT,
    ACTOR_EN_BOM_CHU,
    ACTOR_EN_BUBBLE,
    ACTOR_EN_DIVING_GAME,
    ACTOR_EN_DNT_DEMO,
    ACTOR_EN_DNT_JIJI,
    ACTOR_EN_DS,
    ACTOR_EN_DU,
    ACTOR_EN_EG,
    ACTOR_EN_FU,
    ACTOR_EN_GB,
    ACTOR_EN_GE3,
    ACTOR_EN_GUEST,
    ACTOR_EN_HATA,
    ACTOR_EN_HORSE_GANON,
    ACTOR_EN_HORSE_LINK_CHILD,
    ACTOR_EN_HORSE_ZELDA,
    ACTOR_EN_HS2,
    ACTOR_EN_JS,
    ACTOR_EN_KAKASI,
    ACTOR_EN_KAKASI3,
    ACTOR_EN_MA1,
    ACTOR_EN_MA2,
    ACTOR_EN_MA3,
    ACTOR_EN_MAG,
    ACTOR_EN_MK,
    ACTOR_EN_MS,
    ACTOR_EN_NIW_LADY,
    ACTOR_EN_NWC,
    ACTOR_EN_OE2,
    ACTOR_EN_OKARINA_EFFECT,
    ACTOR_EN_RR,
    ACTOR_EN_SA,
    ACTOR_EN_SCENE_CHANGE,
    ACTOR_EN_SKJNEEDLE,
    ACTOR_EN_SYATEKI_ITM,
    ACTOR_EN_SYATEKI_MAN,
    ACTOR_EN_TAKARA_MAN,
    ACTOR_EN_TORYO,
    ACTOR_EN_VASE,
    ACTOR_EN_ZL1,
    ACTOR_MAGIC_DARK,
    ACTOR_MAGIC_FIRE,
    ACTOR_OBJ_DEKUJR,
    ACTOR_OCEFF_SPOT,

    ACTOR_UNSET_1,
    ACTOR_UNSET_3,
    ACTOR_UNSET_5,
    ACTOR_UNSET_6,
    ACTOR_UNSET_17,
    ACTOR_UNSET_1A,
    ACTOR_UNSET_1F,
    ACTOR_UNSET_22,
    ACTOR_UNSET_31,
    ACTOR_UNSET_36,
    ACTOR_UNSET_53,
    ACTOR_UNSET_73,
    ACTOR_UNSET_74,
    ACTOR_UNSET_75,
    ACTOR_UNSET_76,
    ACTOR_UNSET_78,
    ACTOR_UNSET_79,
    ACTOR_UNSET_7A,
    ACTOR_UNSET_7B,
    ACTOR_UNSET_7E,
    ACTOR_UNSET_7F,
    ACTOR_UNSET_83,
    ACTOR_UNSET_A0,
    ACTOR_UNSET_B2,
    ACTOR_UNSET_CE,
    ACTOR_UNSET_D8,
    ACTOR_UNSET_EA,
    ACTOR_UNSET_EB,
    ACTOR_UNSET_F2,
    ACTOR_UNSET_F3,
    ACTOR_UNSET_FB,
    ACTOR_UNSET_109,
    ACTOR_UNSET_10D,
    ACTOR_UNSET_10E,
    ACTOR_UNSET_128,
    ACTOR_UNSET_129,
    ACTOR_UNSET_134,
    ACTOR_UNSET_154,
    ACTOR_UNSET_15D,
    ACTOR_UNSET_161,
    ACTOR_UNSET_180,
    ACTOR_UNSET_1AA,
};

static std::unordered_map<u16, std::function<s16(s16)>> actorSpecificData;

void CreateActorSpecificData() {
    if (!actorSpecificData.empty()) {
        return;
    }

    actorSpecificData[ACTOR_EN_DEKUNUTS] = [](s16 params) -> s16 {
        bool isFlower = params == DEKUNUTS_FLOWER;
        s16 shotsPerRound = (params >> 8) & 0xFF;
        if (shotsPerRound == 0xFF || shotsPerRound == 0) {
            shotsPerRound = 1;
        }
        ImGui::Checkbox("Flower", &isFlower);
        if (!isFlower) {
            ImGui::InputScalar("Shots Per Round", ImGuiDataType_S16, &shotsPerRound);
        }

        return isFlower ? DEKUNUTS_FLOWER : (shotsPerRound << 8);
    };

    actorSpecificData[ACTOR_EN_TITE] = [](s16 params) -> s16 {
        static const char* items[] = { "Blue", "Red" };
        if (params == 0) {
            params = -2;
        }
        // the + 2 is because the params are -2 & -1 instead of 0 & 1
        int selectedItem = params + 2;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem - 2;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_AM] = [](s16 params) -> s16 {
        static const char* items[] = { "Statue", "Enemy" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_BG_ICE_TURARA] = [](s16 params) -> s16 {
        static const char* items[] = { "Stalagmite", "Stalactite", "Stalactite (Regrow)" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_BG_BREAKWALL] = [](s16 params) -> s16 {
        static const char* items[] = { "DC Entrance", "Wall", "KD Floor", "KD Lava Cover" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_TEST] = [](s16 params) -> s16 {
        static const char* items[] = { "Invisible", "1", "2", "Ceiling", "4", "5" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_TANA] = [](s16 params) -> s16 {
        static const char* items[] = { "Wooden", "Stone (1)", "Stone (2)" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_XC] = [](s16 params) -> s16 {
        static const char* items[] = { "0", "1", "2", "3", "4", "5", "Minuet", "Bolero", "Serenade", "9" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_SHOT_SUN] = [](s16 params) -> s16 {
        static const char* items[] = { "Sun's Song", "Song of Storms", "LH Sun" };
        if (params == 0) {
            params = 0x40;
        }
        // the - 0x40 is because the params are 0x40 & 0x41 instead of 0 & 1
        int selectedItem = params - 0x40;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem + 0x40;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_HONOTRAP] = [](s16 params) -> s16 {
        static const char* items[] = { "Eye", "Flame Move", "Flame Drop" };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_REEBA] = [](s16 params) -> s16 {
        bool isBig = params != 0;
        ImGui::Checkbox("Big", &isBig);

        return isBig;
    };

    actorSpecificData[ACTOR_EN_TK] = [](s16 params) -> s16 {
        bool canTurn = params >= 0;
        ImGui::Checkbox("Can Turn", &canTurn);

        return canTurn ? 0 : -1;
    };

    actorSpecificData[ACTOR_EN_ITEM00] = [](s16 params) -> s16 {
        bool autoCollect = params & 0x8000;
        ImGui::Checkbox("Automatically Collect", &autoCollect);
        u8 collectibleFlag = (params & 0x3F00) >> 8;
        ImGui::InputScalar("Collectible Flag", ImGuiDataType_U8, &collectibleFlag);
        if (collectibleFlag > 0x3F) {
            collectibleFlag = 0x3F;
        }

        static const char* items[] = {
            "Green Rupee",   "Blue Rupee",      "Red Rupee",     "Recovery Heart", "Bombs (A)",      "Arrow",
            "Heart Piece",   "Heart Container", "Arrows (5)",    "Arrows (10)",    "Arrows (30)",    "Bombs (B)",
            "Deku Nuts (5)", "Deku Stick",      "Magic (Large)", "Magic (Small)",  "Deku Seeds (5)", "Small Key",
            "Flexible",      "Gold Rupee",      "Purple Rupee",  "Deku Shield",    "Hylian Shield",  "Zora Tunic",
            "Goron Tunic",   "Bombs (Special)", "Bombchus",
        };

        int selectedItem = params & 0xFF;
        ImGui::Combo("Item", &selectedItem, items, IM_ARRAYSIZE(items));

        return autoCollect * 0x8000 + (collectibleFlag << 8) + selectedItem;
    };

    actorSpecificData[ACTOR_OBJ_COMB] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Green Rupee",   "Blue Rupee",      "Red Rupee",     "Recovery Heart", "Bombs (A)",      "Arrow",
            "Heart Piece",   "Heart Container", "Arrows (5)",    "Arrows (10)",    "Arrows (30)",    "Bombs (B)",
            "Deku Nuts (5)", "Deku Stick",      "Magic (Large)", "Magic (Small)",  "Deku Seeds (5)", "Small Key",
            "Flexible",      "Gold Rupee",      "Purple Rupee",  "Deku Shield",    "Hylian Shield",  "Zora Tunic",
            "Goron Tunic",   "Bombs (Special)", "Bombchus",
        };

        int selectedItem = params & 0xFF;
        ImGui::Combo("Item Drop", &selectedItem, items, IM_ARRAYSIZE(items));

        u8 collectibleFlag = (params & 0x3F00) >> 8;
        if (selectedItem == 6) {
            ImGui::InputScalar("PoH Collectible Flag", ImGuiDataType_U8, &collectibleFlag);
            if (collectibleFlag > 0x3F) {
                collectibleFlag = 0x3F;
            }
        }

        return (collectibleFlag << 8) + selectedItem;
    };

    actorSpecificData[ACTOR_EN_GM] = [](s16 params) -> s16 {
        u8 switchFlag = (params & 0x3F00) >> 8;

        ImGui::InputScalar("Switch Flag", ImGuiDataType_U8, &switchFlag);
        if (switchFlag > 0x3F) {
            switchFlag = 0x3F;
        }

        return switchFlag << 8;
    };

    actorSpecificData[ACTOR_EN_GIRLA] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Deku Nuts (5)",
            "Arrows (30)",
            "Arrows (50)",
            "Bombs (5) (25 Rupees)",
            "Deku Nuts (10)",
            "Deku Stick",
            "Bombs (10)",
            "Fish",
            "Red Potion (30 Rupees)",
            "Green Potion",
            "Blue Potion",
            "Longsword",
            "Hylian Shield",
            "Deku Shield",
            "Goron Tunic",
            "Zora Tunic",
            "Heart",
            "Milk Bottle",
            "Weird Egg",
            "19",
            "20",
            "Bomchu (10) [1]",
            "Bomchu (20) [1]",
            "Bomchu (20) [2]",
            "Bomchu (10) [2]",
            "Bomchu (10) [3]",
            "Bomchu (20) [3]",
            "Bomchu (20) [4]",
            "Bomchu (10) [4]",
            "Deku Seeds (30)",
            "Keaton Mask",
            "Spooky Mask",
            "Skull Mask",
            "Bunny Hood",
            "Mask Of Truth",
            "Zora Mask",
            "Goron Mask",
            "Gerudo Mask",
            "Sold Out",
            "Blue Fire",
            "Bugs",
            "Big Poe",
            "Poe",
            "Fairy",
            "Arrows (10)",
            "Bombs (20)",
            "Bombs (30)",
            "Bombs (5) (35 Rupees)",
            "Red Potion (40 Rupees)",
            "Red Potion (50 Rupees)",
            "Randomizer Item",
        };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_FIRE_ROCK] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Spawned Falling (1)",
            "Broken Piece (1)",
            "Broken Piece (2)",
            "Spawned Falling (2)",
            //"INVALID",
            "Ceiling Spot Spawner",
            "On Floor",
        };
        int selectedItem = params > 3 ? params - 1 : params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem > 3 ? selectedItem + 1 : selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_EX_ITEM] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Bomb Bag Bowling",
            "Heart Piece Bowling",
            "Bombchus Bowling",
            "Bombs Bowling",
            "Purple Rupee Bowling",
            "Bomb Bag Counter",
            "Heart Piece Counter",
            "Bombchus Counter",
            "Bombs Counter",
            "Purple Rupee Counter",
            "Green Rupee Chest",
            "Blue Rupee Chest",
            "Red Rupee Chest",
            "13",
            "14",
            "Small Key Chest",
            "Magic Fire",
            "Magic Wind",
            "Magic Dark",
            "Bullet Bag",
        };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_ELF] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Navi", "Revive Bottle", "Heal Timed", "Kokiri", "Spawner", "Revive Death", "Heal", "Heal Big",
        };
        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_CLEAR_TAG] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Cutscene", // 0
            "Normal",   // 1
            "Laser",    // 100
        };
        int selectedItem = params == 100 ? 2 : params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem == 2 ? 100 : selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_BOMBF] = [](s16 params) -> s16 {
        static const char* items[] = { "Flower", "Body", "Explosion" };
        // the + 1 is because the params are -1, 0 & 1 instead of 0, 1 & 2
        int selectedItem = params + 1;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem - 1;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_BOM] = [](s16 params) -> s16 {
        static const char* items[] = { "Body", "Explosion" };

        int selectedItem = params;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_DOOR_WARP1] = [](s16 params) -> s16 {
        static const char* items[] = {
            "Blue Crystal", // -2
            "Dungeon Adult",  "Dungeon Child",
            "Clear Flag", // Activate on temp clear flag
            "Sages",      // Used by sages warping into chamber of sages during their cutscene
            "Purple Crystal",
            "Yellow", // The colored variants don't warp, they are cutscene setpieces
            "Blue Ruto",
            "Destination", // Spawning in after having taken a warp
            "UNK 7",          "Orange",        "Green", "Red",
        };
        int selectedItem = params + 2;
        if (ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem - 2;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_DY_EXTRA] = [](s16 params) -> s16 {
        static const char* items[] = { "Orange", "Green" };

        int selectedItem = params;
        if (ImGui::Combo("Color", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };

    actorSpecificData[ACTOR_EN_SKB] = [](s16 params) -> s16 {
        u8 size = params;
        ImGui::InputScalar("Size", ImGuiDataType_U8, &size);

        return size;
    };

    actorSpecificData[ACTOR_EN_WF] = [](s16 params) -> s16 {
        static const char* items[] = { "Normal", "White" };

        int selectedItem = params;
        ImGui::Combo("Type", &selectedItem, items, IM_ARRAYSIZE(items));

        u8 switchFlag = (params & 0x3F00) >> 8;
        ImGui::InputScalar("Switch Flag", ImGuiDataType_U8, &switchFlag);
        return (switchFlag << 8) + selectedItem;
    };

    actorSpecificData[ACTOR_EN_BOX] = [](s16 params) -> s16 {
        /*
        trasureFlag = params & 0x1F;        //0b0000 0000 0001 1111
        itemId      = (params >> 5) & 0x7F; //0b0000 1111 1110 0000
        type        = (params >> 12) & 0xF; //0b1111 0000 0000 0000
        */
        u8 treasureFlag = params & 0x1F;
        ImGui::InputScalar("Treasure Flag", ImGuiDataType_U8, &treasureFlag);
        if (treasureFlag > 0x1F) {
            treasureFlag = 0x1F;
        }

        u8 itemId = (params >> 5) & 0x7F;
        ImGui::InputScalar("Item Id", ImGuiDataType_U8, &itemId);
        if (itemId > 0x7F) {
            itemId = 0x7F;
        }

        static const char* items[] = {
            "Big (Default)",
            "Room Clear Big",
            "Decorated Big",
            "Switch Flag Fall Big",
            "4",
            "Small",
            "6",
            "Room Clear Small",
            "Switch Flag Fall Small",
            "9",
            "10",
            "Switch Flag Big",
        };

        int type = (params >> 12) & 0xF;
        ImGui::Combo("Type", &type, items, IM_ARRAYSIZE(items));
        if (type > 0xF) {
            type = 0xF;
        }

        return (type << 12) + (itemId << 5) + treasureFlag;
    };

    actorSpecificData[ACTOR_EN_DOOR] = [](s16 params) -> s16 {
        /**
         * Actor Parameters
         *
         * |                  |       |             |
         * | Transition Index | Type  | Double Door | Switch Flag OR Text Id - 0x0200
         * |------------------|-------|-------------|---------------------------------
         * | 0 0 0 0 0 0      | 0 0 0 | 0           | 0 0 0 0 0 0
         * | 6                | 3     | 1           | 6
         * |
         *
         * Transition Index     1111110000000000    Set by the actor engine when the door is spawned
         * Type                 0000001110000000
         * Double Door          0000000001000000
         * Switch Flag          0000000000111111    For use with the `DOOR_LOCKED` type
         * Text id - 0x0200     0000000000111111    For use with the `DOOR_CHECKABLE` type
         *
         */
        u8 transitionIndex = params >> 10;
        ImGui::InputScalar("Transition Index", ImGuiDataType_U8, &transitionIndex);
        if (transitionIndex > 0x3F) {
            transitionIndex = 0x3F;
        }

        static const char* items[] = {
            "Room Load",     // loads rooms
            "Locked",        // small key locked door
            "Room Load (2)", // loads rooms
            "Scene Exit",    // doesn't load rooms, used for doors paired with scene transition polygons
            "Ajar",          // open slightly but slams shut if Link gets too close
            "Checkable",     // doors that display a textbox when interacting
            "Evening",       // unlocked between 18:00 and 21:00, Dampé's hut
            "Room Load (7)"  // loads rooms
        };

        int type = (params >> 7) & 7;
        ImGui::Combo("Type", &type, items, IM_ARRAYSIZE(items));
        if (type > 7) {
            type = 7;
        }

        bool doubleDoor = ((params >> 6) & 1) != 0;
        ImGui::Checkbox("Double Door", &doubleDoor);

        u8 lowerBits = params & 0x3F;
        if (type == 1) {
            ImGui::InputScalar("Switch Flag", ImGuiDataType_U8, &lowerBits);
            if (lowerBits > 0x3F) {
                lowerBits = 0x3F;
            }
        } else if (type == 5) {
            ImGui::InputScalar("Text ID - 0x200", ImGuiDataType_U8, &lowerBits);
            if (lowerBits > 0x3F) {
                lowerBits = 0x3F;
            }
        } else {
            lowerBits = 0;
        }

        return (transitionIndex << 10) + (type << 7) + (doubleDoor << 6) + lowerBits;
    };

    actorSpecificData[ACTOR_EN_PO_DESERT] = [](s16 params) -> s16 {
        u8 switchFlag = params >> 8;

        ImGui::InputScalar("Path", ImGuiDataType_U8, &switchFlag);

        return switchFlag << 8;
    };

    actorSpecificData[ACTOR_EN_KANBAN] = [](s16 params) -> s16 {
        bool piece = params == (s16)0xFFDD;
        bool fishingSign = params == 0x300;
        if (ImGui::Checkbox("Piece", &piece)) {
            fishingSign = false;
        }
        if (ImGui::Checkbox("Fishing Sign", &fishingSign)) {
            piece = false;
        }

        u8 textId = params;
        if (!piece && !fishingSign) {
            if (ImGui::InputScalar("Text ID", ImGuiDataType_U8, &textId)) {
                textId |= 0x300;
            }
        }

        return piece ? (s16)0xFFDD : (fishingSign ? 0x300 : textId);
    };

    actorSpecificData[ACTOR_EN_KUSA] = [](s16 params) -> s16 {
        static const char* items[] = { "0", "1", "2" };

        int type = params & 3;
        ImGui::Combo("Type", &type, items, IM_ARRAYSIZE(items));

        bool bugs = ((params >> 4) & 1) != 0;
        ImGui::Checkbox("Bugs", &bugs);

        u8 drop = (params >> 8) & 0xF;
        if (type == 2) {
            ImGui::InputScalar("Random Drop Params", ImGuiDataType_U8, &drop);
            if (drop > 0xD) {
                drop = 0xD;
            }
        } else {
            drop = 0;
        }

        return (drop << 8) + (bugs << 4) + type;
    };

    actorSpecificData[ActorDB::Instance->RetrieveId("En_Partner")] = [](s16 params) -> s16 {
        static const char* items[] = { "Port 1", "Port 2", "Port 3", "Port 4" };
        int selectedItem = params;
        if (ImGui::Combo("Controller Port", &selectedItem, items, IM_ARRAYSIZE(items))) {
            return selectedItem;
        }

        return params;
    };
}

std::vector<u16> GetActorsWithDescriptionContainingString(std::string s) {
    std::locale loc;
    for (size_t i = 0; i < s.length(); i += 1) {
        s[i] = std::tolower(s[i], loc);
    }

    std::vector<u16> actors;
    for (int i = 0; i < ActorDB::Instance->GetEntryCount(); i += 1) {
        ActorDB::Entry actorEntry = ActorDB::Instance->RetrieveEntry(i);
        std::string desc = actorEntry.desc;
        for (size_t j = 0; j < desc.length(); j += 1) {
            desc[j] = std::tolower(desc[j], loc);
        }
        if (desc.find(s) != std::string::npos) {
            actors.push_back((u16)i);
        }
    }
    return actors;
}

void ActorViewer_AddTagForActor(Actor* actor) {
    if (!CVarGetInteger(CVAR_ACTOR_NAME_TAGS("Enabled"), 0)) {
        return;
    }

    std::vector<std::string> parts;

    if (CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayID"), 0)) {
        parts.push_back(GetActorDebugName(actor->id));
    }
    if (CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayDescription"), 0)) {
        parts.push_back(GetActorDescription(actor->id));
    }
    if (CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayCategory"), 0)) {
        parts.push_back(acMapping[actor->category]);
    }
    if (CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayParams"), 0)) {
        parts.push_back(fmt::format("0x{:04X} ({})", (u16)actor->params, actor->params));
    }

    std::string tag = "";
    for (size_t i = 0; i < parts.size(); i++) {
        if (i != 0) {
            tag += "\n";
        }
        tag += parts.at(i);
    }

    bool withZBuffer = CVarGetInteger(CVAR_ACTOR_NAME_TAGS("WithZBuffer"), 0);

    NameTag_RegisterForActorWithOptions(actor, tag.c_str(),
                                        { .tag = DEBUG_ACTOR_NAMETAG_TAG, .noZBuffer = !withZBuffer });
}

void ActorViewer_AddTagForAllActors() {
    if (gPlayState == nullptr) {
        return;
    }

    for (size_t i = 0; i < ARRAY_COUNT(gPlayState->actorCtx.actorLists); i++) {
        ActorListEntry currList = gPlayState->actorCtx.actorLists[i];
        Actor* currAct = currList.head;
        while (currAct != nullptr) {
            ActorViewer_AddTagForActor(currAct);
            currAct = currAct->next;
        }
    }
}

void ActorViewerWindow::DrawElement() {
    ImGui::BeginDisabled(CVarGetInteger(CVAR_SETTING("DisableChanges"), 0));
    static ActorInfo newActor = { 0, 0, { 0, 0, 0 }, { 0, 0, 0 } };
    static ImU16 one = 1;
    static std::string filler = "Please select";
    static std::string searchString = "";
    static s16 currentSelectedInDropdown = -1;
    static std::vector<u16> actorSearchResults;

    if (gPlayState != nullptr) {
        if (ImGui::BeginChild("options", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {
            bool toggled = false;
            bool optionChange = false;

            ImGui::SeparatorText("Options");

            toggled = UIWidgets::CVarCheckbox("Actor Name Tags", CVAR_ACTOR_NAME_TAGS("Enabled"),
                                              { { .tooltip = "Adds \"name tags\" above actors for identification" } });

            ImGui::SameLine();

            UIWidgets::Button("Display Items", { { .tooltip = "Click to add display items on the name tags" } });

            if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft | ImGuiPopupFlags_NoReopen)) {
                optionChange |= UIWidgets::CVarCheckbox("ID", CVAR_ACTOR_NAME_TAGS("DisplayID"));
                optionChange |= UIWidgets::CVarCheckbox("Description", CVAR_ACTOR_NAME_TAGS("DisplayDescription"));
                optionChange |= UIWidgets::CVarCheckbox("Category", CVAR_ACTOR_NAME_TAGS("DisplayCategory"));
                optionChange |= UIWidgets::CVarCheckbox("Params", CVAR_ACTOR_NAME_TAGS("DisplayParams"));

                ImGui::EndPopup();
            }

            optionChange |= UIWidgets::CVarCheckbox(
                "Name tags with Z-Buffer", CVAR_ACTOR_NAME_TAGS("WithZBuffer"),
                { { .tooltip = "Allow name tags to be obstructed when behind geometry and actors" } });

            if (toggled || optionChange) {
                bool tagsEnabled = CVarGetInteger(CVAR_ACTOR_NAME_TAGS("Enabled"), 0);
                bool noOptionsEnabled = !CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayID"), 0) &&
                                        !CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayDescription"), 0) &&
                                        !CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayCategory"), 0) &&
                                        !CVarGetInteger(CVAR_ACTOR_NAME_TAGS("DisplayParams"), 0);

                // Save the user an extra click and prevent adding "empty" tags by enabling,
                // disabling, or setting an option based on what changed
                if (tagsEnabled && noOptionsEnabled) {
                    if (toggled) {
                        CVarSetInteger(CVAR_ACTOR_NAME_TAGS("DisplayID"), 1);
                    } else {
                        CVarSetInteger(CVAR_ACTOR_NAME_TAGS("Enabled"), 0);
                    }
                } else if (optionChange && !tagsEnabled && !noOptionsEnabled) {
                    CVarSetInteger(CVAR_ACTOR_NAME_TAGS("Enabled"), 1);
                }

                NameTag_RemoveAllByTag(DEBUG_ACTOR_NAMETAG_TAG);
                ActorViewer_AddTagForAllActors();
            }
        }
        ImGui::EndChild();

        PushStyleCombobox(THEME_COLOR);
        if (ImGui::BeginCombo("Actor Type", acMapping[category])) {
            for (int i = 0; i < acMapping.size(); i++) {
                if (ImGui::Selectable(acMapping[i])) {
                    category = i;
                    PopulateActorDropdown(category, list);
                    break;
                }
            }
            ImGui::EndCombo();
        }

        if (display == nullptr) {
            filler = "Please select";
        }

        if (ImGui::BeginCombo("Actor", filler.c_str())) {
            for (int i = 0; i < list.size(); i++) {
                std::string label = std::to_string(i) + ": " + ActorDB::Instance->RetrieveEntry(list[i]->id).name;
                std::string description = GetActorDescription(list[i]->id);
                if (description != "")
                    label += " (" + description + ")";

                if (ImGui::Selectable(label.c_str(), list[i] == display)) {
                    display = list[i];
                    filler = label;
                    break;
                }
            }
            ImGui::EndCombo();
        }
        PopStyleCombobox();

        PushStyleHeader(THEME_COLOR);
        if (ImGui::TreeNode("Selected Actor")) {
            if (display != nullptr) {
                DrawGroupWithBorder(
                    [&]() {
                        ImGui::Text("Name: %s", ActorDB::Instance->RetrieveEntry(display->id).name.c_str());
                        ImGui::Text("Description: %s", GetActorDescription(display->id).c_str());
                        ImGui::Text("Category: %s", acMapping[display->category]);
                        ImGui::Text("ID: %d", display->id);
                        ImGui::Text("Parameters: %d", display->params);
                        ImGui::Text("Actor List Index: %d", GetActorListIndex(display));
                    },
                    "Selected Actor");
                ImGui::SameLine();
                ImGui::PushItemWidth(ImGui::GetFontSize() * 6);

                DrawGroupWithBorder(
                    [&]() {
                        ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
                        PushStyleInput(THEME_COLOR);
                        ImGui::Text("Actor Position");
                        ImGui::InputScalar("X##CurPos", ImGuiDataType_Float, &display->world.pos.x);
                        ImGui::InputScalar("Y##CurPos", ImGuiDataType_Float, &display->world.pos.y);
                        ImGui::InputScalar("Z##CurPos", ImGuiDataType_Float, &display->world.pos.z);
                        ImGui::PopItemWidth();
                        PopStyleInput();
                    },
                    "Actor Position");
                ImGui::SameLine();
                DrawGroupWithBorder(
                    [&]() {
                        PushStyleInput(THEME_COLOR);
                        ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
                        ImGui::Text("Actor Rotation");
                        ImGui::InputScalar("X##CurRot", ImGuiDataType_S16, &display->world.rot.x);
                        ImGui::InputScalar("Y##CurRot", ImGuiDataType_S16, &display->world.rot.y);
                        ImGui::InputScalar("Z##CurRot", ImGuiDataType_S16, &display->world.rot.z);
                        ImGui::PopItemWidth();
                        PopStyleInput();
                    },
                    "Actor Rotation");

                if (display->category == ACTORCAT_BOSS || display->category == ACTORCAT_ENEMY) {
                    PushStyleInput(THEME_COLOR);
                    ImGui::InputScalar("Enemy Health", ImGuiDataType_U8, &display->colChkInfo.health);
                    PopStyleInput();
                    UIWidgets::InsertHelpHoverText("Some actors might not use this!");
                }

                DrawGroupWithBorder(
                    [&]() {
                        ImGui::Text("flags");
                        UIWidgets::DrawFlagArray32("flags", display->flags);
                    },
                    "flags");

                ImGui::SameLine();

                DrawGroupWithBorder(
                    [&]() {
                        ImGui::Text("bgCheckFlags");
                        UIWidgets::DrawFlagArray16("bgCheckFlags", display->bgCheckFlags);
                    },
                    "bgCheckFlags");

                if (Button("Go to Actor", ButtonOptions().Color(THEME_COLOR))) {
                    Player* player = GET_PLAYER(gPlayState);
                    Math_Vec3f_Copy(&player->actor.world.pos, &display->world.pos);
                    Math_Vec3f_Copy(&player->actor.home.pos, &player->actor.world.pos);
                }
            } else {
                ImGui::Text("Select an actor to display information.");
            }

            if (Button("Fetch from Target",
                       ButtonOptions()
                           .Color(THEME_COLOR)
                           .Tooltip("Grabs actor with target arrow above it. You might need C-Up for enemies"))) {
                Player* player = GET_PLAYER(gPlayState);
                if (player->talkActor != NULL) {
                    display = player->talkActor;
                    category = display->category;
                    PopulateActorDropdown(category, list);
                }
            }
            if (Button("Fetch from Held",
                       ButtonOptions().Color(THEME_COLOR).Tooltip("Grabs actor that Link is holding"))) {
                Player* player = GET_PLAYER(gPlayState);
                if (player->heldActor != NULL) {
                    display = player->heldActor;
                    category = display->category;
                    PopulateActorDropdown(category, list);
                }
            }
            if (Button("Fetch from Interaction",
                       ButtonOptions().Color(THEME_COLOR).Tooltip("Grabs actor from \"interaction range\""))) {
                Player* player = GET_PLAYER(gPlayState);
                if (player->interactRangeActor != NULL) {
                    display = player->interactRangeActor;
                    category = display->category;
                    PopulateActorDropdown(category, list);
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("New...")) {
            // ImGui::PushItemWidth(ImGui::GetFontSize() * 10);

            if (InputString("Search Actor", &searchString, InputOptions().Color(THEME_COLOR))) {
                actorSearchResults = GetActorsWithDescriptionContainingString(searchString);
                currentSelectedInDropdown = -1;
            }

            if (!SohUtils::IsStringEmpty(searchString) && !actorSearchResults.empty()) {
                std::string preview =
                    currentSelectedInDropdown == -1
                        ? "Please Select"
                        : ActorDB::Instance->RetrieveEntry(actorSearchResults[currentSelectedInDropdown]).desc;
                PushStyleCombobox(THEME_COLOR);
                if (ImGui::BeginCombo("Results", preview.c_str())) {
                    for (u8 i = 0; i < actorSearchResults.size(); i++) {
                        if (ImGui::Selectable(ActorDB::Instance->RetrieveEntry(actorSearchResults[i]).desc.c_str(),
                                              i == currentSelectedInDropdown)) {
                            currentSelectedInDropdown = i;
                            newActor.id = actorSearchResults[i];
                        }
                    }
                    ImGui::EndCombo();
                }
                PopStyleCombobox();
            }

            ImGui::Text("%s", GetActorDescription(newActor.id).c_str());
            if (ImGui::InputScalar("ID", ImGuiDataType_S16, &newActor.id, &one)) {
                newActor.params = 0;
            }

            CVarCheckbox("Advanced mode", CVAR_DEVELOPER_TOOLS("ActorViewer.AdvancedParams"),
                         CheckboxOptions().Tooltip("Changes the actor specific param menus with a direct input"));

            if (CVarGetInteger(CVAR_DEVELOPER_TOOLS("ActorViewer.AdvancedParams"), 0)) {
                PushStyleInput(THEME_COLOR);
                ImGui::InputScalar("params", ImGuiDataType_S16, &newActor.params, &one);
                PopStyleInput();
            } else if (std::find(noParamsActors.begin(), noParamsActors.end(), newActor.id) == noParamsActors.end()) {
                CreateActorSpecificData();
                if (actorSpecificData.find(newActor.id) == actorSpecificData.end()) {
                    PushStyleInput(THEME_COLOR);
                    ImGui::InputScalar("params", ImGuiDataType_S16, &newActor.params, &one);
                    PopStyleInput();
                } else {
                    DrawGroupWithBorder(
                        [&]() {
                            ImGui::Text("Actor Specific Data");
                            newActor.params = actorSpecificData[newActor.id](newActor.params);
                        },
                        "Actor Specific Data");
                }
            }

            ImGui::PushItemWidth(ImGui::GetFontSize() * 6);

            DrawGroupWithBorder(
                [&]() {
                    PushStyleInput(THEME_COLOR);
                    ImGui::Text("New Actor Position");
                    ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
                    ImGui::InputScalar("X##NewPos", ImGuiDataType_Float, &newActor.pos.x);
                    ImGui::InputScalar("Y##NewPos", ImGuiDataType_Float, &newActor.pos.y);
                    ImGui::InputScalar("Z##NewPos", ImGuiDataType_Float, &newActor.pos.z);
                    ImGui::PopItemWidth();
                    PopStyleInput();
                },
                "New Actor Position");
            ImGui::SameLine();
            DrawGroupWithBorder(
                [&]() {
                    PushStyleInput(THEME_COLOR);
                    ImGui::Text("New Actor Rotation");
                    ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
                    ImGui::InputScalar("X##NewRot", ImGuiDataType_S16, &newActor.rot.x);
                    ImGui::InputScalar("Y##NewRot", ImGuiDataType_S16, &newActor.rot.y);
                    ImGui::InputScalar("Z##NewRot", ImGuiDataType_S16, &newActor.rot.z);
                    ImGui::PopItemWidth();
                    PopStyleInput();
                },
                "New Actor Rotation");

            if (Button("Fetch from Link", ButtonOptions().Color(THEME_COLOR))) {
                Player* player = GET_PLAYER(gPlayState);
                Vec3f newPos = player->actor.world.pos;
                Vec3s newRot = player->actor.world.rot;
                newActor.pos = newPos;
                newActor.rot = newRot;
            }

            if (Button("Spawn", ButtonOptions().Color(THEME_COLOR))) {
                if (ActorDB::Instance->RetrieveEntry(newActor.id).entry.valid) {
                    Actor_Spawn(&gPlayState->actorCtx, gPlayState, newActor.id, newActor.pos.x, newActor.pos.y,
                                newActor.pos.z, newActor.rot.x, newActor.rot.y, newActor.rot.z, newActor.params, 0);
                } else {
                    Sfx_PlaySfxCentered(NA_SE_SY_ERROR);
                }
            }

            if (Button("Spawn as Child", ButtonOptions().Color(THEME_COLOR))) {
                Actor* parent = display;
                if (parent != NULL) {
                    if (newActor.id >= 0 && newActor.id < ACTOR_ID_MAX &&
                        ActorDB::Instance->RetrieveEntry(newActor.id).entry.valid) {
                        Actor_SpawnAsChild(&gPlayState->actorCtx, parent, gPlayState, newActor.id, newActor.pos.x,
                                           newActor.pos.y, newActor.pos.z, newActor.rot.x, newActor.rot.y,
                                           newActor.rot.z, newActor.params);
                    } else {
                        Sfx_PlaySfxCentered(NA_SE_SY_ERROR);
                    }
                }
            }

            if (Button("Reset", ButtonOptions().Color(THEME_COLOR))) {
                newActor = { 0, 0, { 0, 0, 0 }, { 0, 0, 0 } };
            }

            ImGui::TreePop();
        }
        PopStyleHeader();
    } else {
        ImGui::Text("Global Context needed for actor info!");
    }
    ImGui::EndDisabled();
}

void ActorViewerWindow::InitElement() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorSpawn>([this](void* refActor) {
        Actor* actor = static_cast<Actor*>(refActor);

        // Reload actor list if the new actor belongs to the selected category
        if (category == actor->category) {
            PopulateActorDropdown(actor->category, list);
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorDestroy>([this](void* refActor) {
        Actor* actor = static_cast<Actor*>(refActor);

        // If the actor belongs to the selected category, we need to manually remove it, as it has not been removed from
        // the global actor array yet
        if (category == actor->category) {
            list.erase(std::remove(list.begin(), list.end(), actor), list.end());
        }
        if (display == actor) {
            display = nullptr;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([this](int16_t sceneNum) {
        display = nullptr;
        category = ACTORCAT_SWITCH;
        list.clear();
    });
}

void ActorViewer_RegisterNameTagHooks() {
    COND_HOOK(OnActorInit, CVAR_ACTOR_NAME_TAGS_ENABLED,
              [](void* actor) { ActorViewer_AddTagForActor(static_cast<Actor*>(actor)); });
}

RegisterShipInitFunc nametagInit(ActorViewer_RegisterNameTagHooks, { CVAR_ACTOR_NAME_TAGS_ENABLED_NAME });
