#include "SaveManager.h"
#include "OTRGlobals.h"
#include "Enhancements/game-interactor/GameInteractor.h"
#include "Enhancements/randomizer/context.h"
#include "Enhancements/randomizer/entrance.h"
#include "Enhancements/randomizer/dungeon.h"
#include "Enhancements/randomizer/trial.h"
#include "soh/util.h"
#include "Enhancements/randomizer/hint.h"
#include "Enhancements/randomizer/item.h"
#include "ResourceManagerHelpers.h"

#include "z64.h"
#include "cvar_prefixes.h"
#include "functions.h"
#include "macros.h"
#include <variables.h>
#include <libultraship/libultraship.h>
#include "soh/SohGui/SohGui.hpp"

#define NOGDI // avoid various windows defines that conflict with things in z64.h
#include <spdlog/spdlog.h>

#include <fstream>
#include <filesystem>
#include <array>
#include <mutex>

extern "C" SaveContext gSaveContext;
using namespace std::string_literals;

void SaveManager::WriteSaveFile(const std::filesystem::path& savePath, const uintptr_t addr, void* dramAddr,
                                const size_t size) {
    std::ofstream saveFile = std::ofstream(savePath, std::fstream::in | std::fstream::out | std::fstream::binary);
    saveFile.seekp(addr);
    saveFile.write((char*)dramAddr, size);
    saveFile.close();
}

void SaveManager::ReadSaveFile(std::filesystem::path savePath, uintptr_t addr, void* dramAddr, size_t size) {
    std::ifstream saveFile = std::ifstream(savePath, std::fstream::in | std::fstream::out | std::fstream::binary);

    // If the file doesn't exist, initialize DRAM
    if (saveFile.good()) {
        saveFile.seekg(addr);
        saveFile.read((char*)dramAddr, size);
    } else {
        memset(dramAddr, 0, size);
    }

    saveFile.close();
}

std::filesystem::path SaveManager::GetFileName(int fileNum) {
    const std::filesystem::path sSavePath(Ship::Context::GetPathRelativeToAppDirectory("Save"));
    return sSavePath / ("file" + std::to_string(fileNum + 1) + ".sav");
}

std::filesystem::path SaveManager::GetFileTempName(int fileNum) {
    const std::filesystem::path sSavePath(Ship::Context::GetPathRelativeToAppDirectory("Save"));
    return sSavePath / ("file" + std::to_string(fileNum + 1) + ".temp");
}

std::vector<RandomizerHint> Rando::StaticData::oldVerHintOrder{
    RH_COLOSSUS_GOSSIP_STONE,
    RH_DMC_GOSSIP_STONE,
    RH_DMC_UPPER_GROTTO_GOSSIP_STONE,
    RH_DMT_GOSSIP_STONE,
    RH_DMT_STORMS_GROTTO_GOSSIP_STONE,
    RH_DODONGOS_CAVERN_GOSSIP_STONE,
    RH_ZF_FAIRY_GOSSIP_STONE,
    RH_GC_MAZE_GOSSIP_STONE,
    RH_GC_MEDIGORON_GOSSIP_STONE,
    RH_GV_GOSSIP_STONE,
    RH_GRAVEYARD_GOSSIP_STONE,
    RH_HC_MALON_GOSSIP_STONE,
    RH_HC_ROCK_WALL_GOSSIP_STONE,
    RH_HC_STORMS_GROTTO_GOSSIP_STONE,
    RH_HF_COW_GROTTO_GOSSIP_STONE,
    RH_HF_NEAR_MARKET_GROTTO_GOSSIP_STONE,
    RH_HF_OPEN_GROTTO_GOSSIP_STONE,
    RH_HF_SOUTHEAST_GROTTO_GOSSIP_STONE,
    RH_ZF_JABU_GOSSIP_STONE,
    RH_KF_DEKU_TREE_LEFT_GOSSIP_STONE,
    RH_KF_DEKU_TREE_RIGHT_GOSSIP_STONE,
    RH_KF_GOSSIP_STONE,
    RH_KF_STORMS_GROTTO_GOSSIP_STONE,
    RH_KAK_OPEN_GROTTO_GOSSIP_STONE,
    RH_LH_LAB_GOSSIP_STONE,
    RH_LH_SOUTHEAST_GOSSIP_STONE,
    RH_LH_SOUTHWEST_GOSSIP_STONE,
    RH_LW_GOSSIP_STONE,
    RH_LW_NEAR_SHORTCUTS_GROTTO_GOSSIP_STONE,
    RH_SFM_MAZE_NEAR_LW_GOSSIP_STONE,
    RH_SFM_MAZE_CENTER_GOSSIP_STONE,
    RH_SFM_SARIA_GOSSIP_STONE,
    RH_TOT_LEFT_CENTER_GOSSIP_STONE,
    RH_TOT_LEFTMOST_GOSSIP_STONE,
    RH_TOT_RIGHT_CENTER_GOSSIP_STONE,
    RH_TOT_RIGHTMOST_GOSSIP_STONE,
    RH_ZD_GOSSIP_STONE,
    RH_ZR_NEAR_DOMAIN_GOSSIP_STONE,
    RH_ZR_NEAR_GROTTOS_GOSSIP_STONE,
    RH_ZR_OPEN_GROTTO_GOSSIP_STONE,
};

uint16_t Rando::StaticData::oldVerGossipStoneStart = 706;

SaveManager::SaveManager() {
    coreSectionIDsByName["base"] = SECTION_ID_BASE;
    coreSectionIDsByName["randomizer"] = SECTION_ID_RANDOMIZER;
    coreSectionIDsByName["sohStats"] = SECTION_ID_STATS;
    coreSectionIDsByName["entrances"] = SECTION_ID_ENTRANCES;
    coreSectionIDsByName["scenes"] = SECTION_ID_SCENES;
    coreSectionIDsByName["trackerData"] = SECTION_ID_TRACKER_DATA;
    AddLoadFunction("base", 1, LoadBaseVersion1);
    AddLoadFunction("base", 2, LoadBaseVersion2);
    AddLoadFunction("base", 3, LoadBaseVersion3);
    AddLoadFunction("base", 4, LoadBaseVersion4);
    AddSaveFunction("base", 4, SaveBase, true, SECTION_PARENT_NONE);

    AddLoadFunction("randomizer", 1, LoadRandomizer);
    AddSaveFunction("randomizer", 1, SaveRandomizer, true, SECTION_PARENT_NONE);

    AddInitFunction(InitFileImpl);

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnExitGame>(
        [this](uint32_t fileNum) { ThreadPoolWait(); });

    smThreadPool = std::make_shared<BS::thread_pool>(1);

    for (SaveFileMetaInfo& info : fileMetaInfo) {
        info.valid = false;
        info.deaths = 0;
        for (int i = 0; i < ARRAY_COUNT(info.playerName); i++) {
            info.playerName[i] = '\0';
        }
        info.healthCapacity = 0;
        info.questItems = 0;
        info.defense = 0;
        info.health = 0;

        for (int i = 0; i < ARRAY_COUNT(info.seedHash); i++) {
            info.seedHash[i] = 0;
        }

        info.randoSave = 0;
        info.requiresMasterQuest = 0;
        info.requiresOriginal = 0;

        info.buildVersionMajor = 0;
        info.buildVersionMinor = 0;
        info.buildVersionPatch = 0;
        memset(&info.buildVersion, 0, sizeof(info.buildVersion));
    }
}

void SaveManager::LoadRandomizer() {
    auto randoContext = Rando::Context::GetInstance();
    SaveManager::Instance->LoadArray("itemLocations", RC_MAX, [&](size_t i) {
        SaveManager::Instance->LoadStruct("", [&]() {
            SaveManager::Instance->LoadData("rgID", randoContext->GetItemLocation(i)->RefPlacedItem());
            RandomizerGet rg = RG_NONE;
            SaveManager::Instance->LoadData("fakeRgID", rg, RG_NONE);
            if (rg != RG_NONE) {
                randoContext->overrides.emplace(static_cast<RandomizerCheck>(i),
                                                Rando::ItemOverride(static_cast<RandomizerCheck>(i), rg));
                SaveManager::Instance->LoadStruct("trickName", [&]() {
                    SaveManager::Instance->LoadData("english", randoContext->GetItemOverride(i).GetTrickName().english);
                    SaveManager::Instance->LoadData("german", randoContext->GetItemOverride(i).GetTrickName().german);
                    SaveManager::Instance->LoadData("french", randoContext->GetItemOverride(i).GetTrickName().french);
                });
            }
            uint16_t price = 0;
            SaveManager::Instance->LoadData("price", price, (uint16_t)0);
            if (price > 0) {
                // Technically an item with a custom price (scrub/shopsanity) could have
                // a 0 rupee price, meaning it would not be loaded after the first save and
                // disappear from the save file. However, this is fine, as the default price on
                // all ItemLocations is 0 anyway.
                randoContext->GetItemLocation(i)->SetCustomPrice(price);
            }
        });
    });

    auto entranceCtx = randoContext->GetEntranceShuffler();
    SaveManager::Instance->LoadArray("entrances", ARRAY_COUNT(entranceCtx->entranceOverrides), [&](size_t i) {
        SaveManager::Instance->LoadStruct("", [&]() {
            SaveManager::Instance->LoadData("type", entranceCtx->entranceOverrides[i].type);
            SaveManager::Instance->LoadData("index", entranceCtx->entranceOverrides[i].index);
            SaveManager::Instance->LoadData("destination", entranceCtx->entranceOverrides[i].destination);
            SaveManager::Instance->LoadData("override", entranceCtx->entranceOverrides[i].override);
            SaveManager::Instance->LoadData("overrideDestination",
                                            entranceCtx->entranceOverrides[i].overrideDestination);
        });
    });

    SaveManager::Instance->LoadArray("seed", randoContext->hashIconIndexes.size(), [&](size_t i) {
        SaveManager::Instance->LoadData("", randoContext->hashIconIndexes[i]);
    });

    std::string inputSeed;
    SaveManager::Instance->LoadData("inputSeed", inputSeed);
    randoContext->SetSeedString(inputSeed);

    uint32_t finalSeed;
    SaveManager::Instance->LoadData("finalSeed", finalSeed);
    randoContext->SetSeed(finalSeed);

    SaveManager::Instance->LoadArray("randoSettings", RSK_MAX, [&](size_t i) {
        int value = 0;
        SaveManager::Instance->LoadData("", value);
        randoContext->GetOption(RandomizerSettingKey(i)).Set(value);
    });

    SaveManager::Instance->LoadArray("hintLocations", RH_MAX, [&](size_t i) {
        auto hint = RandomizerHint(i);
        nlohmann::json json;
        SaveManager::Instance->LoadData("", json);
        randoContext->AddHint(hint, Rando::Hint(hint, json));
    });

    SaveManager::Instance->LoadData("triforcePiecesCollected",
                                    gSaveContext.ship.quest.data.randomizer.triforcePiecesCollected);

    SaveManager::Instance->LoadData("pendingIceTrapCount", gSaveContext.ship.pendingIceTrapCount);

    std::shared_ptr<Randomizer> randomizer = OTRGlobals::Instance->gRandomizer;

    size_t mqDungeonCount;
    SaveManager::Instance->LoadData("masterQuestDungeonCount", mqDungeonCount, (size_t)0);

    randoContext->GetDungeons()->ClearAllMQ();
    SaveManager::Instance->LoadArray("masterQuestDungeons", mqDungeonCount, [&](size_t i) {
        size_t dungeonId;
        SaveManager::Instance->LoadData("", dungeonId);
        randoContext->GetDungeon(dungeonId)->SetMQ();
    });

    randoContext->GetTrials()->SkipAll();
    SaveManager::Instance->LoadArray("requiredTrials", randoContext->GetOption(RSK_TRIAL_COUNT).Get(), [&](size_t i) {
        size_t trialId;
        SaveManager::Instance->LoadData("", trialId);
        randoContext->GetTrial(trialId)->SetAsRequired();
    });

    SaveManager::Instance->LoadArray("trickOptions", RT_MAX, [&](size_t i) {
        uint8_t value = 0;
        SaveManager::Instance->LoadData("", value);
        randoContext->GetTrickOption(RandomizerTrick(i)).Set(value);
    });
}

void SaveManager::SaveRandomizer(SaveContext* saveContext, int sectionID, bool fullSave) {

    if (saveContext->ship.quest.id != QUEST_RANDOMIZER)
        return;
    auto randoContext = Rando::Context::GetInstance();

    SaveManager::Instance->SaveArray("itemLocations", RC_MAX, [&](size_t i) {
        SaveManager::Instance->SaveStruct("", [&]() {
            SaveManager::Instance->SaveData("rgID", randoContext->GetItemLocation(i)->GetPlacedRandomizerGet());
            if (randoContext->GetItemLocation(i)->GetPlacedRandomizerGet() == RG_ICE_TRAP) {
                SaveManager::Instance->SaveData("fakeRgID", randoContext->GetItemOverride(i).LooksLike());
                SaveManager::Instance->SaveStruct("trickName", [&]() {
                    SaveManager::Instance->SaveData("english",
                                                    randoContext->GetItemOverride(i).GetTrickName().GetEnglish());
                    SaveManager::Instance->SaveData("german",
                                                    randoContext->GetItemOverride(i).GetTrickName().GetGerman());
                    SaveManager::Instance->SaveData("french",
                                                    randoContext->GetItemOverride(i).GetTrickName().GetFrench());
                });
            }
            if (randoContext->GetItemLocation(i)->HasCustomPrice()) {
                SaveManager::Instance->SaveData("price", randoContext->GetItemLocation(i)->GetPrice());
            }
        });
    });

    auto entranceCtx = randoContext->GetEntranceShuffler();
    SaveManager::Instance->SaveArray("entrances", ARRAY_COUNT(entranceCtx->entranceOverrides), [&](size_t i) {
        SaveManager::Instance->SaveStruct("", [&]() {
            SaveManager::Instance->SaveData("type", entranceCtx->entranceOverrides[i].type);
            SaveManager::Instance->SaveData("index", entranceCtx->entranceOverrides[i].index);
            SaveManager::Instance->SaveData("destination", entranceCtx->entranceOverrides[i].destination);
            SaveManager::Instance->SaveData("override", entranceCtx->entranceOverrides[i].override);
            SaveManager::Instance->SaveData("overrideDestination",
                                            entranceCtx->entranceOverrides[i].overrideDestination);
        });
    });

    SaveManager::Instance->SaveArray("seed", randoContext->hashIconIndexes.size(), [&](size_t i) {
        SaveManager::Instance->SaveData("", randoContext->hashIconIndexes[i]);
    });

    SaveManager::Instance->SaveData("inputSeed", randoContext->GetSeedString());

    SaveManager::Instance->SaveData("finalSeed", randoContext->GetSeed());

    SaveManager::Instance->SaveArray("randoSettings", RSK_MAX, [&](size_t i) {
        SaveManager::Instance->SaveData("", randoContext->GetOption((RandomizerSettingKey(i))).Get());
    });

    SaveManager::Instance->SaveArray("hintLocations", RH_MAX, [&](size_t i) {
        auto hint = randoContext->GetHint(RandomizerHint(i));
        // RANDOTODO a way for saveData to accept a raw JSON would make maintaining hint code nicer.
        // save manager forces code rewrites between the spoiler log and internal saves, when the code needs to do the
        // exact same thing in cases where data needs to be loaded in from the spoiler log for plando mode. fails as
        // push_back is ambiguous
        // SaveManager::Instance->SaveData(Rando::StaticData::hintNames[(uint32_t)hint].GetEnglish(), hint->toJSON());
        SaveManager::Instance->SaveStruct("", [&]() {
            bool enabled = hint->IsEnabled();
            SaveManager::Instance->SaveData("enabled", enabled);
            if (enabled) {
                std::vector<std::string> messages = hint->GetAllMessageStrings(MF_RAW);
                SaveManager::Instance->SaveArray("messages", messages.size(),
                                                 [&](size_t i) { SaveManager::Instance->SaveData("", messages[i]); });

                SaveManager::Instance->SaveData("distribution", hint->GetDistribution());
                SaveManager::Instance->SaveData(
                    "type", Rando::StaticData::hintTypeNames[hint->GetHintType()].GetEnglish(MF_CLEAN));

                std::vector<RandomizerHintTextKey> hintKeys = hint->GetHintTextKeys();
                SaveManager::Instance->SaveArray("hintKeys", hintKeys.size(),
                                                 [&](size_t i) { SaveManager::Instance->SaveData("", hintKeys[i]); });

                std::vector<RandomizerCheck> locations = hint->GetHintedLocations();
                SaveManager::Instance->SaveArray("locations", locations.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData("", Rando::StaticData::GetLocation(locations[i])->GetName());
                });

                std::vector<RandomizerGet> items = hint->GetHintedItems();
                SaveManager::Instance->SaveArray("items", items.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData("",
                                                    Rando::StaticData::GetItemTable()[items[i]].GetName().GetEnglish());
                });

                std::vector<uint8_t> itemNamesChosen = hint->GetItemNamesChosen();
                SaveManager::Instance->SaveArray("itemNamesChosen", itemNamesChosen.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData("", itemNamesChosen[i]);
                });

                std::vector<uint8_t> hintTextsChosen = hint->GetHintTextsChosen();
                SaveManager::Instance->SaveArray("hintTextsChosen", hintTextsChosen.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData("", hintTextsChosen[i]);
                });

                std::vector<uint8_t> areaTextsChosen = hint->GetAreaTextsChosen();
                SaveManager::Instance->SaveArray("areaTextsChosen", areaTextsChosen.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData("", areaTextsChosen[i]);
                });

                std::vector<RandomizerArea> areas = hint->GetHintedAreas();
                SaveManager::Instance->SaveArray("areas", areas.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData(
                        "", Rando::StaticData::hintTextTable[Rando::StaticData::areaNames[areas[i]]]
                                .GetClear()
                                .GetForCurrentLanguage(MF_CLEAN));
                });

                std::vector<TrialKey> trials = hint->GetHintedTrials();
                SaveManager::Instance->SaveArray("trials", trials.size(), [&](size_t i) {
                    SaveManager::Instance->SaveData(
                        "", randoContext->GetTrial(trials[i])->GetName().GetForCurrentLanguage(MF_CLEAN));
                });

                SaveManager::Instance->SaveData("num", hint->GetNum());
            }
        });
    });

    SaveManager::Instance->SaveData("triforcePiecesCollected",
                                    saveContext->ship.quest.data.randomizer.triforcePiecesCollected);

    SaveManager::Instance->SaveData("pendingIceTrapCount", saveContext->ship.pendingIceTrapCount);

    std::shared_ptr<Randomizer> randomizer = OTRGlobals::Instance->gRandomizer;

    SaveManager::Instance->SaveData("masterQuestDungeonCount", randoContext->GetDungeons()->CountMQ());

    SaveManager::Instance->SaveArray("masterQuestDungeons", randoContext->GetDungeons()->GetDungeonListSize(),
                                     [&](size_t i) {
                                         if (randoContext->GetDungeon(i)->IsMQ()) {
                                             SaveManager::Instance->SaveData("", i);
                                         }
                                     });

    SaveManager::Instance->SaveArray("requiredTrials", randoContext->GetTrials()->GetTrialListSize(), [&](size_t i) {
        if (randoContext->GetTrial(i)->IsRequired()) {
            SaveManager::Instance->SaveData("", i);
        }
    });

    SaveManager::Instance->SaveArray("trickOptions", RT_MAX, [&](size_t i) {
        SaveManager::Instance->SaveData("", randoContext->GetTrickOption(RandomizerTrick(i)).Get());
    });
}

