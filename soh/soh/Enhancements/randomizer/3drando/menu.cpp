#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <ctime>

#include "menu.hpp"
#include "playthrough.hpp"
#include "spoiler_log.hpp"
#include "../location_access.h"
#include "soh/Enhancements/debugger/performanceTimer.h"
#include <spdlog/spdlog.h>
#include "../../randomizer/randomizerTypes.h"

namespace {
bool seedChanged;
uint16_t pastSeedLength;
std::vector<std::string> presetEntries;
Rando::Option* currentSetting;
} // namespace

bool GenerateRandomizer(std::set<RandomizerCheck> excludedLocations, std::set<RandomizerTrick> enabledTricks,
                        std::string seedInput) {
    const auto ctx = Rando::Context::GetInstance();
    ResetPerformanceTimers();
    StartPerformanceTimer(PT_WHOLE_SEED);

    srand(static_cast<uint32_t>(time(NULL)));
    // if a blank seed was entered, make a random one
    if (seedInput.empty()) {
        seedInput = std::to_string(rand());
    } else if (seedInput.rfind("seed_testing_count", 0) == 0 && seedInput.length() > 18) {
        int count;
        try {
            count = std::stoi(seedInput.substr(18), nullptr);
        } catch (std::invalid_argument&) { count = 1; } catch (std::out_of_range&) {
            count = 1;
        }
        Playthrough::Playthrough_Repeat(excludedLocations, enabledTricks, count);
        return false; // TODO: Not sure if this is correct but I don't think we support this functionality yet anyway.
    }

    ctx->SetSeedString(seedInput);
    uint32_t seedHash = SohUtils::Hash(ctx->GetSeedString());
    ctx->SetSeed(seedHash);

    ctx->ClearItemLocations();
    int ret = Playthrough::Playthrough_Init(ctx->GetSeed(), excludedLocations, enabledTricks);
    if (ret < 0) {
        if (ret == -1) { // Failed to generate after 5 tries
            SPDLOG_ERROR("Failed to generate after 5 tries.");
            return false;
        } else {
            SPDLOG_ERROR("Error {} with fill.", ret);
            return false;
        }
    }

    StopPerformanceTimer(PT_WHOLE_SEED);
    SPDLOG_DEBUG("Full Seed Genration Time: {}ms", GetPerformanceTimer(PT_WHOLE_SEED).count());
    SPDLOG_DEBUG("LogicReset time: {}ms", GetPerformanceTimer(PT_LOGIC_RESET).count());
    SPDLOG_DEBUG("Area->Reset time: {}ms", GetPerformanceTimer(PT_REGION_RESET).count());
    SPDLOG_DEBUG("Total Entrance Shuffle time: {}ms", GetPerformanceTimer(PT_ENTRANCE_SHUFFLE).count());
    SPDLOG_DEBUG("Total Shopsanity time: {}ms", GetPerformanceTimer(PT_SHOPSANITY).count());
    SPDLOG_DEBUG("Total Dungeon Specific Items time: {}ms", GetPerformanceTimer(PT_OWN_DUNGEON).count());
    SPDLOG_DEBUG("Total Misc Limited Checks time: {}ms", GetPerformanceTimer(PT_LIMITED_CHECKS).count());
    SPDLOG_DEBUG("Total Advancment Checks time: {}ms", GetPerformanceTimer(PT_ADVANCEMENT_ITEMS).count());
    SPDLOG_DEBUG("Total Other Checks time: {}ms", GetPerformanceTimer(PT_REMAINING_ITEMS).count());
    SPDLOG_DEBUG("Total Playthrough Generation time: {}ms", GetPerformanceTimer(PT_PLAYTHROUGH_GENERATION).count());
    SPDLOG_DEBUG("Total PareDownPlaythrough time: {}ms", GetPerformanceTimer(PT_PARE_DOWN_PLAYTHROUGH).count());
    SPDLOG_DEBUG("Total WotH generation time: {}ms", GetPerformanceTimer(PT_WOTH).count());
    SPDLOG_DEBUG("Total Foolish generation time: {}ms", GetPerformanceTimer(PT_FOOLISH).count());
    SPDLOG_DEBUG("Total Overrides time: {}ms", GetPerformanceTimer(PT_OVERRIDES).count());
    SPDLOG_DEBUG("Total Hint Generation time: {}ms", GetPerformanceTimer(PT_HINTS).count());
    SPDLOG_DEBUG("SpoilerLog writing time: {}ms", GetPerformanceTimer(PT_SPOILER_LOG).count());
    SPDLOG_DEBUG("Total Event Access Calculation time: {}ms", GetPerformanceTimer(PT_EVENT_ACCESS).count());
    SPDLOG_DEBUG("Total ToD Access Calculation: {}ms", GetPerformanceTimer(PT_TOD_ACCESS).count());
    SPDLOG_DEBUG("Total Entrance Logic Calculation time: {}ms", GetPerformanceTimer(PT_ENTRANCE_LOGIC).count());
    SPDLOG_DEBUG("Total Check Logic Calculation time: {}ms", GetPerformanceTimer(PT_LOCATION_LOGIC).count());
    return true;
}