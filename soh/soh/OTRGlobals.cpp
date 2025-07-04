#include "OTRGlobals.h"
#include "OTRAudio.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <chrono>

#include "ResourceManagerHelpers.h"
#include "graphic/Fast3D/Fast3dWindow.h"
#include <File.h>
#include <DisplayList.h>
#include <Window.h>
#include <soh/GameVersions.h>

#include "Enhancements/gameconsole.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <time.h>
#endif
#include <AudioPlayer.h>
#include "Enhancements/speechsynthesizer/SpeechSynthesizer.h"
#include "Enhancements/controls/SohInputEditorWindow.h"
#include "Enhancements/cosmetics/CosmeticsEditor.h"
#include "Enhancements/audio/AudioCollection.h"
#include "Enhancements/enhancementTypes.h"
#include "Enhancements/debugconsole.h"
#include "Enhancements/randomizer/randomizer.h"
#include "Enhancements/randomizer/randomizer_entrance_tracker.h"
#include "Enhancements/randomizer/randomizer_item_tracker.h"
#include "Enhancements/randomizer/randomizer_check_tracker.h"
#include "Enhancements/randomizer/3drando/random.hpp"
#include "Enhancements/randomizer/static_data.h"
#include "Enhancements/randomizer/dungeon.h"
#include "Enhancements/gameplaystats.h"
#include "frame_interpolation.h"
#include "variables.h"
#include "z64.h"
#include "macros.h"
#include "Fonts.h"
#include "window/FileDropMgr.h"
#include "window/gui/resource/Font.h"
#include <utils/StringHelper.h>
#include "Enhancements/custom-message/CustomMessageManager.h"
#include "Enhancements/Presets/Presets.h"
#include "util.h"

#if not defined(__SWITCH__) && not defined(__WIIU__)
#include "Extractor/Extract.h"
#endif

#include <Fast3D/interpreter.h>

#ifdef __APPLE__
#include <SDL_scancode.h>
#else
#include <SDL2/SDL_scancode.h>
#endif

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#elif defined(__WIIU__)
#include <port/wiiu/WiiUImpl.h>
#include <coreinit/debug.h> // OSFatal
#endif

#include "Enhancements/custom-message/CustomMessageTypes.h"
#include <functions.h>
#include "Enhancements/item-tables/ItemTableManager.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/ImGuiUtils.h"
#include "ActorDB.h"
#include "SaveManager.h"

#ifdef ENABLE_REMOTE_CONTROL
#include "soh/Network/CrowdControl/CrowdControl.h"
#include "soh/Network/Sail/Sail.h"
CrowdControl* CrowdControl::Instance;
Sail* Sail::Instance;
#endif

#include "Enhancements/mods.h"
#include "Enhancements/game-interactor/GameInteractor.h"
#include "Enhancements/randomizer/draw.h"
#include <libultraship/libultraship.h>

// Resource Types/Factories
#include "resource/type/Array.h"
#include "resource/type/Blob.h"
#include "resource/type/DisplayList.h"
#include "resource/type/Matrix.h"
#include "resource/type/Texture.h"
#include "resource/type/Vertex.h"
#include "soh/resource/type/SohResourceType.h"
#include "soh/resource/type/Animation.h"
#include "soh/resource/type/AudioSample.h"
#include "soh/resource/type/AudioSequence.h"
#include "soh/resource/type/AudioSoundFont.h"
#include "soh/resource/type/CollisionHeader.h"
#include "soh/resource/type/Cutscene.h"
#include "soh/resource/type/Path.h"
#include "soh/resource/type/PlayerAnimation.h"
#include "soh/resource/type/Scene.h"
#include "soh/resource/type/Skeleton.h"
#include "soh/resource/type/SkeletonLimb.h"
#include "soh/resource/type/Text.h"
#include "resource/factory/BlobFactory.h"
#include "resource/factory/DisplayListFactory.h"
#include "resource/factory/MatrixFactory.h"
#include "resource/factory/TextureFactory.h"
#include "resource/factory/VertexFactory.h"
#include "soh/resource/importer/ArrayFactory.h"
#include "soh/resource/importer/AnimationFactory.h"
#include "soh/resource/importer/AudioSampleFactory.h"
#include "soh/resource/importer/AudioSequenceFactory.h"
#include "soh/resource/importer/AudioSoundFontFactory.h"
#include "soh/resource/importer/CollisionHeaderFactory.h"
#include "soh/resource/importer/CutsceneFactory.h"
#include "soh/resource/importer/PathFactory.h"
#include "soh/resource/importer/PlayerAnimationFactory.h"
#include "soh/resource/importer/SceneFactory.h"
#include "soh/resource/importer/SkeletonFactory.h"
#include "soh/resource/importer/SkeletonLimbFactory.h"
#include "soh/resource/importer/TextFactory.h"
#include "soh/resource/importer/BackgroundFactory.h"

#include "soh/config/ConfigUpdaters.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "src/overlays/actors/ovl_En_Dns/z_en_dns.h"
}

void SoH_ProcessDroppedFiles(std::string filePath);

OTRGlobals* OTRGlobals::Instance;
SaveManager* SaveManager::Instance;
CustomMessageManager* CustomMessageManager::Instance;
ItemTableManager* ItemTableManager::Instance;
GameInteractor* GameInteractor::Instance;
AudioCollection* AudioCollection::Instance;
SpeechSynthesizer* SpeechSynthesizer::Instance;

extern "C" char** cameraStrings;
std::vector<std::shared_ptr<std::string>> cameraStdStrings;

Color_RGB8 kokiriColor = { 0x1E, 0x69, 0x1B };
Color_RGB8 goronColor = { 0x64, 0x14, 0x00 };
Color_RGB8 zoraColor = { 0x00, 0xEC, 0x64 };

float previousImGuiScale;

bool prevAltAssets = false;

// Same as NaviColor type from OoT src (z_actor.c), but modified to be sans alpha channel for Controller LED.
typedef struct {
    Color_RGB8 inner;
    Color_RGB8 outer;
} NaviColor_RGB8;

static NaviColor_RGB8 defaultIdleColor = { { 255, 255, 255 }, { 0, 0, 255 } };
static NaviColor_RGB8 defaultNPCColor = { { 150, 150, 255 }, { 150, 150, 255 } };
static NaviColor_RGB8 defaultEnemyColor = { { 255, 255, 0 }, { 200, 155, 0 } };
static NaviColor_RGB8 defaultPropsColor = { { 0, 255, 0 }, { 0, 255, 0 } };

// Labeled according to ActorCategory (included through ActorDB.h)
const NaviColor_RGB8 LEDColorDefaultNaviColorList[] = {
    defaultPropsColor, // ACTORCAT_SWITCH       Switch
    defaultPropsColor, // ACTORCAT_BG           Background (Prop type 1)
    defaultIdleColor,  // ACTORCAT_PLAYER       Player
    defaultPropsColor, // ACTORCAT_EXPLOSIVE    Bomb
    defaultNPCColor,   // ACTORCAT_NPC          NPC
    defaultEnemyColor, // ACTORCAT_ENEMY        Enemy
    defaultPropsColor, // ACTORCAT_PROP         Prop type 2
    defaultPropsColor, // ACTORCAT_ITEMACTION   Item/Action
    defaultPropsColor, // ACTORCAT_MISC         Misc.
    defaultEnemyColor, // ACTORCAT_BOSS         Boss
    defaultPropsColor, // ACTORCAT_DOOR         Door
    defaultPropsColor, // ACTORCAT_CHEST        Chest
    defaultPropsColor, // ACTORCAT_MAX
};

// OTRTODO: A lot of these left in Japanese are used by the mempak manager. LUS does not currently support mempaks.
// Ignore unused ones.
const char* constCameraStrings[] = {
    "INSUFFICIENT",
    "KEYFRAMES",
    "YOU CAN ADD MORE",
    "FINISHED",
    "PLAYING",
    "DEMO CAMERA TOOL",
    "CANNOT PLAY",
    "KEYFRAME   ",
    "PNT   /      ",
    ">            >",
    "<            <",
    "<          >",
    GFXP_KATAKANA "*ﾌﾟﾚｲﾔ-*",
    "E MODE FIX",
    "E MODE ABS",
    GFXP_HIRAGANA "ｶﾞﾒﾝ" GFXP_KATAKANA "   ﾃﾞﾓ", // OTRTODO: Unused, get a translation! Number 15
    GFXP_HIRAGANA "ｶﾞﾒﾝ   ﾌﾂｳ",                  // OTRTODO: Unused, get a translation! Number 16
    "P TIME  MAX",
    GFXP_KATAKANA "ﾘﾝｸ" GFXP_HIRAGANA "    ｷｵｸ", // OTRTODO: Unused, get a translation! Number 18
    GFXP_KATAKANA "ﾘﾝｸ" GFXP_HIRAGANA "     ﾑｼ", // OTRTODO: Unused, get a translation! Number 19
    "*VIEWPT*",
    "*CAMPOS*",
    "DEBUG CAMERA",
    "CENTER/LOCK",
    "CENTER/FREE",
    "DEMO CONTROL",
    GFXP_KATAKANA "ﾒﾓﾘ" GFXP_HIRAGANA "ｶﾞﾀﾘﾏｾﾝ",
    "p",
    "e",
    "s",
    "l",
    "c",
    GFXP_KATAKANA "ﾒﾓﾘﾊﾟｯｸ",
    GFXP_KATAKANA "ｾｰﾌﾞ",
    GFXP_KATAKANA "ﾛｰﾄﾞ",
    GFXP_KATAKANA "ｸﾘｱ-",
    GFXP_HIRAGANA "ｦﾇｶﾅｲﾃﾞﾈ",
    "FREE      BYTE",
    "NEED      BYTE",
    GFXP_KATAKANA "*ﾒﾓﾘ-ﾊﾟｯｸ*",
    GFXP_HIRAGANA "ｦﾐﾂｹﾗﾚﾏｾﾝ",
    GFXP_KATAKANA "ﾌｧｲﾙ " GFXP_HIRAGANA "ｦ",
    GFXP_HIRAGANA "ｼﾃﾓｲｲﾃﾞｽｶ?",
    GFXP_HIRAGANA "ｹﾞﾝｻﾞｲﾍﾝｼｭｳﾁｭｳﾉ",              // OTRTODO: Unused, get a translation! Number 43
    GFXP_KATAKANA "ﾌｧｲﾙ" GFXP_HIRAGANA "ﾊﾊｷｻﾚﾏｽ", // OTRTODO: Unused, get a translation! Number 44
    GFXP_HIRAGANA "ﾊｲ",
    GFXP_HIRAGANA "ｲｲｴ",
    GFXP_HIRAGANA "ｼﾃｲﾏｽ",
    GFXP_HIRAGANA "ｳﾜｶﾞｷ", // OTRTODO: Unused, get a translation! Number 48
    GFXP_HIRAGANA "ｼﾏｼﾀ",
    "USE       BYTE",
    GFXP_HIRAGANA "ﾆｼｯﾊﾟｲ",
    "E MODE REL",
    "FRAME       ",
    "KEY   /       ",
    "(CENTER)",
    "(ORIG)",
    "(PLAYER)",
    "(ALIGN)",
    "(SET)",
    "(OBJECT)",
    GFXP_KATAKANA "ﾎﾟｲﾝﾄNo.     ", // OTRTODO: Unused, need translation. Number 62
    "FOV              ",
    "N FRAME          ",
    "Z ROT            ",
    GFXP_KATAKANA "ﾓ-ﾄﾞ        ", // OTRTODO: Unused, need translation. Number 65
    "  R FOCUS   ",
    "PMAX              ",
    "DEPTH             ",
    "XROT              ",
    "YROT              ",
    GFXP_KATAKANA "ﾌﾚ-ﾑ         ",
    GFXP_KATAKANA "ﾄ-ﾀﾙ         ",
    GFXP_KATAKANA "ｷ-     /   ",
};

OTRGlobals::OTRGlobals() {
    std::vector<std::string> OTRFiles;
    std::string mqPath = Ship::Context::LocateFileAcrossAppDirs("oot-mq.otr", appShortName);
    if (std::filesystem::exists(mqPath)) {
        OTRFiles.push_back(mqPath);
    }
    std::string ootPath = Ship::Context::LocateFileAcrossAppDirs("oot.otr", appShortName);
    if (std::filesystem::exists(ootPath)) {
        OTRFiles.push_back(ootPath);
    }
    std::string sohOtrPath = Ship::Context::LocateFileAcrossAppDirs("soh.otr");
    if (std::filesystem::exists(sohOtrPath)) {
        OTRFiles.push_back(sohOtrPath);
    }
    std::string patchesPath = Ship::Context::LocateFileAcrossAppDirs("mods", appShortName);
    std::vector<std::string> patchOTRs = {};
    if (patchesPath.length() > 0 && std::filesystem::exists(patchesPath)) {
        if (std::filesystem::is_directory(patchesPath)) {
            for (const auto& p : std::filesystem::recursive_directory_iterator(
                     patchesPath, std::filesystem::directory_options::follow_directory_symlink)) {
                if (StringHelper::IEquals(p.path().extension().string(), ".otr") ||
                    StringHelper::IEquals(p.path().extension().string(), ".mpq") ||
                    StringHelper::IEquals(p.path().extension().string(), ".o2r") ||
                    StringHelper::IEquals(p.path().extension().string(), ".zip")) {
                    patchOTRs.push_back(p.path().generic_string());
                }
            }
        }
    }
    std::sort(patchOTRs.begin(), patchOTRs.end(), [](const std::string& a, const std::string& b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                            [](char c1, char c2) { return std::tolower(c1) < std::tolower(c2); });
    });
    OTRFiles.insert(OTRFiles.end(), patchOTRs.begin(), patchOTRs.end());
    std::unordered_set<uint32_t> ValidHashes = {
        OOT_PAL_MQ,     OOT_NTSC_JP_MQ, OOT_NTSC_US_MQ, OOT_PAL_GC_MQ_DBG, OOT_NTSC_US_10,
        OOT_NTSC_US_11, OOT_NTSC_US_12, OOT_PAL_10,     OOT_PAL_11,        OOT_NTSC_JP_GC_CE,
        OOT_NTSC_JP_GC, OOT_NTSC_US_GC, OOT_PAL_GC,     OOT_PAL_GC_DBG1,   OOT_PAL_GC_DBG2,
    };

    context = Ship::Context::CreateUninitializedInstance("Ship of Harkinian", appShortName, "shipofharkinian.json");

    context->InitLogging();
    context->InitGfxDebugger();
    context->InitConfiguration();
    context->InitConsoleVariables();
    context->InitFileDropMgr();

    // tell LUS to reserve 3 SoH specific threads (Game, Audio, Save)
    context->InitResourceManager(OTRFiles, {}, 3);
    prevAltAssets = CVarGetInteger(CVAR_SETTING("AltAssets"), 0);
    context->GetResourceManager()->SetAltAssetsEnabled(prevAltAssets);

    auto controlDeck = std::make_shared<LUS::ControlDeck>(std::vector<CONTROLLERBUTTONS_T>({
        BTN_CUSTOM_MODIFIER1,
        BTN_CUSTOM_MODIFIER2,
        BTN_CUSTOM_OCARINA_NOTE_D4,
        BTN_CUSTOM_OCARINA_NOTE_F4,
        BTN_CUSTOM_OCARINA_NOTE_A4,
        BTN_CUSTOM_OCARINA_NOTE_B4,
        BTN_CUSTOM_OCARINA_NOTE_D5,
        BTN_CUSTOM_OCARINA_DISABLE_SONGS,
        BTN_CUSTOM_OCARINA_PITCH_UP,
        BTN_CUSTOM_OCARINA_PITCH_DOWN,
    }));
    context->InitControlDeck(controlDeck);

    context->InitCrashHandler();
    context->InitConsole();

    Ship::Context::GetInstance()->GetLogger()->set_level(
        (spdlog::level::level_enum)CVarGetInteger(CVAR_DEVELOPER_TOOLS("LogLevel"), 1));
    Ship::Context::GetInstance()->GetLogger()->set_pattern("[%H:%M:%S.%e] [%s:%#] [%l] %v");

    auto sohInputEditorWindow =
        std::make_shared<SohInputEditorWindow>(CVAR_WINDOW("ControllerConfiguration"), "Configure Controller");
    auto sohFast3dWindow =
        std::make_shared<Fast::Fast3dWindow>(std::vector<std::shared_ptr<Ship::GuiWindow>>({ sohInputEditorWindow }));
    context->InitWindow(sohFast3dWindow);

    auto overlay = context->GetInstance()->GetWindow()->GetGui()->GetGameOverlay();
    overlay->LoadFont("Press Start 2P", 12.0f, "fonts/PressStart2P-Regular.ttf");
    overlay->LoadFont("Fipps", 32.0f, "fonts/Fipps-Regular.otf");
    overlay->SetCurrentFont(CVarGetString(CVAR_GAME_OVERLAY_FONT, "Press Start 2P"));

    context->InitAudio({ .SampleRate = 32000, .SampleLength = 1024, .DesiredBuffered = 1680 });

    SPDLOG_INFO("Starting Ship of Harkinian version {} (Branch: {} | Commit: {})", (char*)gBuildVersion,
                (char*)gGitBranch, (char*)gGitCommitHash);

    auto loader = context->GetResourceManager()->GetResourceLoader();
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryTextureV0>(), RESOURCE_FORMAT_BINARY,
                                    "Texture", static_cast<uint32_t>(Fast::ResourceType::Texture), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryTextureV1>(), RESOURCE_FORMAT_BINARY,
                                    "Texture", static_cast<uint32_t>(Fast::ResourceType::Texture), 1);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryVertexV0>(), RESOURCE_FORMAT_BINARY,
                                    "Vertex", static_cast<uint32_t>(Fast::ResourceType::Vertex), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryXMLVertexV0>(), RESOURCE_FORMAT_XML, "Vertex",
                                    static_cast<uint32_t>(Fast::ResourceType::Vertex), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryDisplayListV0>(),
                                    RESOURCE_FORMAT_BINARY, "DisplayList",
                                    static_cast<uint32_t>(Fast::ResourceType::DisplayList), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryXMLDisplayListV0>(), RESOURCE_FORMAT_XML,
                                    "DisplayList", static_cast<uint32_t>(Fast::ResourceType::DisplayList), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryMatrixV0>(), RESOURCE_FORMAT_BINARY,
                                    "Matrix", static_cast<uint32_t>(Fast::ResourceType::Matrix), 0);
    loader->RegisterResourceFactory(std::make_shared<Ship::ResourceFactoryBinaryBlobV0>(), RESOURCE_FORMAT_BINARY,
                                    "Blob", static_cast<uint32_t>(Ship::ResourceType::Blob), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryArrayV0>(), RESOURCE_FORMAT_BINARY,
                                    "Array", static_cast<uint32_t>(SOH::ResourceType::SOH_Array), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryAnimationV0>(), RESOURCE_FORMAT_BINARY,
                                    "Animation", static_cast<uint32_t>(SOH::ResourceType::SOH_Animation), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryPlayerAnimationV0>(),
                                    RESOURCE_FORMAT_BINARY, "PlayerAnimation",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_PlayerAnimation), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinarySceneV0>(), RESOURCE_FORMAT_BINARY,
                                    "Room", static_cast<uint32_t>(SOH::ResourceType::SOH_Room),
                                    0); // Is room scene? maybe?
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryXMLSceneV0>(), RESOURCE_FORMAT_XML, "Room",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_Room), 0); // Is room scene? maybe?
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryCollisionHeaderV0>(),
                                    RESOURCE_FORMAT_BINARY, "CollisionHeader",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_CollisionHeader), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryXMLCollisionHeaderV0>(), RESOURCE_FORMAT_XML,
                                    "CollisionHeader", static_cast<uint32_t>(SOH::ResourceType::SOH_CollisionHeader),
                                    0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinarySkeletonV0>(), RESOURCE_FORMAT_BINARY,
                                    "Skeleton", static_cast<uint32_t>(SOH::ResourceType::SOH_Skeleton), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryXMLSkeletonV0>(), RESOURCE_FORMAT_XML,
                                    "Skeleton", static_cast<uint32_t>(SOH::ResourceType::SOH_Skeleton), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinarySkeletonLimbV0>(),
                                    RESOURCE_FORMAT_BINARY, "SkeletonLimb",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_SkeletonLimb), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryXMLSkeletonLimbV0>(), RESOURCE_FORMAT_XML,
                                    "SkeletonLimb", static_cast<uint32_t>(SOH::ResourceType::SOH_SkeletonLimb), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryPathV0>(), RESOURCE_FORMAT_BINARY,
                                    "Path", static_cast<uint32_t>(SOH::ResourceType::SOH_Path), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryXMLPathV0>(), RESOURCE_FORMAT_XML, "Path",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_Path), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryCutsceneV0>(), RESOURCE_FORMAT_BINARY,
                                    "Cutscene", static_cast<uint32_t>(SOH::ResourceType::SOH_Cutscene), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryTextV0>(), RESOURCE_FORMAT_BINARY,
                                    "Text", static_cast<uint32_t>(SOH::ResourceType::SOH_Text), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryXMLTextV0>(), RESOURCE_FORMAT_XML, "Text",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_Text), 0);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryAudioSampleV2>(), RESOURCE_FORMAT_BINARY,
                                    "AudioSample", static_cast<uint32_t>(SOH::ResourceType::SOH_AudioSample), 2);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryAudioSoundFontV2>(),
                                    RESOURCE_FORMAT_BINARY, "AudioSoundFont",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_AudioSoundFont), 2);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryAudioSequenceV2>(),
                                    RESOURCE_FORMAT_BINARY, "AudioSequence",
                                    static_cast<uint32_t>(SOH::ResourceType::SOH_AudioSequence), 2);
    loader->RegisterResourceFactory(std::make_shared<SOH::ResourceFactoryBinaryBackgroundV0>(), RESOURCE_FORMAT_BINARY,
                                    "Background", static_cast<uint32_t>(SOH::ResourceType::SOH_Background), 0);

    gSaveStateMgr = std::make_shared<SaveStateMgr>();
    gRandoContext->InitStaticData();
    gRandoContext = Rando::Context::CreateInstance();
    Rando::Settings::GetInstance()->AssignContext(gRandoContext);
    Rando::StaticData::InitItemTable(); // RANDOTODO make this not rely on context's logic so it can be initialised in
                                        // InitStaticData
    Rando::Settings::GetInstance()->CreateOptions();
    gRandomizer = std::make_shared<Randomizer>();

    hasMasterQuest = hasOriginal = false;

    previousImGuiScale = defaultImGuiScale;

    fontMonoSmall = CreateFontWithSize(14.0f, "fonts/Inconsolata-Regular.ttf");
    fontMono = CreateFontWithSize(16.0f, "fonts/Inconsolata-Regular.ttf");
    fontMonoLarger = CreateFontWithSize(20.0f, "fonts/Inconsolata-Regular.ttf");
    fontMonoLargest = CreateFontWithSize(24.0f, "fonts/Inconsolata-Regular.ttf");
    fontStandard = CreateFontWithSize(16.0f, "fonts/Montserrat-Regular.ttf");
    fontStandardLarger = CreateFontWithSize(20.0f, "fonts/Montserrat-Regular.ttf");
    fontStandardLargest = CreateFontWithSize(24.0f, "fonts/Montserrat-Regular.ttf");
    ImGui::GetIO().FontDefault = fontStandardLarger;
    ScaleImGui();

    // Move the camera strings from read only memory onto the heap (writable memory)
    // This is in OTRGlobals right now because this is a place that will only ever be run once at the beginning of
    // startup. We should probably find some code in db_camera that does initialization and only run once, and then
    // dealloc on deinitialization.
    cameraStrings = (char**)malloc(sizeof(constCameraStrings));
    for (int32_t i = 0; i < sizeof(constCameraStrings) / sizeof(char*); i++) {
        // OTRTODO: never deallocated...
        auto dup = strdup(constCameraStrings[i]);
        cameraStrings[i] = dup;
    }

    auto versions = context->GetResourceManager()->GetArchiveManager()->GetGameVersions();

    for (uint32_t version : versions) {
        if (!ValidHashes.contains(version)) {
#if defined(__SWITCH__)
            SPDLOG_ERROR("Invalid OTR File!");
#elif defined(__WIIU__)
            Ship::WiiU::ThrowInvalidOTR();
#else
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid OTR File",
                                     "Attempted to load an invalid OTR file. Try regenerating.", nullptr);
            SPDLOG_ERROR("Invalid OTR File!");
#endif
            exit(1);
        }
        switch (version) {
            case OOT_PAL_MQ:
            case OOT_NTSC_JP_MQ:
            case OOT_NTSC_US_MQ:
            case OOT_PAL_GC_MQ_DBG:
                hasMasterQuest = true;
                break;
            case OOT_NTSC_US_10:
            case OOT_NTSC_US_11:
            case OOT_NTSC_US_12:
            case OOT_PAL_10:
            case OOT_PAL_11:
            case OOT_NTSC_JP_GC_CE:
            case OOT_NTSC_JP_GC:
            case OOT_NTSC_US_GC:
            case OOT_PAL_GC:
            case OOT_PAL_GC_DBG1:
            case OOT_PAL_GC_DBG2:
                hasOriginal = true;
                break;
            default:
                break;
        }
    }
}

