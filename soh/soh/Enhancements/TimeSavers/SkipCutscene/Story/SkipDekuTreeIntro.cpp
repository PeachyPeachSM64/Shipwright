#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "src/overlays/actors/ovl_Bg_Treemouth/z_bg_treemouth.h"
}

/**
 * This will skip the Deku Tree intro, and simply open the mouth as you approach it.
 */
void RegisterSkipDekuTreeIntro() {
    COND_VB_SHOULD(VB_PLAY_DEKU_TREE_INTRO_CS,
                   CVarGetInteger(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), IS_RANDO), {
                       BgTreemouth* treeMouth = va_arg(args, BgTreemouth*);
                       Flags_SetEventChkInf(EVENTCHKINF_DEKU_TREE_OPENED_MOUTH);
                       Audio_PlaySoundGeneral(NA_SE_EV_WOODDOOR_OPEN, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                              &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                       BgTreemouth_SetupAction(treeMouth, func_808BC6F8);
                       *should = false;
                   });
}

static RegisterShipInitFunc initFunc(RegisterSkipDekuTreeIntro,
                                     { CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), "IS_RANDO" });
