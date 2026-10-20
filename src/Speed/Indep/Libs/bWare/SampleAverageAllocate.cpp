#include "TimedAverage.h"
extern char sampleAverageVtable[];
extern "C" void * __cdecl malloc(unsigned int size);

// 0x576880: pointer/backing samples are not initialized here.
SampleAverage *SampleAverage::ConstructDefault576880() {
    value4 = 4;
    capacity5 = count6 = next7 = 0;
    vtable0 = sampleAverageVtable;
    sum8 = averageC = 0.0f;
    return this;
}

// 0x5768a0: preserves count, next, sum and average; capacities are unchecked.
void SampleAverage::Allocate5768A0(int capacity) {
    capacity5 = (unsigned char)capacity;
    samples10 = inlineSamples14;
    if (capacity > 5) samples10 = (float *)malloc((unsigned int)capacity * 4u);
    for (int i = 0; i < capacity; ++i) samples10[i] = 0.0f;
}
