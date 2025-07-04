#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/randomizer/context.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "macros.h"
#include "src/overlays/actors/ovl_En_Ko/z_en_ko.h"
#include "z64save.h"
#include "functions.h"
#include "variables.h"
}

static bool sEnteredBlueWarp = false;

// Todo: Move item queueing here

/**
 * This ensures the Kokiri blocking the forest exit checks if you are eligible to leave the forest
 * every frame, instead of only at init. The reason we need to do this is when we skip the blue warp cutscene
 * you end up getting the Kokiri Emerald after the actor has init'd, so the actor doesn't know you have it
 */
void EnKo_MoveWhenReady(EnKo* enKo, PlayState* play) {
    func_80A995CC(enKo, play);

    if ((enKo->actor.params & 0xFF) == ENKO_TYPE_CHILD_3) {
        if (GameInteractor_Should(VB_OPEN_KOKIRI_FOREST, CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD))) {
            enKo->collider.dim.height -= 200;
            Path_CopyLastPoint(enKo->path, &enKo->actor.world.pos);
            enKo->actionFunc = func_80A99384;
        }
    }
}

void SkipBlueWarp_OnActorUpdate(void* actorPtr) {
    EnKo* enKo = static_cast<EnKo*>(actorPtr);

    if ((enKo->actor.params & 0xFF) == ENKO_TYPE_CHILD_3 && enKo->actionFunc == func_80A995CC) {
        enKo->actionFunc = EnKo_MoveWhenReady;
    }
}

void RegisterSkipBlueWarp() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_EN_KO,
                 CVarGetInteger(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), IS_RANDO),
                 SkipBlueWarp_OnActorUpdate);

    /**
     * This will ensure that the Deku Tree Sprout considers the Forest Temple finished when you skip the blue warp
     * cutscene. Typically this checks for if you have the medallion, and when skipping the cutscene at this point you
     * don't have it yet.
     */
    COND_VB_SHOULD(VB_DEKU_JR_CONSIDER_FOREST_TEMPLE_FINISHED,
                   CVarGetInteger(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), IS_RANDO), {
                       if (gSaveContext.entranceIndex == ENTR_KOKIRI_FOREST_DEKU_TREE_BLUE_WARP &&
                           gSaveContext.cutsceneIndex == 0xFFF1) {
                           *should = Flags_GetEventChkInf(EVENTCHKINF_USED_FOREST_TEMPLE_BLUE_WARP);
                       }
                   });

    /**
     * While we could rely on the Item_Give that's normally called, it's not very clear to the player that they
     * received the item when skipping the blue warp cutscene, so we'll prevent that and queue it up to be given
     * to the player instead.
     */
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_BLUE_WARP,
                   CVarGetInteger(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), IS_RANDO), { *should = false; });
}