// Init() here is an extension of InitSram, and thus not truly an initializer for SaveManager itself. don't put any
// class initialization stuff here
void SaveManager::Init() {
    // Wait on saves that snuck through the Wait in OnExitGame
    ThreadPoolWait();
    const std::filesystem::path sSavePath(Ship::Context::GetPathRelativeToAppDirectory("Save"));
    const std::filesystem::path sGlobalPath = sSavePath / std::string("global.sav");
    auto sOldSavePath = Ship::Context::GetPathRelativeToAppDirectory("oot_save.sav");
    auto sOldBackupSavePath = Ship::Context::GetPathRelativeToAppDirectory("oot_save.bak");

    // If the save directory does not exist, create it
    if (!std::filesystem::exists(sSavePath)) {
        std::filesystem::create_directory(sSavePath);
    }

    // If there is a lingering unversioned save, convert it
    if (std::filesystem::exists(sOldSavePath)) {
        ConvertFromUnversioned();
        std::filesystem::rename(sOldSavePath, sOldBackupSavePath);
    }

    // If the global save file exist, load it. Otherwise, create it.
    if (std::filesystem::exists(sGlobalPath)) {
        std::ifstream input(sGlobalPath);

        nlohmann::json globalBlock;
        input >> globalBlock;

        if (!globalBlock.contains("version")) {
            SPDLOG_WARN("Global save does not contain a version. We are reconstructing it.");
            CreateDefaultGlobal();
            return;
        }

        switch (globalBlock["version"].get<int>()) {
            case 1:
                currentJsonContext = &globalBlock;
                LoadData("audioSetting", gSaveContext.audioSetting);
                LoadData("zTargetSetting", gSaveContext.zTargetSetting);
                LoadData("language", gSaveContext.language);
                break;
            default:
                SPDLOG_WARN("Global save has a unrecognized version. We are reconstructing it.");
                CreateDefaultGlobal();
                break;
        }
    } else {
        CreateDefaultGlobal();
    }

    // Load files to initialize metadata
    for (int fileNum = 0; fileNum < MaxFiles; fileNum++) {
        if (std::filesystem::exists(GetFileName(fileNum))) {
            LoadFile(fileNum);
            saveBlock = nlohmann::json::object();
            OTRGlobals::Instance->gRandoContext->ClearItemLocations();
        }
    }
}

void SaveManager::InitMeta(int fileNum) {
    fileMetaInfo[fileNum].valid = true;
    fileMetaInfo[fileNum].deaths = gSaveContext.deaths;
    for (int i = 0; i < ARRAY_COUNT(fileMetaInfo[fileNum].playerName); i++) {
        fileMetaInfo[fileNum].playerName[i] = gSaveContext.playerName[i];
    }
    fileMetaInfo[fileNum].healthCapacity = gSaveContext.healthCapacity;
    fileMetaInfo[fileNum].questItems = gSaveContext.inventory.questItems;
    for (int i = 0; i < ARRAY_COUNT(fileMetaInfo[fileNum].inventoryItems); i++) {
        fileMetaInfo[fileNum].inventoryItems[i] = gSaveContext.inventory.items[i];
    }
    fileMetaInfo[fileNum].equipment = gSaveContext.inventory.equipment;
    fileMetaInfo[fileNum].upgrades = gSaveContext.inventory.upgrades;
    fileMetaInfo[fileNum].isMagicAcquired = gSaveContext.isMagicAcquired;
    fileMetaInfo[fileNum].isDoubleMagicAcquired = gSaveContext.isDoubleMagicAcquired;
    fileMetaInfo[fileNum].rupees = gSaveContext.rupees;
    fileMetaInfo[fileNum].gsTokens = gSaveContext.inventory.gsTokens;
    fileMetaInfo[fileNum].isDoubleDefenseAcquired = gSaveContext.isDoubleDefenseAcquired;
    fileMetaInfo[fileNum].gregFound = Flags_GetRandomizerInf(RAND_INF_GREG_FOUND);
    fileMetaInfo[fileNum].filenameLanguage = gSaveContext.ship.filenameLanguage;
    fileMetaInfo[fileNum].hasWallet = Flags_GetRandomizerInf(RAND_INF_HAS_WALLET) || !IS_RANDO;
    fileMetaInfo[fileNum].defense = gSaveContext.inventory.defenseHearts;
    fileMetaInfo[fileNum].health = gSaveContext.health;
    auto randoContext = Rando::Context::GetInstance();

    for (int i = 0; i < ARRAY_COUNT(fileMetaInfo[fileNum].seedHash); i++) {
        fileMetaInfo[fileNum].seedHash[i] = randoContext->hashIconIndexes[i];
    }

    fileMetaInfo[fileNum].randoSave = IS_RANDO;
    // If the file is marked as a Master Quest file or if we're randomized and have at least one master quest dungeon,
    // we need the mq otr.
    fileMetaInfo[fileNum].requiresMasterQuest =
        IS_MASTER_QUEST || (IS_RANDO && randoContext->GetDungeons()->CountMQ() > 0);
    // If the file is not marked as Master Quest, it could still theoretically be a rando save with all 12 MQ dungeons,
    // in which case we don't actually require a vanilla OTR.
    fileMetaInfo[fileNum].requiresOriginal =
        !IS_MASTER_QUEST && (!IS_RANDO || randoContext->GetDungeons()->CountMQ() < 12);

    fileMetaInfo[fileNum].buildVersionMajor = gSaveContext.ship.stats.buildVersionMajor;
    fileMetaInfo[fileNum].buildVersionMinor = gSaveContext.ship.stats.buildVersionMinor;
    fileMetaInfo[fileNum].buildVersionPatch = gSaveContext.ship.stats.buildVersionPatch;
    SohUtils::CopyStringToCharArray(fileMetaInfo[fileNum].buildVersion, gSaveContext.ship.stats.buildVersion,
                                    ARRAY_COUNT(fileMetaInfo[fileNum].buildVersion));
}

void SaveManager::InitFile(bool isDebug) {
    for (InitFunc& func : initFuncs) {
        func(isDebug);
    }
}

void SaveManager::InitFileImpl(bool isDebug) {
    if (isDebug) {
        InitFileDebug();
    } else {
        InitFileNormal();
    }
}

void SaveManager::InitFileNormal() {
    gSaveContext.totalDays = 0;
    gSaveContext.bgsDayCount = 0;

    gSaveContext.deaths = 0;
    if (ResourceMgr_GetGameRegion(0) == GAME_REGION_PAL && gSaveContext.language != LANGUAGE_JPN) {
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = 0x3E;
        }
        gSaveContext.ship.filenameLanguage = NAME_LANGUAGE_PAL;
    } else { // GAME_REGION_NTSC
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = 0xDF;
        }
        gSaveContext.ship.filenameLanguage =
            (gSaveContext.language == LANGUAGE_JPN) ? NAME_LANGUAGE_NTSC_JPN : NAME_LANGUAGE_NTSC_ENG;
    }
    gSaveContext.n64ddFlag = 0;
    gSaveContext.healthCapacity = 0x30;
    gSaveContext.health = 0x30;
    gSaveContext.magicLevel = 0;
    gSaveContext.magic = 0x30;
    gSaveContext.rupees = 0;
    gSaveContext.swordHealth = 0;
    gSaveContext.naviTimer = 0;
    gSaveContext.isMagicAcquired = 0;
    gSaveContext.isDoubleMagicAcquired = 0;
    gSaveContext.isDoubleDefenseAcquired = 0;
    gSaveContext.bgsFlag = 0;
    gSaveContext.ocarinaGameRoundNum = 0;
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.childEquips.buttonItems); button++) {
        gSaveContext.childEquips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots); button++) {
        gSaveContext.childEquips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.childEquips.equipment = 0;
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.adultEquips.buttonItems); button++) {
        gSaveContext.adultEquips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots); button++) {
        gSaveContext.adultEquips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.adultEquips.equipment = 0;
    gSaveContext.unk_54 = 0;
    gSaveContext.savedSceneNum = SCENE_LINKS_HOUSE;

    // Equipment
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.buttonItems); button++) {
        gSaveContext.equips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.cButtonSlots); button++) {
        gSaveContext.equips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.equips.equipment = 0x1100;

    // Inventory
    for (int item = 0; item < ARRAY_COUNT(gSaveContext.inventory.items); item++) {
        gSaveContext.inventory.items[item] = ITEM_NONE;
    }
    for (int ammo = 0; ammo < ARRAY_COUNT(gSaveContext.inventory.ammo); ammo++) {
        gSaveContext.inventory.ammo[ammo] = 0;
    }
    gSaveContext.inventory.equipment = 0x1100;
    gSaveContext.inventory.upgrades = 0;
    gSaveContext.inventory.questItems = 0;
    for (int dungeon = 0; dungeon < ARRAY_COUNT(gSaveContext.inventory.dungeonItems); dungeon++) {
        gSaveContext.inventory.dungeonItems[dungeon] = 0;
    }
    for (int dungeon = 0; dungeon < ARRAY_COUNT(gSaveContext.inventory.dungeonKeys); dungeon++) {
        gSaveContext.inventory.dungeonKeys[dungeon] = 0xFF;
    }
    gSaveContext.inventory.defenseHearts = 0;
    gSaveContext.inventory.gsTokens = 0;
    for (int scene = 0; scene < ARRAY_COUNT(gSaveContext.sceneFlags); scene++) {
        gSaveContext.sceneFlags[scene].chest = 0;
        gSaveContext.sceneFlags[scene].swch = 0;
        gSaveContext.sceneFlags[scene].clear = 0;
        gSaveContext.sceneFlags[scene].collect = 0;
        gSaveContext.sceneFlags[scene].unk = 0;
        gSaveContext.sceneFlags[scene].rooms = 0;
        gSaveContext.sceneFlags[scene].floors = 0;
    }
    gSaveContext.fw.pos.x = 0;
    gSaveContext.fw.pos.y = 0;
    gSaveContext.fw.pos.z = 0;
    gSaveContext.fw.yaw = 0;
    gSaveContext.fw.playerParams = 0;
    gSaveContext.fw.entranceIndex = 0;
    gSaveContext.fw.roomIndex = 0;
    gSaveContext.fw.set = 0;
    gSaveContext.fw.tempSwchFlags = 0;
    gSaveContext.fw.tempCollectFlags = 0;
    for (int flag = 0; flag < ARRAY_COUNT(gSaveContext.gsFlags); flag++) {
        gSaveContext.gsFlags[flag] = 0;
    }
    for (int highscore = 0; highscore < ARRAY_COUNT(gSaveContext.highScores); highscore++) {
        gSaveContext.highScores[highscore] = 0;
    }
    for (int flag = 0; flag < ARRAY_COUNT(gSaveContext.eventChkInf); flag++) {
        gSaveContext.eventChkInf[flag] = 0;
    }
    for (int flag = 0; flag < ARRAY_COUNT(gSaveContext.itemGetInf); flag++) {
        gSaveContext.itemGetInf[flag] = 0;
    }
    for (int flag = 0; flag < ARRAY_COUNT(gSaveContext.infTable); flag++) {
        gSaveContext.infTable[flag] = 0;
    }
    // Currently randomizer flags are accessible from all quests
    for (int flag = 0; flag < ARRAY_COUNT(gSaveContext.ship.randomizerInf); flag++) {
        gSaveContext.ship.randomizerInf[flag] = 0;
    }
    gSaveContext.worldMapAreaData = 0;
    gSaveContext.scarecrowLongSongSet = 0;
    for (int i = 0; i < ARRAY_COUNT(gSaveContext.scarecrowLongSong); i++) {
        gSaveContext.scarecrowLongSong[i].noteIdx = 0;
        gSaveContext.scarecrowLongSong[i].unk_01 = 0;
        gSaveContext.scarecrowLongSong[i].unk_02 = 0;
        gSaveContext.scarecrowLongSong[i].volume = 0;
        gSaveContext.scarecrowLongSong[i].vibrato = 0;
        gSaveContext.scarecrowLongSong[i].tone = 0;
        gSaveContext.scarecrowLongSong[i].semitone = 0;
    }
    gSaveContext.scarecrowSpawnSongSet = 0;
    for (int i = 0; i < ARRAY_COUNT(gSaveContext.scarecrowSpawnSong); i++) {
        gSaveContext.scarecrowSpawnSong[i].noteIdx = 0;
        gSaveContext.scarecrowSpawnSong[i].unk_01 = 0;
        gSaveContext.scarecrowSpawnSong[i].unk_02 = 0;
        gSaveContext.scarecrowSpawnSong[i].volume = 0;
        gSaveContext.scarecrowSpawnSong[i].vibrato = 0;
        gSaveContext.scarecrowSpawnSong[i].tone = 0;
        gSaveContext.scarecrowSpawnSong[i].semitone = 0;
    }

    gSaveContext.horseData.scene = SCENE_HYRULE_FIELD;
    gSaveContext.horseData.pos.x = -1840;
    gSaveContext.horseData.pos.y = 72;
    gSaveContext.horseData.pos.z = 5497;
    gSaveContext.horseData.angle = -0x6AD9;
    gSaveContext.magicLevel = 0;
    gSaveContext.infTable[29] = 1;
    gSaveContext.sceneFlags[5].swch = 0x40000000;

    // SoH specific
    gSaveContext.ship.backupFW = gSaveContext.fw;
    gSaveContext.ship.pendingSale = ITEM_NONE;
    gSaveContext.ship.pendingSaleMod = MOD_NONE;
    gSaveContext.ship.pendingIceTrapCount = 0;
    gSaveContext.ship.maskMemory = PLAYER_MASK_NONE;

    // Init with normal quest unless only an MQ rom is provided
    gSaveContext.ship.quest.id = OTRGlobals::Instance->HasOriginal() ? QUEST_NORMAL : QUEST_MASTER;

    // RANDOTODO (ADD ITEMLOCATIONS TO GSAVECONTEXT)
}