OTRGlobals::~OTRGlobals() {
}

void OTRGlobals::ScaleImGui() {
    float scale = imguiScaleOptionToValue[CVarGetInteger(CVAR_SETTING("ImGuiScale"), defaultImGuiScale)];
    float newScale = scale / previousImGuiScale;
    ImGui::GetStyle().ScaleAllSizes(newScale);
    ImGui::GetIO().FontGlobalScale = scale;
    previousImGuiScale = scale;
}

ImFont* OTRGlobals::CreateDefaultFontWithSize(float size) {
    auto mImGuiIo = &ImGui::GetIO();
    ImFontConfig fontCfg = ImFontConfig();
    fontCfg.OversampleH = fontCfg.OversampleV = 1;
    fontCfg.PixelSnapH = true;
    fontCfg.SizePixels = size;
    ImFont* font = mImGuiIo->Fonts->AddFontDefault(&fontCfg);
    // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
    float iconFontSize = size * 2.0f / 3.0f;
    static const ImWchar sIconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    mImGuiIo->Fonts->AddFontFromMemoryCompressedBase85TTF(fontawesome_compressed_data_base85, iconFontSize,
                                                          &iconsConfig, sIconsRanges);
    return font;
}

bool OTRGlobals::HasMasterQuest() {
    return hasMasterQuest;
}

bool OTRGlobals::HasOriginal() {
    return hasOriginal;
}

uint32_t OTRGlobals::GetInterpolationFPS() {
    if (CVarGetInteger(CVAR_SETTING("MatchRefreshRate"), 0)) {
        return Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
    } else if (CVarGetInteger(CVAR_VSYNC_ENABLED, 1) ||
               !Ship::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
        return std::min<uint32_t>(Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate(),
                                  CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 20));
    }
    return CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 20);
}

extern "C" void OTRMessage_Init();
extern "C" void AudioMgr_CreateNextAudioBuffer(s16* samples, u32 num_samples);
extern "C" void AudioPlayer_Play(const uint8_t* buf, uint32_t len);
extern "C" int AudioPlayer_Buffered(void);
extern "C" int AudioPlayer_GetDesiredBuffered(void);
std::unordered_map<std::string, ExtensionEntry> ExtensionCache;

void OTRAudio_Thread() {
    while (audio.running) {
        {
            std::unique_lock<std::mutex> Lock(audio.mutex);
            while (!audio.processing && audio.running) {
                audio.cv_to_thread.wait(Lock);
            }

            if (!audio.running) {
                break;
            }
        }
        std::unique_lock<std::mutex> Lock(audio.mutex);
// AudioMgr_ThreadEntry(&gAudioMgr);
//  528 and 544 relate to 60 fps at 32 kHz 32000/60 = 533.333..
//  in an ideal world, one third of the calls should use num_samples=544 and two thirds num_samples=528
#define SAMPLES_HIGH 560
#define SAMPLES_LOW 528

#define AUDIO_FRAMES_PER_UPDATE (R_UPDATE_RATE > 0 ? R_UPDATE_RATE : 1)
#define NUM_AUDIO_CHANNELS 2

        int samples_left = AudioPlayer_Buffered();
        u32 num_audio_samples = samples_left < AudioPlayer_GetDesiredBuffered() ? SAMPLES_HIGH : SAMPLES_LOW;

        // 3 is the maximum authentic frame divisor.
        s16 audio_buffer[SAMPLES_HIGH * NUM_AUDIO_CHANNELS * 3];
        for (int i = 0; i < AUDIO_FRAMES_PER_UPDATE; i++) {
            AudioMgr_CreateNextAudioBuffer(audio_buffer + i * (num_audio_samples * NUM_AUDIO_CHANNELS),
                                           num_audio_samples);
        }

        AudioPlayer_Play((u8*)audio_buffer,
                         num_audio_samples * (sizeof(int16_t) * NUM_AUDIO_CHANNELS * AUDIO_FRAMES_PER_UPDATE));

        audio.processing = false;
        audio.cv_from_thread.notify_one();
    }
}

// C->C++ Bridge
extern "C" void OTRAudio_Init() {
    // Precache all our samples, sequences, etc...
    ResourceMgr_LoadDirectory("audio");

    if (!audio.running) {
        audio.running = true;
        audio.thread = std::thread(OTRAudio_Thread);
    }
}

extern "C" void OTRAudio_Exit() {
    // Tell the audio thread to stop
    {
        std::unique_lock<std::mutex> Lock(audio.mutex);
        audio.running = false;
    }
    audio.cv_to_thread.notify_all();

    // Wait until the audio thread quit
    audio.thread.join();
}

