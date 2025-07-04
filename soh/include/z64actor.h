#ifndef Z64ACTOR_H
#define Z64ACTOR_H

#include "z64dma.h"
#include "z64animation.h"
#include "z64math.h"
#include "z64collision_check.h"
#include "z64bgcheck.h"
#include "soh/Enhancements/item-tables/ItemTableTypes.h"
#include "z64actor_enum.h"
#include "soh/Enhancements/randomizer/randomizerTypes.h"

#define ACTOR_NUMBER_MAX 2000
#define INVISIBLE_ACTOR_MAX 20
#define AM_FIELD_SIZE 0x27A0
#define MASS_IMMOVABLE 0xFF // Cannot be pushed by OC collisions
#define MASS_HEAVY 0xFE // Can only be pushed by OC collisions with IMMOVABLE and HEAVY objects.

struct Actor;
struct PlayState;
struct Lights;

typedef void (*ActorFunc)(struct Actor*, struct PlayState*);
typedef void (*ActorResetFunc)(void);
typedef void (*ActorShadowFunc)(struct Actor*, struct Lights*, struct PlayState*);
typedef u16 (*NpcGetTextIdFunc)(struct PlayState*, struct Actor*);
typedef s16 (*NpcUpdateTalkStateFunc)(struct PlayState*, struct Actor*);

typedef struct {
    Vec3f pos;
    Vec3s rot;
} PosRot; // size = 0x14

typedef struct {
    /* 0x00 */ s16 id;
    /* 0x02 */ u8 category; // Classifies actor and determines when it will update or draw
    /* 0x04 */ u32 flags;
    /* 0x08 */ s16 objectId;
    /* 0x0C */ u32 instanceSize;
    /* 0x10 */ ActorFunc init; // Constructor
    /* 0x14 */ ActorFunc destroy; // Destructor
    /* 0x18 */ ActorFunc update; // Update Function
    /* 0x1C */ ActorFunc draw; // Draw function
    /* 0x20 */ ActorResetFunc reset;
} ActorInit; // size = 0x20

typedef enum {
    /* 0 */ ALLOCTYPE_NORMAL,
    /* 1 */ ALLOCTYPE_ABSOLUTE,
    /* 2 */ ALLOCTYPE_PERMANENT
} AllocType;

typedef struct {
    u8 table[32];
} DamageTable;

typedef struct {
    /* 0x00 */ u8 health;
    /* 0x02 */ s16 cylRadius;
    /* 0x04 */ s16 cylHeight;
    /* 0x06 */ u8 mass;
} CollisionCheckInfoInit;

typedef struct {
    /* 0x00 */ u8 health;
    /* 0x02 */ s16 cylRadius;
    /* 0x04 */ s16 cylHeight;
    /* 0x06 */ s16 cylYShift;
    /* 0x08 */ u8 mass;
} CollisionCheckInfoInit2;

typedef struct {
    /* 0x00 */ DamageTable* damageTable;
    /* 0x04 */ Vec3f displacement; // Amount to correct actor velocity by when colliding into a body
    /* 0x10 */ s16 cylRadius; // Used for various purposes
    /* 0x12 */ s16 cylHeight; // Used for various purposes
    /* 0x14 */ s16 cylYShift; // Unused. Purpose inferred from Cylinder16 and CollisionCheck_CylSideVsLineSeg
    /* 0x16 */ u8 mass; // Used to compute displacement for OC collisions
    /* 0x17 */ u8 health; // Note: some actors may use their own health variable instead of this one
    /* 0x18 */ u8 damage; // Amount to decrement health by
    /* 0x19 */ u8 damageEffect; // Stores what effect should occur when hit by a weapon
    /* 0x1A */ u8 atHitEffect; // Stores what effect should occur when AT connects with an AC
    /* 0x1B */ u8 acHitEffect; // Stores what effect should occur when AC is touched by an AT
} CollisionCheckInfo; // size = 0x1C

