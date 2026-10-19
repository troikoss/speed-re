// GinsuSynthesis: packet state, the scale of a data block to packets, frequency update, the packet release
// (crossfade, jump and submit), and the construction, start and destruction of the packet player. Recovered from
// speed2.exe; addresses cited per routine. The snd calls are the seam declared in Ginsu.h.
#include "Ginsu.h"

// Clears count bytes like the original STOSD/STOSB pair. The volatile stores keep the compiler from turning the
// loop into a memset call (the verifier links no C runtime).
static void GinsuZeroBytes(void *dest, unsigned int count) {
    volatile unsigned char *p = (volatile unsigned char *)dest;
    for (unsigned int i = 0; i < count; ++i) {
        p[i] = 0;
    }
}

// The SNDPACKET record of the submit calls (size 0x24). Only the frame word and psamples[0] are written by the
// original; the other words are uninitialised there and are zero in the rebuild.
static GinsuSndPacket GinsuMakePacket(unsigned int key, short *samples) {
    GinsuSndPacket packet;
    packet.ssf.samplerate = 0;
    packet.ssf.channels = 0;
    packet.ssf.samplerep = 0;
    packet.numframes = key;
    packet.sizeofsamples = 0;
    for (int i = 0; i < 6; ++i) {
        packet.psamples[i] = 0;
    }
    packet.psamples[0] = samples;
    return packet;
}

// 0x4777f0 (cdecl): the release callback handed to SNDPKTPLAY_create. The player calls it with the samples of a
// packet it has finished with and with the GinsuSynthesis that created the player (clientdata).
void GinsuSynthesis::PacketReleaseCallback(void *samples, void *clientdata) {
    GinsuSynthesis *object = static_cast<GinsuSynthesis *>(clientdata);
    object->HandlePacketRelease(static_cast<short *>(samples));
}

// 0x477800 (thiscall, memblock and size on the stack). Two packet buffers of 2n shorts come first (4n bytes each),
// then the player's own block: n = (size - overhead) >> 3 with overhead = SNDPKTPLAY_overhead(8). The player is
// created only when 2n >= 0x50; otherwise packetHandle1C stays -1.
GinsuSynthesis::GinsuSynthesis(void *memblock, int size) {
    vtable0 = (void *)0x78b71c;
    packetHandle1C = -1;
    sampleRate24 = 0;
    synthData28 = 0;
    const int overhead = SNDPKTPLAY_overhead(8);
    const unsigned int n = (unsigned int)(size - overhead) >> 3;
    maxPacketSizeC = (int)(n << 1);
    if (maxPacketSizeC >= 0x50) {
        char *base = static_cast<char *>(memblock);
        packetData[0] = reinterpret_cast<short *>(base);
        packetData[1] = reinterpret_cast<short *>(base + 4 * n);
        packetHandle1C = SNDPKTPLAY_create(PacketReleaseCallback, 0, this, base + 8 * n, overhead);
    }
}

// 0x477870: a started player is stopped first (under the critical section), then destroyed. The retail destructor
// writes the base-class vtable word 0x789ce4 last.
GinsuSynthesis::~GinsuSynthesis() {
    vtable0 = (void *)0x78b71c;
    if (sampleRate24 != 0) {
        SNDSYS_entercritical();
        SNDPKTPLAY_stop(packetHandle1C);
        SNDSYS_leavecritical();
        sampleRate24 = 0;
    }
    SNDPKTPLAY_destroy(packetHandle1C);
    vtable0 = (void *)0x789ce4;
}

// 0x467160: the packet sizes are the data sample rate times the three scales (rounded to int).
bool GinsuSynthesis::SetSynthData(GinsuSynthData &data) {
    const int rate = data.sampleRate18;
    if (rate == 0) {
        return false;
    }
    if (sampleRate24 > 0 && rate != sampleRate24) {
        return false;
    }
    volatile float rateF = (float)rate;
    volatile float x0 = (float)rate * ginsuRateScaleA788890;
    const int a = GinsuRoundNearest(x0);
    volatile float x1 = rateF * ginsuRateScaleB788894;
    const int b = GinsuRoundNearest(x1);
    volatile float x2 = rateF * ginsuRateScaleC788898;
    const int c = GinsuRoundNearest(x2);
    if (c + b > maxPacketSizeC) {
        return false;
    }
    SNDSYS_entercritical();
    noJumpSize10 = a;
    packetSize14 = b;
    overlapSize18 = c;
    synthData28 = &data;
    SNDSYS_leavecritical();
    return true;
}

