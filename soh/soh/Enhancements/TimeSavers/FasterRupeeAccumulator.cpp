#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "z64save.h"
#include "macros.h"
#include "variables.h"
#include "functions.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

void RegisterFasterRupeeAccumulator() {
    COND_HOOK(OnInterfaceUpdate, CVarGetInteger(CVAR_ENHANCEMENT("FasterRupeeAccumulator"), 0), []() {
        if (gSaveContext.rupeeAccumulator == 0) {
            return;
        }

        // Gaining rupees
        if (gSaveContext.rupeeAccumulator > 0) {
            // Wallet is full
            if (gSaveContext.rupees >= CUR_CAPACITY(UPG_WALLET)) {
                return;
            }

            if (gSaveContext.rupeeAccumulator >= 10 && gSaveContext.rupees + 10 < CUR_CAPACITY(UPG_WALLET)) {
                gSaveContext.rupeeAccumulator -= 10;
                gSaveContext.rupees += 10;
            }
            // Losing rupees
        } else if (gSaveContext.rupeeAccumulator < 0) {
            // No rupees to lose
            if (gSaveContext.rupees == 0) {
                return;
            }

            if (gSaveContext.rupeeAccumulator <= -10 && gSaveContext.rupees > 10) {
                gSaveContext.rupeeAccumulator += 10;
                gSaveContext.rupees -= 10;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFasterRupeeAccumulator, { CVAR_ENHANCEMENT("FasterRupeeAccumulator") });