typedef struct {
    /* 0x00 */ Vec3s rot; // Current actor shape rotation
    /* 0x06 */ s16 face; // Used to index eyebrow/eye/mouth textures. Only used by player
    /* 0x08 */ f32 yOffset; // Model y axis offset. Represents model space units
    /* 0x0C */ ActorShadowFunc shadowDraw; // Shadow draw function
    /* 0x10 */ f32 shadowScale; // Changes the size of the shadow
    /* 0x14 */ u8 shadowAlpha; // Default is 255
    /* 0x15 */ u8 feetFloorFlags; // Set if the actor's foot is clipped under the floor. & 1 is right foot, & 2 is left
    /* 0x18 */ Vec3f feetPos[2]; // Update by using `Actor_SetFeetPos` in PostLimbDraw
} ActorShape; // size = 0x30

// Actor is discoverable by the Attention System. This enables Navi to hover over the actor when it is in range.
// The actor can also be locked onto (as long as `ACTOR_FLAG_LOCK_ON_DISABLED` is not set).
#define ACTOR_FLAG_ATTENTION_ENABLED (1 << 0)

// Actor is hostile toward the Player. Player has specific "battle" behavior when locked onto hostile actors.
// Enemy background music will also be played when the player is close enough to a hostile actor.
// Note: This must be paired with `ACTOR_FLAG_ATTENTION_ENABLED` to have any effect
#define ACTOR_FLAG_HOSTILE (1 << 2)

// Actor is considered "friendly"; Opposite flag of `ACTOR_FLAG_HOSTILE`.
// Note that this flag doesn't have any effect on either the actor, or Player's behavior.
// What actually matters is the presence or lack of `ACTOR_FLAG_HOSTILE`.
#define ACTOR_FLAG_FRIENDLY (1 << 3)

// Culling of the actor's update process is disabled.
// In other words, the actor will keep updating even if the actor is outside its own culling volume.
// See `Actor_CullingCheck` for more information about culling.
// See `Actor_CullingVolumeTest` for more information on the test used to determine if an actor should be culled.
#define ACTOR_FLAG_UPDATE_CULLING_DISABLED (1 << 4)

// Culling of the actor's draw process is disabled.
// In other words, the actor will keep drawing even if the actor is outside its own culling volume.
// See `Actor_CullingCheck` for more information about culling.
// See `Actor_CullingVolumeTest` for more information on the test used to determine if an actor should be culled.
// (The original name for this flag is `NO_CULL_DRAW`, known from the Majora's Mask Debug ROM)
#define ACTOR_FLAG_DRAW_CULLING_DISABLED (1 << 5)

// Set if the actor is currently within the bounds of its culling volume.
// In most cases, this flag can be used to determine whether or not an actor is currently culled.
// However this flag still updates even if `ACTOR_FLAG_UPDATE_CULLING_DISABLED` or `ACTOR_FLAG_DRAW_CULLING_DISABLED`
// are set. Meaning, the flag can still have a value of "false" even if it is not actually culled.
// (The original name for this flag is `NO_CULL_FLAG`, known from the Majora's Mask Debug ROM)
#define ACTOR_FLAG_INSIDE_CULLING_VOLUME (1 << 6)

// hidden or revealed by Lens of Truth (depending on room lensMode)
#define ACTOR_FLAG_REACT_TO_LENS (1 << 7)

// Signals that player has accepted an offer to talk to an actor
// Player will retain this flag until the player is finished talking
// Actor will retain this flag until `Actor_TalkOfferAccepted` is called or manually turned off by the actor
#define ACTOR_FLAG_TALK (1 << 8)

// When the hookshot attaches to this actor, the actor will be pulled back as the hookshot retracts.
#define ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR (1 << 9)

// When the hookshot attaches to this actor, Player will be pulled by the hookshot and fly to the actor.
#define ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER (1 << 10)

// A clump of grass (EN_KUSA) has been destroyed.
// This flag is used to communicate with the spawner actor (OBJ_MURE).
#define ACTOR_FLAG_GRASS_DESTROYED (1 << 11)