void SaveManager::InitFileDebug() {
    InitFileNormal();

    // don't apply gDebugSaveFileMode on the title screen
    if (gSaveContext.fileNum != 0xFF) {
        if (CVarGetInteger(CVAR_DEVELOPER_TOOLS("DebugSaveFileMode"), 1) == 2) {
            InitFileMaxed();
            return;
        } else if (CVarGetInteger(CVAR_DEVELOPER_TOOLS("DebugSaveFileMode"), 1) == 0) {
            return;
        }
    }

    gSaveContext.totalDays = 0;
    gSaveContext.bgsDayCount = 0;

    gSaveContext.deaths = 0;
    if (ResourceMgr_GetGameRegion(0) == GAME_REGION_PAL && gSaveContext.language != LANGUAGE_JPN) {
        const static std::array<char, 8> sPlayerName = { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E };
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = sPlayerName[i];
        }
        gSaveContext.ship.filenameLanguage = NAME_LANGUAGE_PAL;
    } else if (gSaveContext.language == LANGUAGE_JPN) { // Japanese
        const static std::array<char, 8> sPlayerName = { 0x81, 0x87, 0x61, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF };
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = sPlayerName[i];
        }
        gSaveContext.ship.filenameLanguage = NAME_LANGUAGE_NTSC_JPN;
    } else { // GAME_REGION_NTSC
        const static std::array<char, 8> sPlayerName = { 0xB6, 0xB3, 0xB8, 0xB5, 0xDF, 0xDF, 0xDF, 0xDF };
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = sPlayerName[i];
        }
        gSaveContext.ship.filenameLanguage =
            (gSaveContext.language == LANGUAGE_JPN) ? NAME_LANGUAGE_NTSC_JPN : NAME_LANGUAGE_NTSC_ENG;
    }
    gSaveContext.n64ddFlag = 0;
    gSaveContext.healthCapacity = 0xE0;
    gSaveContext.health = 0xE0;
    gSaveContext.magicLevel = 0;
    gSaveContext.magic = 0x30;
    gSaveContext.rupees = 150;
    gSaveContext.swordHealth = 8;
    gSaveContext.naviTimer = 0;
    gSaveContext.isMagicAcquired = 1;
    gSaveContext.isDoubleMagicAcquired = 0;
    gSaveContext.isDoubleDefenseAcquired = 0;
    gSaveContext.bgsFlag = 0;
    gSaveContext.ocarinaGameRoundNum = 0;
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.childEquips.buttonItems); button++) {
        gSaveContext.childEquips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots); button++) {
        gSaveContext.childEquips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.childEquips.equipment = 0;
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.adultEquips.buttonItems); button++) {
        gSaveContext.adultEquips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots); button++) {
        gSaveContext.adultEquips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.adultEquips.equipment = 0;
    gSaveContext.unk_54 = 0;
    gSaveContext.savedSceneNum = 0x51;

    // Equipment
    static std::array<u8, 8> sButtonItems = { ITEM_SWORD_MASTER, ITEM_BOW,  ITEM_BOMB, ITEM_OCARINA_FAIRY,
                                              ITEM_NONE,         ITEM_NONE, ITEM_NONE, ITEM_NONE };
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.buttonItems); button++) {
        gSaveContext.equips.buttonItems[button] = sButtonItems[button];
    }
    static std::array<u8, 7> sCButtonSlots = { SLOT_BOW,  SLOT_BOMB, SLOT_OCARINA, SLOT_NONE,
                                               SLOT_NONE, SLOT_NONE, SLOT_NONE };
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.cButtonSlots); button++) {
        gSaveContext.equips.cButtonSlots[button] = sCButtonSlots[button];
    }
    gSaveContext.equips.equipment = 0x1122;

    // Inventory
    static std::array<u8, 24> sItems = {
        ITEM_STICK,     ITEM_NUT,           ITEM_BOMB,         ITEM_BOW,         ITEM_ARROW_FIRE,  ITEM_DINS_FIRE,
        ITEM_SLINGSHOT, ITEM_OCARINA_FAIRY, ITEM_BOMBCHU,      ITEM_HOOKSHOT,    ITEM_ARROW_ICE,   ITEM_FARORES_WIND,
        ITEM_BOOMERANG, ITEM_LENS,          ITEM_BEAN,         ITEM_HAMMER,      ITEM_ARROW_LIGHT, ITEM_NAYRUS_LOVE,
        ITEM_BOTTLE,    ITEM_POTION_RED,    ITEM_POTION_GREEN, ITEM_POTION_BLUE, ITEM_POCKET_EGG,  ITEM_WEIRD_EGG,
    };
    for (int item = 0; item < ARRAY_COUNT(gSaveContext.inventory.items); item++) {
        gSaveContext.inventory.items[item] = sItems[item];
    }
    static std::array<s8, 16> sAmmo = { 50, 50, 10, 30, 1, 1, 30, 1, 50, 1, 1, 1, 1, 1, 1, 1 };
    for (int ammo = 0; ammo < ARRAY_COUNT(gSaveContext.inventory.ammo); ammo++) {
        gSaveContext.inventory.ammo[ammo] = sAmmo[ammo];
    }
    gSaveContext.inventory.equipment = 0x7777;
    gSaveContext.inventory.upgrades = 0x125249;
    gSaveContext.inventory.questItems = 0x1E3FFFF;
    static std::array<u8, 20> sDungeonItems = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (int dungeon = 0; dungeon < ARRAY_COUNT(gSaveContext.inventory.dungeonItems); dungeon++) {
        gSaveContext.inventory.dungeonItems[dungeon] = sDungeonItems[dungeon];
    }
    for (int dungeon = 0; dungeon < ARRAY_COUNT(gSaveContext.inventory.dungeonKeys); dungeon++) {
        gSaveContext.inventory.dungeonKeys[dungeon] = 8;
    }
    gSaveContext.inventory.defenseHearts = 0;
    gSaveContext.inventory.gsTokens = 0;

    gSaveContext.horseData.scene = SCENE_HYRULE_FIELD;
    gSaveContext.horseData.pos.x = -1840;
    gSaveContext.horseData.pos.y = 72;
    gSaveContext.horseData.pos.z = 5497;
    gSaveContext.horseData.angle = -0x6AD9;
    gSaveContext.infTable[0] |= 0x5009;
    gSaveContext.infTable[29] = 0; // unset flag from normal file setup
    gSaveContext.eventChkInf[0] |= 0x123F;
    gSaveContext.eventChkInf[8] |= 1;
    gSaveContext.eventChkInf[12] |= 0x10;

    if (LINK_AGE_IN_YEARS == YEARS_CHILD) {
        gSaveContext.equips.buttonItems[0] = ITEM_SWORD_KOKIRI;
        Inventory_ChangeEquipment(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);
        if (gSaveContext.fileNum == 0xFF) {
            gSaveContext.equips.buttonItems[1] = ITEM_SLINGSHOT;
            gSaveContext.equips.cButtonSlots[0] = SLOT_SLINGSHOT;
            Inventory_ChangeEquipment(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_DEKU);
        }
    }

    gSaveContext.entranceIndex = ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN;
    gSaveContext.magicLevel = 0;
    gSaveContext.sceneFlags[5].swch = 0x40000000;
}

void SaveManager::InitFileMaxed() {
    gSaveContext.totalDays = 0;
    gSaveContext.bgsDayCount = 0;

    gSaveContext.deaths = 0;
    if (ResourceMgr_GetGameRegion(0) == GAME_REGION_PAL && gSaveContext.language != LANGUAGE_JPN) {
        const static std::array<char, 8> sPlayerName = { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E };
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = sPlayerName[i];
        }
        gSaveContext.ship.filenameLanguage = NAME_LANGUAGE_PAL;
    } else if (gSaveContext.language == LANGUAGE_JPN) { // Japanese
        const static std::array<char, 8> sPlayerName = { 0x81, 0x87, 0x61, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF };
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = sPlayerName[i];
        }
        gSaveContext.ship.filenameLanguage = NAME_LANGUAGE_NTSC_JPN;
    } else { // GAME_REGION_NTSC
        const static std::array<char, 8> sPlayerName = { 0xB6, 0xB3, 0xB8, 0xB5, 0xDF, 0xDF, 0xDF, 0xDF };
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
            gSaveContext.playerName[i] = sPlayerName[i];
        }
        gSaveContext.ship.filenameLanguage =
            (gSaveContext.language == LANGUAGE_JPN) ? NAME_LANGUAGE_NTSC_JPN : NAME_LANGUAGE_NTSC_ENG;
    }
    gSaveContext.n64ddFlag = 0;
    gSaveContext.healthCapacity = 0x140;
    gSaveContext.health = 0x140;
    gSaveContext.magicLevel = 2;
    gSaveContext.magic = 0x60;
    gSaveContext.rupees = 500;
    gSaveContext.swordHealth = 8;
    gSaveContext.naviTimer = 0;
    gSaveContext.isMagicAcquired = 1;
    gSaveContext.isDoubleMagicAcquired = 1;
    gSaveContext.isDoubleDefenseAcquired = 1;
    gSaveContext.bgsFlag = 1;
    gSaveContext.ocarinaGameRoundNum = 0;
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.childEquips.buttonItems); button++) {
        gSaveContext.childEquips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots); button++) {
        gSaveContext.childEquips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.childEquips.equipment = 0;
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.adultEquips.buttonItems); button++) {
        gSaveContext.adultEquips.buttonItems[button] = ITEM_NONE;
    }
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots); button++) {
        gSaveContext.adultEquips.cButtonSlots[button] = SLOT_NONE;
    }
    gSaveContext.adultEquips.equipment = 0;
    gSaveContext.unk_54 = 0;
    gSaveContext.savedSceneNum = 0x51;

    // Equipment
    static std::array<u8, 8> sButtonItems = { ITEM_SWORD_MASTER, ITEM_BOW,  ITEM_BOMB, ITEM_OCARINA_TIME,
                                              ITEM_NONE,         ITEM_NONE, ITEM_NONE, ITEM_NONE };
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.buttonItems); button++) {
        gSaveContext.equips.buttonItems[button] = sButtonItems[button];
    }
    static std::array<u8, 7> sCButtonSlots = { SLOT_BOW,  SLOT_BOMB, SLOT_OCARINA, SLOT_NONE,
                                               SLOT_NONE, SLOT_NONE, SLOT_NONE };
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.cButtonSlots); button++) {
        gSaveContext.equips.cButtonSlots[button] = sCButtonSlots[button];
    }
    gSaveContext.equips.equipment = 0x1122;

    // Inventory
    static std::array<u8, 24> sItems = {
        ITEM_STICK,     ITEM_NUT,          ITEM_BOMB,    ITEM_BOW,      ITEM_ARROW_FIRE,  ITEM_DINS_FIRE,
        ITEM_SLINGSHOT, ITEM_OCARINA_TIME, ITEM_BOMBCHU, ITEM_LONGSHOT, ITEM_ARROW_ICE,   ITEM_FARORES_WIND,
        ITEM_BOOMERANG, ITEM_LENS,         ITEM_BEAN,    ITEM_HAMMER,   ITEM_ARROW_LIGHT, ITEM_NAYRUS_LOVE,
        ITEM_FAIRY,     ITEM_FAIRY,        ITEM_BUG,     ITEM_FISH,     ITEM_CLAIM_CHECK, ITEM_MASK_BUNNY,
    };
    for (int item = 0; item < ARRAY_COUNT(gSaveContext.inventory.items); item++) {
        gSaveContext.inventory.items[item] = sItems[item];
    }
    static std::array<s8, 16> sAmmo = { 30, 40, 40, 50, 0, 0, 50, 0, 50, 0, 0, 0, 0, 0, 15, 0 };
    for (int ammo = 0; ammo < ARRAY_COUNT(gSaveContext.inventory.ammo); ammo++) {
        gSaveContext.inventory.ammo[ammo] = sAmmo[ammo];
    }
    gSaveContext.inventory.equipment = 0x7777;
    gSaveContext.inventory.upgrades = 3597531;
    gSaveContext.inventory.questItems = 33554431;
    static std::array<u8, 20> sDungeonItems = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
    for (int dungeon = 0; dungeon < ARRAY_COUNT(gSaveContext.inventory.dungeonItems); dungeon++) {
        gSaveContext.inventory.dungeonItems[dungeon] = sDungeonItems[dungeon];
    }
    for (int dungeon = 0; dungeon < ARRAY_COUNT(gSaveContext.inventory.dungeonKeys); dungeon++) {
        gSaveContext.inventory.dungeonKeys[dungeon] = 9;
    }
    gSaveContext.inventory.defenseHearts = 20;
    gSaveContext.inventory.gsTokens = 100;

    gSaveContext.horseData.scene = SCENE_HYRULE_FIELD;
    gSaveContext.horseData.pos.x = -1840;
    gSaveContext.horseData.pos.y = 72;
    gSaveContext.horseData.pos.z = 5497;
    gSaveContext.horseData.angle = -0x6AD9;
    gSaveContext.infTable[0] |= 0x5009;
    gSaveContext.infTable[29] = 0; // unset flag from normal file setup
    gSaveContext.eventChkInf[0] |= 0x123F;
    gSaveContext.eventChkInf[8] |= 1;
    gSaveContext.eventChkInf[12] |= 0x10;

    // set all the "Entered *" flags for dungeons
    for (int i = 0; i < 0xF; i += 1) {
        Flags_SetInfTable(0x1A0 + i);
    }
    Flags_SetInfTable(INFTABLE_ENTERED_HYRULE_CASTLE);

    Flags_SetEventChkInf(EVENTCHKINF_OPENED_THE_DOOR_OF_TIME);
    Flags_SetEventChkInf(EVENTCHKINF_EPONA_OBTAINED);
    Flags_SetEventChkInf(EVENTCHKINF_KING_ZORA_MOVED);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_NABOORU_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_GOHMA_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_KING_DODONGO_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_PHANTOM_GANON_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_VOLVAGIA_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_MORPHA_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_BARINA_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_BONGO_BONGO_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_GANONDORF_BATTLE);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_MASTER_SWORD_CHAMBER);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_HYRULE_FIELD);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_TRAIL);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_KAKARIKO_VILLAGE);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_ZORAS_DOMAIN);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_HYRULE_CASTLE);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_GORON_CITY);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_TEMPLE_OF_TIME);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_DEKU_TREE);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_DODONGOS_CAVERN);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_LAKE_HYLIA);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_GERUDO_VALLEY);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_GERUDOS_FORTRESS);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_LON_LON_RANCH);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_JABU_JABUS_BELLY);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_GRAVEYARD);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_ZORAS_FOUNTAIN);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_DESERT_COLOSSUS);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_DEATH_MOUNTAIN_CRATER);
    Flags_SetEventChkInf(EVENTCHKINF_ENTERED_GANONS_CASTLE_EXTERIOR);

    if (LINK_AGE_IN_YEARS == YEARS_CHILD) {
        gSaveContext.equips.buttonItems[0] = ITEM_SWORD_KOKIRI;
        Inventory_ChangeEquipment(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);
        if (gSaveContext.fileNum == 0xFF) {
            gSaveContext.equips.buttonItems[1] = ITEM_SLINGSHOT;
            gSaveContext.equips.cButtonSlots[0] = SLOT_SLINGSHOT;
            Inventory_ChangeEquipment(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_DEKU);
        }
    }

    gSaveContext.entranceIndex = ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN;
    gSaveContext.sceneFlags[5].swch = 0x40000000;
}

#if defined(__WIIU__) || defined(__SWITCH__)
// std::filesystem::copy_file doesn't work properly with the Wii U's toolchain atm
int copy_file(const char* src, const char* dst) {
    alignas(0x40) uint8_t buf[4096];
    FILE* r = fopen(src, "r");
    if (!r) {
        return -1;
    }
    FILE* w = fopen(dst, "w");
    if (!w) {
        return -2;
    }

    size_t res;
    while ((res = fread(buf, 1, sizeof(buf), r)) > 0) {
        if (fwrite(buf, 1, res, w) != res) {
            break;
        }
    }

    fclose(r);
    fclose(w);
    return res >= 0 ? 0 : res;
}
#endif

// Threaded SaveFile takes copy of gSaveContext for local unmodified storage

void SaveManager::SaveFileThreaded(int fileNum, SaveContext* saveContext, int sectionID) {
    saveMtx.lock();
    SPDLOG_INFO("Save File - fileNum: {}", fileNum);
    // Needed for first time save, hasn't changed in forever anyway
    saveBlock["version"] = 1;
    if (sectionID == SECTION_ID_BASE) {
        for (auto& sectionHandlerPair : sectionSaveHandlers) {
            auto& saveFuncInfo = sectionHandlerPair.second;
            // Don't call SaveFuncs for sections that aren't tied to game save
            if (!saveFuncInfo.saveWithBase || (saveFuncInfo.name == "randomizer" && !IS_RANDO)) {
                continue;
            }
            nlohmann::json& sectionBlock = saveBlock["sections"][saveFuncInfo.name];
            sectionBlock["version"] = sectionHandlerPair.second.version;
            // If any save file is loaded for medatata, or a spoiler log is loaded (not sure which at this point), there
            // is still data in the "randomizer" section This clears the randomizer data block if and only if the
            // section being called is "randomizer" and the current save file is not a randomizer save file.

            currentJsonContext = &sectionBlock["data"];
            sectionHandlerPair.second.func(saveContext, sectionID, true);
        }
    } else {
        SaveFuncInfo svi = sectionSaveHandlers.find(sectionID)->second;
        auto& sectionName = svi.name;
        auto sectionVersion = svi.version;
        // If section has a parentSection, it is a subsection. Load parentSection version and set sectionBlock to parent
        // string
        if (svi.parentSection != -1 && svi.parentSection < sectionIndex) {
            auto parentSvi = sectionSaveHandlers.find(svi.parentSection)->second;
            sectionName = parentSvi.name;
            sectionVersion = parentSvi.version;
        }
        nlohmann::json& sectionBlock = saveBlock["sections"][sectionName];
        sectionBlock["version"] = sectionVersion;
        currentJsonContext = &sectionBlock["data"];
        svi.func(saveContext, sectionID, false);
    }

    std::filesystem::path fileName = GetFileName(fileNum);
    std::filesystem::path tempFile = GetFileTempName(fileNum);

    if (std::filesystem::exists(tempFile)) {
        std::filesystem::remove(tempFile);
    }

#if defined(__SWITCH__) || defined(__WIIU__)
    FILE* w = fopen(tempFile.c_str(), "w");
    std::string json_string = saveBlock.dump(4);
    fwrite(json_string.c_str(), sizeof(char), json_string.length(), w);
    fclose(w);
#else
    std::ofstream output(tempFile);
    output << std::setw(4) << saveBlock << std::endl;
    output.close();
#endif

    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

#if defined(__SWITCH__) || defined(__WIIU__)
    copy_file(tempFile.c_str(), fileName.c_str());
#else
    std::filesystem::copy_file(tempFile, fileName);
#endif

    if (std::filesystem::exists(tempFile)) {
        std::filesystem::remove(tempFile);
    }

    delete saveContext;
    InitMeta(fileNum);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSaveFile>(fileNum);
    SPDLOG_INFO("Save File Finish - fileNum: {}", fileNum);
    saveMtx.unlock();
}

// SaveSection creates a copy of gSaveContext to prevent mid-save data modification, and passes its reference to
// SaveFileThreaded This should never be called with threaded == false except during file creation
void SaveManager::SaveSection(int fileNum, int sectionID, bool threaded) {
    // Don't save in Boss rush.
    if (fileNum == 0xFF || fileNum == 0xFE) {
        return;
    }
    // Don't save a nonexistent section
    if (sectionID >= sectionIndex) {
        SPDLOG_ERROR("SaveSection: Section ID not registered.");
        return;
    }
    auto saveContext = new SaveContext;
    memcpy(saveContext, &gSaveContext, sizeof(gSaveContext));
    if (threaded) {
        smThreadPool->detach_task(std::bind(&SaveManager::SaveFileThreaded, this, fileNum, saveContext, sectionID));
    } else {
        SaveFileThreaded(fileNum, saveContext, sectionID);
    }
}

void SaveManager::SaveFile(int fileNum) {
    SaveSection(fileNum, SECTION_ID_BASE, true);
}

void SaveManager::SaveGlobal() {
    nlohmann::json globalBlock;
    globalBlock["version"] = 1;
    globalBlock["audioSetting"] = gSaveContext.audioSetting;
    globalBlock["zTargetSetting"] = gSaveContext.zTargetSetting;
    globalBlock["language"] = gSaveContext.language;

    const std::filesystem::path sSavePath(Ship::Context::GetPathRelativeToAppDirectory("Save"));
    const std::filesystem::path sGlobalPath = sSavePath / std::string("global.sav");

    std::ofstream output(sGlobalPath);
    output << std::setw(4) << globalBlock << std::endl;
}