extern "C" void VanillaItemTable_Init() {
    static GetItemEntry getItemTable[] = {
        // clang-format off
        GET_ITEM(ITEM_BOMBS_5,          OBJECT_GI_BOMB_1,        GID_BOMB,             0x32, 0x59, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BOMBS_5),
        GET_ITEM(ITEM_NUTS_5,           OBJECT_GI_NUTS,          GID_NUTS,             0x34, 0x0C, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_NUTS_5),
        GET_ITEM(ITEM_BOMBCHU,          OBJECT_GI_BOMB_2,        GID_BOMBCHU,          0x33, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOMBCHUS_10),
        GET_ITEM(ITEM_BOW,              OBJECT_GI_BOW,           GID_BOW,              0x31, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOW),
        GET_ITEM(ITEM_SLINGSHOT,        OBJECT_GI_PACHINKO,      GID_SLINGSHOT,        0x30, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SLINGSHOT),
        GET_ITEM(ITEM_BOOMERANG,        OBJECT_GI_BOOMERANG,     GID_BOOMERANG,        0x35, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOOMERANG),
        GET_ITEM(ITEM_STICK,            OBJECT_GI_STICK,         GID_STICK,            0x37, 0x0D, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_STICKS_1),
        GET_ITEM(ITEM_HOOKSHOT,         OBJECT_GI_HOOKSHOT,      GID_HOOKSHOT,         0x36, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_HOOKSHOT),
        GET_ITEM(ITEM_LONGSHOT,         OBJECT_GI_HOOKSHOT,      GID_LONGSHOT,         0x4F, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_LONGSHOT),
        GET_ITEM(ITEM_LENS,             OBJECT_GI_GLASSES,       GID_LENS,             0x39, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_LENS),
        GET_ITEM(ITEM_LETTER_ZELDA,     OBJECT_GI_LETTER,        GID_LETTER_ZELDA,     0x69, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_LETTER_ZELDA),
        GET_ITEM(ITEM_OCARINA_TIME,     OBJECT_GI_OCARINA,       GID_OCARINA_TIME,     0x3A, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_OCARINA_OOT),
        GET_ITEM(ITEM_HAMMER,           OBJECT_GI_HAMMER,        GID_HAMMER,           0x38, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_HAMMER),
        GET_ITEM(ITEM_COJIRO,           OBJECT_GI_NIWATORI,      GID_COJIRO,           0x02, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_COJIRO),
        GET_ITEM(ITEM_BOTTLE,           OBJECT_GI_BOTTLE,        GID_BOTTLE,           0x42, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOTTLE),
        GET_ITEM(ITEM_POTION_RED,       OBJECT_GI_LIQUID,        GID_POTION_RED,       0x43, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_POTION_RED),
        GET_ITEM(ITEM_POTION_GREEN,     OBJECT_GI_LIQUID,        GID_POTION_GREEN,     0x44, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_POTION_GREEN),
        GET_ITEM(ITEM_POTION_BLUE,      OBJECT_GI_LIQUID,        GID_POTION_BLUE,      0x45, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_POTION_BLUE),
        GET_ITEM(ITEM_FAIRY,            OBJECT_GI_BOTTLE,        GID_BOTTLE,           0x46, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_FAIRY),
        GET_ITEM(ITEM_MILK_BOTTLE,      OBJECT_GI_MILK,          GID_MILK,             0x98, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MILK_BOTTLE),
        GET_ITEM(ITEM_LETTER_RUTO,      OBJECT_GI_BOTTLE_LETTER, GID_LETTER_RUTO,      0x99, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_LETTER_RUTO),
        GET_ITEM(ITEM_BEAN,             OBJECT_GI_BEAN,          GID_BEAN,             0x48, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BEAN),
        GET_ITEM(ITEM_MASK_SKULL,       OBJECT_GI_SKJ_MASK,      GID_MASK_SKULL,       0x10, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_SKULL),
        GET_ITEM(ITEM_MASK_SPOOKY,      OBJECT_GI_REDEAD_MASK,   GID_MASK_SPOOKY,      0x11, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_SPOOKY),
        GET_ITEM(ITEM_CHICKEN,          OBJECT_GI_NIWATORI,      GID_CHICKEN,          0x48, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_CHICKEN),
        GET_ITEM(ITEM_MASK_KEATON,      OBJECT_GI_KI_TAN_MASK,   GID_MASK_KEATON,      0x12, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_KEATON),
        GET_ITEM(ITEM_MASK_BUNNY,       OBJECT_GI_RABIT_MASK,    GID_MASK_BUNNY,       0x13, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_BUNNY),
        GET_ITEM(ITEM_MASK_TRUTH,       OBJECT_GI_TRUTH_MASK,    GID_MASK_TRUTH,       0x17, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_TRUTH),
        GET_ITEM(ITEM_POCKET_EGG,       OBJECT_GI_EGG,           GID_EGG,              0x01, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_POCKET_EGG),
        GET_ITEM(ITEM_POCKET_CUCCO,     OBJECT_GI_NIWATORI,      GID_CHICKEN,          0x48, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_POCKET_CUCCO),
        GET_ITEM(ITEM_ODD_MUSHROOM,     OBJECT_GI_MUSHROOM,      GID_ODD_MUSHROOM,     0x03, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_ODD_MUSHROOM),
        GET_ITEM(ITEM_ODD_POTION,       OBJECT_GI_POWDER,        GID_ODD_POTION,       0x04, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_ODD_POTION),
        GET_ITEM(ITEM_SAW,              OBJECT_GI_SAW,           GID_SAW,              0x05, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SAW),
        GET_ITEM(ITEM_SWORD_BROKEN,     OBJECT_GI_BROKENSWORD,   GID_SWORD_BROKEN,     0x08, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SWORD_BROKEN),
        GET_ITEM(ITEM_PRESCRIPTION,     OBJECT_GI_PRESCRIPTION,  GID_PRESCRIPTION,     0x09, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_PRESCRIPTION),
        GET_ITEM(ITEM_FROG,             OBJECT_GI_FROG,          GID_FROG,             0x0D, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_FROG),
        GET_ITEM(ITEM_EYEDROPS,         OBJECT_GI_EYE_LOTION,    GID_EYEDROPS,         0x0E, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_EYEDROPS),
        GET_ITEM(ITEM_CLAIM_CHECK,      OBJECT_GI_TICKETSTONE,   GID_CLAIM_CHECK,      0x0A, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_CLAIM_CHECK),
        GET_ITEM(ITEM_SWORD_KOKIRI,     OBJECT_GI_SWORD_1,       GID_SWORD_KOKIRI,     0xA4, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SWORD_KOKIRI),
        GET_ITEM(ITEM_SWORD_BGS,        OBJECT_GI_LONGSWORD,     GID_SWORD_BGS,        0x4B, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SWORD_KNIFE),
        GET_ITEM(ITEM_SHIELD_DEKU,      OBJECT_GI_SHIELD_1,      GID_SHIELD_DEKU,      0x4C, 0xA0, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_SHIELD_DEKU),
        GET_ITEM(ITEM_SHIELD_HYLIAN,    OBJECT_GI_SHIELD_2,      GID_SHIELD_HYLIAN,    0x4D, 0xA0, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_SHIELD_HYLIAN),
        GET_ITEM(ITEM_SHIELD_MIRROR,    OBJECT_GI_SHIELD_3,      GID_SHIELD_MIRROR,    0x4E, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SHIELD_MIRROR),
        GET_ITEM(ITEM_TUNIC_GORON,      OBJECT_GI_CLOTHES,       GID_TUNIC_GORON,      0x50, 0xA0, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_TUNIC_GORON),
        GET_ITEM(ITEM_TUNIC_ZORA,       OBJECT_GI_CLOTHES,       GID_TUNIC_ZORA,       0x51, 0xA0, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_TUNIC_ZORA),
        GET_ITEM(ITEM_BOOTS_IRON,       OBJECT_GI_BOOTS_2,       GID_BOOTS_IRON,       0x53, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOOTS_IRON),
        GET_ITEM(ITEM_BOOTS_HOVER,      OBJECT_GI_HOVERBOOTS,    GID_BOOTS_HOVER,      0x54, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOOTS_HOVER),
        GET_ITEM(ITEM_QUIVER_40,        OBJECT_GI_ARROWCASE,     GID_QUIVER_40,        0x56, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_QUIVER_40),
        GET_ITEM(ITEM_QUIVER_50,        OBJECT_GI_ARROWCASE,     GID_QUIVER_50,        0x57, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_QUIVER_50),
        GET_ITEM(ITEM_BOMB_BAG_20,      OBJECT_GI_BOMBPOUCH,     GID_BOMB_BAG_20,      0x58, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOMB_BAG_20),
        GET_ITEM(ITEM_BOMB_BAG_30,      OBJECT_GI_BOMBPOUCH,     GID_BOMB_BAG_30,      0x59, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_BOMB_BAG_30),
        GET_ITEM(ITEM_BOMB_BAG_40,      OBJECT_GI_BOMBPOUCH,     GID_BOMB_BAG_40,      0x5A, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_BOMB_BAG_40),
        GET_ITEM(ITEM_GAUNTLETS_SILVER, OBJECT_GI_GLOVES,        GID_GAUNTLETS_SILVER, 0x5B, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_GAUNTLETS_SILVER),
        GET_ITEM(ITEM_GAUNTLETS_GOLD,   OBJECT_GI_GLOVES,        GID_GAUNTLETS_GOLD,   0x5C, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_GAUNTLETS_GOLD),
        GET_ITEM(ITEM_SCALE_SILVER,     OBJECT_GI_SCALE,         GID_SCALE_SILVER,     0xCD, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SCALE_SILVER),
        GET_ITEM(ITEM_SCALE_GOLDEN,     OBJECT_GI_SCALE,         GID_SCALE_GOLDEN,     0xCE, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SCALE_GOLDEN),
        GET_ITEM(ITEM_STONE_OF_AGONY,   OBJECT_GI_MAP,           GID_STONE_OF_AGONY,   0x68, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_STONE_OF_AGONY),
        GET_ITEM(ITEM_GERUDO_CARD,      OBJECT_GI_GERUDO,        GID_GERUDO_CARD,      0x7B, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_GERUDO_CARD),
        GET_ITEM(ITEM_OCARINA_FAIRY,    OBJECT_GI_OCARINA_0,     GID_OCARINA_FAIRY,    0x4A, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_OCARINA_FAIRY),
        GET_ITEM(ITEM_SEEDS,            OBJECT_GI_SEED,          GID_SEEDS,            0xDC, 0x50, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_SEEDS_5),
        GET_ITEM(ITEM_HEART_CONTAINER,  OBJECT_GI_HEARTS,        GID_HEART_CONTAINER,  0xC6, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_HEART_CONTAINER),
        GET_ITEM(ITEM_HEART_PIECE_2,    OBJECT_GI_HEARTS,        GID_HEART_PIECE,      0xC2, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_HEART_PIECE),
        GET_ITEM(ITEM_KEY_BOSS,         OBJECT_GI_BOSSKEY,       GID_KEY_BOSS,         0xC7, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_BOSS_KEY,        MOD_NONE, GI_KEY_BOSS),
        GET_ITEM(ITEM_COMPASS,          OBJECT_GI_COMPASS,       GID_COMPASS,          0x67, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_COMPASS),
        GET_ITEM(ITEM_DUNGEON_MAP,      OBJECT_GI_MAP,           GID_DUNGEON_MAP,      0x66, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_MAP),
        GET_ITEM(ITEM_KEY_SMALL,        OBJECT_GI_KEY,           GID_KEY_SMALL,        0x60, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_SMALL_KEY,       MOD_NONE, GI_KEY_SMALL),
        GET_ITEM(ITEM_MAGIC_SMALL,      OBJECT_GI_MAGICPOT,      GID_MAGIC_SMALL,      0x52, 0x6F, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_MAGIC_SMALL),
        GET_ITEM(ITEM_MAGIC_LARGE,      OBJECT_GI_MAGICPOT,      GID_MAGIC_LARGE,      0x52, 0x6E, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_MAGIC_LARGE),
        GET_ITEM(ITEM_WALLET_ADULT,     OBJECT_GI_PURSE,         GID_WALLET_ADULT,     0x5E, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_WALLET_ADULT),
        GET_ITEM(ITEM_WALLET_GIANT,     OBJECT_GI_PURSE,         GID_WALLET_GIANT,     0x5F, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_WALLET_GIANT),
        GET_ITEM(ITEM_WEIRD_EGG,        OBJECT_GI_EGG,           GID_EGG,              0x9A, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_WEIRD_EGG),
        GET_ITEM(ITEM_HEART,            OBJECT_GI_HEART,         GID_HEART,            0x55, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_HEART),
        GET_ITEM(ITEM_ARROWS_SMALL,     OBJECT_GI_ARROW,         GID_ARROWS_SMALL,     0xE6, 0x48, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_ARROWS_SMALL),
        GET_ITEM(ITEM_ARROWS_MEDIUM,    OBJECT_GI_ARROW,         GID_ARROWS_MEDIUM,    0xE6, 0x49, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_ARROWS_MEDIUM),
        GET_ITEM(ITEM_ARROWS_LARGE,     OBJECT_GI_ARROW,         GID_ARROWS_LARGE,     0xE6, 0x4A, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_ARROWS_LARGE),
        GET_ITEM(ITEM_RUPEE_GREEN,      OBJECT_GI_RUPY,          GID_RUPEE_GREEN,      0x6F, 0x00, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_GREEN),
        GET_ITEM(ITEM_RUPEE_BLUE,       OBJECT_GI_RUPY,          GID_RUPEE_BLUE,       0xCC, 0x01, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_BLUE),
        GET_ITEM(ITEM_RUPEE_RED,        OBJECT_GI_RUPY,          GID_RUPEE_RED,        0xF0, 0x02, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_RED),
        GET_ITEM(ITEM_HEART_CONTAINER,  OBJECT_GI_HEARTS,        GID_HEART_CONTAINER,  0xC6, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_HEART_CONTAINER_2),
        GET_ITEM(ITEM_MILK,             OBJECT_GI_MILK,          GID_MILK,             0x98, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_MILK),
        GET_ITEM(ITEM_MASK_GORON,       OBJECT_GI_GOLONMASK,     GID_MASK_GORON,       0x14, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_GORON),
        GET_ITEM(ITEM_MASK_ZORA,        OBJECT_GI_ZORAMASK,      GID_MASK_ZORA,        0x15, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_ZORA),
        GET_ITEM(ITEM_MASK_GERUDO,      OBJECT_GI_GERUDOMASK,    GID_MASK_GERUDO,      0x16, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_MASK_GERUDO),
        GET_ITEM(ITEM_BRACELET,         OBJECT_GI_BRACELET,      GID_BRACELET,         0x79, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BRACELET),
        GET_ITEM(ITEM_RUPEE_PURPLE,     OBJECT_GI_RUPY,          GID_RUPEE_PURPLE,     0xF1, 0x14, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_PURPLE),
        GET_ITEM(ITEM_RUPEE_GOLD,       OBJECT_GI_RUPY,          GID_RUPEE_GOLD,       0xF2, 0x13, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_RUPEE_GOLD),
        GET_ITEM(ITEM_SWORD_BGS,        OBJECT_GI_LONGSWORD,     GID_SWORD_BGS,        0x0C, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_SWORD_BGS),
        GET_ITEM(ITEM_ARROW_FIRE,       OBJECT_GI_M_ARROW,       GID_ARROW_FIRE,       0x70, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_ARROW_FIRE),
        GET_ITEM(ITEM_ARROW_ICE,        OBJECT_GI_M_ARROW,       GID_ARROW_ICE,        0x71, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_ARROW_ICE),
        GET_ITEM(ITEM_ARROW_LIGHT,      OBJECT_GI_M_ARROW,       GID_ARROW_LIGHT,      0x72, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_ARROW_LIGHT),
        GET_ITEM(ITEM_SKULL_TOKEN,      OBJECT_GI_SUTARU,        GID_SKULL_TOKEN,      0xB4, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_SKULLTULA_TOKEN, MOD_NONE, GI_SKULL_TOKEN),
        GET_ITEM(ITEM_DINS_FIRE,        OBJECT_GI_GODDESS,       GID_DINS_FIRE,        0xAD, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_DINS_FIRE),
        GET_ITEM(ITEM_FARORES_WIND,     OBJECT_GI_GODDESS,       GID_FARORES_WIND,     0xAE, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_FARORES_WIND),
        GET_ITEM(ITEM_NAYRUS_LOVE,      OBJECT_GI_GODDESS,       GID_NAYRUS_LOVE,      0xAF, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_NAYRUS_LOVE),
        GET_ITEM(ITEM_BULLET_BAG_30,    OBJECT_GI_DEKUPOUCH,     GID_BULLET_BAG,       0x07, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_BULLET_BAG_30),
        GET_ITEM(ITEM_BULLET_BAG_40,    OBJECT_GI_DEKUPOUCH,     GID_BULLET_BAG,       0x07, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_BULLET_BAG_40),
        GET_ITEM(ITEM_STICKS_5,         OBJECT_GI_STICK,         GID_STICK,            0x37, 0x0D, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_STICKS_5),
        GET_ITEM(ITEM_STICKS_10,        OBJECT_GI_STICK,         GID_STICK,            0x37, 0x0D, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_STICKS_10),
        GET_ITEM(ITEM_NUTS_5,           OBJECT_GI_NUTS,          GID_NUTS,             0x34, 0x0C, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_NUTS_5_2),
        GET_ITEM(ITEM_NUTS_10,          OBJECT_GI_NUTS,          GID_NUTS,             0x34, 0x0C, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_NUTS_10),
        GET_ITEM(ITEM_BOMB,             OBJECT_GI_BOMB_1,        GID_BOMB,             0x32, 0x59, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BOMBS_1),
        GET_ITEM(ITEM_BOMBS_10,         OBJECT_GI_BOMB_1,        GID_BOMB,             0x32, 0x59, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BOMBS_10),
        GET_ITEM(ITEM_BOMBS_20,         OBJECT_GI_BOMB_1,        GID_BOMB,             0x32, 0x59, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BOMBS_20),
        GET_ITEM(ITEM_BOMBS_30,         OBJECT_GI_BOMB_1,        GID_BOMB,             0x32, 0x59, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BOMBS_30),
        GET_ITEM(ITEM_SEEDS_30,         OBJECT_GI_SEED,          GID_SEEDS,            0xDC, 0x50, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_SEEDS_30),
        GET_ITEM(ITEM_BOMBCHUS_5,       OBJECT_GI_BOMB_2,        GID_BOMBCHU,          0x33, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOMBCHUS_5),
        GET_ITEM(ITEM_BOMBCHUS_20,      OBJECT_GI_BOMB_2,        GID_BOMBCHU,          0x33, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_MAJOR,           MOD_NONE, GI_BOMBCHUS_20),
        GET_ITEM(ITEM_FISH,             OBJECT_GI_FISH,          GID_FISH,             0x47, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_FISH),
        GET_ITEM(ITEM_BUG,              OBJECT_GI_INSECT,        GID_BUG,              0x7A, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BUGS),
        GET_ITEM(ITEM_BLUE_FIRE,        OBJECT_GI_FIRE,          GID_BLUE_FIRE,        0x5D, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BLUE_FIRE),
        GET_ITEM(ITEM_POE,              OBJECT_GI_GHOST,         GID_POE,              0x97, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_POE),
        GET_ITEM(ITEM_BIG_POE,          OBJECT_GI_GHOST,         GID_BIG_POE,          0xF9, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_JUNK,            MOD_NONE, GI_BIG_POE),
        GET_ITEM(ITEM_KEY_SMALL,        OBJECT_GI_KEY,           GID_KEY_SMALL,        0xF3, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_SMALL_KEY,       MOD_NONE, GI_DOOR_KEY),
        GET_ITEM(ITEM_RUPEE_GREEN,      OBJECT_GI_RUPY,          GID_RUPEE_GREEN,      0xF4, 0x00, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_GREEN_LOSE),
        GET_ITEM(ITEM_RUPEE_BLUE,       OBJECT_GI_RUPY,          GID_RUPEE_BLUE,       0xF5, 0x01, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_BLUE_LOSE),
        GET_ITEM(ITEM_RUPEE_RED,        OBJECT_GI_RUPY,          GID_RUPEE_RED,        0xF6, 0x02, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_RED_LOSE),
        GET_ITEM(ITEM_RUPEE_PURPLE,     OBJECT_GI_RUPY,          GID_RUPEE_PURPLE,     0xF7, 0x14, CHEST_ANIM_SHORT, ITEM_CATEGORY_JUNK,            MOD_NONE, GI_RUPEE_PURPLE_LOSE),
        GET_ITEM(ITEM_HEART_PIECE_2,    OBJECT_GI_HEARTS,        GID_HEART_PIECE,      0xFA, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_HEART_PIECE_WIN),
        GET_ITEM(ITEM_STICK_UPGRADE_20, OBJECT_GI_STICK,         GID_STICK,            0x90, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_STICK_UPGRADE_20),
        GET_ITEM(ITEM_STICK_UPGRADE_30, OBJECT_GI_STICK,         GID_STICK,            0x91, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_STICK_UPGRADE_30),
        GET_ITEM(ITEM_NUT_UPGRADE_30,   OBJECT_GI_NUTS,          GID_NUTS,             0xA7, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_NUT_UPGRADE_30),
        GET_ITEM(ITEM_NUT_UPGRADE_40,   OBJECT_GI_NUTS,          GID_NUTS,             0xA8, 0x80, CHEST_ANIM_SHORT, ITEM_CATEGORY_LESSER,          MOD_NONE, GI_NUT_UPGRADE_40),
        GET_ITEM(ITEM_BULLET_BAG_50,    OBJECT_GI_DEKUPOUCH,     GID_BULLET_BAG_50,    0x6C, 0x80, CHEST_ANIM_LONG,  ITEM_CATEGORY_LESSER,          MOD_NONE, GI_BULLET_BAG_50),
        GET_ITEM_NONE,
        GET_ITEM_NONE,
        GET_ITEM_NONE // GI_MAX - if you need to add to this table insert it before this entry.
        // clang-format on
    };
    ItemTableManager::Instance->AddItemTable(MOD_NONE);
    for (uint8_t i = 0; i < ARRAY_COUNT(getItemTable); i++) {
        // The vanilla item table array started with ITEM_BOMBS_5,
        // but the GetItemID enum started with GI_NONE. Then everywhere
        // that table was accessed used `GetItemID - 1`. This allows the
        // "first" item of the new map to start at 1, syncing it up with
        // the GetItemID values and removing the need for the `- 1`
        ItemTableManager::Instance->AddItemEntry(MOD_NONE, i + 1, getItemTable[i]);
    }
}