// Actor will not shake when a quake occurs
#define ACTOR_FLAG_IGNORE_QUAKE (1 << 12)

// The hookshot is currently attached to this actor.
// The behavior that occurs after attachment is determined by `ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR` and `ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER`.
// If neither of those flags are set attachment cannot occur, and the hookshot will simply act as a damage source.
//
// This flag is also reused to indicate that an actor is attached to the boomerang.
// This only has an effect for Gold Skulltula Tokens (EN_SI) which has overlapping behavior for hookshot and boomerang.
#define ACTOR_FLAG_HOOKSHOT_ATTACHED (1 << 13)

// When hit by an arrow, the actor will be able to attach to the arrow and fly with it in the air
#define ACTOR_FLAG_CAN_ATTACH_TO_ARROW (1 << 14)

// Actor is currently attached to an arrow and flying with it in the air
#define ACTOR_FLAG_ATTACHED_TO_ARROW (1 << 15)

// Player automatically accepts a Talk Offer without needing to press the A button.
// Player still has to meet all conditions to be able to receive a talk offer (for example, being in range).
#define ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED (1 << 16)

// Actor will be influenced by the pitch (x rot) of Player's left hand when being carried,
// instead of Player's yaw which is the default actor carry behavior.
// This flag is helpful for something like the `BG_HEAVY_BLOCK` actor which Player carries underhanded.
#define ACTOR_FLAG_CARRY_X_ROT_INFLUENCE (1 << 17)

// When locked onto an actor with this flag set, the C-Up button can be used to talk to this actor.
// A C-Up button labeled "Navi" will appear on the HUD when locked on which indicates the actor can be checked with Navi.
// With this flag Player talks directly to the actor with C-Up. It is expected that the resulting dialog should appear
// to be coming from Navi, even though she is not involved at all with this interaction.
#define ACTOR_FLAG_TALK_WITH_C_UP (1 << 18)

// Flags controlling the use of `Actor.sfx`. Do not use directly.
#define ACTOR_FLAG_SFX_ACTOR_POS_2 (1 << 19)
#define ACTOR_AUDIO_FLAG_SFX_CENTERED_1 (1 << 20)
#define ACTOR_AUDIO_FLAG_SFX_CENTERED_2 (1 << 21)

// ignores point lights but not directional lights (such as environment lights)
#define ACTOR_FLAG_IGNORE_POINTLIGHTS (1 << 22)

// When Player is carrying this actor, it can only be thrown, not dropped/placed.
// Typically an actor can only be thrown when moving, but this allows an actor to be thrown when standing still.
#define ACTOR_FLAG_THROW_ONLY (1 << 23)

// When colliding with Player's body AC collider, a "thump" sound will play indicating his body has been hit
#define ACTOR_FLAG_SFX_FOR_PLAYER_BODY_HIT (1 << 24)

// Actor can update even if Player is currently using the ocarina.
// Typically an actor will halt while the ocarina is active (depending on category).
// This flag allows a given actor to be an exception.
#define ACTOR_FLAG_UPDATE_DURING_OCARINA (1 << 25)

// Actor can press and hold down switches.
// See usages of `DynaPolyActor_SetSwitchPressed` and `DynaPolyActor_IsSwitchPressed` for more context on how switches work.
#define ACTOR_FLAG_CAN_PRESS_SWITCHES (1 << 26)

// Player is not able to lock onto the actor.
// Navi will still be able to hover over the actor, assuming `ACTOR_FLAG_ATTENTION_ENABLED` is set.
#define ACTOR_FLAG_LOCK_ON_DISABLED (1 << 27)

// Flag controlling the use of `Actor.sfx`. Do not use directly. See Actor_PlaySfx_FlaggedTimer
#define ACTOR_FLAG_SFX_TIMER (1 << 28)

