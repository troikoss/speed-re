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

// snd library seam (retail addresses in speed2.exe). Every entry point is
// cdecl and the caller pops its arguments. Retail 0x721f99 and 0x721fa7 take the critical section; the
// packet player is 0x7231d4 start, 0x723515 submit, 0x72387b stop, 0x72366a destroy, 0x723138 create and
// 0x723124 overhead. SND_attrsetdef (0x72423e) and SNDplaysetdef (0x721a6b) fill the option structs below.
// Platform implementation: src/Platform/Native/NativeAudio.cpp (SDL2). Not recovered: the snd library itself.
void SNDSYS_entercritical();   // 0x721f99
void SNDSYS_leavecritical();   // 0x721fa7

// SNDSAMPLEFORMAT (4 bytes). StartSynthesis passes {rate, 1 channel, samplerep 8}.
struct GinsuSndFormat {
    unsigned short samplerate;  // 0x00: sampleRate18, low 16 bits
    unsigned char channels;     // 0x02: 1
    unsigned char samplerep;    // 0x03: 8 in UG2
};

// SNDSAMPLEATTR (0x68 bytes), filled by SND_attrsetdef. StartSynthesis then sets rendermode (0x08) to 4.
struct GinsuSndAttr {
    short detune;               // 0x00
    signed char priority;       // 0x02
    signed char vol;            // 0x03: 0x7f from SND_attrsetdef
    signed char pan;            // 0x04: 0x40 from SND_attrsetdef
    signed char fxlevel0;       // 0x05
    signed char bendrange;      // 0x06
    unsigned char platformver;  // 0x07
    unsigned short rendermode;  // 0x08
    unsigned char padchar[2];   // 0x0A: not written by SND_attrsetdef
    unsigned short azimuth[6];  // 0x0C
    void *ptsdata[6];           // 0x18
    int tsdatasize[6];          // 0x30: not written by SND_attrsetdef
    void *puserdata[4];         // 0x48
    int userdatasize[4];        // 0x58
};
typedef char GinsuSndAttrSizeCheck[sizeof(GinsuSndAttr) == 0x68 ? 1 : -1];

// SNDPLAYOPTS (0x18 bytes), filled by SNDplaysetdef. StartSynthesis then sets vol (0x00) to 0.
struct GinsuSndPlayOpts {
    signed char vol;               // 0x00
    signed char bend;              // 0x01
    signed char keynum;            // 0x02
    signed char velocity;          // 0x03
    signed char drylevel;          // 0x04
    signed char fxlevel0;          // 0x05
    unsigned char pad[2];          // 0x06: not written by SNDplaysetdef
    unsigned short azimuth;        // 0x08
    short elevation;               // 0x0A
    unsigned short pitchmult;      // 0x0C: 0x1000
    unsigned short timemult;       // 0x0E: 0x1000
    unsigned short tempomult;      // 0x10: 0x1000
    unsigned short pad2;           // 0x12: not written by SNDplaysetdef
    unsigned short lowpasscutoff;  // 0x14: 0xffff
    unsigned short highpasscutoff; // 0x16: 0
};
typedef char GinsuSndPlayOptsSizeCheck[sizeof(GinsuSndPlayOpts) == 0x18 ? 1 : -1];

// SNDPACKET (size 0x24). Ginsu writes the frame word (0x04) and psamples[0] (0x0C). ssf and sizeofsamples are
// not written by the original (their stack words are uninitialised), so the rebuild writes zero there.
struct GinsuSndPacket {
    GinsuSndFormat ssf;         // 0x00
    unsigned int numframes;     // 0x04: frame count in bits 0..30, continuation in bit 31
    int sizeofsamples;          // 0x08
    void *psamples[6];          // 0x0C: psamples[0] is the sample buffer of the packet
};

int SNDPKTPLAY_overhead(int maxpackets);                                                            // 0x723124
int SNDPKTPLAY_create(void (*preleasefunc)(void *, void *), void (*pframesfunc)(int, int, void *),
                      void *pclientdata, void *pmem, int memsize);                                  // 0x723138
int SNDPKTPLAY_start(int packetinstancehandle, GinsuSndFormat *pssf, GinsuSndAttr *pssa,
                     GinsuSndPlayOpts *pspo);                                                       // 0x7231d4
int SNDPKTPLAY_submit(int packetinstancehandle, GinsuSndPacket *psp);                               // 0x723515
int SNDPKTPLAY_stop(int packetinstancehandle);                                                      // 0x72387b
int SNDPKTPLAY_destroy(int packetinstancehandle);                                                   // 0x72366a
int SND_attrsetdef(GinsuSndAttr *pssa);                                                             // 0x72423e
int SNDplaysetdef(GinsuSndPlayOpts *pspo);                                                          // 0x721a6b

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

// Packet synthesizer state (GinsuSynthesis, size 0x44). The retail vtable word 0x78b71c is kept raw (0x00).
class GinsuSynthesis {
public:
    GinsuSynthesis(void *memblock, int size);                      // 0x477800 (thiscall, RET 8)
    ~GinsuSynthesis();                                             // 0x477870
    bool SetSynthData(GinsuSynthData &data);                      // 0x467160
    bool UpdateFrequency(float targetFrequency, float latency);   // 0x467230
    void HandlePacketRelease(short *samples);                     // 0x466d30
    int StartSynthesis(float startFrequency);                     // 0x4778c0 (thiscall, RET 4)
    static void PacketReleaseCallback(void *samples, void *clientdata);  // 0x4777f0 (cdecl)

    void *vtable0;                       // 0x00: retail vtable 0x78b71c, raw word
    short *packetData[2];                // 0x04, 0x08: sample buffers of the two packets (memblock, memblock + 4n)
    int maxPacketSizeC;                  // 0x0C
    int noJumpSize10;                    // 0x10
    int packetSize14;                    // 0x14
    int overlapSize18;                   // 0x18
    int packetHandle1C;                  // 0x1C: SNDPKTPLAY_create handle, -1 until created
    int sndHandle;                       // 0x20: SNDPKTPLAY_start result
    int sampleRate24;                    // 0x24: rate of the started synthesis, 0 when stopped
    GinsuSynthData *synthData28;         // 0x28
    int playbackPos2C;                   // 0x2C
    int currentPos30;                    // 0x30
    int targetPos34;                     // 0x34
    int packetCountdown38;               // 0x38
    int noJumpRemaining3C;               // 0x3C
    float currentCycle40;                // 0x40
};

#endif