void SaveManager::LoadFile(int fileNum) {
    saveMtx.lock();
    SPDLOG_INFO("Load File - fileNum: {}", fileNum);
    std::filesystem::path fileName = GetFileName(fileNum);
    assert(std::filesystem::exists(fileName));
    InitFile(false);

    std::ifstream input(fileName);

    try {
        saveBlock = nlohmann::json::object();
        input >> saveBlock;
        if (!saveBlock.contains("version")) {
            SPDLOG_ERROR("Save at " + fileName.string() + " contains no version");
            assert(false);
        }
        switch (saveBlock["version"].get<int>()) {
            case 1:
                for (auto& block : saveBlock["sections"].items()) {
                    std::string sectionName = block.key();
                    if (sectionName == "randomizer") {
                        bool hasStats = saveBlock["sections"].contains("sohStats");
                        if (block.value()["data"].contains("aat0") || !hasStats) { // Rachael rando data
                            SohGui::RegisterPopup(
                                "Loading old file",
                                "The file in slot " + std::to_string(fileNum + 1) +
                                    " appears to contain randomizer data, but is a very old format.\n" +
                                    "The randomizer data has been removed, and this file will be treated as a vanilla "
                                    "file.\n" +
                                    "If this was a randomizer file, the file will not work, and should be deleted.");
                            input.close();
                            saveMtx.unlock();
                            SaveFile(fileNum);
                            return;
                        }
                        s16 major = saveBlock["sections"]["sohStats"]["data"]["buildVersionMajor"];
                        s16 minor = saveBlock["sections"]["sohStats"]["data"]["buildVersionMinor"];
                        s16 patch = saveBlock["sections"]["sohStats"]["data"]["buildVersionPatch"];
                        // block loading outdated rando save
                        if (!(major == gBuildVersionMajor && minor == gBuildVersionMinor &&
                              patch == gBuildVersionPatch)) {
                            input.close();
                            std::string newFileName = Ship::Context::GetPathRelativeToAppDirectory("Save") +
                                                      ("/file" + std::to_string(fileNum + 1) + "-" +
                                                       std::to_string(GetUnixTimestamp()) + ".bak");
                            std::filesystem::path newFile(newFileName);

#if defined(__SWITCH__) || defined(__WIIU__)
                            copy_file(fileName.c_str(), newFile.c_str());
#else
                            std::filesystem::copy_file(fileName, newFile);
#endif

                            std::filesystem::remove(fileName);
                            SohGui::RegisterPopup(
                                "Outdated Randomizer Save",
                                "The SoH version in the file in slot " + std::to_string(fileNum + 1) +
                                    " does not match the currently running version.\n" +
                                    "Non-matching rando saves are unsupported, and the file has been renamed to\n" +
                                    "    " + newFileName + "\n" +
                                    "If this was not in error, the file should be deleted.");
                            saveMtx.unlock();
                            SaveFile(fileNum);
                            return;
                        }
                    }
                    int sectionVersion = block.value()["version"];
                    if (sectionName == "randomizer" && sectionVersion != 1) {
                        sectionVersion = 1;
                    }
                    if (!sectionLoadHandlers.contains(sectionName)) {
                        // Unloadable sections aren't necessarily errors, they are probably mods that were unloaded
                        // TODO report in a more noticeable manner
                        SPDLOG_WARN("Save " + GetFileName(fileNum).string() + " contains unloadable section " +
                                    sectionName);
                        continue;
                    }
                    SectionLoadHandler& handler = sectionLoadHandlers[sectionName];
                    if (!handler.contains(sectionVersion)) {
                        // A section that has a loader without a handler for the specific version means that the user
                        // has a mod at an earlier version than the save has. In this case, the user probably wants to
                        // load the save. Report the error so that the user can rectify the error.
                        // TODO report in a more noticeable manner
                        SPDLOG_ERROR("Save " + GetFileName(fileNum).string() + " contains section " + sectionName +
                                     " with an unloadable version " + std::to_string(sectionVersion));
                        assert(false);
                        continue;
                    }
                    currentJsonContext = &block.value()["data"];
                    if (currentJsonContext->empty()) {
                        continue;
                    }
                    handler[sectionVersion]();
                }
                break;
            default:
                SPDLOG_ERROR("Unrecognized save version " + std::to_string(saveBlock["version"].get<int>()) + " in " +
                             GetFileName(fileNum).string());
                assert(false);
                break;
        }
        InitMeta(fileNum);
        GameInteractor::Instance->ExecuteHooks<GameInteractor::OnLoadFile>(fileNum);
    } catch (const std::exception& e) {
        input.close();
        std::filesystem::path newFile(
            Ship::Context::GetPathRelativeToAppDirectory("Save") +
            ("/file" + std::to_string(fileNum + 1) + "-" + std::to_string(GetUnixTimestamp()) + ".bak"));
#if defined(__SWITCH__) || defined(__WIIU__)
        copy_file(fileName.c_str(), newFile.c_str());
#else
        std::filesystem::copy_file(fileName, newFile);
#endif

        std::filesystem::remove(fileName);
        SohGui::RegisterPopup("Error loading save file", "A problem occurred loading the save in slot " +
                                                             std::to_string(fileNum + 1) +
                                                             ".\nSave file corruption is suspected.\n" +
                                                             "The file has been renamed to prevent further issues.");
    }
    saveMtx.unlock();
}

void SaveManager::ThreadPoolWait() {
    if (smThreadPool) {
        smThreadPool->wait();
    }
}

bool SaveManager::SaveFile_Exist(int fileNum) {
    try {
        bool exists = std::filesystem::exists(GetFileName(fileNum));
        SPDLOG_INFO("File[{}] - {}", fileNum, exists ? "exists" : "does not exist");
        return exists;
    } catch (std::filesystem::filesystem_error const& ex) {
        SPDLOG_ERROR("Filesystem error");
        return false;
    }
}

void SaveManager::AddInitFunction(InitFunc func) {
    initFuncs.emplace_back(func);
}

void SaveManager::AddLoadFunction(const std::string& name, int version, LoadFunc func) {
    if (!sectionLoadHandlers.contains(name)) {
        sectionLoadHandlers[name] = SectionLoadHandler();
    }

    if (sectionLoadHandlers[name].contains(version)) {
        SPDLOG_ERROR("Adding load function for section and version that already has one: " + name + ", " +
                     std::to_string(version));
        assert(false);
        return;
    }

    sectionLoadHandlers[name][version] = func;
}

int SaveManager::AddSaveFunction(const std::string& name, int version, SaveFunc func, bool saveWithBase,
                                 int parentSection = -1) {
    if (sectionRegistry.contains(name)) {
        SPDLOG_ERROR("Adding save function for section that already has one: " + name);
        assert(false);
        return -1;
    }

    int index = sectionIndex;
    if (coreSectionIDsByName.contains(name)) {
        index = coreSectionIDsByName.find(name)->second;
    } else {
        sectionIndex++;
    }
    SaveFuncInfo sfi = { name, version, func, saveWithBase, parentSection };
    sectionSaveHandlers.emplace(index, sfi);
    sectionRegistry.emplace(name, index);
    return index;
}

void SaveManager::AddPostFunction(const std::string& name, PostFunc func) {
    if (postHandlers.contains(name)) {
        SPDLOG_ERROR("Adding post function for section that already has one: " + name);
        assert(false);
        return;
    }

    postHandlers[name] = func;
}

// Returns -1 if section name not found
int SaveManager::GetSaveSectionID(std::string& sectionName) {
    if (sectionRegistry.contains(sectionName)) {
        return sectionRegistry.find(sectionName)->second;
    } else {
        return -1;
    }
}

void SaveManager::CreateDefaultGlobal() {
    gSaveContext.audioSetting = 0;
    gSaveContext.zTargetSetting = 0;
    gSaveContext.language = CVarGetInteger(CVAR_SETTING("Languages"), LANGUAGE_ENG);

    SaveGlobal();
}

void SaveManager::LoadBaseVersion1() {
    SaveManager::Instance->LoadData("entranceIndex", gSaveContext.entranceIndex);
    SaveManager::Instance->LoadData("linkAge", gSaveContext.linkAge);
    SaveManager::Instance->LoadData("cutsceneIndex", gSaveContext.cutsceneIndex);
    SaveManager::Instance->LoadData("dayTime", gSaveContext.dayTime);
    SaveManager::Instance->LoadData("nightFlag", gSaveContext.nightFlag);
    SaveManager::Instance->LoadData("totalDays", gSaveContext.totalDays);
    SaveManager::Instance->LoadData("bgsDayCount", gSaveContext.bgsDayCount);
    SaveManager::Instance->LoadData("deaths", gSaveContext.deaths);
    SaveManager::Instance->LoadArray("playerName", ARRAY_COUNT(gSaveContext.playerName),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.playerName[i]); });
    int isRando = 0;
    SaveManager::Instance->LoadData("n64ddFlag", isRando);
    if (isRando) {
        gSaveContext.ship.quest.id = QUEST_RANDOMIZER;
    }
    SaveManager::Instance->LoadData("healthCapacity", gSaveContext.healthCapacity);
    SaveManager::Instance->LoadData("health", gSaveContext.health);
    SaveManager::Instance->LoadData("magicLevel", gSaveContext.magicLevel);
    SaveManager::Instance->LoadData("magic", gSaveContext.magic);
    SaveManager::Instance->LoadData("rupees", gSaveContext.rupees);
    SaveManager::Instance->LoadData("swordHealth", gSaveContext.swordHealth);
    SaveManager::Instance->LoadData("naviTimer", gSaveContext.naviTimer);
    SaveManager::Instance->LoadData("magicAcquired", gSaveContext.isMagicAcquired);
    SaveManager::Instance->LoadData("doubleMagic", gSaveContext.isDoubleMagicAcquired);
    SaveManager::Instance->LoadData("doubleDefense", gSaveContext.isDoubleDefenseAcquired);
    SaveManager::Instance->LoadData("bgsFlag", gSaveContext.bgsFlag);
    SaveManager::Instance->LoadData("ocarinaGameRoundNum", gSaveContext.ocarinaGameRoundNum);
    SaveManager::Instance->LoadStruct("childEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.childEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.childEquips.equipment);
    });
    SaveManager::Instance->LoadStruct("adultEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.adultEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.adultEquips.equipment);
    });
    SaveManager::Instance->LoadData("unk_54", gSaveContext.unk_54);
    SaveManager::Instance->LoadData("savedSceneNum", gSaveContext.savedSceneNum);
    SaveManager::Instance->LoadStruct("equips", []() {
        SaveManager::Instance->LoadArray("buttonItems", ARRAY_COUNT(gSaveContext.equips.buttonItems), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.buttonItems[i], static_cast<uint8_t>(ITEM_NONE));
        });
        SaveManager::Instance->LoadArray("cButtonSlots", ARRAY_COUNT(gSaveContext.equips.cButtonSlots), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.cButtonSlots[i], static_cast<uint8_t>(SLOT_NONE));
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.equips.equipment);
    });
    SaveManager::Instance->LoadStruct("inventory", []() {
        SaveManager::Instance->LoadArray("items", ARRAY_COUNT(gSaveContext.inventory.items), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.items[i]);
        });
        SaveManager::Instance->LoadArray("ammo", ARRAY_COUNT(gSaveContext.inventory.ammo), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.ammo[i]);
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.inventory.equipment);
        SaveManager::Instance->LoadData("upgrades", gSaveContext.inventory.upgrades);
        SaveManager::Instance->LoadData("questItems", gSaveContext.inventory.questItems);
        SaveManager::Instance->LoadArray(
            "dungeonItems", ARRAY_COUNT(gSaveContext.inventory.dungeonItems),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonItems[i]); });
        SaveManager::Instance->LoadArray("dungeonKeys", ARRAY_COUNT(gSaveContext.inventory.dungeonKeys), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonKeys[i]);
        });
        SaveManager::Instance->LoadData("defenseHearts", gSaveContext.inventory.defenseHearts);
        SaveManager::Instance->LoadData("gsTokens", gSaveContext.inventory.gsTokens);
    });
    SaveManager::Instance->LoadArray("sceneFlags", ARRAY_COUNT(gSaveContext.sceneFlags), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("chest", gSaveContext.sceneFlags[i].chest);
            SaveManager::Instance->LoadData("swch", gSaveContext.sceneFlags[i].swch);
            SaveManager::Instance->LoadData("clear", gSaveContext.sceneFlags[i].clear);
            SaveManager::Instance->LoadData("collect", gSaveContext.sceneFlags[i].collect);
            SaveManager::Instance->LoadData("unk", gSaveContext.sceneFlags[i].unk);
            SaveManager::Instance->LoadData("rooms", gSaveContext.sceneFlags[i].rooms);
            SaveManager::Instance->LoadData("floors", gSaveContext.sceneFlags[i].floors);
        });
    });
    SaveManager::Instance->LoadStruct("fw", []() {
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.fw.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.fw.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.fw.pos.z);
        });
        SaveManager::Instance->LoadData("yaw", gSaveContext.fw.yaw);
        SaveManager::Instance->LoadData("playerParams", gSaveContext.fw.playerParams);
        SaveManager::Instance->LoadData("entranceIndex", gSaveContext.fw.entranceIndex);
        SaveManager::Instance->LoadData("roomIndex", gSaveContext.fw.roomIndex);
        SaveManager::Instance->LoadData("set", gSaveContext.fw.set);
        SaveManager::Instance->LoadData("tempSwchFlags", gSaveContext.fw.tempSwchFlags);
        SaveManager::Instance->LoadData("tempCollectFlags", gSaveContext.fw.tempCollectFlags);
    });
    SaveManager::Instance->LoadArray("gsFlags", ARRAY_COUNT(gSaveContext.gsFlags),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.gsFlags[i]); });
    SaveManager::Instance->LoadArray("highScores", ARRAY_COUNT(gSaveContext.highScores),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.highScores[i]); });
    SaveManager::Instance->LoadArray("eventChkInf", ARRAY_COUNT(gSaveContext.eventChkInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.eventChkInf[i]);
    });
    SaveManager::Instance->LoadArray("itemGetInf", ARRAY_COUNT(gSaveContext.itemGetInf),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.itemGetInf[i]); });
    SaveManager::Instance->LoadArray("infTable", ARRAY_COUNT(gSaveContext.infTable),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.infTable[i]); });
    SaveManager::Instance->LoadData("worldMapAreaData", gSaveContext.worldMapAreaData);
    SaveManager::Instance->LoadData("scarecrowCustomSongSet", gSaveContext.scarecrowLongSongSet);
    SaveManager::Instance->LoadArray("scarecrowCustomSong", sizeof(gSaveContext.scarecrowLongSong), [](size_t i) {
        SaveManager::Instance->LoadData("", ((u8*)&gSaveContext.scarecrowLongSong)[i]);
    });
    SaveManager::Instance->LoadData("scarecrowSpawnSongSet", gSaveContext.scarecrowSpawnSongSet);
    SaveManager::Instance->LoadArray("scarecrowSpawnSong", sizeof(gSaveContext.scarecrowSpawnSong), [](size_t i) {
        SaveManager::Instance->LoadData("", ((u8*)&gSaveContext.scarecrowSpawnSong)[i]);
    });
    SaveManager::Instance->LoadStruct("horseData", []() {
        SaveManager::Instance->LoadData("scene", gSaveContext.horseData.scene);
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.horseData.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.horseData.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.horseData.pos.z);
        });
        SaveManager::Instance->LoadData("angle", gSaveContext.horseData.angle);
    });

    SaveManager::Instance->LoadArray("randomizerInf", ARRAY_COUNT(gSaveContext.ship.randomizerInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.ship.randomizerInf[i]);
    });
}