typedef struct Actor {
    /* 0x000 */ s16 id; // Actor ID
    /* 0x002 */ u8 category; // Actor category. Refer to the corresponding enum for values
    /* 0x003 */ s8 room; // Room number the actor is in. -1 denotes that the actor won't despawn on a room change
    /* 0x004 */ u32 flags; // Flags used for various purposes
    /* 0x008 */ PosRot home; // Initial position/rotation when spawned. Can be used for other purposes
    /* 0x01C */ s16 params; // Configurable variable set by the actor's spawn data; original name: "args_data"
    /* 0x01E */ s8 objBankIndex; // Object bank index of the actor's object dependency; original name: "bank"
    /* 0x01F */ s8 targetMode; // Controls how far the actor can be targeted from and how far it can stay locked on
    /* 0x020 */ u16 sfx; // SFX ID to play. Sound plays when value is set, then is cleared the following update cycle
    /* 0x024 */ PosRot world; // Position/rotation in the world
    /* 0x038 */ PosRot focus; // Target reticle focuses on this position. For player this represents head pos and rot
    /* 0x04C */ f32 targetArrowOffset; // Height offset of the target arrow relative to `focus` position
    /* 0x050 */ Vec3f scale; // Scale of the actor in each axis
    /* 0x05C */ Vec3f velocity; // Velocity of the actor in each axis
    /* 0x068 */ f32 speedXZ; // How fast the actor is traveling along the XZ plane
    /* 0x06C */ f32 gravity; // Acceleration due to gravity. Value is added to Y velocity every frame
    /* 0x070 */ f32 minVelocityY; // Sets the lower bounds cap on velocity along the Y axis
    /* 0x074 */ CollisionPoly* wallPoly; // Wall polygon the actor is touching
    /* 0x078 */ CollisionPoly* floorPoly; // Floor polygon directly below the actor
    /* 0x07C */ u8 wallBgId; // Bg ID of the wall polygon the actor is touching
    /* 0x07D */ u8 floorBgId; // Bg ID of the floor polygon directly below the actor
    /* 0x07E */ s16 wallYaw; // Y rotation of the wall polygon the actor is touching
    /* 0x080 */ f32 floorHeight; // Y position of the floor polygon directly below the actor
    /* 0x084 */ f32 yDistToWater; // Distance to the surface of active waterbox. Negative value means above water
    /* 0x088 */ u16 bgCheckFlags; // See comments below actor struct for wip docs. TODO: macros for these flags
    /* 0x08A */ s16 yawTowardsPlayer; // Y rotation difference between the actor and the player
    /* 0x08C */ f32 xyzDistToPlayerSq; // Squared distance between the actor and the player
    /* 0x090 */ f32 xzDistToPlayer; // Distance between the actor and the player in the XZ plane
    /* 0x094 */ f32 yDistToPlayer; // Dist is negative if the actor is above the player
    /* 0x098 */ CollisionCheckInfo colChkInfo; // Variables related to the Collision Check system
    /* 0x0B4 */ ActorShape shape; // Variables related to the physical shape of the actor
    /* 0x0E4 */ Vec3f projectedPos; // Position of the actor in projected space
    /* 0x0F0 */ f32 projectedW; // w component of the projected actor position
    /* 0x0F4 */ f32 uncullZoneForward; // Amount to increase the uncull zone forward by (in projected space)
    /* 0x0F8 */ f32 uncullZoneScale; // Amount to increase the uncull zone scale by (in projected space)
    /* 0x0FC */ f32 uncullZoneDownward; // Amount to increase uncull zone downward by (in projected space)
    /* 0x100 */ Vec3f prevPos; // World position from the previous update cycle
    /* 0x10C */ u8 isTargeted; // Set to true if the actor is currently being targeted by the player
    /* 0x10D */ u8 targetPriority; // Lower values have higher priority. Resets to 0 when player stops targeting
    /* 0x10E */ u16 textId; // Text ID to pass to link/display when interacting with the actor
    /* 0x110 */ u16 freezeTimer; // Actor does not update when set. Timer decrements automatically
    /* 0x112 */ u16 colorFilterParams; // Set color filter to red, blue, or white. Toggle opa or xlu
    /* 0x114 */ u8 colorFilterTimer; // A non-zero value enables the color filter. Decrements automatically
    /* 0x115 */ u8 isDrawn; // Set to true if the actor is currently being drawn. Always stays false for lens actors
    /* 0x116 */ u8 dropFlag; // Configures what item is dropped by the actor from `Item_DropCollectibleRandom`
    /* 0x117 */ u8 naviEnemyId; // Sets what 0600 dialog to display when talking to navi. Default 0xFF
    /* 0x118 */ struct Actor* parent; // Usage is actor specific. Set if actor is spawned via `Actor_SpawnAsChild`
    /* 0x11C */ struct Actor* child; // Usage is actor specific. Set if actor is spawned via `Actor_SpawnAsChild`
    /* 0x120 */ struct Actor* prev; // Previous actor of this category
    /* 0x124 */ struct Actor* next; // Next actor of this category
    /* 0x128 */ ActorFunc init; // Initialization Routine. Called by `Actor_Init` or `Actor_UpdateAll`
    /* 0x12C */ ActorFunc destroy; // Destruction Routine. Called by `Actor_Destroy`
    /* 0x130 */ ActorFunc update; // Update Routine. Called by `Actor_UpdateAll`
    /* 0x134 */ ActorFunc draw; // Draw Routine. Called by `Actor_Draw`
    /* 0x138 */ ActorResetFunc reset;
    /* 0x13C */ char dbgPad[0x10]; // Padding that only exists in the debug rom
} Actor; // size = 0x14C

