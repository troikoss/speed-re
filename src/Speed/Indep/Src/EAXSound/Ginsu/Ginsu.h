// Ginsu granular engine-sound synthesizer of EA SPEED, recovered from UG2 PC v1.2 (speed2.exe).
// The layouts and arithmetic were confirmed in the retail binary at the addresses cited on each member.
// Unknown fields are offset-named.
#ifndef UG2_EAXSOUND_GINSU_H
#define UG2_EAXSOUND_GINSU_H

// Retail globals (speed2.exe addresses; mapped for the native build in OriginalSymbols.ld).
extern float ginsuZero7D7D3C;               // 0x7d7d3c: 0.0f
extern float ginsuOne784250;                // 0x784250: 1.0f
extern float ginsuHalf784260;               // 0x784260: 0.5f, CyclePeriod scale
extern float ginsuThousand784550;           // 0x784550: 1000.0f, UpdateFrequency
extern float ginsuRateScaleA788890;         // 0x788890: packet scale, no-jump size
extern float ginsuRateScaleB788894;         // 0x788894: packet scale, packet size (same value as A)
extern float ginsuRateScaleC788898;         // 0x788898: packet scale, overlap size
extern float ginsuFilterA7ED92C[16];        // 0x7ed92c: EA-XA predictor table, weights the newer sample
extern float ginsuFilterB7ED93C[16];        // 0x7ed93c: second predictor table, weights the older sample
extern float ginsuStepTable7ED950[16][16];  // 0x7ed950: step table [shift][nibble]

// snd library entry points used when a packet is submitted (retail 0x721f99 lock, 0x721fa7 unlock,
// 0x723515 submit; all cdecl).
void GinsuAudioLock(void);
void GinsuAudioUnlock(void);
void GinsuAudioSubmit(unsigned int handle, void *packet);

// 0x466560 (cdecl, one float argument): the ceiling of the value, see GinsuCeilOf.
int GinsuCeilToInt(float value);

// x87 idioms of the original. FIST/FISTP round to nearest; the sign bit of the stored difference
// (value - round(value)) corrects the rounding to a floor or a ceiling.
static inline int GinsuRoundNearest(float value) {
    int result;
#ifdef _MSC_VER
    __asm {
        fld dword ptr value
        fistp dword ptr result
    }
#else
    __asm__ __volatile__("fistpl %0" : "=m"(result) : "t"(value) : "st");
#endif
    return result;
}

// -1 when the sign bit of the float is set, else 0 (the original's SAR 31 of the float bits).
static inline int GinsuSignMask(float value) {
    union {
        float f;
        unsigned int u;
    } bits;
    bits.f = value;
    return (int)bits.u >> 31;
}

// floor(value) = r + sign(value - r), r = round-to-nearest(value)  (FIST, FISUB, SAR, ADD).
static inline int GinsuFloorOf(float value) {
    const int r = GinsuRoundNearest(value);
    volatile float d = value - (float)r;
    return r + GinsuSignMask(d);
}

// ceil(value) = r - sign(r - value), r = round-to-nearest(value)  (FIST, FISUBR, SAR, SUB).
static inline int GinsuCeilOf(float value) {
    const int r = GinsuRoundNearest(value);
    volatile float d = (float)r - value;
    return r - GinsuSignMask(d);
}

// Layout of the granular data block (GinsuSynthData, size 0xB0) and its decoder state.
class GinsuSynthData {
public:
    GinsuSynthData();                                      // 0x466750
    bool BindToData(void *dataPointer);                    // 0x466770
    int FrequencyToSample(float frequency);                // 0x466840
    int CycleToSample(float cycle);                        // 0x466910
    float CyclePeriod(float cycle);                        // 0x4669d0, result left in ST0 unrounded
    float SampleToCycle(int sample);                       // 0x466af0..0x466c52, result left in ST0
    void DecodeBlock(int block);                           // 0x466590
    bool GetSamples(int start, int count, short *dest);    // 0x466c60

    void *vtable0;                  // 0x00: retail vtable 0x789ec8, written by the constructor
    float minFrequency4;            // 0x04
    float maxFrequency8;            // 0x08
    int segCountC;                  // 0x0C
    int cycleCount10;               // 0x10
    int sampleCount14;              // 0x14
    int sampleRate18;               // 0x18
    int *freqPos1C;                 // 0x1C: segCount + 1 ints
    int *cyclePos20;                // 0x20: cycleCount + 1 ints
    unsigned char *sampleData24;    // 0x24: 19-byte EA-XA blocks
    float minPeriod28;              // 0x28
    float sample2C[32];             // 0x2C: decoded samples of the current block (0 and 1: header history)
    int currentBlockAC;             // 0xAC: -1 until a block is decoded
};

// Packet synthesizer state (GinsuSynthesis). Offsets 0x00..0x0B are not identified.
class GinsuSynthesis {
public:
    bool SetSynthData(GinsuSynthData &data);                      // 0x467160
    bool UpdateFrequency(float targetFrequency, float latency);   // 0x467230
    void HandlePacketRelease(short *samples);                     // 0x466d30

    unsigned char unidentified0[0x0C];   // 0x00
    int maxPacketSizeC;                  // 0x0C
    int noJumpSize10;                    // 0x10
    int packetSize14;                    // 0x14
    int overlapSize18;                   // 0x18
    unsigned int packetHandle1C;         // 0x1C: handle passed to the snd submit call
    unsigned char unidentified20[0x04];  // 0x20: not identified, offset kept so the fields below line up
    int sampleRate24;                    // 0x24
    GinsuSynthData *synthData28;         // 0x28
    int playbackPos2C;                   // 0x2C
    int currentPos30;                    // 0x30
    int targetPos34;                     // 0x34
    int packetCountdown38;               // 0x38
    int noJumpRemaining3C;               // 0x3C
    float currentCycle40;                // 0x40
};

#endif
