#include "OTRGlobals.h"
#include <libultraship/libultraship.h>
#include "soh/resource/type/Scene.h"
#include <utils/StringHelper.h>
#include "global.h"
#include "vt.h"
#include "soh/resource/type/Text.h"
#include <message_data_static.h>
#include "Enhancements/custom-message/CustomMessageManager.h"
#include "Enhancements/custom-message/CustomMessageTypes.h"

extern "C" MessageTableEntry* sNesMessageEntryTablePtr;
extern "C" MessageTableEntry* sGerMessageEntryTablePtr;
extern "C" MessageTableEntry* sFraMessageEntryTablePtr;
extern "C" MessageTableEntry* sJpnMessageEntryTablePtr;
extern "C" MessageTableEntry* sStaffMessageEntryTablePtr;
// extern "C" MessageTableEntry* _message_0xFFFC_nes;

static void SetMessageEntry(MessageTableEntry& entry, const SOH::MessageEntry& msgEntry) {
    entry.textId = msgEntry.id;
    entry.typePos = (msgEntry.textboxType << 4) | msgEntry.textboxYPos;
    entry.segment = msgEntry.msg.c_str();
    entry.msgSize = msgEntry.msg.size();
}

static void OTRMessage_LoadCustom(const std::string& folderPath, MessageTableEntry*& table, size_t tableSize) {
    auto lst = *Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->ListFiles(folderPath).get();

    for (auto& tPath : lst) {
        auto file = std::static_pointer_cast<SOH::Text>(
            Ship::Context::GetInstance()->GetResourceManager()->LoadResource(tPath));

        for (size_t j = 0; j < file->messages.size(); ++j) {
            // Check if same text ID exists already
            auto existingEntry = std::find_if(table, table + tableSize, [id = file->messages[j].id](const auto& entry) {
                return entry.textId == id;
            });

            if (existingEntry != table + tableSize) {
                // Replace existing message
                SetMessageEntry(*existingEntry, file->messages[j]);
            }
        }
    }
}

MessageTableEntry* OTRMessage_LoadTable(const std::string& filePath, bool isNES) {
    auto file =
        std::static_pointer_cast<SOH::Text>(Ship::Context::GetInstance()->GetResourceManager()->LoadResource(filePath));

    if (file == nullptr)
        return nullptr;

    // Allocate room for an additional message
    // OTRTODO: Should not be malloc'ing here. It's fine for now since we check elsewhere that the message table is
    // already null.
    MessageTableEntry* table = (MessageTableEntry*)malloc(sizeof(MessageTableEntry) * file->messages.size());

    for (size_t i = 0; i < file->messages.size(); i++) {
        SetMessageEntry(table[i], file->messages[i]);

        if (isNES && file->messages[i].id == 0xFFFC)
            _message_0xFFFC_nes = (char*)file->messages[i].msg.c_str();
    }
    OTRMessage_LoadCustom("override/" + filePath.substr(0, filePath.find_last_of('/')) + "/*", table,
                          file->messages.size() + 1);

    // Assert that the first message starts at the first text ID
    assert(table[0].textId == 0x0001);

    return table;
}

