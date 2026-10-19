// GinsuSynthData: the granular data block ("Gnsu2"), the frequency, cycle and period lookups, the
// EA-XA block decoder and the sample fetch. Each routine is recovered from speed2.exe at the address cited.
#include "Ginsu.h"

static unsigned int GinsuWord(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8) | ((unsigned int)p[2] << 16) |
           ((unsigned int)p[3] << 24);
}

static float GinsuBitsToFloat(unsigned int bits) {
    union {
        float f;
        unsigned int u;
    } value;
    value.u = bits;
    return value.f;
}

// FISTP word with the original's saturation: an out-of-range result (0x8000) takes the sign of the value.
static short GinsuSampleToShort(float value) {
    const int r = GinsuRoundNearest(value);
    if (r >= -32768 && r <= 32767) {
        return (short)r;
    }
    return GinsuSignMask(value) ? (short)-32768 : (short)32767;
}

// 0x466560 (cdecl): the ceiling helper; HandlePacketRelease calls it when z1 < 0.
int GinsuCeilToInt(float value) {
    return GinsuCeilOf(value);
}

// 0x466750: the vtable word is the retail vtable 0x789ec8 (the repo keeps retail vtable words raw).
GinsuSynthData::GinsuSynthData() {
    vtable0 = (void *)0x789ec8;
    minFrequency4 = 0.0f;
    maxFrequency8 = 0.0f;
    segCountC = 0;
    cycleCount10 = 0;
    sampleCount14 = 0;
    sampleRate18 = 0;
}

// 0x466770: magic "Gnsu2" (five bytes, byte 5 is not checked), header words, then the tables.
bool GinsuSynthData::BindToData(void *dataPointer) {
    const unsigned char *bytes = (const unsigned char *)dataPointer;
    minFrequency4 = 0.0f;
    maxFrequency8 = 0.0f;
    segCountC = 0;
    cycleCount10 = 0;
    sampleCount14 = 0;
    sampleRate18 = 0;
    if (bytes[0] != 'G' || bytes[1] != 'n' || bytes[2] != 's' || bytes[3] != 'u' || bytes[4] != '2') {
        return false;
    }
    minFrequency4 = GinsuBitsToFloat(GinsuWord(bytes + 0x08));
    maxFrequency8 = GinsuBitsToFloat(GinsuWord(bytes + 0x0C));
    segCountC = (int)GinsuWord(bytes + 0x10);
    cycleCount10 = (int)GinsuWord(bytes + 0x14);
    sampleCount14 = (int)GinsuWord(bytes + 0x18);
    sampleRate18 = (int)GinsuWord(bytes + 0x1C);
    freqPos1C = (int *)(bytes + 0x20);
    cyclePos20 = freqPos1C + segCountC + 1;
    sampleData24 = (unsigned char *)(cyclePos20 + cycleCount10 + 1);
    currentBlockAC = -1;
    int minimum = sampleCount14;
    for (int i = 0; i < cycleCount10; ++i) {
        const int diff = cyclePos20[i + 1] - cyclePos20[i];
        if (diff < minimum) {
            minimum = diff;
        }
    }
    minPeriod28 = (float)minimum;
    return true;
}

// 0x466840
int GinsuSynthData::FrequencyToSample(float frequency) {
    if (segCountC < 1) {
        return 0;
    }
    if (frequency <= minFrequency4) {
        return freqPos1C[0];
    }
    if (frequency >= maxFrequency8) {
        return freqPos1C[segCountC];
    }
    volatile float t = ((frequency - minFrequency4) * (float)segCountC) / (maxFrequency8 - minFrequency4);
    const int idx = GinsuFloorOf(t);
    volatile float v = (float)(freqPos1C[idx + 1] - freqPos1C[idx]) * (t - (float)idx) + (float)freqPos1C[idx];
    return GinsuRoundNearest(v);
}

