#include "TimedAverage.h"
extern float sampleAverageZero7D7D3C;
extern float sampleAverageOne784250;

// 0x576970: update the byte-indexed ring, retaining both sum stores.
void SampleAverage::AdvanceSample(float value) {
    if (count6 < capacity5) ++count6;
    sum8 = value + sum8;
    float *sample = samples10 + next7;
    sum8 = sum8 - *sample;
    *sample = value;
    ++next7;
    averageC = sum8 / count6;
    if (next7 >= capacity5) next7 = 0;
}

// 0x576a10: fill without changing the ring index.
void SampleAverage::FillSamples(float value) {
    for (int i = 0; i < capacity5; ++i) samples10[i] = value;
    count6 = capacity5;
    averageC = value;
    sum8 = (float)capacity5 * value;
}

// 0x576910: the initial multiplication propagates non-finite sums; it is
// not replaced by a clear. Re-read the byte capacity at each iteration.
void SampleAverage::RecalculateSamples() {
    sum8 = sum8 * sampleAverageZero7D7D3C;
    for (int i = 0; i < capacity5; ++i) sum8 = samples10[i] + sum8;
    const int count = count6 < 1 ? 1 : count6;
    averageC = (sampleAverageOne784250 / count) * sum8;
}
