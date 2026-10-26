#include "CarRenderBounds.h"
#ifdef _MSC_VER
extern "C" double fabs(double);
#pragma intrinsic(fabs)
#else
#include <math.h>
#endif

// Original 0x5bc210. The maximum product is rounded to float before comparison;
// the minimum product and six accumulators remain in x87 registers. Read all
// input components before storing either output, including overlapping records.
void CarRenderTransformBounds(const WheelMatrix *matrix,float *minimum,float *maximum) {
    const float *m = matrix->elements;
    long double minX=m[12],minY=m[13],minZ=m[14];
    long double maxX=m[12],maxY=m[13],maxZ=m[14];
    for (int i=0;i<3;++i) {
        long double low = static_cast<long double>(minimum[i])*m[i*4];
        volatile float high = static_cast<long double>(maximum[i])*m[i*4];
        if (low<high) { minX+=low; maxX+=high; }
        else { minX+=high; maxX+=low; }
        low = static_cast<long double>(minimum[i])*m[i*4+1];
        high = static_cast<long double>(maximum[i])*m[i*4+1];
        if (low<high) { minY+=low; maxY+=high; }
        else { minY+=high; maxY+=low; }
        low = static_cast<long double>(minimum[i])*m[i*4+2];
        high = static_cast<long double>(maximum[i])*m[i*4+2];
        if (low<high) { minZ+=low; maxZ+=high; }
        else { minZ+=high; maxZ+=low; }
    }
    minimum[0]=minX; minimum[1]=minY; minimum[2]=minZ;
    maximum[0]=maxX; maximum[1]=maxY; maximum[2]=maxZ;
}

// Original 0x5bc300 (__thiscall, RET 0xc). Returns 0 outside, 1 intersecting,
// 2 fully inside. Center/extents have explicit float stores before plane tests.
int CarRenderView::TestBounds(const float *minimum,const float *maximum,WheelMatrix *world) {
    float low[3] = {minimum[0],minimum[1],minimum[2]};
    float high[3] = {maximum[0],maximum[1],maximum[2]};
    if (world) CarRenderTransformBounds(world,low,high);
    volatile float center[3],extent[3];
    for (int i=0;i<3;++i) center[i]=(static_cast<long double>(high[i])+low[i])*0.5f;
    for (int i=0;i<3;++i) extent[i]=static_cast<long double>(high[i])-center[i];
    bool intersects=false;
    for (int i=0;i<6;++i) {
        const float *p=clip00->planesC0[i];
        long double x=fabs(static_cast<double>(p[0])),y=fabs(static_cast<double>(p[1])),z=fabs(static_cast<double>(p[2]));
        long double radius=(z*extent[2]+x*extent[0])+y*extent[1];
        long double distance=((static_cast<long double>(center[2])*p[2]+
            static_cast<long double>(center[0])*p[0])+static_cast<long double>(center[1])*p[1])+p[3];
        if (distance+radius<fengZero7D7D3C) return 0;
        if (distance-radius<fengZero7D7D3C) intersects=true;
    }
    return intersects ? 1 : 2;
}