void SaveManager::LoadBaseVersion2() {
    SaveManager::Instance->LoadData("entranceIndex", gSaveContext.entranceIndex);
    SaveManager::Instance->LoadData("linkAge", gSaveContext.linkAge);
    SaveManager::Instance->LoadData("cutsceneIndex", gSaveContext.cutsceneIndex);
    SaveManager::Instance->LoadData("dayTime", gSaveContext.dayTime);
    SaveManager::Instance->LoadData("nightFlag", gSaveContext.nightFlag);
    SaveManager::Instance->LoadData("totalDays", gSaveContext.totalDays);
    SaveManager::Instance->LoadData("bgsDayCount", gSaveContext.bgsDayCount);
    SaveManager::Instance->LoadData("deaths", gSaveContext.deaths);
    SaveManager::Instance->LoadArray("playerName", ARRAY_COUNT(gSaveContext.playerName),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.playerName[i]); });
    int isRando = 0;
    SaveManager::Instance->LoadData("n64ddFlag", isRando);
    if (isRando) {
        gSaveContext.ship.quest.id = QUEST_RANDOMIZER;
    }
    SaveManager::Instance->LoadData("healthCapacity", gSaveContext.healthCapacity);
    SaveManager::Instance->LoadData("health", gSaveContext.health);
    SaveManager::Instance->LoadData("magicLevel", gSaveContext.magicLevel);
    SaveManager::Instance->LoadData("magic", gSaveContext.magic);
    SaveManager::Instance->LoadData("rupees", gSaveContext.rupees);
    SaveManager::Instance->LoadData("swordHealth", gSaveContext.swordHealth);
    SaveManager::Instance->LoadData("naviTimer", gSaveContext.naviTimer);
    SaveManager::Instance->LoadData("magicAcquired", gSaveContext.isMagicAcquired);
    SaveManager::Instance->LoadData("doubleMagic", gSaveContext.isDoubleMagicAcquired);
    SaveManager::Instance->LoadData("doubleDefense", gSaveContext.isDoubleDefenseAcquired);
    SaveManager::Instance->LoadData("bgsFlag", gSaveContext.bgsFlag);
    SaveManager::Instance->LoadData("ocarinaGameRoundNum", gSaveContext.ocarinaGameRoundNum);
    SaveManager::Instance->LoadStruct("childEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.childEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.childEquips.equipment);
    });
    SaveManager::Instance->LoadStruct("adultEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.adultEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.adultEquips.equipment);
    });
    SaveManager::Instance->LoadData("unk_54", gSaveContext.unk_54);
    SaveManager::Instance->LoadData("savedSceneNum", gSaveContext.savedSceneNum);
    SaveManager::Instance->LoadStruct("equips", []() {
        SaveManager::Instance->LoadArray("buttonItems", ARRAY_COUNT(gSaveContext.equips.buttonItems), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.buttonItems[i], static_cast<uint8_t>(ITEM_NONE));
        });
        SaveManager::Instance->LoadArray("cButtonSlots", ARRAY_COUNT(gSaveContext.equips.cButtonSlots), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.cButtonSlots[i], static_cast<uint8_t>(SLOT_NONE));
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.equips.equipment);
    });
    SaveManager::Instance->LoadStruct("inventory", []() {
        SaveManager::Instance->LoadArray("items", ARRAY_COUNT(gSaveContext.inventory.items), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.items[i]);
        });
        SaveManager::Instance->LoadArray("ammo", ARRAY_COUNT(gSaveContext.inventory.ammo), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.ammo[i]);
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.inventory.equipment);
        SaveManager::Instance->LoadData("upgrades", gSaveContext.inventory.upgrades);
        SaveManager::Instance->LoadData("questItems", gSaveContext.inventory.questItems);
        SaveManager::Instance->LoadArray(
            "dungeonItems", ARRAY_COUNT(gSaveContext.inventory.dungeonItems),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonItems[i]); });
        SaveManager::Instance->LoadArray("dungeonKeys", ARRAY_COUNT(gSaveContext.inventory.dungeonKeys), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonKeys[i]);
        });
        SaveManager::Instance->LoadData("defenseHearts", gSaveContext.inventory.defenseHearts);
        SaveManager::Instance->LoadData("gsTokens", gSaveContext.inventory.gsTokens);
    });
    SaveManager::Instance->LoadStruct("sohStats", []() {
        SaveManager::Instance->LoadData("heartPieces", gSaveContext.ship.stats.heartPieces);
        SaveManager::Instance->LoadData("heartContainers", gSaveContext.ship.stats.heartContainers);
        SaveManager::Instance->LoadArray("dungeonKeys", ARRAY_COUNT(gSaveContext.ship.stats.dungeonKeys), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.ship.stats.dungeonKeys[i]);
        });
        SaveManager::Instance->LoadData("rtaTiming", gSaveContext.ship.stats.rtaTiming);
        SaveManager::Instance->LoadData("fileCreatedAt", gSaveContext.ship.stats.fileCreatedAt);
        SaveManager::Instance->LoadData("playTimer", gSaveContext.ship.stats.playTimer);
        SaveManager::Instance->LoadData("pauseTimer", gSaveContext.ship.stats.pauseTimer);
        SaveManager::Instance->LoadArray(
            "timestamps", ARRAY_COUNT(gSaveContext.ship.stats.itemTimestamp),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.ship.stats.itemTimestamp[i]); });
        SaveManager::Instance->LoadArray("counts", ARRAY_COUNT(gSaveContext.ship.stats.count), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.ship.stats.count[i]);
        });
        SaveManager::Instance->LoadArray(
            "scenesDiscovered", ARRAY_COUNT(gSaveContext.ship.stats.scenesDiscovered),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.ship.stats.scenesDiscovered[i]); });
        SaveManager::Instance->LoadArray(
            "entrancesDiscovered", ARRAY_COUNT(gSaveContext.ship.stats.entrancesDiscovered),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.ship.stats.entrancesDiscovered[i]); });
    });
    SaveManager::Instance->LoadArray("sceneFlags", ARRAY_COUNT(gSaveContext.sceneFlags), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("chest", gSaveContext.sceneFlags[i].chest);
            SaveManager::Instance->LoadData("swch", gSaveContext.sceneFlags[i].swch);
            SaveManager::Instance->LoadData("clear", gSaveContext.sceneFlags[i].clear);
            SaveManager::Instance->LoadData("collect", gSaveContext.sceneFlags[i].collect);
            SaveManager::Instance->LoadData("unk", gSaveContext.sceneFlags[i].unk);
            SaveManager::Instance->LoadData("rooms", gSaveContext.sceneFlags[i].rooms);
            SaveManager::Instance->LoadData("floors", gSaveContext.sceneFlags[i].floors);
        });
    });
    SaveManager::Instance->LoadStruct("fw", []() {
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.fw.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.fw.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.fw.pos.z);
        });
        SaveManager::Instance->LoadData("yaw", gSaveContext.fw.yaw);
        SaveManager::Instance->LoadData("playerParams", gSaveContext.fw.playerParams);
        SaveManager::Instance->LoadData("entranceIndex", gSaveContext.fw.entranceIndex);
        SaveManager::Instance->LoadData("roomIndex", gSaveContext.fw.roomIndex);
        SaveManager::Instance->LoadData("set", gSaveContext.fw.set);
        SaveManager::Instance->LoadData("tempSwchFlags", gSaveContext.fw.tempSwchFlags);
        SaveManager::Instance->LoadData("tempCollectFlags", gSaveContext.fw.tempCollectFlags);
    });
    SaveManager::Instance->LoadArray("gsFlags", ARRAY_COUNT(gSaveContext.gsFlags),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.gsFlags[i]); });
    SaveManager::Instance->LoadArray("highScores", ARRAY_COUNT(gSaveContext.highScores),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.highScores[i]); });
    SaveManager::Instance->LoadArray("eventChkInf", ARRAY_COUNT(gSaveContext.eventChkInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.eventChkInf[i]);
    });
    SaveManager::Instance->LoadArray("itemGetInf", ARRAY_COUNT(gSaveContext.itemGetInf),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.itemGetInf[i]); });
    SaveManager::Instance->LoadArray("infTable", ARRAY_COUNT(gSaveContext.infTable),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.infTable[i]); });
    SaveManager::Instance->LoadData("worldMapAreaData", gSaveContext.worldMapAreaData);
    SaveManager::Instance->LoadData("scarecrowCustomSongSet", gSaveContext.scarecrowLongSongSet);
    SaveManager::Instance->LoadArray("scarecrowCustomSong", ARRAY_COUNT(gSaveContext.scarecrowLongSong), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowLongSong[i].noteIdx);
            SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowLongSong[i].unk_01);
            SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowLongSong[i].unk_02);
            SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowLongSong[i].volume);
            SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowLongSong[i].vibrato);
            SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowLongSong[i].tone);
            SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowLongSong[i].semitone);
        });
    });
    SaveManager::Instance->LoadData("scarecrowSpawnSongSet", gSaveContext.scarecrowSpawnSongSet);
    SaveManager::Instance->LoadArray("scarecrowSpawnSong", ARRAY_COUNT(gSaveContext.scarecrowSpawnSong), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowSpawnSong[i].noteIdx);
            SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowSpawnSong[i].unk_01);
            SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowSpawnSong[i].unk_02);
            SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowSpawnSong[i].volume);
            SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowSpawnSong[i].vibrato);
            SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowSpawnSong[i].tone);
            SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowSpawnSong[i].semitone);
        });
    });
    SaveManager::Instance->LoadStruct("horseData", []() {
        SaveManager::Instance->LoadData("scene", gSaveContext.horseData.scene);
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.horseData.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.horseData.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.horseData.pos.z);
        });
        SaveManager::Instance->LoadData("angle", gSaveContext.horseData.angle);
    });

    SaveManager::Instance->LoadArray("randomizerInf", ARRAY_COUNT(gSaveContext.ship.randomizerInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.ship.randomizerInf[i]);
    });
    int isMQ = 0;
    SaveManager::Instance->LoadData("isMasterQuest", isMQ);
    if (isMQ) {
        gSaveContext.ship.quest.id = QUEST_MASTER;
    }

    // Workaround for breaking save compatibility from 5.0.2 -> 5.1.0 in commit d7c35221421bf712b5ead56a360f81f624aca4bc
    if (!gSaveContext.isMagicAcquired) {
        SaveManager::Instance->LoadData("isMagicAcquired", gSaveContext.isMagicAcquired);
    }
    if (!gSaveContext.isDoubleMagicAcquired) {
        SaveManager::Instance->LoadData("isDoubleMagicAcquired", gSaveContext.isDoubleMagicAcquired);
    }
    if (!gSaveContext.isDoubleDefenseAcquired) {
        SaveManager::Instance->LoadData("isDoubleDefenseAcquired", gSaveContext.isDoubleDefenseAcquired);
    }
    if (!gSaveContext.scarecrowLongSongSet) {
        SaveManager::Instance->LoadData("scarecrowLongSongSet", gSaveContext.scarecrowLongSongSet);
        if (gSaveContext.scarecrowLongSongSet) {
            SaveManager::Instance->LoadArray(
                "scarecrowLongSong", ARRAY_COUNT(gSaveContext.scarecrowLongSong), [](size_t i) {
                    SaveManager::Instance->LoadStruct("", [&i]() {
                        SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowLongSong[i].noteIdx);
                        SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowLongSong[i].unk_01);
                        SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowLongSong[i].unk_02);
                        SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowLongSong[i].volume);
                        SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowLongSong[i].vibrato);
                        SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowLongSong[i].tone);
                        SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowLongSong[i].semitone);
                    });
                });
        }
    }
}

void SaveManager::LoadBaseVersion3() {
    SaveManager::Instance->LoadData("entranceIndex", gSaveContext.entranceIndex);
    SaveManager::Instance->LoadData("linkAge", gSaveContext.linkAge);
    SaveManager::Instance->LoadData("cutsceneIndex", gSaveContext.cutsceneIndex);
    SaveManager::Instance->LoadData("dayTime", gSaveContext.dayTime);
    SaveManager::Instance->LoadData("nightFlag", gSaveContext.nightFlag);
    SaveManager::Instance->LoadData("totalDays", gSaveContext.totalDays);
    SaveManager::Instance->LoadData("bgsDayCount", gSaveContext.bgsDayCount);
    SaveManager::Instance->LoadData("deaths", gSaveContext.deaths);
    SaveManager::Instance->LoadArray("playerName", ARRAY_COUNT(gSaveContext.playerName),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.playerName[i]); });
    int isRando = 0;
    SaveManager::Instance->LoadData("n64ddFlag", isRando);
    if (isRando) {
        gSaveContext.ship.quest.id = QUEST_RANDOMIZER;
    }
    SaveManager::Instance->LoadData("healthCapacity", gSaveContext.healthCapacity);
    SaveManager::Instance->LoadData("health", gSaveContext.health);
    SaveManager::Instance->LoadData("magicLevel", gSaveContext.magicLevel);
    SaveManager::Instance->LoadData("magic", gSaveContext.magic);
    SaveManager::Instance->LoadData("rupees", gSaveContext.rupees);
    SaveManager::Instance->LoadData("swordHealth", gSaveContext.swordHealth);
    SaveManager::Instance->LoadData("naviTimer", gSaveContext.naviTimer);
    SaveManager::Instance->LoadData("isMagicAcquired", gSaveContext.isMagicAcquired);
    SaveManager::Instance->LoadData("isDoubleMagicAcquired", gSaveContext.isDoubleMagicAcquired);
    SaveManager::Instance->LoadData("isDoubleDefenseAcquired", gSaveContext.isDoubleDefenseAcquired);
    SaveManager::Instance->LoadData("bgsFlag", gSaveContext.bgsFlag);
    SaveManager::Instance->LoadData("ocarinaGameRoundNum", gSaveContext.ocarinaGameRoundNum);
    SaveManager::Instance->LoadStruct("childEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.childEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.childEquips.equipment);
    });
    SaveManager::Instance->LoadStruct("adultEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.adultEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.adultEquips.equipment);
    });
    SaveManager::Instance->LoadData("unk_54", gSaveContext.unk_54);
    SaveManager::Instance->LoadData("savedSceneNum", gSaveContext.savedSceneNum);
    SaveManager::Instance->LoadStruct("equips", []() {
        SaveManager::Instance->LoadArray("buttonItems", ARRAY_COUNT(gSaveContext.equips.buttonItems), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.buttonItems[i], static_cast<uint8_t>(ITEM_NONE));
        });
        SaveManager::Instance->LoadArray("cButtonSlots", ARRAY_COUNT(gSaveContext.equips.cButtonSlots), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.cButtonSlots[i], static_cast<uint8_t>(SLOT_NONE));
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.equips.equipment);
    });
    SaveManager::Instance->LoadStruct("inventory", []() {
        SaveManager::Instance->LoadArray("items", ARRAY_COUNT(gSaveContext.inventory.items), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.items[i]);
        });
        SaveManager::Instance->LoadArray("ammo", ARRAY_COUNT(gSaveContext.inventory.ammo), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.ammo[i]);
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.inventory.equipment);
        SaveManager::Instance->LoadData("upgrades", gSaveContext.inventory.upgrades);
        SaveManager::Instance->LoadData("questItems", gSaveContext.inventory.questItems);
        SaveManager::Instance->LoadArray(
            "dungeonItems", ARRAY_COUNT(gSaveContext.inventory.dungeonItems),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonItems[i]); });
        SaveManager::Instance->LoadArray("dungeonKeys", ARRAY_COUNT(gSaveContext.inventory.dungeonKeys), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonKeys[i]);
        });
        SaveManager::Instance->LoadData("defenseHearts", gSaveContext.inventory.defenseHearts);
        SaveManager::Instance->LoadData("gsTokens", gSaveContext.inventory.gsTokens);
    });
    SaveManager::Instance->LoadStruct("sohStats", []() {
        SaveManager::Instance->LoadCharArray("buildVersion", gSaveContext.ship.stats.buildVersion,
                                             ARRAY_COUNT(gSaveContext.ship.stats.buildVersion));
        SaveManager::Instance->LoadData("buildVersionMajor", gSaveContext.ship.stats.buildVersionMajor);
        SaveManager::Instance->LoadData("buildVersionMinor", gSaveContext.ship.stats.buildVersionMinor);
        SaveManager::Instance->LoadData("buildVersionPatch", gSaveContext.ship.stats.buildVersionPatch);

        SaveManager::Instance->LoadData("heartPieces", gSaveContext.ship.stats.heartPieces);
        SaveManager::Instance->LoadData("heartContainers", gSaveContext.ship.stats.heartContainers);
        SaveManager::Instance->LoadArray("dungeonKeys", ARRAY_COUNT(gSaveContext.ship.stats.dungeonKeys), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.ship.stats.dungeonKeys[i]);
        });
        SaveManager::Instance->LoadData("rtaTiming", gSaveContext.ship.stats.rtaTiming);
        SaveManager::Instance->LoadData("fileCreatedAt", gSaveContext.ship.stats.fileCreatedAt);
        SaveManager::Instance->LoadData("playTimer", gSaveContext.ship.stats.playTimer);
        SaveManager::Instance->LoadData("pauseTimer", gSaveContext.ship.stats.pauseTimer);
        SaveManager::Instance->LoadArray(
            "itemTimestamps", ARRAY_COUNT(gSaveContext.ship.stats.itemTimestamp),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.ship.stats.itemTimestamp[i]); });
        SaveManager::Instance->LoadArray(
            "sceneTimestamps", ARRAY_COUNT(gSaveContext.ship.stats.sceneTimestamps), [](size_t i) {
                SaveManager::Instance->LoadStruct("", [&i]() {
                    SaveManager::Instance->LoadData("scene", gSaveContext.ship.stats.sceneTimestamps[i].scene);
                    SaveManager::Instance->LoadData("room", gSaveContext.ship.stats.sceneTimestamps[i].room);
                    SaveManager::Instance->LoadData("sceneTime", gSaveContext.ship.stats.sceneTimestamps[i].sceneTime);
                    SaveManager::Instance->LoadData("roomTime", gSaveContext.ship.stats.sceneTimestamps[i].roomTime);
                    SaveManager::Instance->LoadData("isRoom", gSaveContext.ship.stats.sceneTimestamps[i].isRoom);
                });
            });
        SaveManager::Instance->LoadData("tsIdx", gSaveContext.ship.stats.tsIdx);
        SaveManager::Instance->LoadArray("counts", ARRAY_COUNT(gSaveContext.ship.stats.count), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.ship.stats.count[i]);
        });
        SaveManager::Instance->LoadArray(
            "scenesDiscovered", ARRAY_COUNT(gSaveContext.ship.stats.scenesDiscovered),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.ship.stats.scenesDiscovered[i]); });
        SaveManager::Instance->LoadArray(
            "entrancesDiscovered", ARRAY_COUNT(gSaveContext.ship.stats.entrancesDiscovered),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.ship.stats.entrancesDiscovered[i]); });
    });
    SaveManager::Instance->LoadArray("sceneFlags", ARRAY_COUNT(gSaveContext.sceneFlags), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("chest", gSaveContext.sceneFlags[i].chest);
            SaveManager::Instance->LoadData("swch", gSaveContext.sceneFlags[i].swch);
            SaveManager::Instance->LoadData("clear", gSaveContext.sceneFlags[i].clear);
            SaveManager::Instance->LoadData("collect", gSaveContext.sceneFlags[i].collect);
            SaveManager::Instance->LoadData("unk", gSaveContext.sceneFlags[i].unk);
            SaveManager::Instance->LoadData("rooms", gSaveContext.sceneFlags[i].rooms);
            SaveManager::Instance->LoadData("floors", gSaveContext.sceneFlags[i].floors);
        });
    });
    SaveManager::Instance->LoadStruct("fw", []() {
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.fw.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.fw.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.fw.pos.z);
        });
        SaveManager::Instance->LoadData("yaw", gSaveContext.fw.yaw);
        SaveManager::Instance->LoadData("playerParams", gSaveContext.fw.playerParams);
        SaveManager::Instance->LoadData("entranceIndex", gSaveContext.fw.entranceIndex);
        SaveManager::Instance->LoadData("roomIndex", gSaveContext.fw.roomIndex);
        SaveManager::Instance->LoadData("set", gSaveContext.fw.set);
        SaveManager::Instance->LoadData("tempSwchFlags", gSaveContext.fw.tempSwchFlags);
        SaveManager::Instance->LoadData("tempCollectFlags", gSaveContext.fw.tempCollectFlags);
    });
    SaveManager::Instance->LoadArray("gsFlags", ARRAY_COUNT(gSaveContext.gsFlags),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.gsFlags[i]); });
    SaveManager::Instance->LoadArray("highScores", ARRAY_COUNT(gSaveContext.highScores),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.highScores[i]); });
    SaveManager::Instance->LoadArray("eventChkInf", ARRAY_COUNT(gSaveContext.eventChkInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.eventChkInf[i]);
    });
    SaveManager::Instance->LoadArray("itemGetInf", ARRAY_COUNT(gSaveContext.itemGetInf),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.itemGetInf[i]); });
    SaveManager::Instance->LoadArray("infTable", ARRAY_COUNT(gSaveContext.infTable),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.infTable[i]); });
    SaveManager::Instance->LoadData("worldMapAreaData", gSaveContext.worldMapAreaData);
    SaveManager::Instance->LoadData("scarecrowLongSongSet", gSaveContext.scarecrowLongSongSet);
    SaveManager::Instance->LoadArray("scarecrowLongSong", ARRAY_COUNT(gSaveContext.scarecrowLongSong), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowLongSong[i].noteIdx);
            SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowLongSong[i].unk_01);
            SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowLongSong[i].unk_02);
            SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowLongSong[i].volume);
            SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowLongSong[i].vibrato);
            SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowLongSong[i].tone);
            SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowLongSong[i].semitone);
        });
    });
    SaveManager::Instance->LoadData("scarecrowSpawnSongSet", gSaveContext.scarecrowSpawnSongSet);
    SaveManager::Instance->LoadArray("scarecrowSpawnSong", ARRAY_COUNT(gSaveContext.scarecrowSpawnSong), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowSpawnSong[i].noteIdx);
            SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowSpawnSong[i].unk_01);
            SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowSpawnSong[i].unk_02);
            SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowSpawnSong[i].volume);
            SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowSpawnSong[i].vibrato);
            SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowSpawnSong[i].tone);
            SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowSpawnSong[i].semitone);
        });
    });
    SaveManager::Instance->LoadStruct("horseData", []() {
        SaveManager::Instance->LoadData("scene", gSaveContext.horseData.scene);
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.horseData.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.horseData.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.horseData.pos.z);
        });
        SaveManager::Instance->LoadData("angle", gSaveContext.horseData.angle);
    });

    SaveManager::Instance->LoadArray("randomizerInf", ARRAY_COUNT(gSaveContext.ship.randomizerInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.ship.randomizerInf[i]);
    });
    int isMQ = 0;
    SaveManager::Instance->LoadData("isMasterQuest", isMQ);
    if (isMQ) {
        gSaveContext.ship.quest.id = QUEST_MASTER;
    }
    SaveManager::Instance->LoadStruct("backupFW", []() {
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.ship.backupFW.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.ship.backupFW.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.ship.backupFW.pos.z);
        });
        SaveManager::Instance->LoadData("yaw", gSaveContext.ship.backupFW.yaw);
        SaveManager::Instance->LoadData("playerParams", gSaveContext.ship.backupFW.playerParams);
        SaveManager::Instance->LoadData("entranceIndex", gSaveContext.ship.backupFW.entranceIndex);
        SaveManager::Instance->LoadData("roomIndex", gSaveContext.ship.backupFW.roomIndex);
        SaveManager::Instance->LoadData("set", gSaveContext.ship.backupFW.set);
        SaveManager::Instance->LoadData("tempSwchFlags", gSaveContext.ship.backupFW.tempSwchFlags);
        SaveManager::Instance->LoadData("tempCollectFlags", gSaveContext.ship.backupFW.tempCollectFlags);
    });
    SaveManager::Instance->LoadData("dogParams", gSaveContext.dogParams);
}