// 0x467230: target sample from the frequency, then the packet countdown from latency.
bool GinsuSynthesis::UpdateFrequency(float targetFrequency, float latency) {
    const int targetSample = synthData28->FrequencyToSample(targetFrequency);
    SNDSYS_entercritical();
    targetPos34 = targetSample;
    volatile float x = latency / (ginsuRateScaleB788894 * ginsuThousand784550);
    const int r = GinsuRoundNearest(x);
    volatile float d = x - (float)r;
    packetCountdown38 = r + GinsuSignMask(d) + 1;
    SNDSYS_leavecritical();
    return true;
}

// 0x466d30: release one packet of packetSize14 samples into samples[]. The crossfade branch (k != 0)
// writes overlap samples of the old path, then blends them with the samples from the jumped-to position.
void GinsuSynthesis::HandlePacketRelease(short *samples) {
    GinsuSynthData *data = synthData28;
    currentCycle40 = data->SampleToCycle(currentPos30);
    volatile float x1 = (float)(targetPos34 - currentPos30) / (float)packetCountdown38;
    int k = 0;
    float multiplier = ginsuOne784250;
    bool unroundedD = false;
    volatile float dc = 0.0f;
    volatile float y1 = 0.0f;
    volatile float qRounded = 0.0f;
    if (noJumpRemaining3C < packetSize14) {
        volatile float r1 = data->SampleToCycle(playbackPos2C);
        volatile float pktF = (float)packetSize14;
        volatile float x2 = pktF / data->CyclePeriod(r1);
        dc = currentCycle40 - r1;
        y1 = (x1 / data->CyclePeriod(currentCycle40)) - x2;
        volatile float z1 = ((float)noJumpRemaining3C / (float)packetSize14) * y1 + dc;
        volatile float z2 = y1 + dc;
        qRounded = (float)noJumpRemaining3C / (float)packetSize14;
        const int fz1 = GinsuFloorOf(z1);
        const int fz2 = GinsuFloorOf(z2);
        if (fz1 != fz2) {
            k = (z1 < z2) ? GinsuCeilOf(z1) : GinsuFloorOf(z1);
            unroundedD = (k != 0);
        } else {
            volatile float wp = (ginsuRateScaleA788890 / ginsuRateScaleB788894) * y1;
            volatile float w = GinsuSignMask(wp) ? -wp : wp;
            const int rw = GinsuRoundNearest(w);
            volatile float dw = (float)rw - w;
            const int cw = rw - GinsuSignMask(dw);
            const float az1 = GinsuSignMask(z1) ? -z1 : z1;
            if ((float)cw < az1) {
                k = (z1 < 0.0f) ? GinsuCeilToInt(z1) : GinsuFloorOf(z1);
                multiplier = (k != 0) ? (float)qRounded : ginsuOne784250;
            }
        }
    }
    volatile float product;
    if (unroundedD) {
        product = (float)packetSize14 * (((float)k - dc) / y1);
    } else {
        product = (float)packetSize14 * multiplier;
    }
    const int n = GinsuRoundNearest(product);

    data->GetSamples(playbackPos2C, n, samples);
    playbackPos2C += n;
    noJumpRemaining3C -= n;
    if (noJumpRemaining3C < 0) {
        noJumpRemaining3C = 0;
    }
    int spliceCount = n;
    if (k != 0) {
        short *out = samples + n;
        data->GetSamples(playbackPos2C, overlapSize18, out);
        volatile float u = (float)k + data->SampleToCycle(playbackPos2C);
        playbackPos2C = data->CycleToSample(u);
        short buf[256];
        data->GetSamples(playbackPos2C, overlapSize18, buf);
        volatile float step2 = ginsuOne784250 / (float)overlapSize18;
        volatile float acc = 0.0f;
        for (int i = 0; i < overlapSize18; ++i) {
            const int a = out[i];
            const int b = buf[i];
            volatile float val = (float)(b - a) * acc + (float)a;
            out[i] = (short)GinsuRoundNearest(val);
            acc = step2 + acc;
        }
        const int written = overlapSize18 + n;
        playbackPos2C += overlapSize18;
        noJumpRemaining3C = noJumpSize10 - overlapSize18;
        const int rem = packetSize14 - written;
        if (rem > 0) {
            data->GetSamples(playbackPos2C, rem, samples + written);
            playbackPos2C += rem;
            noJumpRemaining3C -= rem;
            spliceCount = packetSize14;
        } else {
            spliceCount = written;
        }
    }
    volatile float posF = (float)currentPos30 + x1;
    currentPos30 = GinsuRoundNearest(posF);
    if (packetCountdown38 > 1) {
        packetCountdown38 -= 1;
    }
    // The original keys the packet with an uninitialised frame word (bit 31); modelled here as 0.
    unsigned int key = 0;
    key ^= (key ^ (unsigned int)spliceCount) & 0x7fffffffu;
    GinsuSndPacket packet = GinsuMakePacket(key, samples);
    SNDSYS_entercritical();
    SNDPKTPLAY_submit(packetHandle1C, &packet);
    SNDSYS_leavecritical();
}