std::unordered_map<ItemID, GetItemID> ItemIDtoGetItemIDMap{
    { ITEM_ARROWS_LARGE, GI_ARROWS_LARGE },
    { ITEM_ARROWS_MEDIUM, GI_ARROWS_MEDIUM },
    { ITEM_ARROWS_SMALL, GI_ARROWS_SMALL },
    { ITEM_ARROW_FIRE, GI_ARROW_FIRE },
    { ITEM_ARROW_ICE, GI_ARROW_ICE },
    { ITEM_ARROW_LIGHT, GI_ARROW_LIGHT },
    { ITEM_BEAN, GI_BEAN },
    { ITEM_BIG_POE, GI_BIG_POE },
    { ITEM_BLUE_FIRE, GI_BLUE_FIRE },
    { ITEM_BOMB, GI_BOMBS_1 },
    { ITEM_BOMBCHU, GI_BOMBCHUS_10 },
    { ITEM_BOMBCHUS_20, GI_BOMBCHUS_20 },
    { ITEM_BOMBCHUS_5, GI_BOMBCHUS_5 },
    { ITEM_BOMBS_10, GI_BOMBS_10 },
    { ITEM_BOMBS_20, GI_BOMBS_20 },
    { ITEM_BOMBS_30, GI_BOMBS_30 },
    { ITEM_BOMBS_5, GI_BOMBS_5 },
    { ITEM_BOMB_BAG_20, GI_BOMB_BAG_20 },
    { ITEM_BOMB_BAG_30, GI_BOMB_BAG_30 },
    { ITEM_BOMB_BAG_40, GI_BOMB_BAG_40 },
    { ITEM_BOOMERANG, GI_BOOMERANG },
    { ITEM_BOOTS_HOVER, GI_BOOTS_HOVER },
    { ITEM_BOOTS_IRON, GI_BOOTS_IRON },
    { ITEM_BOTTLE, GI_BOTTLE },
    { ITEM_BOW, GI_BOW },
    { ITEM_BRACELET, GI_BRACELET },
    { ITEM_BUG, GI_BUGS },
    { ITEM_BULLET_BAG_30, GI_BULLET_BAG_30 },
    { ITEM_BULLET_BAG_40, GI_BULLET_BAG_40 },
    { ITEM_BULLET_BAG_50, GI_BULLET_BAG_50 },
    { ITEM_CHICKEN, GI_CHICKEN },
    { ITEM_CLAIM_CHECK, GI_CLAIM_CHECK },
    { ITEM_COJIRO, GI_COJIRO },
    { ITEM_COMPASS, GI_COMPASS },
    { ITEM_DINS_FIRE, GI_DINS_FIRE },
    { ITEM_DUNGEON_MAP, GI_MAP },
    { ITEM_EYEDROPS, GI_EYEDROPS },
    { ITEM_FAIRY, GI_FAIRY },
    { ITEM_FARORES_WIND, GI_FARORES_WIND },
    { ITEM_FISH, GI_FISH },
    { ITEM_FROG, GI_FROG },
    { ITEM_GAUNTLETS_GOLD, GI_GAUNTLETS_GOLD },
    { ITEM_GAUNTLETS_SILVER, GI_GAUNTLETS_SILVER },
    { ITEM_GERUDO_CARD, GI_GERUDO_CARD },
    { ITEM_HAMMER, GI_HAMMER },
    { ITEM_HEART, GI_HEART },
    { ITEM_HEART_CONTAINER, GI_HEART_CONTAINER },
    { ITEM_HEART_CONTAINER, GI_HEART_CONTAINER_2 },
    { ITEM_HEART_PIECE_2, GI_HEART_PIECE },
    { ITEM_HEART_PIECE_2, GI_HEART_PIECE_WIN },
    { ITEM_HOOKSHOT, GI_HOOKSHOT },
    { ITEM_KEY_BOSS, GI_KEY_BOSS },
    { ITEM_KEY_SMALL, GI_DOOR_KEY },
    { ITEM_KEY_SMALL, GI_KEY_SMALL },
    { ITEM_LENS, GI_LENS },
    { ITEM_LETTER_RUTO, GI_LETTER_RUTO },
    { ITEM_LETTER_ZELDA, GI_LETTER_ZELDA },
    { ITEM_LONGSHOT, GI_LONGSHOT },
    { ITEM_MAGIC_LARGE, GI_MAGIC_LARGE },
    { ITEM_MAGIC_SMALL, GI_MAGIC_SMALL },
    { ITEM_MASK_BUNNY, GI_MASK_BUNNY },
    { ITEM_MASK_GERUDO, GI_MASK_GERUDO },
    { ITEM_MASK_GORON, GI_MASK_GORON },
    { ITEM_MASK_KEATON, GI_MASK_KEATON },
    { ITEM_MASK_SKULL, GI_MASK_SKULL },
    { ITEM_MASK_SPOOKY, GI_MASK_SPOOKY },
    { ITEM_MASK_TRUTH, GI_MASK_TRUTH },
    { ITEM_MASK_ZORA, GI_MASK_ZORA },
    { ITEM_MILK, GI_MILK },
    { ITEM_MILK_BOTTLE, GI_MILK_BOTTLE },
    { ITEM_NAYRUS_LOVE, GI_NAYRUS_LOVE },
    { ITEM_NUT, GI_NUTS_5 },
    { ITEM_NUTS_10, GI_NUTS_10 },
    { ITEM_NUTS_5, GI_NUTS_5 },
    { ITEM_NUTS_5, GI_NUTS_5_2 },
    { ITEM_NUT_UPGRADE_30, GI_NUT_UPGRADE_30 },
    { ITEM_NUT_UPGRADE_40, GI_NUT_UPGRADE_40 },
    { ITEM_OCARINA_FAIRY, GI_OCARINA_FAIRY },
    { ITEM_OCARINA_TIME, GI_OCARINA_OOT },
    { ITEM_ODD_MUSHROOM, GI_ODD_MUSHROOM },
    { ITEM_ODD_POTION, GI_ODD_POTION },
    { ITEM_POCKET_CUCCO, GI_POCKET_CUCCO },
    { ITEM_POCKET_EGG, GI_POCKET_EGG },
    { ITEM_POE, GI_POE },
    { ITEM_POTION_BLUE, GI_POTION_BLUE },
    { ITEM_POTION_GREEN, GI_POTION_GREEN },
    { ITEM_POTION_RED, GI_POTION_RED },
    { ITEM_PRESCRIPTION, GI_PRESCRIPTION },
    { ITEM_QUIVER_40, GI_QUIVER_40 },
    { ITEM_QUIVER_50, GI_QUIVER_50 },
    { ITEM_RUPEE_BLUE, GI_RUPEE_BLUE },
    { ITEM_RUPEE_BLUE, GI_RUPEE_BLUE_LOSE },
    { ITEM_RUPEE_GOLD, GI_RUPEE_GOLD },
    { ITEM_RUPEE_GREEN, GI_RUPEE_GREEN },
    { ITEM_RUPEE_GREEN, GI_RUPEE_GREEN_LOSE },
    { ITEM_RUPEE_PURPLE, GI_RUPEE_PURPLE },
    { ITEM_RUPEE_PURPLE, GI_RUPEE_PURPLE_LOSE },
    { ITEM_RUPEE_RED, GI_RUPEE_RED },
    { ITEM_RUPEE_RED, GI_RUPEE_RED_LOSE },
    { ITEM_SAW, GI_SAW },
    { ITEM_SCALE_GOLDEN, GI_SCALE_GOLDEN },
    { ITEM_SCALE_SILVER, GI_SCALE_SILVER },
    { ITEM_SEEDS, GI_SEEDS_5 },
    { ITEM_SEEDS_30, GI_SEEDS_30 },
    { ITEM_SHIELD_DEKU, GI_SHIELD_DEKU },
    { ITEM_SHIELD_HYLIAN, GI_SHIELD_HYLIAN },
    { ITEM_SHIELD_MIRROR, GI_SHIELD_MIRROR },
    { ITEM_SKULL_TOKEN, GI_SKULL_TOKEN },
    { ITEM_SLINGSHOT, GI_SLINGSHOT },
    { ITEM_STICK, GI_STICKS_1 },
    { ITEM_STICKS_10, GI_STICKS_10 },
    { ITEM_STICKS_5, GI_STICKS_5 },
    { ITEM_STICK_UPGRADE_20, GI_STICK_UPGRADE_20 },
    { ITEM_STICK_UPGRADE_30, GI_STICK_UPGRADE_30 },
    { ITEM_STONE_OF_AGONY, GI_STONE_OF_AGONY },
    { ITEM_SWORD_BGS, GI_SWORD_BGS },
    { ITEM_SWORD_BGS, GI_SWORD_KNIFE },
    { ITEM_SWORD_BROKEN, GI_SWORD_BROKEN },
    { ITEM_SWORD_KOKIRI, GI_SWORD_KOKIRI },
    { ITEM_TUNIC_GORON, GI_TUNIC_GORON },
    { ITEM_TUNIC_ZORA, GI_TUNIC_ZORA },
    { ITEM_WALLET_ADULT, GI_WALLET_ADULT },
    { ITEM_WALLET_GIANT, GI_WALLET_GIANT },
    { ITEM_WEIRD_EGG, GI_WEIRD_EGG },
};

extern "C" GetItemID RetrieveGetItemIDFromItemID(ItemID itemID) {
    if (ItemIDtoGetItemIDMap.contains(itemID)) {
        return ItemIDtoGetItemIDMap.at(itemID);
    }
    return GI_MAX;
}

std::unordered_map<ItemID, RandomizerGet> ItemIDtoRandomizerGetMap{
    { ITEM_SONG_MINUET, RG_MINUET_OF_FOREST },
    { ITEM_SONG_BOLERO, RG_BOLERO_OF_FIRE },
    { ITEM_SONG_SERENADE, RG_SERENADE_OF_WATER },
    { ITEM_SONG_REQUIEM, RG_REQUIEM_OF_SPIRIT },
    { ITEM_SONG_NOCTURNE, RG_NOCTURNE_OF_SHADOW },
    { ITEM_SONG_PRELUDE, RG_PRELUDE_OF_LIGHT },
    { ITEM_SONG_LULLABY, RG_ZELDAS_LULLABY },
    { ITEM_SONG_EPONA, RG_EPONAS_SONG },
    { ITEM_SONG_SARIA, RG_SARIAS_SONG },
    { ITEM_SONG_SUN, RG_SUNS_SONG },
    { ITEM_SONG_TIME, RG_SONG_OF_TIME },
    { ITEM_SONG_STORMS, RG_SONG_OF_STORMS },
    { ITEM_MEDALLION_FOREST, RG_FOREST_MEDALLION },
    { ITEM_MEDALLION_FIRE, RG_FIRE_MEDALLION },
    { ITEM_MEDALLION_WATER, RG_WATER_MEDALLION },
    { ITEM_MEDALLION_SPIRIT, RG_SPIRIT_MEDALLION },
    { ITEM_MEDALLION_SHADOW, RG_SHADOW_MEDALLION },
    { ITEM_MEDALLION_LIGHT, RG_LIGHT_MEDALLION },
    { ITEM_KOKIRI_EMERALD, RG_KOKIRI_EMERALD },
    { ITEM_GORON_RUBY, RG_GORON_RUBY },
    { ITEM_ZORA_SAPPHIRE, RG_ZORA_SAPPHIRE },
    { ITEM_SWORD_MASTER, RG_MASTER_SWORD },
};

extern "C" RandomizerGet RetrieveRandomizerGetFromItemID(ItemID itemID) {
    if (ItemIDtoRandomizerGetMap.contains(itemID)) {
        return ItemIDtoRandomizerGetMap.at(itemID);
    }
    return RG_MAX;
}

extern "C" void OTRExtScanner() {
    auto lst = *Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->ListFiles().get();

    for (auto& rPath : lst) {
        std::vector<std::string> raw = StringHelper::Split(rPath, ".");
        std::string ext = raw[raw.size() - 1];
        std::string nPath = rPath.substr(0, rPath.size() - (ext.size() + 1));
        replace(nPath.begin(), nPath.end(), '\\', '/');

        ExtensionCache[nPath] = { rPath, ext };
    }
}

typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} OTRVersion;

// Read the port version from an OTR file
OTRVersion ReadPortVersionFromOTR(std::string otrPath) {
    OTRVersion version = {};

    // Use a temporary archive instance to load the otr and read the version file
    auto archive = std::make_shared<Ship::OtrArchive>(otrPath);
    if (archive->Open()) {
        auto t = archive->LoadFile("portVersion");
        if (t != nullptr && t->IsLoaded) {
            auto stream = std::make_shared<Ship::MemoryStream>(t->Buffer->data(), t->Buffer->size());
            auto reader = std::make_shared<Ship::BinaryReader>(stream);
            Ship::Endianness endianness = (Ship::Endianness)reader->ReadUByte();
            reader->SetEndianness(endianness);
            version.major = reader->ReadUInt16();
            version.minor = reader->ReadUInt16();
            version.patch = reader->ReadUInt16();
        }
        archive->Close();
    }

    return version;
}

// Check that a soh.otr exists and matches the version of soh running
// Otherwise show a message and exit
void CheckSoHOTRVersion(std::string otrPath) {
    std::string msg;

#if defined(__SWITCH__)
    msg = "\x1b[4;2HPlease re-extract it from the download."
          "\x1b[6;2HPress the Home button to exit...";
#elif defined(__WIIU__)
    msg = "Please extract the soh.otr from the Ship of Harkinian download\nto your folder.\n\nPress and hold the power "
          "button to shutdown...";
#else
    msg = "Please extract the soh.otr from the Ship of Harkinian download to your folder.\n\nExiting...";
#endif

    if (!std::filesystem::exists(otrPath)) {
#if not defined(__SWITCH__) && not defined(__WIIU__)
        Extractor::ShowErrorBox("soh.otr file is missing", msg.c_str());
        exit(1);
#elif defined(__SWITCH__)
        Ship::Switch::PrintErrorMessageToScreen(("\x1b[2;2HYou are missing the soh.otr file." + msg).c_str());
#elif defined(__WIIU__)
        OSFatal(("You are missing the soh.otr file\n\n" + msg).c_str());
#endif
    }

    OTRVersion otrVersion = ReadPortVersionFromOTR(otrPath);

    if (otrVersion.major != gBuildVersionMajor || otrVersion.minor != gBuildVersionMinor ||
        otrVersion.patch != gBuildVersionPatch) {
#if not defined(__SWITCH__) && not defined(__WIIU__)
        Extractor::ShowErrorBox("soh.otr file version does not match", msg.c_str());
        exit(1);
#elif defined(__SWITCH__)
        Ship::Switch::PrintErrorMessageToScreen(("\x1b[2;2HYou have an old soh.otr file." + msg).c_str());
#elif defined(__WIIU__)
        OSFatal(("You have an old soh.otr file\n\n" + msg).c_str());
#endif
    }
}

// Checks the program version stored in the otr and compares the major value to soh
// For Windows/Mac/Linux if the version doesn't match, offer to
void DetectOTRVersion(std::string fileName, bool isMQ) {
    bool isOtrOld = false;
    std::string otrPath = Ship::Context::LocateFileAcrossAppDirs(fileName, appShortName);

    // Doesn't exist so nothing to do here
    if (!std::filesystem::exists(otrPath)) {
        return;
    }

    OTRVersion otrVersion = ReadPortVersionFromOTR(otrPath);

    if (otrVersion.major != gBuildVersionMajor) {
        isOtrOld = true;
    }

    if (isOtrOld) {
#if not defined(__SWITCH__) && not defined(__WIIU__)
        char msgBuf[250];
        char version[18]; // 5 digits for int16_max (x3) + separators + terminator

        if (otrVersion.major != 0 || otrVersion.minor != 0 || otrVersion.patch != 0) {
            snprintf(version, 18, "%d.%d.%d", otrVersion.major, otrVersion.minor, otrVersion.patch);
        } else {
            snprintf(version, 18, "no version found");
        }

        snprintf(msgBuf, 250,
                 "The %s file was generated with a different version of Ship of Harkinian.\nOTR version: %s\n\n"
                 "You must regenerate to be able to play, otherwise the program will exit.\nWould you like to "
                 "regenerate it now?",
                 fileName.c_str(), version);

        if (Extractor::ShowYesNoBox("Old OTR File Found", msgBuf) == IDYES) {
            std::string installPath = Ship::Context::GetAppBundlePath();
            if (!std::filesystem::exists(installPath + "/assets/extractor")) {
                Extractor::ShowErrorBox(
                    "Extractor assets not found",
                    "Unable to regenerate. Missing assets/extractor folder needed to generate OTR file.\n\nExiting...");
                exit(1);
            }

            Extractor extract;
            if (!extract.Run(Ship::Context::GetAppDirectoryPath(appShortName),
                             isMQ ? RomSearchMode::MQ : RomSearchMode::Vanilla)) {
                Extractor::ShowErrorBox("Error", "An error occured, no OTR file was generated.\n\nExiting...");
                exit(1);
            }
            extract.CallZapd(installPath, Ship::Context::GetAppDirectoryPath(appShortName));
        } else {
            exit(1);
        }

#elif defined(__SWITCH__)
        Ship::Switch::PrintErrorMessageToScreen("\x1b[2;2HYou've launched the Ship with an old game OTR file."
                                                "\x1b[4;2HPlease regenerate a new game OTR and relaunch."
                                                "\x1b[6;2HPress the Home button to exit...");
#elif defined(__WIIU__)
        OSFatal("You've launched the Ship with an old a game OTR file.\n\n"
                "Please generate a game OTR and relaunch.\n\n"
                "Press and hold the Power button to shutdown...");
#endif
    }
}

bool IsSubpath(const std::filesystem::path& path, const std::filesystem::path& base) {
    auto rel = std::filesystem::relative(path, base);
    return !rel.empty() && rel.native()[0] != '.';
}

bool PathTestCleanup(FILE* tfile) {
    try {
        if (std::filesystem::exists("./text.txt"))
            std::filesystem::remove("./text.txt");
        if (std::filesystem::exists("./test/"))
            std::filesystem::remove("./test/");
    } catch (std::filesystem::filesystem_error const& ex) { return false; }
    return true;
}

void CheckAndCreateModFolder() {
    try {
        std::string modsPath = Ship::Context::LocateFileAcrossAppDirs("mods", appShortName);
        if (!std::filesystem::exists(modsPath)) {
            // Create mods folder relative to app dir
            modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods", appShortName);
            std::string filePath = modsPath + "/custom_mod_files_go_here.txt";
            if (std::filesystem::create_directories(modsPath)) {
                std::ofstream(filePath).close();
            }
        }
    } catch (std::filesystem::filesystem_error const& ex) {
        // Couldn't make the folder, continue silently
        return;
    }
}

extern "C" void InitOTR() {

#ifdef __SWITCH__
    Ship::Switch::Init(Ship::PreInitPhase);
#elif defined(__WIIU__)
    Ship::WiiU::Init(appShortName);
#endif

#ifdef _WIN32
    char* tempVar = getenv("TEMP");
    std::filesystem::path tempPath;
    try {
        tempPath = std::filesystem::canonical(tempVar);
    } catch (std::filesystem::filesystem_error const& ex) {
        std::string userPath = getenv("USERPROFILE");
        userPath.append("\\AppData\\Local\\Temp");
        tempPath = std::filesystem::canonical(userPath);
    }
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, _countof(buffer));
    auto ownPath = std::filesystem::canonical(buffer).parent_path();
    if (IsSubpath(ownPath, tempPath)) {
        Extractor::ShowErrorBox("Error", "SoH is running in a temp folder. Extract the .zip and run again.");
        exit(1);
    }
    FILE* tfile = fopen("./text.txt", "w");
    std::filesystem::path tfolder = std::filesystem::path("./test/");
    bool error = false;
    try {
        create_directories(tfolder);
    } catch (std::filesystem::filesystem_error const& ex) { error = true; }
    if (tfile == NULL || error) {
        Extractor::ShowErrorBox(
            "Error", "SoH does not have proper file permissions. Please move it to a folder that does and run again.");
        PathTestCleanup(tfile);
        exit(1);
    }
    fclose(tfile);
    if (!PathTestCleanup(tfile)) {
        Extractor::ShowErrorBox(
            "Error", "SoH does not have proper file permissions. Please move it to a folder that does and run again.");
        exit(1);
    }
    if (ownPath.string().find("OneDrive") != std::string::npos) {
        Extractor::ShowErrorBox(
            "Error",
            "SoH appears to be in a OneDrive folder, which will cause issues. "
            "Please move it to a folder outside of OneDrive, like the root of a drive (e.g. \"C:\\Games\\SoH\").");
        exit(1);
    }
#endif

#if not defined(__SWITCH__) && not defined(__WIIU__)
    CheckAndCreateModFolder();
#endif

    CheckSoHOTRVersion(Ship::Context::LocateFileAcrossAppDirs("soh.otr"));

    if (!std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs("oot-mq.otr", appShortName)) &&
        !std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs("oot.otr", appShortName))) {

#if not defined(__SWITCH__) && not defined(__WIIU__)
        std::string installPath = Ship::Context::GetAppBundlePath();
        if (!std::filesystem::exists(installPath + "/assets/extractor")) {
            Extractor::ShowErrorBox(
                "Extractor assets not found",
                "No OTR files found. Missing assets/extractor folder needed to generate OTR file.\n\nExiting...");
            exit(1);
        }

        bool generatedOtrIsMQ = false;
        if (Extractor::ShowYesNoBox("No OTR Files", "No OTR files found. Generate one now?") == IDYES) {
            Extractor extract;
            if (!extract.Run(Ship::Context::GetAppDirectoryPath(appShortName))) {
                Extractor::ShowErrorBox("Error", "An error occured, no OTR file was generated.\n\nExiting...");
                exit(1);
            }
            extract.CallZapd(installPath, Ship::Context::GetAppDirectoryPath(appShortName));
            generatedOtrIsMQ = extract.IsMasterQuest();
        } else {
            exit(1);
        }
        if (Extractor::ShowYesNoBox("Extraction Complete", "ROM Extracted. Extract another?") == IDYES) {
            Extractor extract;
            if (!extract.Run(Ship::Context::GetAppDirectoryPath(appShortName),
                             generatedOtrIsMQ ? RomSearchMode::Vanilla : RomSearchMode::MQ)) {
                Extractor::ShowErrorBox(
                    "Error",
                    "An error occured, an OTR file may have been generated by a different step.\n\nContinuing...");
            } else {
                extract.CallZapd(installPath, Ship::Context::GetAppDirectoryPath(appShortName));
            }
        }

#elif defined(__SWITCH__)
        Ship::Switch::PrintErrorMessageToScreen("\x1b[2;2HYou've launched the Ship without a game OTR file."
                                                "\x1b[4;2HPlease generate a game OTR and relaunch."
                                                "\x1b[6;2HPress the Home button to exit...");
#elif defined(__WIIU__)
        OSFatal("You've launched the Ship without a game OTR file.\n\n"
                "Please generate a game OTR and relaunch.\n\n"
                "Press and hold the Power button to shutdown...");
#endif
    }

    DetectOTRVersion("oot.otr", false);
    DetectOTRVersion("oot-mq.otr", true);

    OTRGlobals::Instance = new OTRGlobals();
    CustomMessageManager::Instance = new CustomMessageManager();
    ItemTableManager::Instance = new ItemTableManager();
    GameInteractor::Instance = new GameInteractor();
    SaveManager::Instance = new SaveManager();

    std::shared_ptr<Ship::Config> conf = OTRGlobals::Instance->context->GetConfig();
    conf->RegisterVersionUpdater(std::make_shared<SOH::ConfigVersion1Updater>());
    conf->RegisterVersionUpdater(std::make_shared<SOH::ConfigVersion2Updater>());
    conf->RegisterVersionUpdater(std::make_shared<SOH::ConfigVersion3Updater>());
    conf->RunVersionUpdates();

    SohGui::SetupGuiElements();
    ShipInit::InitAll();

    Rando::StaticData::InitHashMaps();
    OTRGlobals::Instance->gRandoContext->AddExcludedOptions();
    AudioCollection::Instance = new AudioCollection();
    ActorDB::Instance = new ActorDB();
