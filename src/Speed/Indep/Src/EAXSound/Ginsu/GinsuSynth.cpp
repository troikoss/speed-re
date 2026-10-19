// GinsuSynthesis: packet state, the scale of a data block to packets, frequency update and the packet
// release (crossfade, jump and submit). Recovered from speed2.exe; addresses cited per routine.
#include "Ginsu.h"

// The packet record submitted with the snd library call (frame offsets 0x3C..0x4B of 0x466d30).
// Offsets 0x3C and 0x44 are not written by the original; the rebuild zero-initialises them.
struct GinsuPacket {
    int unidentified0;
    unsigned int key;
    int unidentified8;
    short *samples;
};

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
    GinsuAudioLock();
    noJumpSize10 = a;
    packetSize14 = b;
    overlapSize18 = c;
    synthData28 = &data;
    GinsuAudioUnlock();
    return true;
}

// 0x467230: target sample from the frequency, then the packet countdown from latency.
bool GinsuSynthesis::UpdateFrequency(float targetFrequency, float latency) {
    const int targetSample = synthData28->FrequencyToSample(targetFrequency);
    GinsuAudioLock();
    targetPos34 = targetSample;
    volatile float x = latency / (ginsuRateScaleB788894 * ginsuThousand784550);
    const int r = GinsuRoundNearest(x);
    volatile float d = x - (float)r;
    packetCountdown38 = r + GinsuSignMask(d) + 1;
    GinsuAudioUnlock();
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
    GinsuPacket packet = {0, key, 0, samples};
    GinsuAudioLock();
    GinsuAudioSubmit(packetHandle1C, &packet);
    GinsuAudioUnlock();
}