void RegisterShouldPlayBlueWarp() {
    /**
     * This will override the transitions into the blue warp cutscenes, set any appropriate flags, and
     * set the entrance index to where you would normally end up after the blue warp cutscene. This
     * should also account for the difference between your first and following visits to the blue warp.
     */
    REGISTER_VB_SHOULD(VB_PLAY_TRANSITION_CS, {
        // Do nothing when in a boss rush
        if (IS_BOSS_RUSH) {
            return;
        }

        bool overrideBlueWarpDestinations =
            IS_RANDO && (RAND_GET_OPTION(RSK_SHUFFLE_DUNGEON_ENTRANCES) != RO_DUNGEON_ENTRANCE_SHUFFLE_OFF ||
                         RAND_GET_OPTION(RSK_SHUFFLE_BOSS_ENTRANCES) != RO_BOSS_ROOM_ENTRANCE_SHUFFLE_OFF);

        // Force blue warp skip on when ER needs to place Link somewhere else.
        // This is preferred over having story cutscenes play in the overworld and then reloading Link somewhere else
        // after.
        if (CVarGetInteger(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), IS_RANDO) ||
            overrideBlueWarpDestinations) {
            bool isBlueWarpCutscene = false;
            // Deku Tree Blue warp
            if (gSaveContext.entranceIndex == ENTR_KOKIRI_FOREST_0 && gSaveContext.cutsceneIndex == 0xFFF1) {
                gSaveContext.entranceIndex = ENTR_KOKIRI_FOREST_DEKU_TREE_BLUE_WARP;
                isBlueWarpCutscene = true;
                // Dodongo's Cavern Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT &&
                       gSaveContext.cutsceneIndex == 0xFFF1) {
                gSaveContext.entranceIndex = ENTR_DEATH_MOUNTAIN_TRAIL_DODONGO_BLUE_WARP;
                isBlueWarpCutscene = true;
                // Jabu Jabu's Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_ZORAS_FOUNTAIN_JABU_JABU_BLUE_WARP &&
                       gSaveContext.cutsceneIndex == 0xFFF0) {
                gSaveContext.entranceIndex = ENTR_ZORAS_FOUNTAIN_JABU_JABU_BLUE_WARP;
                isBlueWarpCutscene = true;
                // Forest Temple Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_CHAMBER_OF_THE_SAGES_0 && gSaveContext.cutsceneIndex == 0x0 &&
                       gSaveContext.chamberCutsceneNum == CHAMBER_CS_FOREST) {
                // Normally set in the blue warp cutscene
                Flags_SetEventChkInf(EVENTCHKINF_SPOKE_TO_DEKU_TREE_SPROUT);

                if (IS_RANDO) {
                    gSaveContext.entranceIndex = ENTR_SACRED_FOREST_MEADOW_FOREST_TEMPLE_BLUE_WARP;
                } else {
                    gSaveContext.entranceIndex = ENTR_KOKIRI_FOREST_12;
                }

                isBlueWarpCutscene = true;
                // Fire Temple Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_KAKARIKO_VILLAGE_FRONT_GATE &&
                       gSaveContext.cutsceneIndex == 0xFFF3) {
                // Normally set in the blue warp cutscene
                Flags_SetEventChkInf(EVENTCHKINF_DEATH_MOUNTAIN_ERUPTED);

                gSaveContext.entranceIndex = ENTR_DEATH_MOUNTAIN_CRATER_FIRE_TEMPLE_BLUE_WARP;
                isBlueWarpCutscene = true;
                // Water Temple Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_CHAMBER_OF_THE_SAGES_0 && gSaveContext.cutsceneIndex == 0x0 &&
                       gSaveContext.chamberCutsceneNum == CHAMBER_CS_WATER) {
                // Normally set in the blue warp cutscene
                gSaveContext.dayTime = gSaveContext.skyboxTime = 0x4800;
                Flags_SetEventChkInf(EVENTCHKINF_RAISED_LAKE_HYLIA_WATER);

                gSaveContext.entranceIndex = ENTR_LAKE_HYLIA_WATER_TEMPLE_BLUE_WARP;
                isBlueWarpCutscene = true;
                // Spirit Temple Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_CHAMBER_OF_THE_SAGES_0 && gSaveContext.cutsceneIndex == 0x0 &&
                       gSaveContext.chamberCutsceneNum == CHAMBER_CS_SPIRIT) {
                gSaveContext.entranceIndex = ENTR_DESERT_COLOSSUS_SPIRIT_TEMPLE_BLUE_WARP;
                isBlueWarpCutscene = true;
                // Shadow Temple Blue warp
            } else if (gSaveContext.entranceIndex == ENTR_CHAMBER_OF_THE_SAGES_0 && gSaveContext.cutsceneIndex == 0x0 &&
                       gSaveContext.chamberCutsceneNum == CHAMBER_CS_SHADOW) {
                gSaveContext.entranceIndex = ENTR_GRAVEYARD_SHADOW_TEMPLE_BLUE_WARP;
                isBlueWarpCutscene = true;
            }

            if (isBlueWarpCutscene) {
                if (gSaveContext.entranceIndex != ENTR_LAKE_HYLIA_WATER_TEMPLE_BLUE_WARP) {
                    // Normally set in the blue warp cutscene
                    gSaveContext.dayTime = gSaveContext.skyboxTime = 0x8000;
                }

                *should = false;
                gSaveContext.cutsceneIndex = 0;
            }

            // This is outside the above condition because we want to handle both first and following visits to the blue
            // warp
            if (sEnteredBlueWarp && overrideBlueWarpDestinations) {
                Entrance_OverrideBlueWarp();
            }
        }

        sEnteredBlueWarp = false;
    });

    /**
     * Using this hook to simply observe that Link has entered a bluewarp
     * This way we know to allow entrance rando overrides to be processed on the next tranisition hook
     */
    REGISTER_VB_SHOULD(VB_PLAY_BLUE_WARP_CS, { sEnteredBlueWarp = true; });
}

static RegisterShipInitFunc initHooks(RegisterSkipBlueWarp,
                                      { CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), "IS_RANDO" });
static RegisterShipInitFunc initUnconditionalHooks(RegisterShouldPlayBlueWarp);
