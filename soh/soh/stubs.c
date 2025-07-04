#include <libultraship/libultra.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "z64.h"
#include "OTRGlobals.h"
//#include <math.h>

u32 osResetType;
u32 osTvType = OS_TV_NTSC;
// u32 osTvType = OS_TV_PAL;
OSViMode osViModeNtscLan1;
OSViMode osViModeMpalLan1;
OSViMode osViModeFpalLan1;
OSViMode osViModePalLan1;
// AudioContext gAudioContext;
// unk_D_8016E750 D_8016E750[4];
u8 gLetterTLUT[4][32];
u8 gFontFF[999];
DmaEntry gDmaDataTable[0x60C];
// u8 D_80133418;
u16 gAudioSEFlagSwapSource[64];
u16 gAudioSEFlagSwapTarget[64];
u8 gAudioSEFlagSwapMode[64];

// Zbuffer and Color framebuffer
u16 D_0E000000[SCREEN_WIDTH * SCREEN_HEIGHT];
u16 D_0F000000[SCREEN_WIDTH * SCREEN_HEIGHT];

u8 osAppNmiBuffer[2048];

f32 qNaN0x10000 = 0x7F810000;

// void gSPTextureRectangle(Gfx* pkt, s32 xl, s32 yl, s32 xh, s32 yh, u32 tile, u32 s, s32 t, u32 dsdx, u32 dtdy)
//{
//	__gSPTextureRectangle(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy);
// }

OSId osGetThreadId(OSThread* thread) {
}

OSPri osGetThreadPri(OSThread* thread) {
}

void osSetThreadPri(OSThread* thread, OSPri pri) {
}

void osCreatePiManager(OSPri pri, OSMesgQueue* cmdQ, OSMesg* cmdBuf, s32 cmdMsgCnt) {
}

s32 osPfsFreeBlocks(OSPfs* pfs, s32* leftoverBytes) {
}

s32 osEPiWriteIo(OSPiHandle* handle, u32 devAddr, u32 data) {
}

s32 osPfsReadWriteFile(OSPfs* pfs, s32 fileNo, u8 flag, s32 offset, ptrdiff_t size, u8* data) {
}

s32 osPfsDeleteFile(OSPfs* pfs, u16 companyCode, u32 gameCode, u8* gameName, u8* extName) {
}

s32 osPfsFileState(OSPfs* pfs, s32 fileNo, OSPfsState* state) {
}

s32 osPfsInitPak(OSMesgQueue* mq, OSPfs* pfs, s32 channel) {
}

s32 __osPfsCheckRamArea(OSPfs* pfs) {
}

s32 osPfsChecker(OSPfs* pfs) {
}

s32 osPfsFindFile(OSPfs* pfs, u16 companyCode, u32 gameCode, u8* gameName, u8* extName, s32* fileNo) {
}

s32 osPfsAllocateFile(OSPfs* pfs, u16 companyCode, u32 gameCode, u8* gameName, u8* extName, s32 length, s32* fileNo) {
}

OSIntMask osSetIntMask(OSIntMask a) {
    return 0;
}

s32 osAfterPreNMI(void) {
    return 0;
}

s32 osProbeRumblePak(OSMesgQueue* ctrlrqueue, OSPfs* pfs, u32 channel) {
}

s32 osSetRumble(OSPfs* pfs, u32 vibrate) {
    return 0;
}

void osCreateThread(OSThread* thread, OSId id, void (*entry)(void*), void* arg, void* sp, OSPri pri) {
}

void osStartThread(OSThread* thread) {
}

void osStopThread(OSThread* thread) {
}

void osDestroyThread(OSThread* thread) {
}

void osWritebackDCache(void* vaddr, s32 nbytes) {
}

void osInvalICache(void* vaddr, s32 nbytes) {
}

s32 osContStartQuery(OSMesgQueue* mq) {
}

void osContGetQuery(OSContStatus* data) {
}

u32 __osGetFpcCsr() {
    return 0;
}

void __osSetFpcCsr(u32 a0) {
}

s32 __osDisableInt(void) {
}

void __osRestoreInt(s32 a0) {
}

OSThread* __osGetActiveQueue(void) {
}

OSThread* __osGetCurrFaultedThread(void) {
}

u32 osMemSize = 1024 * 1024 * 1024;

void Audio_osInvalDCache(void* buf, s32 size) {
}

void Audio_osWritebackDCache(void* mem, s32 size) {
}

s32 osAiSetFrequency(u32 freq) {
    // this is based off the math from the original method
    /*

    s32 osAiSetFrequency(u32 frequency) {
        u8 bitrate;
        f32 dacRateF = ((f32)osViClock / frequency) + 0.5f;
        u32 dacRate = dacRateF;

        if (dacRate < 132) {
            return -1;
        }

        bitrate = (dacRate / 66);
        if (bitrate > 16) {
            bitrate = 16;
        }

        HW_REG(AI_DACRATE_REG, u32) = dacRate - 1;
        HW_REG(AI_BITRATE_REG, u32) = bitrate - 1;
        return osViClock / (s32)dacRate;
    }

    */

    // bitrate is unused

    // osViClock comes from
    // #define VI_NTSC_CLOCK 48681812 /* Hz = 48.681812 MHz */
    // s32 osViClock = VI_NTSC_CLOCK;

    // frequency was originally 32000

    // given all of that, dacRate is
    // (u32)(((f32)48681812 / 32000) + 0.5f)
    // which evaluates to 1521 (which is > 132)

    // this leaves us with a final calculation of
    // 48681812 / 1521
    // which evaluates to 32006

    return 32006;
}

void osInvalDCache(void* vaddr, s32 nbytes) {
}

void osWritebackDCacheAll(void) {
}

void Audio_SetBGM(u32 bgmId) {
}

s32 osContSetCh(u8 ch) {
}

u32 osDpGetStatus(void) {
}

void osDpSetStatus(u32 status) {
}

u32 __osSpGetStatus() {
}

void __osSpSetStatus(u32 status) {
}

OSPiHandle* osDriveRomInit() {
}

void __osInitialize_common(void) {
}

void __osInitialize_autodetect(void) {
}

void __osExceptionPreamble() {
}

void __osCleanupThread(void) {
}

s32 _Printf(PrintCallback a, void* arg, const char* fmt, va_list ap) {
    unsigned char buffer[4096];

    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    a(arg, buffer, strlen(buffer));
}

void osSpTaskLoad(OSTask* task) {
}

void osSpTaskStartGo(OSTask* task) {
}

void osSetUpMempakWrite(s32 channel, OSPifRam* buf) {
}

u32 osGetMemSize(void) {
    return 1024 * 1024 * 1024;
}

s32 osEPiReadIo(OSPiHandle* handle, u32 devAddr, u32* data) {
    return 0;
}

void osSpTaskYield(void) {
}

s32 osStopTimer(OSTimer* timer) {
}

OSYieldResult osSpTaskYielded(OSTask* task) {
}

void osViExtendVStart(u32 arg0) {
}