typedef enum {
    /* 0 */ FOOT_LEFT,
    /* 1 */ FOOT_RIGHT
} ActorFootIndex;

/*
BgCheckFlags WIP documentation:
& 0x001 : Standing on the ground
& 0x002 : Has touched the ground (only active for 1 frame)
& 0x004 : Has left the ground (only active for 1 frame)
& 0x008 : Touching a wall
& 0x010 : Touching a ceiling
& 0x020 : On or below water surface
& 0x040 : Has touched water (actor is responsible for unsetting this the frame it touches the water)
& 0x080 : Similar to & 0x1 but with no velocity check and is cleared every frame
& 0x100 : Crushed between a floor and ceiling (triggers a void for player)
& 0x200 : Unknown (only set/used by player so far)
*/

/*
colorFilterParams WIP documentation
& 0x8000 : white
& 0x4000 : red
if neither of the above are set : blue

(& 0x1F00 >> 5) | 7 : color intensity
0x2000 : translucent, else opaque
*/

#define DYNA_INTERACT_ACTOR_ON_TOP (1 << 0)  // There is an actor standing on the collision of the dynapoly actor
#define DYNA_INTERACT_PLAYER_ON_TOP (1 << 1) // The player actor is standing on the collision of the dynapoly actor
#define DYNA_INTERACT_PLAYER_ABOVE \
    (1 << 2) // The player is directly above the collision of the dynapoly actor (any distance above)
#define DYNA_INTERACT_ACTOR_SWITCH_PRESSED \
    (1 << 3) // An actor that is capable of pressing switches is on top of the dynapoly actor

typedef struct DynaPolyActor {
    /* 0x000 */ struct Actor actor;
    /* 0x14C */ s32 bgId;
    /* 0x150 */ f32 unk_150;
    /* 0x154 */ f32 unk_154;
    /* 0x158 */ s16 unk_158; // y rotation?
    /* 0x15A */ u16 unk_15A;
    /* 0x15C */ u32 transformFlags;
    /* 0x160 */ u8 interactFlags;
    /* 0x162 */ s16 unk_162;
} DynaPolyActor; // size = 0x164

typedef struct {
    /* 0x00 */ MtxF* matrices;
    /* 0x04 */ s16* objectIds;
    /* 0x08 */ s16 count;
    /* 0x0C */ Gfx** dLists;
    /* 0x10 */ s32 val; // used for various purposes: both a status indicator and counter
    /* 0x14 */ s32 prevLimbIndex;
} BodyBreak;