extern "C" void OTRMessage_Init() {
    // OTRTODO: Added a lot of null checks here so that we don't malloc the table multiple times causing a memory leak.
    // We really ought to fix the implementation such that we aren't malloc'ing new tables.
    // Once we fix the implementation, remove these NULL checks.
    if (sNesMessageEntryTablePtr == NULL) {
        sNesMessageEntryTablePtr = OTRMessage_LoadTable("text/nes_message_data_static/nes_message_data_static", true);
    }
    if (sGerMessageEntryTablePtr == NULL) {
        sGerMessageEntryTablePtr = OTRMessage_LoadTable("text/ger_message_data_static/ger_message_data_static", false);
    }
    if (sFraMessageEntryTablePtr == NULL) {
        sFraMessageEntryTablePtr = OTRMessage_LoadTable("text/fra_message_data_static/fra_message_data_static", false);
    }
    if (sJpnMessageEntryTablePtr == NULL) {
        sJpnMessageEntryTablePtr = OTRMessage_LoadTable("text/jpn_message_data_static/jpn_message_data_static", false);
    }
    // Note: Make sure this loads after PAL nes_message_data_static, so that message 0xFFFC is definitely loaded if it
    // exists
    if (sNesMessageEntryTablePtr == NULL) {
        sNesMessageEntryTablePtr =
            OTRMessage_LoadTable("text/nes_message_data_static/ntsc_nes_message_data_static", false);
    }

    if (sStaffMessageEntryTablePtr == NULL) {
        auto file2 =
            std::static_pointer_cast<SOH::Text>(Ship::Context::GetInstance()->GetResourceManager()->LoadResource(
                "text/staff_message_data_static/staff_message_data_static"));
        // OTRTODO: Should not be malloc'ing here. It's fine for now since we check that the message table is already
        // null.
        sStaffMessageEntryTablePtr = (MessageTableEntry*)malloc(sizeof(MessageTableEntry) * file2->messages.size());

        for (size_t i = 0; i < file2->messages.size(); i++) {
            SetMessageEntry(sStaffMessageEntryTablePtr[i], file2->messages[i]);
        }
        OTRMessage_LoadCustom("override/text/staff_message_data_static/*", sStaffMessageEntryTablePtr,
                              file2->messages.size());

        // Assert staff credits start at the first credits ID
        assert(sStaffMessageEntryTablePtr[0].textId == 0x0500);
    }

    CustomMessageManager::Instance->AddCustomMessageTable(customMessageTableID);
    CustomMessageManager::Instance->CreateGetItemMessage(
        customMessageTableID, (GetItemID)TEXT_GS_NO_FREEZE, ITEM_SKULL_TOKEN,
        CustomMessage("You got a %rGold Skulltula Token%w!&You've collected %r[[gsCount]]%w tokens&in total!\x0E\x3C",
                      "Ein %rGoldenes Skulltula-Symbol%w!&Du hast nun insgesamt %r[[gsCount]]&%wGoldene "
                      "Skulltula-Symbole&gesammelt!\x0E\x3C",
                      "Vous obtenez un %rSymbole de&Skulltula d'or%w! Vous avez&collecté %r[[gsCount]]%w symboles en "
                      "tout!\x0E\x3C",
                      TEXTBOX_TYPE_BLUE));
    CustomMessageManager::Instance->CreateGetItemMessage(
        customMessageTableID, (GetItemID)TEXT_GS_FREEZE, ITEM_SKULL_TOKEN,
        CustomMessage(
            "You got a %rGold Skulltula Token%w!&You've collected %r[[gsCount]]%w tokens&in total!",
            "Ein %rGoldenes Skulltula-Symbol%w!&Du hast nun insgesamt %r[[gsCount]]&%wGoldene "
            "Skulltula-Symbole&gesammelt!",
            "Vous obtenez un %rSymbole de&Skulltula d'or%w! Vous avez&collecté %r[[gsCount]]%w symboles en tout!",
            TEXTBOX_TYPE_BLUE));
    CustomMessageManager::Instance->CreateMessage(
        customMessageTableID, TEXT_BUY_BOMBCHUS_10_DESC,
        CustomMessage("\x08%rBombchu  10 pieces  99 Rupees&%wThis looks like a toy mouse, but&it's actually a "
                      "self-propelled time&bomb!\x09\x0A",
                      "\x08%rKrabbelmine  10 Stück  99 Rubine&%wDas ist eine praktische Zeitbombe,&die Du als "
                      "Distanzwaffe&einsetzen kannst!\x09\x0A",
                      "\x08%rMissile  10 unités  99 Rubis&%wProfilée comme une souris&mécanique, cette arme est "
                      "&destructrice!!!\x09\x0A"));
    CustomMessageManager::Instance->CreateMessage(
        customMessageTableID, TEXT_BUY_BOMBCHUS_10_PROMPT,
        CustomMessage("\x08"
                      "Bombchu  10 pieces   99 Rupees\x09&&\x1B%gBuy&Don't buy%w",
                      "\x08Krabbelmine  10 Stück  99 Rubine\x09&&\x1B%gKaufen!&Nicht kaufen!%w",
                      "\x08Missiles  10 unités   99 Rubis\x09&&\x1B%gAcheter&Ne pas acheter%w"));
    CustomMessageManager::Instance->CreateGetItemMessage(
        customMessageTableID, (GetItemID)TEXT_HEART_CONTAINER, ITEM_HEART_CONTAINER,
        CustomMessage(
            "You got a %rHeart Container%w!&You've collected %r[[heartContainerCount]]%w containers&in total!",
            "Ein %rHerzcontainer%w!&Du hast nun insgesamt %r[[heartContainerCount]]%w&Herzcontainer gesammelt!",
            "Vous obtenez un %rCoeur&d'Energie%w! Vous en avez&collecté %r[[heartContainerCount]]%w en tout!"));
    CustomMessageManager::Instance->CreateGetItemMessage(
        customMessageTableID, (GetItemID)TEXT_HEART_PIECE, ITEM_HEART_PIECE,
        CustomMessage("You got a %rHeart Piece%w!&You've collected %r[[heartPieceCount]]%w pieces&in total!",
                      "Ein %rHerzteil%w!&Du hast nun insgesamt %r[[heartPieceCount]]%w&Herzteile gesammelt!",
                      "Vous obtenez un %rQuart de&Coeur%w! Vous en avez collecté&%r[[heartPieceCount]]%w en tout!",
                      TEXTBOX_TYPE_BLUE));
    CustomMessageManager::Instance->CreateMessage(
        customMessageTableID, TEXT_MARKET_GUARD_NIGHT,
        CustomMessage("You look bored. Wanna go out for a&walk?\x1B&%gYes&No%w",
                      "Du siehst gelangweilt aus.&Willst Du einen Spaziergang machen?\x1B&%gJa&Nein%w",
                      "Tu as l'air de t'ennuyer. Tu veux&aller faire un tour?\x1B&%gOui&Non%w"));
    CustomMessageManager::Instance->CreateMessage(
        customMessageTableID, TEXT_FISHERMAN_LEAVE,
        CustomMessage("Hey! Hey!&You can't take the rod out of here!&I'm serious!^Do you want to quit?&\x1B&%gYes&No%w",
                      "Hey! Hey!&Du kannst die Angel doch nicht&einfach mitnehmen!&Ganz im Ernst!^Möchtest Du "
                      "aufhören?&\x1B&%gJa&Nein%w",
                      "Holà! Holà!&Les cannes ne sortent pas d'ici!&Je suis sérieux!^Voulez-vous "
                      "arrêter?&\x1B&%gOui&Non%w")); // TODO Used AI translation as placeholder
    CustomMessageManager::Instance->CreateMessage(
        customMessageTableID, TEXT_CARPET_SALESMAN_CUSTOM_FAIL_TO_BUY,
        CustomMessage("I'm sorry I can't sell you these fine specimens, they need an #experienced owner#.^"
                      "Come back when you have had #Bombchus# of your own.",
                      { QM_RED, QM_GREEN }));
    CustomMessageManager::Instance->CreateMessage(
        customMessageTableID, TEXT_BIG_POE_COLLECTED_RANDO,
        CustomMessage("You have #" + CustomMessage::POINTS("\x01") + "# points.", { QM_RED }));
}