void SaveManager::LoadBaseVersion4() {
    SaveManager::Instance->LoadData("entranceIndex", gSaveContext.entranceIndex);
    SaveManager::Instance->LoadData("linkAge", gSaveContext.linkAge);
    SaveManager::Instance->LoadData("cutsceneIndex", gSaveContext.cutsceneIndex);
    SaveManager::Instance->LoadData("dayTime", gSaveContext.dayTime);
    SaveManager::Instance->LoadData("nightFlag", gSaveContext.nightFlag);
    SaveManager::Instance->LoadData("totalDays", gSaveContext.totalDays);
    SaveManager::Instance->LoadData("bgsDayCount", gSaveContext.bgsDayCount);
    SaveManager::Instance->LoadData("deaths", gSaveContext.deaths);
    SaveManager::Instance->LoadArray("playerName", ARRAY_COUNT(gSaveContext.playerName),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.playerName[i]); });
    int isRando = 0;
    SaveManager::Instance->LoadData("n64ddFlag", isRando);
    if (isRando) {
        gSaveContext.ship.quest.id = QUEST_RANDOMIZER;
    }
    SaveManager::Instance->LoadData("healthCapacity", gSaveContext.healthCapacity);
    SaveManager::Instance->LoadData("health", gSaveContext.health);
    SaveManager::Instance->LoadData("magicLevel", gSaveContext.magicLevel);
    SaveManager::Instance->LoadData("magic", gSaveContext.magic);
    SaveManager::Instance->LoadData("rupees", gSaveContext.rupees);
    SaveManager::Instance->LoadData("swordHealth", gSaveContext.swordHealth);
    SaveManager::Instance->LoadData("naviTimer", gSaveContext.naviTimer);
    SaveManager::Instance->LoadData("isMagicAcquired", gSaveContext.isMagicAcquired);
    SaveManager::Instance->LoadData("isDoubleMagicAcquired", gSaveContext.isDoubleMagicAcquired);
    SaveManager::Instance->LoadData("isDoubleDefenseAcquired", gSaveContext.isDoubleDefenseAcquired);
    SaveManager::Instance->LoadData("bgsFlag", gSaveContext.bgsFlag);
    SaveManager::Instance->LoadData("ocarinaGameRoundNum", gSaveContext.ocarinaGameRoundNum);
    SaveManager::Instance->LoadStruct("childEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.childEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.childEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.childEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.childEquips.equipment);
    });
    SaveManager::Instance->LoadStruct("adultEquips", []() {
        SaveManager::Instance->LoadArray(
            "buttonItems", ARRAY_COUNT(gSaveContext.adultEquips.buttonItems), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.buttonItems[i],
                                                static_cast<uint8_t>(ITEM_NONE));
            });
        SaveManager::Instance->LoadArray(
            "cButtonSlots", ARRAY_COUNT(gSaveContext.adultEquips.cButtonSlots), [](size_t i) {
                SaveManager::Instance->LoadData("", gSaveContext.adultEquips.cButtonSlots[i],
                                                static_cast<uint8_t>(SLOT_NONE));
            });
        SaveManager::Instance->LoadData("equipment", gSaveContext.adultEquips.equipment);
    });
    SaveManager::Instance->LoadData("unk_54", gSaveContext.unk_54);
    SaveManager::Instance->LoadData("savedSceneNum", gSaveContext.savedSceneNum);
    SaveManager::Instance->LoadStruct("equips", []() {
        SaveManager::Instance->LoadArray("buttonItems", ARRAY_COUNT(gSaveContext.equips.buttonItems), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.buttonItems[i], static_cast<uint8_t>(ITEM_NONE));
        });
        SaveManager::Instance->LoadArray("cButtonSlots", ARRAY_COUNT(gSaveContext.equips.cButtonSlots), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.equips.cButtonSlots[i], static_cast<uint8_t>(SLOT_NONE));
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.equips.equipment);
    });
    SaveManager::Instance->LoadStruct("inventory", []() {
        SaveManager::Instance->LoadArray("items", ARRAY_COUNT(gSaveContext.inventory.items), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.items[i]);
        });
        SaveManager::Instance->LoadArray("ammo", ARRAY_COUNT(gSaveContext.inventory.ammo), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.ammo[i]);
        });
        SaveManager::Instance->LoadData("equipment", gSaveContext.inventory.equipment);
        SaveManager::Instance->LoadData("upgrades", gSaveContext.inventory.upgrades);
        SaveManager::Instance->LoadData("questItems", gSaveContext.inventory.questItems);
        SaveManager::Instance->LoadArray(
            "dungeonItems", ARRAY_COUNT(gSaveContext.inventory.dungeonItems),
            [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonItems[i]); });
        SaveManager::Instance->LoadArray("dungeonKeys", ARRAY_COUNT(gSaveContext.inventory.dungeonKeys), [](size_t i) {
            SaveManager::Instance->LoadData("", gSaveContext.inventory.dungeonKeys[i]);
        });
        SaveManager::Instance->LoadData("defenseHearts", gSaveContext.inventory.defenseHearts);
        SaveManager::Instance->LoadData("gsTokens", gSaveContext.inventory.gsTokens);
    });
    SaveManager::Instance->LoadArray("sceneFlags", ARRAY_COUNT(gSaveContext.sceneFlags), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("chest", gSaveContext.sceneFlags[i].chest);
            SaveManager::Instance->LoadData("swch", gSaveContext.sceneFlags[i].swch);
            SaveManager::Instance->LoadData("clear", gSaveContext.sceneFlags[i].clear);
            SaveManager::Instance->LoadData("collect", gSaveContext.sceneFlags[i].collect);
            SaveManager::Instance->LoadData("unk", gSaveContext.sceneFlags[i].unk);
            SaveManager::Instance->LoadData("rooms", gSaveContext.sceneFlags[i].rooms);
            SaveManager::Instance->LoadData("floors", gSaveContext.sceneFlags[i].floors);
        });
    });
    SaveManager::Instance->LoadStruct("fw", []() {
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.fw.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.fw.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.fw.pos.z);
        });
        SaveManager::Instance->LoadData("yaw", gSaveContext.fw.yaw);
        SaveManager::Instance->LoadData("playerParams", gSaveContext.fw.playerParams);
        SaveManager::Instance->LoadData("entranceIndex", gSaveContext.fw.entranceIndex);
        SaveManager::Instance->LoadData("roomIndex", gSaveContext.fw.roomIndex);
        SaveManager::Instance->LoadData("set", gSaveContext.fw.set);
        SaveManager::Instance->LoadData("tempSwchFlags", gSaveContext.fw.tempSwchFlags);
        SaveManager::Instance->LoadData("tempCollectFlags", gSaveContext.fw.tempCollectFlags);
    });
    SaveManager::Instance->LoadArray("gsFlags", ARRAY_COUNT(gSaveContext.gsFlags),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.gsFlags[i]); });
    SaveManager::Instance->LoadArray("highScores", ARRAY_COUNT(gSaveContext.highScores),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.highScores[i]); });
    SaveManager::Instance->LoadArray("eventChkInf", ARRAY_COUNT(gSaveContext.eventChkInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.eventChkInf[i]);
    });
    SaveManager::Instance->LoadArray("itemGetInf", ARRAY_COUNT(gSaveContext.itemGetInf),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.itemGetInf[i]); });
    SaveManager::Instance->LoadArray("infTable", ARRAY_COUNT(gSaveContext.infTable),
                                     [](size_t i) { SaveManager::Instance->LoadData("", gSaveContext.infTable[i]); });
    SaveManager::Instance->LoadData("worldMapAreaData", gSaveContext.worldMapAreaData);
    SaveManager::Instance->LoadData("scarecrowLongSongSet", gSaveContext.scarecrowLongSongSet);
    SaveManager::Instance->LoadArray("scarecrowLongSong", ARRAY_COUNT(gSaveContext.scarecrowLongSong), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowLongSong[i].noteIdx);
            SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowLongSong[i].unk_01);
            SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowLongSong[i].unk_02);
            SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowLongSong[i].volume);
            SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowLongSong[i].vibrato);
            SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowLongSong[i].tone);
            SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowLongSong[i].semitone);
        });
    });
    SaveManager::Instance->LoadData("scarecrowSpawnSongSet", gSaveContext.scarecrowSpawnSongSet);
    SaveManager::Instance->LoadArray("scarecrowSpawnSong", ARRAY_COUNT(gSaveContext.scarecrowSpawnSong), [](size_t i) {
        SaveManager::Instance->LoadStruct("", [&i]() {
            SaveManager::Instance->LoadData("noteIdx", gSaveContext.scarecrowSpawnSong[i].noteIdx);
            SaveManager::Instance->LoadData("unk_01", gSaveContext.scarecrowSpawnSong[i].unk_01);
            SaveManager::Instance->LoadData("unk_02", gSaveContext.scarecrowSpawnSong[i].unk_02);
            SaveManager::Instance->LoadData("volume", gSaveContext.scarecrowSpawnSong[i].volume);
            SaveManager::Instance->LoadData("vibrato", gSaveContext.scarecrowSpawnSong[i].vibrato);
            SaveManager::Instance->LoadData("tone", gSaveContext.scarecrowSpawnSong[i].tone);
            SaveManager::Instance->LoadData("semitone", gSaveContext.scarecrowSpawnSong[i].semitone);
        });
    });
    SaveManager::Instance->LoadStruct("horseData", []() {
        SaveManager::Instance->LoadData("scene", gSaveContext.horseData.scene);
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.horseData.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.horseData.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.horseData.pos.z);
        });
        SaveManager::Instance->LoadData("angle", gSaveContext.horseData.angle);
    });

    SaveManager::Instance->LoadArray("randomizerInf", ARRAY_COUNT(gSaveContext.ship.randomizerInf), [](size_t i) {
        SaveManager::Instance->LoadData("", gSaveContext.ship.randomizerInf[i]);
    });
    int isMQ = 0;
    SaveManager::Instance->LoadData("isMasterQuest", isMQ);
    if (isMQ) {
        gSaveContext.ship.quest.id = QUEST_MASTER;
    }
    SaveManager::Instance->LoadStruct("backupFW", []() {
        SaveManager::Instance->LoadStruct("pos", []() {
            SaveManager::Instance->LoadData("x", gSaveContext.ship.backupFW.pos.x);
            SaveManager::Instance->LoadData("y", gSaveContext.ship.backupFW.pos.y);
            SaveManager::Instance->LoadData("z", gSaveContext.ship.backupFW.pos.z);
        });
        SaveManager::Instance->LoadData("yaw", gSaveContext.ship.backupFW.yaw);
        SaveManager::Instance->LoadData("playerParams", gSaveContext.ship.backupFW.playerParams);
        SaveManager::Instance->LoadData("entranceIndex", gSaveContext.ship.backupFW.entranceIndex);
        SaveManager::Instance->LoadData("roomIndex", gSaveContext.ship.backupFW.roomIndex);
        SaveManager::Instance->LoadData("set", gSaveContext.ship.backupFW.set);
        SaveManager::Instance->LoadData("tempSwchFlags", gSaveContext.ship.backupFW.tempSwchFlags);
        SaveManager::Instance->LoadData("tempCollectFlags", gSaveContext.ship.backupFW.tempCollectFlags);
    });
    SaveManager::Instance->LoadData("dogParams", gSaveContext.dogParams);
    SaveManager::Instance->LoadData("filenameLanguage", gSaveContext.ship.filenameLanguage);
    SaveManager::Instance->LoadData("maskMemory", gSaveContext.ship.maskMemory);
}