// 0x466910
int GinsuSynthData::CycleToSample(float cycle) {
    if (cycleCount10 < 1) {
        return 0;
    }
    if (cycle <= ginsuZero7D7D3C) {
        return cyclePos20[0];
    }
    if ((float)cycleCount10 <= cycle) {
        return cyclePos20[cycleCount10];
    }
    const int idx = GinsuFloorOf(cycle);
    volatile float v = (float)(cyclePos20[idx + 1] - cyclePos20[idx]) * (cycle - (float)idx) +
                       (float)cyclePos20[idx];
    return GinsuRoundNearest(v);
}

// 0x4669d0: returns the period with the result left in ST0 (unrounded, as the original).
float GinsuSynthData::CyclePeriod(float cycle) {
    const int count = cycleCount10;
    if (count < 1) {
        return ginsuZero7D7D3C;
    }
    int idx = GinsuFloorOf(cycle);
    const int *p = cyclePos20;
    if (idx < 1) {
        if (idx < 0) {
            cycle = 0.0f;
            idx = 0;
        }
        return (float)(p[1] - p[0]) + (cycle - (float)idx) * ((float)(p[2] - p[0]) * ginsuHalf784260 - (float)(p[1] - p[0]));
    }
    if (idx < count - 1) {
        return (float)(p[idx + 1] - p[idx - 1]) * ginsuHalf784260 +
               (cycle - (float)idx) * ((float)(p[idx + 2] - p[idx]) * ginsuHalf784260 -
                                       (float)(p[idx + 1] - p[idx - 1]) * ginsuHalf784260);
    }
    if (idx >= count) {
        cycle = (float)count;
        idx = count - 1;
    }
    return (float)(p[count] - p[count - 2]) * ginsuHalf784260 +
           (cycle - (float)idx) * ((float)(p[count] - p[count - 1]) -
                                   (float)(p[count] - p[count - 2]) * ginsuHalf784260);
}

// 0x466af0..0x466c52: binary search of the cycle table. Found path returns with ST0 unrounded.
// Found path: the original's FSTP ST(1) at 0x466c4d (DD D9) leaves only the result in ST0; the x87 stack depth is unchanged by the call.
float GinsuSynthData::SampleToCycle(int sample) {
    const int count = cycleCount10;
    if (count < 1) {
        return ginsuZero7D7D3C;
    }
    const int *cyc = cyclePos20;
    if (!(sample > cyc[0])) {
        return ginsuZero7D7D3C;
    }
    if (sample >= cyc[count]) {
        return (float)count;
    }
    int lo = 0;
    int hi = count;
    for (;;) {
        volatile float v = ((float)(sample - cyc[lo]) / (float)(cyc[hi] - cyc[lo])) * (float)(hi - lo);
        const int idx = lo + GinsuFloorOf(v);
        if (sample < cyc[idx]) {
            volatile float q = (float)(cyc[idx] - sample) / minPeriod28;
            const int cand = idx - GinsuCeilOf(q);
            if (cand > lo) {
                lo = cand;
            }
            hi = idx;
        } else if (sample < cyc[idx + 1]) {
            return ((float)(sample - cyc[idx]) / (float)(cyc[idx + 1] - cyc[idx])) + (float)idx;
        } else {
            volatile float q = (float)(sample - cyc[idx]) / minPeriod28;
            lo = idx + 1;
            const int cand = idx + 1 + GinsuCeilOf(q);
            if (cand < hi) {
                hi = cand;
            }
        }
    }
}

static float GinsuPredictEven(float c1, float c2, float a, float b, float h) { return c1 * a + (c2 * b + h); }
static float GinsuPredictHigh(float c1, float c2, float a, float b, float h) { return (c1 * a + h) + c2 * b; }
static float GinsuOddEven(float c1, float c2, float a, float b, float h, float l) { return (c2 * a + l) + c1 * (c1 * a + (c2 * b + h)); }
static float GinsuOddHigh(float c1, float c2, float a, float b, float h, float l) { return (c2 * a + l) + c1 * ((c1 * a + h) + c2 * b); }
static float GinsuOddLate(float c1, float c2, float a, float b, float h, float l) { return (c1 * (c1 * a + (c2 * b + h)) + l) + c2 * a; }