#ifdef __APPLE__
    SpeechSynthesizer::Instance = new DarwinSpeechSynthesizer();
#elif defined(_WIN32)
    SpeechSynthesizer::Instance = new SAPISpeechSynthesizer();
#elif ESPEAK
    SpeechSynthesizer::Instance = new ESpeakSpeechSynthesizer();
#else
    SpeechSynthesizer::Instance = new SpeechLogger();
#endif
    SpeechSynthesizer::Instance->Init();

#ifdef ENABLE_REMOTE_CONTROL
    CrowdControl::Instance = new CrowdControl();
    Sail::Instance = new Sail();
#endif

    OTRMessage_Init();
    OTRAudio_Init();
    OTRExtScanner();
    VanillaItemTable_Init();
    DebugConsole_Init();

    InitMods();
    ActorDB::AddBuiltInCustomActors();
    // #region SOH [Randomizer] TODO: Remove these and refactor spoiler file handling for randomizer
    CVarClear(CVAR_GENERAL("RandomizerNewFileDropped"));
    CVarClear(CVAR_GENERAL("RandomizerDroppedFile"));
    // #endregion
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnFileDropped>(SoH_ProcessDroppedFiles);

    RegisterImGuiItemIcons();

    time_t now = time(NULL);
    tm* tm_now = localtime(&now);
    if (tm_now->tm_mon == 11 && tm_now->tm_mday >= 24 && tm_now->tm_mday <= 25) {
        CVarRegisterInteger(CVAR_GENERAL("LetItSnow"), 1);
    } else {
        CVarClear(CVAR_GENERAL("LetItSnow"));
    }

    srand(now);
#ifdef ENABLE_REMOTE_CONTROL
    SDLNet_Init();
    if (CVarGetInteger(CVAR_REMOTE_CROWD_CONTROL("Enabled"), 0)) {
        CrowdControl::Instance->Enable();
    }
    if (CVarGetInteger(CVAR_REMOTE_SAIL("Enabled"), 0)) {
        Sail::Instance->Enable();
    }
#endif
}

extern "C" void SaveManager_ThreadPoolWait() {
    SaveManager::Instance->ThreadPoolWait();
}

extern "C" void DeinitOTR() {
    SaveManager_ThreadPoolWait();
    OTRAudio_Exit();
#ifdef ENABLE_REMOTE_CONTROL
    if (CVarGetInteger(CVAR_REMOTE_CROWD_CONTROL("Enabled"), 0)) {
        CrowdControl::Instance->Disable();
    }
    if (CVarGetInteger(CVAR_REMOTE_SAIL("Enabled"), 0)) {
        Sail::Instance->Disable();
    }
    SDLNet_Quit();
#endif

    // Destroying gui here because we have shared ptrs to LUS objects which output to SPDLOG which is destroyed before
    // these shared ptrs.
    SohGui::Destroy();

    OTRGlobals::Instance->context = nullptr;
}

#ifdef _WIN32
extern "C" uint64_t GetFrequency() {
    LARGE_INTEGER nFreq;

    QueryPerformanceFrequency(&nFreq);

    return nFreq.QuadPart;
}

extern "C" uint64_t GetPerfCounter() {
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);

    return ticks.QuadPart;
}
#else
extern "C" uint64_t GetFrequency() {
    return 1000; // sec -> ms
}

extern "C" uint64_t GetPerfCounter() {
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);

    uint64_t remainingMs = (monotime.tv_nsec / 1000000);

    // in milliseconds
    return monotime.tv_sec * 1000 + remainingMs;
}
#endif

extern "C" uint64_t GetUnixTimestamp() {
    auto time = std::chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    return (uint64_t)millis.count();
}

extern "C" void Graph_StartFrame() {
#ifndef __WIIU__
    using Ship::KbScancode;
    int32_t dwScancode = OTRGlobals::Instance->context->GetWindow()->GetLastScancode();
    OTRGlobals::Instance->context->GetWindow()->SetLastScancode(-1);

    switch (dwScancode) {
        case KbScancode::LUS_KB_F1: {
            std::shared_ptr<SohModalWindow> modal = static_pointer_cast<SohModalWindow>(
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Modal Window"));
            if (modal->IsPopupOpen("Menu Moved")) {
                modal->DismissPopup();
            } else {
                modal->RegisterPopup("Menu Moved",
                                     "The menubar, accessed by hitting F1, no longer exists.\nThe new menu can be "
                                     "accessed by hitting the Esc button instead.",
                                     "OK");
            }
            break;
        }
        case KbScancode::LUS_KB_F5: {
            if (CVarGetInteger(CVAR_CHEAT("SaveStatesEnabled"), 0) == 0) {
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(
                    6.0f, true, "Save states not enabled. Check Cheats Menu.");
                return;
            }
            const unsigned int slot = OTRGlobals::Instance->gSaveStateMgr->GetCurrentSlot();
            const SaveStateReturn stateReturn =
                OTRGlobals::Instance->gSaveStateMgr->AddRequest({ slot, RequestType::SAVE });

            switch (stateReturn) {
                case SaveStateReturn::SUCCESS:
                    SPDLOG_INFO("[SOH] Saved state to slot {}", slot);
                    break;
                case SaveStateReturn::FAIL_WRONG_GAMESTATE:
                    SPDLOG_ERROR("[SOH] Can not save a state outside of \"GamePlay\"");
                    break;
                    [[unlikely]] default : break;
            }
            break;
        }
        case KbScancode::LUS_KB_F6: {
            if (CVarGetInteger(CVAR_CHEAT("SaveStatesEnabled"), 0) == 0) {
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(
                    6.0f, true, "Save states not enabled. Check Cheats Menu.");
                return;
            }
            unsigned int slot = OTRGlobals::Instance->gSaveStateMgr->GetCurrentSlot();
            slot++;
            if (slot > 5) {
                slot = 0;
            }
            OTRGlobals::Instance->gSaveStateMgr->SetCurrentSlot(slot);
            SPDLOG_INFO("Set SaveState slot to {}.", slot);
            break;
        }
        case KbScancode::LUS_KB_F7: {
            if (CVarGetInteger(CVAR_CHEAT("SaveStatesEnabled"), 0) == 0) {
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(
                    6.0f, true, "Save states not enabled. Check Cheats Menu.");
                return;
            }
            const unsigned int slot = OTRGlobals::Instance->gSaveStateMgr->GetCurrentSlot();
            const SaveStateReturn stateReturn =
                OTRGlobals::Instance->gSaveStateMgr->AddRequest({ slot, RequestType::LOAD });

            switch (stateReturn) {
                case SaveStateReturn::SUCCESS:
                    SPDLOG_INFO("[SOH] Loaded state from slot {}", slot);
                    break;
                case SaveStateReturn::FAIL_INVALID_SLOT:
                    SPDLOG_ERROR("[SOH] Invalid State Slot Number {}", slot);
                    break;
                case SaveStateReturn::FAIL_STATE_EMPTY:
                    SPDLOG_ERROR("[SOH] State Slot {} is empty", slot);
                    break;
                case SaveStateReturn::FAIL_WRONG_GAMESTATE:
                    SPDLOG_ERROR("[SOH] Can not load a state outside of \"GamePlay\"");
                    break;
                    [[unlikely]] default : break;
            }

            break;
        }
#if defined(_WIN32) || defined(__APPLE__)
        case KbScancode::LUS_KB_F9: {
            // Toggle TTS
            CVarSetInteger(CVAR_SETTING("A11yTTS"), !CVarGetInteger(CVAR_SETTING("A11yTTS"), 0));
            break;
        }
#endif
        case KbScancode::LUS_KB_TAB: {
            CVarSetInteger(CVAR_SETTING("AltAssets"), !CVarGetInteger(CVAR_SETTING("AltAssets"), 0));
            break;
        }
    }
#endif

    auto dropMgr = Ship::Context::GetInstance()->GetFileDropMgr();
    if (dropMgr->FileDropped()) {
        std::string filePath = dropMgr->GetDroppedFile();
        if (!filePath.empty()) {
            GameInteractor::Instance->ExecuteHooks<GameInteractor::OnFileDropped>(filePath);
        }
        dropMgr->ClearDroppedFile();
    }
}

void RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(OTRGlobals::Instance->context->GetWindow());

    if (wnd == nullptr) {
        return;
    }

    // Process window events for resize, mouse, keyboard events
    wnd->HandleEvents();

    UIWidgets::Colors themeColor =
        static_cast<UIWidgets::Colors>(CVarGetInteger(CVAR_SETTING("Menu.Theme"), UIWidgets::Colors::LightBlue));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, UIWidgets::ColorValues.at(themeColor));
    for (const auto& m : mtx_replacements) {
        wnd->DrawAndRunGraphicsCommands(Commands, m);
    }
    ImGui::PopStyleColor();
}

// C->C++ Bridge
extern "C" void Graph_ProcessGfxCommands(Gfx* commands) {
    {
        std::unique_lock<std::mutex> Lock(audio.mutex);
        audio.processing = true;
    }

    audio.cv_to_thread.notify_one();
    std::vector<std::unordered_map<Mtx*, MtxF>> mtx_replacements;
    int target_fps = OTRGlobals::Instance->GetInterpolationFPS();
    static int last_fps;
    static int last_update_rate;
    static int time;
    int fps = target_fps;
    int original_fps = 60 / R_UPDATE_RATE;
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());

    if (target_fps == 20 || original_fps > target_fps) {
        fps = original_fps;
    }

    if (last_fps != fps || last_update_rate != R_UPDATE_RATE) {
        time = 0;
    }

    // time_base = fps * original_fps (one second)
    int next_original_frame = fps;

    while (time + original_fps <= next_original_frame) {
        time += original_fps;
        if (time != next_original_frame) {
            mtx_replacements.push_back(FrameInterpolation_Interpolate((float)time / next_original_frame));
        } else {
            mtx_replacements.emplace_back();
        }
    }

    time -= fps;

    if (wnd != nullptr) {
        wnd->SetTargetFps(fps);
    }

    // When the gfx debugger is active, only run with the final mtx
    if (GfxDebuggerIsDebugging()) {
        mtx_replacements.clear();
        mtx_replacements.emplace_back();
    }

    RunCommands(commands, mtx_replacements);

    last_fps = fps;
    last_update_rate = R_UPDATE_RATE;

    {
        std::unique_lock<std::mutex> Lock(audio.mutex);
        while (audio.processing) {
            audio.cv_from_thread.wait(Lock);
        }
    }

    bool curAltAssets = CVarGetInteger(CVAR_SETTING("AltAssets"), 0);
    if (prevAltAssets != curAltAssets) {
        prevAltAssets = curAltAssets;
        Ship::Context::GetInstance()->GetResourceManager()->SetAltAssetsEnabled(curAltAssets);
        gfx_texture_cache_clear();
        SOH::SkeletonPatcher::UpdateSkeletons();
        GameInteractor::Instance->ExecuteHooks<GameInteractor::OnAssetAltChange>();
    }

    // OTRTODO: FIGURE OUT END FRAME POINT
    /* if (OTRGlobals::Instance->context->lastScancode != -1)
         OTRGlobals::Instance->context->lastScancode = -1;*/
}

float divisor_num = 0.0f;

extern "C" void OTRGetPixelDepthPrepare(float x, float y) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());
    if (wnd == nullptr) {
        return;
    }

    wnd->GetPixelDepthPrepare(x, y);
}

extern "C" uint16_t OTRGetPixelDepth(float x, float y) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());
    if (wnd == nullptr) {
        return 0;
    }

    return wnd->GetPixelDepth(x, y);
}

extern "C" Sprite* GetSeedTexture(uint8_t index) {
    return OTRGlobals::Instance->gRandoContext->GetSeedTexture(index);
}

extern "C" uint8_t GetSeedIconIndex(uint8_t index) {
    return OTRGlobals::Instance->gRandoContext->hashIconIndexes[index];
}

std::map<std::string, SoundFontSample*> cachedCustomSFs;

extern "C" SoundFontSample* ReadCustomSample(const char* path) {
    return nullptr;
    /*
    if (!ExtensionCache.contains(path))
        return nullptr;

    ExtensionEntry entry = ExtensionCache[path];

    auto sampleRaw = Ship::Context::GetInstance()->GetResourceManager()->LoadFile(entry.path);
    uint32_t* strem = (uint32_t*)sampleRaw->Buffer.get();
    uint8_t* strem2 = (uint8_t*)strem;

    SoundFontSample* sampleC = new SoundFontSample;

    if (entry.ext == "wav") {
        drwav_uint32 channels;
        drwav_uint32 sampleRate;
        drwav_uint64 totalPcm;
        drmp3_int16* pcmData =
            drwav_open_memory_and_read_pcm_frames_s16(strem2, sampleRaw->BufferSize, &channels, &sampleRate, &totalPcm,
    NULL); sampleC->size = totalPcm; sampleC->sampleAddr = (uint8_t*)pcmData; sampleC->codec = CODEC_S16;

        sampleC->loop = new AdpcmLoop;
        sampleC->loop->start = 0;
        sampleC->loop->end = sampleC->size - 1;
        sampleC->loop->count = 0;
        sampleC->sampleRateMagicValue = 'RIFF';
        sampleC->sampleRate = sampleRate;

        cachedCustomSFs[path] = sampleC;
        return sampleC;
    } else if (entry.ext == "mp3") {
        drmp3_config mp3Info;
        drmp3_uint64 totalPcm;
        drmp3_int16* pcmData =
            drmp3_open_memory_and_read_pcm_frames_s16(strem2, sampleRaw->BufferSize, &mp3Info, &totalPcm, NULL);

        sampleC->size = totalPcm * mp3Info.channels * sizeof(short);
        sampleC->sampleAddr = (uint8_t*)pcmData;
        sampleC->codec = CODEC_S16;

        sampleC->loop = new AdpcmLoop;
        sampleC->loop->start = 0;
        sampleC->loop->end = sampleC->size;
        sampleC->loop->count = 0;
        sampleC->sampleRateMagicValue = 'RIFF';
        sampleC->sampleRate = mp3Info.sampleRate;

        cachedCustomSFs[path] = sampleC;
        return sampleC;
    }

    return nullptr;
    */
}

ImFont* OTRGlobals::CreateFontWithSize(float size, std::string fontPath) {
    auto mImGuiIo = &ImGui::GetIO();
    ImFont* font;
    if (fontPath == "") {
        ImFontConfig fontCfg = ImFontConfig();
        fontCfg.OversampleH = fontCfg.OversampleV = 1;
        fontCfg.PixelSnapH = true;
        fontCfg.SizePixels = size;
        font = mImGuiIo->Fonts->AddFontDefault(&fontCfg);
    } else {
        auto initData = std::make_shared<Ship::ResourceInitData>();
        initData->Format = RESOURCE_FORMAT_BINARY;
        initData->Type = static_cast<uint32_t>(RESOURCE_TYPE_FONT);
        initData->ResourceVersion = 0;
        initData->Path = fontPath;
        std::shared_ptr<Ship::Font> fontData = std::static_pointer_cast<Ship::Font>(
            Ship::Context::GetInstance()->GetResourceManager()->LoadResource(fontPath, false, initData));
        font = mImGuiIo->Fonts->AddFontFromMemoryTTF(fontData->Data, fontData->DataSize, size);
    }
    // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
    float iconFontSize = size * 2.0f / 3.0f;
    static const ImWchar sIconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    mImGuiIo->Fonts->AddFontFromMemoryCompressedBase85TTF(fontawesome_compressed_data_base85, iconFontSize,
                                                          &iconsConfig, sIconsRanges);
    return font;
}

std::filesystem::path GetSaveFile(std::shared_ptr<Ship::Config> Conf) {
    const std::string fileName =
        Conf->GetString("Game.SaveName", Ship::Context::GetPathRelativeToAppDirectory("oot_save.sav"));
    std::filesystem::path saveFile = std::filesystem::absolute(fileName);

    if (!exists(saveFile.parent_path())) {
        create_directories(saveFile.parent_path());
    }

    return saveFile;
}

std::filesystem::path GetSaveFile() {
    const std::shared_ptr<Ship::Config> pConf = OTRGlobals::Instance->context->GetConfig();

    return GetSaveFile(pConf);
}