void SaveManager::SaveBase(SaveContext* saveContext, int sectionID, bool fullSave) {
    SaveManager::Instance->SaveData("entranceIndex", saveContext->entranceIndex);
    SaveManager::Instance->SaveData("linkAge", saveContext->linkAge);
    SaveManager::Instance->SaveData("cutsceneIndex", saveContext->cutsceneIndex);
    SaveManager::Instance->SaveData("dayTime", saveContext->dayTime);
    SaveManager::Instance->SaveData("nightFlag", saveContext->nightFlag);
    SaveManager::Instance->SaveData("totalDays", saveContext->totalDays);
    SaveManager::Instance->SaveData("bgsDayCount", saveContext->bgsDayCount);
    SaveManager::Instance->SaveData("deaths", saveContext->deaths);
    SaveManager::Instance->SaveArray("playerName", ARRAY_COUNT(saveContext->playerName), [&](size_t i) {
        SaveManager::Instance->SaveData("", saveContext->playerName[i]);
    });
    SaveManager::Instance->SaveData("n64ddFlag", saveContext->ship.quest.id == QUEST_RANDOMIZER);
    SaveManager::Instance->SaveData("healthCapacity", saveContext->healthCapacity);
    SaveManager::Instance->SaveData("health", saveContext->health);
    SaveManager::Instance->SaveData("magicLevel", saveContext->magicLevel);
    SaveManager::Instance->SaveData("magic", saveContext->magic);
    SaveManager::Instance->SaveData("rupees", saveContext->rupees);
    SaveManager::Instance->SaveData("swordHealth", saveContext->swordHealth);
    SaveManager::Instance->SaveData("naviTimer", saveContext->naviTimer);
    SaveManager::Instance->SaveData("isMagicAcquired", saveContext->isMagicAcquired);
    SaveManager::Instance->SaveData("isDoubleMagicAcquired", saveContext->isDoubleMagicAcquired);
    SaveManager::Instance->SaveData("isDoubleDefenseAcquired", saveContext->isDoubleDefenseAcquired);
    SaveManager::Instance->SaveData("bgsFlag", saveContext->bgsFlag);
    SaveManager::Instance->SaveData("ocarinaGameRoundNum", saveContext->ocarinaGameRoundNum);
    SaveManager::Instance->SaveStruct("childEquips", [&]() {
        SaveManager::Instance->SaveArray(
            "buttonItems", ARRAY_COUNT(saveContext->childEquips.buttonItems),
            [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->childEquips.buttonItems[i]); });
        SaveManager::Instance->SaveArray(
            "cButtonSlots", ARRAY_COUNT(saveContext->childEquips.cButtonSlots),
            [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->childEquips.cButtonSlots[i]); });
        SaveManager::Instance->SaveData("equipment", saveContext->childEquips.equipment);
    });
    SaveManager::Instance->SaveStruct("adultEquips", [&]() {
        SaveManager::Instance->SaveArray(
            "buttonItems", ARRAY_COUNT(saveContext->adultEquips.buttonItems),
            [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->adultEquips.buttonItems[i]); });
        SaveManager::Instance->SaveArray(
            "cButtonSlots", ARRAY_COUNT(saveContext->adultEquips.cButtonSlots),
            [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->adultEquips.cButtonSlots[i]); });
        SaveManager::Instance->SaveData("equipment", saveContext->adultEquips.equipment);
    });
    SaveManager::Instance->SaveData("unk_54", saveContext->unk_54);
    SaveManager::Instance->SaveData("savedSceneNum", saveContext->savedSceneNum);
    SaveManager::Instance->SaveStruct("equips", [&]() {
        SaveManager::Instance->SaveArray("buttonItems", ARRAY_COUNT(saveContext->equips.buttonItems), [&](size_t i) {
            SaveManager::Instance->SaveData("", saveContext->equips.buttonItems[i]);
        });
        SaveManager::Instance->SaveArray("cButtonSlots", ARRAY_COUNT(saveContext->equips.cButtonSlots), [&](size_t i) {
            SaveManager::Instance->SaveData("", saveContext->equips.cButtonSlots[i]);
        });
        SaveManager::Instance->SaveData("equipment", saveContext->equips.equipment);
    });
    SaveManager::Instance->SaveStruct("inventory", [&]() {
        SaveManager::Instance->SaveArray("items", ARRAY_COUNT(saveContext->inventory.items), [&](size_t i) {
            SaveManager::Instance->SaveData("", saveContext->inventory.items[i]);
        });
        SaveManager::Instance->SaveArray("ammo", ARRAY_COUNT(saveContext->inventory.ammo), [&](size_t i) {
            SaveManager::Instance->SaveData("", saveContext->inventory.ammo[i]);
        });
        SaveManager::Instance->SaveData("equipment", saveContext->inventory.equipment);
        SaveManager::Instance->SaveData("upgrades", saveContext->inventory.upgrades);
        SaveManager::Instance->SaveData("questItems", saveContext->inventory.questItems);
        SaveManager::Instance->SaveArray(
            "dungeonItems", ARRAY_COUNT(saveContext->inventory.dungeonItems),
            [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->inventory.dungeonItems[i]); });
        SaveManager::Instance->SaveArray("dungeonKeys", ARRAY_COUNT(saveContext->inventory.dungeonKeys), [&](size_t i) {
            SaveManager::Instance->SaveData("", saveContext->inventory.dungeonKeys[i]);
        });
        SaveManager::Instance->SaveData("defenseHearts", saveContext->inventory.defenseHearts);
        SaveManager::Instance->SaveData("gsTokens", saveContext->inventory.gsTokens);
    });
    SaveManager::Instance->SaveArray("sceneFlags", ARRAY_COUNT(saveContext->sceneFlags), [&](size_t i) {
        SaveManager::Instance->SaveStruct("", [&]() {
            SaveManager::Instance->SaveData("chest", saveContext->sceneFlags[i].chest);
            SaveManager::Instance->SaveData("swch", saveContext->sceneFlags[i].swch);
            SaveManager::Instance->SaveData("clear", saveContext->sceneFlags[i].clear);
            SaveManager::Instance->SaveData("collect", saveContext->sceneFlags[i].collect);
            SaveManager::Instance->SaveData("unk", saveContext->sceneFlags[i].unk);
            SaveManager::Instance->SaveData("rooms", saveContext->sceneFlags[i].rooms);
            SaveManager::Instance->SaveData("floors", saveContext->sceneFlags[i].floors);
        });
    });
    SaveManager::Instance->SaveStruct("fw", [&]() {
        SaveManager::Instance->SaveStruct("pos", [&]() {
            SaveManager::Instance->SaveData("x", saveContext->fw.pos.x);
            SaveManager::Instance->SaveData("y", saveContext->fw.pos.y);
            SaveManager::Instance->SaveData("z", saveContext->fw.pos.z);
        });
        SaveManager::Instance->SaveData("yaw", saveContext->fw.yaw);
        SaveManager::Instance->SaveData("playerParams", saveContext->fw.playerParams);
        SaveManager::Instance->SaveData("entranceIndex", saveContext->fw.entranceIndex);
        SaveManager::Instance->SaveData("roomIndex", saveContext->fw.roomIndex);
        SaveManager::Instance->SaveData("set", saveContext->fw.set);
        SaveManager::Instance->SaveData("tempSwchFlags", saveContext->fw.tempSwchFlags);
        SaveManager::Instance->SaveData("tempCollectFlags", saveContext->fw.tempCollectFlags);
    });
    SaveManager::Instance->SaveArray("gsFlags", ARRAY_COUNT(saveContext->gsFlags),
                                     [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->gsFlags[i]); });
    SaveManager::Instance->SaveArray("highScores", ARRAY_COUNT(saveContext->highScores), [&](size_t i) {
        SaveManager::Instance->SaveData("", saveContext->highScores[i]);
    });
    SaveManager::Instance->SaveArray("eventChkInf", ARRAY_COUNT(saveContext->eventChkInf), [&](size_t i) {
        SaveManager::Instance->SaveData("", saveContext->eventChkInf[i]);
    });
    SaveManager::Instance->SaveArray("itemGetInf", ARRAY_COUNT(saveContext->itemGetInf), [&](size_t i) {
        SaveManager::Instance->SaveData("", saveContext->itemGetInf[i]);
    });
    SaveManager::Instance->SaveArray("infTable", ARRAY_COUNT(saveContext->infTable),
                                     [&](size_t i) { SaveManager::Instance->SaveData("", saveContext->infTable[i]); });
    SaveManager::Instance->SaveData("worldMapAreaData", saveContext->worldMapAreaData);
    SaveManager::Instance->SaveData("scarecrowLongSongSet", saveContext->scarecrowLongSongSet);
    SaveManager::Instance->SaveArray("scarecrowLongSong", ARRAY_COUNT(saveContext->scarecrowLongSong), [&](size_t i) {
        SaveManager::Instance->SaveStruct("", [&]() {
            SaveManager::Instance->SaveData("noteIdx", saveContext->scarecrowLongSong[i].noteIdx);
            SaveManager::Instance->SaveData("unk_01", saveContext->scarecrowLongSong[i].unk_01);
            SaveManager::Instance->SaveData("unk_02", saveContext->scarecrowLongSong[i].unk_02);
            SaveManager::Instance->SaveData("volume", saveContext->scarecrowLongSong[i].volume);
            SaveManager::Instance->SaveData("vibrato", saveContext->scarecrowLongSong[i].vibrato);
            SaveManager::Instance->SaveData("tone", saveContext->scarecrowLongSong[i].tone);
            SaveManager::Instance->SaveData("semitone", saveContext->scarecrowLongSong[i].semitone);
        });
    });
    SaveManager::Instance->SaveData("scarecrowSpawnSongSet", saveContext->scarecrowSpawnSongSet);
    SaveManager::Instance->SaveArray("scarecrowSpawnSong", ARRAY_COUNT(saveContext->scarecrowSpawnSong), [&](size_t i) {
        SaveManager::Instance->SaveStruct("", [&]() {
            SaveManager::Instance->SaveData("noteIdx", saveContext->scarecrowSpawnSong[i].noteIdx);
            SaveManager::Instance->SaveData("unk_01", saveContext->scarecrowSpawnSong[i].unk_01);
            SaveManager::Instance->SaveData("unk_02", saveContext->scarecrowSpawnSong[i].unk_02);
            SaveManager::Instance->SaveData("volume", saveContext->scarecrowSpawnSong[i].volume);
            SaveManager::Instance->SaveData("vibrato", saveContext->scarecrowSpawnSong[i].vibrato);
            SaveManager::Instance->SaveData("tone", saveContext->scarecrowSpawnSong[i].tone);
            SaveManager::Instance->SaveData("semitone", saveContext->scarecrowSpawnSong[i].semitone);
        });
    });
    SaveManager::Instance->SaveStruct("horseData", [&]() {
        SaveManager::Instance->SaveData("scene", saveContext->horseData.scene);
        SaveManager::Instance->SaveStruct("pos", [&]() {
            SaveManager::Instance->SaveData("x", saveContext->horseData.pos.x);
            SaveManager::Instance->SaveData("y", saveContext->horseData.pos.y);
            SaveManager::Instance->SaveData("z", saveContext->horseData.pos.z);
        });
        SaveManager::Instance->SaveData("angle", saveContext->horseData.angle);
    });

    SaveManager::Instance->SaveArray("randomizerInf", ARRAY_COUNT(saveContext->ship.randomizerInf), [&](size_t i) {
        SaveManager::Instance->SaveData("", saveContext->ship.randomizerInf[i]);
    });
    SaveManager::Instance->SaveData("isMasterQuest", saveContext->ship.quest.id == QUEST_MASTER);
    SaveManager::Instance->SaveStruct("backupFW", [&]() {
        SaveManager::Instance->SaveStruct("pos", [&]() {
            SaveManager::Instance->SaveData("x", saveContext->ship.backupFW.pos.x);
            SaveManager::Instance->SaveData("y", saveContext->ship.backupFW.pos.y);
            SaveManager::Instance->SaveData("z", saveContext->ship.backupFW.pos.z);
        });
        SaveManager::Instance->SaveData("yaw", saveContext->ship.backupFW.yaw);
        SaveManager::Instance->SaveData("playerParams", saveContext->ship.backupFW.playerParams);
        SaveManager::Instance->SaveData("entranceIndex", saveContext->ship.backupFW.entranceIndex);
        SaveManager::Instance->SaveData("roomIndex", saveContext->ship.backupFW.roomIndex);
        SaveManager::Instance->SaveData("set", saveContext->ship.backupFW.set);
        SaveManager::Instance->SaveData("tempSwchFlags", saveContext->ship.backupFW.tempSwchFlags);
        SaveManager::Instance->SaveData("tempCollectFlags", saveContext->ship.backupFW.tempCollectFlags);
    });
    SaveManager::Instance->SaveData("dogParams", saveContext->dogParams);
    SaveManager::Instance->SaveData("filenameLanguage", saveContext->ship.filenameLanguage);
    SaveManager::Instance->SaveData("maskMemory", saveContext->ship.maskMemory);
}

// Load a string into a char array based on size and ensuring it is null terminated when overflowed
void SaveManager::LoadCharArray(const std::string& name, char* destination, size_t size) {
    std::string temp;
    SaveManager::Instance->LoadData(name, temp);
    SohUtils::CopyStringToCharArray(destination, temp, size);
}

void SaveManager::SaveArray(const std::string& name, const size_t size, SaveArrayFunc func) {
    // Create an empty array and set it as the current save context, then call the function that saves an array entry.
    nlohmann::json* saveJsonContext = currentJsonContext;
    currentJsonContext = &(*currentJsonContext)[name.c_str()];
    *currentJsonContext = nlohmann::json::array();
    for (size_t i = 0; i < size; i++) {
        func(i);
    }
    currentJsonContext = saveJsonContext;
}

void SaveManager::SaveStruct(const std::string& name, SaveStructFunc func) {
    // Create an empty struct and set it as the current save context, then call the function that saves the struct.
    // If it is an array entry, save it to the array instead.
    if (name == "") {
        nlohmann::json* saveJsonContext = currentJsonContext;
        nlohmann::json object = nlohmann::json::object();
        currentJsonContext = &object;
        func();
        currentJsonContext = saveJsonContext;
        (*currentJsonContext).push_back(object);
    } else {
        nlohmann::json* saveJsonContext = currentJsonContext;
        currentJsonContext = &(*currentJsonContext)[name.c_str()];
        *currentJsonContext = nlohmann::json::object();
        func();
        currentJsonContext = saveJsonContext;
    }
}

void SaveManager::LoadArray(const std::string& name, const size_t size, LoadArrayFunc func) {
    // Create an empty array and set it as the current save context, then call the function that loads an array entry.
    nlohmann::json* saveJsonContext = currentJsonContext;
    currentJsonContext = &(*currentJsonContext)[name.c_str()];
    currentJsonArrayContext = currentJsonContext->begin();
    size_t i = 0;
    for (; (currentJsonArrayContext != currentJsonContext->end()) && (i < size); i++, currentJsonArrayContext++) {
        func(i);
    }
    // Handle remainer of items. Either this was data that was manually deleted, or a later version extended the size of
    // the array. The later members will be default constructed.
    for (; i < size; i++) {
        func(i);
    }
    currentJsonContext = saveJsonContext;
}

void SaveManager::LoadStruct(const std::string& name, LoadStructFunc func) {
    // Create an empty struct and set it as the current load context, then call the function that loads the struct.
    // If it is an array entry, load it from the array instead.
    if (name == "") {
        nlohmann::json* saveJsonContext = currentJsonContext;
        nlohmann::json emptyObject = nlohmann::json::object();
        if (currentJsonArrayContext != currentJsonContext->end()) {
            currentJsonContext = &currentJsonArrayContext.value();
        } else {
            // This array member is past the data in the json file. Therefore, default construct it.
            // By assigning an empty object here, all attempts to load data members of it will default construct them.
            currentJsonContext = &emptyObject;
        }
        func();
        currentJsonContext = saveJsonContext;
    } else {
        nlohmann::json* saveJsonContext = currentJsonContext;
        currentJsonContext = &(*currentJsonContext)[name.c_str()];
        func();
        currentJsonContext = saveJsonContext;
    }
}

void SaveManager::CopyZeldaFile(int from, int to) {
    assert(std::filesystem::exists(GetFileName(from)));
    DeleteZeldaFile(to);
#if defined(__WIIU__) || defined(__SWITCH__)
    copy_file(GetFileName(from).c_str(), GetFileName(to).c_str());
#else
    std::filesystem::copy_file(GetFileName(from), GetFileName(to));
#endif
    fileMetaInfo[to].valid = true;
    fileMetaInfo[to].deaths = fileMetaInfo[from].deaths;
    for (int i = 0; i < ARRAY_COUNT(fileMetaInfo[to].playerName); i++) {
        fileMetaInfo[to].playerName[i] = fileMetaInfo[from].playerName[i];
    }
    for (int i = 0; i < ARRAY_COUNT(fileMetaInfo[to].seedHash); i++) {
        fileMetaInfo[to].seedHash[i] = fileMetaInfo[from].seedHash[i];
    }
    fileMetaInfo[to].healthCapacity = fileMetaInfo[from].healthCapacity;
    fileMetaInfo[to].questItems = fileMetaInfo[from].questItems;
    fileMetaInfo[to].defense = fileMetaInfo[from].defense;
    fileMetaInfo[to].health = fileMetaInfo[from].health;
    fileMetaInfo[to].randoSave = fileMetaInfo[from].randoSave;
    fileMetaInfo[to].requiresMasterQuest = fileMetaInfo[from].requiresMasterQuest;
    fileMetaInfo[to].requiresOriginal = fileMetaInfo[from].requiresOriginal;
    fileMetaInfo[to].buildVersionMajor = fileMetaInfo[from].buildVersionMajor;
    fileMetaInfo[to].buildVersionMinor = fileMetaInfo[from].buildVersionMinor;
    fileMetaInfo[to].buildVersionPatch = fileMetaInfo[from].buildVersionPatch;
    fileMetaInfo[to].filenameLanguage = fileMetaInfo[from].filenameLanguage;
    SohUtils::CopyStringToCharArray(fileMetaInfo[to].buildVersion, fileMetaInfo[from].buildVersion,
                                    ARRAY_COUNT(fileMetaInfo[to].buildVersion));
}

void SaveManager::DeleteZeldaFile(int fileNum) {
    if (std::filesystem::exists(GetFileName(fileNum))) {
        std::filesystem::remove(GetFileName(fileNum));
    }
    fileMetaInfo[fileNum].valid = false;
    fileMetaInfo[fileNum].randoSave = false;
    fileMetaInfo[fileNum].requiresMasterQuest = false;
    fileMetaInfo[fileNum].requiresOriginal = false;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnDeleteFile>(fileNum);
}

bool SaveManager::IsRandoFile() {
    return IS_RANDO;
}

// Functionality required to convert old saves into versioned saves

// DO NOT EDIT ANY OF THE FOLLOWING STRUCTS
// They MUST remain unchanged to handle parsing the binary saves of old

typedef struct {
    /* 0x00 */ u8 buttonItems[4];
    /* 0x04 */ u8 cButtonSlots[3];
    /* 0x08 */ u16 equipment;
} ItemEquips_v0; // size = 0x0A

typedef struct {
    /* 0x00 */ u8 items[24];
    /* 0x18 */ s8 ammo[16];
    /* 0x28 */ u16 equipment;
    /* 0x2C */ u32 upgrades;
    /* 0x30 */ u32 questItems;
    /* 0x34 */ u8 dungeonItems[20];
    /* 0x48 */ s8 dungeonKeys[19];
    /* 0x5B */ s8 defenseHearts;
    /* 0x5C */ s16 gsTokens;
} Inventory_v0; // size = 0x5E

typedef struct {
    /* 0x00 */ u32 chest;
    /* 0x04 */ u32 swch;
    /* 0x08 */ u32 clear;
    /* 0x0C */ u32 collect;
    /* 0x10 */ u32 unk;
    /* 0x14 */ u32 rooms;
    /* 0x18 */ u32 floors;
} SavedSceneFlags_v0; // size = 0x1C

typedef struct {
    s32 x, y, z;
} Vec3i_v0; // size = 0x0C

typedef struct {
    /* 0x00 */ Vec3i_v0 pos;
    /* 0x0C */ s32 yaw;
    /* 0x10 */ s32 playerParams;
    /* 0x14 */ s32 entranceIndex;
    /* 0x18 */ s32 roomIndex;
    /* 0x1C */ s32 set;
    /* 0x20 */ s32 tempSwchFlags;
    /* 0x24 */ s32 tempCollectFlags;
} FaroresWindData_v0; // size = 0x28

typedef struct {
    s16 x, y, z;
} Vec3s_v0; // size = 0x06

typedef struct {
    /* 0x00 */ s16 scene;
    /* 0x02 */ Vec3s_v0 pos;
    /* 0x08 */ s16 angle;
} HorseData_v0; // size = 0x0A

typedef struct {
    f32 x, y, z;
} Vec3f_v0; // size = 0x0C

