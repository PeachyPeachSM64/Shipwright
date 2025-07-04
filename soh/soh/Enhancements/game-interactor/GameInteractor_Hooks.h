#pragma once

#include "vanilla-behavior/GIVanillaBehavior.h"
#include "GameInteractor.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
// MARK: - Gameplay
void GameInteractor_ExecuteOnZTitleInit(void* gameState);
void GameInteractor_ExecuteOnZTitleUpdate(void* gameState);
void GameInteractor_ExecuteOnLoadGame(int32_t fileNum);
void GameInteractor_ExecuteOnExitGame(int32_t fileNum);
void GameInteractor_ExecuteOnGameStateMainStart();
void GameInteractor_ExecuteOnGameFrameUpdate();
void GameInteractor_ExecuteOnItemReceiveHooks(GetItemEntry itemEntry);
void GameInteractor_ExecuteOnSaleEndHooks(GetItemEntry itemEntry);
void GameInteractor_ExecuteOnTransitionEndHooks(int16_t sceneNum);
void GameInteractor_ExecuteOnSceneInit(int16_t sceneNum);
void GameInteractor_ExecuteAfterSceneCommands(int16_t sceneNum);
void GameInteractor_ExecuteOnSceneFlagSet(int16_t sceneNum, int16_t flagType, int16_t flag);
void GameInteractor_ExecuteOnSceneFlagUnset(int16_t sceneNum, int16_t flagType, int16_t flag);
void GameInteractor_ExecuteOnFlagSet(int16_t flagType, int16_t flag);
void GameInteractor_ExecuteOnFlagUnset(int16_t flagType, int16_t flag);
void GameInteractor_ExecuteOnSceneSpawnActors();
void GameInteractor_ExecuteOnPlayerUpdate();
void GameInteractor_ExecuteOnSetDoAction(uint16_t action);
void GameInteractor_ExecuteOnOcarinaSongAction();
void GameInteractor_ExecuteOnCuccoOrChickenHatch();
void GameInteractor_ExecuteOnActorInit(void* actor);
void GameInteractor_ExecuteOnActorSpawn(void* actor);
void GameInteractor_ExecuteOnActorUpdate(void* actor);
void GameInteractor_ExecuteOnActorKill(void* actor);
void GameInteractor_ExecuteOnActorDestroy(void* actor);
void GameInteractor_ExecuteOnEnemyDefeat(void* actor);
void GameInteractor_ExecuteOnBossDefeat(void* actor);
void GameInteractor_ExecuteOnTimestamp(u8 item);
void GameInteractor_ExecuteOnPlayerBonk();
void GameInteractor_ExecuteOnPlayerHealthChange(int16_t amount);
void GameInteractor_ExecuteOnPlayerBottleUpdate(int16_t contents);
void GameInteractor_ExecuteOnPlayerHoldUpShield();
void GameInteractor_ExecuteOnPlayerFirstPersonControl(Player* player);
void GameInteractor_ExecuteOnPlayerShieldControl(float_t* sp50, float_t* sp54);
void GameInteractor_ExecuteOnPlayerProcessStick();
void GameInteractor_ExecuteOnShopSlotChangeHooks(uint8_t cursorIndex, int16_t price);
void GameInteractor_ExecuteOnPlayDestroy();
void GameInteractor_ExecuteOnPlayDrawEnd();
bool GameInteractor_Should(GIVanillaBehavior flag, uint32_t result, ...);

// MARK: -  Save Files
void GameInteractor_ExecuteOnSaveFile(int32_t fileNum);
void GameInteractor_ExecuteOnLoadFile(int32_t fileNum);
void GameInteractor_ExecuteOnDeleteFile(int32_t fileNum);

// MARK: - Dialog
void GameInteractor_ExecuteOnDialogMessage();
void GameInteractor_ExecuteOnPresentTitleCard();
void GameInteractor_ExecuteOnInterfaceUpdate();
void GameInteractor_ExecuteOnKaleidoscopeUpdate(int16_t inDungeonScene);

// MARK: - Main Menu
void GameInteractor_ExecuteOnPresentFileSelect();
void GameInteractor_ExecuteOnUpdateFileSelectSelection(uint16_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileSelectConfirmationSelection(uint16_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileCopySelection(uint16_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileCopyConfirmationSelection(uint16_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileEraseSelection(uint16_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileEraseConfirmationSelection(uint16_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileAudioSelection(uint8_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileTargetSelection(uint8_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileLanguageSelection(uint8_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileQuestSelection(uint8_t questIndex);
void GameInteractor_ExecuteOnUpdateFileBossRushOptionSelection(uint8_t optionIndex, uint8_t optionValue);
void GameInteractor_ExecuteOnUpdateFileRandomizerOptionSelection(uint8_t optionIndex);
void GameInteractor_ExecuteOnUpdateFileNameSelection(int16_t charCode);
void GameInteractor_ExecuteOnFileChooseMain(void* gameState);

// MARK: - Game
void GameInteractor_ExecuteOnSetGameLanguage();

// MARK: - System
void GameInteractor_RegisterOnAssetAltChange(void (*fn)(void));

// Mark: - Pause Menu
void GameInteractor_ExecuteOnKaleidoUpdate();

#ifdef __cplusplus
}
#endif