#define BODYBREAK_OBJECT_DEFAULT -1 // use the same object as the actor
#define BODYBREAK_STATUS_READY -1
#define BODYBREAK_STATUS_FINISHED 0

typedef enum {
    /* 0x00 */ ITEM00_RUPEE_GREEN,
    /* 0x01 */ ITEM00_RUPEE_BLUE,
    /* 0x02 */ ITEM00_RUPEE_RED,
    /* 0x03 */ ITEM00_HEART,
    /* 0x04 */ ITEM00_BOMBS_A,
    /* 0x05 */ ITEM00_ARROWS_SINGLE,
    /* 0x06 */ ITEM00_HEART_PIECE,
    /* 0x07 */ ITEM00_HEART_CONTAINER,
    /* 0x08 */ ITEM00_ARROWS_SMALL,
    /* 0x09 */ ITEM00_ARROWS_MEDIUM,
    /* 0x0A */ ITEM00_ARROWS_LARGE,
    /* 0x0B */ ITEM00_BOMBS_B,
    /* 0x0C */ ITEM00_NUTS,
    /* 0x0D */ ITEM00_STICK,
    /* 0x0E */ ITEM00_MAGIC_LARGE,
    /* 0x0F */ ITEM00_MAGIC_SMALL,
    /* 0x10 */ ITEM00_SEEDS,
    /* 0x11 */ ITEM00_SMALL_KEY,
    /* 0x12 */ ITEM00_FLEXIBLE,
    /* 0x13 */ ITEM00_RUPEE_ORANGE,
    /* 0x14 */ ITEM00_RUPEE_PURPLE,
    /* 0x15 */ ITEM00_SHIELD_DEKU,
    /* 0x16 */ ITEM00_SHIELD_HYLIAN,
    /* 0x17 */ ITEM00_TUNIC_ZORA,
    /* 0x18 */ ITEM00_TUNIC_GORON,
    /* 0x19 */ ITEM00_BOMBS_SPECIAL,
    /* 0x1A */ ITEM00_BOMBCHU,
    /* 0x1B */ ITEM00_SOH_DUMMY,
    /* 0x1C */ ITEM00_SOH_GIVE_ITEM_ENTRY,
    /* 0x1D */ ITEM00_SOH_GIVE_ITEM_ENTRY_GI,
    /* 0x1E */ ITEM00_MAX,
    /* 0xFF */ ITEM00_NONE = 0xFF
} Item00Type;

struct EnItem00;

typedef void (*EnItem00ActionFunc)(struct EnItem00*, struct PlayState*);

typedef struct EnItem00 {
    /* 0x000 */ Actor actor;
    /* 0x14C */ EnItem00ActionFunc actionFunc;
    /* 0x150 */ s16 collectibleFlag;
    /* 0x152 */ s16 getItemId;
    /* 0x154 */ s16 unk_154;
    /* 0x156 */ s16 unk_156;
    /* 0x158 */ s16 unk_158;
    /* 0x15A */ s16 unk_15A;
    /* 0x15C */ f32 scale;
    /* 0x160 */ ColliderCylinder collider;
    // #region SOH [Randomizer]
    RandomizerCheck randoCheck;
    RandomizerInf randoInf;
    /*       */ s16 ogParams;
    /*       */ GetItemEntry itemEntry;
    // #endregion
} EnItem00; // size = 0x1AC

// Only A_OBJ_SIGNPOST_OBLONG and A_OBJ_SIGNPOST_ARROW are used in room files.
typedef enum {
    /* 0x00 */ A_OBJ_BLOCK_SMALL,
    /* 0x01 */ A_OBJ_BLOCK_LARGE,
    /* 0x02 */ A_OBJ_BLOCK_HUGE,
    /* 0x03 */ A_OBJ_BLOCK_SMALL_ROT,
    /* 0x04 */ A_OBJ_BLOCK_LARGE_ROT,
    /* 0x05 */ A_OBJ_CUBE_SMALL,
    /* 0x06 */ A_OBJ_UNKNOWN_6,
    /* 0x07 */ A_OBJ_GRASS_CLUMP,
    /* 0x08 */ A_OBJ_TREE_STUMP,
    /* 0x09 */ A_OBJ_SIGNPOST_OBLONG,
    /* 0x0A */ A_OBJ_SIGNPOST_ARROW,
    /* 0x0B */ A_OBJ_BOULDER_FRAGMENT,
    /* 0x0C */ A_OBJ_MAX
} AObjType;

