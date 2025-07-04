#include "util.h"

#include <string.h>
#include <vector>
#include <algorithm>
#include <array>
#include <assert.h>
#include <spdlog/spdlog.h>
#include "Enhancements/randomizer/randomizerTypes.h"

std::vector<std::string> sceneNames = {
    "Inside the Deku Tree",
    "Dodongo's Cavern",
    "Inside Jabu-Jabu's Belly",
    "Forest Temple",
    "Fire Temple",
    "Water Temple",
    "Spirit Temple",
    "Shadow Temple",
    "Bottom of the Well",
    "Ice Cavern",
    "Ganon's Tower",
    "Gerudo Training Ground",
    "Thieves' Hideout",
    "Inside Ganon's Castle",
    "Ganon's Tower (Collapsing)",
    "Inside Ganon's Castle (Collapsing)",
    "Treasure Box Shop",
    "Gohma's Lair",
    "King Dodongo's Lair",
    "Barinade's Lair",
    "Phantom Ganon's Lair",
    "Volvagia's Lair",
    "Morpha's Lair",
    "Twinrova's Lair",
    "Bongo Bongo's Lair",
    "Ganondorf's Lair",
    "Tower Collapse Exterior",
    "Market Entrance (Child - Day)",
    "Market Entrance (Child - Night)",
    "Market Entrance (Ruins)",
    "Back Alley (Child - Day)",
    "Back Alley (Child - Night)",
    "Market (Child - Day)",
    "Market (Child - Night)",
    "Market (Ruins)",
    "Temple of Time Exterior (Child - Day)",
    "Temple of Time Exterior (Child - Night)",
    "Temple of Time Exterior (Ruins)",
    "Know-It-All Brothers' House",
    "House of Twins",
    "Mido's House",
    "Saria's House",
    "Carpenter Boss's House",
    "Back Alley House (Man in Green)",
    "Bazaar",
    "Kokiri Shop",
    "Goron Shop",
    "Zora Shop",
    "Kakariko Potion Shop",
    "Market Potion Shop",
    "Bombchu Shop",
    "Happy Mask Shop",
    "Link's House",
    "Back Alley House (Dog Lady)",
    "Stable",
    "Impa's House",
    "Lakeside Laboratory",
    "Carpenters' Tent",
    "Gravekeeper's Hut",
    "Great Fairy's Fountain (Upgrades)",
    "Fairy's Fountain",
    "Great Fairy's Fountain (Spells)",
    "Grottos",
    "Grave (Redead)",
    "Grave (Fairy's Fountain)",
    "Royal Family's Tomb",
    "Shooting Gallery",
    "Temple of Time",
    "Chamber of the Sages",
    "Castle Hedge Maze (Day)",
    "Castle Hedge Maze (Night)",
    "Cutscene Map",
    "Dampe's Grave & Windmill",
    "Fishing Pond",
    "Castle Courtyard",
    "Bombchu Bowling Alley",
    "Ranch House & Silo",
    "Guard House",
    "Granny's Potion Shop",
    "Ganon's Tower Collapse & Arena",
    "House of Skulltula",
    "Hyrule Field",
    "Kakariko Village",
    "Graveyard",
    "Zora's River",
    "Kokiri Forest",
    "Sacred Forest Meadow",
    "Lake Hylia",
    "Zora's Domain",
    "Zora's Fountain",
    "Gerudo Valley",
    "Lost Woods",
    "Desert Colossus",
    "Gerudo's Fortress",
    "Haunted Wasteland",
    "Hyrule Castle",
    "Death Mountain Trail",
    "Death Mountain Crater",
    "Goron City",
    "Lon Lon Ranch",
    "Ganon's Castle Exterior",
    "Jungle Gym",
    "Ganondorf Test Room",
    "Depth Test",
    "Stalfos Mini-Boss Room",
    "Stalfos Boss Room",
    "Sutaru",
    "Castle Hedge Maze (Early)",
    "Sasa Test",
    "Treasure Chest Room",
};

