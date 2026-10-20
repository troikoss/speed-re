#include "DriveFeedbackCurve.h"

// Original 0x576770 as called from 0x5f6740 (ecx is the curve).
DriveFeedbackCurve *DriveFeedbackCurve::Construct(DriveFeedbackPair *pairs, int count) {
    this->pairs = pairs;
    this->count = count;
    return this;
}