// 0x466590: decode one 19-byte block into sample2C[2..31] (sample2C[0], [1] are the header history).
// Samples come in groups of five bytes; each byte holds two 4-bit codes (high nibble first). The
// per-sample association differs between the high and low nibble and between groups; keep it exactly.
void GinsuSynthData::DecodeBlock(int block) {
    currentBlockAC = block;
    const unsigned char *data = sampleData24 + block * 19;
    sample2C[0] = (float)((data[0] & 0xF0) + (int)(signed char)data[1] * 256);
    sample2C[1] = (float)((data[2] & 0xF0) + (int)(signed char)data[3] * 256);
    const int filter = data[0] & 0x0F;
    const float c1 = ginsuFilterA7ED92C[filter];
    const float c2 = ginsuFilterB7ED93C[filter];
    const int row = data[2] & 0x0F;
    float *s = sample2C;
    for (int group = 0; group < 3; ++group) {
        const unsigned char *g = data + 4 + 5 * group;
        const int n = 2 + 10 * group;
        const float h0 = ginsuStepTable7ED950[row][g[0] >> 4];
        const float l0 = ginsuStepTable7ED950[row][g[0] & 0x0F];
        const float h1 = ginsuStepTable7ED950[row][g[1] >> 4];
        const float l1 = ginsuStepTable7ED950[row][g[1] & 0x0F];
        const float h2 = ginsuStepTable7ED950[row][g[2] >> 4];
        const float l2 = ginsuStepTable7ED950[row][g[2] & 0x0F];
        const float h3 = ginsuStepTable7ED950[row][g[3] >> 4];
        const float l3 = ginsuStepTable7ED950[row][g[3] & 0x0F];
        const float h4 = ginsuStepTable7ED950[row][g[4] >> 4];
        const float l4 = ginsuStepTable7ED950[row][g[4] & 0x0F];
        // byte 0: sample n (high nibble), sample n+1 (low nibble, uses the unrounded sample n)
        s[n]     = GinsuPredictEven(c1, c2, s[n - 1], s[n - 2], h0);
        s[n + 1] = GinsuOddEven(c1, c2, s[n - 1], s[n - 2], h0, l0);
        // byte 1
        s[n + 2] = GinsuPredictHigh(c1, c2, s[n + 1], s[n], h1);
        s[n + 3] = GinsuOddHigh(c1, c2, s[n + 1], s[n], h1, l1);
        // byte 2
        s[n + 4] = GinsuPredictEven(c1, c2, s[n + 3], s[n + 2], h2);
        s[n + 5] = GinsuOddLate(c1, c2, s[n + 3], s[n + 2], h2, l2);
        // byte 3
        s[n + 6] = GinsuPredictEven(c1, c2, s[n + 5], s[n + 4], h3);
        s[n + 7] = GinsuOddLate(c1, c2, s[n + 5], s[n + 4], h3, l3);
        // byte 4
        s[n + 8] = GinsuPredictEven(c1, c2, s[n + 7], s[n + 6], h4);
        s[n + 9] = GinsuOddLate(c1, c2, s[n + 7], s[n + 6], h4, l4);
    }
}

// 0x466c60: copy count samples from start into dest (int16, saturated). Decodes the block of start if it
// is not current, and decodes the next block eagerly when the read reaches the end of a block.
bool GinsuSynthData::GetSamples(int start, int count, short *dest) {
    if (start < 0) {
        return false;
    }
    if (start + count - 1 >= sampleCount14) {
        return false;
    }
    const int block = start >> 5;
    if (block != currentBlockAC) {
        DecodeBlock(block);
    }
    int pos = start & 31;
    for (int i = 0; i < count; ++i) {
        dest[i] = GinsuSampleToShort(sample2C[pos]);
        ++pos;
        if (pos == 32) {
            DecodeBlock(currentBlockAC + 1);
            pos = 0;
        }
    }
    return true;
}