std::vector<std::string> itemNames = {
    "Deku Stick",
    "Deku Nut",
    "Bomb",
    "Fairy Bow",
    "Fire Arrow",
    "Din's Fire",
    "Fairy Slingshot",
    "Fairy Ocarina",
    "Ocarina of Time",
    "Bombchu",
    "Hookshot",
    "Longshot",
    "Ice Arrow",
    "Farore's Wind",
    "Boomerang",
    "Lens of Truth",
    "Magic Bean",
    "Megaton Hammer",
    "Light Arrow",
    "Nayru's Love",
    "Empty Bottle",
    "Red Potion",
    "Green Potion",
    "Blue Potion",
    "Bottled Fairy",
    "Fish",
    "Lon Lon Milk & Bottle",
    "Ruto's Letter",
    "Blue Fire",
    "Bug",
    "Big Poe",
    "Lon Lon Milk (Half)",
    "Poe",
    "Weird Egg",
    "Chicken",
    "Zelda's Letter",
    "Keaton Mask",
    "Skull Mask",
    "Spooky Mask",
    "Bunny Hood",
    "Goron Mask",
    "Zora Mask",
    "Gerudo Mask",
    "Mask of Truth",
    "SOLD OUT",
    "Pocket Egg",
    "Pocket Cucco",
    "Cojiro",
    "Odd Mushroom",
    "Odd Potion",
    "Poacher's Saw",
    "Goron's Sword (Broken)",
    "Prescription",
    "Eyeball Frog",
    "Eye Drops",
    "Claim Check",
    "Fairy Bow & Fire Arrow",
    "Fairy Bow & Ice Arrow",
    "Fairy Bow & Light Arrow",
    "Kokiri Sword",
    "Master Sword",
    "Giant's Knife & Biggoron's Sword",
    "Deku Shield",
    "Hylian Shield",
    "Mirror Shield",
    "Kokiri Tunic",
    "Goron Tunic",
    "Zora Tunic",
    "Kokiri Boots",
    "Iron Boots",
    "Hover Boots",
    "Bullet Bag (30)",
    "Bullet Bag (40)",
    "Bullet Bag (50)",
    "Quiver (30)",
    "Big Quiver (40)",
    "Biggest Quiver (50)",
    "Bomb Bag (20)",
    "Big Bomb Bag (30)",
    "Biggest Bomb Bag (40)",
    "Goron's Bracelet",
    "Silver Gauntlets",
    "Golden Gauntlets",
    "Silver Scale",
    "Golden Scale",
    "Giant's Knife (Broken)",
    "Adult's Wallet",
    "Giant's Wallet",
    "Deku Seeds (5)",
    "Fishing Pole",
    "Minuet of Forest",
    "Bolero of Fire",
    "Serenade of Water",
    "Requiem of Spirit",
    "Nocturne of Shadow",
    "Prelude of Light",
    "Zelda's Lullaby",
    "Epona's Song",
    "Saria's Song",
    "Sun's Song",
    "Song of Time",
    "Song of Storms",
    "Forest Medallion",
    "Fire Medallion",
    "Water Medallion",
    "Spirit Medallion",
    "Shadow Medallion",
    "Light Medallion",
    "Kokiri's Emerald",
    "Goron's Ruby",
    "Zora's Sapphire",
    "Stone of Agony",
    "Gerudo's Card",
    "Gold Skulltula Token",
    "Heart Container",
    "Piece of Heart",
    "Boss Key",
    "Compass",
    "Dungeon Map",
    "Small Key",
    "Small Magic Jar",
    "Large Magic Jar",
    "Piece of Heart",
    "[Removed]",
    "[Removed]",
    "[Removed]",
    "[Removed]",
    "[Removed]",
    "[Removed]",
    "[Removed]",
    "Lon Lon Milk",
    "Recovery Heart",
    "Green Rupee",
    "Blue Rupee",
    "Red Rupee",
    "Purple Rupee",
    "Huge Rupee",
    "[Removed]",
    "Deku Sticks (5)",
    "Deku Sticks (10)",
    "Deku Nuts (5)",
    "Deku Nuts (10)",
    "Bombs (5)",
    "Bombs (10)",
    "Bombs (20)",
    "Bombs (30)",
    "Arrows (Small)",
    "Arrows (Medium)",
    "Arrows (Large)",
    "Deku Seeds (30)",
    "Bombchu (5)",
    "Bombchu (20)",
    "Deku Stick Upgrade (20)",
    "Deku Stick Upgrade (30)",
    "Deku Nut Upgrade (30)",
    "Deku Nut Upgrade (40)",
};