typedef struct {
    /* 0x00 */ Vec3f_v0 pos;
    /* 0x0C */ s16 yaw;
    /* 0x0E */ s16 playerParams;
    /* 0x10 */ s16 entranceIndex;
    /* 0x12 */ u8 roomIndex;
    /* 0x13 */ s8 data;
    /* 0x14 */ u32 tempSwchFlags;
    /* 0x18 */ u32 tempCollectFlags;
} RespawnData_v0; // size = 0x1C

typedef struct {
    /* 0x0000 */ s32 entranceIndex; // start of `save` substruct, originally called "memory"
    /* 0x0004 */ s32 linkAge;       // 0: Adult; 1: Child
    /* 0x0008 */ s32 cutsceneIndex;
    /* 0x000C */ u16 dayTime; // "zelda_time"
    /* 0x0010 */ s32 nightFlag;
    /* 0x0014 */ s32 totalDays;
    /* 0x0018 */ s32 bgsDayCount; // increments with totalDays, can be cleared with `Environment_ClearBgsDayCount`
    /* 0x001C */ char newf[6];    // string "ZELDAZ". start of `info` substruct, originally called "information"
    /* 0x0022 */ u16 deaths;
    /* 0x0024 */ char playerName[8];
    /* 0x002C */ s16 n64ddFlag;
    /* 0x002E */ s16 healthCapacity; // "max_life"
    /* 0x0030 */ s16 health;         // "now_life"
    /* 0x0032 */ s8 magicLevel;
    /* 0x0033 */ s8 magic;
    /* 0x0034 */ s16 rupees;
    /* 0x0036 */ u16 swordHealth;
    /* 0x0038 */ u16 naviTimer;
    /* 0x003A */ u8 magicAcquired;
    /* 0x003B */ char unk_3B[0x01];
    /* 0x003C */ u8 doubleMagic;
    /* 0x003D */ u8 doubleDefense;
    /* 0x003E */ u8 bgsFlag;
    /* 0x003F */ u8 ocarinaGameRoundNum;
    /* 0x0040 */ ItemEquips_v0 childEquips;
    /* 0x004A */ ItemEquips_v0 adultEquips;
    /* 0x0054 */ u32 unk_54; // this may be incorrect, currently used for alignement
    /* 0x0058 */ char unk_58[0x0E];
    /* 0x0066 */ s16 savedSceneNum;
    /* 0x0068 */ ItemEquips_v0 equips;
    /* 0x0074 */ Inventory_v0 inventory;
    /* 0x00D4 */ SavedSceneFlags_v0 sceneFlags[124];
    /* 0x0E64 */ FaroresWindData_v0 fw;
    /* 0x0E8C */ char unk_E8C[0x10];
    /* 0x0E9C */ s32 gsFlags[6];
    /* 0x0EB4 */ char unk_EB4[0x4];
    /* 0x0EB8 */ s32 highScores[7];
    /* 0x0ED4 */ u16 eventChkInf[14]; // "event_chk_inf"
    /* 0x0EF0 */ u16 itemGetInf[4];   // "item_get_inf"
    /* 0x0EF8 */ u16 infTable[30];    // "inf_table"
    /* 0x0F34 */ char unk_F34[0x04];
    /* 0x0F38 */ u32 worldMapAreaData; // "area_arrival"
    /* 0x0F3C */ char unk_F3C[0x4];
    /* 0x0F40 */ u8 scarecrowCustomSongSet;
    /* 0x0F41 */ u8 scarecrowCustomSong[0x360];
    /* 0x12A1 */ char unk_12A1[0x24];
    /* 0x12C5 */ u8 scarecrowSpawnSongSet;
    /* 0x12C6 */ u8 scarecrowSpawnSong[0x80];
    /* 0x1346 */ char unk_1346[0x02];
    /* 0x1348 */ HorseData_v0 horseData;
    /* 0x1352 */ u16 checksum; // "check_sum"
    /* 0x1354 */ s32 fileNum;  // "file_no"
    /* 0x1358 */ char unk_1358[0x0004];
    /* 0x135C */ s32 gameMode;
    /* 0x1360 */ s32 sceneSetupIndex;
    /* 0x1364 */ s32 respawnFlag;           // "restart_flag"
    /* 0x1368 */ RespawnData_v0 respawn[3]; // "restart_data"
    /* 0x13BC */ f32 entranceSpeed;
    /* 0x13C0 */ u16 entranceSound;
    /* 0x13C2 */ char unk_13C2[0x0001];
    /* 0x13C3 */ u8 unk_13C3;
    /* 0x13C4 */ s16 dogParams;
    /* 0x13C6 */ u8 textTriggerFlags;
    /* 0x13C7 */ u8 showTitleCard;
    /* 0x13C8 */ s16 nayrusLoveTimer;
    /* 0x13CA */ char unk_13CA[0x0002];
    /* 0x13CC */ s16 rupeeAccumulator;
    /* 0x13CE */ s16 timerState;
    /* 0x13D0 */ s16 timerSeconds;
    /* 0x13D2 */ s16 subTimerState;
    /* 0x13D4 */ s16 subTimerSeconds;
    /* 0x13D6 */ s16 timerX[2];
    /* 0x13DA */ s16 timerY[2];
    /* 0x13DE */ char unk_13DE[0x0002];
    /* 0x13E0 */ u8 seqId;
    /* 0x13E1 */ u8 natureAmbienceId;
    /* 0x13E2 */ u8 buttonStatus[5];
    /* 0x13E7 */ u8 forceRisingButtonAlphas; // alpha related
    /* 0x13E8 */ u16 unk_13E8;               // alpha type?
    /* 0x13EA */ u16 unk_13EA;               // also alpha type?
    /* 0x13EC */ u16 unk_13EC;               // alpha type counter?
    /* 0x13EE */ u16 unk_13EE;               // previous alpha type?
    /* 0x13F0 */ s16 unk_13F0;               // magic related
    /* 0x13F2 */ s16 unk_13F2;               // magic related
    /* 0x13F4 */ s16 unk_13F4;               // magic related
    /* 0x13F6 */ s16 unk_13F6;               // magic related
    /* 0x13F8 */ s16 unk_13F8;               // magic related
    /* 0x13FA */ u16 eventInf[4];            // "event_inf"
    /* 0x1402 */ u16 mapIndex;               // intended for maps/minimaps but commonly used as the dungeon index
    /* 0x1404 */ u16 minigameState;
    /* 0x1406 */ u16 minigameScore; // "yabusame_total"
    /* 0x1408 */ char unk_1408[0x0001];
    /* 0x1409 */ u8 language; // NTSC 0: Japanese; 1: English | PAL 0: English; 1: German; 2: French
    /* 0x140A */ u8 audioSetting;
    /* 0x140B */ char unk_140B[0x0001];
    /* 0x140C */ u8 zTargetSetting; // 0: Switch; 1: Hold
    /* 0x140E */ u16 forcedSeqId;   // immediately start playing the sequence if set
    /* 0x1410 */ u8 unk_1410;       // transition related
    /* 0x1411 */ char unk_1411[0x0001];
    /* 0x1412 */ u16 nextCutsceneIndex;
    /* 0x1414 */ u8 cutsceneTrigger;
    /* 0x1415 */ u8 chamberCutsceneNum;
    /* 0x1416 */ u16 nextDayTime; // "next_zelda_time"
    /* 0x1418 */ u8 fadeDuration;
    /* 0x1419 */ u8 unk_1419; // transition related
    /* 0x141A */ u16 skyboxTime;
    /* 0x141C */ u8 dogIsLost;
    /* 0x141D */ u8 nextTransition;
    /* 0x141E */ char unk_141E[0x0002];
    /* 0x1420 */ s16 worldMapArea;
    /* 0x1422 */ s16 sunsSongState; // controls the effects of suns song
    /* 0x1424 */ s16 healthAccumulator;
} SaveContext_v0; // size = 0x1428

void CopyV0Save(SaveContext_v0& src, SaveContext& dst) {
    dst.entranceIndex = src.entranceIndex;
    dst.linkAge = src.linkAge;
    dst.cutsceneIndex = src.cutsceneIndex;
    dst.dayTime = src.dayTime;
    dst.nightFlag = src.nightFlag;
    dst.totalDays = src.totalDays;
    dst.bgsDayCount = src.bgsDayCount;
    dst.deaths = src.deaths;
    for (size_t i = 0; i < ARRAY_COUNT(src.playerName); i++) {
        dst.playerName[i] = src.playerName[i];
    }
    dst.n64ddFlag = src.n64ddFlag;
    dst.healthCapacity = src.healthCapacity;
    dst.health = src.health;
    dst.magicLevel = src.magicLevel;
    dst.magic = src.magic;
    dst.rupees = src.rupees;
    dst.swordHealth = src.swordHealth;
    dst.naviTimer = src.naviTimer;
    dst.isMagicAcquired = src.magicAcquired;
    dst.isDoubleMagicAcquired = src.doubleMagic;
    dst.isDoubleDefenseAcquired = src.doubleDefense;
    dst.bgsFlag = src.bgsFlag;
    dst.ocarinaGameRoundNum = src.ocarinaGameRoundNum;
    for (size_t i = 0; i < ARRAY_COUNT(src.childEquips.buttonItems); i++) {
        dst.childEquips.buttonItems[i] = src.childEquips.buttonItems[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.childEquips.cButtonSlots); i++) {
        dst.childEquips.cButtonSlots[i] = src.childEquips.cButtonSlots[i];
    }
    dst.childEquips.equipment = src.childEquips.equipment;
    for (size_t i = 0; i < ARRAY_COUNT(src.adultEquips.buttonItems); i++) {
        dst.adultEquips.buttonItems[i] = src.adultEquips.buttonItems[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.adultEquips.cButtonSlots); i++) {
        dst.adultEquips.cButtonSlots[i] = src.adultEquips.cButtonSlots[i];
    }
    dst.adultEquips.equipment = src.adultEquips.equipment;
    dst.unk_54 = src.unk_54;
    dst.savedSceneNum = src.savedSceneNum;
    for (size_t i = 0; i < ARRAY_COUNT(src.equips.buttonItems); i++) {
        dst.equips.buttonItems[i] = src.equips.buttonItems[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.equips.cButtonSlots); i++) {
        dst.equips.cButtonSlots[i] = src.equips.cButtonSlots[i];
    }
    dst.equips.equipment = src.equips.equipment;
    for (size_t i = 0; i < ARRAY_COUNT(src.inventory.items); i++) {
        dst.inventory.items[i] = src.inventory.items[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.inventory.ammo); i++) {
        dst.inventory.ammo[i] = src.inventory.ammo[i];
    }
    dst.inventory.equipment = src.inventory.equipment;
    dst.inventory.upgrades = src.inventory.upgrades;
    dst.inventory.questItems = src.inventory.questItems;
    for (size_t i = 0; i < ARRAY_COUNT(src.inventory.dungeonItems); i++) {
        dst.inventory.dungeonItems[i] = src.inventory.dungeonItems[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.inventory.dungeonKeys); i++) {
        dst.inventory.dungeonKeys[i] = src.inventory.dungeonKeys[i];
    }
    dst.inventory.defenseHearts = src.inventory.defenseHearts;
    dst.inventory.gsTokens = src.inventory.gsTokens;
    for (size_t i = 0; i < ARRAY_COUNT(src.sceneFlags); i++) {
        dst.sceneFlags[i].chest = src.sceneFlags[i].chest;
        dst.sceneFlags[i].swch = src.sceneFlags[i].swch;
        dst.sceneFlags[i].clear = src.sceneFlags[i].clear;
        dst.sceneFlags[i].collect = src.sceneFlags[i].collect;
        dst.sceneFlags[i].unk = src.sceneFlags[i].unk;
        dst.sceneFlags[i].rooms = src.sceneFlags[i].rooms;
        dst.sceneFlags[i].floors = src.sceneFlags[i].floors;
    }
    dst.fw.pos.x = src.fw.pos.x;
    dst.fw.pos.y = src.fw.pos.y;
    dst.fw.pos.z = src.fw.pos.z;
    dst.fw.yaw = src.fw.yaw;
    dst.fw.playerParams = src.fw.playerParams;
    dst.fw.entranceIndex = src.fw.entranceIndex;
    dst.fw.roomIndex = src.fw.roomIndex;
    dst.fw.set = src.fw.set;
    dst.fw.tempSwchFlags = src.fw.tempSwchFlags;
    dst.fw.tempCollectFlags = src.fw.tempCollectFlags;
    for (size_t i = 0; i < ARRAY_COUNT(src.gsFlags); i++) {
        dst.gsFlags[i] = src.gsFlags[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.highScores); i++) {
        dst.highScores[i] = src.highScores[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.eventChkInf); i++) {
        dst.eventChkInf[i] = src.eventChkInf[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.itemGetInf); i++) {
        dst.itemGetInf[i] = src.itemGetInf[i];
    }
    for (size_t i = 0; i < ARRAY_COUNT(src.infTable); i++) {
        dst.infTable[i] = src.infTable[i];
    }
    dst.worldMapAreaData = src.worldMapAreaData;
    dst.scarecrowLongSongSet = src.scarecrowCustomSongSet;
    memcpy(&dst.scarecrowLongSong[0], &src.scarecrowCustomSong[0], sizeof(src.scarecrowCustomSong));
    dst.scarecrowSpawnSongSet = src.scarecrowSpawnSongSet;
    memcpy(&dst.scarecrowSpawnSong[0], &src.scarecrowSpawnSong[0], sizeof(src.scarecrowSpawnSong));
    dst.horseData.scene = src.horseData.scene;
    dst.horseData.pos.x = src.horseData.pos.x;
    dst.horseData.pos.y = src.horseData.pos.y;
    dst.horseData.pos.z = src.horseData.pos.z;
    dst.horseData.angle = src.horseData.angle;
}

void SaveManager::ConvertFromUnversioned() {
    static char sZeldaMagic[] = { '\0', '\0', '\0', '\x98', '\x09', '\x10', '\x21', 'Z', 'E', 'L', 'D', 'A' };
#define SLOT_SIZE (sizeof(SaveContext_v0) + 0x28)
#define SLOT_OFFSET(index) (SRAM_HEADER_SIZE + 0x10 + (index * SLOT_SIZE))

    std::ifstream input("oot_save.sav", std::ios::binary);

    std::vector<char> data(std::istreambuf_iterator<char>(input), {});
    input.close();

    for (size_t i = 0; i < ARRAY_COUNT(sZeldaMagic) - 3; i++) {
        if (sZeldaMagic[i + SRAM_HEADER_MAGIC] != data[i + SRAM_HEADER_MAGIC]) {
            CreateDefaultGlobal();
            return;
        }
    }

    gSaveContext.audioSetting = data[SRAM_HEADER_SOUND] & 3;
    gSaveContext.zTargetSetting = data[SRAM_HEADER_ZTARGET] & 1;
    gSaveContext.language = data[SRAM_HEADER_LANGUAGE];
    if (gSaveContext.language >= LANGUAGE_MAX) {
        gSaveContext.language = CVarGetInteger(CVAR_SETTING("Languages"), LANGUAGE_ENG);
    }
    SaveGlobal();

    for (int fileNum = 0; fileNum < 3; fileNum++) {
        SaveContext_v0* file = reinterpret_cast<SaveContext_v0*>(&data[SLOT_OFFSET(fileNum)]);
        if ((file->newf[0] == 'Z') && (file->newf[1] == 'E') && (file->newf[2] == 'L') && (file->newf[3] == 'D') &&
            (file->newf[4] == 'A') && (file->newf[5] == 'Z')) {
            // If a save is valid, convert the save by storing the current save context, converting the file, loading
            // it, saving it, then restoring the save context.
            static SaveContext saveContextSave = gSaveContext;
            InitFile(false);
            CopyV0Save(*file, gSaveContext);
            SaveSection(fileNum, SECTION_ID_BASE, false);
            InitMeta(fileNum);
            gSaveContext = saveContextSave;
        }
    }

#undef SLOT_SIZE
#undef SLOT_OFFSET
}

// C to C++ bridge

extern "C" void Save_Init(void) {
    SaveManager::Instance->Init();
}

extern "C" void Save_InitFile(int isDebug) {
    SaveManager::Instance->InitFile(isDebug != 0);
}

extern "C" void Save_SaveFile(void) {
    SaveManager::Instance->SaveFile(gSaveContext.fileNum);
}

extern "C" void Save_SaveSection(int sectionID) {
    SaveManager::Instance->SaveSection(gSaveContext.fileNum, sectionID, true);
}

extern "C" void Save_SaveGlobal(void) {
    SaveManager::Instance->SaveGlobal();
}

extern "C" void Save_LoadFile(void) {
    // Handle vanilla context reset
    OTRGlobals::Instance->gRandoContext->GetLogic()->SetContext(nullptr);
    Rando::Settings::GetInstance()->ClearContext();
    OTRGlobals::Instance->gRandoContext.reset();
    OTRGlobals::Instance->gRandoContext = Rando::Context::CreateInstance();
    OTRGlobals::Instance->gRandoContext->GetLogic()->SetSaveContext(&gSaveContext);
    Rando::Settings::GetInstance()->AssignContext(OTRGlobals::Instance->gRandoContext);
    OTRGlobals::Instance->gRandoContext->AddExcludedOptions();
    SaveManager::Instance->LoadFile(gSaveContext.fileNum);
}

extern "C" void Save_AddLoadFunction(char* name, int version, SaveManager::LoadFunc func) {
    SaveManager::Instance->AddLoadFunction(name, version, func);
}

extern "C" void Save_AddSaveFunction(char* name, int version, SaveManager::SaveFunc func, bool saveWithBase) {
    SaveManager::Instance->AddSaveFunction(name, version, func, saveWithBase);
}

extern "C" SaveFileMetaInfo* Save_GetSaveMetaInfo(int fileNum) {
    return &SaveManager::Instance->fileMetaInfo[fileNum];
}

extern "C" void Save_CopyFile(int from, int to) {
    SaveManager::Instance->CopyZeldaFile(from, to);
}

extern "C" void Save_DeleteFile(int fileNum) {
    SaveManager::Instance->DeleteZeldaFile(fileNum);
}

extern "C" u32 Save_Exist(int fileNum) {
    return SaveManager::Instance->SaveFile_Exist(fileNum);
}
