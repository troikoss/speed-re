#include "D3DXMatrixScalar.h"

// Original scalar D3DX library implementation 0x63cee1..0x63d118. Exact aliases
// use a temporary; distinct partially overlapping records retain direct stores.
WheelMatrix *__stdcall D3DXMatrixMultiplyScalar(WheelMatrix *output,const WheelMatrix *left,const WheelMatrix *right) {
    WheelMatrix temporary;
    float *r=(output==left || output==right) ? temporary.elements : output->elements;
    const float *a=left->elements,*b=right->elements;
    r[0]=((((static_cast<long double>(a[3])*static_cast<long double>(b[12]))+(static_cast<long double>(a[0])*static_cast<long double>(b[0])))+(static_cast<long double>(a[2])*static_cast<long double>(b[8])))+(static_cast<long double>(b[4])*static_cast<long double>(a[1])));
    r[1]=((((static_cast<long double>(a[1])*static_cast<long double>(b[5]))+(static_cast<long double>(a[3])*static_cast<long double>(b[13])))+(static_cast<long double>(a[2])*static_cast<long double>(b[9])))+(static_cast<long double>(b[1])*static_cast<long double>(a[0])));
    r[2]=((((static_cast<long double>(a[3])*static_cast<long double>(b[14]))+(static_cast<long double>(b[2])*static_cast<long double>(a[0])))+(static_cast<long double>(a[2])*static_cast<long double>(b[10])))+(static_cast<long double>(b[6])*static_cast<long double>(a[1])));
    r[3]=((((static_cast<long double>(a[3])*static_cast<long double>(b[15]))+(static_cast<long double>(b[11])*static_cast<long double>(a[2])))+(static_cast<long double>(a[0])*static_cast<long double>(b[3])))+(static_cast<long double>(b[7])*static_cast<long double>(a[1])));
    r[4]=((((static_cast<long double>(a[7])*static_cast<long double>(b[12]))+(static_cast<long double>(a[4])*static_cast<long double>(b[0])))+(static_cast<long double>(b[4])*static_cast<long double>(a[5])))+(static_cast<long double>(a[6])*static_cast<long double>(b[8])));
    r[5]=((((static_cast<long double>(a[6])*static_cast<long double>(b[9]))+(static_cast<long double>(a[7])*static_cast<long double>(b[13])))+(static_cast<long double>(b[5])*static_cast<long double>(a[5])))+(static_cast<long double>(b[1])*static_cast<long double>(a[4])));
    r[6]=((((static_cast<long double>(b[6])*static_cast<long double>(a[5]))+(static_cast<long double>(a[7])*static_cast<long double>(b[14])))+(static_cast<long double>(b[2])*static_cast<long double>(a[4])))+(static_cast<long double>(a[6])*static_cast<long double>(b[10])));
    r[7]=((((static_cast<long double>(b[7])*static_cast<long double>(a[5]))+(static_cast<long double>(a[7])*static_cast<long double>(b[15])))+(static_cast<long double>(a[6])*static_cast<long double>(b[11])))+(static_cast<long double>(a[4])*static_cast<long double>(b[3])));
    r[8]=((((static_cast<long double>(a[11])*static_cast<long double>(b[12]))+(static_cast<long double>(a[8])*static_cast<long double>(b[0])))+(static_cast<long double>(b[4])*static_cast<long double>(a[9])))+(static_cast<long double>(a[10])*static_cast<long double>(b[8])));
    r[9]=((((static_cast<long double>(a[11])*static_cast<long double>(b[13]))+(static_cast<long double>(a[10])*static_cast<long double>(b[9])))+(static_cast<long double>(b[5])*static_cast<long double>(a[9])))+(static_cast<long double>(b[1])*static_cast<long double>(a[8])));
    r[10]=((((static_cast<long double>(b[6])*static_cast<long double>(a[9]))+(static_cast<long double>(b[2])*static_cast<long double>(a[8])))+(static_cast<long double>(a[11])*static_cast<long double>(b[14])))+(static_cast<long double>(a[10])*static_cast<long double>(b[10])));
    r[11]=((((static_cast<long double>(a[11])*static_cast<long double>(b[15]))+(static_cast<long double>(a[10])*static_cast<long double>(b[11])))+(static_cast<long double>(b[7])*static_cast<long double>(a[9])))+(static_cast<long double>(a[8])*static_cast<long double>(b[3])));
    r[12]=((((static_cast<long double>(a[15])*static_cast<long double>(b[12]))+(static_cast<long double>(a[14])*static_cast<long double>(b[8])))+(static_cast<long double>(a[12])*static_cast<long double>(b[0])))+(static_cast<long double>(b[4])*static_cast<long double>(a[13])));
    r[13]=((((static_cast<long double>(a[15])*static_cast<long double>(b[13]))+(static_cast<long double>(b[5])*static_cast<long double>(a[13])))+(static_cast<long double>(a[14])*static_cast<long double>(b[9])))+(static_cast<long double>(b[1])*static_cast<long double>(a[12])));
    r[14]=((((static_cast<long double>(a[14])*static_cast<long double>(b[10]))+(static_cast<long double>(b[6])*static_cast<long double>(a[13])))+(static_cast<long double>(b[2])*static_cast<long double>(a[12])))+(static_cast<long double>(a[15])*static_cast<long double>(b[14])));
    r[15]=((((static_cast<long double>(a[15])*static_cast<long double>(b[15]))+(static_cast<long double>(b[7])*static_cast<long double>(a[13])))+(static_cast<long double>(a[12])*static_cast<long double>(b[3])))+(static_cast<long double>(b[11])*static_cast<long double>(a[14])));
    if (r!=output->elements) {
        unsigned int *to=reinterpret_cast<unsigned int *>(output->elements);
        const unsigned int *from=reinterpret_cast<const unsigned int *>(temporary.elements);
        for (int i=0;i<16;++i) to[i]=from[i];
    }
    return output;
}