std::vector<std::string> questItemNames = {
    "Forest Medallion",   "Fire Medallion",   "Water Medallion", "Spirit Medallion",     "Shadow Medallion",
    "Light Medallion",    "Minuet of Forest", "Bolero of Fire",  "Serenade of Water",    "Requiem of Spirit",
    "Nocturne of Shadow", "Prelude of Light", "Zelda's Lullaby", "Epona's Song",         "Saria's Song",
    "Sun's Song",         "Song of Time",     "Song of Storms",  "Kokiri's Emerald",     "Goron's Ruby",
    "Zora's Sapphire",    "Stone of Agony",   "Gerudo's Card",   "Gold Skulltula Token",
};

std::array<std::string, RA_MAX> rcareaPrefixes = {
    "KF",
    "LW",
    "SFM",
    "HF",
    "LH",
    "GV",
    "GF",
    "Wasteland",
    "Colossus",
    "Market",
    "HC",
    "Kak",
    "Graveyard",
    "DMT",
    "GC",
    "DMC",
    "ZR",
    "ZD",
    "ZF",
    "LLR",
    "Deku Tree",
    "Dodongos Cavern",
    "Jabu Jabus Belly",
    "Forest Temple",
    "Fire Temple",
    "Water Temple",
    "Spirit Temple",
    "Shadow Temple",
    "Bottom of the Well",
    "Ice Cavern",
    "Gerudo Training Ground",
    "Ganon's Castle",
};

const std::string& SohUtils::GetSceneName(int32_t scene) {
    if (scene > sceneNames.size()) {
        SPDLOG_WARN("Passed invalid scene id to SohUtils::GetSceneName: ({})", scene);
        assert(false);
        return "";
    }

    return sceneNames[scene];
}

const std::string& SohUtils::GetItemName(int32_t item) {
    if (item > itemNames.size()) {
        SPDLOG_WARN("Passed invalid item id to SohUtils::GetItemName: ({})", item);
        assert(false);
        return "";
    }

    return itemNames[item];
}

const std::string& SohUtils::GetQuestItemName(int32_t item) {
    if (item > questItemNames.size()) {
        SPDLOG_WARN("Passed invalid quest item id to SohUtils::GetQuestItemName: ({})", item);
        assert(false);
        return "";
    }

    return questItemNames[item];
}

const std::string& SohUtils::GetRandomizerCheckAreaPrefix(int32_t rcarea) {
    if (rcarea > rcareaPrefixes.size()) {
        SPDLOG_WARN("Passed invalid rcarea to SohUtils::GetRandomizerCheckAreaPrefix: ({})", rcarea);
        assert(false);
        return "";
    }

    return rcareaPrefixes[rcarea];
}

void SohUtils::CopyStringToCharArray(char* destination, std::string source, size_t size) {
    if (size > 0) {
        strncpy(destination, source.c_str(), size - 1);
        destination[size - 1] = '\0';
    }
}

std::string SohUtils::Sanitize(std::string stringValue) {
    stringValue.erase(std::remove_if(stringValue.begin(), stringValue.end(),
                                     [](char const c) { return '\n' == c || '\r' == c || '\0' == c || '\x1A' == c; }),
                      stringValue.end());

    return stringValue;
}

size_t SohUtils::CopyStringToCharBuffer(char* buffer, const std::string& source, const size_t maxBufferSize) {
    if (!source.empty() && maxBufferSize > 0) {
        memset(buffer, 0, maxBufferSize);
        const size_t copiedCharLen = std::min<size_t>(maxBufferSize - 1, source.length());
        memcpy(buffer, source.c_str(), copiedCharLen);
        return copiedCharLen;
    }

    return 0;
}

bool SohUtils::IsStringEmpty(std::string str) {
    // Remove spaces at the beginning of the string
    std::string::size_type start = str.find_first_not_of(' ');
    // Remove spaces at the end of the string
    std::string::size_type end = str.find_last_not_of(' ');

    // Check if the string is empty after stripping spaces
    return start == std::string::npos || end == std::string::npos;
}

uint32_t SohUtils::Hash(std::string str) {
    // FNV-1a
    const size_t len = str.size();
    uint32_t hval = 0x811c9dc5;
    for (size_t pos = 0; pos < len; pos++) {
        hval ^= (uint32_t)str[pos];
        hval *= 0x01000193;
    }
    return hval;
}