// 0x4778c0 (thiscall, one float on the stack). The format is the data block rate with one channel and samplerep 8;
// the attributes are the SND_attrsetdef defaults with rendermode 4, and the options are the SNDplaysetdef defaults
// with volume 0. Under the critical section the player starts at the sample of startFrequency; on success both
// packet buffers are cleared, the rate is recorded and two packets are submitted. The return value is the start
// result (sndHandle), negative when refused.
int GinsuSynthesis::StartSynthesis(float startFrequency) {
    if (packetHandle1C < 0 || synthData28 == 0) {
        return -1;
    }
    if (sampleRate24 != 0) {
        SNDSYS_entercritical();
        SNDPKTPLAY_stop(packetHandle1C);
        SNDSYS_leavecritical();
        sampleRate24 = 0;
    }
    const int target = synthData28->FrequencyToSample(startFrequency);
    playbackPos2C = target;
    currentPos30 = target;
    targetPos34 = target;
    packetCountdown38 = 1;
    noJumpRemaining3C = noJumpSize10;
    currentCycle40 = synthData28->SampleToCycle(target);

    GinsuSndFormat format;
    format.samplerate = (unsigned short)synthData28->sampleRate18;
    format.channels = 1;
    format.samplerep = 8;
    GinsuSndAttr attr;
    SND_attrsetdef(&attr);
    attr.rendermode = 4;
    GinsuSndPlayOpts options;
    SNDplaysetdef(&options);
    options.vol = 0;

    SNDSYS_entercritical();
    sndHandle = SNDPKTPLAY_start(packetHandle1C, &format, &attr, &options);
    if (sndHandle >= 0) {
        GinsuZeroBytes(packetData[0], (unsigned int)packetSize14 * 2u);
        GinsuZeroBytes(packetData[1], (unsigned int)packetSize14 * 2u);
        sampleRate24 = synthData28->sampleRate18;
        // Same frame word as HandlePacketRelease: bit 31 is the uninitialised continuation, modelled as zero.
        unsigned int key = 0;
        key ^= (key ^ (unsigned int)packetSize14) & 0x7fffffffu;
        GinsuSndPacket packet = GinsuMakePacket(key, packetData[0]);
        SNDPKTPLAY_submit(packetHandle1C, &packet);
        packet.psamples[0] = packetData[1];
        SNDPKTPLAY_submit(packetHandle1C, &packet);
    }
    SNDSYS_leavecritical();
    return sndHandle;
}