void OTRGlobals::CheckSaveFile(size_t sramSize) const {
    const std::shared_ptr<Ship::Config> pConf = Instance->context->GetConfig();

    std::filesystem::path savePath = GetSaveFile(pConf);
    std::fstream saveFile(savePath, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (saveFile.fail()) {
        saveFile.open(savePath, std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
        for (int i = 0; i < sramSize; ++i) {
            saveFile.write("\0", 1);
        }
    }
    saveFile.close();
}

extern "C" void Ctx_ReadSaveFile(uintptr_t addr, void* dramAddr, size_t size) {
    SaveManager::ReadSaveFile(GetSaveFile(), addr, dramAddr, size);
}

extern "C" void Ctx_WriteSaveFile(uintptr_t addr, void* dramAddr, size_t size) {
    SaveManager::WriteSaveFile(GetSaveFile(), addr, dramAddr, size);
}

std::wstring StringToU16(const std::string& s) {
    std::vector<unsigned long> result;
    size_t i = 0;

    while (i < s.size()) {
        unsigned long uni;
        size_t nbytes = 0;
        bool error = false;
        unsigned char c = s[i++];
        if (c < 0x80) { // ascii
            uni = c;
            nbytes = 0;
        } else if (c == GFXP_HIRAGANA_CHAR) { // Start Hiragana Mode
            uni = c;
            nbytes = 0;
        } else if (c == GFXP_KATAKANA_CHAR) { // Start Katakana Mode
            uni = c;
            nbytes = 0;
        } else if (c <= 0xBF) { // Invalid Characters (Skipped)
            nbytes = 0;
            uni = '\1';
        } else if (c <= 0xDF) {
            uni = c & 0x1F;
            nbytes = 1;
        } else if (c <= 0xEF) {
            uni = c & 0x0F;
            nbytes = 2;
        } else if (c <= 0xF7) {
            uni = c & 0x07;
            nbytes = 3;
        }
        for (size_t j = 0; j < nbytes; ++j) {
            unsigned char c = s[i++];
            uni <<= 6;
            uni += c & 0x3F;
        }
        if (uni != '\1')
            result.push_back(uni);
    }
    std::wstring utf16;
    for (size_t i = 0; i < result.size(); ++i) {
        unsigned long uni = result[i];
        if (uni <= 0xFFFF) {
            utf16 += (wchar_t)uni;
        } else {
            uni -= 0x10000;
            utf16 += (wchar_t)((uni >> 10) + 0xD800);
            utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
        }
    }
    return utf16;
}

extern "C" void OTRGfxPrint(const char* str, void* printer, void (*printImpl)(void*, char)) {
    const std::vector<uint32_t> hira1 = {
        u'を', u'ぁ', u'ぃ', u'ぅ', u'ぇ', u'ぉ', u'ゃ', u'ゅ', u'ょ', u'っ', u'-',  u'あ', u'い',
        u'う', u'え', u'お', u'か', u'き', u'く', u'け', u'こ', u'さ', u'し', u'す', u'せ', u'そ',
    };

    const std::vector<uint32_t> hira2 = {
        u'た', u'ち', u'つ', u'て', u'と', u'な', u'に', u'ぬ', u'ね', u'の', u'は', u'ひ', u'ふ', u'へ', u'ほ', u'ま',
        u'み', u'む', u'め', u'も', u'や', u'ゆ', u'よ', u'ら', u'り', u'る', u'れ', u'ろ', u'わ', u'ん', u'゛', u'゜',
    };

    const std::vector<uint32_t> kata1 = {
        u'ヲ', u'ァ', u'ィ', u'ゥ', u'ェ', u'ォ', u'ャ', u'ュ', u'ョ', u'ッ', u'ー',
    };

    const std::vector<uint32_t> kata2 = {
        u'ア', u'イ', u'ウ', u'エ', u'オ', u'カ', u'キ', u'ク', u'ケ', u'コ', u'サ', u'シ', u'ス', u'セ', u'ソ',
        u'タ', u'チ', u'ツ', u'テ', u'ト', u'ナ', u'ニ', u'ヌ', u'ネ', u'ノ', u'ハ', u'ヒ', u'フ', u'ヘ', u'ホ',
        u'マ', u'ミ', u'ム', u'メ', u'モ', u'ヤ', u'ユ', u'ヨ', u'ラ', u'リ', u'ル', u'レ', u'ロ', u'ワ', u'ン',
    };

    std::wstring wstr = StringToU16(str);
    bool hiraganaMode = false;

    for (const auto& c : wstr) {
        if (c < 0x80) {
            printImpl(printer, c);
        } else if (c == GFXP_HIRAGANA_CHAR) {
            hiraganaMode = true;
        } else if (c == GFXP_KATAKANA_CHAR) {
            hiraganaMode = false;
        } else if (c >= u'｡' && c <= u'ﾟ') { // katakana (hankaku)
            if (hiraganaMode && c >= u'ｦ' && c <= u'ｿ') {
                printImpl(printer, c - 0xFEC0 - 0x20); // Hiragana Mode, Block 1
            } else if (hiraganaMode && c >= u'ﾀ' && c <= u'ﾝ') {
                printImpl(printer, c - 0xFEC0 + 0x20); // Hiragana Mode, Block 2
            } else {
                printImpl(printer, c - 0xFEC0);
            }
        } else if (c == u'　') { // zenkaku space
            printImpl(printer, u' ');
        } else {
            auto it = std::find(hira1.begin(), hira1.end(), c);
            if (it != hira1.end()) { // hiragana block 1
                printImpl(printer, 0x86 + std::distance(hira1.begin(), it));
            }

            auto it2 = std::find(hira2.begin(), hira2.end(), c);
            if (it2 != hira2.end()) { // hiragana block 2
                printImpl(printer, 0xe0 + std::distance(hira2.begin(), it2));
            }

            auto it3 = std::find(kata1.begin(), kata1.end(), c);
            if (it3 != kata1.end()) { // katakana zenkaku block 1
                printImpl(printer, 0xa6 + std::distance(kata1.begin(), it3));
            }

            auto it4 = std::find(kata2.begin(), kata2.end(), c);
            if (it4 != kata2.end()) { // katakana zenkaku block 2
                printImpl(printer, 0xb1 + std::distance(kata2.begin(), it4));
            }
        }
    }
}

extern "C" uint32_t OTRGetCurrentWidth() {
    return OTRGlobals::Instance->context->GetWindow()->GetWidth();
}

extern "C" uint32_t OTRGetCurrentHeight() {
    return OTRGlobals::Instance->context->GetWindow()->GetHeight();
}

Color_RGB8 GetColorForControllerLED() {
    auto brightness = CVarGetFloat(CVAR_SETTING("LEDBrightness"), 1.0f) / 1.0f;
    Color_RGB8 color = { 0, 0, 0 };
    if (brightness > 0.0f) {
        LEDColorSource source =
            static_cast<LEDColorSource>(CVarGetInteger(CVAR_SETTING("LEDColorSource"), LED_SOURCE_TUNIC_ORIGINAL));
        bool criticalOverride = CVarGetInteger(CVAR_SETTING("LEDCriticalOverride"), 1);
        if (gPlayState && (source == LED_SOURCE_TUNIC_ORIGINAL || source == LED_SOURCE_TUNIC_COSMETICS)) {
            switch (CUR_EQUIP_VALUE(EQUIP_TYPE_TUNIC)) {
                case EQUIP_VALUE_TUNIC_KOKIRI:
                    color = source == LED_SOURCE_TUNIC_COSMETICS
                                ? CVarGetColor24(CVAR_COSMETIC("Link.KokiriTunic.Value"), kokiriColor)
                                : kokiriColor;
                    break;
                case EQUIP_VALUE_TUNIC_GORON:
                    color = source == LED_SOURCE_TUNIC_COSMETICS
                                ? CVarGetColor24(CVAR_COSMETIC("Link.GoronTunic.Value"), goronColor)
                                : goronColor;
                    break;
                case EQUIP_VALUE_TUNIC_ZORA:
                    color = source == LED_SOURCE_TUNIC_COSMETICS
                                ? CVarGetColor24(CVAR_COSMETIC("Link.ZoraTunic.Value"), zoraColor)
                                : zoraColor;
                    break;
            }
        }
        if (gPlayState && (source == LED_SOURCE_NAVI_ORIGINAL || source == LED_SOURCE_NAVI_COSMETICS)) {
            Actor* arrowPointedActor = gPlayState->actorCtx.targetCtx.arrowPointedActor;
            if (arrowPointedActor) {
                ActorCategory category = (ActorCategory)arrowPointedActor->category;
                switch (category) {
                    case ACTORCAT_PLAYER:
                        if (source == LED_SOURCE_NAVI_COSMETICS &&
                            CVarGetInteger(CVAR_COSMETIC("Navi.IdlePrimary.Changed"), 0)) {
                            color = CVarGetColor24(CVAR_COSMETIC("Navi.IdlePrimary.Value"), defaultIdleColor.inner);
                            break;
                        }
                        color = LEDColorDefaultNaviColorList[category].inner;
                        break;
                    case ACTORCAT_NPC:
                        if (source == LED_SOURCE_NAVI_COSMETICS &&
                            CVarGetInteger(CVAR_COSMETIC("Navi.NPCPrimary.Changed"), 0)) {
                            color = CVarGetColor24(CVAR_COSMETIC("Navi.NPCPrimary.Value"), defaultNPCColor.inner);
                            break;
                        }
                        color = LEDColorDefaultNaviColorList[category].inner;
                        break;
                    case ACTORCAT_ENEMY:
                    case ACTORCAT_BOSS:
                        if (source == LED_SOURCE_NAVI_COSMETICS &&
                            CVarGetInteger(CVAR_COSMETIC("Navi.EnemyPrimary.Changed"), 0)) {
                            color = CVarGetColor24(CVAR_COSMETIC("Navi.EnemyPrimary.Value"), defaultEnemyColor.inner);
                            break;
                        }
                        color = LEDColorDefaultNaviColorList[category].inner;
                        break;
                    default:
                        if (source == LED_SOURCE_NAVI_COSMETICS &&
                            CVarGetInteger(CVAR_COSMETIC("Navi.PropsPrimary.Changed"), 0)) {
                            color = CVarGetColor24(CVAR_COSMETIC("Navi.PropsPrimary.Value"), defaultPropsColor.inner);
                            break;
                        }
                        color = LEDColorDefaultNaviColorList[category].inner;
                }
            } else { // No target actor.
                if (source == LED_SOURCE_NAVI_COSMETICS &&
                    CVarGetInteger(CVAR_COSMETIC("Navi.IdlePrimary.Changed"), 0)) {
                    color = CVarGetColor24(CVAR_COSMETIC("Navi.IdlePrimary.Value"), defaultIdleColor.inner);
                } else {
                    color = LEDColorDefaultNaviColorList[ACTORCAT_PLAYER].inner;
                }
            }
        }
        if (source == LED_SOURCE_CUSTOM) {
            color = CVarGetColor24(CVAR_SETTING("LEDPort1Color"), { 255, 255, 255 });
        }
        if (gPlayState && (criticalOverride || source == LED_SOURCE_HEALTH)) {
            if (HealthMeter_IsCritical()) {
                color = { 0xFF, 0, 0 };
            } else if (gSaveContext.healthCapacity != 0 && source == LED_SOURCE_HEALTH) {
                if (gSaveContext.health / gSaveContext.healthCapacity <= 0.4f) {
                    color = { 0xFF, 0xFF, 0 };
                } else {
                    color = { 0, 0xFF, 0 };
                }
            }
        }
        color.r = color.r * brightness;
        color.g = color.g * brightness;
        color.b = color.b * brightness;
    }

    return color;
}

extern "C" void OTRControllerCallback(uint8_t rumble) {
    // We call this every tick, SDL accounts for this use and prevents driver spam
    // https://github.com/libsdl-org/SDL/blob/f17058b562c8a1090c0c996b42982721ace90903/src/joystick/SDL_joystick.c#L1114-L1144
    Ship::Context::GetInstance()->GetControlDeck()->GetControllerByPort(0)->GetLED()->SetLEDColor(
        GetColorForControllerLED());

    static std::shared_ptr<SohInputEditorWindow> controllerConfigWindow = nullptr;
    if (controllerConfigWindow == nullptr) {
        controllerConfigWindow = std::dynamic_pointer_cast<SohInputEditorWindow>(
            Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Controller Configuration"));
    } else if (controllerConfigWindow->TestingRumble()) {
        return;
    }

    if (rumble) {
        Ship::Context::GetInstance()->GetControlDeck()->GetControllerByPort(0)->GetRumble()->StartRumble();
    } else {
        Ship::Context::GetInstance()->GetControlDeck()->GetControllerByPort(0)->GetRumble()->StopRumble();
    }
}

extern "C" float OTRGetAspectRatio() {
    return Ship::Context::GetInstance()->GetWindow()->GetAspectRatio();
}

extern "C" float OTRGetDimensionFromLeftEdge(float v) {
    return (SCREEN_WIDTH / 2 - SCREEN_HEIGHT / 2 * OTRGetAspectRatio() + (v));
}

extern "C" float OTRGetDimensionFromRightEdge(float v) {
    return (SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 * OTRGetAspectRatio() - (SCREEN_WIDTH - v));
}

// Gets the width of the current render target area
extern "C" uint32_t OTRGetGameRenderWidth() {
    auto fastWnd = dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());
    auto intP = fastWnd->GetInterpreterWeak().lock();

    if (!intP) {
        assert(false && "Lost reference to Fast::Interpreter");
        return 320;
    }

    uint32_t height, width;
    intP->GetCurDimensions(&width, &height);

    return width;
}

// Gets the height of the current render target area
extern "C" uint32_t OTRGetGameRenderHeight() {
    auto fastWnd = dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());
    auto intP = fastWnd->GetInterpreterWeak().lock();

    if (!intP) {
        assert(false && "Lost reference to Fast::Interpreter");
        return 240;
    }

    uint32_t height, width;
    intP->GetCurDimensions(&width, &height);

    return height;
}

f32 floorf(f32 x); // RANDOTODO False positive error "allowing all exceptions is incompatible with previous function"
f32 ceilf(f32 x);  // This gets annoying

extern "C" int16_t OTRGetRectDimensionFromLeftEdge(float v) {
    return ((int)floorf(OTRGetDimensionFromLeftEdge(v)));
}

extern "C" int16_t OTRGetRectDimensionFromRightEdge(float v) {
    return ((int)ceilf(OTRGetDimensionFromRightEdge(v)));
}

extern "C" int AudioPlayer_Buffered(void) {
    return AudioPlayerBuffered();
}

extern "C" int AudioPlayer_GetDesiredBuffered(void) {
    return AudioPlayerGetDesiredBuffered();
}

extern "C" void AudioPlayer_Play(const uint8_t* buf, uint32_t len) {
    AudioPlayerPlayFrame(buf, len);
}

extern "C" int Controller_ShouldRumble(size_t slot) {
    // don't rumble if we don't have rumble mappings
    if (Ship::Context::GetInstance()
            ->GetControlDeck()
            ->GetControllerByPort(static_cast<uint8_t>(slot))
            ->GetRumble()
            ->GetAllRumbleMappings()
            .empty()) {
        return 0;
    }

    // don't rumble if we don't have connected gamepads
    if (Ship::Context::GetInstance()
            ->GetControlDeck()
            ->GetConnectedPhysicalDeviceManager()
            ->GetConnectedSDLGamepadsForPort(slot)
            .empty()) {
        return 0;
    }

    // rumble
    return 1;
}

extern "C" size_t GetEquipNowMessage(char* buffer, char* src, const size_t maxBufferSize) {
    CustomMessage customMessage("\x04\x1A\x08"
                                "Would you like to equip it now?"
                                "\x09&&"
                                "\x1B%g"
                                "Yes"
                                "&"
                                "No"
                                "%w\x02",
                                "\x04\x1A\x08"
                                "M"
                                "\x9A"
                                "chtest Du es jetzt ausr\x9Esten?"
                                "\x09&&"
                                "\x1B%g"
                                "Ja!"
                                "&"
                                "Nein!"
                                "%w\x02",
                                "\x04\x1A\x08"
                                "D\x96sirez-vous l'\x96quiper maintenant?"
                                "\x09&&"
                                "\x1B%g"
                                "Oui"
                                "&"
                                "Non"
                                "%w\x02");
    customMessage.Format();

    std::string postfix = customMessage.GetForCurrentLanguage();
    std::string str;
    std::string FixedBaseStr(src);
    int RemoveControlChar = FixedBaseStr.find_first_of("\x02");

    if (RemoveControlChar != std::string::npos) {
        FixedBaseStr = FixedBaseStr.substr(0, RemoveControlChar);
    }
    str = FixedBaseStr + postfix;

    if (!str.empty()) {
        memset(buffer, 0, maxBufferSize);
        const size_t copiedCharLen = std::min<size_t>(maxBufferSize - 1, str.length());
        memcpy(buffer, str.c_str(), copiedCharLen);
        return copiedCharLen;
    }
    return 0;
}

extern "C" void Randomizer_ParseSpoiler(const char* fileLoc) {
    OTRGlobals::Instance->gRandoContext->ParseSpoiler(fileLoc);
}

extern "C" void Randomizer_LoadHintMessages() {
    OTRGlobals::Instance->gRandomizer->LoadHintMessages();
}

extern "C" void Randomizer_LoadMerchantMessages() {
    OTRGlobals::Instance->gRandomizer->LoadMerchantMessages();
}

extern "C" bool Randomizer_IsTrialRequired(s32 trialFlag) {
    return OTRGlobals::Instance->gRandomizer->IsTrialRequired(trialFlag);
}

extern "C" u32 SpoilerFileExists(const char* spoilerFileName) {
    return OTRGlobals::Instance->gRandomizer->SpoilerFileExists(spoilerFileName);
}

extern "C" u8 Randomizer_GetSettingValue(RandomizerSettingKey randoSettingKey) {
    return OTRGlobals::Instance->gRandoContext->GetOption(randoSettingKey).Get();
}

extern "C" RandomizerCheck Randomizer_GetCheckFromActor(s16 actorId, s16 sceneNum, s16 actorParams) {
    return OTRGlobals::Instance->gRandomizer->GetCheckFromActor(actorId, sceneNum, actorParams);
}

extern "C" ScrubIdentity Randomizer_IdentifyScrub(s32 sceneNum, s32 actorParams, s32 respawnData) {
    return OTRGlobals::Instance->gRandomizer->IdentifyScrub(sceneNum, actorParams, respawnData);
}

extern "C" BeehiveIdentity Randomizer_IdentifyBeehive(s32 sceneNum, s16 xPosition, s32 respawnData) {
    return OTRGlobals::Instance->gRandomizer->IdentifyBeehive(sceneNum, xPosition, respawnData);
}

extern "C" ShopItemIdentity Randomizer_IdentifyShopItem(s32 sceneNum, u8 slotIndex) {
    return OTRGlobals::Instance->gRandomizer->IdentifyShopItem(sceneNum, slotIndex);
}

extern "C" CowIdentity Randomizer_IdentifyCow(s32 sceneNum, s32 posX, s32 posZ) {
    return OTRGlobals::Instance->gRandomizer->IdentifyCow(sceneNum, posX, posZ);
}

extern "C" FishIdentity Randomizer_IdentifyFish(s32 sceneNum, s32 actorParams) {
    return OTRGlobals::Instance->gRandomizer->IdentifyFish(sceneNum, actorParams);
}

extern "C" GetItemEntry ItemTable_Retrieve(int16_t getItemID) {
    GetItemEntry giEntry = ItemTableManager::Instance->RetrieveItemEntry(MOD_NONE, getItemID);
    return giEntry;
}

extern "C" GetItemEntry ItemTable_RetrieveEntry(s16 tableID, s16 getItemID) {
    if (tableID == MOD_RANDOMIZER) {
        return Rando::StaticData::RetrieveItem(static_cast<RandomizerGet>(getItemID)).GetGIEntry_Copy();
    }
    return ItemTableManager::Instance->RetrieveItemEntry(tableID, getItemID);
}

extern "C" GetItemEntry Randomizer_GetItemFromActor(s16 actorId, s16 sceneNum, s16 actorParams, GetItemID ogId) {
    return OTRGlobals::Instance->gRandomizer->GetItemFromActor(actorId, sceneNum, actorParams, ogId);
}

extern "C" GetItemEntry Randomizer_GetItemFromActorWithoutObtainabilityCheck(s16 actorId, s16 sceneNum, s16 actorParams,
                                                                             GetItemID ogId) {
    return OTRGlobals::Instance->gRandomizer->GetItemFromActor(actorId, sceneNum, actorParams, ogId, false);
}

extern "C" GetItemEntry Randomizer_GetItemFromKnownCheck(RandomizerCheck randomizerCheck, GetItemID ogId) {
    return OTRGlobals::Instance->gRandomizer->GetItemFromKnownCheck(randomizerCheck, ogId);
}

extern "C" GetItemEntry Randomizer_GetItemFromKnownCheckWithoutObtainabilityCheck(RandomizerCheck randomizerCheck,
                                                                                  GetItemID ogId) {
    return OTRGlobals::Instance->gRandomizer->GetItemFromKnownCheck(randomizerCheck, ogId, false);
}

extern "C" RandomizerInf Randomizer_GetRandomizerInfFromCheck(RandomizerCheck randomizerCheck) {
    return OTRGlobals::Instance->gRandomizer->GetRandomizerInfFromCheck(randomizerCheck);
}

extern "C" ItemObtainability Randomizer_GetItemObtainabilityFromRandomizerCheck(RandomizerCheck randomizerCheck) {
    return OTRGlobals::Instance->gRandomizer->GetItemObtainabilityFromRandomizerCheck(randomizerCheck);
}

extern "C" bool Randomizer_IsCheckShuffled(RandomizerCheck rc) {
    return CheckTracker::IsCheckShuffled(rc);
}

extern "C" GetItemEntry GetItemMystery() {
    return GET_ITEM_MYSTERY;
}

extern "C" uint8_t Randomizer_IsSeedGenerated() {
    return OTRGlobals::Instance->gRandoContext->IsSeedGenerated() ? 1 : 0;
}

extern "C" void Randomizer_SetSeedGenerated(bool seedGenerated) {
    OTRGlobals::Instance->gRandoContext->SetSeedGenerated(seedGenerated);
}

extern "C" uint8_t Randomizer_IsSpoilerLoaded() {
    return OTRGlobals::Instance->gRandoContext->IsSpoilerLoaded() ? 1 : 0;
}

extern "C" void Randomizer_SetSpoilerLoaded(bool spoilerLoaded) {
    OTRGlobals::Instance->gRandoContext->SetSpoilerLoaded(spoilerLoaded);
}

extern "C" uint8_t Randomizer_GenerateRandomizer() {
    return GenerateRandomizer() ? 1 : 0;
}

extern "C" void Randomizer_ShowRandomizerMenu() {
    SohGui::ShowRandomizerSettingsMenu();
}

CustomMessage Randomizer_GetCustomGetItemMessage(Player* player) {
    s16 giid;
    if (player->getItemEntry.objectId != OBJECT_INVALID) {
        giid = player->getItemEntry.getItemId;
    } else {
        giid = player->getItemId;
    }
    const CustomMessage getItemText =
        CustomMessageManager::Instance->RetrieveMessage(Randomizer::getItemMessageTableID, giid);
    return getItemText;
}

extern "C" int CustomMessage_RetrieveIfExists(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    uint16_t textId = msgCtx->textId;
    Font* font = &msgCtx->font;
    char* buffer = font->msgBuf;
    const size_t maxBufferSize = sizeof(font->msgBuf);
    CustomMessage messageEntry;
    s16 actorParams = 0;
    if (IS_RANDO) {
        auto ctx = Rando::Context::GetInstance();
        bool nonBeanMerchants = ctx->GetOption(RSK_SHUFFLE_MERCHANTS).Is(RO_SHUFFLE_MERCHANTS_ALL_BUT_BEANS) ||
                                ctx->GetOption(RSK_SHUFFLE_MERCHANTS).Is(RO_SHUFFLE_MERCHANTS_ALL);
        Player* player = GET_PLAYER(play);
        if (textId == TEXT_RANDOMIZER_CUSTOM_ITEM) {
            if (player->getItemEntry.getItemId == RG_ICE_TRAP) {
                messageEntry = Randomizer::GetIceTrapMessage();
            } else if (player->getItemEntry.getItemId == RG_TRIFORCE_PIECE) {
                messageEntry = Randomizer::GetTriforcePieceMessage();
            } else {
                messageEntry = Randomizer_GetCustomGetItemMessage(player);
            }
        } else if (textId == TEXT_ITEM_DUNGEON_MAP || textId == TEXT_ITEM_COMPASS) {
            if (DUNGEON_ITEMS_CAN_BE_OUTSIDE_DUNGEON(RSK_SHUFFLE_MAPANDCOMPASS)) {
                if (textId == TEXT_ITEM_DUNGEON_MAP) {
                    messageEntry =
                        OTRGlobals::Instance->gRandomizer->GetMapGetItemMessageWithHint(player->getItemEntry);
                } else {
                    messageEntry = Randomizer_GetCustomGetItemMessage(player);
                }
            }
        } else if (textId == TEXT_ITEM_KEY_BOSS) {
            if (player->getItemEntry.getItemId == RG_GANONS_CASTLE_BOSS_KEY) {
                if (DUNGEON_ITEMS_CAN_BE_OUTSIDE_DUNGEON(RSK_GANONS_BOSS_KEY)) {
                    messageEntry = Randomizer_GetCustomGetItemMessage(player);
                }
            } else {
                if (DUNGEON_ITEMS_CAN_BE_OUTSIDE_DUNGEON(RSK_BOSS_KEYSANITY)) {
                    messageEntry = Randomizer_GetCustomGetItemMessage(player);
                }
            }
        } else if (textId == TEXT_ITEM_KEY_SMALL) {
            if (player->getItemEntry.getItemId == RG_GERUDO_FORTRESS_SMALL_KEY) {
                if (Randomizer_GetSettingValue(RSK_GERUDO_KEYS) != RO_GERUDO_KEYS_VANILLA) {
                    messageEntry = Randomizer_GetCustomGetItemMessage(player);
                }
            } else {
                if (DUNGEON_ITEMS_CAN_BE_OUTSIDE_DUNGEON(RSK_KEYSANITY)) {
                    messageEntry = Randomizer_GetCustomGetItemMessage(player);
                }
            }
        } else if (textId == TEXT_RANDOMIZER_GOSSIP_STONE_HINTS &&
                   Randomizer_GetSettingValue(RSK_GOSSIP_STONE_HINTS) != RO_GOSSIP_STONES_NONE &&
                   (Randomizer_GetSettingValue(RSK_GOSSIP_STONE_HINTS) == RO_GOSSIP_STONES_NEED_NOTHING ||
                    (Randomizer_GetSettingValue(RSK_GOSSIP_STONE_HINTS) == RO_GOSSIP_STONES_NEED_TRUTH &&
                     Player_GetMask(play) == PLAYER_MASK_TRUTH) ||
                    (Randomizer_GetSettingValue(RSK_GOSSIP_STONE_HINTS) == RO_GOSSIP_STONES_NEED_STONE &&
                     CHECK_QUEST_ITEM(QUEST_STONE_OF_AGONY)))) {

            Actor* stone = player->talkActor;
            RandomizerHint stoneHint = RH_NONE;
            s16 hintParams = stone->params & 0xFF;

            if (Rando::StaticData::stoneParamsToHint.contains(hintParams)) {
                stoneHint = Rando::StaticData::stoneParamsToHint[hintParams];
            } else if (hintParams == 0x18) {
                // look for the chest in the actorlist to determine
                // which grotto we're in
                int numOfActorLists = sizeof(play->actorCtx.actorLists) / sizeof(play->actorCtx.actorLists[0]);
                for (int i = 0; i < numOfActorLists; i++) {
                    if (play->actorCtx.actorLists[i].length) {
                        if (play->actorCtx.actorLists[i].head->id == 10 &&
                            Rando::StaticData::grottoChestParamsToHint.contains(
                                play->actorCtx.actorLists[i].head->params)) {
                            // use the chest params to find the stone hint
                            stoneHint =
                                Rando::StaticData::grottoChestParamsToHint[play->actorCtx.actorLists[i].head->params];
                        }
                    }
                }
            }
            if (stoneHint == RH_NONE) {
                messageEntry = CustomMessage("INVALID STONE. PARAMS: " + std::to_string(hintParams));
            } else {
                messageEntry = ctx->GetHint(stoneHint)->GetHintMessage(MF_AUTO_FORMAT);
            }
        } else if ((textId == TEXT_ALTAR_CHILD || textId == TEXT_ALTAR_ADULT)) {
            // rando hints at altar
            messageEntry = (LINK_IS_ADULT) ? ctx->GetHint(RH_ALTAR_ADULT)->GetHintMessage()
                                           : ctx->GetHint(RH_ALTAR_CHILD)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (textId == TEXT_GANONDORF) {
            if (ctx->GetOption(RSK_GANONDORF_HINT)) {
                if (ctx->GetOption(RSK_SHUFFLE_MASTER_SWORD) &&
                    !CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER)) {
                    messageEntry = INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT
                                       ? ctx->GetHint(RH_GANONDORF_HINT)->GetHintMessage(MF_AUTO_FORMAT, 1)
                                       : ctx->GetHint(RH_GANONDORF_HINT)->GetHintMessage(MF_AUTO_FORMAT, 2);
                } else {
                    messageEntry = INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT
                                       ? ctx->GetHint(RH_GANONDORF_JOKE)->GetHintMessage(MF_AUTO_FORMAT)
                                       : ctx->GetHint(RH_GANONDORF_HINT)->GetHintMessage(MF_AUTO_FORMAT, 0);
                }
            }
        } else if (textId == TEXT_SHEIK_NEED_HOOK || textId == TEXT_SHEIK_HAVE_HOOK) {
            messageEntry = OTRGlobals::Instance->gRandomizer->GetSheikMessage(gPlayState->sceneNum, textId);
            // Shop items each have two message entries, second one offset by NUM_SHOP_ITEMS
            // textId: TEXT_SHOP_ITEM_RANDOM + (randomizerInf - RAND_INF_SHOP_ITEMS_KF_SHOP_ITEM_1)
            // textId: TEXT_SHOP_ITEM_RANDOM + ((randomizerInf - RAND_INF_SHOP_ITEMS_KF_SHOP_ITEM_1) + NUM_SHOP_ITEMS)
        } else if (textId >= TEXT_SHOP_ITEM_RANDOM && textId < TEXT_SHOP_ITEM_RANDOM_CONFIRM) {
            RandomizerCheck rc = OTRGlobals::Instance->gRandomizer->GetCheckFromRandomizerInf(
                (RandomizerInf)((textId - TEXT_SHOP_ITEM_RANDOM) + RAND_INF_SHOP_ITEMS_KF_SHOP_ITEM_1));
            messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(rc, TEXT_SHOP_ITEM_RANDOM);
        } else if (textId >= TEXT_SHOP_ITEM_RANDOM_CONFIRM && textId <= TEXT_SHOP_ITEM_RANDOM_CONFIRM_END) {
            RandomizerCheck rc = OTRGlobals::Instance->gRandomizer->GetCheckFromRandomizerInf(
                (RandomizerInf)((textId - TEXT_SHOP_ITEM_RANDOM_CONFIRM) + RAND_INF_SHOP_ITEMS_KF_SHOP_ITEM_1));
            messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(rc, TEXT_SHOP_ITEM_RANDOM_CONFIRM);
        } else if (textId == TEXT_SCRUB_RANDOM) {
            EnDns* enDns = (EnDns*)player->talkActor;
            RandomizerCheck rc = OTRGlobals::Instance->gRandomizer->GetCheckFromRandomizerInf(
                (RandomizerInf)enDns->sohScrubIdentity.randomizerInf);
            messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(
                rc, TEXT_SCRUB_RANDOM, TEXT_SCRUB_RANDOM_FREE,
                Randomizer_GetSettingValue(RSK_SCRUB_TEXT_HINT) == RO_GENERIC_OFF);
        } else if (CVarGetInteger(CVAR_RANDOMIZER_ENHANCEMENT("RandomizeRupeeNames"), 1) &&
                   (textId == TEXT_BLUE_RUPEE || textId == TEXT_RED_RUPEE || textId == TEXT_PURPLE_RUPEE ||
                    textId == TEXT_HUGE_RUPEE)) {
            messageEntry = Randomizer::GetRupeeMessage(textId);
            // In rando, replace Navi's general overworld hints with rando-related gameplay tips
        } else if (CVarGetInteger(CVAR_RANDOMIZER_ENHANCEMENT("RandoRelevantNavi"), 1) &&
                   textId >= TEXT_NAVI_DEKU_TREE_SUMMONS && textId <= TEXT_NAVI_TRY_TO_KEEP_MOVING) {
            u16 naviTextId = Random(0, NUM_NAVI_MESSAGES);
            messageEntry = CustomMessageManager::Instance->RetrieveMessage(Randomizer::NaviRandoMessageTableID,
                                                                           naviTextId, MF_FORMATTED);
        } else if (textId == TEXT_BEAN_SALESMAN_BUY_FOR_10 &&
                   (ctx->GetOption(RSK_SHUFFLE_MERCHANTS).Is(RO_SHUFFLE_MERCHANTS_BEANS_ONLY) ||
                    ctx->GetOption(RSK_SHUFFLE_MERCHANTS).Is(RO_SHUFFLE_MERCHANTS_ALL))) {
            messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(
                RC_ZR_MAGIC_BEAN_SALESMAN, TEXT_BEAN_SALESMAN_BUY_FOR_10, TEXT_NONE,
                Randomizer_GetSettingValue(RSK_MERCHANT_TEXT_HINT) == RO_GENERIC_OFF);
        } else if (textId == TEXT_BEAN_SALESMAN_BUY_FOR_100) {
            messageEntry = CustomMessageManager::Instance->RetrieveMessage(
                Randomizer::merchantMessageTableID, TEXT_BEAN_SALESMAN_BUY_FOR_100, MF_AUTO_FORMAT);
        } else if (textId == TEXT_GRANNYS_SHOP && !Flags_GetRandomizerInf(RAND_INF_MERCHANTS_GRANNYS_SHOP) &&
                   nonBeanMerchants &&
                   (ctx->GetOption(RSK_SHUFFLE_ADULT_TRADE) || INV_CONTENT(ITEM_CLAIM_CHECK) == ITEM_CLAIM_CHECK)) {
            messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(
                RC_KAK_GRANNYS_SHOP, TEXT_GRANNYS_SHOP, TEXT_NONE,
                Randomizer_GetSettingValue(RSK_MERCHANT_TEXT_HINT) == RO_GENERIC_OFF);
        } else if (textId == TEXT_MEDIGORON && nonBeanMerchants) {
            messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(
                RC_GC_MEDIGORON, TEXT_MEDIGORON, TEXT_NONE,
                Randomizer_GetSettingValue(RSK_MERCHANT_TEXT_HINT) == RO_GENERIC_OFF);
        } else if (textId == TEXT_CARPET_SALESMAN_1 && !Flags_GetRandomizerInf(RAND_INF_MERCHANTS_CARPET_SALESMAN) &&
                   nonBeanMerchants) {
            if (Randomizer_GetSettingValue(RSK_MERCHANT_TEXT_HINT)) {
                messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(
                    RC_WASTELAND_BOMBCHU_SALESMAN, TEXT_CARPET_SALESMAN_1, TEXT_NONE,
                    Randomizer_GetSettingValue(RSK_MERCHANT_TEXT_HINT) == RO_GENERIC_OFF);
            } else {
                messageEntry = OTRGlobals::Instance->gRandomizer->GetMerchantMessage(
                    RC_WASTELAND_BOMBCHU_SALESMAN, TEXT_CARPET_SALESMAN_MYSTERIOUS, TEXT_NONE,
                    Randomizer_GetSettingValue(RSK_MERCHANT_TEXT_HINT) == RO_GENERIC_OFF);
            }
        } else if (textId == TEXT_CARPET_SALESMAN_ARMS_DEALER) {
            messageEntry = CustomMessageManager::Instance->RetrieveMessage(Randomizer::merchantMessageTableID, textId,
                                                                           MF_AUTO_FORMAT);
        } else if (textId == TEXT_SKULLTULA_PEOPLE_IM_CURSED) {
            actorParams = player->talkActor->params;
            if (actorParams == 1 && ctx->GetOption(RSK_KAK_10_SKULLS_HINT)) {
                messageEntry = ctx->GetHint(RH_KAK_10_SKULLS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
            } else if (actorParams == 2 && ctx->GetOption(RSK_KAK_20_SKULLS_HINT)) {
                messageEntry = ctx->GetHint(RH_KAK_20_SKULLS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
            } else if (actorParams == 3 && ctx->GetOption(RSK_KAK_30_SKULLS_HINT)) {
                messageEntry = ctx->GetHint(RH_KAK_30_SKULLS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
            } else if (actorParams == 4 && ctx->GetOption(RSK_KAK_40_SKULLS_HINT)) {
                messageEntry = ctx->GetHint(RH_KAK_40_SKULLS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
            } else if (ctx->GetOption(RSK_KAK_50_SKULLS_HINT)) {
                messageEntry = ctx->GetHint(RH_KAK_50_SKULLS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
            }
        } else if (textId == TEXT_DAMPES_DIARY && ctx->GetOption(RSK_DAMPES_DIARY_HINT)) {
            messageEntry = ctx->GetHint(RH_DAMPES_DIARY)->GetHintMessage(MF_AUTO_FORMAT);
        } else if ((textId == TEXT_CHEST_GAME_PROCEED || textId == TEXT_CHEST_GAME_REAL_GAMBLER ||
                    textId == TEXT_CHEST_GAME_THANKS_A_LOT) &&
                   play->sceneNum == SCENE_TREASURE_BOX_SHOP && ctx->GetOption(RSK_GREG_HINT)) {
            messageEntry = ctx->GetHint(RH_GREG_RUPEE)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (textId == TEXT_WARP_MINUET_OF_FOREST && ctx->GetOption(RSK_WARP_SONG_HINTS)) {
            messageEntry = ctx->GetHint(RH_MINUET_WARP_LOC)->GetHintMessage(MF_FORMATTED);
        } else if (textId == TEXT_WARP_BOLERO_OF_FIRE && ctx->GetOption(RSK_WARP_SONG_HINTS)) {
            messageEntry = ctx->GetHint(RH_BOLERO_WARP_LOC)->GetHintMessage(MF_FORMATTED);
        } else if (textId == TEXT_WARP_SERENADE_OF_WATER && ctx->GetOption(RSK_WARP_SONG_HINTS)) {
            messageEntry = ctx->GetHint(RH_SERENADE_WARP_LOC)->GetHintMessage(MF_FORMATTED);
        } else if (textId == TEXT_WARP_REQUIEM_OF_SPIRIT && ctx->GetOption(RSK_WARP_SONG_HINTS)) {
            messageEntry = ctx->GetHint(RH_REQUIEM_WARP_LOC)->GetHintMessage(MF_FORMATTED);
        } else if (textId == TEXT_WARP_NOCTURNE_OF_SHADOW && ctx->GetOption(RSK_WARP_SONG_HINTS)) {
            messageEntry = ctx->GetHint(RH_NOCTURNE_WARP_LOC)->GetHintMessage(MF_FORMATTED);
        } else if (textId == TEXT_WARP_PRELUDE_OF_LIGHT && ctx->GetOption(RSK_WARP_SONG_HINTS)) {
            messageEntry = ctx->GetHint(RH_PRELUDE_WARP_LOC)->GetHintMessage(MF_FORMATTED);
        } else if (textId >= TEXT_WARP_MINUET_OF_FOREST && textId <= TEXT_WARP_PRELUDE_OF_LIGHT &&
                   ctx->GetOption(RSK_SHUFFLE_WARP_SONGS)) {
            messageEntry = CustomMessageManager::Instance->RetrieveMessage(Randomizer::hintMessageTableID,
                                                                           TEXT_WARP_MINUET_OF_FOREST, MF_FORMATTED);
        } else if (textId == TEXT_LAKE_HYLIA_WATER_SWITCH_NAVI || textId == TEXT_LAKE_HYLIA_WATER_SWITCH_SIGN) {
            messageEntry =
                CustomMessageManager::Instance->RetrieveMessage(Randomizer::hintMessageTableID, textId, MF_AUTO_FORMAT);
        } else if (textId == TEXT_SHOOTING_GALLERY_MAN_COME_BACK_WITH_BOW) {
            messageEntry = CustomMessageManager::Instance->RetrieveMessage(
                Randomizer::hintMessageTableID, TEXT_SHOOTING_GALLERY_MAN_COME_BACK_WITH_BOW, MF_AUTO_FORMAT);
        } else if (textId == TEXT_FIRE_TEMPLE_GORON_OWE_YOU_BIG_TIME ||
                   (textId >= TEXT_FIRE_TEMPLE_GORON_FALLING_DOORS_SECRET &&
                    textId <= TEXT_FIRE_TEMPLE_GORON_SOUNDS_DIFFERENT_SECRET)) {
            u16 choice = Random(0, NUM_GORON_MESSAGES);
            messageEntry = OTRGlobals::Instance->gRandomizer->GetGoronMessage(choice);
        } else if (textId == TEXT_FROGS_UNDERWATER && ctx->GetOption(RSK_FROGS_HINT)) {
            messageEntry = ctx->GetHint(RH_FROGS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (Randomizer_GetSettingValue(RSK_LOACH_HINT) &&
                   (textId == TEXT_FISHING_CLOUDY || textId == TEXT_FISHING_TRY_ANOTHER_LURE ||
                    textId == TEXT_FISHING_SECRETS || textId == TEXT_FISHING_GOOD_FISHERMAN ||
                    textId == TEXT_FISHING_DIFFERENT_POND || textId == TEXT_FISHING_SCRATCHING ||
                    textId == TEXT_FISHING_TRY_ANOTHER_LURE_WITH_SINKING_LURE)) {
            messageEntry = ctx->GetHint(RH_LOACH_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if ((textId == TEXT_FISHING_POND_START || textId == TEXT_FISHING_POND_START_MET) &&
                   ctx->GetOption(RSK_SHUFFLE_FISHING_POLE) && !Flags_GetRandomizerInf(RAND_INF_FISHING_POLE_FOUND)) {
            messageEntry = OTRGlobals::Instance->gRandomizer->GetFishingPondOwnerMessage(textId);
        } else if (textId == TEXT_SARIA_SFM && gPlayState->sceneNum == SCENE_SACRED_FOREST_MEADOW &&
                   ctx->GetOption(RSK_SARIA_HINT)) {
            messageEntry = ctx->GetHint(RH_SARIA_HINT)->GetHintMessage(MF_AUTO_FORMAT, 0);
        } else if ((textId >= TEXT_SARIAS_SONG_FACE_TO_FACE && textId <= TEXT_SARIAS_SONG_CHANNELING_POWER) &&
                   ctx->GetOption(RSK_SARIA_HINT)) {
            messageEntry = ctx->GetHint(RH_SARIA_HINT)->GetHintMessage(MF_AUTO_FORMAT, 1);
        } else if (ctx->GetOption(RSK_BIGGORON_HINT) &&
                   (textId == TEXT_BIGGORON_BETTER_AT_SMITHING || textId == TEXT_BIGGORON_WAITING_FOR_YOU ||
                    textId == TEXT_BIGGORON_RETURN_AFTER_A_FEW_DAYS || textId == TEXT_BIGGORON_I_MAAAADE_THISSSS)) {
            messageEntry = ctx->GetHint(RH_BIGGORON_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (ctx->GetOption(RSK_BIG_POES_HINT) &&
                   (textId == TEXT_GHOST_SHOP_EXPLAINATION || textId == TEXT_GHOST_SHOP_CARD_HAS_POINTS)) {
            messageEntry = ctx->GetHint(RH_BIG_POES_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (ctx->GetOption(RSK_CHICKENS_HINT) &&
                   (textId >= TEXT_ANJU_PLEASE_BRING_MY_CUCCOS_BACK && textId <= TEXT_ANJU_PLEASE_BRING_1_CUCCO)) {
            messageEntry = ctx->GetHint(RH_CHICKENS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if ((textId == TEXT_MALON_EVERYONE_TURNING_EVIL || textId == TEXT_MALON_I_SING_THIS_SONG) &&
                   ctx->GetOption(RSK_MALON_HINT)) {
            messageEntry = ctx->GetHint(RH_MALON_HINT)->GetHintMessage(MF_AUTO_FORMAT, 0);
        } else if (textId == TEXT_MALON_HOW_IS_EPONA_DOING && ctx->GetOption(RSK_MALON_HINT)) {
            messageEntry = ctx->GetHint(RH_MALON_HINT)->GetHintMessage(MF_AUTO_FORMAT, 1);
        } else if (textId == TEXT_MALON_OBSTICLE_COURSE && ctx->GetOption(RSK_MALON_HINT)) {
            messageEntry = ctx->GetHint(RH_MALON_HINT)->GetHintMessage(MF_AUTO_FORMAT, 2);
        } else if (textId == TEXT_MALON_INGO_MUST_HAVE_BEEN_TEMPTED && ctx->GetOption(RSK_MALON_HINT)) {
            messageEntry = ctx->GetHint(RH_MALON_HINT)->GetHintMessage(MF_AUTO_FORMAT, 3);
        } else if (ctx->GetOption(RSK_KAK_100_SKULLS_HINT) && textId == TEXT_SKULLTULA_PEOPLE_MAKE_YOU_VERY_RICH) {
            messageEntry = ctx->GetHint(RH_KAK_100_SKULLS_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (textId == TEXT_GF_HBA_SIGN && ctx->GetOption(RSK_HBA_HINT)) {
            messageEntry = ctx->GetHint(RH_HBA_HINT)->GetHintMessage(MF_AUTO_FORMAT, 0);
        } else if (textId == TEXT_HBA_NOT_ON_HORSE && ctx->GetOption(RSK_HBA_HINT)) {
            messageEntry = ctx->GetHint(RH_HBA_HINT)->GetHintMessage(MF_AUTO_FORMAT, 1);
        } else if (textId == TEXT_HBA_INITIAL_EXPLAINATION && ctx->GetOption(RSK_HBA_HINT)) {
            messageEntry = ctx->GetHint(RH_HBA_HINT)->GetHintMessage(MF_AUTO_FORMAT, 2);
        } else if (textId == TEXT_HBA_ALREADY_HAVE_1000 && ctx->GetOption(RSK_HBA_HINT)) {
            messageEntry = ctx->GetHint(RH_HBA_HINT)->GetHintMessage(MF_AUTO_FORMAT, 3);
        } else if (textId == TEXT_CARPET_SALESMAN_CUSTOM_FAIL_TO_BUY) {
            messageEntry =
                CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, textId, MF_AUTO_FORMAT);
        } else if (textId == TEXT_MASK_SHOP_SIGN && ctx->GetOption(RSK_MASK_SHOP_HINT)) {
            messageEntry = ctx->GetHint(RH_MASK_SHOP_HINT)->GetHintMessage(MF_AUTO_FORMAT);
        } else if (textId == TEXT_BIG_POE_COLLECTED_RANDO) {
            messageEntry =
                CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, textId, MF_AUTO_FORMAT);
        } else if (textId == TEXT_GERUDO_GUARD_FRIENDLY && player->talkActor->id == ACTOR_EN_GE2) {
            // TODO_TRANSLATE Translate into french and german
            messageEntry = CustomMessage("Want me to throw you in jail?&\x1B#Yes please&No thanks#", { QM_GREEN });
            messageEntry.AutoFormat();
        }
    }
    if (textId == TEXT_GS_NO_FREEZE || textId == TEXT_GS_FREEZE) {
        if (CVarGetInteger(CVAR_ENHANCEMENT("InjectItemCounts.GoldSkulltula"), 0) != 0) {
            // The freeze text cannot be manually dismissed and must be auto-dismissed.
            // This is fine and even wanted when skull tokens are not shuffled, but when
            // when they are shuffled we want to be able to manually dismiss the box.
            // Otherwise if we get a token from a chest or an NPC we get stuck in the ItemGet
            // animation until the text box auto-dismisses.
            // RANDOTODO: Implement a way to determine if an item came from a skulltula and
            // inject the auto-dismiss control code if it did.
            bool gsTokensShuffled = Randomizer_GetSettingValue(RSK_SHUFFLE_TOKENS) != RO_TOKENSANITY_OFF;
            if (CVarGetInteger(CVAR_ENHANCEMENT("SkulltulaFreeze"), 0) != 0 && !(IS_RANDO && gsTokensShuffled)) {
                textId = TEXT_GS_NO_FREEZE;
            } else {
                textId = TEXT_GS_FREEZE;
            }
            // In vanilla, GS token count is incremented prior to the text box displaying
            // In rando we need to bump the token count by one to show the correct count
            s16 gsCount = gSaveContext.inventory.gsTokens + ((IS_RANDO && gsTokensShuffled) ? 1 : 0);
            messageEntry = CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, textId, MF_FORMATTED);
            messageEntry.Replace("[[gsCount]]", std::to_string(gsCount));
        } else if (CVarGetInteger(CVAR_ENHANCEMENT("SkulltulaFreeze"), 0) != 0 &&
                   (!IS_RANDO || Randomizer_GetSettingValue(RSK_SHUFFLE_TOKENS) == RO_TOKENSANITY_OFF)) {
            messageEntry = CustomMessage::LoadVanillaMessageTableEntry(TEXT_GS_FREEZE);
            messageEntry.Replace(CustomMessage::MESSAGE_END(), "\x0E\x3C");
            messageEntry += CustomMessage::MESSAGE_END();
        }
    } else if ((IS_RANDO || CVarGetInteger(CVAR_ENHANCEMENT("BetterBombchuShopping"), 0)) &&
               (textId == TEXT_BUY_BOMBCHUS_10_DESC || textId == TEXT_BUY_BOMBCHUS_10_PROMPT)) {
        messageEntry = CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, textId, MF_FORMATTED);
    } else if (textId == TEXT_HEART_CONTAINER &&
               CVarGetInteger(CVAR_ENHANCEMENT("InjectItemCounts.HeartContainer"), 0)) {
        messageEntry =
            CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, TEXT_HEART_CONTAINER, MF_FORMATTED);
        messageEntry.Replace("[[heartContainerCount]]", std::to_string(gSaveContext.ship.stats.heartContainers + 1));
    } else if (textId >= TEXT_HEART_PIECE && textId < TEXT_HEART_CONTAINER &&
               CVarGetInteger(CVAR_ENHANCEMENT("InjectItemCounts.HeartPiece"), 0)) {
        messageEntry =
            CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, TEXT_HEART_PIECE, MF_FORMATTED);
        messageEntry.Replace("[[heartPieceCount]]", std::to_string(gSaveContext.ship.stats.heartPieces + 1));
    } else if (textId == TEXT_MARKET_GUARD_NIGHT && CVarGetInteger(CVAR_ENHANCEMENT("MarketSneak"), 0) &&
               play->sceneNum == SCENE_MARKET_ENTRANCE_NIGHT) {
        messageEntry = CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, TEXT_MARKET_GUARD_NIGHT,
                                                                       MF_FORMATTED);
    }
    if (textId == TEXT_FISHERMAN_LEAVE && CVarGetInteger(CVAR_ENHANCEMENT("QuitFishingAtDoor"), 0)) {
        messageEntry =
            CustomMessageManager::Instance->RetrieveMessage(customMessageTableID, TEXT_FISHERMAN_LEAVE, MF_FORMATTED);
    }
    font->charTexBuf[0] = (messageEntry.GetTextBoxType() << 4) | messageEntry.GetTextBoxPosition();
    switch (gSaveContext.language) {
        case LANGUAGE_FRA:
            return msgCtx->msgLength = font->msgLength =
                       SohUtils::CopyStringToCharBuffer(buffer, messageEntry.GetFrench(MF_RAW), maxBufferSize);
        case LANGUAGE_GER:
            return msgCtx->msgLength = font->msgLength =
                       SohUtils::CopyStringToCharBuffer(buffer, messageEntry.GetGerman(MF_RAW), maxBufferSize);
        case LANGUAGE_ENG:
        default:
            return msgCtx->msgLength = font->msgLength =
                       SohUtils::CopyStringToCharBuffer(buffer, messageEntry.GetEnglish(MF_RAW), maxBufferSize);
    }
    return false;
}

extern "C" void Overlay_DisplayText(float duration, const char* text) {
    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(duration, true, text);
}

extern "C" void Overlay_DisplayText_Seconds(int seconds, const char* text) {
    float duration = seconds * OTRGlobals::Instance->GetInterpolationFPS() * 0.05;
    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(duration, true, text);
}

extern "C" void EntranceTracker_SetCurrentGrottoID(s16 entranceIndex) {
    SetCurrentGrottoIDForTracker(entranceIndex);
}

extern "C" void EntranceTracker_SetLastEntranceOverride(s16 entranceIndex) {
    SetLastEntranceOverrideForTracker(entranceIndex);
}

extern "C" void Gfx_RegisterBlendedTexture(const char* name, u8* mask, u8* replacement) {
    if (auto intP = dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow())
                        ->GetInterpreterWeak()
                        .lock()) {
        intP->RegisterBlendedTexture(name, mask, replacement);
    } else {
        assert(false && "Lost reference to Fast::Interpreter");
    }
}

extern "C" void Gfx_UnregisterBlendedTexture(const char* name) {
    if (auto intP = dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow())
                        ->GetInterpreterWeak()
                        .lock()) {
        intP->UnregisterBlendedTexture(name);
    } else {
        assert(false && "Lost reference to Fast::Interpreter");
    }
}

extern "C" void Gfx_TextureCacheDelete(const uint8_t* texAddr) {
    char* imgName = (char*)texAddr;

    if (texAddr == nullptr) {
        return;
    }

    if (ResourceMgr_OTRSigCheck(imgName)) {
        texAddr = (const uint8_t*)ResourceMgr_GetResourceDataByNameHandlingMQ(imgName);
    }

    if (auto intP = dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow())
                        ->GetInterpreterWeak()
                        .lock()) {
        intP->TextureCacheDelete(texAddr);
    } else {
        assert(false && "Lost reference to Fast::Interpreter");
    }
}

void SoH_ProcessDroppedFiles(std::string filePath) {
    try {
        std::ifstream configStream(filePath);
        if (!configStream) {
            return;
        }

        nlohmann::json configJson;
        configStream >> configJson;

        // #region SOH [Randomizer] TODO: Refactor spoiler file handling for randomizer
        if (configJson.contains("version") && configJson.contains("finalSeed")) {
            CVarSetString(CVAR_GENERAL("RandomizerDroppedFile"), filePath.c_str());
            CVarSetInteger(CVAR_GENERAL("RandomizerNewFileDropped"), 1);
            return;
        }
        // #endregion

        if (!configJson.contains("CVars")) {
            return;
        }

        CVarClearBlock(CVAR_PREFIX_ENHANCEMENT);
        CVarClearBlock(CVAR_PREFIX_CHEAT);
        CVarClearBlock(CVAR_PREFIX_RANDOMIZER_SETTING);
        CVarClearBlock(CVAR_PREFIX_RANDOMIZER_ENHANCEMENT);
        CVarClearBlock(CVAR_PREFIX_DEVELOPER_TOOLS);

        // Flatten everything under CVars into a single array
        auto cvars = configJson["CVars"].flatten();

        for (auto& [key, value] : cvars.items()) {
            // Replace slashes with dots in key, and remove leading dot
            std::string path = key;
            std::replace(path.begin(), path.end(), '/', '.');
            if (path[0] == '.') {
                path.erase(0, 1);
            }
            if (value.is_string()) {
                CVarSetString(path.c_str(), value.get<std::string>().c_str());
            } else if (value.is_number_integer()) {
                CVarSetInteger(path.c_str(), value.get<int>());
            } else if (value.is_number_float()) {
                CVarSetFloat(path.c_str(), value.get<float>());
            }
        }

        Rando::Settings::GetInstance()->UpdateOptionProperties();

        auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
        gui->GetGuiWindow("Console")->Hide();
        gui->GetGuiWindow("Actor Viewer")->Hide();
        gui->GetGuiWindow("Collision Viewer")->Hide();
        gui->GetGuiWindow("Save Editor")->Hide();
        gui->GetGuiWindow("Display List Viewer")->Hide();
        gui->GetGuiWindow("Stats")->Hide();
        std::dynamic_pointer_cast<Ship::ConsoleWindow>(
            Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
            ->ClearBindings();

        gui->SaveConsoleVariablesNextFrame();
        ShipInit::Init("*");

        uint32_t finalHash = SohUtils::Hash(configJson.dump());
        gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Configuration Loaded. Hash: %d", finalHash);
    } catch (std::exception& e) {
        SPDLOG_ERROR("Failed to load config file: {}", e.what());
        auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
        gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Failed to load config file");
        return;
    } catch (...) {
        SPDLOG_ERROR("Failed to load config file");
        auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
        gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Failed to load config file");
        return;
    }
}

extern "C" void CheckTracker_RecalculateAvailableChecks() {
    CheckTracker::RecalculateAvailableChecks();
}