struct EnAObj;

typedef void (*EnAObjActionFunc)(struct EnAObj*, struct PlayState*);

typedef struct EnAObj {
    /* 0x000 */ DynaPolyActor dyna;
    /* 0x164 */ EnAObjActionFunc actionFunc;
    /* 0x168 */ s32 rotateWaitTimer;
    /* 0x16C */ s16 textId;
    /* 0x16E */ s16 rotateState;
    /* 0x170 */ s16 rotateForTimer;
    /* 0x172 */ s16 rotSpeedY;
    /* 0x174 */ s16 rotSpeedX;
    /* 0x178 */ f32 focusYoffset;
    /* 0x17C */ ColliderCylinder collider;
} EnAObj; // size = 0x1C8

typedef enum {
    /* 0x00 */ ACTORCAT_SWITCH,
    /* 0x01 */ ACTORCAT_BG,
    /* 0x02 */ ACTORCAT_PLAYER,
    /* 0x03 */ ACTORCAT_EXPLOSIVE,
    /* 0x04 */ ACTORCAT_NPC,
    /* 0x05 */ ACTORCAT_ENEMY,
    /* 0x06 */ ACTORCAT_PROP,
    /* 0x07 */ ACTORCAT_ITEMACTION,
    /* 0x08 */ ACTORCAT_MISC,
    /* 0x09 */ ACTORCAT_BOSS,
    /* 0x0A */ ACTORCAT_DOOR,
    /* 0x0B */ ACTORCAT_CHEST,
    /* 0x0C */ ACTORCAT_MAX
} ActorCategory;

//#define DEFINE_ACTOR(_0, enum, _2) enum,

typedef enum {
    DOORLOCK_NORMAL,
    DOORLOCK_BOSS,
    DOORLOCK_NORMAL_SPIRIT
} DoorLockType;

typedef enum {
    /* 0x0 */ NPC_TALK_STATE_IDLE, // NPC not currently talking to player
    /* 0x1 */ NPC_TALK_STATE_TALKING, // NPC is currently talking to player
    /* 0x2 */ NPC_TALK_STATE_ACTION, // An NPC-defined action triggered in the conversation
    /* 0x3 */ NPC_TALK_STATE_ITEM_GIVEN // NPC finished giving an item and text box is done
} NpcTalkState;

typedef enum {
    /* 0x0 */ NPC_TRACKING_PLAYER_AUTO_TURN, // Determine tracking mode based on player position, see Npc_UpdateAutoTurn
    /* 0x1 */ NPC_TRACKING_NONE, // Don't track the target (usually the player)
    /* 0x2 */ NPC_TRACKING_HEAD_AND_TORSO, // Track target by turning the head and the torso
    /* 0x3 */ NPC_TRACKING_HEAD, // Track target by turning the head
    /* 0x4 */ NPC_TRACKING_FULL_BODY // Track target by turning the body, torso and head
} NpcTrackingMode;

typedef struct {
    /* 0x00 */ s16 talkState;
    /* 0x02 */ s16 trackingMode;
    /* 0x04 */ s16 autoTurnTimer;
    /* 0x06 */ s16 autoTurnState;
    /* 0x08 */ Vec3s headRot;
    /* 0x0E */ Vec3s torsoRot;
    /* 0x14 */ f32 yOffset; // Y position offset to add to actor position when calculating angle to target
    /* 0x18 */ Vec3f trackPos;
    /* 0x24 */ char unk_24[0x4];
} NpcInteractInfo; // size = 0x28

#endif
